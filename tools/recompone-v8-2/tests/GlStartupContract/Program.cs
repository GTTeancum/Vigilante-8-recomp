using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using Silk.NET.GLFW;
using Silk.NET.OpenGL;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

// Normal case: one genuine host startup in a clean, asset-free directory.
// The rejection case uses an isolated build with a rejected prim shader, not
// an environment variable capable of changing the shipping shader.
if (args.Length > 1 || args.Length == 1 && args[0] is not
    ("--expect-rejected" or "--developer-oracle" or "--render-storage"))
    return 2;
bool rejected = args is ["--expect-rejected"];
bool oracle = args is ["--developer-oracle"];
bool renderStorage = args is ["--render-storage"];
bool expectReady = !rejected, expectActive = !rejected && !oracle;
var data = Directory.CreateTempSubdirectory("v8-gl-startup-");
string log = Path.Combine(Path.GetTempPath(), $"{data.Name}.log");
foreach (string key in new[] { "RECOMPONE_USER_DATA_DIR", "RECOMPONE_MOD_DIR", "RECOMPONE_LOADING_CARD_DIR", "RECOMPONE_TEXTURE_PACK_DIR" })
    Environment.SetEnvironmentVariable(key, data.FullName);
Environment.SetEnvironmentVariable("RECOMPONE_LOG_PATH", log);
Environment.SetEnvironmentVariable("RECOMPONE_HEADLESS", "0");
Environment.SetEnvironmentVariable("RECOMPONE_WINDOW_VISIBLE", "0");
Environment.SetEnvironmentVariable("RECOMPONE_ORIGINAL_RENDERER_ORACLE", oracle ? "1" : "0");
Environment.SetEnvironmentVariable("RECOMPONE_GPU_HLE", oracle ? "0" : "1");
Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", "0");
Environment.SetEnvironmentVariable("SDL_AUDIODRIVER", "dummy");
File.WriteAllText(Path.Combine(data.FullName, "interface.ini"),
    "[RecompOne]\nFullscreen=False\nStartMaximized=False\nGraphicsPreset=Enhanced\n");
Environment.SetEnvironmentVariable("RECOMPONE_OUTPUT_RESOLUTION", "800x600");
Environment.SetEnvironmentVariable("RECOMPONE_PRESENTATION_RESOLUTION", "640x480");

byte[] scanline = new byte[1280 * 3];
for (int offset = 0; offset < scanline.Length; offset += 3)
{
    scanline[offset] = 23;
    scanline[offset + 1] = 179;
    scanline[offset + 2] = 207;
}
// Exercise the same startup pressure as a packaged game, but only with
// generated fixtures. One named card is selected and read back below.
int cards = renderStorage ? 19 : 1;
for (int i = 0; i < cards; i++)
{
    int height = i % 3 == 0 && i != 0 ? 448 : 384;
    string name = i == 0 ? "test" : $"fixture_{i}";
    using var card = File.Create(Path.Combine(data.FullName, name + "_loading_card_4x.ppm"));
    card.Write(Encoding.ASCII.GetBytes($"P6\n1280 {height}\n255\n"));
    for (int row = 0; row < height; ++row) card.Write(scanline);
}

// Optional Linux CI surface; GLFW still creates a real desktop OpenGL 4.5
// context via Mesa OSMesa and runs the same runtime and ImGui draw calls.
if (OperatingSystem.IsLinux() &&
    Environment.GetEnvironmentVariable("V8_GL_TEST_OFFSCREEN") == "1")
    Glfw.GetApi().InitHint((InitHint)0x00050003, 0x00060005); // GLFW_PLATFORM, GLFW_PLATFORM_NULL

