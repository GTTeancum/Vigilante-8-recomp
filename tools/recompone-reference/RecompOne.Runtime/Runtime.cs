using RecompOne.Runtime.Context;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime;

public enum RunMode { Retail, Devkit }

public static class Runtime
{
    static bool? _lastDisplayEnabled;
    static int _presentTraceCount;
    static readonly bool TraceVSync =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VSYNC") == "1";
    public static CpuContext? Cpu { get; private set; }
    public static IMemory? Mem { get; private set; }
    public static Gpu? Gpu;
    public static Spu? Spu;
    public static Cdrom.CdController? Cd;
    public static string GameTitle { get; private set; } = "";

    public static RunMode Mode { get; private set; } = RunMode.Retail;
    public static void ConfigureGameTitle(string title) => GameTitle = title;
    public static void SetMode(RunMode mode) => Mode = mode; //devkit vs retail, devkits reads from sim and has more ram
    public static string CdPath => Config.ConfigManager.Game.CdPath;
    static string? _activeLoosePath;
    public static void SetLoosePath(string? path) =>
        _activeLoosePath = string.IsNullOrWhiteSpace(path)
            ? null
            : Path.GetFullPath(path);

    public static string? ResolveLoosePath()
    {
        if (_activeLoosePath != null && Directory.Exists(_activeLoosePath))
            return _activeLoosePath;
        string? configured = Environment.GetEnvironmentVariable("RECOMPONE_LOOSE_DIR");
        if (configured == "0") return null;
        string candidate = string.IsNullOrWhiteSpace(configured)
            ? AppContext.BaseDirectory
            : Path.GetFullPath(configured);
        if (string.IsNullOrWhiteSpace(configured) &&
            !File.Exists(Path.Combine(candidate, "SYSTEM.CNF")))
            return null;
        return Directory.Exists(candidate) ? candidate : null;
    }
    
    public static Config.ViewConfig View => Config.ConfigManager.View;
    public static void SaveView() => Config.ConfigManager.SaveView(Host.Window.PanelManager.Panels);
    
    public static Hardware.MemoryCard CardA = new("carda.sav") { Enabled = true };
    public static Hardware.MemoryCard CardB = new("cardb.sav") { Enabled = true };
    public static readonly Memory.RamLogger RamLog = new();
    public static readonly Dispatch.OverlayEventLog OverlayLog = new();

    public static void Initialize(string title)
    {
        ConfigureGameTitle(title);
        Diagnostics.ConsoleMirror.Install();
        HostWindow.Initialize(title);
        Audio.Initialize();
        bool forceMute = Environment.GetEnvironmentVariable("RECOMPONE_MUTE") == "1";
        Audio.SetMasterVolume(forceMute || Config.ConfigManager.Game.Muted
            ? 0f
            : Config.ConfigManager.Game.MasterVolume);
    }

    public static void WaitForValidDisc() => HostWindow.WaitForValidDisc();

    public static void SetContext(CpuContext c, IMemory m)
    {
        Cpu = c;
        Mem = m;
    }

    public static void PresentFrame()
    {
        int traceFrame = _presentTraceCount++;
        if (TraceVSync && traceFrame < 10) Console.Error.WriteLine($"[VSync] present {traceFrame}: window");
        if (Gpu != null && _lastDisplayEnabled != Gpu.DisplayEnabled)
        {
            _lastDisplayEnabled = Gpu.DisplayEnabled;
            Console.WriteLine($"[GPU] display={Gpu.DisplayEnabled} area={Gpu.DisplayX},{Gpu.DisplayY} {Gpu.DisplayWidth}x{Gpu.DisplayHeight} hle={Hle.GpuHle.Active}");
        }
        HostWindow.Present(Gpu);
        Gpu.EndProjectionFrame(Hle.GpuHle.GameplayActive);
        // GTE-to-GPU depth correlation is needed only within the frame whose
        // geometry was just submitted. Clearing here prevents stale screen
        // coordinates from being associated with a later camera view.
        Gte.BeginFrame();
        if (TraceVSync && traceFrame < 10) Console.Error.WriteLine($"[VSync] present {traceFrame}: audio");
        Audio.Attach(Spu);
        FrameClock.Throttle();
        if (TraceVSync && traceFrame < 10) Console.Error.WriteLine($"[VSync] present {traceFrame}: devices");
        Sdk.LibCd.Tick();
        if (Mem != null) { Bios.BiosB.RefreshPad(Mem); Sdk.LibPad.Refresh(Mem); } //is this correct?
        if (TraceVSync && traceFrame < 10) Console.Error.WriteLine($"[VSync] present {traceFrame}: irq");
        DispatchIrq(0); //using this to dispatch irqs too if necessary, probably not needed after the rest of stuff is reimplemented
        if (TraceVSync && traceFrame < 10) Console.Error.WriteLine($"[VSync] present {traceFrame}: done");
    }

    public static void DispatchIrq(int irq)
    {
        if (Cpu != null && Mem != null)
            Interrupts.Deliver(irq, Cpu, Mem);
    }

    public static void Shutdown()
    {
        Audio.Shutdown();
        HostWindow.Shutdown();
    }
}
