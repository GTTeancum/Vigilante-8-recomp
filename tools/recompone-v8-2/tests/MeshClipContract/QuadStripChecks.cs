using RecompOne.Runtime;
using RecompOne.Runtime.Hle;

static class QuadStripChecks
{
    public static int Run()
    {
        var oldBackend = GpuHle.Backend;
        bool oldActive = GpuHle.Active;
        var sink = new Sink();
        GpuHle.Backend = sink;
        GpuHle.Active = true;
        int checks = 0;
        try
        {
            foreach (uint kind in new uint[] { 0x28, 0x2C, 0x38, 0x3C })
            foreach (bool mirrored in new[] { false, true })
            {
                sink.Areas.Clear();
                var gpu = new Gpu();
                gpu.WriteGp0((kind << 24) | 0x808080);
                for (int i = 0; i < 4; i++)
                {
                    if (i > 0 && (kind & 0x10) != 0) gpu.WriteGp0(0x808080);
                    int x = (i & 1) == 0 ? 10 : 60;
                    if (mirrored) x = 70 - x;
                    int y = i < 2 ? 10 : 60;
                    gpu.WriteGp0((uint)(x | (y << 16)));
                    if ((kind & 4) != 0) gpu.WriteGp0((uint)(i * 16));
                }
                if (sink.Areas.Count != 2 ||
                    sink.Areas.Any(a => mirrored ? a >= 0 : a <= 0))
                    throw new Exception($"GP0 quad {kind:X2}: inconsistent strip winding, mirror={mirrored}");
                checks++;
            }
        }
        finally { GpuHle.Backend = oldBackend; GpuHle.Active = oldActive; }
        return checks;
    }

    sealed class Sink : IGpuBackend
    {
        public readonly List<float> Areas = [];
        public bool Ready => true;
        public void DrawTri(in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f) =>
            Areas.Add((b.X-a.X)*(c.Y-a.Y)-(b.Y-a.Y)*(c.X-a.X));
        public void SetDrawEnv(in HleDrawEnv e) { }
        public void DrawRect(in HleRect r, in PrimFlags f) { }
        public void DrawLine(in HleVertex a, in HleVertex b, in PrimFlags f) { }
        public void FillRect(int x,int y,int w,int h,ushort c) { }
        public void CopyVram(int sx,int sy,int dx,int dy,int w,int h) { }
        public void WriteVram(int x,int y,int w,int h,ReadOnlySpan<ushort> p) { }
        public void ReadVram(int x,int y,int w,int h,Span<ushort> p) { }
        public void Flush() { }
        public void Present(in HleDispEnv d) { }
    }
}
