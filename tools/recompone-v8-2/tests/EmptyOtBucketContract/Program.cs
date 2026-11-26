// Synthetic native OT/GP0 lists. No guest, retail files, GL, or window needed.
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

bool baseline = args.Contains("--baseline"); // run the same test DLL with an unmodified v5 runtime
bool trace = Environment.GetEnvironmentVariable("RECOMPONE_TRACE_MESHES") == "1";
bool bench = args.Contains("--bench");
static void Check(bool condition, string message)
{
    if (!condition) throw new Exception(message);
}
static string Hash(ReadOnlySpan<byte> data) => Convert.ToHexString(SHA256.HashData(data));
static uint Xy(uint x, uint y) => x | y << 16;
static void Tag(PSMemory m, uint at, uint next, params uint[] words)
{
    m.WriteU32(at, (next & 0x00ff_ffff) | (uint)words.Length << 24);
    for (int i = 0; i < words.Length; i++) m.WriteU32(at + (uint)(4 * i + 4), words[i]);
}

void Verify(RunMode mode, bool enhanced)
{
    Runtime.SetMode(mode);
    var m = new PSMemory();
    var gpu = Runtime.Gpu!;
    var ctx = new CpuContext();
    Runtime.SetContext(ctx, m);
    var sink = new Sink();
    GpuHle.Backend = sink; GpuHle.ResetSceneTracking();
    GpuHle.Active = enhanced; GpuHle.GameplayActive = true;
    _ = GpuHle.ConsumeVehiclePacketOwnershipMetrics();
    uint b = mode == RunMode.Devkit ? 0x0021_0000u : 0x0001_0000u;
    uint e0=b+0x04, e1=b+0x30, e2=b+0x0c, e3=b+0x00, e4=b+0x20;
    uint lead=b+0x200, vehicle=b+0x400, water=b+0x300, bad=b+0x500, marker=b+0x600, world=b+0x380;
    // Noncontiguous 0-word buckets with a payload at the head, consecutive
    // buckets/payloads, a malformed packet, a 1-word direct marker, and a
    // tagged empty tail. The reserved high-bit and 0xffffff sentinels both stop.
    Tag(m,lead,e0,0x600000ff,Xy(8,8),Xy(12,12));
    Tag(m,e0,e1); Tag(m,e1,vehicle);
    Tag(m,vehicle,water,0x2000ff00,Xy(8,8),Xy(21,8),Xy(8,21));
    Tag(m,water,e2,0x280000ff,Xy(29,22),Xy(35,22),Xy(29,28),Xy(35,28));
    Tag(m,e2,bad);
    // Valid green rectangle followed by a truncated polygon: the *entire* packet must be rejected.
    Tag(m,bad,marker,0x6000ff00,Xy(110,100),Xy(4,4),0x200000ff);
    Tag(m,marker,e3,0u); Tag(m,e3,world);
    Tag(m,world,e4,0x20ff0000,Xy(40,32),Xy(48,32),Xy(40,40));
    Tag(m,e4,mode == RunMode.Devkit ? 0x800001u : 0xffffffu);
    // Metadata deliberately overlaps empty tags, and alternates radically
    // between payloads. No skipped bucket may leak a previous material/group.
    GpuHle.RegisterVehiclePacketRange(vehicle|0xa0000000, (vehicle+20)|0xa0000000);
    GpuHle.RegisterVehiclePacket(e0); GpuHle.RegisterVehiclePacket(e4);
    GpuHle.RegisterWaterBasePacketRange(e1,e1+4);
    GpuHle.RegisterWaterBasePacketRange(water,water+24);
    GpuHle.RegisterDreamcastWaterBaseQuad(water,water+24,new(default,default,default,default,31,63,127));
    var mesh = new GpuHle.DreamcastWaterSurfaceMesh([new(1,2,3,4,5,6,7,8,9,10,11)],1,1);
    GpuHle.RegisterDreamcastWaterSurfaceMesh(water,water+24,mesh);
    GpuHle.RegisterWorldObjectPacketRange(world,world+20);
    var tex = new GpuHle.TerrainCellTextures([default],1,true,default);
    GpuHle.RegisterCoarseTerrainPacket(e1,tex,false,"empty",0,0,direct:true);
    GpuHle.RegisterCoarseTerrainPacket(marker,tex,false,"marker",0,0,direct:true);
    ctx.A0 = lead | 0xa0000000;
    LibGpu.DrawOTag(ctx,m);
    var counts = GpuHle.ConsumeVehiclePacketOwnershipMetrics();
    Check(counts.Lookups == (baseline || trace ? 11 : 6), $"Work count: {counts.Lookups}");
    // Bypass DrawOTag for this last command: the table must have reset all provenance and depth.
    foreach (var word in new uint[]{0x6000ffff,Xy(60,60),Xy(2,2)}) gpu.WriteGp0(word);
    if (enhanced)
    {
        var p = sink.Packets;
        Check(p.Select(f => f.PacketAddress).SequenceEqual(new[]{lead,vehicle,water,water,marker,world,0u}),
            "Draw count/address/order, malformed rejection, or direct marker changed");
        Check(p.Select(f => f.OtIndex).SequenceEqual(new[]{0,4,4,4,3,2,0}), "OT bucket depths changed");
        Check(p[1].Vehicle && !p[2].Vehicle && p[2].WaterBaseGroup==water &&
            p[2].WaterBaseQuad?.G == 63 && p[2].WaterSurfaceMesh==mesh &&
            p[5].WorldObject && !p[5].Vehicle && p[5].WaterSurfaceMesh==null &&
            !p[^1].Vehicle && !p[^1].WorldObject && p[^1].WaterBaseGroup==0,
            "Payload ownership/provenance leaked across buckets");
    }
    else
    {
        Check(gpu.Vram[100*Gpu.VramWidth+110]==0 && gpu.Vram[60*Gpu.VramWidth+60]!=0 &&
            gpu.Vram[10*Gpu.VramWidth+10]!=0, "Software image or invalid-packet rejection changed");
    }

    // Menus may split a GP0 primitive across payloads. An empty marker between
    // halves must not consume the GPU FIFO or alter the final packet's provenance.
    GpuHle.GameplayActive=false;
    uint front=b+0x900, tail=b+0x940;
    Tag(m,front,e0,0x20007fff,Xy(100,80)); Tag(m,e0,e1); Tag(m,e1,tail);
    Tag(m,tail,0xffffff,Xy(113,80),Xy(100,93)); GpuHle.RegisterVehiclePacket(tail);
    ctx.A0=front | 0x80000000;
    LibGpu.DrawOTag(ctx,m);
    Check(enhanced ? sink.Packets[^1].PacketAddress==tail && sink.Packets[^1].OtIndex==1 && sink.Packets[^1].Vehicle
        : gpu.Vram[82*Gpu.VramWidth+102]!=0, "Menu split command not preserved");
    int events=sink.Events.Count;
    string imageBefore = Hash(MemoryMarshal.AsBytes(gpu.Vram.AsSpan()));
    Tag(m,e0,e1); Tag(m,e1,0x800005);
    ctx.A0=e0; LibGpu.DrawOTag(ctx,m);
    Check(events==sink.Events.Count && imageBefore==Hash(MemoryMarshal.AsBytes(gpu.Vram.AsSpan())),
        "Empty-only list emitted work");
    // Every field of every backend event is serialized, including vertex data,
    // env, primitive flags, packet addresses/depths, and optional water metadata.
    string eventsHash=Hash(Encoding.UTF8.GetBytes(string.Join("\n",sink.Events)));
    // Frozen pair-5 reference digests for the complete synthetic frames.
    string expectedPackets = !enhanced
        ? "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"
        : mode == RunMode.Retail
            ? "75529F65C7962BE16E2F2284405A7FB6C52A178117344D4ECF562678AFC56EF4"
            : "1063BA31DF33362888E0DE6FB0D7E44A959B72685D9A40199ACADA3FEB713735";
    string expectedVram = enhanced
        ? "30E14955EBF1352266DC2FF8067E68104607E750ABB9D3B36582B8AF909FCB58"
        : "82A3762E834FF89F604CFB124536BB07CF014B4A4B33C8D61FC51FE4174A0892";
    Check(eventsHash == expectedPackets && imageBefore == expectedVram, "Complete v5 synthetic frame mismatch");
    Console.WriteLine($"PARITY {mode} enhanced={enhanced} events={events} packetSHA={eventsHash} vramSHA={imageBefore}");
}

