using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Dispatch;

internal sealed class RelocatedMemory : IMemory
{
    static readonly bool TraceInvalidAddresses =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_RELOCATED_MEMORY") == "1";

    internal IMemory Inner { get; }
    internal uint LinkedBase { get; }
    internal uint Size { get; }
    internal uint Delta { get; }

    internal RelocatedMemory(IMemory inner, uint linkedBase, uint size, uint delta)
    {
        Inner = inner is RelocatedMemory relocated ? relocated.Inner : inner;
        LinkedBase = linkedBase;
        Size = size;
        Delta = delta;
    }

    internal bool Matches(uint linkedBase, uint size, uint delta) =>
        LinkedBase == linkedBase && Size == size && Delta == delta;

    uint Address(uint address)
    {
        uint mapped =
            address >= LinkedBase && address - LinkedBase < Size
                ? address + Delta
                : address;
        bool inputInvalidRam =
            address >= MemoryMap.Kseg0Base &&
            (address & MemoryMap.PhysicalMask) >= MemoryMap.RamWindow;
        bool mappedInvalidRam =
            mapped >= MemoryMap.Kseg0Base &&
            (mapped & MemoryMap.PhysicalMask) >= MemoryMap.RamWindow;
        if (TraceInvalidAddresses && (inputInvalidRam || mappedInvalidRam))
        {
            Console.Error.WriteLine(
                "[RelocatedMemory] out-of-RAM access " +
                $"input=0x{address:X8} mapped=0x{mapped:X8} " +
                $"linked=0x{LinkedBase:X8}+0x{Size:X} " +
                $"delta=0x{Delta:X8}");
        }
        return mapped;
    }

    public byte ReadU8(uint address) => Inner.ReadU8(Address(address));
    public ushort ReadU16(uint address) => Inner.ReadU16(Address(address));
    public uint ReadU32(uint address) => Inner.ReadU32(Address(address));
    public void WriteU8(uint address, byte value) => Inner.WriteU8(Address(address), value);
    public void WriteU16(uint address, ushort value) => Inner.WriteU16(Address(address), value);
    public void WriteU32(uint address, uint value) => Inner.WriteU32(Address(address), value);
    public void WriteGteWord(uint address, int register) =>
        Inner.WriteGteWord(Address(address), register);
    public void WritePreciseGteVertex(
        uint address, in PreciseGteVertexData vertex) =>
        Inner.WritePreciseGteVertex(Address(address), vertex);
    public bool TryGetPreciseGteVertex(
        uint address, uint packedScreenPosition,
        out PreciseGteVertexData vertex) =>
        Inner.TryGetPreciseGteVertex(
            Address(address), packedScreenPosition,
            out vertex);
    public uint ReadWordLeft(uint current, uint address) => Inner.ReadWordLeft(current, Address(address));
    public uint ReadWordRight(uint current, uint address) => Inner.ReadWordRight(current, Address(address));
    public void WriteWordLeft(uint address, uint value) => Inner.WriteWordLeft(Address(address), value);
    public void WriteWordRight(uint address, uint value) => Inner.WriteWordRight(Address(address), value);
    public void LoadBytes(uint address, byte[] data) => Inner.LoadBytes(Address(address), data);
    public void ZeroRange(uint address, uint length) => Inner.ZeroRange(Address(address), length);
    public void SetCd(CdController cd) => Inner.SetCd(cd);
}
