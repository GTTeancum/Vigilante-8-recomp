using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Policy controller only: the retail routine still constructs, animates and
/// drives water attachments. Positions, velocities and buoyancy are never
/// overwritten here. Coordinates use native fixed point with Y downward.
/// </summary>
public static class V82AutoWaterski
{
    const uint TransformAddress = 0x8003E32C;
    const uint TerrainHeightAddress = 0x8001B750;
    const int TransitionTicks = 32;
    const int DryTicksRequired = 20;
    const int SurfaceApproach = 0x10000;
    sealed class State { public bool Owned; public int DryTicks; }
    static readonly Dictionary<uint, State> States = [];

    public static void Reset() { States.Clear(); V82WaterAttachmentFit.Reset(); }
    public static void Forget(uint vehicle) => States.Remove(vehicle);

    public static bool HasWaterMovement(IMemory memory, uint vehicle) =>
        memory.ReadU8(vehicle + 0xAC) == V82TransformationPolicy.WaterMode ||
        V82VehicleRegistry.UsesFlyingController(memory, vehicle);

    public static void BeforeVehiclePhysics(CpuContext c, IMemory memory)
    {
        if (!GpuHle.GameplayActive || c.A0 == 0) return;
        var m = Dispatcher.UnwrapMemory(memory);
        uint vehicle = c.A0;
        var policy = V82TransformationSettings.Normalize(ConfigManager.Game.V82Transformations);
        States.TryGetValue(vehicle, out var state);
        if (policy == V82TransformationMode.All && state == null) return;
        if (m.ReadU8(vehicle + 8) != 2 || m.ReadU16(vehicle + 0x1C) == 0 ||
            !V82VehicleRegistry.CanTransform(m, vehicle) ||
            V82VehicleRegistry.UsesFlyingController(m, vehicle))
        {
            States.Remove(vehicle);
            return;
        }
        byte mode = m.ReadU8(vehicle + 0xAC);
        ushort transition = m.ReadU16(vehicle + 0xB4);
        if (policy != V82TransformationMode.AutoWaterski)
        {
            if (mode != 0 && transition == 0 &&
                (policy == V82TransformationMode.None || state?.Owned == true))
                Request(c, m, vehicle, 0, "policy-change");
            if (m.ReadU8(vehicle + 0xAC) == 0) States.Remove(vehicle);
            return;
        }

        state ??= new State();
        States[vehicle] = state;
        int x = I32(m, vehicle + 0x34), y = I32(m, vehicle + 0x38), z = I32(m, vehicle + 0x3C);
        int plane = I32(m, c.GP + 0xDB0);
        int boundaryZ = I32(m, c.GP + 0xDA0);
        int dx = I32(m, vehicle + 0x80) >> 7;
        int dy = I32(m, vehicle + 0x84) >> 7;
        int dz = I32(m, vehicle + 0x88) >> 7;
        bool water = false;
        bool dry = false;
        if (plane != 0)
        {
            int ground = Height(c, m, x, z);
            bool region = InImportedHazard(m, c.GP, x, z);
            water = SurfaceRisk(plane, boundaryZ, x, y, z, ground, region);
            // Check the path through the native 32-tick animation, not just
            // its endpoint: a narrow water crossing must not be skipped.
            for (int tick = 8; !water && tick <= TransitionTicks; tick += 8)
            {
                int px = unchecked(x + dx * tick), pz = unchecked(z + dz * tick);
                int py = unchecked(y + Math.Max(0, dy) * tick);
                water = SurfaceRisk(plane, boundaryZ, px, py, pz,
                    Height(c, m, px, pz), InImportedHazard(m, c.GP, px, pz));
            }
            // Retraction requires dry ground, not a moment airborne above
            // water. Spatial margin plus a sustained dry interval avoids
            // repeatedly rebuilding the wheel hierarchy at the shoreline.
            dry = !water && (ground <= (long)plane - 0x1000 || (!region && z >= boundaryZ)) &&
                Math.Abs((long)ground - y) <= SurfaceApproach;
        }
        else dry = true;

        state.DryTicks = dry ? Math.Min(state.DryTicks + 1, DryTicksRequired) : 0;
        if (transition != 0) return; // Respect the native animation lock.
        if (water)
        {
            if (mode != V82TransformationPolicy.WaterMode)
            {
                Request(c, m, vehicle, V82TransformationPolicy.WaterMode, "water-approach");
                state.Owned = m.ReadU8(vehicle + 0xAC) == V82TransformationPolicy.WaterMode;
            }
            else state.Owned = true;
        }
        else if (mode != 0 &&
            (mode != V82TransformationPolicy.WaterMode || state.DryTicks >= DryTicksRequired))
        {
            Request(c, m, vehicle, 0, "dry-ground");
            state.Owned = false;
        }
    }

    public static bool SurfaceRisk(int plane, int boundaryZ, int x, int y, int z,
        int terrainY, bool importedRegion) =>
        plane != 0 && (z < boundaryZ || importedRegion) &&
        terrainY > (long)plane + 0x1000 && y >= (long)plane - SurfaceApproach;

    static bool InImportedHazard(IMemory m, uint gp, int x, int z)
    {
        uint node = m.ReadU32(gp + 0x10D8);
        for (int count = 0; count < 256 && node >= 0x80010000 && node < 0x80800000 - 0x20; count++)
        {
            uint next = m.ReadU32(node);
            if (next == 0) break;
            if (m.ReadU16(node + 8) == 0x0043 && m.ReadU16(node + 0xA) == 0xFFFF)
            {
                int rx = (short)m.ReadU16(node + 0xC), rz = (short)m.ReadU16(node + 0xE);
                int tx = x >> 16, tz = z >> 16;
                if (tx >= rx && tx <= rx + m.ReadU16(node + 0x10) &&
                    tz >= rz && tz <= rz + m.ReadU16(node + 0x12)) return true;
            }
            node = next;
        }
        return false;
    }

    static int I32(IMemory m, uint p) => unchecked((int)m.ReadU32(p));
    static int Height(CpuContext c, IMemory m, int x, int z)
    {
        var snapshot = c.Snapshot();
        try
        {
            c.A0 = unchecked((uint)x); c.A1 = unchecked((uint)z);
            Dispatcher.Call(c, m, TerrainHeightAddress);
            return unchecked((int)c.V0);
        }
        finally { c.Restore(snapshot); }
    }

    static void Request(CpuContext c, IMemory m, uint vehicle, uint mode, string reason)
    {
        var snapshot = c.Snapshot();
        try
        {
            using var permission = V82TransformationPolicy.AuthorizeWaterTransition(vehicle);
            c.A0 = vehicle; c.A1 = mode;
            // +12C is a damage shield, not an expiration clock. Automatic
            // equipment must not repeatedly grant the pickup's 500 HP shield.
            c.A2 = 0;
            Dispatcher.Call(c, m, TransformAddress);
            Console.Error.WriteLine($"[V82AutoWaterski] vehicle=0x{vehicle:X8} reason={reason} " +
                $"requested={mode} actual={m.ReadU8(vehicle + 0xAC)} transition={m.ReadU16(vehicle + 0xB4)} " +
                $"shield={m.ReadU16(vehicle + 0x12C)} tick={GpuHle.DebugGameplayTick}");
        }
        finally { c.Restore(snapshot); }
    }
}