void CountWork(int buckets, int packets)
{
    Runtime.SetMode(RunMode.Devkit);
    var m=new PSMemory(); var ctx=new CpuContext();
    Runtime.SetContext(ctx,m);
    GpuHle.Backend=null; GpuHle.ResetSceneTracking(); GpuHle.Active=false; GpuHle.GameplayActive=true;
    const uint end=0xffffff, bucketBase=0x20000, packetBase=0x40000;
    uint head=end;
    int emitted=0;
    // Rendering commands are intentionally tiny: isolate table traversal and
    // packet classification, not triangle rasterization or driver performance.
    for(int i=0;i<Math.Max(buckets,packets);i++)
    {
        int target=(i+1)*packets/Math.Max(buckets,packets);
        while(emitted<target)
        {
            uint p=packetBase+16u*(uint)emitted++; Tag(m,p,head,0xe1000600);head=p;
        }
        if(i<buckets){uint e=bucketBase+4u*(uint)i; Tag(m,e,head); head=e;}
    }
    for(uint i=0;i<4;i++)
    {
        uint p=packetBase+16u*i*(uint)packets/4; GpuHle.RegisterVehiclePacketRange(p,p+16);
    }
    ctx.A0=head|0x80000000;
    for(int k=0;k<16;k++) LibGpu.DrawOTag(ctx,m);
    _=GpuHle.ConsumeVehiclePacketOwnershipMetrics();
    int reps=bench?128:2;
    long before=GC.GetAllocatedBytesForCurrentThread();
    long stamp=Stopwatch.GetTimestamp();
    for(int k=0;k<reps;k++) LibGpu.DrawOTag(ctx,m);
    double seconds=Stopwatch.GetElapsedTime(stamp).TotalSeconds;
    long bytes=GC.GetAllocatedBytesForCurrentThread()-before;
    var metrics=GpuHle.ConsumeVehiclePacketOwnershipMetrics();
    Check(metrics.Lookups==(long)reps*(packets+(baseline||trace?buckets:0)),
        $"Unexpected metadata lookup count {metrics.Lookups}");
    Console.WriteLine($"WORK buckets={buckets} payloads={packets} traversals={reps} ownership={metrics.Lookups} range-tests={metrics.RangeTests} allocated={bytes}"+
        (bench?$" microseconds/table={1e6*seconds/reps:F2}":""));
}

