// Synthetic submitted vertices and real Mesa render/VRAM readbacks. No game or media.
using System.Buffers.Binary;
using System.Collections;
using System.Numerics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text.Json;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

const int W = 320, H = 240, sideX = 704, sideY = 184, sideW = 64, sideH = 48;
const int textureX = 512;
const ushort texture = 0x108, nativeGreen = 0x83e0;
const BindingFlags Private = BindingFlags.Instance | BindingFlags.NonPublic;
var clipBounds = (9f,31f,247f,251f);
var smallBounds = (15f,22f,18f,26f);
int samples = args.Contains("--msaa4") ? 4 : 0;
bool diagnostics = args.Contains("--diagnostics");
int checks = 0;

Environment.SetEnvironmentVariable("RECOMPONE_TRACE_ENHANCED_PERFORMANCE", "1");
Environment.SetEnvironmentVariable("RECOMPONE_V82_NEAR_CLIP", "1");
Environment.SetEnvironmentVariable("RECOMPONE_ENHANCED_NEAR_PLANE", "1");
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_MODAL_RECTS", diagnostics ? "1" : null);
Environment.SetEnvironmentVariable("RECOMPONE_DIRECT_TERRAIN", "1");
Environment.SetEnvironmentVariable("RECOMPONE_DISABLE_PLANAR_QUAD_MAPPING", "1");
Environment.SetEnvironmentVariable("RECOMPONE_DISABLE_STRAIGHT_EDGE_MAPPING", "1");
var pack = Directory.CreateTempSubdirectory("v8-synthetic-triangle-");
MakePack(pack.FullName);
Environment.SetEnvironmentVariable("RECOMPONE_TEXTURE_PACK_DIR", pack.FullName);

GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with {
    Title = "TriangleUvContract", Size = new Vector2D<int>(W, H), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5)) });
window.Initialize();
using var gl = GL.GetApi(window);
GlVram.Scale = 1;
var view = ConfigManager.View;
view.MsaaSamples = samples;
view.EnhancedDepthBuffer = view.HighResolution3D = view.PerspectiveCorrectTextures = true;
view.HighResolutionTextures = view.TextureSmoothing = true;
view.EnhancedFog = view.EnhancedShadows = view.EnhancedParticles = view.HudAnchoring = false;
var backend = new EnhancedGlBackend(gl);

