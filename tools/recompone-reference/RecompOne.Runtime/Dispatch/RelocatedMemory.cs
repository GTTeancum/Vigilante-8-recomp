using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Dispatch;

internal sealed class RelocatedMemory : IMemory
{
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

    uint Address(uint address) =>
        address >= LinkedBase && address - LinkedBase < Size ? address + Delta : address;

    public byte ReadU8(uint address) => Inner.ReadU8(Address(address));
    public ushort ReadU16(uint address) => Inner.ReadU16(Address(address));
    public uint ReadU32(uint address) => Inner.ReadU32(Address(address));
    public void WriteU8(uint address, byte value) => Inner.WriteU8(Address(address), value);
    public void WriteU16(uint address, ushort value) => Inner.WriteU16(Address(address), value);
    public void WriteU32(uint address, uint value) => Inner.WriteU32(Address(address), value);
    public uint ReadWordLeft(uint current, uint address) => Inner.ReadWordLeft(current, Address(address));
    public uint ReadWordRight(uint current, uint address) => Inner.ReadWordRight(current, Address(address));
    public void WriteWordLeft(uint address, uint value) => Inner.WriteWordLeft(Address(address), value);
    public void WriteWordRight(uint address, uint value) => Inner.WriteWordRight(Address(address), value);
    public void LoadBytes(uint address, byte[] data) => Inner.LoadBytes(Address(address), data);
    public void ZeroRange(uint address, uint length) => Inner.ZeroRange(Address(address), length);
    public void SetCd(CdController cd) => Inner.SetCd(cd);
}
