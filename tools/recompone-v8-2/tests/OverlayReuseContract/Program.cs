using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

var memory = new PSMemory();
var cpu = new CpuContext();
const uint actual = 0x80180000;
var sand = new TestOverlay("reuse_sand", 0x1104, 0x1224, 1);
var oil = new TestOverlay("reuse_oil", 0x1168, 0x12B8, 2);
Dispatcher.Register(sand.Name, sand);
Dispatcher.Register(oil.Name, oil);
memory.WriteU32(actual, sand.ImageSize);
Dispatcher.Call(cpu, memory, actual + 0x100);
if (cpu.V0 != 1) throw new Exception("initial overlay dispatch");
// The native loader reuses an allocation, including the same entry offset.
// No explicit dispatcher load notification occurs for this DLL allocation.
memory.WriteU32(actual, oil.ImageSize);
Dispatcher.Call(cpu, memory, actual + 0x100);
if (cpu.V0 != 2) throw new Exception("reused allocation invoked retired overlay");
Dispatcher.Call(cpu, memory, actual + 0x100);
if (cpu.V0 != 2) throw new Exception("replacement dispatch was not retained");
Console.WriteLine("Overlay reuse: 3 checks passed");

sealed class TestOverlay(string name, uint imageSize, uint size, uint result) : IOverlay
{
    public string Name => name;
    public uint Base => 0x80100000;
    public uint Size => size;
    public uint ImageSize => imageSize;
    public IReadOnlyDictionary<uint, Action<CpuContext, IMemory>> Functions { get; } =
        new Dictionary<uint, Action<CpuContext, IMemory>>
        { [0x80100100] = (c, m) => c.V0 = result };
}
