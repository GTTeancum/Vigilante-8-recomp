using System.Collections;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using RecompOne.Runtime;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

// Source-only replay. No translated guest, assets, window input, or full build.
if (args.Length != 2 || args[0] is not ("legacy" or "trial" or "direct") ||
    !int.TryParse(args[1], out int samples) || samples is not (0 or 4))
    return 2;
bool trial = args[0] == "trial", direct = args[0] == "direct";
const int W = 64, H = 64;
const string Title = "Vigilante 8: 2nd Offense PC";
Runtime.ConfigureGameTitle(Title);

GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with
{
    Title = "StreamBatchContract", Size = new Vector2D<int>(W, H), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} GL={gl.GetStringS(StringName.Version)}");

GlVram.Scale = 1;
var view = ConfigManager.View;
view.MsaaSamples = samples;
view.EnhancedDepthBuffer = view.HighResolution3D = view.PerspectiveCorrectTextures = true;
view.EnhancedFog = view.EnhancedShadows = view.EnhancedParticles = false;
view.HighResolutionTextures = view.TextureSmoothing = view.HudAnchoring = false;
var backend = new EnhancedGlBackend(gl);
backend.InitGl();
Require(backend.Ready, "Enhanced renderer failed to initialize");
GpuHle.Active = GpuHle.GameplayActive = true;
GpuHle.Backend = backend;
GpuHle.DebugGameplayTick = 1;
GpuHle.WideAspect = GpuHle.BaseAspect;
var memory = new PSMemory();
var cpu = new CpuContext { GP = 0x80070000 };
V82Compat.BeginWorldRenderTerrainCache(cpu, memory);
Require(V82Compat.DirectSceneRendering == direct, "the stock world hook changed direct terrain mode");

var frameEnv = new HleDrawEnv { ClipX1 = W - 1, ClipY1 = H - 1 };
var ui = new PrimFlags { Material = HleMaterialKind.Ui };
var land = new PrimFlags { Material = HleMaterialKind.TerrainRoute, DreamcastTerrainDepth = true, OtIndex = 50 };
const BindingFlags Instance = BindingFlags.Instance | BindingFlags.NonPublic;
Type backendType = typeof(EnhancedGlBackend);
FieldInfo cursorField = backendType.GetField("_streamVertex", Instance)!;
FieldInfo countField = backendType.GetField("_count", Instance)!;
int capacity = ((Array)Field("_verts")).Length;
int stride = Marshal.SizeOf(((Array)Field("_verts")).GetType().GetElementType()!);
uint vbo = (uint)Field("_vbo");
int nonzero = 0, wraps = 0, zero = 0, uploads = 0;
uint query = gl.GenQuery();
GpuHle.NotifyDisplay(0, 0, W, H);
backend.SetDrawEnv(frameEnv);