try
{
    backend.InitGl(); Check(backend.Ready, "GL startup");
    Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} | {gl.GetStringS(StringName.Version)}");
    object atlas = Get<object>("_textureReplacements");
    Check((int)atlas.GetType().GetProperty("Count")!.GetValue(atlas)! == 2, "synthetic atlas");
    GpuHle.Backend = backend; GpuHle.Active = GpuHle.GameplayActive = true;
    GpuHle.DebugGameplayTick = 3; GpuHle.WideAspect = 16f / 9f;
    backend.WriteVram(textureX,0,256,256,Enumerable.Repeat(nativeGreen,256*256).ToArray());
    var opaque = new PrimFlags { Material = HleMaterialKind.Opaque };
    var textured = opaque; textured.Textured = textured.RawTexture = true; textured.TPage = texture;
    var ui = new PrimFlags { Material = HleMaterialKind.Ui };

    Target(sideX,sideY,sideW,sideH);
    backend.DrawRect(new HleRect {X=sideX+1,Y=sideY+1,W=sideW-2,H=sideH-2,R=240,B=80},ui);
    backend.Flush();
    var side = Get<GlDisplayRt>("_kTarget");
    Target(0,0,W,H);
    backend.DrawRect(new HleRect {X=1,Y=1,W=2,H=2,B=255},ui);
    backend.Flush();
    var main = Get<GlDisplayRt>("_kTarget");
    Check(side != main && side.Samples == samples && main.Samples == samples &&
          main.Margin > 0 && side.Dirty, "two actual GL targets, AA and dirty source");

    HleVertex a = Vertex(30,10,247,251,-9), b = Vertex(65,30,9,31,11), c = Vertex(34,85,67,83,11);
    long before = Get<long>("_tracePartialWritebacks");
    backend.DrawTri(a,b,c,textured);
    Check(Get<long>("_tracePartialWritebacks") == before && side.Dirty,
        "live feedback must not read back the clipped-away target");
    object[] clipped = Batch();
    Check(clipped.Length == 6, "near clip must emit two child triangles");
    Bounds(clipped,clipBounds,"near clip must preserve the original authored footprint");
    var expected = new (float u,float v,float z)[] {
        (128,141,1),(9,31,11),(67,83,11),(128,141,1),(67,83,11),(157,167,1) };
    Check(clipped.Select(v => (F(v,"U"),F(v,"V"),F(v,"ViewZ"))).SequenceEqual(expected), "exact live near clip vertices");
    Check(clipped.All(v => F(v,"ReplacementW") > 0), "clipped children missed authored atlas crop");
    Check(a.HasAuthoredUvBounds && a.AuthoredMaxU == -777 && b.AuthoredMinV == -888,
          "mutated caller vertices");
    Describe("clipped",clipped); backend.Flush();

    backend.DrawTri(Vertex(96,16,15,22),Vertex(124,16,18,22),Vertex(96,44,15,26),textured);
    object[] ordinary = Batch();
    Check(ordinary.Length == 3 && ordinary.All(v => F(v,"ReplacementW") > 0), "ordinary atlas hit");
    Bounds(ordinary,smallBounds,"public DrawTri must compute from live input, not stale authored fields");
    Describe("ordinary",ordinary); backend.Flush();

    before = Get<long>("_tracePartialWritebacks");
    backend.DrawTri(Vertex(140,14,199,194),Vertex(162,14,220,194),Vertex(140,35,200,212),textured);
    Check(Get<long>("_tracePartialWritebacks") == before + 1,
          "live feedback must write back an intersecting dirty target");
    Bounds(Batch(),(199,194,220,212),"ordinary feedback footprint");
    Describe("feedback",Batch()); backend.Flush();

    // The public coarse-packet path builds 16 textured leaves near, and one
    // untextured leaf far. It reaches the generated submission without
    // reflecting into or substituting the production terrain builder.
    Terrain(0x14000,2100f,108,32,smallBounds,32,true);
    Terrain(0x16000,12000f,153,26,(1,2,1,2),2,false);

    // Non-world modal geometry retains its original footprint but moves by
    // the active wide target margin. Compare identical input on either side
    // of the native modal scope before drawing to actual color/depth targets.
    var ma = Vertex(180,34,2,5); var mb = Vertex(206,34,12,5); var mc = Vertex(180,57,2,20);
    backend.DrawTri(ma,mb,mc,ui);
    object[] nonModal = Batch();
    Check(nonModal.Length == 3, "non-modal comparison primitive");
    backend.Flush();
    GpuHle.SignalNativeModal();
    backend.DrawTri(ma,mb,mc,ui);
    var modal = Batch();
    Check(modal.Length == 3 && modal.Zip(nonModal).All(pair =>
        F(pair.First,"X") == F(pair.Second,"X") - main.Margin &&
        F(pair.First,"Y") == F(pair.Second,"Y")), "native modal must shift each X by the active margin");
    Bounds(modal,(2,5,12,20),"modal footprint");
    Describe("modal",modal); backend.Flush(); GpuHle.NativeModalHold = 0;

    var effect = new PrimFlags { Material = HleMaterialKind.ScreenEffect, SemiTrans = true, OtIndex = 35, TPage = 0x20 };
    backend.DrawTri(Vertex(204,115,15,22,550),Vertex(243,115,18,22,550),Vertex(204,139,15,26,550),effect);
    var queue = Get<IList>("_deferredScreenEffects");
    Check(queue.Count == 1 && Get<int>("_count") == 0, "deferred effect held its triangle");
    object defer = queue[0]!;
    var deferred = ((Array)defer.GetType().GetProperty("Vertices")!.GetValue(defer)!).Cast<object>().ToArray();
    Check(deferred.Length == 3, "deferred batch vertex count");
    T Pending<T>(string field) => (T)defer.GetType().GetProperty(field)!.GetValue(defer)!;
    Check(Pending<GlDisplayRt>("Target") == main && Pending<bool>("Transparent") &&
          Pending<bool>("DepthTest") && !Pending<bool>("DepthWrite") &&
          Pending<HleMaterialKind>("Material") == HleMaterialKind.ScreenEffect &&
          Pending<int>("Blend") == 1 && Pending<int>("ClipX1") == W - 1,
        "deferred batch lost its draw state");
    Bounds(deferred,smallBounds,"deferred footprint");
    Describe("deferred",deferred);
    backend.DrawRect(new HleRect {X=214,Y=145,W=22,H=12,G=150},ui);
    backend.Flush(); Check(queue.Count == 0, "UI did not replay pending triangle");

    // Full-size nonempty target readbacks include the widescreen margins as
    // well as packed native VRAM (both display targets and texture data).
    ReadTarget("main",main); ReadTarget("side",side);
    ushort[] allVram = new ushort[1024 * 512];
    backend.ReadVram(0,0,1024,512,allVram);
    Check(allVram.Any(x => (x & 0x7fff) == (nativeGreen & 0x7fff)) &&
          allVram.Take(240 * 1024).Any(x => (x & 0x7fff) == 0x001f), "render/VRAM checker was blank");
    Console.WriteLine("full-native-vram=" + Digest(allVram.AsSpan()));
    Check(gl.GetError() == GLEnum.NoError, "OpenGL error during readback");
    Console.WriteLine($"PASS triangle UV checks={checks} samples={samples} diagnostics={diagnostics}");
}
finally
{
    GpuHle.Backend = null;
    backend.Dispose();
    pack.Delete(true);
}

