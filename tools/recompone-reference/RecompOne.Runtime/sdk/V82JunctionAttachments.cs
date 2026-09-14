using System.Numerics;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime.Sdk;

// Retain the authored road/patch connections in enhanced geometry. LOAD8010719C
// chooses fractional child attachments for road strips; LOAD801079A0 separately
// quantizes the patch vertices. Their native pixel snapping hides small gaps
// which become visible when Enhanced retains subpixel projection.
// The native JUNC mesh and RSEG gameplay records are never modified.
public static class V82JunctionAttachments
{
    static readonly bool Enabled = Environment.GetEnvironmentVariable("RECOMPONE_V82_JUNCTION_ATTACHMENTS") != "0";
    public static IReadOnlyDictionary<(int,int,int), Vector3>? ActiveOffsets { get; private set; }
    static readonly Stack<(uint Mesh, IReadOnlyDictionary<(int,int,int), Vector3>? Offsets)> Scopes = [];
    static readonly Dictionary<uint, (uint Mesh, IReadOnlyDictionary<(int,int,int), Vector3>? Offsets)> Cache = [];
    static int traces;
    public static void Reset() { Scopes.Clear(); Cache.Clear(); ActiveOffsets=null; traces=0; }
    public static void Enter(IMemory m, uint node)
    {
        if (!Enabled || node==0 || !GpuHle.Active || !GpuHle.GameplayActive) { Scopes.Push(default); return; }
        if (!Cache.TryGetValue(node,out var entry))
        {
            entry=(m.ReadU32(node+0x18),Build(m,node));
            if(Cache.Count>=128)Cache.Clear();
            Cache[node]=entry;
        }
        Scopes.Push(entry);
    }
    public static void Exit() { if(Scopes.Count>0)Scopes.Pop(); }
    public static IReadOnlyDictionary<(int,int,int), Vector3>? BeginMesh(uint mesh)
    {
        var previous=ActiveOffsets;
        ActiveOffsets=Scopes.TryPeek(out var entry)&&entry.Mesh==mesh ? entry.Offsets : null;
        return previous;
    }
    public static void EndMesh(IReadOnlyDictionary<(int,int,int), Vector3>? previous)=>ActiveOffsets=previous;
    static int I(IMemory m,uint p)=>unchecked((int)m.ReadU32(p));
    static short S(IMemory m,uint p)=>unchecked((short)m.ReadU16(p));
    static List<(int A,int B)> Boundary(IMemory m,uint mesh,int vertexCount)
    {
        // Read the exact face stream consumed by 80021F70: the native loader
        // has converted vertex indices to eight-byte offsets and moved the
        // packet dispatch kind into command bits 26..29.
        int faces=m.ReadU16(mesh+6);
        if(faces>16384)return [];
        uint p=m.ReadU32(mesh+0x10);
        ReadOnlySpan<int> sizes=[12,32,20,32,12,24,12,24,16,28,12,28,24,24,0,24];
        var counts=new Dictionary<(int,int),int>();
        for(int i=0;i<faces;i++)
        {
            int kind=(m.ReadU8(p+3)>>2)&15,size=sizes[kind];
            if(size==0)return [];
            if(kind==10){p+=(uint)(size+m.ReadU16(p+10)*8);continue;}
            int a=m.ReadU16(p+4),b=m.ReadU16(p+6),c=m.ReadU16(p+8);
            if(((a|b|c)&7)!=0)return [];
            a>>=3;b>>=3;c>>=3;
            if(a>=vertexCount||b>=vertexCount||c>=vertexCount)return [];
            if(a!=b&&b!=c&&c!=a){Add(a,b);Add(b,c);Add(c,a);}
            p+=(uint)size;
        }
        return counts.Where(e=>e.Value==1).Select(e=>e.Key).ToList();
        void Add(int a,int b){var edge=a<b?(a,b):(b,a);counts[edge]=counts.GetValueOrDefault(edge)+1;}
    }
    static IReadOnlyDictionary<(int,int,int),Vector3>? Build(IMemory m,uint node)
    {
        uint mesh=m.ReadU32(node+0x18),bank=m.ReadU32(node+0xC),bin=m.ReadU32(bank+4);
        int count=m.ReadU16(mesh+4),scale=m.ReadU8(mesh+1),slot=m.ReadU16(node+0x14),slots=I(m,bin+0x18);
        if(count<3||count>4096||scale>15||slot>=slots||slots>65535)return null;
        uint data=m.ReadU32(mesh+8);var verts=new (int X,int Y,int Z)[count];
        for(int i=0;i<count;i++){uint p=data+(uint)i*8;verts[i]=(S(m,p),S(m,p+2),S(m,p+4));}
        var boundary=Boundary(m,mesh,count);if(boundary.Count<3)return null;
        uint trig=0x80065C90u+((uint)m.ReadU16(node+0x16)&4095)*4;
        int sin=S(m,trig),cos=S(m,trig+2);float unit=MathF.ScaleB(1,16-scale);
        int child=m.ReadU16(bin+0x1Cu+(uint)slot*28+0x1A);var connections=new List<Vector2>();var visited=new HashSet<int>();
        while(child!=65535&&child<slots&&visited.Add(child))
        {
            uint p=bin+0x1Cu+(uint)child*28;
            // LOAD8010719C visits every authored child attachment. It rounds
            // the Q12 rotation toward zero, including negative coordinates.
            int x=unchecked(cos*I(m,p+4)+sin*I(m,p+12));
            int z=unchecked(-sin*I(m,p+4)+cos*I(m,p+12));
            var connection=new Vector2((x/4096)/unit,(z/4096)/unit);
            if(connection.LengthSquared()>0)connections.Add(connection);
            child=m.ReadU16(p+0x18);
        }
        var planes=new List<(Vector2 Point,Vector2 Normal,HashSet<int> Vertices)>();
        // JUNC embeds its edge pointers immediately after the 0x1C header.
        // LOAD801079A0 stores the allocated edge count as a byte at +0x11;
        // +0x12 is a separate optional field selected by flag 0x40.
        int edges=m.ReadU8(node+0x11);
        if(edges>64)return null;
        for(int i=0;i<edges;i++)
        {
            uint edge=m.ReadU32(node+0x1Cu+(uint)i*4);if(edge==0)continue;
            bool start=m.ReadU32(edge)==node;
            if(!start&&m.ReadU32(edge+4)!=node)continue;
            uint control=edge+(start?16u:24u);var direction=new Vector2(I(m,control)/unit,I(m,control+4)/unit);
            if(direction.LengthSquared()<1||connections.Count==0)continue;
            var point=connections.OrderByDescending(p=>Vector2.Dot(Vector2.Normalize(p),Vector2.Normalize(direction))).First();
            if(Vector2.Dot(point,direction)<=0)continue;
            var outward=direction-point;if(outward.LengthSquared()<1)continue;
            var normal=Vector2.Normalize(outward);
            // Two native coordinate quantizations (asset and rotated mesh)
            // bound the endpoint discrepancy; unrelated interior edges fail.
            const float uncertainty=2.8284272f;
            (int A,int B)? seed=null;float nearest=uncertainty;
            foreach(var edgePair in boundary)
            {
                var a=Position(edgePair.A);var b=Position(edgePair.B);var d=b-a;
                if(d.LengthSquared()==0)continue;
                float t=Vector2.Dot(point-a,d)/d.LengthSquared();
                float distance=Vector2.Distance(point,a+t*d);
                if(t>=0&&t<=1&&distance<=nearest){seed=edgePair;nearest=distance;}
            }
            if(seed is not { } first)continue;
            bool OnPlane(int v)=>MathF.Abs(Vector2.Dot(point-Position(v),normal))<=uncertainty;
            if(!OnPlane(first.A)||!OnPlane(first.B))continue;
            var connected=new HashSet<int>{first.A,first.B};bool changed;
            do
            {
                changed=false;
                foreach(var pair in boundary)
                    if((connected.Contains(pair.A)||connected.Contains(pair.B))&&OnPlane(pair.A)&&OnPlane(pair.B))
                    {changed|=connected.Add(pair.A);changed|=connected.Add(pair.B);}
            }while(changed);
            planes.Add((point,normal,connected));
        }
        var offsets=new Dictionary<(int,int,int),Vector3>();
        for(int i=0;i<verts.Length;i++)
        {
            var v=verts[i];
            var p=new Vector2(v.X,v.Z);var delta=Vector2.Zero;
            foreach(var plane in planes)
            {
                float distance=Vector2.Dot(plane.Point-p,plane.Normal);
                if(plane.Vertices.Contains(i))delta+=plane.Normal*distance;
            }
            if(delta.LengthSquared()>1e-10f)offsets[v]=new(delta.X,0,delta.Y);
        }
        if(traces++<32)Console.Error.WriteLine($"[JunctionAttachments] node={node:X8} slot={slot} vertices={count} children={connections.Count} edges={edges} planes={planes.Count} corrected={offsets.Count}");
        return offsets;
        Vector2 Position(int index)=>new(verts[index].X,verts[index].Z);
    }
}