try
{
    Sprite(0, 0, W, H, 0, 0, 0);
    Flush("initial display", 6);
    var rt = (GlDisplayRt)Field("_classifiedTarget");
    Require(rt.Samples == samples && rt.W == W && rt.H == H && rt.Margin == 0,
        $"requested {samples}x but got {rt.Samples}x or unexpected dimensions");
    CheckStorage();

    // Actual draws through the full-size production VBO. No cursor injection,
    // GL queries or readbacks inside this loop. One unique pixel records each
    // completed batch, including those immediately before and after wrap.
    byte[] colors = [255, 0, 0, 0, 255, 0, 0, 0, 255];
    for (int b = 0; b < 96; b++)
    {
        int color = 3 * (b % 3);
        for (int r = 0; r < 1024; r++)
            Sprite(2 + b % 20, 35 + b / 20, 1, 1, colors[color], colors[color + 1], colors[color + 2]);
        Flush("wrap stress", 6 * 1024);
    }
    Require(wraps == (trial || direct ? 2 : 0), "wrap stress did not cross two real storage boundaries");
    CheckStorage();
    var wrapped = Capture("wrap", rt);
    for (int b = 0; b < 96; b++)
        Require(wrapped.Vram[(35 + b / 20) * W + 2 + b % 20] == (31 << (5 * (b % 3))),
            $"draw from stress batch {b} is missing");

    var nativeEnv = new HleDrawEnv { ClipX0 = 128, ClipX1 = 191, ClipY1 = H - 1 };
    backend.SetDrawEnv(nativeEnv);
    Sprite(131, 3, 6, 6, 255, 0, 255);
    Flush("native VRAM transition", 6, eligible: false);
    var native = new ushort[1];
    backend.ReadVram(133, 5, 1, 1, native);
    Require(native[0] == 0x7c1f, "native VRAM offset-zero control did not draw");
    backend.SetDrawEnv(frameEnv);
    Sprite(55, 51, 4, 4, 255, 0, 0);
    Flush("resume after native VRAM", 6);
    GpuHle.GameplayActive = false;
    Sprite(55, 51, 4, 4, 0, 255, 0);
    Flush("menu", 6, eligible: false);
    GpuHle.GameplayActive = true;
    Sprite(55, 55, 4, 4, 255, 255, 0);
    Flush("resume after menu", 6);

    Runtime.ConfigureGameTitle("Vigilante 8");
    Sprite(48, 55, 4, 4, 0, 255, 255);
    Flush("other title", 6, eligible: false);
    Runtime.ConfigureGameTitle(Title);
    GpuHle.Active = false;
    Sprite(55, 59, 4, 4, 255, 0, 255);
    Flush("inactive enhanced dispatch", 6, eligible: false);
    GpuHle.Active = true;

    backend.SetDrawEnv(frameEnv with { SetMask = true });
    Sprite(48, 30, 5, 4, 255, 0, 0);
    Flush("set mask", 6);
    backend.SetDrawEnv(frameEnv with { CheckMask = true });
    Sprite(48, 30, 8, 4, 0, 0, 255);
    Flush("check mask", 6);
    backend.SetDrawEnv(frameEnv);
    var mask = Capture("mask-and-transition", rt);
    Require(Pixel(mask, 50, 31) == 0x801f, "set/check mask lost its protected pixel");
    if (samples == 0)
        Require(Pixel(mask, 54, 31) == 0x7c00, "check-mask draw did not reach its unmasked pixel");
    Require(Pixel(mask, 56, 52) == 0x03e0 && Pixel(mask, 56, 56) == 0x03ff &&
        Pixel(mask, 49, 56) == 0x7fe0 && Pixel(mask, 56, 60) == 0x7c1f,
        "a mode-transition draw failed");

    Quad(4, 4, 43, 30, 1000, 0, 255, 0, land);
    Flush("terrain strict LESS write", 6);
    Quad(4, 4, 43, 30, 1000, 255, 0, 0, land);
    Flush("terrain strict LESS rejection", 6);
    Quad(24, 10, 36, 24, 250, 255, 0, 0, new PrimFlags { Material = HleMaterialKind.Opaque, OtIndex = 40 });
    Flush("ordinary occluder", 6);
    var beforeRoad = Capture("terrain-and-occluder", rt);
    Require(Pixel(beforeRoad, 13, 17) == 0x03e0 && (DepthStencil(beforeRoad, 13, 17) & 255) == 1,
        "terrain/stencil did not keep the first coplanar layer");
    Require(Pixel(beforeRoad, 27, 17) == 0x001f && (DepthStencil(beforeRoad, 27, 17) & 255) == 0,
        "nonterrain occluder did not clear terrain stencil");

    var road = new PrimFlags { Material = HleMaterialKind.TerrainRoute, NativeRoad = true, NativeRoadPrioritySz = 200, OtIndex = 40 };
    Quad(9, 13, 42, 21, 1050, 0, 0, 255, road);
    Quad(9, 22, 22, 25, 1050, 255, 255, 0, road);
    Replay("_deferredRoads", 16, [12]);
    var afterRoad = Capture("four-pass-road", rt);
    Require(Pixel(afterRoad, 13, 17) == 0x7c00 && Pixel(afterRoad, 13, 23) == 0x03ff &&
        (DepthStencil(afterRoad, 13, 17) & 255) == 0 &&
        (DepthStencil(afterRoad, 13, 17) >> 8) > (DepthStencil(beforeRoad, 13, 17) >> 8),
        "road priority did not publish correct color, real depth and stencil retirement");
    Require(Pixel(afterRoad, 27, 17) == Pixel(beforeRoad, 27, 17) &&
        DepthStencil(afterRoad, 27, 17) == DepthStencil(beforeRoad, 27, 17),
        "road obscured foreground geometry");

    backend.WriteVram(256, 256, 2, 2, new ushort[] { 0x83e0, 0x83e0, 0x83e0, 0x83e0 });
    Quad(8, 26, 22, 31, 550, 128, 128, 128, new PrimFlags
    {
        Material = HleMaterialKind.Subtractive, Textured = true, RawTexture = true,
        SemiTrans = true, Vehicle = true, OtIndex = 40, TPage = 0x154
    });
    Replay("_deferredVehicleSubtractive", 4, [6]);
    var afterShadow = Capture("subtractive-two-pass", rt);
    Require((Pixel(afterShadow, 12, 27) & 0x03e0) < (Pixel(afterRoad, 12, 27) & 0x03e0) &&
        DepthStencil(afterShadow, 12, 27) == DepthStencil(afterRoad, 12, 27),
        "subtractive vehicle shadow failed to darken terrain without replacing its depth/stencil");

    // A tiny synthetic XWAT chunk exercises the genuine surface upload and
    // authored base-quad replacement (neither uses the submitted PS1 XY).
    byte[] xwat =
    [
        (byte)'X', (byte)'W', (byte)'A', (byte)'T', 0, 0, 0, 40,
        0x10, 0, 0, 0, 0x08, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0,
        0, 0, 0xff, 0x83, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0x11, 0x11, 0x11, 0x11
    ];
    Require(GpuHle.TryRegisterDreamcastWaterTexture("synthetic", xwat), "fixture XWAT rejected");
    var baseQuad = new GpuHle.DreamcastWaterBaseQuad(new(44, 5, 900), new(60, 5, 900),
        new(44, 18, 900), new(60, 18, 900), 0, 0, 255);
    var mesh = new GpuHle.DreamcastWaterSurfaceMesh(
        [Wave(45, 8), Wave(58, 8), Wave(45, 21), Wave(58, 8), Wave(58, 21), Wave(45, 21)],
        2, 2, GpuHle.CurrentDreamcastWaterTexture);
    long surfaceReplacements = (long)Field("_traceDreamcastWaterSurfaceReplacements");
    Quad(46, 26, 60, 32, 900, 255, 0, 0, new PrimFlags
    {
        Material = HleMaterialKind.WaterBase, SemiTrans = true, OtIndex = 40,
        WaterBaseGroup = 0x20000, WaterBaseQuad = baseQuad, WaterSurfaceMesh = mesh
    });
    Replay("_deferredWater", 4, [6, 6]);
    var afterWater = Capture("authored-base-and-xwat", rt);
    Require((long)Field("_traceDreamcastWaterSurfaceReplacements") == surfaceReplacements + 1 &&
        Pixel(afterWater, 49, 7) != Pixel(afterShadow, 49, 7) &&
        (Pixel(afterWater, 48, 19) & 0x03ff) != 0 &&
        Pixel(afterWater, 59, 26) == Pixel(afterShadow, 59, 26) &&
        (DepthStencil(afterWater, 48, 19) >> 8) < (DepthStencil(afterShadow, 48, 19) >> 8),
        "authored base/surface pixels, source suppression or surface depth were not exercised");

    Quad(2, 50, 9, 57, 150, 0, 255, 0, new PrimFlags
    { Material = HleMaterialKind.ScreenEffect, SemiTrans = true, OtIndex = 40, TPage = 0x20 });
    Replay("_deferredScreenEffects", 2, [6]);
    var afterEffect = Capture("screen-effect", rt);
    Require(Pixel(afterEffect, 4, 53) != Pixel(afterWater, 4, 53), "deferred screen effect did not draw");
    var present = backend.PresentDisplay(0, 0, W, H);
    Require(present.tex != 0 && present.w == W && present.h == H, "present did not use intended extent");
    gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, (uint)Field("_presentFbo"));
    Console.WriteLine($"OUTPUT presentation rgba={Hash(ReadColor())}");
    CheckStorage();
    V82Compat.EndWorldRenderTerrainCache(cpu, memory);
    Require(V82Compat.DirectSceneRendering == direct, "the trial changed direct terrain state");
    Require((nonzero > 100) == (trial || direct) && zero >= (trial ? 5 : 1), "stream/legacy coverage missing");
    NoGlError("end of replay");
    Console.WriteLine($"PASS samples={samples} mode={args[0]} direct={direct} capacity={capacity} stride={stride} " +
        $"uploads={uploads} nonzero={nonzero} zero={zero} wraps={wraps} native=0x{native[0]:x4} " +
        $"check-mask-unprotected=0x{Pixel(mask, 54, 31):x4}");
    return 0;
}
finally
{
    gl.DeleteQuery(query);
    backend.Dispose();
}

