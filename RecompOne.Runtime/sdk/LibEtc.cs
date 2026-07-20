using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class LibEtc
{
    static int _vcount;

    public static void VSync(CpuContext c, IMemory m)
    {
        int mode = (int)c.A0;
        Log.Sdk($"VSync({mode})");
        if (mode < 0) { c.V0 = (uint)_vcount; return; }
        if (mode == 1) { c.V0 = 0; return; }

        Runtime.PresentFrame();
        _vcount++;
        c.V0 = 0;
    }
}
