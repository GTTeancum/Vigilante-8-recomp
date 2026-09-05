using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Memory;

public sealed class PSMemory : IMemory
{
    private static readonly uint? _watchedWriteAddress = ParseWatchedWriteAddress();
    private static readonly bool _watchDmaLinksOnly =
        Environment.GetEnvironmentVariable("RECOMPONE_WATCH_WRITE_DMA_LINK") == "1";
    private static int _watchedWriteCount;

    private readonly byte[] _ram = new byte[Runtime.Mode == RunMode.Devkit ? MemoryMap.DevkitRamSize : MemoryMap.RetailRamSize];
    private readonly byte[] _scratchpad = new byte[MemoryMap.ScratchpadSize];
    private readonly byte[] _hwregs = new byte[MemoryMap.HwRegsSize];
    private readonly byte[] _bios = new byte[MemoryMap.BiosSize];
    private const int PrecisePageShift = 10;
    private const int PreciseWordsPerPage = 1 << (PrecisePageShift - 2);

    private sealed class PreciseGtePage
    {
        public readonly PreciseGteVertexData[] Vertices =
            new PreciseGteVertexData[PreciseWordsPerPage];
        public readonly bool[] Valid = new bool[PreciseWordsPerPage];
    }

    // Exact enhanced-renderer vertex provenance is sparse, but it is queried
    // and invalidated in the hottest memory paths. A direct page table keeps
    // inactive RAM almost free while avoiding a Dictionary hash/remove for
    // every emulated write and every submitted GPU vertex.
    private readonly PreciseGtePage?[] _preciseGteRamPages;
    private readonly PreciseGtePage?[] _preciseGteScratchpadPages;

    private readonly Gpu _gpu = new();
    private readonly Spu _spu = new();
    private readonly Mdec _mdec = new();
    private readonly Timers _timers = new();
    private readonly Dma _dma;
    private CdController? _cd;

    public ReadOnlySpan<byte> Ram => _ram;
    internal byte[] RamBuffer => _ram;

    public PSMemory()
    {
        _preciseGteRamPages =
            new PreciseGtePage?[_ram.Length >> PrecisePageShift];
        _preciseGteScratchpadPages =
            new PreciseGtePage?[MemoryMap.ScratchpadSize >> PrecisePageShift];
        _dma = new Dma(this, _gpu, _spu, _mdec, () => Runtime.DispatchIrq(3));
        Runtime.Gpu = _gpu;
        Runtime.Spu = _spu;
        Bios.KromFont.InstallInto(_bios);
    }

