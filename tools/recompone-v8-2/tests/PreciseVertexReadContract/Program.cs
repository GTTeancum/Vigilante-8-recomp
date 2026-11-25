// Exercise the same public guest memory and register-copy paths as the game.
// Expected answers below do not call the production geometry validator.
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

const uint bank = 0x200000u;
uint[] segments = [0, 0x80000000u, 0xA0000000u, 0xE0000000u];
int hits = 0, misses = 0;
ulong digest = 14695981039346656037UL;

static PreciseGteVertexData Shape(uint key) =>
    new(key, 0x9A7B, -0f, -12.75f, 3.25f, -18f, 5.5f, -6.5f, 0f, 7f, 0.125f);
static PreciseGteVertexData WithField(PreciseGteVertexData v, int i, float value) => i switch
{
    0 => v with { PerspectiveW = value },
    1 => v with { ViewX = value },
    2 => v with { ViewY = value },
    3 => v with { ViewZ = value },
    4 => v with { PreciseX = value },
    5 => v with { PreciseY = value },
    6 => v with { ProjectionCenterX = value },
    7 => v with { ProjectionCenterY = value },
    8 => v with { ProjectionScale = value },
    _ => throw new ArgumentOutOfRangeException(nameof(i))
};
static uint[] Fields(PreciseGteVertexData v) =>
    [v.PackedScreenPosition, v.Depth,
     BitConverter.SingleToUInt32Bits(v.PerspectiveW), BitConverter.SingleToUInt32Bits(v.ViewX),
     BitConverter.SingleToUInt32Bits(v.ViewY), BitConverter.SingleToUInt32Bits(v.ViewZ),
     BitConverter.SingleToUInt32Bits(v.PreciseX), BitConverter.SingleToUInt32Bits(v.PreciseY),
     BitConverter.SingleToUInt32Bits(v.ProjectionCenterX), BitConverter.SingleToUInt32Bits(v.ProjectionCenterY),
     BitConverter.SingleToUInt32Bits(v.ProjectionScale)];
void Expect(PSMemory m, uint address, uint key, PreciseGteVertexData? expected)
{
    var actual = Shape(0xDEADBEEF);
    bool hit = m.TryGetPreciseGteVertex(address, key, out actual);
    uint[] bits = Fields(actual);
    if (hit != expected.HasValue || !bits.SequenceEqual(Fields(expected.GetValueOrDefault())))
        throw new Exception($"Wrong vertex/result at {address:X8} for {key:X8}, expected hit={expected.HasValue}");
    if (hit) hits++; else misses++;
    foreach (uint value in bits) digest = unchecked((digest ^ value) * 1099511628211UL);
    digest = unchecked((digest ^ (hit ? 1u : 0u)) * 1099511628211UL);
}
void Wire(PSMemory m, uint address, uint expected)
{
    if (m.ReadU32(address) != expected) throw new Exception($"Wrong guest word at {address:X8}");
}