void Flush(string label, int n, bool eligible = true)
{
    int before = Cursor();
    Require((int)countField.GetValue(backend)! == n, $"{label}: unexpected queued batch size");
    backend.Flush();
    VerifyUploads(label, before, eligible, [n]);
}

void VerifyUploads(string label, int cursor, bool eligible, int[] sizes)
{
    foreach (int n in sizes)
    {
        bool stream = direct || trial && eligible;
        int start = stream && cursor + n <= capacity ? cursor : 0;
        if (stream && start == 0 && cursor != 0) wraps++;
        if (start == 0) zero++; else nonzero++;
        uploads++;
        cursor = start + n;
    }
    Require(Cursor() == cursor, $"{label}: upload ended at {Cursor()}, expected {cursor}");
    Require(V82Compat.DirectSceneRendering == direct, $"{label}: direct terrain unexpectedly changed");
}

// A UI packet causes the real deferred replay. It remains queued while the
// primitive count and streaming positions are checked for the deferred draws.
void Replay(string deferred, uint prims, int[] sizes)
{
    Require(((ICollection)Field(deferred)).Count > 0 && (int)countField.GetValue(backend)! == 0,
        $"{deferred}: entry did not reach the expected queue");
    int cursor = Cursor();
    if (trial || direct)
        Require(cursor > 0 && cursor + sizes.Sum() <= capacity, $"{deferred}: fixture missed nonzero offset");
    gl.BeginQuery(QueryTarget.PrimitivesGenerated, query);
    Sprite(61, 61, 1, 1, 255, 255, 255);
    gl.EndQuery(QueryTarget.PrimitivesGenerated);
    gl.GetQueryObject(query, QueryObjectParameterName.Result, out uint count);
    Require(count == prims && ((ICollection)Field(deferred)).Count == 0,
        $"{deferred}: expected {prims} submitted triangles, got {count}");
    VerifyUploads(deferred, cursor, true, sizes);
    Flush("UI after " + deferred, 6);
    NoGlError(deferred);
}