    private static uint? ParseWatchedWriteAddress()
    {
        string? text = Environment.GetEnvironmentVariable("RECOMPONE_WATCH_WRITE");
        if (string.IsNullOrWhiteSpace(text))
            return null;

        if (text.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            text = text[2..];

        return uint.TryParse(
            text,
            System.Globalization.NumberStyles.HexNumber,
            System.Globalization.CultureInfo.InvariantCulture,
            out uint address)
            ? MemoryMap.ToPhysical(address)
            : null;
    }

    private static void TraceWatchedWrite(uint phys, uint value, int size)
    {
        if (_watchedWriteAddress is not uint watched ||
            watched < phys || watched >= phys + (uint)size)
            return;
        if (_watchDmaLinksOnly && size == 4 &&
            ((value & 0xFF000000u) != 0u || (value & 0x00F00000u) != 0x00700000u))
            return;

        int count = System.Threading.Interlocked.Increment(ref _watchedWriteCount);
        if (count <= 64)
            Console.Error.WriteLine(
                $"[MemoryWatch] #{count} write{size * 8} " +
                $"phys=0x{phys:X8} watched=0x{watched:X8} " +
                $"value=0x{value:X8}{Environment.NewLine}{Environment.StackTrace}");
    }

    public void SetCd(CdController cd) { _cd = cd; _dma.SetCd(cd); }

    private static bool IsDmaChcr(uint phys) => phys >= 0x1F801080u && phys < 0x1F8010F0u && (phys & 0xFu) == 8u;

    private uint Hw32(uint phys)
    {
        int o = (int)(phys - MemoryMap.HwRegsBase);
        return (uint)(_hwregs[o] | (_hwregs[o + 1] << 8) | (_hwregs[o + 2] << 16) | (_hwregs[o + 3] << 24));
    }

    private void Hw32(uint phys, uint v)
    {
        int o = (int)(phys - MemoryMap.HwRegsBase);
        _hwregs[o] = (byte)v;
        _hwregs[o + 1] = (byte)(v >> 8);
        _hwregs[o + 2] = (byte)(v >> 16);
        _hwregs[o + 3] = (byte)(v >> 24);
    }

    private void TrackWrite(uint phys, int size)
    {
        if (phys < MemoryMap.RamWindow)
        {
            uint off = phys % (uint)_ram.Length;
            if (RamLogger.TrackWrites)
                Runtime.RamLog.RecordWrite(off, size);
            Dispatcher.NotifyWrite(off);
        }

    }

    private bool TryPrecisePageCoordinates(
        uint phys, out PreciseGtePage?[] pages,
        out int pageIndex, out int wordIndex)
    {
        if (phys < MemoryMap.RamWindow)
        {
            uint offset = phys % (uint)_ram.Length;
            pages = _preciseGteRamPages;
            pageIndex = (int)(offset >> PrecisePageShift);
            wordIndex = (int)((offset >> 2) &
                (PreciseWordsPerPage - 1));
            return true;
        }
        if (phys >= MemoryMap.ScratchpadBase &&
            phys < MemoryMap.ScratchpadBase +
                MemoryMap.ScratchpadSize)
        {
            uint offset = phys - MemoryMap.ScratchpadBase;
            pages = _preciseGteScratchpadPages;
            pageIndex = (int)(offset >> PrecisePageShift);
            wordIndex = (int)((offset >> 2) &
                (PreciseWordsPerPage - 1));
            return true;
        }
        pages = _preciseGteRamPages;
        pageIndex = 0;
        wordIndex = 0;
        return false;
    }

    private void InvalidatePreciseGteWord(uint phys)
    {
        if (!TryPrecisePageCoordinates(
                phys, out PreciseGtePage?[] pages,
                out int pageIndex, out int wordIndex))
            return;
        PreciseGtePage? page = pages[pageIndex];
        if (page != null)
            page.Valid[wordIndex] = false;
    }

    private void InvalidatePreciseGteVertex(uint phys, int size)
    {
        uint lastPhys = phys + (uint)Math.Max(size - 1, 0);
        InvalidatePreciseGteWord(phys);
        if ((phys & ~3u) != (lastPhys & ~3u))
            InvalidatePreciseGteWord(lastPhys);
    }

    private void TrackRead(uint phys, int size)
    {
        if (RamLogger.TrackReads && phys < MemoryMap.RamWindow)
            Runtime.RamLog.RecordRead(phys % (uint)_ram.Length, size);
    }

    // Reads of an unmapped address terminate the process today. Hardware does
    // no such thing: it returns whatever the bus yields and the game carries
    // on. Quest mode reaches a call that passes a corrupted string pointer for
    // two of the eighteen character types, and the resulting fatal read is
    // what the player sees as the game freezing on that quest. Returning zero
    // ends the string harmlessly and keeps the game running, while a bad
    // *write* still throws, because that corrupts state and must be caught.
    static readonly bool StrictUnmappedReads =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_STRICT_UNMAPPED_READS") == "1";
    static readonly bool TraceUnmappedStack =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_UNMAPPED_STACK") == "1";
    static readonly byte[] _unmappedRead = new byte[8];
    readonly HashSet<uint> _reportedUnmapped = [];

    private Span<byte> Resolve(uint address, int size) =>
        Resolve(address, size, forWrite: true);

    private Span<byte> Resolve(uint address, int size, bool forWrite)
    {
        uint phys = MemoryMap.ToPhysical(address);

        if (phys < MemoryMap.RamWindow)
            return _ram.AsSpan((int)(phys % (uint)_ram.Length), size);

        if (phys >= MemoryMap.ScratchpadBase && phys < MemoryMap.ScratchpadBase + MemoryMap.ScratchpadSize)
            return _scratchpad.AsSpan((int)(phys - MemoryMap.ScratchpadBase), size);

        if (phys >= MemoryMap.HwRegsBase && phys < MemoryMap.HwRegsBase + MemoryMap.HwRegsSize)
            return _hwregs.AsSpan((int)(phys - MemoryMap.HwRegsBase), size);

        if (phys >= MemoryMap.BiosBase && phys < MemoryMap.BiosBase + MemoryMap.BiosSize)
            return _bios.AsSpan((int)(phys - MemoryMap.BiosBase), size);

        V8Compat.TraceUnmappedMemoryAddress(address, size);
        if (!forWrite && !StrictUnmappedReads)
        {
            if (_reportedUnmapped.Add(address))
            {
                Console.Error.WriteLine(
                    $"[Memory] unmapped read at 0x{address:X8} " +
                    $"size={size}; returning zero");
                if (TraceUnmappedStack)
                    Console.Error.WriteLine(Environment.StackTrace);
            }
            Array.Clear(_unmappedRead);
            return _unmappedRead.AsSpan(0, size);
        }
        throw new InvalidOperationException($"unmapped address: 0x{address:X8}");
    }

    private static bool IsCd(uint phys) => phys >= 0x1F801800u && phys <= 0x1F801803u;
    private static bool IsSpu(uint phys) => phys >= 0x1F801C00u && phys < 0x1F801E80u;

    public byte ReadU8(uint address)
    {
        uint phys = MemoryMap.ToPhysical(address);
        TrackRead(phys, 1);
        if (_cd != null && IsCd(phys)) return _cd.Read(phys);
        return Resolve(address, 1, forWrite: false)[0];
    }

    public ushort ReadU16(uint address)
    {
        uint phys = MemoryMap.ToPhysical(address);
        TrackRead(phys, 2);
        if (_cd != null && IsCd(phys)) return _cd.Read(phys);
        if (IsSpu(phys)) return _spu.ReadReg16(phys);
        if (Timers.InRange(phys) && _timers.TryRead(phys, out uint tv)) return (ushort)tv;
        var s = Resolve(address, 2, forWrite: false);
        return (ushort)(s[0] | (s[1] << 8));
    }

    public uint ReadU32(uint address)
    {
        uint phys = MemoryMap.ToPhysical(address);
        TrackRead(phys, 4);
        if (phys == 0x1F801810u) return _gpu.ReadData();
        if (phys == 0x1F801814u) return _gpu.ReadStat();
        if (phys == 0x1F801820u) return _mdec.ReadData();
        if (phys == 0x1F801824u) return _mdec.ReadStatus();
        if (phys == 0x1F8010F4u) return _dma.ReadDicr();
        if (_cd != null && IsCd(phys)) return _cd.Read(phys);
        if (IsSpu(phys)) return (uint)(_spu.ReadReg16(phys) | (_spu.ReadReg16(phys + 2) << 16));
        if (Timers.InRange(phys) && _timers.TryRead(phys, out uint tv)) return tv;
        var s = Resolve(address, 4, forWrite: false);
        return (uint)(s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24));
    }

