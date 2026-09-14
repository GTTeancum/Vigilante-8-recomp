using System.Numerics;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

// Camera-independent model preparation. Bytes, rather than model identity,
// validate the bounds, including deformation and reused heap addresses.
public static class V82ModelBounds
{
    static readonly bool Verify = Environment.GetEnvironmentVariable("RECOMPONE_VERIFY_MODEL_BOUNDS") == "1";
    record Probe(uint Start,uint Mesh,float Left,float Right,float Top,float Bottom);
    static readonly Stack<Probe?> Probes=[];
    static int Bad;
    static readonly Stack<bool> SkipFaces = [];
    public static void BeginGroup(CpuContext c, IMemory m) => SkipFaces.Push(Outside(c,m));
    public static void SkipInvisibleFaces(CpuContext c)
    {
        // Preserve native matrix/light setup and its shared GTE side effects.
        // Only the face traversal is skipped, through the original epilogue.
        if (SkipFaces.TryPeek(out bool skip) && skip) c.S5 = 0;
    }
    public static void EndGroup(CpuContext c, IMemory m)
    {
        if(SkipFaces.Count>0) SkipFaces.Pop();
        VerifyRendered(c,m);
    }
    static readonly bool Enabled = Environment.GetEnvironmentVariable("RECOMPONE_SHARED_MODEL_BOUNDS") != "0";
    sealed class Entry
    {
        public byte[] Source = [];
        public Vector3 Min, Max;
    }
    static readonly Dictionary<(uint, int), Entry> Cache = [];
    static PSMemory? Owner;
    static long Reused, Built, Rejected;
    static int CachedBytes;

