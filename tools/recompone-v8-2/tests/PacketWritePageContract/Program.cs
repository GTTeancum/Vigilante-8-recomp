// Public API differential contract for the conservative guest packet-store page hint.
// Source-only. No graphics window, retail asset, or generated guest code.
using RecompOne.Runtime;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

var tex = new GpuHle.TerrainCellTextures([default], 1, true, default);
var t = new GpuHle.TerrainTransitionPacket(tex, 1, 4, 2, 5, 3, 6, "test", 4, 9);
var n = new GpuHle.TriangleNclipPacket(17, 18, -13.5, true, true, 1, 0, 2, 0, 1, 2, 3, 4, 5);
long checks = 0;
ulong digest = 0xCBF29CE484222325;
void Check(bool yes, string msg) { if (!yes) throw new Exception(msg); }
foreach (var mode in new[] { RunMode.Retail, RunMode.Devkit })
{
    Runtime.SetMode(mode);
    uint mask = (mode == RunMode.Retail ? MemoryMap.RetailRamSize : MemoryMap.DevkitRamSize) - 1;
    uint K(uint a) => a & mask;
    var mem = new PSMemory();
    // Modal, coarse, transition, nclip, pending nclip link, native road, terrain route.
    const int Modal=1, Coarse=2, Transition=4, Nclip=8, Pending=16, Road=32, Route=64;
    var state = new Dictionary<uint,int>();
    void Put(uint a, int flag) { a=K(a); state[a]=state.GetValueOrDefault(a) | flag; }
    int Get(uint a) => state.GetValueOrDefault(K(a));
    void Reset() { GpuHle.ResetSceneTracking(); state.Clear(); }
    void Register(uint a, int kind)
    {
        uint tag = a | 0xA0000000;
        switch (kind)
        {
            case Modal: GpuHle.RegisterNativeModalPanel(tag); break;
            case Coarse: GpuHle.RegisterCoarseTerrainPacket(tag,tex,true,"test",4,9); break;
            case Transition: GpuHle.RegisterTerrainTransitionPacket(tag,t); break;
            case Nclip: GpuHle.RegisterTriangleNclipPacket(tag,n); Put(a,Pending); break;
            default: throw new Exception("Unsupported tag");
        }
        Put(a,kind);
    }
    void Store(uint a, bool road=false, bool route=false, bool actual=false)
    {
        int old=Get(a);
        if (road) GpuHle.BeginNativeRoadPacketWrites(211);
        if (route) GpuHle.BeginTerrainRoutePacketWrites();
        if (actual) mem.WriteU32(a|0x80000000,0x12345678); else GpuHle.ObservePacketWrite(a);
        if (route) GpuHle.EndTerrainRoutePacketWrites();
        if (road) GpuHle.EndNativeRoadPacketWrites();
        if (a<MemoryMap.RamWindow)
            state[K(a)]=((old & Pending)!=0 ? Nclip:0) | (road ? Road:0) | (route ? Route:0);
    }
    void Verify(uint a)
    {
        uint k=a | 0x80000000; int s=Get(a);
        bool m=GpuHle.IsNativeModalPanel(k), c=GpuHle.TryGetCoarseTerrainPacket(k,out var ct),
            tr=GpuHle.TryGetTerrainTransitionPacket(k,out var tt), nc=GpuHle.TryGetTriangleNclipPacket(k,out var nt),
            road=GpuHle.IsNativeRoadPacket(k), route=GpuHle.IsTerrainRoutePacket(k);
        int got=(m?Modal:0)|(c?Coarse:0)|(tr?Transition:0)|(nc?Nclip:0)|(road?Road:0)|(route?Route:0);
        if (got != (s & ~Pending) || (c && (ct.Source!="test" || ct.X!=4 || ct.Z!=9 || !ct.SecondHalf)) ||
            (tr && tt!=t) || (nc && nt!=n) || (road && GpuHle.NativeRoadPriority(k)!=211))
            throw new Exception($"Packet-state mismatch {mode}: address={a:X} expected={s} actual={got}");
        digest = unchecked((digest ^ (uint)got ^ k) * 0x100000001B3); checks++;
    }
    // Every independent publisher must mark a cold page, including the nclip link exception.
    int i=0;
    foreach (int field in new[] { Modal, Coarse, Transition, Nclip, Road, Route })
    {
        Reset(); uint a=0x131u+0x2000u*(uint)++i, samePage=a+12; // deliberately unaligned
        if (field==Road || field==Route) Store(a,field==Road,field==Route); else Register(a,field);
        Verify(a); Store(samePage); Verify(a);
        Store(a,actual:true); Verify(a); Store(a,actual:true); Verify(a);
        Check(mem.ReadU32(a|0xA0000000)==0x12345678,"Word-store path not exercised");
    }
    Reset(); uint low=0x5031, mirror=low+MemoryMap.RetailRamSize;
    Register(low,Modal); Store(mirror,actual:true); Verify(low); Verify(mirror);
    Check(GpuHle.IsNativeModalPanel(low)==(mode==RunMode.Devkit),"Incorrect retail/devkit alias");
    Register(mask,Modal); GpuHle.ObservePacketWrite(0x1FFFFFFF); Verify(mask); Store(mask); Verify(mask);
    // Two packets share one page but occupy different retirement ranges. A page may
    // remain marked after partial cleanup. Road/modal survive BeginPacketArena by design.
    Reset(); uint first=0x8001, other=first+128; Store(first,true,true); Register(first,Modal);
    Register(other,Coarse); GpuHle.BeginPacketArena(first,first+96); state[first]&=~Route;
    Verify(first); Verify(other); Store(first); Store(other); Verify(first); Verify(other);
    // Independent expected state over deterministic mixed writes, aliases, and resets.
    uint[] loc = Enumerable.Range(0,96).Select(j => 0x10000u + (uint)((j/6)*4096+(j%6)*5)).ToArray();
    var rnd = new Random(0x42A9); int[] kinds=[Modal,Coarse,Transition,Nclip];
    for (int step=0; step<18000; step++)
    {
        uint a=loc[rnd.Next(loc.Length)] + (rnd.Next(4)==0 ? MemoryMap.RetailRamSize : 0u);
        switch (rnd.Next(12))
        {
            case <4: Register(a,kinds[rnd.Next(4)]); break;
            case 4: Store(a,true,false); break;
            case 5: Store(a,false,true); break;
            case 6: Store(a,true,true); break;
            case 7: // invalid registrations must not affect an existing key
                GpuHle.RegisterCoarseTerrainPacket(a,default,false,"ignored",0,0);
                GpuHle.RegisterTerrainTransitionPacket(a,default); break;
            case 8 when step%113==0: Reset(); break;
            default: Store(a,actual:step%7==0); break;
        }
        Verify(a); Verify(a^MemoryMap.RetailRamSize);
        if (step%191==0) foreach (uint p in loc) { Verify(p); Verify(p|MemoryMap.RetailRamSize); }
    }
}
Console.WriteLine($"packet-write contract PASS: public state checks={checks}, digest={digest:X16}");
