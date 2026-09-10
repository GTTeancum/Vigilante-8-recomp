using System.Numerics;
using RecompOne.Runtime.Hle;

var random = new Random(81329);
int rejected = 0, triangles = 0;
var samples = new DreamcastTerrainGeometry.Sample[25];
var vertices = new DreamcastTerrainGeometry.Vertex[25];
var leaves = new DreamcastTerrainGeometry.Leaf[16];
for (int test = 0; test < 10000; test++)
{
    var rotation = Matrix4x4.CreateFromYawPitchRoll((float)random.NextDouble()*6.28f,
        (float)random.NextDouble()*2-1, (float)random.NextDouble()-.5f);
    var translation = new Vector3(random.Next(-12000,12000), random.Next(-4000,4000), random.Next(-12000,16000));
    var axis = Vector3.TransformNormal(Vector3.UnitY, rotation);
    int originX = test % 7 == 0 ? 32500 : random.Next(-2000,2000);
    for (int x = 0; x < 5; x++)
    for (int z = 0; z < 5; z++)
    {
        float height = random.Next(0,2048)*8;
        var world = new Vector3(unchecked((short)(originX+x*256)), height, z*256);
        samples[x*5+z] = new(Vector3.TransformNormal(world,rotation)+translation,
            height, new(128), new(40));
    }
    float width = test % 3 == 0 ? 160 : 320, heightView = width*.75f;
    float centerX = width/2, centerY = heightView/2, scale = width*.8f;
    bool culled = DreamcastTerrainGeometry.OutsideViewport(samples,axis,centerX,centerY,scale,
        -width*.167f,width*1.167f,0,heightView);
    if (!culled) continue;
    rejected++;
    int count = DreamcastTerrainGeometry.Build(samples,axis,test%2==0,vertices,leaves);
    for (int i = 0; i < count; i++)
    {
        var leaf = leaves[i];
        // Test both possible authored diagonals against actual morphed leaves.
        foreach (var tri in new[] {
            new[] {leaf.TopLeft.View,leaf.TopRight.View,leaf.BottomLeft.View},
            new[] {leaf.BottomRight.View,leaf.TopRight.View,leaf.BottomLeft.View},
            new[] {leaf.TopLeft.View,leaf.TopRight.View,leaf.BottomRight.View},
            new[] {leaf.TopLeft.View,leaf.BottomLeft.View,leaf.BottomRight.View}})
        {
            var polygon = tri.Select(v => new Vector3(v.X*scale+v.Z*centerX,
                v.Y*scale+v.Z*centerY,v.Z)).ToList();
            // Independently clip the final triangles against the view volume.
            foreach (Func<Vector3,float> plane in new Func<Vector3,float>[] {
                v=>v.Z-1, v=>v.X+width*.167f*v.Z, v=>width*1.167f*v.Z-v.X,
                v=>v.Y, v=>heightView*v.Z-v.Y })
            {
                var next = new List<Vector3>();
                if (polygon.Count == 0) break;
                var a=polygon[^1]; float da=plane(a);
                foreach (var b in polygon)
                {
                    float db=plane(b);
                    if ((da>=0)!=(db>=0)) next.Add(Vector3.Lerp(a,b,da/(da-db)));
                    if(db>=0) next.Add(b);
                    a=b; da=db;
                }
                polygon=next;
            }
            if(polygon.Count>=3) throw new Exception($"Visible triangle rejected: test={test} leaf={i}");
            triangles++;
        }
    }
}
if(rejected<1000 || triangles<1000) throw new Exception("Insufficient rejection coverage");
Console.WriteLine($"PASS 10000 authored terrain patches; {rejected} culled patches, {triangles} morphed triangles independently clipped; signed coordinate wraps, both diagonals and distance modes.");

var regionVersion = typeof(RecompOne.Runtime.Sdk.V82Compat).GetMethod("TerrainRegionVersion",
    System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static)!;
var memory = new RecompOne.Runtime.Memory.PSMemory();
long Version(uint address = 0x10000) => (long)regionVersion.Invoke(null, [memory,address,64])!;
void NextFrame() => RecompOne.Runtime.Sdk.V82Compat.BeginSharedTerrainFrame();
NextFrame(); long initial = Version();
NextFrame(); if(Version()!=initial) throw new Exception("Unchanged source invalidated");
memory.WriteU32(0x20000,123);
NextFrame(); if(Version()!=initial) throw new Exception("Unrelated write invalidated source");
memory.WriteU16(0x10002,0x1234);
NextFrame(); long changed=Version(); if(changed==initial) throw new Exception("Height/texture edit missed");
memory.LoadBytes(0x10000,new byte[64]);
NextFrame(); if(Version()==changed) throw new Exception("Bulk source replacement missed");
if(Version(0x30000)==Version()) throw new Exception("Distinct source pages share identity");
RecompOne.Runtime.Sdk.V82Compat.EndSharedTerrainFrame();
Console.WriteLine("PASS source snapshot invalidation: stable frames, unrelated writes, direct edits, bulk replacement and page identity.");
