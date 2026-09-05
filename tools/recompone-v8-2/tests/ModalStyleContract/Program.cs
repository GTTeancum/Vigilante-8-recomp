using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Hle;

int checks = 0;
void Check(bool condition, string name)
{
    if (!condition) throw new Exception(name);
    checks++;
}
foreach (var size in new[] {(64, 74, 192, 60), (32, 24, 256, 180), (72, 95, 176, 70)})
{
    var m = new PSMemory();
    var c = new CpuContext { GP = 0x80070000, A0 = 0x80020000,
        A1 = 0x80021000, SP = 0x801F0000, RA = 0x8001285C };
    const uint packet = 0x80030000;
    m.WriteU32(c.GP + 0x610, packet);
    m.WriteU32(c.A1, 0x00FFFFFF);
    m.WriteU16(c.A0, (ushort)size.Item1);
    m.WriteU16(c.A0+2, (ushort)size.Item2);
    m.WriteU16(c.A0+4, (ushort)size.Item3);
    m.WriteU16(c.A0+6, (ushort)size.Item4);
    GpuHle.Active = true;
    GpuHle.GameplayActive = false;
    Check(V82ModalStyle.DrawPanel(c,m), "non-gameplay retains native builder");
    Check(m.ReadU32(c.GP+0x610) == packet, "fallback does not allocate");
    GpuHle.GameplayActive = true;
    Check(!V82ModalStyle.DrawPanel(c,m), "enhanced modal replaces presentation");
    Check(m.ReadU32(packet) == 0x04FFFFFF, "four GP0 words and original link");
    Check(m.ReadU32(c.A1) == 0x30000, "OT points to new panel");
    Check(m.ReadU32(c.GP+0x610) == packet+20, "packet allocator advances exactly");
    Check(m.ReadU32(packet+12) == m.ReadU32(c.A0), "authored origin retained");
    Check(m.ReadU32(packet+16) == m.ReadU32(c.A0+4), "authored size retained");
    Check(c.SP == 0x801F0000 && c.RA == 0x8001285C, "stack and return preserved");
    Check(GpuHle.IsNativeModalPanel(packet) && GpuHle.IsNativeModalPanel(0x30000), "normalized provenance");
    GpuHle.ObservePacketWrite(0x30000);
    Check(!GpuHle.IsNativeModalPanel(packet), "reused packet cannot retain panel style");
    c.A2 = c.A0;
    try
    {
        using var inset = V82ModalStyle.InsetChoice(c, m);
        Check(m.ReadU16(c.A0) == size.Item1 + 12, "left arrow gutter");
        Check(m.ReadU16(c.A0+4) == size.Item3 - 24, "both arrow gutters");
        throw new InvalidOperationException("exercise unwind");
    }
    catch (InvalidOperationException) { }
    Check(m.ReadU16(c.A0) == size.Item1 && m.ReadU16(c.A0+4) == size.Item3,
        "choice geometry restored even on unwind");
}
Console.WriteLine($"PASS: {checks} native modal packet assertions");