    public void WriteU8(uint address, byte value)
    {
        uint phys = MemoryMap.ToPhysical(address);
        InvalidatePreciseGteVertex(phys, 1);
        if (_watchedWriteAddress.HasValue)
            TraceWatchedWrite(phys, value, 1);
        TrackWrite(phys, 1);
        if (_cd != null && IsCd(phys)) { _cd.Write(phys, value); return; }
        Resolve(address, 1)[0] = value;
    }

    public void WriteU16(uint address, ushort value)
    {
        uint phys = MemoryMap.ToPhysical(address);
        InvalidatePreciseGteVertex(phys, 2);
        if (_watchedWriteAddress.HasValue)
            TraceWatchedWrite(phys, value, 2);
        TrackWrite(phys, 2);
        if (_cd != null && IsCd(phys)) { _cd.Write(phys, (byte)value); return; }
        if (IsSpu(phys)) { _spu.WriteReg16(phys, value); return; }
        if (_timers.TryWrite(phys, value)) return;
        var s = Resolve(address, 2);
        s[0] = (byte)value;
        s[1] = (byte)(value >> 8);
    }

    public void WriteU32(uint address, uint value)
    {
        uint phys = MemoryMap.ToPhysical(address);
        InvalidatePreciseGteVertex(phys, 4);
        if (_watchedWriteAddress.HasValue)
            TraceWatchedWrite(phys, value, 4);
        RecompOne.Runtime.Hle.GpuHle.ObservePacketWrite(phys);
        TrackWrite(phys, 4);
        if (phys == 0x1F801810u) { _gpu.WriteGp0(value); return; }
        if (phys == 0x1F801814u) { _gpu.WriteGp1(value); return; }
        if (phys == 0x1F801820u) { _mdec.Write0(value); return; }
        if (phys == 0x1F801824u) { _mdec.WriteControl(value); return; }
        if (phys == 0x1F8010F4u) { _dma.WriteDicr(value); return; }
        if (IsDmaChcr(phys) && (value & 0x01000000u) != 0)
        {
            Hw32(phys, value & ~0x01000000u);
            _dma.Run((int)((phys - 0x1F801080u) / 0x10u), Hw32(phys - 8u), Hw32(phys - 4u), value);
            return;
        }
        if (_cd != null && IsCd(phys)) { _cd.Write(phys, (byte)value); return; }
        if (IsSpu(phys)) { _spu.WriteReg16(phys, (ushort)value); _spu.WriteReg16(phys + 2, (ushort)(value >> 16)); return; }
        if (_timers.TryWrite(phys, value)) return;
        var s = Resolve(address, 4);
        s[0] = (byte)value;
        s[1] = (byte)(value >> 8);
        s[2] = (byte)(value >> 16);
        s[3] = (byte)(value >> 24);
    }