    public static bool Outside(CpuContext c, IMemory m)
    {
        if (Verify) Probes.Push(null);
        if (!Enabled || !V82Compat.DirectSceneRendering || !GpuHle.GameplayActive ||
            V82Compat.ObjectRenderScopeDepth == 0 ||
            GpuHle.Backend is not Enhanced.EnhancedGlBackend { Ready: true } ||
            m is not PSMemory memory || RamLogger.TrackReads ||
            Runtime.Gpu is not { } gpu || !gpu.TryGetProjectionViewport(
                out float left, out float right, out float top, out float bottom, packetCoordinates: true)) return false;
        uint mesh = c.A0;
        int count = m.ReadU16(mesh + 4);
        int shift = m.ReadU8(mesh + 1);
        uint vertices = m.ReadU32(mesh + 8);
        uint physical = vertices & 0x1FFFFFFF;
        if (count <= 0 || count > 8192 || shift > 15 ||
            (ulong)physical + (uint)(count * 8) > (ulong)memory.Ram.Length) return false;
        var bounds = ReadBounds(memory, vertices, count);
        // Enhanced attachment endpoints can retain fractions discarded by the
        // native mesh. Include their actual positions in the visibility box.
        if (V82JunctionAttachments.ActiveOffsets is { } attachments)
            foreach (var (vertex, delta) in attachments)
            {
                var point = new Vector3(vertex.Item1, vertex.Item2, vertex.Item3) + delta;
                bounds.Min = Vector3.Min(bounds.Min, point);
                bounds.Max = Vector3.Max(bounds.Max, point);
            }
        Span<float> r = stackalloc float[9];
        for (int i = 0; i < 9; i++) r[i] = unchecked((short)m.ReadU16(c.A1 + (uint)i*2)) / 4096f;
        int tx = unchecked((int)m.ReadU32(c.A1+20)) >> (16-shift);
        int ty = unchecked((int)m.ReadU32(c.A1+24)) >> (16-shift);
        int tz = unchecked((int)m.ReadU32(c.A1+28)) >> (16-shift);
        Span<DreamcastTerrainGeometry.Sample> corners = stackalloc DreamcastTerrainGeometry.Sample[8];
        for (int i = 0; i < 8; i++)
        {
            float x = (i&1)==0 ? bounds.Min.X : bounds.Max.X;
            float y = (i&2)==0 ? bounds.Min.Y : bounds.Max.Y;
            float z = (i&4)==0 ? bounds.Min.Z : bounds.Max.Z;
            corners[i] = new(new(tx+r[0]*x+r[1]*y+r[2]*z,
                ty+r[3]*x+r[4]*y+r[5]*z, tz+r[6]*x+r[7]*y+r[8]*z), 0, default, default);
        }
        // Native exceptional projections can still produce legacy screen-space
        // packets. A 3D box does not describe their saturated raster footprint.
        // Keep that traversal unless the entire model is in the regular GTE range.
        float focal = Gte.ReadControl(26)&0xFFFF;
        foreach (var corner in corners)
            if (corner.View.Z <= focal*.5f) return false;
        bool outside = DreamcastTerrainGeometry.OutsideViewport(corners, default,
            unchecked((int)Gte.ReadControl(24))/65536f, unchecked((int)Gte.ReadControl(25))/65536f,
            Gte.ReadControl(26)&0xFFFF, left, right, top, bottom);
        if (outside && (++Rejected == 1 || Rejected % 10000 == 0))
            Console.Error.WriteLine($"[SharedModelBounds] rejected={Rejected} reused={Reused} rebuilt={Built}");
        if (Verify && outside)
        {
            Probes.Pop(); Probes.Push(new(m.ReadU32(c.GP+0x610),mesh,left,right,top,bottom));
            return false;
        }
        return outside;
    }
    public static void VerifyRendered(CpuContext c, IMemory m)
    {
        if (!Verify || Probes.Count==0 || Probes.Pop() is not { } probe) return;
        uint end=m.ReadU32(c.GP+0x610);
        for(uint p=probe.Start;p+8<=end;)
        {
            uint words=m.ReadU32(p)>>24;
            if(words==0 || p+(words+1)*4>end) break;
            var vs=new List<PreciseGteVertexData>();
            for(uint a=p+8;a<p+(words+1)*4;a+=4)
                if(m.TryGetPreciseGteVertex(a,m.ReadU32(a),out var v)) vs.Add(v);
            if(vs.Count>=3)
            {
                var points=vs.Select(v=>new DreamcastTerrainGeometry.Sample(new(v.ViewX,v.ViewY,v.ViewZ),0,default,default)).ToArray();
                var v=vs[0];
                if(!DreamcastTerrainGeometry.OutsideViewport(points,default,v.ProjectionCenterX,v.ProjectionCenterY,v.ProjectionScale,
                    probe.Left,probe.Right,probe.Top,probe.Bottom) && Bad++<12)
                    Console.Error.WriteLine($"[ModelBoundsMismatch] mesh={probe.Mesh:X8} flags={m.ReadU8(probe.Mesh):X2} packet={p:X8} command={m.ReadU32(p+4):X8} count={vs.Count} xy={v.ViewX},{v.ViewY},{v.ViewZ} projection={v.ProjectionCenterX},{v.ProjectionCenterY},{v.ProjectionScale}");
            }
            p+=(words+1)*4;
        }
    }

    internal static (Vector3 Min, Vector3 Max) ReadBounds(PSMemory memory, uint vertices, int count)
    {
        uint physical = vertices & 0x1FFFFFFF;
        if (count <= 0 || count > 8192 || (ulong)physical + (uint)(count*8) > (ulong)memory.Ram.Length)
            throw new ArgumentOutOfRangeException(nameof(count));
        if (!ReferenceEquals(Owner, memory) || Cache.Count > 2048 || CachedBytes > 2*1024*1024)
        { Cache.Clear(); CachedBytes=0; Owner = memory; }
        var key = (physical, count);
        if (!Cache.TryGetValue(key, out var entry)) { entry = new(); Cache.Add(key, entry); CachedBytes += count*8; }
        var bytes = memory.Ram.Slice((int)physical, count * 8);
        if (!bytes.SequenceEqual(entry.Source))
        {
            entry.Source = bytes.ToArray();
            entry.Min = new(float.MaxValue); entry.Max = new(float.MinValue);
            for (uint i = 0; i < count; i++)
            {
                uint p = vertices + i * 8;
                Vector3 v = new(unchecked((short)memory.ReadU16(p)), unchecked((short)memory.ReadU16(p+2)), unchecked((short)memory.ReadU16(p+4)));
                entry.Min = Vector3.Min(entry.Min, v); entry.Max = Vector3.Max(entry.Max, v);
            }
            Built++;
        }
        else Reused++;
        return (entry.Min, entry.Max);
    }
}