long before = GC.GetTotalAllocatedBytes(precise: true);
Runtime.Initialize("Vigilante 8: 2nd Offense PC");
Console.WriteLine($"[StartupContract] managed initialization bytes={GC.GetTotalAllocatedBytes(precise: true) - before}");
try
{
    var backend = GpuHle.Backend as EnhancedGlBackend;
    var m = new PSMemory();
    var gpu = Runtime.Gpu!;
    if (renderStorage) CheckBeforeFirstDraw(backend!);
    gpu.WriteGp0(0xe3000000); gpu.WriteGp0(0xe4000000 | (63u << 10) | 63);
    gpu.WriteGp0(0x600000ff); gpu.WriteGp0((8u << 16) | 8); gpu.WriteGp0((16u << 16) | 16);
    gpu.WriteGp0(0xc0000000); gpu.WriteGp0((12u << 16) | 12); gpu.WriteGp0(0x00010001);
    uint pixel = gpu.ReadData() & 0x7fff;
    GpuHle.GameplayActive = true;
    var c = new CpuContext { GP = 0x80070000, A0 = 0x80020000, A1 = 0x80021000 };
    const uint original = 0x80030000;
    m.WriteU32(c.GP + 0x610, original);
    m.WriteU32(c.A1, 0x00ffffff);
    bool nativePanel = V82ModalStyle.DrawPanel(c, m);
    uint next = m.ReadU32(c.GP + 0x610);
    Console.WriteLine($"readiness={backend?.Ready} advertised={GpuHle.Active} native-panel={nativePanel} " +
        $"allocator=0x{next:X8} pixel={pixel:x4} log={log}");
    if (backend?.Ready != expectReady || GpuHle.Active != expectActive || pixel != 0x001f ||
        nativePanel == expectActive || next != (expectActive ? original + 20 : original))
        throw new Exception("Actual renderer and native gameplay hooks disagree.");

    // Read the same auto-flushed on-disk log a player can attach; not a
    // deferred string buffer which could vanish when a driver terminates.
    string s = File.ReadAllText(log);
    string[] checkpoints = {
        "[GlBackend] driver=", "[GlStartup] enhanced VRAM ",
        "[GlStartup] prim VertexShader", "[GlStartup] prim FragmentShader", "[GlStartup] prim link",
        rejected ? "[GlBackend] enhanced renderer unavailable" : "[GlStartup] enhanced VRAM storage",
        $"loaded {cards} loading card overlays"
    };
    int i = 0;
    foreach (string text in checkpoints)
    {
        i = s.IndexOf(text, i, StringComparison.Ordinal);
        if (i < 0) throw new Exception($"Missing or out-of-order startup checkpoint: {text}");
        i += text.Length;
    }
    if (rejected && (!s.Contains("[GlBackend] link failed (prim):") ||
        s.Contains("[GlStartup] enhanced VRAM storage")))
        throw new Exception("Primitive link rejection did not avoid enhanced VRAM allocation.");
    if (s.Contains("window unavailable")) throw new Exception("Unexpected headless startup");
    if (renderStorage) CheckEnhancedOutput(gpu, m, backend!);
    Console.WriteLine($"PASS ready={expectReady} active={expectActive} GP0 readback, native hook and persistent diagnostics");
    return 0;
}
catch (Exception e)
{
    Console.Error.WriteLine($"FAIL: {e.Message}");
    return 1;
}
finally
{
    Runtime.Shutdown();
    data.Delete(recursive: true);
}

static object Host(string field)
{
    var type = typeof(Runtime).Assembly.GetType("RecompOne.Runtime.Host.HostWindow")!;
    return type.GetField(field, BindingFlags.Static | BindingFlags.NonPublic)!.GetValue(null)!;
}

static uint Handle(object value, string field) =>
    (uint)value.GetType().GetField(field, BindingFlags.Instance | BindingFlags.NonPublic)!
        .GetValue(value)!;

static void CheckBeforeFirstDraw(EnhancedGlBackend backend)
{
    var gl = (GL)Host("_gl");
    if (Handle(backend, "_presentFbo") != 0 ||
        Handle(Host("_presentationRenderer"), "_upscaleFbo") != 0 ||
        Handle(Host("_presentationRenderer"), "_fxaaFbo") != 0)
        throw new Exception("Output FBOs were allocated before the first display.");
    var pixel = new ushort[] { 0xffff };
    backend.ReadVram(1023, 511, 1, 1, pixel);
    if (pixel[0] != 0 || gl.GetError() != GLEnum.NoError)
        throw new Exception("Untouched enhanced VRAM was not zero-initialized.");
    gl.GetInteger(GLEnum.MaxTextureSize, out int textureLimit);
    int savedScale = GlVram.Scale;
    var oversized = new GlVram(gl);
    try
    {
        GlVram.Scale = textureLimit / Gpu.VramWidth + 1;
        try
        {
            oversized.Init();
            throw new Exception("Oversized VRAM was accepted.");
        }
        catch (InvalidOperationException e) when (e.Message.Contains("OpenGL limit"))
        {
            if (oversized.Texture != 0 || oversized.Fbo != 0 || gl.GetError() != GLEnum.NoError)
                throw new Exception("Rejected VRAM dimensions touched native storage.");
        }
    }
    finally
    {
        GlVram.Scale = savedScale;
        oversized.Dispose();
    }
}

