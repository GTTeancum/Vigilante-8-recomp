// Source-only public-entry-point contract. No disc, generated game module or GPU.
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

Environment.SetEnvironmentVariable("RECOMPONE_V82_LOD", null);
Environment.SetEnvironmentVariable("RECOMPONE_LOD_MODE", null);
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_PACKET_ARENAS", null);
Runtime.SetMode(RunMode.Devkit);
GpuHle.GameplayActive = true;
var cpu = new CpuContext { GP = 0x80010000 };
var memory = new PSMemory();
var textures = new GpuHle.TerrainCellTextures([default], 1, true, default);
string[] names = ["first", "second", "replacement"];

GpuHle.TerrainTransitionPacket Transition(uint address, int version) =>
    new(textures, 1, 2, 3, 4, 5, 6, names[version], address, (uint)version);
GpuHle.TriangleNclipPacket Winding(uint address, int version) =>
    new(address, -13 - version, 12.5 + version, true, true, 1, 2, 0, 11, 22, 33, 44, 55, 66);

void Register(uint address, int version)
{
    uint alias = address | 0xA0000000u;
    GpuHle.RegisterCoarseTerrainPacket(alias, textures, version == 1, names[version], address, (uint)version);
    GpuHle.RegisterTerrainTransitionPacket(alias, Transition(address, version));
    GpuHle.RegisterTriangleNclipPacket(alias, Winding(address, version));
    GpuHle.RegisterPacketOwner(alias, names[version]);
}

void Check(uint address, int version)
{
    uint alias = address | 0x80000000u;
    bool present = version >= 0;
    if (GpuHle.TryGetCoarseTerrainPacket(alias, out var c) != present ||
        GpuHle.TryGetTerrainTransitionPacket(alias, out var t) != present ||
        GpuHle.TryGetTriangleNclipPacket(alias, out var w) != present ||
        GpuHle.DescribePacketOwner(alias) != (present ? names[version] : "unresolved") ||
        (present && (c != new GpuHle.CoarseTerrainPacket(textures, version == 1, names[version], address, (uint)version) ||
                     t != Transition(address, version) || w != Winding(address, version))))
        throw new Exception($"Wrong packet data at {address:X8}: expected version {version}.");
}

void CheckFlips(string mode, uint first, uint size)
{
    GpuHle.ResetSceneTracking();
    ConfigManager.View.LevelOfDetail = mode;
    uint second = first + size, limit = second + size;
    // Interleave insertion order across both arenas. Include both sides of every boundary.
    uint[] probes = [first, second, first + 4, second + 4, first + 8, second + 8,
        second - 1, limit - 1, first - 1, limit];
    var expected = probes.ToDictionary(a => a, a => a < second ? 0 : 1);
    foreach (uint address in probes) Register(address, expected[address]);
    void Verify() { foreach (uint address in probes) Check(address, expected[address]); }

    GpuHle.BeginPacketArena(second, second);
    GpuHle.BeginPacketArena(second + 4, second);
    Verify();
    for (int flip = 0; flip < 6; flip++)
    {
        uint slot = (uint)flip & 1u, start = first + slot * size;
        memory.WriteU32(cpu.GP + 0x20, slot);
        V82Compat.ActivateExpandedPrimitiveBuffer(cpu, memory); // actual retail buffer-flip post-hook
        foreach (uint address in probes)
            if (address >= start && address < start + size) expected[address] = -1;
        Verify();
        // Reuse freed entries in a different order; the other list is still queued for drawing.
        foreach (uint address in probes.Reverse())
        {
            if (address < start || address >= start + size) continue;
            expected[address] = flip % names.Length;
            Register(address, expected[address]);
        }
        Verify();
    }
    Console.WriteLine($"PASS {mode}: both alternating buffers, boundaries, aliases, values, reuse and invalid ranges.");
}

CheckFlips("Stock", 0x00074A68u, 0x20000u);
CheckFlips("Maximum", 0x00200000u, 0x80000u);

const int flips = 32;
long RetirementBytes(int count)
{
    GpuHle.ResetSceneTracking();
    const uint first = 0x200000, size = 0x80000;
    void Fill(uint start) { for (uint i = 0; i < count; i++) Register(start + 4u * i, 0); }
    Fill(first);
    Fill(first + size);
    long result = 0;
    for (int i = 0; i < flips; i++)
    {
        uint start = first + (uint)(i & 1) * size;
        long before = GC.GetAllocatedBytesForCurrentThread();
        GpuHle.BeginPacketArena(start, start + size);
        result += GC.GetAllocatedBytesForCurrentThread() - before;
        uint queued = first + (uint)((i & 1) ^ 1) * size;
        for (uint n = 0; n < count; n++)
        {
            Check(start + 4u * n, -1);
            Check(queued + 4u * n, 0);
        }
        Fill(start); // not part of the retirement allocation measurement
    }
    return result;
}

RetirementBytes(2); // JIT and lazy-initialized collections
long sparse = RetirementBytes(1), dense = RetirementBytes(2048);
Console.WriteLine($"Managed bytes for {flips} flips touching four maps: 1 key/map/flip = {sparse}; 2048 = {dense}.");
if (dense > sparse + flips * 64L)
    throw new Exception("Temporary retirement storage still scales with the number of recycled packets.");
