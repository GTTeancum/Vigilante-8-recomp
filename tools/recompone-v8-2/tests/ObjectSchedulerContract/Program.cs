using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

// Runs the real scheduler, dispatcher and PS memory with synthetic list nodes.
// A zero-tick pass exercises object callbacks without starting presentation.
const uint gp = 0x80006000, sp = 0x801F0000, head = 0x80003000;
const uint objects = 0x80030000, callback = 0x80058000;
static uint Node(int id) => 0x80010000 + (uint)id * 16;
static uint Object(int id) => objects + (uint)id * 32;
var mem = new PSMemory();
mem.WriteU32(gp + 0x1088, head);
mem.WriteU32(gp + 0x610, 0x80074A68); // ordinary primitive cursor
var trace = new List<int>();
var mode = Mode.Record;
bool nested = false;
uint digest = 0;
int calls = 0;
static CpuContext NewCpu() => new() { GP = gp, SP = sp, S0 = 42, RA = 17 };
static void Check(bool test, string message)
{
    if (!test) throw new Exception(message);
}
void Build(int count)
{
    mem.WriteU32(head, count == 0 ? 0 : Node(1));
    for (int i = 1; i <= count; ++i)
    {
        mem.WriteU32(Node(i), i == count ? 0 : Node(i + 1));
        mem.WriteU32(Node(i) + 8, Object(i));
        mem.WriteU32(Object(i), callback);
        mem.WriteU32(Object(i) + 12, 0);
    }
}
void Run(CpuContext cpu)
{
    cpu.A0 = 0;
    V82Compat.RunObjectScheduler(cpu, mem);
}
Dispatcher.RegisterHostFunction(callback, (c, m) =>
{
    Check(c.A1 == 0 && c.A2 == 0 && c.RA == 0x80031418, "callback ABI");
    Check(c.S0 == 42 && c.GP == gp && (c.SP == sp - 16 || c.SP == sp - 32), "caller registers");
    int id = (int)((c.A0 - objects) / 32);
    ++calls;
    uint count = m.ReadU32(Object(id) + 12) + 1;
    m.WriteU32(Object(id) + 12, count);
    digest = unchecked((digest * 16777619) ^ ((uint)id + count));
    if (mode != Mode.Measure) trace.Add(nested ? -id : id);
    if (mode == Mode.Relink && id == 1) m.WriteU32(Node(1), Node(3));
    if (mode == Mode.Reenter && !nested && id == 2)
    {
        nested = true;
        Run(c); // inner pass walks the same cyclic list as the outer pass
        nested = false;
    }
    if (mode == Mode.Abort && id == 2) throw new FixtureAbort();
    c.S0 = 99; c.SP = 0; c.GP = 0; // scheduler restores these after each call
});
var cpu = NewCpu();
void Expect(params int[] ids)
{
    trace.Clear();
    Run(cpu);
    Check(trace.SequenceEqual(ids), $"callbacks [{string.Join(',', trace)}] != [{string.Join(',', ids)}]");
    Check(cpu.S0 == 42 && cpu.SP == sp && cpu.GP == gp && cpu.RA == 17, "pass clobbered caller state");
}
Build(0); Expect();
Build(3); Expect(1, 2, 3);
mem.WriteU32(Node(3), Node(2)); Expect(1, 2, 3); Expect(1, 2, 3);
Build(3); mode = Mode.Relink; Expect(1, 3); // read next *after* callback
Build(3); mem.WriteU32(Node(3), Node(1));
mode = Mode.Reenter; Expect(1, 2, -1, -2, -3, 3);
Check(Enumerable.Range(1, 3).All(i => mem.ReadU32(Object(i) + 12) == 2), "nested guest writes");
mode = Mode.Abort;
try { Run(cpu); throw new Exception("callback exception lost"); } catch (FixtureAbort) { }
cpu = NewCpu(); mode = Mode.Record; Expect(1, 2, 3);

// Invalid lists still hit the original hard limit; no stale node survives a failed pass.
const uint longList = 0x80080000;
for (uint i = 0; i <= 100000; ++i)
    mem.WriteU32(longList + i * 12, i == 100000 ? 0 : longList + (i + 1) * 12);
mem.WriteU32(head, longList);
try { Run(cpu); throw new Exception("scheduler accepted 100001 nodes"); }
catch (InvalidOperationException ex) when (ex.Message.Contains("exceeded 100000 callbacks")) { }
cpu = NewCpu(); Build(1); Expect(1);

// Existing gameplay receipts show passes with 24-27 objects. Start warmed, below
// the scheduler's 600-pass diagnostic, and do not allocate in the callback.
const int size = 27, passes = 256;
Build(size); mode = Mode.Measure;
for (int i = 0; i < 32; ++i) Run(cpu);
calls = 0; digest = 0;
long before = GC.GetAllocatedBytesForCurrentThread();
for (int i = 0; i < passes; ++i) Run(cpu);
long bytes = GC.GetAllocatedBytesForCurrentThread() - before;
Check(calls == passes * size, "lost callbacks while measuring");
Check(Enumerable.Range(1, size).All(i => mem.ReadU32(Object(i) + 12) == passes + 32), "wrong guest state");
Console.WriteLine($"scheduler: {passes} passes, {calls} callbacks, state={digest:X8}, allocated={bytes} bytes");
if (!args.Contains("--baseline")) Check(bytes == 0, "warmed ordinary passes allocated");

enum Mode { Record, Relink, Reenter, Abort, Measure }
sealed class FixtureAbort : Exception { }