static unsafe void CheckEnhancedOutput(Gpu gpu, IMemory mem, EnhancedGlBackend backend)
{
    var gl = (GL)Host("_gl");
    object presenter = Host("_presentationRenderer");
    var rgba = new byte[640 * 480 * 4];
    const int width = 96, height = 72;
    gpu.WriteGp1(0x06000000 | (uint)((width * 10) << 12));
    gpu.WriteGp1(0x07000000 | (uint)(height << 10));
    gpu.WriteGp1(0x08000000);
    gpu.WriteGp1(0x03000000);
    if (gpu.DisplayWidth != width || gpu.DisplayHeight != height)
        throw new Exception("Wrong test display dimensions.");
    Runtime.View.Widescreen = false;
    gpu.WriteGp0(0xe3000000);
    gpu.WriteGp0(0xe4000000 | ((uint)(height - 1) << 10) | (uint)(width - 1));
    gpu.WriteGp0(0x600000ff); // untextured opaque red rectangle through the GL primitive program
    gpu.WriteGp0((8u << 16) | 8);
    gpu.WriteGp0((32u << 16) | 32);
    backend.Flush();
    var vram = new ushort[2];
    backend.ReadVram(20, 20, 1, 1, vram.AsSpan(0, 1));
    backend.ReadVram(80, 64, 1, 1, vram.AsSpan(1));
    if ((vram[0] & 0x7fff) != 0x001f || vram[1] != 0)
        throw new Exception("GPU draw did not reach enhanced VRAM.");

    void Get(uint texture, string phase)
    {
        if (texture == 0) throw new Exception($"{phase} texture is absent.");
        gl.PixelStore(PixelStoreParameter.PackAlignment, 1);
        fixed (byte* dst = rgba)
            gl.GetTextureImage(texture, 0, PixelFormat.Rgba, PixelType.UnsignedByte, (uint)rgba.Length, dst);
        var error = gl.GetError();
        if (error != GLEnum.NoError)
            throw new Exception($"{phase} OpenGL status was {error}.");
        Console.WriteLine($"[StartupContract] {phase} RGBA SHA256={Convert.ToHexString(SHA256.HashData(rgba))}");
    }

    void Pixel(int x, int y, byte r, byte g, byte b)
    {
        int offset = (y * 640 + x) * 4;
        if (rgba[offset] != r || rgba[offset + 1] != g || rgba[offset + 2] != b)
            throw new Exception($"Pixel {x},{y}: expected {r}/{g}/{b}, " +
                $"got {rgba[offset]}/{rgba[offset + 1]}/{rgba[offset + 2]}.");
    }

    const uint name = 0x80070000;
    foreach (var (value, i) in Encoding.ASCII.GetBytes("LEVELS/TEST.BIN\0").Select((b, i) => (b, i)))
        mem.WriteU8(name + (uint)i, value);
    V82ArenaRegistry.CaptureLevelLoad(new CpuContext { A0 = name }, mem);
    GpuHle.GameplayActive = true;
    GpuHle.DebugGameplayTick = 0;
    Runtime.PresentFrame();
    Get(Handle(presenter, "_upscaleTexture"), "loading card");
    Pixel(320, 140, 23, 179, 207);
    Pixel(600, 450, 0, 0, 0);
    if (Handle(presenter, "_fxaaFbo") != 0)
        throw new Exception("FXAA framebuffer allocated for loading artwork that doesn't use it.");
    V82ArenaRegistry.EndLevelLoad();

    // Enter gameplay. No mode switch, software renderer, or disabled AA.
    GpuHle.DebugGameplayTick = 1;
    Runtime.PresentFrame();
    Get(Handle(presenter, "_fxaaTexture"), "gameplay FXAA");
    Pixel(134, 134, 255, 0, 0);
    Pixel(600, 450, 0, 0, 0);
    if (!GpuHle.Active || GlVram.Scale != 4 || Runtime.View.MsaaSamples != 2 ||
        !Runtime.View.HighResolutionTextures)
        throw new Exception("Graphics quality changed to pass startup.");

    // Exercise enhanced 24-bit display and a real output reallocation too.
    // Each pair of RGB pixels spans exactly three guest 16-bit words.
    const int fmWidth = 100, fmHeight = 68, fmWords = fmWidth * 3 / 2;
    gpu.WriteGp1(0x06000000 | (uint)((fmWidth * 10) << 12));
    gpu.WriteGp1(0x07000000 | (uint)(fmHeight << 10));
    gpu.WriteGp1(0x08000010);
    ushort[] movieWords = new ushort[fmWords * fmHeight];
    for (int i = 0; i < movieWords.Length; i += 3)
    {
        movieWords[i] = 0x5634; movieWords[i + 1] = 0x349a; movieWords[i + 2] = 0x9a56;
    }
    backend.WriteVram(0, 0, fmWords, fmHeight, movieWords);
    GpuHle.GameplayActive = false;
    Runtime.PresentFrame();
    Get(Handle(presenter, "_upscaleTexture"), "resized 24-bit output");
    Pixel(320, 240, 0x34, 0x56, 0x9a);

    Console.WriteLine("PASS enhanced zeroed VRAM, real GP0 mesh, HD card, FXAA, and resized 24-bit output");
}
