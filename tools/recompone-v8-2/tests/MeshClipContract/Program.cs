using System.Reflection;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;

// Real runtime memory/provenance and GTE, no game, graphics or audio launch.
var submit = typeof(V82MeshClipCompat).GetMethod("SubmitExactTriangle",
    BindingFlags.NonPublic | BindingFlags.Static)!;
const uint scratch = 0x1F800000, packet = 0x10000, table = 0x20000;
int checks = 0;
void Check(bool value, string message)
{
    if (!value) throw new Exception(message);
    checks++;
}
(PSMemory m, CpuContext c, PreciseGteVertexData[] vertices) Fixture(uint kind, float[] depths)
{
    var m = new PSMemory();
    var c = new CpuContext { A0 = packet, A1 = scratch, SP = 0x30000, RA = 0x12345678 };
    Gte.WriteControl(29, 0x555); // ZSF3
    m.WriteU32(scratch + 4, table);
    m.WriteU16(scratch + 8, 0);
    m.WriteU16(scratch + 0xC, 0x7E46);
    m.WriteU16(scratch + 0xE, 0x413);
    m.WriteU32(scratch + 0x10, (kind << 24) | 0x123456);
    var vertices = new PreciseGteVertexData[3];
    for (int i = 0; i < 3; i++)
    {
        uint source = scratch + 0x14u + (uint)i * 0x18;
        uint packed = (uint)(ushort)(-300 + i * 10) | ((uint)(100 + i * 20) << 16);
        ushort depth = (ushort)Math.Clamp(depths[i], 0, 65535);
        vertices[i] = new(packed, depth, depths[i], -180 + i * 20, i * 4,
            depths[i], -300 + i * 10, 100 + i * 20, 160, 120, 256);
        m.WritePreciseGteVertex(source + 0x10, vertices[i]);
        m.WriteU32(source + 0x14, depth);
        m.WriteU16(source + 8, (ushort)(0xAA00 + i * 20));
        m.WriteU32(source + 0xC, 0x445566u + (uint)i);
    }
    return (m, c, vertices);
}
bool Submit(CpuContext c, IMemory m) => (bool)submit.Invoke(null, [c, m])!;
foreach (uint kind in new uint[] {0x20, 0x24, 0x30, 0x34})
foreach (float[] depths in new[] {new float[] {40, 90, 260}, new float[] {180, 200, 240}, new float[] {-40, 30, 180}})
{
    var (m, c, vertices) = Fixture(kind, depths);
    Check(!Submit(c, m), $"{kind:X2}: must submit exact visible/crossing triangle");
    bool textured = (kind & 4) != 0, gouraud = (kind & 16) != 0;
    uint stride = (gouraud ? 8u : 4u) + (textured ? 4u : 0u);
    uint words = kind switch {0x20 => 4, 0x24 => 7, 0x30 => 6, _ => 9};
    Check(m.ReadU32(packet) >> 24 == words, "packet length");
    Check(c.V0 == packet + 4 * (words + 1) && c.A0 == c.V0, "native return pointer");
    Check(c.SP == 0x30000 && c.RA == 0x12345678, "caller stack and return address preserved");
    uint bucket = Math.Max(1, Gte.Read(7));
    Check(m.ReadU32(table + bucket * 4) == packet, "native ordering table link");
    for (int i = 0; i < 3; i++)
    {
        uint xy = packet + 8u + (uint)i * stride;
        Check(m.TryGetPreciseGteVertex(xy, m.ReadU32(xy), out var actual) && actual == vertices[i],
            "exact source camera coordinates retained, including behind-camera endpoints");
        if (textured)
            Check(m.ReadU16(xy + 4) == 0xAA00 + i * 20, "authored UV retained");
        if (gouraud)
            Check((m.ReadU32(xy - 4) & 0xFFFFFF) == 0x445566 + i, "authored Gouraud color retained");
    }
    Check(m.ReadU32(packet + 4) >> 24 == kind, "native command retained");
    if (textured)
    {
        Check(m.ReadU16(packet + 14) == 0x7E46, "CLUT retained");
        Check(m.ReadU16(packet + 8 + stride + 6) == 0x413, "tpage/blend flags retained");
    }
}
{
    var (m, c, _) = Fixture(0x24, [-40, -20, 0]);
    Check(!Submit(c, m) && c.V0 == packet && m.ReadU32(packet) == 0, "fully behind rejected without packet");
}
{
    var (m, c, _) = Fixture(0x24, [20, 50, 100]);
    m.WriteU32(scratch + 0x3C, 0xBAD); // invalidate exact address/value provenance
    Check(Submit(c, m) && m.ReadU32(packet) == 0, "incomplete provenance falls back without partial output");
}
{
    var (m, c, v) = Fixture(0x24, [20, 50, 100]);
    m.WritePreciseGteVertex(scratch + 0x3C, v[1] with { ProjectionScale = 128 });
    Check(Submit(c, m) && m.ReadU32(packet) == 0, "mixed camera projections fall back");
}
{
    HleVertex V(float x, float y, float z = 100) => new()
    {
        ViewX = x,
        ViewY = y,
        ViewZ = z,
        ProjectionCenterX = 160,
        ProjectionCenterY = 120,
        ProjectionScale = 256,
        HasViewSpace = true,
    };
    Check(!EnhancedGlBackend.Ps1CullBackFace(
            V(0, 0), V(10, 0), V(0, 10)),
        "PS1 scenery retains positive screen-space area");
    Check(EnhancedGlBackend.Ps1CullBackFace(
            V(0, 0), V(0, 10), V(10, 0)),
        "PS1 scenery removes negative screen-space area");
    Check(!EnhancedGlBackend.Ps1CullBackFace(
            V(0, 0), V(5, 5), V(10, 10)),
        "PS1 scenery leaves degenerate faces unchanged");
    // Hoover rib_1 packets 0/1 are the nearer exterior X=40 wall;
    // packets 2/3 are its farther X=-39 wall (native tick 301 capture).
    HleVertex[] near = [V(-734, -236.2358f, 958.9451f),
        V(-643, -236.2358f, 958.9451f), V(-640, -110.1660f, 974.2930f),
        V(-766, -50.6055f, 981.5439f)];
    HleVertex[] far = [V(-734, -245.7830f, 1037.3665f),
        V(-766, -60.1526f, 1059.9653f), V(-640, -119.7131f, 1052.7144f),
        V(-643, -245.7830f, 1037.3665f)];
    foreach (float advance in new[] {0f, 200f, 600f})
    {
        HleVertex Shift(HleVertex v) { v.ViewZ -= advance; return v; }
        Check(!EnhancedGlBackend.Ps1CullBackFace(Shift(near[0]), Shift(near[1]), Shift(near[2])) &&
              !EnhancedGlBackend.Ps1CullBackFace(Shift(near[2]), Shift(near[3]), Shift(near[0])),
            "Hoover nearer exterior wall survives camera approach");
        Check(EnhancedGlBackend.Ps1CullBackFace(Shift(far[0]), Shift(far[1]), Shift(far[2])) &&
              EnhancedGlBackend.Ps1CullBackFace(Shift(far[2]), Shift(far[3]), Shift(far[0])),
            "Hoover farther inward-facing wall is rejected during approach");
    }
    Check(EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.WaterBase),
        "Dreamcast untextured water pass disables face culling");
    Check(EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.WaterSurface),
        "Dreamcast textured water pass disables face culling");
    Check(!EnhancedGlBackend.DreamcastDisablesFaceCulling(
            HleMaterialKind.Opaque),
        "ordinary opaque material does not inherit the water no-cull rule");
}
{
    // Unproject each water vertex using its submitted depth. Every result
    // must lie on the authored plane, independently of viewport resolution.
    foreach (int projection in new[] { 128, 256, 512 })
    foreach (var normal in new[] { (X: (short)0, Y: (short)4066, Z: (short)495),
                                  (X: (short)4066, Y: (short)495, Z: (short)0) })
    {
        int width = projection * 5 / 3, height = projection * 15 / 16;
        const int delta = 137573;
        var plane = V82Compat.BuildDreamcastWaterBaseQuad(
            normal.X, normal.Y, normal.Z, delta, projection, width, height);
        foreach (var vertex in new[] { plane.V0, plane.V1, plane.V2, plane.V3 })
        {
            float z = vertex.CameraDepth / 256f;
            float x = (vertex.X - width * 0.5f) * z / projection;
            float y = (vertex.Y - height * 0.5f) * z / projection;
            float distance = (normal.X * x + normal.Y * y + normal.Z * z) / 4096f;
            Check(MathF.Abs(distance - delta / 65536f) < 0.0001f,
                "water raster depth and projected position meet the same world plane");
        }
    }
}
{
    // Captured from the shared PS1 base-water function on Louisiana. The
    // expected vertices are the direct algebra and fixed reciprocal-depth
    // planes decoded from Dreamcast 8C0D72A0, not values from this helper.
    var quad = V82Compat.BuildDreamcastWaterBaseQuad(
        0,
        4066,
        495,
        3092480 - 2954907,
        256,
        427,
        240);
    Check(MathF.Abs(quad.V0.X) < 0.001f &&
          MathF.Abs(quad.V2.X) < 0.001f,
        "Dreamcast horizontal water strip begins at viewport left");
    Check(MathF.Abs(quad.V1.X - 427f) < 0.001f &&
          MathF.Abs(quad.V3.X - 427f) < 0.001f,
        "Dreamcast horizontal water strip reaches viewport right");
    Check(MathF.Abs(quad.V0.Y - 132.89019f) < 0.001f &&
          MathF.Abs(quad.V1.Y - 132.89019f) < 0.001f,
        "Dreamcast first reciprocal-depth edge matches retail equation");
    Check(MathF.Abs(quad.V2.Y - 97.09473f) < 0.001f &&
          MathF.Abs(quad.V3.Y - 97.09473f) < 0.001f,
        "Dreamcast second reciprocal-depth edge matches retail equation");
    Check(MathF.Abs(quad.V0.CameraDepth - 3145.728f) < 0.001f &&
          MathF.Abs(quad.V2.CameraDepth - 16777.217f) < 0.01f,
        "Dreamcast reciprocal water depths convert to PS1 camera units");
}
{
    var quad = V82Compat.BuildDreamcastWaterBaseQuad(
        4096,
        0,
        0,
        0,
        256,
        320,
        240);
    Check(MathF.Abs(quad.V0.Y) < 0.001f &&
          MathF.Abs(quad.V2.Y) < 0.001f &&
          MathF.Abs(quad.V1.Y - 240f) < 0.001f &&
          MathF.Abs(quad.V3.Y - 240f) < 0.001f,
        "Dreamcast vertical water branch spans the viewport height");
    Check(MathF.Abs(quad.V0.X - 160f) < 0.001f &&
          MathF.Abs(quad.V3.X - 160f) < 0.001f,
        "Dreamcast vertical water branch retains the decoded centre");
}
{
    // Exact Louisiana camera installed by the retail PS1 water routine. The
    // coordinate seam was independently reproduced from the traced fixed-
    // point transform before this Dreamcast XWAT mesh builder was added.
    short[] cameraToWorld =
    [
        -2255, -414, 3394,
        0, 4066, 495,
        -3420, 272, -2239,
    ];
    var mesh = V82Compat.BuildDreamcastWaterSurfaceMesh(
        0,
        4066,
        495,
        137573,
        256,
        427,
        240,
        cameraToWorld,
        57564424,
        2954907,
        82733044,
        150);
    Check(mesh != null, "Dreamcast XWAT mesh exists for traced Louisiana camera");
    Check(mesh!.Rows is > 0 and <= 31 && mesh.Columns is > 0 and <= 31,
        "Dreamcast XWAT cell spans retain the decoded 31-cell caps");
    Check(mesh.Vertices.Length == mesh.Rows * mesh.Columns * 6,
        "Dreamcast XWAT row strips triangulate every rectangular cell");
    Check(mesh.Vertices.All(vertex =>
            float.IsFinite(vertex.X) &&
            float.IsFinite(vertex.Y) &&
            float.IsFinite(vertex.CameraDepth) &&
            float.IsFinite(vertex.U) &&
            float.IsFinite(vertex.V)),
        "Dreamcast XWAT projection and centered-slope UVs remain finite");
    Check(mesh.Vertices.All(vertex => vertex.CameraDepth >= 1f),
        "Dreamcast XWAT vertices retain positive host camera depth");
    var repeated = V82Compat.BuildDreamcastWaterSurfaceMesh(
        0, 4066, 495, 137573, 256, 427, 240, cameraToWorld,
        57564424, 2954907, 82733044, 150);
    Check(repeated != null && mesh.Vertices.SequenceEqual(repeated.Vertices),
        "Dreamcast XWAT mesh is deterministic for one retail camera/tick");
}
{
    // The panorama emitter links static packets into the far OT bucket;
    // scenery advances the dynamic packet cursor. Their lifetimes differ.
    var m = new PSMemory();
    var c = new CpuContext { GP = 0x40000 };
    const uint ot = 0x50000, oldPacket = 0x60000;
    const uint skyA = 0x61000, skyB = 0x62000, backdrop = 0x63000;
    m.WriteU32(c.GP + 0xCE0, ot);
    m.WriteU32(ot + 0x3FFC, oldPacket);
    m.WriteU32(c.GP + 0x610, 0x70000);
    GpuHle.ClearPacketOwners();
    V82Compat.BeginSceneryPass(c, m);
    m.WriteU32(c.GP + 0x610, 0x70100);
    V82Compat.EndSceneryPass(c, m);
    Check(!GpuHle.IsSkyPacket(0x70040),
        "scenery packets must never inherit panorama edge extension");
    V82Compat.BeginLatePass(c, m);
    m.WriteU32(skyA, (9u << 24) | oldPacket);
    m.WriteU32(skyB, (9u << 24) | skyA);
    m.WriteU32(backdrop, (11u << 24) | skyB);
    m.WriteU32(ot + 0x3FFC, backdrop);
    V82Compat.EndLatePass(c, m);
    Check(GpuHle.IsSkyPacket(skyA) && GpuHle.IsSkyPacket(skyB),
        "both static panorama packets belong to the actual emitter");
    Check(!GpuHle.IsSkyPacket(oldPacket),
        "panorama provenance stops at the previous ordering-table head");
    Check(!GpuHle.IsSkyPacket(0x70040),
        "panorama provenance does not include dynamic scenery arena");
    GpuHle.ClearPacketOwners();
}
{
    ushort[] vram = new ushort[1024 * 512];
    for (int i = 0; i < vram.Length; i++)
        vram[i] = (ushort)((i * 7919) ^ (i >> 9));
    foreach (int x in new[] { -1, 0, 1, 62, 63, 64, 1000, 1023, 1024 })
    foreach (int y in new[] { -1, 0, 63, 64, 500, 511, 512 })
    foreach (var size in new[] { (2, 1), (1, 64), (8, 8), (64, 1) })
    {
        var region = VramReadbackRegion.ForRead(x, y, size.Item1, size.Item2);
        ushort[] pixels = new ushort[region.Width * region.Height];
        for (int row = 0; row < region.Height; row++)
            vram.AsSpan((region.Y + row) * 1024 + region.X, region.Width)
                .CopyTo(pixels.AsSpan(row * region.Width));
        ushort[] actual = new ushort[size.Item1 * size.Item2];
        ushort[] expected = new ushort[actual.Length];
        region.CopyTo(pixels, x, y, size.Item1, size.Item2, actual);
        VramReadbackRegion.Full.CopyTo(vram, x, y, size.Item1, size.Item2, expected);
        Check(region.Contains(x, y, size.Item1, size.Item2) && actual.SequenceEqual(expected),
            $"regional VRAM read preserves every word at {x},{y} size {size}");
    }
    var tile = VramReadbackRegion.ForRead(64, 64, 2, 1);
    Check(tile.Width * tile.Height == 4096 && tile.Contains(65, 65, 2, 1),
        "small VRAM reads reuse a 4096-pixel window rather than downloading 524288 pixels");
    Check(!tile.Contains(63, 64, 2, 1) && !tile.Contains(127, 64, 2, 1),
        "requests outside the cached VRAM window must refill it");
}
// Regression: the imported tower's planar face is not a parallelogram.
// Both triangle halves must map their shared diagonal intersection to the
// texture center, independent of camera rotation/translation and depth.
{
    System.Numerics.Vector3[] face = [new(-39,-127,-3), new(-39,-127,-94),
        new(-39,60,-126), new(-39,0,0)];
    float[] q = new float[4];
    Check(PlanarQuadMapping.TryWeights(face, q), "tower planar mapping");
    var a = face[2]-face[0]; var b = face[3]-face[1];
    var n = System.Numerics.Vector3.Cross(a,b);
    float s = System.Numerics.Vector3.Dot(System.Numerics.Vector3.Cross(face[1]-face[0],b),n)/n.LengthSquared();
    float t = System.Numerics.Vector3.Dot(System.Numerics.Vector3.Cross(face[1]-face[0],a),n)/n.LengthSquared();
    Check(MathF.Abs(s-0.5f)>0.05f, "old per-triangle mapping exposes real tower seam");
    Check(MathF.Abs(s*q[2]/((1-s)*q[0]+s*q[2])-0.5f)<0.00001f, "first diagonal maps to UV center");
    Check(MathF.Abs(t*q[3]/((1-t)*q[1]+t*q[3])-0.5f)<0.00001f, "second diagonal maps to UV center");
    var rotation=System.Numerics.Quaternion.CreateFromYawPitchRoll(0.6f,0.12f,0.2f);
    var transformed=face.Select(p=>System.Numerics.Vector3.Transform(p,rotation)+new System.Numerics.Vector3(-700,-200,1050)).ToArray();
    float[] rotatedQ=new float[4];
    Check(PlanarQuadMapping.TryWeights(transformed,rotatedQ), "rotated translated planar face");
    for(int i=0;i<4;i++) Check(MathF.Abs(q[i]-rotatedQ[i])<0.00001f, "camera-independent UV weights");
    System.Numerics.Vector3[] rectangle=[new(0,0,100),new(5,0,100),new(5,9,100),new(0,9,100)];
    Check(PlanarQuadMapping.TryWeights(rectangle,rotatedQ) && rotatedQ.All(x=>x==1f), "parallelogram exactly preserves existing interpolation");
    rectangle[2]=new(-2,-2,100);
    Check(!PlanarQuadMapping.TryWeights(rectangle,rotatedQ), "concave face rejected");
    face[3].X += 10;
    Check(!PlanarQuadMapping.TryWeights(face,q), "nonplanar face rejected");
    Check(!PlanarQuadMapping.TryWeights(new System.Numerics.Vector3[4],q), "degenerate face rejected");
}
// Exercise the backend's actual pairing/writeback without creating a GL
// context. A shared edge must receive identical Q in both emitted triangles.
{
    var backendType=typeof(EnhancedGlBackend);
    var vertexType=backendType.GetNestedType("GlVertex",BindingFlags.NonPublic)!;
    var candidateField=backendType.GetField("_quadCandidates",BindingFlags.Instance|BindingFlags.NonPublic)!;
    var verticesField=backendType.GetField("_verts",BindingFlags.Instance|BindingFlags.NonPublic)!;
    var pairMethod=backendType.GetMethod("CorrectWorldQuad",BindingFlags.Instance|BindingFlags.NonPublic)!;
    (object Backend,Array Vertices) PairFixture(bool nonplanar)
    {
        var backend=System.Runtime.CompilerServices.RuntimeHelpers.GetUninitializedObject(backendType);
        var vertices=Array.CreateInstance(vertexType,6);
        (float Y,float Z,float U,float V)[] data=[(-127,-94,26,0),(60,-126,26,39),(0,0,0,39),
            (0,0,0,39),(-127,-3,0,0),(-127,-94,26,0)];
        for(int i=0;i<6;i++)
        {
            var v=Activator.CreateInstance(vertexType)!;
            void Set(string name,object value)=>vertexType.GetField(name)!.SetValue(v,value);
            Set("ViewX",nonplanar && i==4 ? -29f : -39f);
            Set("ViewY",data[i].Y); Set("ViewZ",data[i].Z+1000f);
            Set("U",data[i].U); Set("V",data[i].V);
            Set("HasViewSpace",2f); Set("PerspectiveW",data[i].Z+1000f);
            vertices.SetValue(v,i);
        }
        verticesField.SetValue(backend,vertices);
        candidateField.SetValue(backend,Activator.CreateInstance(candidateField.FieldType));
        pairMethod.Invoke(backend,[0]); pairMethod.Invoke(backend,[3]);
        return (backend,vertices);
    }
    float W(Array vertices,int i)=>(float)vertexType.GetField("PerspectiveW")!.GetValue(vertices.GetValue(i))!;
    var valid=PairFixture(false);
    Check(Enumerable.Range(0,6).All(i=>W(valid.Vertices,i)<0), "backend writes projective Q to both halves");
    Check(W(valid.Vertices,0)==W(valid.Vertices,5) && W(valid.Vertices,2)==W(valid.Vertices,3), "backend shared-edge Q agrees exactly");
    var invalid=PairFixture(true);
    Check(Enumerable.Range(0,6).All(i=>W(invalid.Vertices,i)>0), "backend preserves a nonplanar authored crease");
}
{
    // Secret Base warehouse header: an intermediate point on a straight
    // bottom edge uses U84 although its source position lies at U79.95.
    var a = new System.Numerics.Vector3(-856, -31, 1) / 16f;
    var b = new System.Numerics.Vector3(-856, -30, -575) / 16f;
    var mid = new System.Numerics.Vector3(-856, -31, -410) / 16f;
    Check(RecompOne.Runtime.Enhanced.PlanarQuadMapping.TryStraightEdgeUv(a,b,mid,
        new(0,22),new(112,22),new(84,22),out var uv), "split planar texture edge recognized");
    Check(MathF.Abs(uv.X - 79.9164f) < 0.01f && uv.Y == 22,
        "source edge subdivision receives continuous UV");
    Check(!RecompOne.Runtime.Enhanced.PlanarQuadMapping.TryStraightEdgeUv(a,b,mid + new System.Numerics.Vector3(1,0,0),
        new(0,22),new(112,22),new(84,22),out _), "real bent edge preserved");
    Check(!RecompOne.Runtime.Enhanced.PlanarQuadMapping.TryStraightEdgeUv(a,b,mid,
        new(0,22),new(112,22),new(84,30),out _), "authored non-straight UV edge preserved");
    Check(PlanarQuadMapping.TryStraightEdgeUv(
        new(473,-209.3257f,604.5232f), new(-102,-208.3330f,604.6440f), new(62,-209.3257f,604.5232f),
        new(40,22),new(152,22),new(129,22),out var capturedUv) && MathF.Abs(capturedUv.X-120.057f)<0.01f,
        "actual Secret Base camera-space edge tolerates authored integer quantization");
}
{
    var type = typeof(EnhancedGlBackend);
    var vt = type.GetNestedType("GlVertex", BindingFlags.NonPublic)!;
    var backend = System.Runtime.CompilerServices.RuntimeHelpers.GetUninitializedObject(type);
    var vertices = Array.CreateInstance(vt, 6);
    (float Y,float Z,float U,float V)[] data = [(-31,-410,84,22),(-30,-575,112,22),(-112,-575,112,0),
        (-31,1,0,22),(-31,-410,84,22),(-112,-575,112,0)];
    for (int i=0; i<6; i++)
    {
        var v = Activator.CreateInstance(vt)!;
        void Set(string field, object value) => vt.GetField(field)!.SetValue(v,value);
        Set("ViewX", -856f/16); Set("ViewY",data[i].Y/16); Set("ViewZ",data[i].Z/16+1000);
        Set("U",data[i].U); Set("V",data[i].V); Set("HasViewSpace",2f); Set("PerspectiveW",1000f);
        float minU=i<3 ? 84 : 0;
        Set("UvMinX",minU); Set("UvMaxX",112f); Set("UvMinY",0f); Set("UvMaxY",22f);
        Set("ReplacementX",2+minU*2); Set("ReplacementY",5414f);
        Set("ReplacementW",(113-minU)*2); Set("ReplacementH",46f);
        vertices.SetValue(v,i);
    }
    type.GetField("_verts",BindingFlags.Instance|BindingFlags.NonPublic)!.SetValue(backend,vertices);
    var method=type.GetMethod("CorrectSplitTextureEdge",BindingFlags.Instance|BindingFlags.NonPublic)!;
    method.Invoke(backend,[0]); method.Invoke(backend,[3]);
    float U(int i)=>(float)vt.GetField("U")!.GetValue(vertices.GetValue(i))!;
    Check(MathF.Abs(U(0)-79.9164f)<0.01f && U(0)==U(4),
        "backend repairs both copies of the warehouse split edge");
    Check(U(1)==112 && U(2)==112 && U(3)==0 && U(5)==112,
        "backend retains all surrounding texture corners");
    Check(Enumerable.Range(0,6).All(i =>
        (float)vt.GetField("ReplacementX")!.GetValue(vertices.GetValue(i))! == 2 &&
        (float)vt.GetField("ReplacementW")!.GetValue(vertices.GetValue(i))! == 226 &&
        (float)vt.GetField("UvMinX")!.GetValue(vertices.GetValue(i))! == 0),
        "different crops of one texture acquire a shared window without clamping the corrected edge");
    // OT playback may reverse the source triangle sequence.
    for (int i=0; i<6; i++)
    {
        int source=(i+3)%6;
        var v=vertices.GetValue(source)!;
        vt.GetField("U")!.SetValue(v,data[source].U);
        vertices.SetValue(v,source);
    }
    var reversed=Array.CreateInstance(vt,6);
    for (int i=0; i<6; i++) reversed.SetValue(vertices.GetValue((i+3)%6),i);
    vertices=reversed;
    type.GetField("_verts",BindingFlags.Instance|BindingFlags.NonPublic)!.SetValue(backend,vertices);
    type.GetField("_lastSplitTextureEdge",BindingFlags.Instance|BindingFlags.NonPublic)!.SetValue(backend,-1);
    method.Invoke(backend,[0]); method.Invoke(backend,[3]);
    Check(MathF.Abs(U(1)-79.9164f)<0.01f && U(1)==U(3),
        "backend repairs the split edge with reversed OT submission order");
    // Two repeated corners in either triangle are not two distinct shared
    // endpoints. In particular, repeated index 2 used to produce index -1.
    vertices.SetValue(vertices.GetValue(5),0);
    vertices.SetValue(vertices.GetValue(5),1);
    type.GetField("_lastSplitTextureEdge",BindingFlags.Instance|BindingFlags.NonPublic)!.SetValue(backend,3);
    method.Invoke(backend,[3]);
    Check(true,"collapsed prior triangle cannot form an out-of-range shared edge");
    var swapped=Array.CreateInstance(vt,6);
    for(int i=0;i<6;i++) swapped.SetValue(vertices.GetValue((i+3)%6),i);
    type.GetField("_verts",BindingFlags.Instance|BindingFlags.NonPublic)!.SetValue(backend,swapped);
    method.Invoke(backend,[3]);
    Check(true,"collapsed current triangle cannot form an out-of-range shared edge");
}
{
    // A coarse native rejection must retain its authored subdivision data,
    // tied to the originating traversal rather than a later camera/buffer.
    RecompOne.Runtime.Config.ConfigManager.View.HighResolution3D = true;
    RecompOne.Runtime.Config.ConfigManager.View.LevelOfDetail = "Maximum";
    var m = new PSMemory();
    var c = new CpuContext { GP=0x80080000, A0=4, A1=4, A2=0x10000 };
    m.WriteU32(0x800B9470,0x80100000);
    var field=typeof(V82Compat).GetField("_culledTerrainCells",BindingFlags.Static|BindingFlags.NonPublic)!;
    V82Compat.TraceTerrainTraversalPolygon(c,m);
    V82Compat.BeginTerrainRoutePacketWrites(c,m);
    c.V0=c.A2;
    V82Compat.EndTerrainRoutePacketWrites(c,m);
    var cells=(List<GpuHle.CoarseTerrainPacket>)field.GetValue(null)!;
    Check(cells.Count==1 && cells[0].X==4 && cells[0].Z==4,
        "rejected coarse cell retained for enhanced terrain clipping");
    Check(cells[0].Textures.DistanceColors.Patch?.Samples.Length==25,
        "rejected cell retains every authored terrain sample");
    Check(ReferenceEquals(cells[0].Textures.DistanceColors.Patch!.CulledCells,cells),
        "terrain snapshot owns its rejected-cell cohort");
    c.A0=8;
    V82Compat.BeginTerrainRoutePacketWrites(c,m);
    c.V0=c.A2+28;
    V82Compat.EndTerrainRoutePacketWrites(c,m);
    Check(cells.Count==1,"native emitted cells are not added to the repair list");
    V82Compat.TraceTerrainTraversalPolygon(c,m);
    Check(!ReferenceEquals(field.GetValue(null),cells) && cells.Count==1,
        "next traversal cannot mutate the previous buffer's retained cells");
}
{
    var type=typeof(EnhancedGlBackend);
    var backend=System.Runtime.CompilerServices.RuntimeHelpers.GetUninitializedObject(type);
    var method=type.GetMethod("V",BindingFlags.Instance|BindingFlags.NonPublic)!;
    GpuHle.GameplayActive=true;
    foreach (int margin in new[] {0,54})
    foreach (int origin in new[] {0,240})
    {
        var target=new GlDisplayRt { X=0, Y=origin, W=320, H=240, Margin=margin };
        var source=new HleVertex { X=160, Y=origin+120, ProjectionCenterX=160,
            ProjectionCenterY=origin+120, ProjectionScale=256, ViewZ=1000,
            HasViewSpace=true, HasProjectiveW=true, PerspectiveW=1000 };
        var flags=new PrimFlags { Material=HleMaterialKind.TerrainRoute };
        var result=method.Invoke(backend,[source,flags,false,true,false,0f,target])!;
        float Field(string name)=>(float)result.GetType().GetField(name)!.GetValue(result)!;
        Check(Field("ProjectionCenterX")+margin==target.Wide1x*0.5f &&
            Field("ProjectionCenterY")-origin==120,
            "world projection and deferred water share one target margin in both buffers");
    }
    GpuHle.GameplayActive=false;
}
{
    var m=new PSMemory(); var c=new CpuContext { GP=0x80080000 };
    var view=RecompOne.Runtime.Config.ConfigManager.View;
    bool oldHigh=view.HighResolution3D, oldWide=view.Widescreen;
    float oldAspect=GpuHle.WideAspect;
    view.HighResolution3D=true; view.Widescreen=true; GpuHle.GameplayActive=true;
    foreach(float aspect in new[] {16f/9f,21f/9f})
    foreach(uint width in new uint[] {320,512,640})
    {
        GpuHle.WideAspect=aspect;
        m.WriteU32(c.GP+0xEDC,width);
        uint targetWidth=width+(uint)(2*GpuHle.WideMargin((int)width));
        V82Compat.ExpandTerrainFrustum(c,m);
        Check(m.ReadU32(c.GP+0xEDC)==targetWidth,"water/terrain frustum exactly spans symmetric framebuffer margins");
        V82Compat.RestoreTerrainFrustum(c,m);
        Check(m.ReadU32(c.GP+0xEDC)==width,"terrain frustum restores authored width");
        V82Compat.ExpandObjectFrustum(c,m);
        Check(m.ReadU32(c.GP+0xEDC)==targetWidth,"objects use the same widened viewport as water");
        V82Compat.RestoreObjectFrustum(c,m);
        Check(m.ReadU32(c.GP+0xEDC)==width,"object frustum restores authored width");
    }
    GpuHle.WideAspect=oldAspect; GpuHle.GameplayActive=false;
    view.HighResolution3D=oldHigh; view.Widescreen=oldWide;
}
Console.WriteLine($"PASS: {checks} mesh clipping packet/provenance assertions");
