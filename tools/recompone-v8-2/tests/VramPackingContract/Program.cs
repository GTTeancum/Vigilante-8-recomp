using System.Text.Json;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

{
    var m = new PSMemory();
    var c = new CpuContext { SP=0x801FF000, A0=32, A1=16, A2=64, A3=256 };
    uint root=0x80190000, occupied=root+24, free=root+48;
    m.WriteU32(root+8,4); m.WriteU32(root+16,occupied); m.WriteU32(root+20,free);
    m.WriteU32(occupied+8,1); m.WriteU16(free,320); m.WriteU16(free+4,64); m.WriteU16(free+6,256);
    m.WriteU32(c.SP+0x48,64); m.WriteU32(c.SP+0x4C,256);
    if(NativeVramPlacement.FindFreeLeaf(c,m,root)!=free) throw new Exception("Pinned split hides free child");
    Console.WriteLine("PASS pinned native split retains allocatable children while occupied leaves stay excluded");
}

// All 48 real Quest loads, including the late full-page texture that exposed
// fragmentation in Houston's final mission. Reorder only guest reservations.
using var matrix = JsonDocument.Parse(File.ReadAllText(
    Path.Combine(AppContext.BaseDirectory, "quest-matrix.json")));
int baselineFailures=0;
foreach (var fixture in matrix.RootElement.EnumerateArray())
{
foreach (bool improved in new[] { false, true })
{
    var requests = fixture.GetProperty("requests").Deserialize<uint[][]>()!;
    if(improved)
    {
        int guests=fixture.GetProperty("guests").GetInt32();
        requests=requests.Take(guests).OrderBy(r=>r[3]==1?0:1)
            .ThenByDescending(r=>r[0]).ThenByDescending(r=>r[1])
            .Concat(requests.Skip(guests)).ToArray();
    }
    var m = new PSMemory();
    var c = new CpuContext { GP = 0x80060000, SP = 0x801FF000 };
    uint[] targets = [0x80020B04, 0x80020CD4, 0x80020D00, 0x80020D50, 0x80020D50];
    for (uint i = 0; i < 5; i++) m.WriteU32(0x80010598 + 4*i, targets[i]);
    Vigilante82PC.Heap = 0x80100000;
    Vigilante82PC.Live.Clear();
    Vigilante82PC.Failures = 0;
    Vigilante82PC.Improved = improved;
    Vigilante82PC.func_8002091C(c,m);
    var occupied = new bool[1024*512];
    var rectangles = new List<(uint X,uint Y)>();
    foreach (var r in requests)
    {
        c.A0=r[0]; c.A1=r[1]; c.A2=r[2]; c.A3=r[3];
        m.WriteU32(c.SP+16,r[4]); m.WriteU32(c.SP+20,r[5]);
        Vigilante82PC.func_80020A80(c,m);
        if (c.V0 == 0) continue;
        uint x=m.ReadU16(c.V0), y=m.ReadU16(c.V0+2);
        if (x+r[0]>1024 || y+r[1]>512 || (x<320 && y<480)) throw new Exception("VRAM/framebuffer bounds");
        // Retail permits images wider than a page limit (e.g. loading cards)
        // if their origin is page-aligned. Match that original signed check.
        uint ox=x & (r[2]-1), oy=y & (r[3]-1);
        if ((ox != 0 && (long)ox > (long)r[4]-r[0]) ||
            (oy != 0 && (long)oy > (long)r[5]-r[1])) throw new Exception("Texture page origin constraint");
        for(uint yy=y;yy<y+r[1];yy++) for(uint xx=x;xx<x+r[0];xx++)
        {
            if(occupied[yy*1024+xx]) throw new Exception("Overlapping texture allocation");
            occupied[yy*1024+xx]=true;
        }
        rectangles.Add((x,y));
    }
    if (improved && Vigilante82PC.Failures != 0) throw new Exception("Improved packing failed");
    if (!improved) baselineFailures+=Vigilante82PC.Failures;
    // Coordinates remain valid for native frees even when tree nodes coalesce.
    foreach (var rect in rectangles.AsEnumerable().Reverse())
    {
        c.A0=rect.X; c.A1=rect.Y;
        Vigilante82PC.func_80020F5C(c,m);
        if(c.V0 != 1) throw new Exception("Native rectangle free failed");
    }
    Vigilante82PC.func_80020754(c,m);
    if(Vigilante82PC.Live.Count != 0) throw new Exception("Native allocator nodes leaked");
    Console.WriteLine($"PASS {fixture.GetProperty("vehicle").GetString()} mission={fixture.GetProperty("mission").GetInt32()} improved={improved}: {requests.Length} requests, failures={Vigilante82PC.Failures}; bounds, page constraints, overlap and complete native teardown verified");
}
}
if(baselineFailures==0)throw new Exception("Fixtures no longer reproduce original fragmentation");