foreach (RunMode mode in new[]{RunMode.Retail,RunMode.Devkit})
foreach (bool enhanced in new[]{true,false}) Verify(mode,enhanced);
foreach (int buckets in new[]{0,64,4096}) CountWork(buckets,512);

sealed class Sink : IGpuBackend
{
    static readonly JsonSerializerOptions Json=new(){IncludeFields=true};
    public bool Ready=>true;
    public readonly List<PrimFlags> Packets=[];
    public readonly List<string> Events=[];
    void Record<T>(string kind,T value)=>Events.Add(kind+":"+JsonSerializer.Serialize(value,Json));
    public void SetDrawEnv(in HleDrawEnv e)=>Record("env",e);
    public void DrawTri(in HleVertex a,in HleVertex b,in HleVertex c,in PrimFlags f){Packets.Add(f);Record("tri",(a,b,c,f));}
    public void DrawRect(in HleRect r,in PrimFlags f){Packets.Add(f);Record("rect",(r,f));}
    public void DrawLine(in HleVertex a,in HleVertex b,in PrimFlags f){Packets.Add(f);Record("line",(a,b,f));}
    public void FillRect(int x,int y,int w,int h,ushort c)=>Record("fill",(x,y,w,h,c));
    public void CopyVram(int sx,int sy,int dx,int dy,int w,int h)=>Record("copy",(sx,sy,dx,dy,w,h));
    public void WriteVram(int x,int y,int w,int h,ReadOnlySpan<ushort> p)=>Record("write",(x,y,w,h,p.ToArray()));
    public void ReadVram(int x,int y,int w,int h,Span<ushort> p){p.Clear();Record("read",(x,y,w,h));}
    public void Flush()=>Record("flush",0);
    public void Present(in HleDispEnv d)=>Record("present",d);
}
