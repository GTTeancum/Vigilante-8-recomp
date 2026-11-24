using System.Text;
using RecompOne.Runtime;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;

// Normal case: one genuine host startup in a clean, asset-free directory.
// The rejection case uses an isolated build with a rejected prim shader, not
// an environment variable capable of changing the shipping shader.
if (args.Length > 1 || args.Length == 1 && args[0] is not ("--expect-rejected" or "--developer-oracle"))
    return 2;
bool rejected = args is ["--expect-rejected"];
bool oracle = args is ["--developer-oracle"];
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
using (var card = File.Create(Path.Combine(data.FullName, "test_loading_card_4x.ppm")))
{
    card.Write(Encoding.ASCII.GetBytes("P6\n1280 384\n255\n"));
    card.SetLength(card.Length + 1280 * 384 * 3);
}

Runtime.Initialize("Vigilante 8: 2nd Offense");
try
{
    var backend = GpuHle.Backend as EnhancedGlBackend;
    var gpu = Runtime.Gpu = new Gpu();
    gpu.WriteGp0(0xe3000000); gpu.WriteGp0(0xe4000000 | (63u << 10) | 63);
    gpu.WriteGp0(0x600000ff); gpu.WriteGp0((8u << 16) | 8); gpu.WriteGp0((16u << 16) | 16);
    gpu.WriteGp0(0xc0000000); gpu.WriteGp0((12u << 16) | 12); gpu.WriteGp0(0x00010001);
    uint pixel = gpu.ReadData() & 0x7fff;
    GpuHle.GameplayActive = true;
    var m = new PSMemory();
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
        "[GlBackend] driver=", "loaded 1 loading card overlays",
        "[GlStartup] sizing presentation targets", "[GlStartup] enhanced VRAM",
        "[GlStartup] prim VertexShader", "[GlStartup] prim FragmentShader", "[GlStartup] prim link",
        rejected ? "[GlBackend] enhanced renderer unavailable" : "[GlStartup] enhanced draw resources"
    };
    int i = 0;
    foreach (string text in checkpoints)
    {
        i = s.IndexOf(text, i, StringComparison.Ordinal);
        if (i < 0) throw new Exception($"Missing or out-of-order startup checkpoint: {text}");
        i += text.Length;
    }
    if (rejected && !s.Contains("[GlBackend] link failed (prim):"))
        throw new Exception("Negative control did not reject the primitive program.");
    if (s.Contains("window unavailable")) throw new Exception("Unexpected headless startup");
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
