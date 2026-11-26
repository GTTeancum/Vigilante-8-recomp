using System.Numerics;
using System.Reflection;
using System.Security.Cryptography;
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

// Measure the production input preparation called by BeginTerrainRoutePacketWrites,
// without issuing GPU packets or needing original game data.
Environment.SetEnvironmentVariable("RECOMPONE_DIRECT_TERRAIN", "1");
Environment.SetEnvironmentVariable("RECOMPONE_VERIFY_SHARED_TERRAIN", "1");
Environment.SetEnvironmentVariable("RECOMPONE_V82_LOD", null);
Environment.SetEnvironmentVariable("RECOMPONE_LOD_MODE", null);
ConfigManager.View.HighResolution3D = true;
ConfigManager.View.LevelOfDetail = "Maximum";
GpuHle.DebugGameplayTick = 1;
var readCell = typeof(V82Compat)
    .GetMethod("ReadTerrainCellTextures", BindingFlags.Static | BindingFlags.NonPublic)!
    .CreateDelegate<Func<CpuContext, IMemory, GpuHle.TerrainCellTextures>>();
const uint table = 0x800B9470, descriptors = 0x800B7270;
var memory = new PSMemory();
var probe = new ReadProbe(memory);
var c = new CpuContext { GP = 0x800A0000 };
short[] rotation = [4096, -512, 256, 768, 3072, -1024, -1536, 2048, 3584];
int[] translation = [-1234, 456, 7890];
int[] origin = [123, -481, -389];
var authored = new Vector3[256];
bool hasAuthored = false;
bool baseline = args.Contains("--baseline");
using var output = new MemoryStream();
using var digest = new BinaryWriter(output);

