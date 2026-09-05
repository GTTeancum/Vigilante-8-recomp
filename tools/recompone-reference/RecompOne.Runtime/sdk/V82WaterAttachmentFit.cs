using System.Numerics;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Fits only the body-facing ends of the authored water support arms. Native
/// wheel contacts, flotation points, animation, and shared source vertices are
/// unchanged. Per-instance vertex copies exist only during packet generation.
/// </summary>
internal static class V82WaterAttachmentFit
{
    // Candidate remains opt-in until native visual verification is complete.
    static readonly bool Enabled = Environment.GetEnvironmentVariable("RECOMPONE_V82_WATER_ATTACHMENT_FIT") == "1";
    static readonly int[] PacketSizes = [12,32,20,32,12,24,12,24,16,28,12,28,24,24,0,24];
    readonly record struct Pose(Vector3 X, Vector3 Y, Vector3 Z, Vector3 Position)
    {
        public static Pose Identity => new(Vector3.UnitX,Vector3.UnitY,Vector3.UnitZ,Vector3.Zero);
        public Vector3 Vector(Vector3 v) => X*v.X+Y*v.Y+Z*v.Z;
        public Vector3 Point(Vector3 v) => Vector(v)+Position;
        public Pose Compose(Pose b) => new(Vector(b.X),Vector(b.Y),Vector(b.Z),Point(b.Position));
        public Vector3 InversePoint(Vector3 p)
        {
            var v=p-Position;
            float det=Vector3.Dot(X,Vector3.Cross(Y,Z));
            if (MathF.Abs(det)<0.001f) throw new InvalidDataException("Singular attachment transform");
            return new(Vector3.Dot(v,Vector3.Cross(Y,Z))/det,
                Vector3.Dot(v,Vector3.Cross(Z,X))/det,Vector3.Dot(v,Vector3.Cross(X,Y))/det);
        }
    }
    readonly record struct Triangle(Vector3 A,Vector3 B,Vector3 C);
    readonly record struct Node(uint Address,Pose Pose);
    internal sealed record Replacement(uint Mesh,uint Original,byte[] Vertices,int Changed);
    sealed record Cached(ulong Signature,List<Replacement> Replacements);
    static readonly Dictionary<uint,Cached> Cache=[];
    public static void Reset() => Cache.Clear();

    internal static bool IsSharedWaterSupport(IMemory m,uint node,uint bank,ushort slot)
    {
        // 8003E4A8 first prefers a vehicle-authored transformation override.
        // Only its fallback uses the common/registry transformation bank and
        // mode table. An authored steering/body assembly is not a ski arm.
        return Valid(node,0x80) && Valid(bank,0x1C) &&
            m.ReadU32(node+0x5C)==bank && m.ReadU16(node+0x1A)==slot;
    }

    // Read-only snapshot of the most recently generated render copies. This
    // is diagnostic evidence, not a request to rebuild or apply the fit.
    internal static object? DescribeCached(IMemory m,uint vehicle)
    {
        if (!Cache.TryGetValue(vehicle,out var cached)) return null;
        return new { signature=cached.Signature, replacements=cached.Replacements.Select(r=>new {
            mesh=r.Mesh, original=r.Original, currentSource=m.ReadU32(r.Mesh+8),
            scaleShift=m.ReadU8(r.Mesh+1), changed=r.Changed,
            originalVertices=Enumerable.Range(0,r.Vertices.Length/8).Select(i=>new[]{
                (int)(short)m.ReadU16(r.Original+(uint)i*8),
                (int)(short)m.ReadU16(r.Original+(uint)i*8+2),
                (int)(short)m.ReadU16(r.Original+(uint)i*8+4)}).ToArray(),
            fittedVertices=Enumerable.Range(0,r.Vertices.Length/8).Select(i=>new[]{
                (int)BitConverter.ToInt16(r.Vertices,i*8),
                (int)BitConverter.ToInt16(r.Vertices,i*8+2),
                (int)BitConverter.ToInt16(r.Vertices,i*8+4)}).ToArray()
        }).ToArray(), note="Most recent cached render copies before this gameplay tick; native pointers must be restored." };
    }

