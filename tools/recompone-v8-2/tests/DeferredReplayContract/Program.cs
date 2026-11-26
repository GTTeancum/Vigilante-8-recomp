// Synthetic replay through the real OpenGL backend. No translated guest or media.
using System.Collections;
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

bool diagnostics = args.Contains("--diagnostics");
int samples = args.Contains("--msaa4") ? 4 : 0;
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_ENHANCED_PERFORMANCE", "1");
Environment.SetEnvironmentVariable("RECOMPONE_DIRECT_TERRAIN", "0");
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_WATER_DEPTH_PROBE", diagnostics ? "1" : null);
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_V82_NATIVE_WATER", diagnostics ? "1" : null);
Environment.SetEnvironmentVariable("RECOMPONE_TRACE_TRIANGLE_PROBE", diagnostics ? "49,11" : null);
const int W = 64, H = 240, Second = 128;
const BindingFlags Private = BindingFlags.Instance | BindingFlags.NonPublic;

GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with
{
    Title = "DeferredReplayContract", Size = new Vector2D<int>(W, H), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
GlVram.Scale = 1;
var view = ConfigManager.View;
view.MsaaSamples = samples;
view.EnhancedDepthBuffer = view.HighResolution3D = view.PerspectiveCorrectTextures = true;
view.EnhancedFog = view.EnhancedShadows = view.EnhancedParticles = false;
view.HighResolutionTextures = view.TextureSmoothing = view.HudAnchoring = false;
var backend = new EnhancedGlBackend(gl);

try
{
    backend.InitGl();
    Check(backend.Ready, "No Enhanced GL renderer");
    GpuHle.Backend = backend;
    GpuHle.Active = GpuHle.GameplayActive = true;
    GpuHle.DebugGameplayTick = 1;
    GpuHle.WideAspect = GpuHle.BaseAspect;
    Target(0); Sprite(0, 0, W, H, 0, 0, 0); backend.Flush();
    var firstRt = (GlDisplayRt)Field("_classifiedTarget");
    Target(Second); Sprite(Second, 0, W, H, 0, 0, 0); backend.Flush();
    var secondRt = (GlDisplayRt)Field("_classifiedTarget");
    Check(firstRt.Samples == samples && secondRt.Samples == samples && firstRt != secondRt,
        "Must exercise two actual targets and the requested AA");
    Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} samples={samples} diagnostics={diagnostics}");

    Target(0);
    var terrain = new PrimFlags { Material = HleMaterialKind.TerrainRoute, DreamcastTerrainDepth = true, OtIndex = 50 };
    Quad(4, 5, 43, 45, 1000, 0, 255, 0, terrain);
    backend.Flush();
    backend.WriteVram(256, 256, 2, 2, new ushort[] { 0x83e0, 0x83e0, 0x83e0, 0x83e0 });
    var before = Read(0);
    var road = new PrimFlags { Material = HleMaterialKind.TerrainRoute, NativeRoad = true, NativeRoadPrioritySz = 200, OtIndex = 40 };
    Quad(9, 13, 42, 21, 1050, 0, 0, 255, road);
    Quad(8, 26, 22, 33, 550, 128, 128, 128, new PrimFlags
    {
        Material = HleMaterialKind.Subtractive, Textured = true, RawTexture = true,
        SemiTrans = true, Vehicle = true, OtIndex = 40, TPage = 0x154
    });

    // Tiny synthetic XWAT chunk also exercises the actual texture upload.
    // The displaced surface has 9 vertices while the placeholder quad has 6.
    byte[] xwat =
    [
        (byte)'X', (byte)'W', (byte)'A', (byte)'T', 0, 0, 0, 40,
        0x10, 0, 0, 0, 0x08, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0,
        0, 0, 0xff, 0x83, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0x11, 0x11, 0x11, 0x11
    ];
    Check(GpuHle.TryRegisterDreamcastWaterTexture("synthetic", xwat), "Fixture XWAT rejected");
    var water = new GpuHle.DreamcastWaterBaseQuad(new(44, 5, 900), new(60, 5, 900),
        new(44, 18, 900), new(60, 18, 900), 0, 0, 255);
    var mesh = new GpuHle.DreamcastWaterSurfaceMesh(
        [Wave(45, 8), Wave(58, 8), Wave(45, 21), Wave(58, 8), Wave(58, 21), Wave(45, 21),
         Wave(46, 23), Wave(55, 23), Wave(46, 28)], 2, 2, GpuHle.CurrentDreamcastWaterTexture);
    Quad(46, 34, 60, 42, 900, 255, 0, 0, new PrimFlags
    {
        Material = HleMaterialKind.WaterBase, SemiTrans = true, OtIndex = 40,
        WaterBaseGroup = 0x20000, WaterBaseQuad = water, WaterSurfaceMesh = mesh
    });
    Quad(3, 46, 23, 56, 150, 255, 0, 0, Effect(0x20));
    Target(Second);
    Quad(Second + 10, 50, Second + 25, 64, 150, 0, 255, 0, Effect(0x20));
    Check(Queue("_deferredRoads").Count == 1 && Queue("_deferredVehicleSubtractive").Count == 1 &&
          Queue("_deferredWater").Count == 1 && Queue("_deferredScreenEffects").Count == 4,
        "Fixture did not populate all four mixed queues");
    long submitted = Vertices();
    Sprite(Second + 31, 10, 4, 4, 0, 0, 255); // ordinary UI boundary replays all four queues
    Check(Vertices() - submitted == 39 && Queue("_deferredScreenEffects").Count == 0 && Queue("_deferredWater").Count == 0,
        "Mixed replay did not submit the base, 9-vertex mesh, road, shadow and two effects before UI");
    backend.Flush();
    ushort[] a = Read(0), b = Read(Second);
    Check(Pixel(a, 6, 51) == 0x001f && Pixel(b, 15, 55) == 0x03e0 && Pixel(b, 6, 51) == 0,
        "Queued draw lost its old target/clip before adopting the next target");
    Check(Pixel(a, 13, 17) == 0x7c00 &&
          (Pixel(a, 12, 29) & 0x03e0) < Pixel(before, 12, 29) &&
          Pixel(a, 49, 7) != 0 && Pixel(a, 48, 24) != 0 && Pixel(a, 57, 37) == 0,
        $"Road={Pixel(a, 13, 17):X4} shadow={Pixel(before, 12, 29):X4}->{Pixel(a, 12, 29):X4} " +
        $"water={Pixel(a, 49, 7):X4}/{Pixel(a, 48, 24):X4} placeholder={Pixel(a, 57, 37):X4}");
    Check(Pixel(b, 32, 11) == 0x7c00, "Ordinary UI did not survive replay");
    Print("mixed-a", a); Print("mixed-b", b);

    // Real loading-glyph classifier and both replay targets. Palette index 1
    // is cyan; the old implementation shifts the glyph from y=209 to y=192.
    backend.WriteVram(1, 480, 1, 1, new ushort[] { 0x7fe0 });
    backend.WriteVram(5 * 64 + 8, 16, 4, 15, Enumerable.Repeat((ushort)0x0101, 60).ToArray());
    GpuHle.DebugGameplayTick = 0;
    foreach (int x in new[] { 0, Second })
    {
        Target(x);
        backend.DrawRect(new HleRect { X = x + 10, Y = 209, W = 8, H = 15, U = 16, V = 16 },
            new PrimFlags { Material = HleMaterialKind.Ui, Textured = true, RawTexture = true, TPage = 0x0a5, Clut = 0x7800 });
    }
    Check(Queue("_deferredLoadingPrompt").Count == 2, "Fixture missed deferred loading classifier");
    var presentation = backend.PresentDisplay(0, 0, W, H);
    Check(presentation.tex != 0 && Queue("_deferredLoadingPrompt").Count == 0, "Present did not drain loading queue");
    a = Read(0); b = Read(Second);
    Check(Pixel(a, 12, 195) == 0x7fe0 && Pixel(b, 12, 195) == 0x7fe0 &&
          Pixel(a, 12, 211) == 0 && Pixel(b, 12, 211) == 0,
        $"Loading replay shifted A={Pixel(a, 12, 195):X4} B={Pixel(b, 12, 195):X4}, " +
        $"unshifted A={Pixel(a, 12, 211):X4} B={Pixel(b, 12, 211):X4}");
    Print("loading-a", a); Print("loading-b", b);

    // Normal vertices are emitted in multiples of 3. Pad only the end of
    // the second batch with degenerate vertices to hit the exact array limit.
    // Raw blend values differ but give the same BlendStateOf grouping.
    GpuHle.DebugGameplayTick = 2;
    int capacity = ((Array)Field("_verts")).Length;
    foreach (int extra in new[] { 0, 1 })
    {
        int y = 80 + extra * 18;
        Target(0);
        HleVertex V(int vx, int vy, byte red, byte green) => Vertex(0, vx, vy, 150, red, green, 0);
        backend.DrawTri(V(9, y, 255, 0), V(18, y, 255, 0), V(9, y + 12, 255, 0), Effect(0x20));
        backend.DrawTri(V(27, y, 0, 255), V(38, y, 0, 255), V(27, y + 12, 0, 255), Effect(0));
        var queue = Queue("_deferredScreenEffects");
        Check(queue.Count == 2, "Capacity fixture missed array replay");
        object pending = queue[1]!;
        PropertyInfo property = pending.GetType().GetProperty("Vertices")!;
        var original = (Array)property.GetValue(pending)!;
        var padded = Array.CreateInstance(original.GetType().GetElementType()!, capacity - 3 + extra);
        Array.Copy(original, padded, original.Length);
        property.SetValue(pending, padded);
        queue[1] = pending;
        long flushes = Flushes();
        submitted = Vertices();
        Sprite(60, 203, 2, 2, 0, 0, 255);
        Check(Flushes() - flushes == 1 + extra && Vertices() - submitted == capacity + extra,
            $"Capacity replay ({extra} extra): incorrect boundary flush count or upload extent");
        backend.Flush();
        a = Read(0);
        Check(Pixel(a, 11, y + 3) == 0x001f && (Pixel(a, 29, y + 3) & 0x03e0) != 0,
            "Capacity spill/merge lost real vertices");
        Print($"capacity-{extra}", a);
    }

    // Joint X/Y VRAM wrap intersects only the first display. This snapshots
    // the actual wrapped read order without changing its capture/writeback.
    ushort[] pattern = [31, 992, 31744, 32767, 32799, 33760, 64512, 65535, 66, 132, 264, 528, 1056, 2112, 4224, 8448];
    ushort[] wrapped = new ushort[pattern.Length];
    backend.WriteVram(1022, 510, 4, 4, pattern);
    backend.ReadVram(1022, 510, 4, 4, wrapped);
    Check(pattern.SequenceEqual(wrapped), "Joint native VRAM wrap damaged pixels");
    Print("wrap", wrapped);
    Check(gl.GetError() == GLEnum.NoError, "OpenGL reported an error during replay");
    Console.WriteLine("PASS mixed target/order, capacity, loading and wrap");
}
finally
{
    GpuHle.Backend = null;
    backend.Dispose();
}