void Terrain(uint addr, float z, int y, int size, (float,float,float,float) bounds, int triangles, bool textured)
{
    var positions = new DreamcastTerrainGeometry.Sample[25];
    for (int x = 0; x < 5; x++)
    for (int row = 0; row < 5; row++)
        positions[x * 5 + row] = new(new((64 + x * size / 4f - 160) * z / 64,
            (y + row * size / 4f - 120) * z / 64,z),0,new(180,210,240),new(120,180,230));
    var patch = new GpuHle.TerrainPatchGeometry(positions,Vector3.Zero,160,120,64);
    var td = new GpuHle.TerrainTextureDescriptor(0,0x160f,0x1612,0x1a0f,0x1a12,texture,0,0,100,150,200);
    var colors = new GpuHle.TerrainQuadDistanceColors(default,default,default,default,false,true,patch);
    var cells = new GpuHle.TerrainCellTextures(Enumerable.Repeat(td,16).ToArray(),4,true,colors);
    GpuHle.RegisterCoarseTerrainPacket(addr,cells,false,"synthetic",0,0);
    var flags = new PrimFlags { Material=HleMaterialKind.TerrainRoute, PacketAddress=addr, OtIndex=50 };
    long prev = Get<long>("_traceGeneratedTerrainTriangles");
    var template = Vertex(10,10,1,2);
    backend.DrawTri(template,template,template,flags);
    Check(Get<long>("_traceGeneratedTerrainTriangles") == prev + triangles, "wrong generated terrain leaf count");
    var prepared = Batch();
    Check(prepared.Length == triangles * 3 &&
          prepared.All(v => (((int)Value(v,"Texpage") & 0x8000) == 0) == textured),
        "generated terrain lost vertices or native texture class");
    Bounds(prepared,bounds,"generated terrain footprint");
    Describe(textured ? "generated-textured" : "generated-flat",prepared);
    backend.Flush();
}

