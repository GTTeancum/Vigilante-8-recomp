using System.Buffers.Binary;
using RecompOne.Runtime;
using RecompOne.Runtime.Sdk;

int checks = 0;
void Check(bool value, string name)
{
    if (!value) throw new Exception(name);
    checks++;
}
byte[] source = new byte[12 + 48];
void Word(int offset, ushort value) => BinaryPrimitives.WriteUInt16LittleEndian(source.AsSpan(offset), value);
Word(0, 2); Word(2, 6); Word(4, 0); Word(6, 2048); Word(8, 2); Word(10, 1024);
for (int i = 12; i < source.Length; i++) source[i] = (byte)i;
var bank = new SelectionVoiceBank(source);
Check(bank.Count == 2 && bank.MaximumSampleBytes == 32, "resident slot size");
Check(bank.Pitch(0) == 2048 && bank.Pitch(1) == 1024, "authored pitch preserved");
Check(bank.Sample(0).SequenceEqual(source.AsSpan(12, 16).ToArray()), "first source boundaries");
Check(bank.Sample(1).SequenceEqual(source.AsSpan(28, 32).ToArray()), "last source boundaries");
source[12] = 0;
Check(bank.Sample(0)[0] == 12, "source is immutable");
byte[] emptyRange = (byte[])source.Clone(); emptyRange[8] = 0;
byte[] badPitch = (byte[])source.Clone(); badPitch[10] = 0xFF; badPitch[11] = 0xFF;
foreach (var malformed in new[] { Array.Empty<byte>(), source[..^1], emptyRange, badPitch })
{
    bool rejected = false;
    try { _ = new SelectionVoiceBank(malformed); }
    catch (InvalidDataException) { rejected = true; }
    Check(rejected, "reject malformed source");
}
var spu = new Spu();
Array.Fill(spu.Ram, (byte)0x55);
bool started = false;
spu.LoadRamAndStart(0x2000, bank.Sample(0), () =>
{
    Check(spu.Ram.AsSpan(0x2000, 16).SequenceEqual(bank.Sample(0)), "upload precedes key-on");
    spu.WriteReg16(0x1F801C26, 0x400);
    spu.WriteReg16(0x1F801C24, bank.Pitch(0));
    spu.WriteReg16(0x1F801D88, 4);
    started = true;
});
Check(started && spu.ReadReg16(0x1F801C24) == 2048, "voice uses authored pitch");
Check(spu.Ram.AsSpan(0, 0x2000).ToArray().All(x => x == 0x55), "preceding live bank preserved");
Check(spu.Ram.AsSpan(0x2010).ToArray().All(x => x == 0x55), "following live bank preserved");
spu.LoadRamAndStart(0x3000, bank.Sample(1), () =>
{
    spu.WriteReg16(0x1F801C36, 0x600);
    spu.WriteReg16(0x1F801C34, bank.Pitch(1));
    spu.WriteReg16(0x1F801D88, 8);
});
Check(spu.Ram.AsSpan(0x2000, 16).SequenceEqual(bank.Sample(0)), "second player preserves first player's bytes");
Check(spu.ReadReg16(0x1F801C24) == 2048 && spu.ReadReg16(0x1F801C34) == 1024,
    "two player pitches remain independent");
var memory = new RecompOne.Runtime.Memory.PSMemory();
var cpu = new RecompOne.Runtime.Context.CpuContext();
const uint allocator = 0x800BDD78;
memory.WriteU32(allocator, 0x12345678);
var active = typeof(V82VehicleRegistry).GetProperty("NativeSelectorAudioActive",
    System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic)!;
active.SetValue(null, true);
try
{
    cpu.A0 = 286848;
    V82Compat.SpuMallocPre(cpu, memory);
    cpu.V0 = uint.MaxValue;
    V82Compat.SpuMallocPost(cpu, memory);
    Check(memory.ReadU32(allocator) == 0x12345678 && cpu.V0 == uint.MaxValue,
        "selector allocation failure cannot reset live bank allocator");
}
finally { active.SetValue(null, false); }
cpu.RA = 0x8010798C;
Check(V82VehicleRegistry.PlayNativeSelectorAcceptance(cpu, memory), "stock acceptance uses unchanged native sound player");
cpu.RA = 0x801086CC;
Check(V82VehicleRegistry.PlayNativeSelectorAcceptance(cpu, memory), "native enemy acceptance remains unchanged");
Console.WriteLine($"{checks} selector audio assertions passed");