    internal sealed class Scope(CpuContext c,IMemory m,uint allocation,List<Replacement> replacements) : IDisposable
    {
        public void Dispose()
        {
            foreach (var replacement in replacements) m.WriteU32(replacement.Mesh+8,replacement.Original);
            var saved=c.Snapshot();
            try { c.A0=allocation; V82Compat.PcFree(c,m); }
            finally { c.Restore(saved); }
        }
    }

    public static Scope? Begin(CpuContext c,IMemory m,uint vehicle)
    {
        if (!Enabled || m.ReadU8(vehicle+0xAC)!=2 || m.ReadU16(vehicle+0xB4)!=0) return null;
        uint transformBank=V82VehicleRegistry.TransformBankForObject(m,vehicle,m.ReadU32(0x800C61C0));
        uint transformTable=V82VehicleRegistry.TransformTableForObject(m,vehicle,0x80063F74);
        if (!Valid(transformTable,36)) return null;
        var mounts=Enumerable.Range(0,6).Select(i=>new {
            Node=m.ReadU32(vehicle+0x104+(uint)i*4),
            Slot=m.ReadU16(transformTable+24+(uint)i*2)
        }).Where(p=>IsSharedWaterSupport(m,p.Node,transformBank,p.Slot)).Select(p=>p.Node).ToHashSet();
        if (mounts.Count==0) { Cache.Remove(vehicle); return null; }
        var nodes=new List<Node>();
        var seen=new HashSet<uint>();
        void Visit(uint address,Pose parent,int depth)
        {
            if (!Valid(address,0x80) || depth>24 || seen.Count>=256 || !seen.Add(address)) return;
            var pose=address==vehicle ? Pose.Identity : parent.Compose(ReadPose(m,address));
            nodes.Add(new(address,pose));
            uint child=m.ReadU32(address+0x10);
            for (int i=0;i<256 && Valid(child,0x80) && !seen.Contains(child);i++)
            {
                Visit(child,pose,depth+1);
                child=m.ReadU32(child+0xC);
            }
        }
        Visit(vehicle,Pose.Identity,0);
        uint bodyBank=m.ReadU32(vehicle+0x5C);
        // Invalidate on assembly/pose/damage changes, not world movement.
        ulong signature=14695981039346656037UL;
        void Mix(uint value) { signature=unchecked((signature^value)*1099511628211UL); }
        Mix(bodyBank); Mix(m.ReadU16(vehicle+0x1C));
        // Outboard ski pivots and unrelated weapon animation do not change
        // either endpoint's body target. Do not rebuild the fit for those.
        foreach (var node in nodes.Where(n=>mounts.Contains(n.Address) || m.ReadU32(n.Address+0x5C)==bodyBank))
        {
            Mix(node.Address); Mix(m.ReadU32(node.Address+0x40)); Mix(m.ReadU32(node.Address+0x68));
            if (node.Address!=vehicle)
                for (uint o=0x20;o<=0x3C;o+=4) Mix(m.ReadU32(node.Address+o));
        }
        if (!Cache.TryGetValue(vehicle,out var cached) || cached.Signature!=signature)
        {
            var triangles=new List<Triangle>();
            bool validBody=true;
            foreach (var node in nodes)
                if (!mounts.Contains(node.Address) && m.ReadU32(node.Address+0x5C)==bodyBank)
                    validBody &= ReadTriangles(m,m.ReadU32(node.Address+0x40),node.Pose,triangles);
            var replacements=new List<Replacement>();
            if (validBody && triangles.Count>0)
                foreach (var node in nodes.Where(n=>mounts.Contains(n.Address)))
                    foreach (uint mesh in new[] {m.ReadU32(node.Address+0x40),m.ReadU32(node.Address+0x68)}.Distinct())
                    {
                        var replacement=Fit(m,mesh,node.Pose,triangles);
                        if (replacement!=null) replacements.Add(replacement);
                    }
            cached=new(signature,replacements);
            if (Cache.Count>=256) Cache.Clear();
            bool first=!Cache.ContainsKey(vehicle);
            Cache[vehicle]=cached;
            if (first) Console.Error.WriteLine($"[WaterAttachmentFit] vehicle=0x{vehicle:X8} type={m.ReadU8(vehicle+0xDC)} body-valid={validBody} body-triangles={triangles.Count} meshes={replacements.Count} vertices={replacements.Sum(r=>r.Changed)}");
        }
        if (cached.Replacements.Count==0) return null;
        uint allocation;
        var saved=c.Snapshot();
        try { c.A0=(uint)cached.Replacements.Sum(r=>r.Vertices.Length); V82Compat.PcMalloc(c,m); allocation=c.V0; }
        finally { c.Restore(saved); }
        if (allocation==0) return null;
        uint cursor=allocation;
        foreach (var replacement in cached.Replacements)
        {
            for (int i=0;i<replacement.Vertices.Length;i++) m.WriteU8(cursor+(uint)i,replacement.Vertices[i]);
            m.WriteU32(replacement.Mesh+8,cursor);
            cursor+=(uint)replacement.Vertices.Length;
        }
        return new Scope(c,m,allocation,cached.Replacements);
    }

