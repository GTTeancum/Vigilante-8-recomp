using System.Numerics;
using RecompOne.Runtime.Memory;
using static RecompOne.Runtime.Sdk.V82WaterAttachmentFit;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Intentional visual correction of retail water mounts. Translate the complete
/// assembly during packet generation; never change vertices or physics contacts.
/// The native folding rotation and all child animation remain authoritative.
/// </summary>
internal static class V82WaterAttachmentPlacement
{
    readonly record struct Mount(uint Address, Pose Rest, Pose Parent);
    internal readonly record struct Placement(uint Node, Vector3 Offset);
    sealed record Cached(ulong Signature, Placement[] Placements);
    static readonly Dictionary<uint, Cached> Cache = [];
    public static void Reset() => Cache.Clear();

    public static object? DescribeCached(uint vehicle) =>
        Cache.TryGetValue(vehicle, out var cache)
            ? cache.Placements.Select(p => new {
                node = p.Node, vehicleOffset = new[] { p.Offset.X, p.Offset.Y, p.Offset.Z }
            }).ToArray() : null;

    internal sealed class Scope(IMemory memory, (uint Address, uint X, uint Y, uint Z)[] saved) : IDisposable
    {
        bool disposed;
        public void Dispose()
        {
            if (disposed) return;
            disposed = true;
            foreach (var p in saved)
            {
                memory.WriteU32(p.Address + 0x34, p.X);
                memory.WriteU32(p.Address + 0x38, p.Y);
                memory.WriteU32(p.Address + 0x3C, p.Z);
            }
        }
    }

    // Retail changes mode at transition start and swaps meshes at tick 16.
    // A water mesh can therefore still be present while mode is already dry.
    internal static float Blend(byte mode, int ticks)
    {
        float t = mode == 2 ? (16 - ticks) / 16f : (ticks - 16) / 16f;
        t = Math.Clamp(t, 0, 1);
        return t * t * (3 - 2 * t);
    }

