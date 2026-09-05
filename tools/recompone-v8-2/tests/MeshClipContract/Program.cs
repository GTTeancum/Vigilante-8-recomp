using System.Reflection;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

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
Console.WriteLine($"PASS: {checks} mesh clipping packet/provenance assertions");