    static Replacement? Fit(IMemory m,uint mesh,Pose pose,List<Triangle> body)
    {
        if (!ReadVertices(m,mesh,pose,out var points,out uint source,out float unit)) return null;
        float inward=pose.Position.X<0 ? 1 : -1;
        float edge=points.Max(p=>p.X*inward);
        var edgePoints=points.Where(p=>p.X*inward>=edge-unit).ToArray();
        var center=edgePoints.Aggregate(Vector3.Zero,(sum,p)=>sum+p)/edgePoints.Length;
        var direction=new Vector3(inward,0,0);
        Vector3 anchor=center; float distance=float.MaxValue;
        // A support extends inward toward the chassis. Nearest-point fitting
        // can instead select an outboard bumper and shear the four corners
        // toward different body parts. Prefer the first inward body hit and
        // preserve the authored end face as one rigid cross-section.
        foreach (var triangle in body)
            if (RayHit(center,direction,triangle,out float travel) && travel<distance)
            { distance=travel; anchor=center+direction*travel; }
        if (distance==float.MaxValue)
            foreach (var triangle in body)
            {
                var nearest=Closest(center,triangle);
                float travel=Vector3.Distance(center,nearest);
                if (travel<distance) { distance=travel; anchor=nearest; }
            }
        if (distance<=unit || !float.IsFinite(distance)) return null;
        var offset=anchor-center;
        offset+=Vector3.Normalize(offset)*unit;
        var bytes=new byte[points.Length*8];
        for (int i=0;i<bytes.Length;i++) bytes[i]=m.ReadU8(source+(uint)i);
        int changed=0;
        for (int i=0;i<points.Length;i++)
        {
            if (points[i].X*inward<edge-unit) continue;
            // One raw-vertex quantum of overlap prevents quantization from
            // reopening a hairline seam. No authored per-vehicle offsets.
            var local=pose.InversePoint(points[i]+offset)/unit;
            var components=new[]{local.X,local.Y,local.Z};
            if (components.Any(v=>!float.IsFinite(v) || v<short.MinValue || v>short.MaxValue)) return null;
            for (int axis=0;axis<3;axis++)
            {
                short value=(short)MathF.Round(components[axis]);
                bytes[i*8+axis*2]=(byte)value; bytes[i*8+axis*2+1]=(byte)(value>>8);
            }
            changed++;
        }
        return changed==0 ? null : new(mesh,source,bytes,changed);
    }

    static bool RayHit(Vector3 origin,Vector3 direction,Triangle t,out float travel)
    {
        // Two-sided Moller-Trumbore: body winding must not change where an
        // attachment joins. No renderer culling or physics state is changed.
        travel=0;
        var ab=t.B-t.A; var ac=t.C-t.A;
        var p=Vector3.Cross(direction,ac);
        float determinant=Vector3.Dot(ab,p);
        if (MathF.Abs(determinant)<0.000001f) return false;
        float inverse=1/determinant;
        var relative=origin-t.A;
        float u=Vector3.Dot(relative,p)*inverse;
        if (u<0 || u>1) return false;
        var q=Vector3.Cross(relative,ab);
        float v=Vector3.Dot(direction,q)*inverse;
        if (v<0 || u+v>1) return false;
        travel=Vector3.Dot(ac,q)*inverse;
        return travel>=0 && float.IsFinite(travel);
    }

