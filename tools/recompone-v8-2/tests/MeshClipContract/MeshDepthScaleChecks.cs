using RecompOne.Runtime;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

static class MeshDepthScaleChecks
{
    public static int Run()
    {
        var memory = new PSMemory();
        const uint mesh = 0x80050000;
        float original = Gte.PreciseViewScale;
        try
        {
            for (int i = 0; i < 8; i++) Gte.WriteControl(i, 0);
            Gte.WriteControl(0, 4096);
            Gte.WriteControl(2, 4096);
            Gte.WriteControl(4, 4096);
            Gte.WriteControl(24, 160u << 16);
            Gte.WriteControl(25, 120u << 16);
            Gte.WriteControl(26, 256);
            // The same point (0.25, 0, 4), including the junction mesh scale.
            uint? screen = null;
            int checks = 0;
            foreach (int shift in new[] { 7, 8, 9, 10 })
            {
                memory.WriteU8(mesh + 1, (byte)shift);
                float previous = V82MeshClipCompat.BeginMeshDepth(memory, mesh);
                try
                {
                    Gte.Write(0, (uint)(1 << (shift - 2)));
                    Gte.Write(1, (uint)(4 << shift));
                    Gte.Execute(0x4A180001);
                    if (!Gte.TryGetStoreVertex(14, out var v)) throw new Exception("Missing mesh vertex");
                    if (v.ViewX != 64 || v.ViewZ != 1024 || v.PerspectiveW != 1024)
                        throw new Exception("Mesh depth must use shared scene units");
                    if (v.Depth != 4 << shift) throw new Exception("Native SZ must remain unchanged");
                    screen ??= v.PackedScreenPosition;
                    if (screen != v.PackedScreenPosition) throw new Exception("Projection changed across mesh scales");
                    checks += 4;
                }
                finally { Gte.PreciseViewScale = previous; }
            }
            return checks;
        }
        finally { Gte.PreciseViewScale = original; }
    }
}