    public void WriteGteWord(uint address, int register)
    {
        bool precise = Gte.TryGetStoreVertex(register, out var vertex);
        if (precise)
            WritePreciseGteVertex(address, vertex);
        else
            WriteU32(address, Gte.Read(register));
    }

    public void WritePreciseGteVertex(
        uint address, in PreciseGteVertexData vertex)
    {
        WriteU32(address, vertex.PackedScreenPosition);
        uint phys = MemoryMap.ToPhysical(address);
        if (!vertex.Valid ||
            !TryPrecisePageCoordinates(
                phys, out PreciseGtePage?[] pages,
                out int pageIndex, out int wordIndex))
            return;
        PreciseGtePage page = pages[pageIndex] ??= new PreciseGtePage();
        page.Vertices[wordIndex] = vertex;
        page.Valid[wordIndex] = true;
    }

    public bool TryGetPreciseGteVertex(
        uint address, uint packedScreenPosition,
        out PreciseGteVertexData vertex)
    {
        uint phys = MemoryMap.ToPhysical(address);
        if (TryPrecisePageCoordinates(
                phys, out PreciseGtePage?[] pages,
                out int pageIndex, out int wordIndex) &&
            pages[pageIndex] is PreciseGtePage page &&
            page.Valid[wordIndex] &&
            (vertex = page.Vertices[wordIndex]).Valid &&
            vertex.PackedScreenPosition == packedScreenPosition &&
            vertex.Valid)
            return true;
        vertex = default;
        return false;
    }

    public uint ReadWordLeft(uint current, uint address)
    {
        int shift = (int)((address & 3) * 8);
        uint word = ReadU32(address & ~3u);
        return (current & (0x00FFFFFFu >> shift)) | (word << (24 - shift));
    }

    public uint ReadWordRight(uint current, uint address)
    {
        int shift = (int)((address & 3) * 8);
        uint word = ReadU32(address & ~3u);
        return (current & (0xFFFFFF00u << (24 - shift))) | (word >> shift);
    }

    public void WriteWordLeft(uint address, uint value)
    {
        uint aligned = address & ~3u;
        int shift = (int)((address & 3) * 8);
        uint mem = ReadU32(aligned);
        WriteU32(aligned, (mem & (0xFFFFFF00u << shift)) | (value >> (24 - shift)));
    }

    public void WriteWordRight(uint address, uint value)
    {
        uint aligned = address & ~3u;
        int shift = (int)((address & 3) * 8);
        uint mem = ReadU32(aligned);
        WriteU32(aligned, (mem & (0x00FFFFFFu >> (24 - shift))) | (value << shift));
    }

    public void LoadBytes(uint address, byte[] data)
    {
        for (int i = 0; i < data.Length; i++)
            WriteU8(address + (uint)i, data[i]);
    }

    public void ZeroRange(uint address, uint length)
    {
        for (uint i = 0; i < length; i++)
            WriteU8(address + i, 0);
    }
}