static void Check(bool valid, string condition)
{
    if (!valid) throw new Exception(condition);
}
static uint Block(int px, int pz) => table + 4u * (uint)(px * 32 + pz);
void AddPage(int px, int pz, int seed)
{
    uint pointer = 0x80100000 + (uint)seed * 0x8000u;
    memory.WriteU32(Block(px, pz), pointer);
    for (uint i = 0; i < 4096; ++i)
    {
        uint shade = ((i / 64) * 11 + (i % 64) * 5 + (uint)seed * 7) & 31;
        memory.WriteU16(pointer + i * 2, (ushort)((shade << 11) | ((i * 53 + (uint)seed * 83) & 2047)));
    }
    for (uint i = 0; i < 0x2042; ++i)
        memory.WriteU8(pointer + 0x2000 + i, unchecked((byte)(i * 19 + (uint)seed * 43 + i / 64)));
}
uint Descriptor(int id) => descriptors + (uint)id * 32;
static Vector3 Rgb(uint word) => new(word & 255, (word >> 8) & 255, (word >> 16) & 255);
Vector3 Average(int id) => hasAuthored ? authored[id] : Rgb(memory.ReadU32(Descriptor(id) + 28));
void PreserveAuthored()
{
    for (int i = 0; i < 256; ++i) authored[i] = Rgb(memory.ReadU32(Descriptor(i) + 28));
    V82Compat.CaptureAuthoredTerrainMaterialAverages(c, memory);
    hasAuthored = true;
}
void InstallCamera()
{
    for (int i = 0; i < 4; ++i)
        Gte.WriteControl(i, (uint)(ushort)rotation[2*i] | ((uint)(ushort)rotation[2*i+1] << 16));
    Gte.WriteControl(4, (ushort)rotation[8]);
    for (int i = 0; i < 3; ++i)
    {
        Gte.WriteControl(5 + i, unchecked((uint)translation[i]));
        memory.WriteU32(0x1F800084 + (uint)i * 4, unchecked((uint)origin[i]));
    }
    Gte.WriteControl(24, 0x00884000); // 136.25
    Gte.WriteControl(25, unchecked((uint)-0x00038000)); // -3.5
    Gte.WriteControl(26, 287);
}
void Mix(Vector3 v)
{
    digest.Write(v.X); digest.Write(v.Y); digest.Write(v.Z);
}
uint Page(int x, int z) => (uint)x < 2048 && (uint)z < 2048 ? memory.ReadU32(Block(x / 64, z / 64)) : 0;
uint Record(int x, int z)
{
    uint page = Page(x, z);
    return page is >= 0x80000000 and <= 0x807FBFBE
        ? page + 2u * (uint)((x % 64) * 64 + (z % 64)) : 0;
}
Vector3 TileAverage(int x, int z)
{
    uint record = Record(x, z);
    return Average(record == 0 ? 0 : memory.ReadU8(record + 0x2000));
}
void Evaluate(string label, int x, int z, bool counted = true, bool highDetail = true, bool valid = true)
{
    c.A0 = (uint)x; c.A1 = (uint)z;
    probe.Reset();
    var result = readCell(c, counted ? probe : memory);
    var reads = probe.Reads;
    Check(result.Valid == valid, label + ": cell validity");
    if (!valid) return;
    Check(result.GridSize == 4 && result.Tiles is { Length: 16 }, label + ": tile grid");
    uint textureGrid = Page(x,z) + 0x2000 + (uint)((x % 64) * 64 + (z % 64));
    for (int tx = 0; tx < 4; ++tx)
    for (int tz = 0; tz < 4; ++tz)
    {
        byte id = memory.ReadU8(textureGrid + (uint)(tx * 64 + tz));
        uint d = Descriptor(id);
        var average = Average(id);
        var expected = new GpuHle.TerrainTextureDescriptor(id, memory.ReadU16(d), memory.ReadU16(d + 4),
            memory.ReadU16(d + 8), memory.ReadU16(d + 12), memory.ReadU16(d + 6),
            memory.ReadU16(c.GP + 0xDA8), memory.ReadU8(d + 31), (byte)average.X, (byte)average.Y, (byte)average.Z);
        Check(result.Get(tx,tz) == expected, label + ": texture descriptor");
        digest.Write(id); digest.Write(expected.Uv00); digest.Write(expected.Flags); Mix(average);
    }
    var colors = result.DistanceColors;
    Check(colors.Valid == highDetail && (colors.Patch != null) == highDetail, label + ": lighting gate");
    if (!highDetail) return;
    var patch = colors.Patch!;
    float R(int index) => rotation[index] / 4096f;
    Check(patch.Samples.Length == 25 && patch.HeightAxis == new Vector3(R(1), R(4), R(7)) &&
        patch.ProjectionCenterX == 136.25f && patch.ProjectionCenterY == -3.5f && patch.ProjectionScale == 287,
        label + ": camera snapshot");
    Check(colors.Alternate == (unchecked((sbyte)memory.ReadU8(c.GP + 0x31)) >= 6), label + ": signed distance mode");
    Vector3 low = Rgb(memory.ReadU32(c.GP + 0xE04)), high = Rgb(memory.ReadU32(c.GP + 0xDAC));
    Span<GpuHle.TerrainDistanceColor> corners = stackalloc GpuHle.TerrainDistanceColor[4];
    for (int sx = 0; sx < 5; ++sx)
    for (int sz = 0; sz < 5; ++sz)
    {
        int wx = x + sx, wz = z + sz;
        uint record = Record(wx, wz);
        ushort packed = record == 0 ? (ushort)0 : memory.ReadU16(record);
        float height = (packed & 2047) * 8;
        float vx = unchecked((short)(wx * 256 + origin[0]));
        float vy = unchecked((short)((int)height + origin[1]));
        float vz = unchecked((short)(wz * 256 + origin[2]));
        float Position(int row) => translation[row] + R(row*3)*vx + R(row*3+1)*vy + R(row*3+2)*vz;
        int Shade(float a, float b) => (int)a + (int)((b - a) * (packed >> 11) / 31);
        Vector3 ramp = new(Shade(low.X, high.X), Shade(low.Y, high.Y), Shade(low.Z, high.Z));
        Vector3 total = TileAverage(wx - 1, wz - 1) + TileAverage(wx - 1, wz) +
            TileAverage(wx, wz - 1) + TileAverage(wx, wz);
        Vector3 flat = new((int)(ramp.X * total.X) >> 10, (int)(ramp.Y * total.Y) >> 10, (int)(ramp.Z * total.Z) >> 10);
        var sample = patch.Samples[sx * 5 + sz];
        Check(sample == new DreamcastTerrainGeometry.Sample(new(Position(0), Position(1), Position(2)), height, ramp, flat),
            label + $": sample {sx},{sz}");
        if ((sx == 0 || sx == 4) && (sz == 0 || sz == 4))
            corners[sx / 4 + (sz / 4) * 2] = new((byte)ramp.X, (byte)ramp.Y, (byte)ramp.Z, (byte)flat.X, (byte)flat.Y, (byte)flat.Z);
        Mix(sample.View); digest.Write(sample.Height); Mix(sample.Ramp); Mix(sample.Flat);
    }
    Check(colors.TopLeft == corners[0] && colors.TopRight == corners[1] &&
        colors.BottomLeft == corners[2] && colors.BottomRight == corners[3], label + ": corner order and colors");
    if (counted)
    {
        int limit = hasAuthored ? 221 : 377;
        Check(baseline || reads.Bytes + reads.HalfWords + reads.Words <= limit,
            label + ": terrain patch exceeded guest read budget");
        Console.WriteLine($"{label}: read8={reads.Bytes}, read16={reads.HalfWords}, read32={reads.Words}");
    }
}
void Shared(string label)
{
    V82Compat.BeginSharedTerrainFrame();
    try { Evaluate(label, 382, 446, counted: false); }
    finally { V82Compat.EndSharedTerrainFrame(); }
}

