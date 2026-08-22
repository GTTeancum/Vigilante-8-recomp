using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Behavior-free text diagnostics for V8:2's stock native water path.</summary>
public static class V82NativeWaterDiagnostics
{
    static readonly bool Enabled =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_NATIVE_WATER") == "1";
    static uint _generation;
    static uint _frame;

    public static void Initialize(CpuContext c, IMemory m, uint argument, uint color)
    {
        if (!Enabled)
            return;
        _generation++;
        _frame = 0;
        uint plane = unchecked(0x002FF800u - argument);
        Console.Error.WriteLine(
            $"[V82NativeWater] init generation={_generation} " +
            $"argument=0x{argument:X8} plane=0x{plane:X8} " +
            $"color=0x{color:X8} gp=0x{c.GP:X8}");
    }

    public static void Frame(CpuContext c, IMemory m)
    {
        if (!Enabled)
            return;
        _frame++;
        if (_frame > 10 && _frame % 60 != 0)
            return;
        Console.Error.WriteLine(
            $"[V82NativeWater] frame generation={_generation} index={_frame} " +
            $"plane=0x{m.ReadU32(c.GP + 0xDB0u):X8} " +
            $"color=0x{m.ReadU32(c.GP + 0xDA0u):X8} " +
            $"xwat-clut=0x{m.ReadU16(c.GP + 0xDC6u):X4} " +
            $"xwat-tpage=0x{m.ReadU16(c.GP + 0xDC8u):X4} " +
            $"xwat-size=0x{m.ReadU16(c.GP + 0xDCAu):X4}");
    }
}
