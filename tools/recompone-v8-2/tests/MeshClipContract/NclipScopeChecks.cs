using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class NclipScopeChecks
{
    public static int Run()
    {
        bool oldHigh = ConfigManager.View.HighResolution3D;
        bool oldGameplay = GpuHle.GameplayActive;
        ConfigManager.View.HighResolution3D = true;
        GpuHle.GameplayActive = true;
        var m = new PSMemory();
        var c = new CpuContext { GP = 0x80080000, A0 = 0x80050000 };
        for (int i = 0; i < 8; i++) Gte.WriteControl(i, 0);
        Gte.WriteControl(0, 4096);
        Gte.WriteControl(2, 4096);
        Gte.WriteControl(4, 4096);
        Gte.WriteControl(24, 0);
        Gte.WriteControl(25, 0);
        Gte.WriteControl(26, 256);
        int FaceTest()
        {
            Gte.Write(0, unchecked((ushort)-32)); Gte.Write(1, 256);
            Gte.Write(2, 32u << 16); Gte.Write(3, 256);
            Gte.Write(4, 32); Gte.Write(5, 256);
            Gte.Execute(0x4A280030);
            Gte.Execute(0x4B400006);
            return unchecked((int)Gte.Read(24));
        }
        try
        {
            if (FaceTest() >= 0)
                throw new Exception("Unscoped road/effect face test must retain its native sign");
            V82Compat.BeginObjectRender(c, m);
            try
            {
                if (FaceTest() <= 0)
                    throw new Exception("Owned mesh must reach Enhanced's replacement cull");
            }
            finally { V82Compat.EndObjectRender(c, m); }
            if (FaceTest() >= 0)
                throw new Exception("Object cull bypass must end with its render scope");
            return 3;
        }
        finally
        {
            ConfigManager.View.HighResolution3D = oldHigh;
            GpuHle.GameplayActive = oldGameplay;
        }
    }
}