    public static Scope? Begin(IMemory m, uint vehicle)
    {
        if (!Valid(vehicle, 0x134)) return null;
        byte mode = m.ReadU8(vehicle + 0xAC);
        int ticks = m.ReadU16(vehicle + 0xB4);
        float blend = Blend(mode, ticks);
        if (blend == 0) return null;
        uint bank = V82VehicleRegistry.TransformBankForObject(m, vehicle, m.ReadU32(0x800C61C0));
        uint table = V82VehicleRegistry.TransformTableForObject(m, vehicle, 0x80063F74);
        if (!Valid(bank, 0x1C) || !Valid(table, 36)) return null;
        var candidates = new HashSet<uint>();
        for (uint i = 0; i < 6; i++)
        {
            uint node = m.ReadU32(vehicle + 0x104 + i * 4);
            if (IsSharedWaterSupport(m, node, bank, m.ReadU16(table + 24 + i * 2)))
                candidates.Add(node);
        }
        if (candidates.Count == 0) return null; // Preserve authored overrides.

        uint bodyBank = m.ReadU32(vehicle + 0x5C);
        var mounts = new List<Mount>();
        var bodies = new List<(uint Mesh, Pose Pose)>();
        var seen = new HashSet<uint>();
        ulong signature = 14695981039346656037UL;
        void Mix(uint x) => signature = unchecked((signature ^ x) * 1099511628211UL);
        void MixPose(Pose p)
        {
            foreach (var v in new[] { p.X, p.Y, p.Z, p.Position })
            { Mix(BitConverter.SingleToUInt32Bits(v.X)); Mix(BitConverter.SingleToUInt32Bits(v.Y)); Mix(BitConverter.SingleToUInt32Bits(v.Z)); }
        }
        void Visit(uint node, uint parentNode, Pose parent, int depth)
        {
            if (!Valid(node, 0x80) || depth > 24 || seen.Count >= 256 || !seen.Add(node)) return;
            if (node != vehicle && (m.ReadU32(node + 4) & 2) != 0) return;
            Pose local = node == vehicle ? Pose.Identity : ReadPose(m, node);
            Pose pose = parent.Compose(local);
            if (candidates.Contains(node))
            {
                // The temporary transition parent supplies the fold rotation.
                // Solve against the unfolded mount; apply the correction through
                // the actual parent inverse so the entire subtree moves rigidly.
                Pose rest = parentNode == vehicle ? local : local with {
                    Position = ReadPose(m, parentNode).Position + local.Position
                };
                mounts.Add(new(node, rest, parent));
                Mix(node); Mix(m.ReadU32(node + 0x40)); MixPose(rest);
                return; // Animated outboard children do not change the socket.
            }
            if (m.ReadU32(node + 0x5C) == bodyBank)
            {
                uint mesh = m.ReadU32(node + 0x40);
                if (mesh != 0)
                { bodies.Add((mesh, pose)); Mix(mesh); MixPose(pose); }
            }
            uint child = m.ReadU32(node + 0x10);
            for (int i = 0; i < 256 && Valid(child, 0x80) && !seen.Contains(child); i++)
            { Visit(child, node, pose, depth + 1); child = m.ReadU32(child + 0xC); }
        }
        Mix(bodyBank);
        Visit(vehicle, 0, Pose.Identity, 0);
        if (!Cache.TryGetValue(vehicle, out var cache) || cache.Signature != signature)
        {
            var body = new List<Triangle>();
            foreach (var b in bodies)
                if (!ReadTriangles(m, b.Mesh, b.Pose, body)) return null;
            if (body.Count == 0) return null;
            var placements = new List<Placement>();
            foreach (var mount in mounts)
            {
                if (!ReadVertices(m, m.ReadU32(mount.Address + 0x40), mount.Rest,
                        out var points, out _, out float unit)) continue;
                Vector3 offset = SocketOffset(points, mount.Rest.Position.X, unit, body);
                if (offset.LengthSquared() > 0) placements.Add(new(mount.Address, offset));
            }
            cache = new(signature, placements.ToArray());
            if (Cache.Count >= 256) Cache.Clear();
            bool first = !Cache.ContainsKey(vehicle);
            Cache[vehicle] = cache;
            if (first) Console.Error.WriteLine($"[WaterAttachmentPlacement] vehicle=0x{vehicle:X8} type={m.ReadU8(vehicle + 0xDC)} rigid-assemblies={placements.Count}");
        }
        var changes = new List<(uint Address, uint X, uint Y, uint Z)>();
        foreach (var placement in cache.Placements)
        {
            Mount mount = mounts.First(p => p.Address == placement.Node);
            Vector3 delta = mount.Parent.InversePoint(mount.Parent.Position + placement.Offset * blend);
            if (!float.IsFinite(delta.X) || !float.IsFinite(delta.Y) || !float.IsFinite(delta.Z)) continue;
            uint n = placement.Node;
            uint x = m.ReadU32(n + 0x34), y = m.ReadU32(n + 0x38), z = m.ReadU32(n + 0x3C);
            changes.Add((n, x, y, z));
            m.WriteU32(n + 0x34, unchecked((uint)((int)x + (int)MathF.Round(delta.X))));
            m.WriteU32(n + 0x38, unchecked((uint)((int)y + (int)MathF.Round(delta.Y))));
            m.WriteU32(n + 0x3C, unchecked((uint)((int)z + (int)MathF.Round(delta.Z))));
        }
        return changes.Count == 0 ? null : new Scope(m, changes.ToArray());
    }

    internal static Vector3 SocketOffset(Vector3[] points, float mountX, float unit, List<Triangle> body)
    {
        float inward = mountX < 0 ? 1 : -1;
        float edge = points.Max(p => p.X * inward);
        var end = points.Where(p => p.X * inward >= edge - unit).ToArray();
        Vector3 center = end.Aggregate(Vector3.Zero, (a, b) => a + b) / end.Length;
        Vector3 direction = new(inward, 0, 0);
        // Cast from outside the chassis. Casting inward from an embedded end
        // would find the opposite wall and wrongly move an already joined arm.
        float outside = body.SelectMany(t => new[] { t.A.X, t.B.X, t.C.X }).Min(x => x * inward) - unit;
        Vector3 origin = center with { X = outside * inward };
        float distance = float.MaxValue;
        foreach (var t in body)
            if (RayHit(origin, direction, t, out float d) && d < distance) distance = d;
        Vector3 target;
        if (distance != float.MaxValue)
        {
            target = origin + direction * distance;
            if ((target.X - center.X) * inward <= unit) return Vector3.Zero;
        }
        else
        {
            target = center;
            foreach (var t in body)
            {
                Vector3 nearest = Closest(center, t);
                float d = Vector3.DistanceSquared(center, nearest);
                if (d < distance) { distance = d; target = nearest; }
            }
            if (distance <= unit * unit) return Vector3.Zero;
        }
        Vector3 offset = target - center;
        return offset + Vector3.Normalize(offset) * unit;
    }
}
