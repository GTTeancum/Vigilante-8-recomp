using System.Reflection;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;

// Real runtime memory/provenance and GTE, no game, graphics or audio launch.
var submit = typeof(V82MeshClipCompat).GetMethod("SubmitExactTriangle",
    BindingFlags.NonPublic | BindingFlags.Static)!;
const uint scratch = 0x1F800000, packet = 0x10000, table = 0x20000;
int checks = 0;
void Check(bool value, string message)
{
    if (!value) throw new Exception(message);
    checks++;
}
(PSMemory m, CpuContext c, PreciseGteVertexData[] vertices) Fixture(uint kind, float[] depths)
{
    var m = new PSMemory();
    var c = new CpuContext { A0 = packet, A1 = scratch, SP = 0x30000, RA = 0x12345678 };
    Gte.WriteControl(29, 0x555); // ZSF3
    m.WriteU32(scratch + 4, table);
    m.WriteU16(scratch + 8, 0);
    m.WriteU16(scratch + 0xC, 0x7E46);
    m.WriteU16(scratch + 0xE, 0x413);
    m.WriteU32(scratch + 0x10, (kind << 24) | 0x123456);
    var vertices = new PreciseGteVertexData[3];
    for (int i = 0; i < 3; i++)
    {
        uint source = scratch + 0x14u + (uint)i * 0x18;
        uint packed = (uint)(ushort)(-300 + i * 10) | ((uint)(100 + i * 20) << 16);
        ushort depth = (ushort)Math.Clamp(depths[i], 0, 65535);
        vertices[i] = new(packed, depth, depths[i], -180 + i * 20, i * 4,
            depths[i], -300 + i * 10, 100 + i * 20, 160, 120, 256);
        m.WritePreciseGteVertex(source + 0x10, vertices[i]);
        m.WriteU32(source + 0x14, depth);
        m.WriteU16(source + 8, (ushort)(0xAA00 + i * 20));
        m.WriteU32(source + 0xC, 0x445566u + (uint)i);
    }
    return (m, c, vertices);
}
bool Submit(CpuContext c, IMemory m) => (bool)submit.Invoke(null, [c, m])!;
foreach (uint kind in new uint[] {0x20, 0x24, 0x30, 0x34})
foreach (float[] depths in new[] {new float[] {40, 90, 260}, new float[] {180, 200, 240}, new float[] {-40, 30, 180}})
{
    var (m, c, vertices) = Fixture(kind, depths);
    Check(!Submit(c, m), $"{kind:X2}: must submit exact visible/crossing triangle");
    bool textured = (kind & 4) != 0, gouraud = (kind & 16) != 0;
    uint stride = (gouraud ? 8u : 4u) + (textured ? 4u : 0u);
    uint words = kind switch {0x20 => 4, 0x24 => 7, 0x30 => 6, _ => 9};
    Check(m.ReadU32(packet) >> 24 == words, "packet length");
    Check(c.V0 == packet + 4 * (words + 1) && c.A0 == c.V0, "native return pointer");
    Check(c.SP == 0x30000 && c.RA == 0x12345678, "caller stack and return address preserved");
    uint bucket = Math.Max(1, Gte.Read(7));
    Check(m.ReadU32(table + bucket * 4) == packet, "native ordering table link");
    for (int i = 0; i < 3; i++)
    {
        uint xy = packet + 8u + (uint)i * stride;
        Check(m.TryGetPreciseGteVertex(xy, m.ReadU32(xy), out var actual) && actual == vertices[i],
            "exact source camera coordinates retained, including behind-camera endpoints");
        if (textured)
            Check(m.ReadU16(xy + 4) == 0xAA00 + i * 20, "authored UV retained");
        if (gouraud)
            Check((m.ReadU32(xy - 4) & 0xFFFFFF) == 0x445566 + i, "authored Gouraud color retained");
    }
    Check(m.ReadU32(packet + 4) >> 24 == kind, "native command retained");
    if (textured)
    {
        Check(m.ReadU16(packet + 14) == 0x7E46, "CLUT retained");
        Check(m.ReadU16(packet + 8 + stride + 6) == 0x413, "tpage/blend flags retained");
    }
}
{
    var (m, c, _) = Fixture(0x24, [-40, -20, 0]);
    Check(!Submit(c, m) && c.V0 == packet && m.ReadU32(packet) == 0, "fully behind rejected without packet");
}
{
    var (m, c, _) = Fixture(0x24, [20, 50, 100]);
    m.WriteU32(scratch + 0x3C, 0xBAD); // invalidate exact address/value provenance
    Check(Submit(c, m) && m.ReadU32(packet) == 0, "incomplete provenance falls back without partial output");
}
{
    var (m, c, v) = Fixture(0x24, [20, 50, 100]);
    m.WritePreciseGteVertex(scratch + 0x3C, v[1] with { ProjectionScale = 128 });
    Check(Submit(c, m) && m.ReadU32(packet) == 0, "mixed camera projections fall back");
}
{
    HleVertex V(float x, float y, float z = 100) => new()
    {
        ViewX = x,
        ViewY = y,
        ViewZ = z,
        ProjectionCenterX = 160,
        ProjectionCenterY = 120,
        ProjectionScale = 256,
        HasViewSpace = true,
    };
    Check(EnhancedGlBackend.DreamcastCullClockwise(
            V(0, 0), V(10, 0), V(0, 10)),
        "Dreamcast PVR_CULLING_CW removes positive screen-space area");
    Check(!EnhancedGlBackend.DreamcastCullClockwise(
            V(0, 0), V(0, 10), V(10, 0)),
        "Dreamcast PVR_CULLING_CW retains counter-clockwise area");
    Check(!EnhancedGlBackend.DreamcastCullClockwise(
            V(0, 0), V(5, 5), V(10, 10)),
        "Dreamcast PVR_CULLING_CW retains a degenerate strip face");
    Check(EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.WaterBase),
        "Dreamcast untextured water pass disables face culling");
    Check(EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.WaterSurface),
        "Dreamcast textured water pass disables face culling");
    Check(!EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.Opaque),
        "ordinary opaque material does not inherit the water no-cull rule");
}
{
    // Unproject each water vertex using its submitted depth. Every result
    // must lie on the authored plane, independently of viewport resolution.
    foreach (int projection in new[] { 128, 256, 512 })
    foreach (var normal in new[] { (X: (short)0, Y: (short)4066, Z: (short)495),
                                  (X: (short)4066, Y: (short)495, Z: (short)0) })
    {
        int width = projection * 5 / 3, height = projection * 15 / 16;
        const int delta = 137573;
        var plane = V82Compat.BuildDreamcastWaterBaseQuad(
            normal.X, normal.Y, normal.Z, delta, projection, width, height);
        foreach (var vertex in new[] { plane.V0, plane.V1, plane.V2, plane.V3 })
        {
            float z = vertex.CameraDepth / 256f;
            float x = (vertex.X - width * 0.5f) * z / projection;
            float y = (vertex.Y - height * 0.5f) * z / projection;
            float distance = (normal.X * x + normal.Y * y + normal.Z * z) / 4096f;
            Check(MathF.Abs(distance - delta / 65536f) < 0.0001f,
                "water raster depth and projected position meet the same world plane");
        }
    }
}
{
    // Captured from the shared PS1 base-water function on Louisiana. The
    // expected vertices are the direct algebra and fixed reciprocal-depth
    // planes decoded from Dreamcast 8C0D72A0, not values from this helper.
    var quad = V82Compat.BuildDreamcastWaterBaseQuad(
        0,
        4066,
        495,
        3092480 - 2954907,
        256,
        427,
        240);
    Check(MathF.Abs(quad.V0.X) < 0.001f &&
          MathF.Abs(quad.V2.X) < 0.001f,
        "Dreamcast horizontal water strip begins at viewport left");
    Check(MathF.Abs(quad.V1.X - 427f) < 0.001f &&
          MathF.Abs(quad.V3.X - 427f) < 0.001f,
        "Dreamcast horizontal water strip reaches viewport right");
    Check(MathF.Abs(quad.V0.Y - 132.89019f) < 0.001f &&
          MathF.Abs(quad.V1.Y - 132.89019f) < 0.001f,
        "Dreamcast first reciprocal-depth edge matches retail equation");
    Check(MathF.Abs(quad.V2.Y - 97.09473f) < 0.001f &&
          MathF.Abs(quad.V3.Y - 97.09473f) < 0.001f,
        "Dreamcast second reciprocal-depth edge matches retail equation");
    Check(MathF.Abs(quad.V0.CameraDepth - 3145.728f) < 0.001f &&
          MathF.Abs(quad.V2.CameraDepth - 16777.217f) < 0.01f,
        "Dreamcast reciprocal water depths convert to PS1 camera units");
}
{
    var quad = V82Compat.BuildDreamcastWaterBaseQuad(
        4096,
        0,
        0,
        0,
        256,
        320,
        240);
    Check(MathF.Abs(quad.V0.Y) < 0.001f &&
          MathF.Abs(quad.V2.Y) < 0.001f &&
          MathF.Abs(quad.V1.Y - 240f) < 0.001f &&
          MathF.Abs(quad.V3.Y - 240f) < 0.001f,
        "Dreamcast vertical water branch spans the viewport height");
    Check(MathF.Abs(quad.V0.X - 160f) < 0.001f &&
          MathF.Abs(quad.V3.X - 160f) < 0.001f,
        "Dreamcast vertical water branch retains the decoded centre");
}
{
    // Exact Louisiana camera installed by the retail PS1 water routine. The
    // coordinate seam was independently reproduced from the traced fixed-
    // point transform before this Dreamcast XWAT mesh builder was added.
    short[] cameraToWorld =
    [
        -2255, -414, 3394,
        0, 4066, 495,
        -3420, 272, -2239,
    ];
    var mesh = V82Compat.BuildDreamcastWaterSurfaceMesh(
        0,
        4066,
        495,
        137573,
        256,
        427,
        240,
        cameraToWorld,
        57564424,
        2954907,
        82733044,
        150);
    Check(mesh != null, "Dreamcast XWAT mesh exists for traced Louisiana camera");
    Check(mesh!.Rows is > 0 and <= 31 && mesh.Columns is > 0 and <= 31,
        "Dreamcast XWAT cell spans retain the decoded 31-cell caps");
    Check(mesh.Vertices.Length == mesh.Rows * mesh.Columns * 6,
        "Dreamcast XWAT row strips triangulate every rectangular cell");
    Check(mesh.Vertices.All(vertex =>
            float.IsFinite(vertex.X) &&
            float.IsFinite(vertex.Y) &&
            float.IsFinite(vertex.CameraDepth) &&
            float.IsFinite(vertex.U) &&
            float.IsFinite(vertex.V)),
        "Dreamcast XWAT projection and centered-slope UVs remain finite");
    Check(mesh.Vertices.All(vertex => vertex.CameraDepth >= 1f),
        "Dreamcast XWAT vertices retain positive host camera depth");
    var repeated = V82Compat.BuildDreamcastWaterSurfaceMesh(
        0, 4066, 495, 137573, 256, 427, 240, cameraToWorld,
        57564424, 2954907, 82733044, 150);
    Check(repeated != null && mesh.Vertices.SequenceEqual(repeated.Vertices),
        "Dreamcast XWAT mesh is deterministic for one retail camera/tick");
}
{
    // The panorama emitter links static packets into the far OT bucket;
    // scenery advances the dynamic packet cursor. Their lifetimes differ.
    var m = new PSMemory();
    var c = new CpuContext { GP = 0x40000 };
    const uint ot = 0x50000, oldPacket = 0x60000;
    const uint skyA = 0x61000, skyB = 0x62000, backdrop = 0x63000;
    m.WriteU32(c.GP + 0xCE0, ot);
    m.WriteU32(ot + 0x3FFC, oldPacket);
    m.WriteU32(c.GP + 0x610, 0x70000);
    GpuHle.ClearPacketOwners();
    V82Compat.BeginSceneryPass(c, m);
    m.WriteU32(c.GP + 0x610, 0x70100);
    V82Compat.EndSceneryPass(c, m);
    Check(!GpuHle.IsSkyPacket(0x70040),
        "scenery packets must never inherit panorama edge extension");
    V82Compat.BeginLatePass(c, m);
    m.WriteU32(skyA, (9u << 24) | oldPacket);
    m.WriteU32(skyB, (9u << 24) | skyA);
    m.WriteU32(backdrop, (11u << 24) | skyB);
    m.WriteU32(ot + 0x3FFC, backdrop);
    V82Compat.EndLatePass(c, m);
    Check(GpuHle.IsSkyPacket(skyA) && GpuHle.IsSkyPacket(skyB),
        "both static panorama packets belong to the actual emitter");
    Check(!GpuHle.IsSkyPacket(oldPacket),
        "panorama provenance stops at the previous ordering-table head");
    Check(!GpuHle.IsSkyPacket(0x70040),
        "panorama provenance does not include dynamic scenery arena");
    GpuHle.ClearPacketOwners();
}
Console.WriteLine($"PASS: {checks} mesh clipping packet/provenance assertions");