foreach (var mode in new[] { RunMode.Retail, RunMode.Devkit })
{
    Runtime.SetMode(mode);
    var m = new PSMemory();
    foreach (uint start in new[] { 0x53F8u, MemoryMap.ScratchpadBase + 0x50u })
    {
        var good = Shape(0);
        var other = Shape(0xBABB1Eu) with { ViewZ = 0f, PerspectiveW = float.MinValue,
            ProjectionCenterY = -0f, ProjectionScale = float.Epsilon };
        Expect(m, start, 0, null);
        m.WritePreciseGteVertex(start, good);
        foreach (uint segment in segments)
        {
            for (uint offset = 0; offset < 4; offset++) Expect(m, segment | (start + offset), 0, good);
            Expect(m, segment | start, 0xFFFFFFFFu, null);
        }
        m.WritePreciseGteVertex(start + 4, other);
        Expect(m, start + 4, other.PackedScreenPosition, other);
        foreach (float bad in new[] { float.NaN, float.NegativeInfinity, float.PositiveInfinity })
        for (int i = 0; i < 9; i++)
        {
            // Same packed key: a rejected replacement must not leave old provenance alive.
            m.WritePreciseGteVertex(start, good);
            m.WritePreciseGteVertex(start, WithField(good, i, bad));
            Wire(m, start, good.PackedScreenPosition);
            Expect(m, start, good.PackedScreenPosition, null);
            Expect(m, start + 4, other.PackedScreenPosition, other);
        }
        foreach (float badScale in new[] { 0f, -0f, -float.Epsilon, -1.5f })
        {
            m.WritePreciseGteVertex(start, good);
            m.WritePreciseGteVertex(start, good with { ProjectionScale = badScale });
            Expect(m, start, good.PackedScreenPosition, null);
        }
        m.WritePreciseGteVertex(start, good);
        m.WriteU32(start, good.PackedScreenPosition); // same ordinary bits still invalidate
        Expect(m, start, good.PackedScreenPosition, null);
        Expect(m, start + 4, other.PackedScreenPosition, other);
        m.WritePreciseGteVertex(start, good);
        m.LoadBytes(start + 1, [0]);
        Expect(m, start, good.PackedScreenPosition, null);
        m.ZeroRange(start + 5, 1);
        Expect(m, start + 4, other.PackedScreenPosition, null);
    }

    // Cross-word and cross-1-KiB-page stores, including unaligned precise
    // stores: only the starting slot receives new metadata.
    const uint pageEdge = 0x87FC;
    foreach (int width in new[] { 1, 2, 4 })
    for (uint off = 0; off < 4; off++)
    {
        for (uint i = 0; i < 3; i++) m.WritePreciseGteVertex(pageEdge + 4*i, Shape(0x61000000u + i));
        uint a = 0xA0000000u | (pageEdge + off);
        if (width == 1) m.WriteU8(a, 0); else if (width == 2) m.WriteU16(a, 0); else m.WriteU32(a, 0);
        Expect(m, pageEdge, 0x61000000, null);
        Expect(m, pageEdge + 4, 0x61000001, off + (uint)width > 4 ? null : Shape(0x61000001));
        Expect(m, pageEdge + 8, 0x61000002, Shape(0x61000002));
        var v = Shape(0x00112233u + off);
        m.WritePreciseGteVertex(a, v); Wire(m, a, v.PackedScreenPosition);
        Expect(m, pageEdge, v.PackedScreenPosition, v);
        if (off != 0) Expect(m, pageEdge + 4, 0x61000001, null);
    }

    const uint point = 0x2490;
    for (uint b = 0; b < 4; b++)
    {
        var record = Shape(0xDAD00000u + b) with { ViewX = b };
        m.WritePreciseGteVertex(0xA0000000u + b*bank + point, record);
    }
    Runtime.SetMode(mode == RunMode.Retail ? RunMode.Devkit : RunMode.Retail);
    var stranger = new PSMemory();
    foreach (uint segment in segments)
    for (uint b = 0; b < 4; b++)
    {
        uint winner = mode == RunMode.Retail ? 3u : b;
        uint a = segment | (b*bank + point), key = 0xDAD00000u + winner;
        Expect(m, a, key, Shape(key) with { ViewX = winner });
        Expect(stranger, a, key, null);
    }
    var carried = Shape(0x98761234u) with { PreciseX = -0f, PerspectiveW = -1f };
    var cpu = new CpuContext();
    m.WritePreciseGteVertex(point, carried);
    cpu.LoadWord(4, m, point); cpu.CopyRegister(5, 4);
    cpu.StoreWord(5, m, point + 4);
    Expect(m, point + 4, carried.PackedScreenPosition, carried);
    cpu.A1 = carried.PackedScreenPosition; // scalar re-assignment, same packed bits
    cpu.StoreWord(5, m, point + 4);
    Expect(m, point + 4, carried.PackedScreenPosition, null);

    foreach (uint a in new[] { MemoryMap.BiosBase + 0x40u, MemoryMap.HwRegsBase + 0x400u })
    {
        m.WritePreciseGteVertex(a, carried);
        Wire(m, a, carried.PackedScreenPosition);
        Expect(m, a, carried.PackedScreenPosition, null);
    }
    foreach (uint a in new[] { MemoryMap.RamWindow, MemoryMap.ScratchpadBase + MemoryMap.ScratchpadSize,
                                MemoryMap.Expansion1Base, 0xFFFFFFFFu })
        Expect(m, a, carried.PackedScreenPosition, null);
}
Console.WriteLine($"Precise vertex read contract PASS hits={hits} misses={misses} digest={digest:X16}");