void CheckStorage()
{
    Require((uint)Field("_vbo") == vbo, "primitive VBO ID changed");
    gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
    gl.GetBufferParameter(GLEnum.ArrayBuffer, GLEnum.BufferSize, out int size);
    Require(size == capacity * stride, "primitive VBO storage grew");
    NoGlError("fixed storage");
}

(ushort[] Vram, uint[] DepthStencil) Capture(string label, GlDisplayRt rt)
{
    rt.Resolve(gl);
    if (rt.Samples != 0)
    {
        // Same format and extent: read one resolved depth/stencil sample
        // per pixel, not every MSAA sample.
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, rt.DrawFbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, rt.Fbo);
        gl.BlitFramebuffer(0, 0, W, H, 0, 0, W, H,
            ClearBufferMask.DepthBufferBit | ClearBufferMask.StencilBufferBit, BlitFramebufferFilter.Nearest);
    }
    gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, rt.Fbo);
    byte[] color = ReadColor();
    uint[] depth = new uint[W * H];
    gl.ReadPixels<uint>(0, 0, W, H, PixelFormat.DepthStencil, PixelType.UnsignedInt248, depth.AsSpan());
    ushort[] nativePixels = new ushort[W * H];
    backend.ReadVram(0, 0, W, H, nativePixels);
    Console.WriteLine($"OUTPUT {label} rgba={Hash(color)} depth-stencil={Hash(MemoryMarshal.AsBytes(depth.AsSpan()))} " +
        $"vram={Hash(MemoryMarshal.AsBytes(nativePixels.AsSpan()))}");
    NoGlError(label);
    return (nativePixels, depth);
}

byte[] ReadColor()
{
    byte[] pixels = new byte[W * H * 4];
    gl.ReadPixels<byte>(0, 0, W, H, PixelFormat.Rgba, PixelType.UnsignedByte, pixels.AsSpan());
    return pixels;
}

object Field(string name) => backendType.GetField(name, Instance)!.GetValue(backend)!;
int Cursor() => (int)cursorField.GetValue(backend)!;
void NoGlError(string label)
{
    var error = gl.GetError();
    Require(error == GLEnum.NoError, $"{label}: OpenGL error {error}");
}
static string Hash(ReadOnlySpan<byte> bytes) => Convert.ToHexString(SHA256.HashData(bytes));
static void Require(bool ok, string message)
{
    if (!ok) throw new InvalidOperationException(message);
}
static ushort Pixel((ushort[] Vram, uint[] DepthStencil) image, int x, int y) => image.Vram[y * W + x];
static uint DepthStencil((ushort[] Vram, uint[] DepthStencil) image, int x, int y) => image.DepthStencil[y * W + x];
static GpuHle.DreamcastWaterSurfaceVertex Wave(float x, float y) =>
    new(x, y, 800, x / 64, y / 64, 0, 0, 800, 32, 32, 64);
void Sprite(int x, int y, int w, int h, byte r, byte g, byte b) =>
    backend.DrawRect(new HleRect { X = x, Y = y, W = w, H = h, R = r, G = g, B = b }, ui);
void Quad(int x, int y, int r, int b, float z, byte red, byte green, byte blue, PrimFlags f)
{
    HleVertex Vertex(int sx, int sy) => new()
    {
        X = sx, Y = sy, Z = z, ViewX = (sx - 32) * z / 64, ViewY = (sy - 32) * z / 64, ViewZ = z,
        ProjectionCenterX = 32, ProjectionCenterY = 32, ProjectionScale = 64,
        PerspectiveW = z, HasViewSpace = true, HasGteZ = true, HasCoherentGteZ = true, HasProjectiveW = true,
        R = red, G = green, B = blue
    };
    var a = Vertex(x, y); var d = Vertex(r, y); var c = Vertex(x, b); var e = Vertex(r, b);
    backend.DrawTri(a, d, c, f);
    backend.DrawTri(d, e, c, f);
}
