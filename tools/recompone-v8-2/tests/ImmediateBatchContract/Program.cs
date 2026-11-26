// The same synthetic submissions run against frozen and candidate runtimes.
// Exercise immediate DrawTri/Rect/Line, not the deferred replay machinery.
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

const int Width = 64, Height = 240, Second = 128, N = 4096;
int samples = args.Contains("--msaa4") ? 4 : 0;
if (Environment.GetEnvironmentVariable("BATCH_CONTRACT_RUNTIME_SHA256") is { } expected)
{
    using var loaded = File.OpenRead(typeof(EnhancedGlBackend).Assembly.Location);
    Check(Convert.ToHexString(SHA256.HashData(loaded)).Equals(expected, StringComparison.OrdinalIgnoreCase),
          "loaded runtime does not match the comparison input");
}
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_ENHANCED_PERFORMANCE", "1");
Environment.SetEnvironmentVariable("RECOMPONE_DIRECT_TERRAIN", "1");
GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with {
    Title = "ImmediateBatchContract", Size = new Vector2D<int>(Width, Height), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
GlVram.Scale = 1;
var view = ConfigManager.View;
view.MsaaSamples = samples;
view.EnhancedDepthBuffer = view.HighResolution3D = view.PerspectiveCorrectTextures = true;
view.HighResolutionTextures = view.HudAnchoring = view.TextureSmoothing = false;
view.EnhancedFog = view.EnhancedParticles = view.EnhancedShadows = false;
var backend = new EnhancedGlBackend(gl);
HleDrawEnv env = default;
var opaque = Flags(HleMaterialKind.Opaque);
var additive = Flags(HleMaterialKind.Additive, true, 1);
var reflection = Flags(HleMaterialKind.VehicleReflection, true, 3);
var subtract = Flags(HleMaterialKind.Subtractive, true, 2);

try
{
    backend.InitGl(); Check(backend.Ready, "Enhanced backend did not initialize");
    GpuHle.Backend = backend; GpuHle.Active = GpuHle.GameplayActive = true;
    GpuHle.DebugGameplayTick = 3; GpuHle.WideAspect = GpuHle.BaseAspect;
    foreach (int origin in new[]{ 0, Second })
    {
        Target(origin);
        backend.DrawRect(new HleRect { X = origin, W = Width, H = Height }, opaque);
        backend.Flush();
        Check(Get<GlDisplayRt>("_kTarget").Samples == samples, "fixture missed requested AA");
    }

    Target(0);
    var variants = new[]{ opaque, additive, Flags(HleMaterialKind.AlphaTest, false, 2), reflection };
    long flushBefore = Flushes();
    for (int i = 0; i < N; i++)
    {
        // All four raw states use the ordinary dual-source group. No GTE or OT
        // depth on these triangles. Each one reaches the immediate Begin path.
        Tri(6 + i % 10 * 4, 8 + i % 7 * 4, variants[i % variants.Length]);
    }
    Check(Count() == N * 3 && Flushes() == flushBefore, "matching batch broke or lost geometry");
    Raw(reflection);
    Rect(8, 43, subtract);
    Check(Flushes() == flushBefore + 1 && Count() == 6, "incompatible blend did not close previous batch");
    var trace = Get<object>("_traceStateTransitions");
    var transitions = (string)trace.GetType().GetMethod("FormatTopTransitions",
        BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public)!.Invoke(trace, null)!;
    Check(transitions.Contains("T:VehicleReflection:00->T:Subtractive:00=1"), "trace lost last raw material");
    // Different raw blends can even share the subtractive group. The FINAL one
    // controls whether Flush uses one or two passes.
    var subtractOne = Flags(HleMaterialKind.Subtractive, true, 1);
    Line(10, 52, subtractOne);
    Raw(subtractOne);
    Check(Count() == 12 && Flushes() == flushBefore + 1, "equivalent subtractive modes split");
    backend.Flush();
    Console.WriteLine($"compatible={N} vertices={N*3} flushes=1 last={reflection.Material}/{reflection.BlendMode} " +
        $"subtractive-last={subtractOne.BlendMode}");

    // Control: when every primitive changes the effective blend group, all
    // submissions need the full state, and every predecessor must flush.
    flushBefore = Flushes();
    for (int i = 0; i < 32; i++) Tri(14 + i % 4 * 6, 60 + i / 4 * 4, i % 2 == 0 ? opaque : subtract);
    backend.Flush();
    Check(Flushes() - flushBefore == 32, "alternating state control merged");
    Console.WriteLine("alternating=32 vertices=96 flushes=32");

    // Change clip, independent texture-window axes, set/check mask and target.
    Rect(2, 105, opaque);
    env.ClipX0 = 11; env.ClipY0 = 2; env.ClipX1 = 54; env.ClipY1 = 235;
    env.TwMaskX = 3; env.TwMaskY = 5; env.TwOffX = 1; env.TwOffY = 4; env.SetMask = true;
    backend.SetDrawEnv(env);
    flushBefore = Flushes(); Rect(9, 110, opaque);
    Check(Flushes() == flushBefore + 1 && Count() == 6, "clip/window/mask boundary missed");
    Check(Get<int>("_kClipX0") == 11 && Get<int>("_kClipY0") == 2 && Get<int>("_kClipX1") == 54 &&
          Get<int>("_kClipY1") == 235 && Get<int>("_kTwAndX") == 231 && Get<int>("_kTwAndY") == 215 &&
          Get<int>("_kTwOrX") == 8 && Get<int>("_kTwOrY") == 32 && Get<int>("_kSetMask") == 1, "stale draw environment");
    env.TwOffX |= 16; env.TwOffY |= 16; // uncared bits: same effective window
    backend.SetDrawEnv(env); Line(20, 114, additive); Raw(additive);
    Check(Flushes() == flushBefore + 1 && Count() == 12, "equivalent texture window split");
    env.CheckMask = true; backend.SetDrawEnv(env); Rect(18, 122, opaque);
    Check(Get<int>("_kCheckMask") == 1 && Flushes() == flushBefore + 2, "check-mask boundary missed");
    var oldTarget = Get<GlDisplayRt>("_kTarget");
    Target(Second); Rect(Second + 4, 14, opaque, 0, 255, 0);
    Check(Count() == 6 && Get<GlDisplayRt>("_kTarget") != oldTarget &&
          Get<int>("_kSetMask") == 0 && Get<int>("_kCheckMask") == 0, "target switch retained old state");

    // Native terrain depth (strict LESS) and ordinary opaque painter depth
    // must remain distinct even when blend, target and clip match.
    Target(0); var terrain = Flags(HleMaterialKind.TerrainRoute);
    terrain.OtIndex = 50; terrain.DreamcastTerrainDepth = true;
    Tri(15, 145, terrain, true);
    Check(Get<bool>("_kDepthTest") && Get<bool>("_kDepthWrite") && Get<bool>("_kSourceDepthCompareWrite") &&
          Get<bool>("_kDreamcastTerrainDepth"), "authored terrain depth was not installed");
    flushBefore = Flushes(); Tri(23, 145, terrain, true);
    Check(Flushes() == flushBefore && Count() == 6, "matching terrain depth split");
    Rect(5, 160, opaque);
    Check(Flushes() == flushBefore + 1 && !Get<bool>("_kDepthTest") && !Get<bool>("_kDepthWrite") &&
          !Get<bool>("_kSourceDepthCompareWrite") && !Get<bool>("_kDreamcastTerrainDepth"), "stale terrain depth");

    // Real VBO boundary. Pad only unused entries with degenerate vertices so
    // equality and the next spill are reachable without a giant generated scene.
    var vertices = Get<Array>("_verts"); int capacity = vertices.Length;
    Array.Clear(vertices, Count(), capacity - Count());
    Field("_count").SetValue(backend, capacity - 6);
    flushBefore = Flushes(); Rect(8, 177, opaque);
    Check(Count() == capacity && Flushes() == flushBefore, "exact capacity flushed early");
    Rect(22, 177, opaque);
    Check(Count() == 6 && Flushes() == flushBefore + 1 && Get<GlDisplayRt>("_kTarget") == oldTarget,
          "one-over capacity failed to start the next batch on the right target");
    backend.Flush();

    view.TextureSmoothing = true; backend.PresentDisplay(0, 0, Width, Height);
    Target(0); Rect(2, 202, opaque);
    Check(Get<int>("_kTextureSmoothing") == 1, "first new frame reused stale smoothing");
    backend.Flush();
    var a = Read(0); var b = Read(Second);
    Check((a[204 * Width + 4] & 0x7fff) == 0x001f &&
          (b[16 * Width + 6] & 0x7fff) == 0x03e0 && (b[204 * Width + 4] & 0x7fff) == 0,
          "public VRAM observation lost/crossed targets");
    Check(gl.GetError() == GLEnum.NoError, "OpenGL error");
    Console.WriteLine($"pixels A={Digest(a)} B={Digest(b)} samples={samples}");
    Console.WriteLine("PASS immediate batch compatibility, raw state, transitions, clip/window/masks, target, depth and capacity");
}
finally { GpuHle.Backend = null; backend.Dispose(); }

FieldInfo Field(string name) => typeof(EnhancedGlBackend).GetField(name, BindingFlags.Instance | BindingFlags.NonPublic)!;
T Get<T>(string name) => (T)Field(name).GetValue(backend)!;
int Count() => Get<int>("_count");
long Flushes() => Get<long>("_traceFlushes");
void Raw(PrimFlags f) => Check(Get<bool>("_kTransparent") == f.SemiTrans && Get<int>("_kBlend") == f.BlendMode &&
    Get<HleMaterialKind>("_kMaterial") == f.Material, "raw primitive state was not updated on a compatible batch");
static PrimFlags Flags(HleMaterialKind kind, bool semi = false, int mode = 0) =>
    new(){ Material = kind, SemiTrans = semi, TPage = (ushort)(mode << 5) };
void Target(int x)
{
    GpuHle.NotifyDisplay(x, 0, Width, Height);
    env = new() { ClipX0 = x, ClipX1 = x + Width - 1, ClipY1 = Height - 1 }; backend.SetDrawEnv(env);
}
void Rect(int x, int y, PrimFlags f, byte r = 255, byte g = 0, byte b = 0) =>
    backend.DrawRect(new HleRect { X = x, Y = y, W = 8, H = 7, R = r, G = g, B = b }, f);
void Line(int x, int y, PrimFlags f) => backend.DrawLine(new HleVertex { X=x, Y=y, G=255 }, new HleVertex { X=x+4, Y=y+1, G=255 }, f);
void Tri(int x, int y, PrimFlags f, bool projected = false)
{
    HleVertex V(int vx, int vy) => new(){ X = vx, Y = vy, G = 128, B = 255, Z = 850, PerspectiveW = 850,
        ViewX = (vx - 32) * 850f / 64, ViewY = (vy - 32) * 850f / 64, ViewZ = 850,
        ProjectionCenterX = 32, ProjectionCenterY = 32, ProjectionScale = 64,
        HasGteZ = projected, HasCoherentGteZ = projected, HasProjectiveW = projected, HasViewSpace = projected };
    backend.DrawTri(V(x, y), V(x + 3, y), V(x, y + 3), f);
}
ushort[] Read(int x) { var pixels = new ushort[Width*Height]; backend.ReadVram(x,0,Width,Height,pixels); return pixels; }
static string Digest(ushort[] pixels) => Convert.ToHexString(SHA256.HashData(MemoryMarshal.AsBytes(pixels.AsSpan())));
static void Check(bool yes, string why) { if (!yes) throw new InvalidOperationException(why); }
