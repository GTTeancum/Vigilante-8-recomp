using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Give exact mesh geometry to Enhanced rather than the PS1 screen clipper.</summary>
public static class V82MeshClipCompat
{
    static readonly bool Enabled =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_MODERN_MESH_CLIP") != "0";
    static int _traceCount;

    // All four retail subdividers share this scratch layout. They recursively
    // approximate affine texturing, discard leaves with median SZ < 128, and
    // test saturated SXY against the retail window. Enhanced instead has exact
    // camera vertices, perspective-correct UVs, and a real near-plane clipper.
    // Submit the original triangle with its native attributes and OT;
    // do not manufacture intersections from the saturated PS1 projection.
    public static bool SubmitTriangle(CpuContext c, IMemory m)
    {
        if (!Enabled || !ConfigManager.View.HighResolution3D ||
            !GpuHle.GameplayActive || V82Compat.ObjectRenderScopeDepth == 0)
            return true;
        return SubmitExactTriangle(c, m);
    }

    internal static bool SubmitExactTriangle(CpuContext c, IMemory m)
    {
        uint scratch = c.A1;
        uint command = m.ReadU32(scratch + 0x10u);
        uint kind = (command >> 24) & 0xFCu;
        if (kind is not (0x20u or 0x24u or 0x30u or 0x34u))
            return true;
        Span<PreciseGteVertexData> vertices = stackalloc PreciseGteVertexData[3];
        float minZ = float.MaxValue;
        float maxZ = float.MinValue;
        for (int i = 0; i < 3; i++)
        {
            uint xy = scratch + 0x24u + (uint)i * 0x18u;
            if (!m.TryGetPreciseGteVertex(xy, m.ReadU32(xy), out vertices[i]))
                return true;
            minZ = MathF.Min(minZ, vertices[i].ViewZ);
            maxZ = MathF.Max(maxZ, vertices[i].ViewZ);
            if (i > 0 && (vertices[i].ProjectionScale != vertices[0].ProjectionScale ||
                vertices[i].ProjectionCenterX != vertices[0].ProjectionCenterX ||
                vertices[i].ProjectionCenterY != vertices[0].ProjectionCenterY))
                return true;
        }
        if (maxZ < 1f)
        {
            c.V0 = c.A0;
            return false;
        }

        // Reproduce AVSZ3 and the caller's OT shift, as in the native leaf
        // emitters; only visibility/subdivision changes, not painter ordering.
        for (int i = 0; i < 3; i++)
            Gte.LoadMemoryWord(17 + i, m, scratch + 0x28u + (uint)i * 0x18u);
        Gte.Execute(0x4B58002Du);
        uint shift = m.ReadU16(scratch + 8u) & 31u;
        uint bucket = Math.Max(1u, Gte.Read(7) >> (int)shift);
        uint ot = m.ReadU32(scratch + 4u) + bucket * 4u;
        uint packet = c.A0;
        bool gouraud = (kind & 0x10u) != 0;
        bool textured = (kind & 4u) != 0;
        uint stride = (gouraud ? 8u : 4u) + (textured ? 4u : 0u);
        uint words = 1u + 3u * stride / 4u - (gouraud ? 1u : 0u);
        m.WriteU32(packet, (m.ReadU32(ot) & 0xFFFFFFu) | (words << 24));
        m.WriteU32(ot, packet & 0xFFFFFFu);
        m.WriteU32(packet + 4u, command);
        for (int i = 0; i < 3; i++)
        {
            uint source = scratch + 0x14u + (uint)i * 0x18u;
            uint xy = packet + 8u + (uint)i * stride;
            if (gouraud)
            {
                uint color = m.ReadU32(source + 0xCu) & 0xFFFFFFu;
                m.WriteU32(xy - 4u, color | (i == 0 ? command & 0xFF000000u : 0u));
            }
            m.WritePreciseGteVertex(xy, vertices[i]);
            if (textured)
            {
                uint uv = m.ReadU16(source + 8u);
                if (i < 2)
                    uv |= (uint)m.ReadU16(scratch + 0xCu + (uint)i * 2u) << 16;
                m.WriteU32(xy + 4u, uv);
            }
        }
        c.V0 = c.A0 = packet + (words + 1u) * 4u;
        if (_traceCount++ < 32)
            Console.Error.WriteLine($"[V82ModernMeshClip] kind={kind:X2} minZ={minZ:F3} maxZ={maxZ:F3} packet={packet:X8} bucket={bucket}");
        return false;
    }
}