void Target(int x)
{
    GpuHle.NotifyDisplay(x, 0, W, H);
    backend.SetDrawEnv(new HleDrawEnv { ClipX0 = x, ClipX1 = x + W - 1, ClipY1 = H - 1 });
}
void Sprite(int x, int y, int w, int h, byte r, byte g, byte b) =>
    backend.DrawRect(new HleRect { X = x, Y = y, W = w, H = h, R = r, G = g, B = b },
        new PrimFlags { Material = HleMaterialKind.Ui });
void Quad(int x, int y, int right, int bottom, float z, byte r, byte g, byte b, PrimFlags f)
{
    int origin = x >= Second ? Second : 0;
    HleVertex V(int sx, int sy) => Vertex(origin, sx, sy, z, r, g, b);
    var a = V(x, y); var c = V(x, bottom); var d = V(right, y); var e = V(right, bottom);
    backend.DrawTri(a, d, c, f);
    backend.DrawTri(d, e, c, f);
}
static HleVertex Vertex(int origin, int x, int y, float z, byte r, byte g, byte b) => new()
{
    X = x, Y = y, Z = z, ViewX = (x - origin - 32) * z / 64, ViewY = (y - 32) * z / 64, ViewZ = z,
    ProjectionCenterX = origin + 32, ProjectionCenterY = 32, ProjectionScale = 64,
    PerspectiveW = z, HasViewSpace = true, HasGteZ = true, HasCoherentGteZ = true, HasProjectiveW = true,
    R = r, G = g, B = b
};
static PrimFlags Effect(ushort tpage) => new()
    { Material = HleMaterialKind.ScreenEffect, SemiTrans = true, OtIndex = 40, TPage = tpage };
static GpuHle.DreamcastWaterSurfaceVertex Wave(float x, float y) =>
    new(x, y, 800, x / 64, y / 64, 0, 0, 800, 32, 32, 64);
object Field(string field) => typeof(EnhancedGlBackend).GetField(field, Private)!.GetValue(backend)!;
IList Queue(string field) => (IList)Field(field);
long Vertices() => (long)Field("_traceBatchVertices");
long Flushes() => (long)Field("_traceFlushes");
ushort[] Read(int x)
{
    ushort[] pixels = new ushort[W * H];
    backend.ReadVram(x, 0, W, H, pixels);
    return pixels;
}
static ushort Pixel(ushort[] image, int x, int y) => (ushort)(image[y * W + x] & 0x7fff);
static void Print(string name, ushort[] image) =>
    Console.WriteLine($"{name} rgba5551={Convert.ToHexString(SHA256.HashData(MemoryMarshal.AsBytes(image.AsSpan())))}");
static void Check(bool ok, string message)
{
    if (!ok) throw new InvalidOperationException(message);
}