AddPage(5, 6, 1); AddPage(5, 7, 2); AddPage(6, 6, 3); AddPage(6, 7, 4);
AddPage(0, 0, 5); AddPage(31, 31, 6);
for (int i = 0; i < 256; ++i)
{
    uint d = Descriptor(i);
    for (uint j = 0; j < 16; j += 2) memory.WriteU16(d + j, (ushort)(i * 41 + j * 19));
    memory.WriteU32(d + 28, 0x53421D39u ^ ((uint)i * 0x00011503u));
}
memory.WriteU32(c.GP + 0xDA4, 0x0009D4FE); // unrelated fog word
memory.WriteU16(c.GP + 0xDA8, 0x2394);
memory.WriteU32(c.GP + 0xE04, 0x11EF23C1);
memory.WriteU32(c.GP + 0xDAC, 0x7722E9A3);
memory.WriteU8(c.GP + 0x31, 5);
InstallCamera();
Evaluate("live interior", 332, 403);
Evaluate("live cross-page", 382, 446);
V82Compat.BeginSharedTerrainFrame();
try { Evaluate("live cached source", 382, 446, counted: false); }
finally { V82Compat.EndSharedTerrainFrame(); }
memory.WriteU32(Descriptor(0) + 28, 0x7587F12C);
Evaluate("live lower and missing neighbors", 0, 0);
PreserveAuthored();
Evaluate("authored interior", 332, 403);
Evaluate("authored cross-page", 382, 446);
Shared("authored cache seed");
memory.WriteU8(c.GP + 0x31, 6);
Evaluate("authored upper boundary", 2047, 2047);
uint lastPage = Block(6,7), savedPage = memory.ReadU32(lastPage);
memory.WriteU32(lastPage, 0x807FFFC0); // looks like a RAM pointer, but cannot hold the full page
memory.WriteU8(c.GP + 0x31, 0x86); // signed negative, not alternate
Evaluate("authored invalid neighbor page", 382, 446);
memory.WriteU32(lastPage, savedPage);
memory.WriteU8(c.GP + 0x31, 5);
Evaluate("invalid origin page", 1000, 1000, valid: false);
Evaluate("origin outside world", 2048, 2048, valid: false);
for (int i = 0; i < 256; ++i)
{
    memory.WriteU32(Descriptor(i) + 28, 0xCDBB8844u ^ (uint)i);
    memory.WriteU16(Descriptor(i), (ushort)(i * 5));
}
translation[0] = -3211;
rotation[3] = -1522;
origin[2] = 721;
InstallCamera();
Shared("cache updates camera and descriptor UVs but keeps authored averages");
uint changed = Record(384, 449);
memory.WriteU16(changed, 0x9DEC);
memory.WriteU8(changed + 0x2000, 0);
Shared("cache invalidates edited neighbor height and texture");
PreserveAuthored();
Shared("cache invalidates recaptured authored averages");
var verifiedShared = typeof(V82Compat).GetField("_sharedTerrainVerified", BindingFlags.Static | BindingFlags.NonPublic)!;
Check((long)verifiedShared.GetValue(null)! > 0, "never exercised shared-cache reuse");

