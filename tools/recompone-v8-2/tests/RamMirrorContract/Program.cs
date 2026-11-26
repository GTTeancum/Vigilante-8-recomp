// Standalone guest RAM contract. The oracle is a little-endian byte array,
// addressed independently by segment, physical bank and the selected RAM size.
using System.Security.Cryptography;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

const uint Bank = 0x200000;
uint[] segments = [0, 0x80000000, 0xA0000000, 0xE0000000];
int checks = 0, failedSpans = 0;
void Check(bool ok, string reason)
{
    if (!ok) throw new Exception(reason);
    checks++;
}

foreach (var mode in new[] { RunMode.Retail, RunMode.Devkit })
{
    Runtime.SetMode(mode);
    var mem = new PSMemory();
    var bytes = new byte[mode == RunMode.Devkit ? 4 * Bank : Bank];
    Check(mem.Ram.Length == bytes.Length, "instance size");
    uint Offset(uint a) => ((a & 0x1FFFFFFFu) % (uint)bytes.Length);
    void Store(uint a, uint v, int size)
    {
        switch (size)
        {
            case 1: mem.WriteU8(a, (byte)v); break;
            case 2: mem.WriteU16(a, (ushort)v); break;
            default: mem.WriteU32(a, v); break;
        }
    }
    uint Read(uint a, int size) => size switch
    {
        1 => mem.ReadU8(a), 2 => mem.ReadU16(a), _ => mem.ReadU32(a)
    };
    void ExpectShort(Action action)
    {
        try { action(); }
        catch (ArgumentOutOfRangeException) { failedSpans++; return; }
        throw new Exception("a scalar access improperly wrapped across the end of RAM");
    }
    void CheckRead(uint a, int width)
    {
        uint offset = Offset(a);
        if (offset + width > bytes.Length) { ExpectShort(() => Read(a, width)); return; }
        uint value = 0;
        for (int i = 0; i < width; i++) value |= (uint)bytes[offset + i] << (8 * i);
        Check(Read(a, width) == value, "array oracle read");
    }
    void Round(uint withinBank, uint bank, uint segment, int width, uint value)
    {
        uint address = segment | (bank * Bank + withinBank);
        uint offset = Offset(address);
        if (offset + width > bytes.Length) ExpectShort(() => Store(address, value, width));
        else
        {
            Store(address, value, width);
            for (int i = 0; i < width; i++) bytes[offset + i] = (byte)(value >> (8 * i));
        }
        for (uint b = 0; b < 4; b++)
            CheckRead(segments[(b + bank) % 4] | (b * Bank + withinBank), width);
    }
    var rng = new Random(0xC017);
    foreach (uint slot in new uint[] { 0, 1, 3, 0x3FE, 0x3FF, 0x400, 0xFFFF, Bank-4, Bank-3, Bank-2, Bank-1 })
        for (uint bank = 0; bank < 4; bank++)
            foreach (var segment in segments)
                foreach (int width in new[] { 1, 2, 4 })
                    Round(slot, bank, segment, width, (uint)rng.NextInt64(1L << 32));
    for (int i = 0; i < 4096; i++)
        Round((uint)rng.Next((int)Bank), (uint)rng.Next(4), segments[rng.Next(4)],
            1 << rng.Next(3), (uint)rng.NextInt64(1L << 32));
    Check(mem.Ram.SequenceEqual(bytes), "whole backing RAM including failed writes");
    string hash = Convert.ToHexString(SHA256.HashData(mem.Ram));

    // Keep the created instance alive through a global mode change. Its mirrors,
    // precise provenance, tracking, and delayed overlay notifications stay local.
    Runtime.SetMode(mode == RunMode.Retail ? RunMode.Devkit : RunMode.Retail);
    var vertices = new PreciseGteVertexData[4];
    const uint point = 0xFC00;
    for (uint b = 0; b < 4; b++)
    {
        vertices[b] = new(0x43210000u + b, 9, 12, -1, 2, -3, 4, 5, 6, 7, 8);
        mem.WritePreciseGteVertex(0xA0000000u + b * Bank + point, vertices[b]);
    }
    for (uint b = 0; b < 4; b++)
    {
        var expected = vertices[mode == RunMode.Retail ? 3u : b];
        uint a = 0x80000000u + b * Bank + point;
        Check(mem.ReadU32(a) == expected.PackedScreenPosition, "instance-local RAM mirrors");
        Check(mem.TryGetPreciseGteVertex(a, expected.PackedScreenPosition, out var actual)
            && actual == expected, "instance-local provenance mirrors");
    }
    mem.WriteU16(Bank + point - 1, 0);
    for (uint b = 0; b < 4; b++)
    {
        var expected = vertices[mode == RunMode.Retail ? 3u : b];
        Check(mem.TryGetPreciseGteVertex(b * Bank + point, expected.PackedScreenPosition, out _)
            == (mode == RunMode.Devkit && b != 1), "straddled word invalidation");
    }

    var overlay = new MirrorOverlay($"mirror_{mode}", (int)mode + 123);
    Dispatcher.Register(overlay.Name, overlay);
    Dispatcher.LoadByLba(overlay.LbaStart);
    mem.WriteU8(0xA0204010, 7);
    Check(Dispatcher.ActiveNames.Contains(overlay.Name) == (mode == RunMode.Retail), "mirrored overlay notification");
    mem.WriteU8(0x80004010, 9);
    Check(Dispatcher.ActiveNames.Contains(overlay.Name), "base overlay notification");

    // The 2 MiB debugger map records each byte, even when a span then fails.
    var log = Runtime.RamLog;
    RamLogger.TrackReads = RamLogger.TrackWrites = true;
    log.Tick();
    uint end = (uint)bytes.Length - 1;
    ExpectShort(() => mem.WriteU32(end | 0xA0000000, 0x1234));
    ExpectShort(() => mem.ReadU16(end | 0x80000000));
    foreach (int index in new[] { (int)Bank-1, 0, 1, 2 })
        Check(log.GetWriteStamp(index) == log.Cycle, "write stamp");
    foreach (int index in new[] { (int)Bank-1, 0 })
        Check(log.GetReadStamp(index) == log.Cycle, "read stamp");
    Check(log.GetReadStamp(1) != log.Cycle && log.GetWriteStamp(3) != log.Cycle, "tracking extent");
    RamLogger.TrackReads = RamLogger.TrackWrites = false;

    // Addresses outside RAM do not get folded into a bank.
    foreach (uint address in new[] { MemoryMap.ScratchpadBase, MemoryMap.HwRegsBase + 0x400u, MemoryMap.BiosBase })
    {
        mem.WriteU32(address + 16, 0x56341200u + address);
        Check(mem.ReadU32(address + 16 | 0xA0000000u) == 0x56341200u + address, "separate mapping");
    }
    Console.WriteLine($"{mode} RAM sha256={hash}");
}
Console.WriteLine($"RAM mirror contract PASS checks={checks} short-spans={failedSpans}");

sealed class MirrorOverlay(string name, int lba) : IOverlay
{
    public string Name => name;
    public int LbaStart => lba;
    public uint Base => 0x80004000;
    public uint Size => 0x800;
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions { get; } =
        new Dictionary<uint, Action<CpuContext, IMemory>>();
}