void Check(bool condition, string reason)
{
    if (!condition) throw new InvalidOperationException(reason);
    checks++;
}
void Target(int x,int y,int w,int h)
{
    GpuHle.NotifyDisplay(x,y,w,h);
    backend.SetDrawEnv(new HleDrawEnv { ClipX0=x,ClipY0=y,ClipX1=x+w-1,ClipY1=y+h-1 });
}
static HleVertex Vertex(int x, int y, short u, short v, float z = 0)
{
    return new HleVertex {
        X=x,Y=y,U=u,V=v,R=162,G=168,B=248, Z=z, PerspectiveW=z, ViewZ=z,
        ViewX=(x-160)*z/64,ViewY=(y-120)*z/64,ProjectionCenterX=160,ProjectionCenterY=120,ProjectionScale=64,
        HasViewSpace=z!=0,HasProjectiveW=z!=0,HasGteZ=z!=0,HasCoherentGteZ=z!=0,
        HasAuthoredUvBounds=true, AuthoredMinU=-999, AuthoredMinV=-888, AuthoredMaxU=-777, AuthoredMaxV=-666 };
}
T Get<T>(string name) => (T)typeof(EnhancedGlBackend).GetField(name,Private)!.GetValue(backend)!;
object[] Batch() => Get<Array>("_verts").Cast<object>().Take(Get<int>("_count")).ToArray();
static object Value(object vertex,string name) => vertex.GetType().GetField(name)!.GetValue(vertex)!;
static float F(object vertex,string name) => (float)Value(vertex,name);
void Bounds(object[] batch,(float,float,float,float) bounds,string reason) =>
    Check(batch.All(v => (F(v,"UvMinX"),F(v,"UvMinY"),F(v,"UvMaxX"),F(v,"UvMaxY")) == bounds),reason);

void Describe(string label, object[] batch)
{
    using var data = new MemoryStream();
    using var stream = new BinaryWriter(data);
    foreach (var v in batch)
    foreach (var f in v.GetType().GetFields().OrderBy(f => f.MetadataToken))
        switch (f.GetValue(v))
        {
            case float value: stream.Write(value); break;
            case uint value: stream.Write(value); break;
            case int value: stream.Write(value); break;
            default: throw new InvalidOperationException("Unexpected GPU vertex field type " + f.Name);
        }
    var target = Get<GlDisplayRt>("_kTarget");
    string state = $"{target.X},{target.Y},{target.Samples};" +
        $"{Get<bool>("_kTransparent")},{Get<int>("_kBlend")},{Get<HleMaterialKind>("_kMaterial")};" +
        $"{Get<bool>("_kDepthTest")},{Get<bool>("_kDepthWrite")},{Get<bool>("_kSourceDepthCompareWrite")};" +
        $"{Get<int>("_kSetMask")},{Get<int>("_kCheckMask")},{Get<int>("_kTextureSmoothing")};" +
        $"{Get<int>("_kTwAndX")},{Get<int>("_kTwAndY")},{Get<int>("_kTwOrX")},{Get<int>("_kTwOrY")};" +
        $"{Get<int>("_kClipX0")},{Get<int>("_kClipY0")},{Get<int>("_kClipX1")},{Get<int>("_kClipY1")}";
    Console.WriteLine($"{label} vertices={batch.Length} sha256={Convert.ToHexString(SHA256.HashData(data.ToArray()))} " +
        $"state={state} flushes={Get<long>("_traceFlushes")} feedback={Get<long>("_tracePartialWritebacks")}");
}

