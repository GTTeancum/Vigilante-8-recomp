using System.Numerics;
using System.Reflection;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Hle;

var read = typeof(V82ModelBounds).GetMethod("ReadBounds",BindingFlags.Static|BindingFlags.NonPublic)!;
var memory = new PSMemory();
(Vector3 Min,Vector3 Max) Bounds(PSMemory m,uint p=0x10000,int n=3) => ((Vector3,Vector3))read.Invoke(null,[m,p,n])!;
void Vertex(int index,int x,int y,int z)
{
    uint p=0x10000+(uint)index*8;
    memory.WriteU16(p,unchecked((ushort)x)); memory.WriteU16(p+2,unchecked((ushort)y)); memory.WriteU16(p+4,unchecked((ushort)z));
}
Vertex(0,-100,20,40); Vertex(1,100,-20,400); Vertex(2,0,50,-10);
var original=Bounds(memory);
if(original!=(new Vector3(-100,-20,-10),new Vector3(100,50,400))) throw new Exception("Original bounds");
if(Bounds(memory)!=original) throw new Exception("Stable repeated camera");
memory.WriteU32(0x20000,123);
if(Bounds(memory)!=original) throw new Exception("Unrelated memory write");
Vertex(2,200,-100,500);
if(Bounds(memory)!=(new Vector3(-100,-100,40),new Vector3(200,20,500))) throw new Exception("Deformation missed");
memory.LoadBytes(0x10000,new byte[24]);
if(Bounds(memory)!=(Vector3.Zero,Vector3.Zero)) throw new Exception("Heap address reuse missed");
Vertex(0,-500,100,20);
if(Bounds(new PSMemory())!=(Vector3.Zero,Vector3.Zero)) throw new Exception("Memory owner changed");
if(Bounds(memory).Min.X!=-500) throw new Exception("Memory owner restored");
Console.WriteLine("PASS model source invalidation: camera reuse, deformation, unrelated writes, heap replacement, memory ownership.");

var rng=new Random(8721);
int rejected=0,triangles=0;
var points=new Vector3[24];
var corners=new DreamcastTerrainGeometry.Sample[8];
for(int test=0;test<10000;test++)
{
    Vector3 lo=new(rng.Next(-32768,0),rng.Next(-32768,0),rng.Next(-32768,0));
    Vector3 hi=new(rng.Next(1,32768),rng.Next(1,32768),rng.Next(1,32768));
    var rotation=Matrix4x4.CreateFromYawPitchRoll((float)rng.NextDouble()*6.28f,(float)rng.NextDouble()*6.28f,(float)rng.NextDouble()*6.28f);
    Vector3 offset=new(rng.Next(-100000,100000),rng.Next(-100000,100000),rng.Next(-100000,100000));
    for(int i=0;i<8;i++)
    {
        Vector3 p=new((i&1)==0?lo.X:hi.X,(i&2)==0?lo.Y:hi.Y,(i&4)==0?lo.Z:hi.Z);
        corners[i]=new(Vector3.TransformNormal(p,rotation)+offset,0,default,default);
    }
    if(!DreamcastTerrainGeometry.OutsideViewport(corners,default,80,60,160,0,160,0,120)) continue;
    rejected++;
    for(int i=0;i<24;i++) points[i]=Vector3.TransformNormal(new Vector3(
        lo.X+(hi.X-lo.X)*(float)rng.NextDouble(),lo.Y+(hi.Y-lo.Y)*(float)rng.NextDouble(),lo.Z+(hi.Z-lo.Z)*(float)rng.NextDouble()),rotation)+offset;
    for(int i=0;i<24;i+=3)
    {
        var polygon=points.Skip(i).Take(3).Select(v=>new Vector3(v.X*160+v.Z*80,v.Y*160+v.Z*60,v.Z)).ToList();
        foreach(Func<Vector3,float> plane in new Func<Vector3,float>[] {v=>v.Z-1,v=>v.X,v=>160*v.Z-v.X,v=>v.Y,v=>120*v.Z-v.Y})
        {
            if(polygon.Count==0) break;
            var output=new List<Vector3>(); var a=polygon[^1]; float da=plane(a);
            foreach(var b in polygon)
            {
                float db=plane(b);
                if((da>=0)!=(db>=0)) output.Add(Vector3.Lerp(a,b,da/(da-db)));
                if(db>=0) output.Add(b);
                a=b; da=db;
            }
            polygon=output;
        }
        if(polygon.Count>=3) throw new Exception("Rejected visible model triangle");
        triangles++;
    }
}
if(rejected<1000) throw new Exception("Insufficient coverage");
Console.WriteLine($"PASS 10000 model bounds, {rejected} rejected, {triangles} contained triangles independently clipped.");
WaterMaterialChecks.Run();
