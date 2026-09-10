using RecompOne.Runtime;
using RecompOne.Runtime.Hle;

static class WaterMaterialChecks
{
    public static void Run()
    {
        var oldBackend = GpuHle.Backend;
        bool oldActive = GpuHle.Active, oldGameplay = GpuHle.GameplayActive;
        var sink = new Sink();
        GpuHle.Backend = sink; GpuHle.Active = true; GpuHle.GameplayActive = true;
        try
        {
            // Actual native water scanline command: draw mode + flat quad at
            // the far OT bucket, without a projected model vertex behind it.
            foreach (var kind in new[] { HleMaterialKind.WaterBase, HleMaterialKind.WaterSurface, HleMaterialKind.Ui })
            foreach (bool semi in new[] { false, true })
            {
                uint address = 0x10000u + (uint)kind * 0x100;
                GpuHle.BeginPacketArena(address, address+0x100);
                if (kind == HleMaterialKind.WaterBase) GpuHle.RegisterWaterBasePacketRange(address,address+28);
                if (kind == HleMaterialKind.WaterSurface) GpuHle.RegisterWaterSurfacePacketRange(address,address+28);
                var gpu = new Gpu();
                gpu.BeginOrderingTable(); gpu.SetOrderingTableDepth(0xFFF); gpu.SetOrderingTablePacket(address,6);
                sink.Materials.Clear();
                gpu.WriteGp0(0xE1000600);
                gpu.WriteGp0((semi ? 0x2A000000u : 0x28000000u) | 0xFFFFFF);
                gpu.WriteGp0(61u<<16); gpu.WriteGp0((61u<<16)|214);
                gpu.WriteGp0(62u<<16); gpu.WriteGp0((62u<<16)|214);
                if (sink.Materials.Count != 2 || sink.Materials.Any(m=>m!=kind))
                    throw new Exception($"Native scanline lost {kind} provenance: {string.Join(',',sink.Materials)}");
                GpuHle.BeginPacketArena(address,address+0x100);
            }
        }
        finally { GpuHle.Backend=oldBackend; GpuHle.Active=oldActive; GpuHle.GameplayActive=oldGameplay; }
        Console.WriteLine("PASS native water scanlines retain provenance without incidental model projection history; unowned UI unchanged.");
    }
    sealed class Sink : IGpuBackend
    {
        public readonly List<HleMaterialKind> Materials=[];
        public bool Ready=>true;
        public void DrawTri(in HleVertex a,in HleVertex b,in HleVertex c,in PrimFlags f)=>Materials.Add(f.Material);
        public void SetDrawEnv(in HleDrawEnv e) { }
        public void DrawRect(in HleRect r,in PrimFlags f) { }
        public void DrawLine(in HleVertex a,in HleVertex b,in PrimFlags f) { }
        public void FillRect(int x,int y,int w,int h,ushort c) { }
        public void CopyVram(int sx,int sy,int dx,int dy,int w,int h) { }
        public void WriteVram(int x,int y,int w,int h,ReadOnlySpan<ushort> p) { }
        public void ReadVram(int x,int y,int w,int h,Span<ushort> p) { }
        public void Flush() { }
        public void Present(in HleDispEnv d) { }
    }
}