    static bool ReadTriangles(IMemory m,uint mesh,Pose pose,List<Triangle> triangles)
    {
        if (mesh==0) return true; // Native transform-only parent.
        if (!ReadVertices(m,mesh,pose,out var vertices,out _,out _)) return false;
        uint packet=m.ReadU32(mesh+0x10); int count=m.ReadU16(mesh+6);
        if (count>8192) return false;
        var parsed=new List<Triangle>();
        for (int i=0;i<count;i++)
        {
            if (!Valid(packet,32)) return false;
            // Native loader 8001EA0C..8001EA80 shifts the on-disk kind
            // left by two and converts vertex indices to 8-byte offsets.
            int kind=(m.ReadU8(packet+3)>>2)&15, size=PacketSizes[kind];
            if (size==0) return false;
            if (kind==10) size+=m.ReadU16(packet+10)*8;
            else
            {
                int a=m.ReadU16(packet+4),b=m.ReadU16(packet+6),d=m.ReadU16(packet+8);
                if (((a|b|d)&7)!=0) return false;
                a>>=3; b>>=3; d>>=3;
                if (a>=vertices.Length || b>=vertices.Length || d>=vertices.Length) return false;
                if (Vector3.Cross(vertices[b]-vertices[a],vertices[d]-vertices[a]).LengthSquared()>1)
                    parsed.Add(new(vertices[a],vertices[b],vertices[d]));
            }
            packet+=(uint)size;
        }
        triangles.AddRange(parsed);
        return true;
    }
    static bool ReadVertices(IMemory m,uint mesh,Pose pose,out Vector3[] points,out uint source,out float unit)
    {
        points=[]; source=0; unit=1;
        if (!Valid(mesh,0x1C)) return false;
        int count=m.ReadU16(mesh+4),shift=m.ReadU8(mesh+1);
        source=m.ReadU32(mesh+8);
        if (count==0 || count>4096 || shift>16 || !Valid(source,count*8)) return false;
        unit=1<<(16-shift); points=new Vector3[count];
        for (uint i=0;i<count;i++) points[i]=pose.Point(new Vector3(
            (short)m.ReadU16(source+i*8),(short)m.ReadU16(source+i*8+2),(short)m.ReadU16(source+i*8+4))*unit);
        return true;
    }
    static bool Valid(uint p,int length)=>p>=0x80010000 && p<0x80800000-length;
    static Pose ReadPose(IMemory m,uint p)=>new(
        new Vector3((short)m.ReadU16(p+0x20),(short)m.ReadU16(p+0x26),(short)m.ReadU16(p+0x2C))/4096,
        new Vector3((short)m.ReadU16(p+0x22),(short)m.ReadU16(p+0x28),(short)m.ReadU16(p+0x2E))/4096,
        new Vector3((short)m.ReadU16(p+0x24),(short)m.ReadU16(p+0x2A),(short)m.ReadU16(p+0x30))/4096,
        new Vector3((int)m.ReadU32(p+0x34),(int)m.ReadU32(p+0x38),(int)m.ReadU32(p+0x3C)));
    static Vector3 Closest(Vector3 p,Triangle t)
    {
        var ab=t.B-t.A; var ac=t.C-t.A; var ap=p-t.A;
        float d1=Vector3.Dot(ab,ap),d2=Vector3.Dot(ac,ap);
        if (d1<=0 && d2<=0) return t.A;
        var bp=p-t.B; float d3=Vector3.Dot(ab,bp),d4=Vector3.Dot(ac,bp);
        if (d3>=0 && d4<=d3) return t.B;
        float vc=d1*d4-d3*d2;
        if (vc<=0 && d1>=0 && d3<=0) return t.A+ab*d1/(d1-d3);
        var cp=p-t.C; float d5=Vector3.Dot(ab,cp),d6=Vector3.Dot(ac,cp);
        if (d6>=0 && d5<=d6) return t.C;
        float vb=d5*d2-d1*d6;
        if (vb<=0 && d2>=0 && d6<=0) return t.A+ac*d2/(d2-d6);
        float va=d3*d6-d5*d4;
        if (va<=0 && d4-d3>=0 && d5-d6>=0) return t.B+(t.C-t.B)*(d4-d3)/((d4-d3)+(d5-d6));
        return t.A+(ab*vb+ac*vc)/(va+vb+vc);
    }
}