int GlInt(GetPName name) { gl.GetInteger(name,out int value); return value; }
string GlVector(GetPName name)
{
    int[] values = new int[4]; gl.GetInteger(name, values.AsSpan());
    return string.Join(',', values);
}
void ReadTarget(string label,GlDisplayRt rt)
{
    int draw = GlInt(GetPName.DrawFramebufferBinding), read = GlInt(GetPName.ReadFramebufferBinding);
    string state = $"{gl.IsEnabled(EnableCap.Blend)},{gl.IsEnabled(EnableCap.DepthTest)},{gl.IsEnabled(EnableCap.ScissorTest)}," +
        $"{gl.IsEnabled(EnableCap.StencilTest)},{gl.IsEnabled(EnableCap.CullFace)};" +
        $"{GlInt(GetPName.DepthFunc)},{GlInt(GetPName.DepthWritemask)},{GlInt(GetPName.StencilWritemask)};" +
        $"{GlInt(GetPName.BlendSrcRgb)},{GlInt(GetPName.BlendDstRgb)},{GlInt(GetPName.BlendEquationRgb)};" +
        $"{GlVector(GetPName.Viewport)};{GlVector(GetPName.ScissorBox)};{GlVector(GetPName.ColorWritemask)};" +
        $"{draw == rt.DrawFbo},{read == rt.Fbo}";
    rt.Resolve(gl);
    if (rt.MsaaFbo != 0)
    {
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer,rt.MsaaFbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer,rt.Fbo);
        gl.BlitFramebuffer(0,0,rt.TexW,rt.TexH,0,0,rt.TexW,rt.TexH,ClearBufferMask.DepthBufferBit,BlitFramebufferFilter.Nearest);
    }
    gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer,rt.Fbo);
    var rgba = new byte[rt.TexW * rt.TexH * 4]; var depth = new float[rt.TexW * rt.TexH];
    gl.ReadPixels<byte>(0,0,(uint)rt.TexW,(uint)rt.TexH,PixelFormat.Rgba,PixelType.UnsignedByte,rgba);
    gl.ReadPixels<float>(0,0,(uint)rt.TexW,(uint)rt.TexH,PixelFormat.DepthComponent,PixelType.Float,depth);
    gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer,(uint)read);
    gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer,(uint)draw);
    Check(rgba.Any(c => c != 0) && (label != "main" || depth.Any(d => d < 1f)), "blank GL color/depth output");
    Console.WriteLine($"{label} size={rt.TexW}x{rt.TexH} color={Digest(rgba.AsSpan())} depth={Digest(depth.AsSpan())} gl={state}");
}

static string Digest<T>(Span<T> values) where T : unmanaged =>
    Convert.ToHexString(SHA256.HashData(MemoryMarshal.AsBytes(values)));
static ulong Key(ushort color,int width,int height)
{
    ulong hash = 14695981039346656037UL;
    void Add(int value) => hash = unchecked((hash ^ (byte)value) * 1099511628211UL);
    static int Expand(int x) => ((x & 31) << 3) | ((x & 31) >> 2);
    Add(width); Add(width >> 8); Add(height); Add(height >> 8);
    for (int i=0; i<width*height; i++) { Add(Expand(color)); Add(Expand(color>>5)); Add(Expand(color>>10)); Add(color>>15 == 1 ? 255 : 0); }
    return hash;
}
static void MakePack(string dir)
{
    byte[] dds = new byte[128 + 8];
    void U32(int i,uint v) => BinaryPrimitives.WriteUInt32LittleEndian(dds.AsSpan(i),v);
    U32(0,0x20534444); U32(4,124); U32(8,0x100f); U32(12,1); U32(16,2); U32(20,8);
    U32(76,32); U32(80,0x41); U32(88,32); U32(92,0xff); U32(96,0xff00);
    U32(100,0xff0000); U32(104,0xff000000); U32(108,0x1000);
    U32(128,0xff0000ff); U32(132,0xffff0000);
    File.WriteAllBytes(Path.Combine(dir,"synthetic.dds"),dds);
    var entries = new[] { Key(nativeGreen,4,5), Key(nativeGreen,239,221) }.Select((key,x) =>
        new { key=key.ToString("x16"),image="synthetic.dds",x,y=0,width=1,height=1 });
    File.WriteAllText(Path.Combine(dir,"manifest.json"),JsonSerializer.Serialize(new { format=3,entries }));
}