// Measure retained output allocations independently of the read counter or
// assertion allocations. The per-call neighbor input is not retained.
c.A0 = 332; c.A1 = 403;
for (int i = 0; i < 256; ++i) _ = readCell(c, memory);
const int trials = 1024;
long startBytes = GC.GetAllocatedBytesForCurrentThread();
for (int i = 0; i < trials; ++i) _ = readCell(c, memory);
long allocated = GC.GetAllocatedBytesForCurrentThread() - startBytes;
Console.WriteLine($"managed allocation across {trials} authored patch builds: {allocated} bytes");
ConfigManager.View.HighResolution3D = false;
Evaluate("native resolution keeps old lighting gate", 382, 446, highDetail: false);
ConfigManager.View.HighResolution3D = true;
ConfigManager.View.LevelOfDetail = "Stock";
Evaluate("stock detail keeps old lighting gate", 382, 446, highDetail: false);
Console.WriteLine($"lighting output SHA-256 {Convert.ToHexString(SHA256.HashData(output.ToArray()))}");

sealed class ReadProbe(PSMemory inner) : IMemory
{
    public (int Bytes, int HalfWords, int Words) Reads;
    public void Reset() => Reads = default;
    public byte ReadU8(uint address) { ++Reads.Bytes; return inner.ReadU8(address); }
    public ushort ReadU16(uint address) { ++Reads.HalfWords; return inner.ReadU16(address); }
    public uint ReadU32(uint address) { ++Reads.Words; return inner.ReadU32(address); }
    public uint ReadWordLeft(uint current, uint address) => throw new NotSupportedException();
    public uint ReadWordRight(uint current, uint address) => throw new NotSupportedException();
    public void WriteU8(uint address, byte value) => throw new NotSupportedException();
    public void WriteU16(uint address, ushort value) => throw new NotSupportedException();
    public void WriteU32(uint address, uint value) => throw new NotSupportedException();
    public void WriteWordLeft(uint address, uint value) => throw new NotSupportedException();
    public void WriteWordRight(uint address, uint value) => throw new NotSupportedException();
    public void WriteGteWord(uint address, int register) => throw new NotSupportedException();
    public bool TryGetPreciseGteVertex(uint address, uint packedScreenPosition, out PreciseGteVertexData vertex) =>
        throw new NotSupportedException();
    public void WritePreciseGteVertex(uint address, in PreciseGteVertexData vertex) => throw new NotSupportedException();
    public void LoadBytes(uint address, byte[] data) => throw new NotSupportedException();
    public void ZeroRange(uint address, uint length) => throw new NotSupportedException();
    public void SetCd(RecompOne.Runtime.Cdrom.CdController cd) => throw new NotSupportedException();
}
