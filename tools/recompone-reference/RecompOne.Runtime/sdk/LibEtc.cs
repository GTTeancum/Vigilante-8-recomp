using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class LibEtc
{
    static int _vcount;
    static readonly bool TraceVSync =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VSYNC") == "1";

    public static void VSync(CpuContext c, IMemory m)
    {
        int mode = (int)c.A0;
        Log.Sdk($"VSync({mode})");
        if (TraceVSync && (_vcount < 10 || (_vcount % 300) == 0))
            Console.Error.WriteLine($"[VSync] enter count={_vcount} mode={mode}");
        if (mode < 0) { c.V0 = (uint)_vcount; return; }
        if (mode == 1) { c.V0 = 0; return; }

        Runtime.PresentFrame();
        _vcount++;
        if (TraceVSync && _vcount <= 10)
            Console.Error.WriteLine($"[VSync] leave count={_vcount}");
        V8Compat.TraceGameplayHeartbeat(c, m);
        c.V0 = 0;
    }
}
