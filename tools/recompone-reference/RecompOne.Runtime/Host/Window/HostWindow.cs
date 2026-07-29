using System.Numerics;
using ImGuiNET;
using Silk.NET.Input;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.OpenGL.Extensions.ImGui;
using Silk.NET.Windowing;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host.Window;

namespace RecompOne.Runtime.Host;

internal static class HostWindow
{
    static IWindow? _window;
    static GL? _gl;
    static ImGuiController? _imgui;
    static bool _headless;
    static Gpu? _gpu;

    static uint _displayTex;
    static uint _vramTex;
    static uint _ramTex;
    static Hle.GlBackend? _glBackend;
    static PresentationRenderer? _presentationRenderer;

    static byte[] _rgbDisplay = [];
    static ushort[] _hleDisplay = [];
    static byte[] _rgbVram = [];
    static byte[] _ramFront = new byte[Memory.RamLogger.Width * Memory.RamLogger.Height * 4];
    static byte[] _ramBack = new byte[Memory.RamLogger.Width * Memory.RamLogger.Height * 4];
    static Task? _ramTask;
    static volatile bool _ramReady;
    static int _ramFrame;
    static int _displayProbeFrame;
    static uint _lastDisplayHash;
    static string? _requestedDisplayCapture;
    static string? _pendingPresentationCapture;
    static readonly string? _outputResolutionOverride =
        Environment.GetEnvironmentVariable("RECOMPONE_OUTPUT_RESOLUTION");
    static readonly string? _antiAliasingOverride =
        Environment.GetEnvironmentVariable("RECOMPONE_ANTI_ALIASING");
    static readonly string? _presentationResolutionOverride =
        Environment.GetEnvironmentVariable("RECOMPONE_PRESENTATION_RESOLUTION");
    static readonly int _presentationCaptureFrame =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_PRESENTATION_CAPTURE_FRAME"), out int captureFrame)
            ? Math.Max(1, captureFrame)
            : 0;
    static readonly HashSet<int> _presentationCaptureFrames =
        (Environment.GetEnvironmentVariable("RECOMPONE_PRESENTATION_CAPTURE_FRAMES") ?? "")
        .Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
        .Select(value => int.TryParse(value, out int frame) ? frame : 0)
        .Where(frame => frame > 0)
        .ToHashSet();
    static int _presentationFrame;
    static readonly bool _capturePresentation =
        Environment.GetEnvironmentVariable("RECOMPONE_PRESENTATION_CAPTURE") == "1";
    static readonly bool _windowVisible =
        Environment.GetEnvironmentVariable("RECOMPONE_WINDOW_VISIBLE") != "0";
    static readonly int _displayProbeInterval =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_DISPLAY_PROBE_INTERVAL"), out int interval)
            ? Math.Max(1, interval)
            : 0;

    static bool _layoutPending = true;
    static bool _closed;
    static DiscPickerPopup? _discPicker;

    public static void Initialize(string title)
    {
        ConfigManager.Load();
        var outputSize = ParseOutputResolution(_outputResolutionOverride ?? ConfigManager.View.OutputResolution);

        try
        {
            var options = WindowOptions.Default with
            {
                Size = new Vector2D<int>(outputSize.width, outputSize.height),
                Title = title,
                IsVisible = _windowVisible,
                VSync = false,
                UpdatesPerSecond = 0,
                FramesPerSecond = 0,
                WindowState = ConfigManager.View.Fullscreen ? WindowState.Fullscreen : WindowState.Normal,
                API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5)),
            };
            _window = Silk.NET.Windowing.Window.Create(options);
            _window.Load += OnLoad;
            _window.Render += OnRender;
            _window.Closing += OnClosing;
            _window.Initialize();
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"[Host] window unavailable {e.Message}");
            _headless = true;
        }
    }

    public static void Present(Gpu? gpu)
    {
        _gpu = gpu;
        if (_headless || _window == null) return;
        try { _window.DoEvents(); }
        catch (Exception e) {
            Console.WriteLine(e.Message);
        }
        if (_window.IsClosing) { Runtime.Shutdown(); Environment.Exit(0); }
        InputManager.Poll();
        if (InputManager.ConsumeTopBarToggle())
        {
            ConfigManager.View.HideTopBar = !ConfigManager.View.HideTopBar;
            ConfigManager.SaveView(PanelManager.Panels);
        }
        if (InputManager.ConsumeFullscreenToggle())
        {
            ConfigManager.View.Fullscreen = !ConfigManager.View.Fullscreen;
            SetFullscreen(ConfigManager.View.Fullscreen);
            ConfigManager.SaveView(PanelManager.Panels);
        }
        _window.DoRender();
    }

    internal static void Pump()
    {
        if (_headless || _window == null) return;
        try { _window.DoEvents(); } catch { }
        if (_window.IsClosing) { Runtime.Shutdown(); Environment.Exit(0); }
        _window.DoRender();
    }

    public static void Shutdown()
    {
        if (!_headless && _window != null && !_window.IsClosing)
            _window.Close();
        InputManager.Shutdown();
    }

    public static void SetFullscreen(bool on)
    {
        if (_window == null) return;
        _window.WindowState = on ? WindowState.Fullscreen : WindowState.Normal;
        if (!on)
        {
            var size = ParseOutputResolution(ConfigManager.View.OutputResolution);
            _window.Size = new Vector2D<int>(size.width, size.height);
        }
    }

    public static void SetOutputResolution(string resolution)
    {
        if (_window == null || ConfigManager.View.Fullscreen) return;
        var size = ParseOutputResolution(resolution);
        _window.Size = new Vector2D<int>(size.width, size.height);
    }

    static (int width, int height) ParseOutputResolution(string resolution)
    {
        string[] parts = resolution.Split('x', 'X');
        if (parts.Length == 2 &&
            int.TryParse(parts[0], out int width) &&
            int.TryParse(parts[1], out int height) &&
            width is >= 640 and <= 7680 && height is >= 480 and <= 4320)
            return (width, height);
        return (1280, 720);
    }

    public static bool IsKeyDown(Key k) => InputManager.IsKeyDown(k);

    internal static void RequestDisplayCapture(string label)
    {
        string sanitized = new string(
            label.Where(ch => char.IsAsciiLetterOrDigit(ch) || ch == '_').ToArray());
        _requestedDisplayCapture = sanitized;
        if (_capturePresentation && Hle.GpuHle.Active)
            _pendingPresentationCapture = sanitized;
    }

    public static void RequestDiscPath() => _discPicker?.Show();

    public static void WaitForValidDisc() // wait for disc path to be valid before running it!!
    {
        if (_headless || _window == null) return;
        while (true)
        {
            var path = ConfigManager.Game.CdPath;
            if (!string.IsNullOrWhiteSpace(path) && File.Exists(path)) return;

            try { _window.DoEvents(); } catch { }
            if (_window.IsClosing) { Runtime.Shutdown(); Environment.Exit(0); }
            InputManager.Poll();
            _window.DoRender();
        }
    }

    static void OnLoad()
    {
        var input = _window!.CreateInput();
        InputManager.Initialize(input);

        _gl = GL.GetApi(_window);
        _gl.ClearColor(0.08f, 0.08f, 0.08f, 1f);

        var fb = _window!.FramebufferSize;
        _gl.Viewport(0, 0, (uint)fb.X, (uint)fb.Y);
        _window.FramebufferResize += size => _gl?.Viewport(0, 0, (uint)size.X, (uint)size.Y);
        _displayTex = CreateTexture(_gl);
        _vramTex= CreateTexture(_gl);
        _ramTex = CreateTexture(_gl);
        _presentationRenderer = new PresentationRenderer(_gl);
        _presentationRenderer.Initialize();

        string? graphicsPresetOverride =
            Environment.GetEnvironmentVariable("RECOMPONE_GRAPHICS_PRESET");
        if (graphicsPresetOverride is "Original" or "Enhanced")
            ConfigManager.View.ApplyGraphicsPreset(graphicsPresetOverride);
        string? widescreenOverride =
            Environment.GetEnvironmentVariable("RECOMPONE_WIDESCREEN");
        if (widescreenOverride is "0" or "1")
            ConfigManager.View.Widescreen = widescreenOverride == "1";
        string? smoothingOverride =
            Environment.GetEnvironmentVariable("RECOMPONE_TEXTURE_SMOOTHING");
        if (smoothingOverride is "0" or "1")
            ConfigManager.View.TextureSmoothing = smoothingOverride == "1";

        string? hleOverride = Environment.GetEnvironmentVariable("RECOMPONE_GPU_HLE");
        bool hleActive = hleOverride == "1" ||
            (hleOverride != "0" && ConfigManager.View.HighResolution3D);
        // A 2x native framebuffer (640x480 for normal gameplay) retains the
        // clean Dreamcast/PS2-class geometry target while avoiding the 16x
        // fragment load of the former 4x-per-axis path. Presentation remains
        // at the host resolution and MSAA still resolves sub-pixel edges.
        Hle.GlVram.Scale = ConfigManager.View.HighResolution3D ? 2 : 1;
        _glBackend = new Hle.GlBackend(_gl);
        _glBackend.InitGl();
        Hle.GpuHle.Active = hleActive;
        Hle.GpuHle.Backend = _glBackend;
        Hle.GpuHle.NativeResolution = false;
        ApplyGraphicsView();
        Console.WriteLine(
            $"[Host] PS1 color dithering={(ConfigManager.View.Ps1Dithering ? "On (fidelity)" : "Off (enhanced default)")}");
        Console.WriteLine(
            $"[Host] PS1 texture smoothing={(ConfigManager.View.TextureSmoothing ? "On (enhanced default)" : "Off (fidelity)")}");
        Console.WriteLine(
            $"[Host] PS1 texture projection fix={(ConfigManager.View.PerspectiveCorrectTextures ? "On (enhanced default)" : "Off (fidelity)")}");
        Console.WriteLine(
            $"[Host] graphics preset={ConfigManager.View.GraphicsPreset} " +
            $"wide={(ConfigManager.View.Widescreen ? "On" : "Off")} " +
            $"msaa={ConfigManager.View.MsaaSamples}x aniso={ConfigManager.View.AnisotropicFiltering}x " +
            $"mipmaps={(ConfigManager.View.TextureMipmaps ? "On" : "Off")} " +
            $"shadows={(ConfigManager.View.EnhancedShadows ? "Enhanced" : "Stock")} " +
            $"particles={(ConfigManager.View.EnhancedParticles ? "Enhanced" : "Stock")} " +
            $"draw-distance={(ConfigManager.View.ExtendedDrawDistance ? "Extended" : "Stock")} " +
            $"fog={(ConfigManager.View.EnhancedFog ? "Enhanced" : "Stock")}");

        _imgui = new ImGuiController(_gl, _window, input, null, ConfigureImGui);

        PanelManager.Register(new OutputPanel());
        PanelManager.Register(new VramViewerPanel());
        PanelManager.Register(new CpuStatePanel());
        PanelManager.Register(new RamMapPanel());
        PanelManager.Register(new MemoryEditorPanel());
        PanelManager.Register(new SpuViewerPanel());
        PanelManager.Register(new CdDebugPanel());
        PanelManager.Register(new ConsolePanel());
        PanelManager.Register(new OverlayEventsPanel());
        PanelManager.Register(new SettingsPopup());
        PanelManager.Register(new Modding.ModsPopup());
        PanelManager.Register(new AboutPopup());

        SettingsRegistry.Register(new InputSettingsSection());
        SettingsRegistry.Register(new DisplaySettingsSection());
        SettingsRegistry.Register(new AudioSettingsSection());
        if (Runtime.GameTitle.Contains("2nd Offense", StringComparison.Ordinal))
            MenuRegistry.Register("Cheats", V82CheatMenu.Draw);
        MenuRegistry.Register("Guest Vehicles", GuestVehicleMenu.Draw);
        if (Runtime.GameTitle.Equals(
                "Vigilante 8 PC", StringComparison.Ordinal))
            MenuRegistry.Register("Guest Arenas", GuestArenaMenu.Draw);

        _discPicker = new DiscPickerPopup();
        PanelManager.Register(_discPicker);

        ConfigManager.ApplyViewToPanels(PanelManager.Panels);

        var cdPath = ConfigManager.Game.CdPath;
        if (Runtime.ResolveLoosePath() == null &&
            (string.IsNullOrWhiteSpace(cdPath) || !File.Exists(cdPath)))
            _discPicker.Show();
    }

    static void ConfigureImGui()
    {
        var io = ImGui.GetIO();
        io.ConfigFlags |= ImGuiConfigFlags.DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        unsafe { io.NativePtr->IniFilename = null; }

        if (Config.ConfigManager.ApplyImGuiLayout())
            _layoutPending = false;
    }

    static void OnRender(double dt)
    {
        var gl = _gl!;
        ApplyGraphicsView();
        _imgui!.Update((float)dt);
    
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        var fbDef = _window!.FramebufferSize;
        gl.Viewport(0, 0, (uint)fbDef.X, (uint)fbDef.Y);
        gl.ClearColor(0.08f, 0.08f, 0.08f, 1f);
        gl.Clear(ClearBufferMask.ColorBufferBit);

        Runtime.RamLog.Tick();
        Memory.RamLogger.TrackReads =
            PanelManager.Get<RamMapPanel>()?.IsOpen == true ||
            PanelManager.Get<MemoryEditorPanel>()?.IsOpen == true;

        var gpu = _gpu;
        if (gpu != null)
        {

            if (Hle.GpuHle.Active && _glBackend is { Ready: true } && gpu.DisplayEnabled)
            {
                var wf = _window!.FramebufferSize;
                var (tex, tw, th, aspect) = _glBackend.PresentDisplay(
                    gpu.DisplayX, gpu.DisplayY,
                    gpu.DisplayWidth, gpu.DisplayHeight,
                    gpu.Display24Bit,
                    outW: wf.X, outH: wf.Y);
                ProbeHleDisplay(_glBackend, gpu, gpu.DisplayWidth, gpu.DisplayHeight);
                if (tex != 0) PresentTexture(gl, tex, tw, th, aspect);
                gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
                gl.Viewport(0, 0, (uint)wf.X, (uint)wf.Y);
            }
            else
            {
                UploadDisplayTexture(gl, gpu);
            }

            if (PanelManager.Get<VramViewerPanel>()?.IsOpen == true)
                UploadVramTexture(gl, gpu);
        }

        if (PanelManager.Get<RamMapPanel>()?.IsOpen == true)
        {
            QueueRamConvert();
            if (_ramReady) FlushRamTexture(gl);
        }

        if (!ConfigManager.View.HideTopBar)
            MainMenuBar.Draw();

        DrawDockspace();
        PanelManager.DrawPanels();
        MenuRegistry.DrawWindows();
        Modding.ModLoadingPopup.Draw();
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        gl.Viewport(0, 0, (uint)fbDef.X, (uint)fbDef.Y);
        _imgui.Render();
    }

    static void DrawDockspace()
    {
        var viewport = ImGui.GetMainViewport();
        ImGui.SetNextWindowPos(viewport.WorkPos);
        ImGui.SetNextWindowSize(viewport.WorkSize);
        ImGui.SetNextWindowViewport(viewport.ID);

        const ImGuiWindowFlags hostFlags = ImGuiWindowFlags.NoDocking | 
                                           ImGuiWindowFlags.NoTitleBar |
                                           ImGuiWindowFlags.NoCollapse |
                                           ImGuiWindowFlags.NoResize |
                                           ImGuiWindowFlags.NoMove |
                                           ImGuiWindowFlags.NoBringToFrontOnFocus |
                                           ImGuiWindowFlags.NoBackground;

        ImGui.PushStyleVar(ImGuiStyleVar.WindowRounding, 0f);
        ImGui.PushStyleVar(ImGuiStyleVar.WindowBorderSize, 0f);
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, Vector2.Zero);
        ImGui.Begin("##DockHost", hostFlags);
        ImGui.PopStyleVar(3);
        uint dockId = ImGui.GetID("##MainDock");
        int openCount = PanelManager.Panels.Count(p => p.IsOpen && p is not AboutPopup);
        var dockFlags = openCount <= 1 ? (ImGuiDockNodeFlags)4096 : ImGuiDockNodeFlags.None;
        ImGui.DockSpace(dockId, Vector2.Zero, dockFlags);

        if (_layoutPending)
        {
            _layoutPending = false;
            DockBuilder.SetupCenterLayout(dockId, viewport.WorkSize, "Output");
        }

        ImGui.End();
    }

    static void OnClosing()
    {
        if (_closed) return;
        _closed = true;
        ConfigManager.SaveView(PanelManager.Panels);
        ConfigManager.SaveGame();
        PanelManager.Shutdown();
        _glBackend?.Dispose();
        _presentationRenderer?.Dispose();
        _imgui?.Dispose();
        _gl?.DeleteTexture(_displayTex);
        _gl?.DeleteTexture(_vramTex);
        _gl?.DeleteTexture(_ramTex);
    }

    static uint CreateTexture(GL gl)
    {
        var tex = gl.GenTexture();
        gl.BindTexture(TextureTarget.Texture2D, tex);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Nearest);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Nearest);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        return tex;
    }

    static void UploadDisplayTexture(GL gl, Gpu gpu)
    {
        int w = gpu.DisplayWidth, h = gpu.DisplayHeight;
        if (!gpu.DisplayEnabled || w <= 0 || h <= 0) return;
        int needed = w * h * 3;
        if (_rgbDisplay.Length < needed) _rgbDisplay = new byte[needed];
        ConvertDisplay(gpu, w, h);
        ProbeDisplay(gpu, w, h);
        gl.BindTexture(TextureTarget.Texture2D, _displayTex);
        gl.TexImage2D<byte>(TextureTarget.Texture2D, 0, InternalFormat.Rgb, (uint)w, (uint)h, 0,
            PixelFormat.Rgb, PixelType.UnsignedByte, _rgbDisplay.AsSpan(0, needed));
        PresentTexture(gl, _displayTex, w, h, 4f / 3f);
    }

    static void ApplyGraphicsView()
    {
        // Only gameplay owns the expanded framebuffer. SHELL menus and FMVs
        // are authored at 4:3 and must remain pillarboxed rather than stretched.
        bool wide = ConfigManager.View.Widescreen && Hle.GpuHle.GameplayActive;
        Hle.GpuHle.WideAspect = wide ? 16f / 9f : 0f;
        Hle.GpuHle.TargetAspect = wide ? 16f / 9f : Hle.GpuHle.BaseAspect;
        Hle.GpuHle.OutputAspect = Hle.GpuHle.TargetAspect;
    }

    static void PresentTexture(GL gl, uint sourceTexture, int sourceWidth, int sourceHeight, float aspect)
    {
        var framebuffer = _window!.FramebufferSize;
        var output = OutputPanel.GetPresentationSize(aspect, framebuffer.X, framebuffer.Y);
        if (!string.IsNullOrWhiteSpace(_presentationResolutionOverride))
        {
            var forced = ParseOutputResolution(_presentationResolutionOverride);
            output = (forced.width, forced.height);
        }
        bool fxaa = (_antiAliasingOverride ?? ConfigManager.View.AntiAliasing)
            .Equals("FXAA", StringComparison.OrdinalIgnoreCase);
        uint texture = sourceTexture;
        if (_presentationRenderer is { Ready: true })
        {
            string? capture = _pendingPresentationCapture;
            _pendingPresentationCapture = null;
            ++_presentationFrame;
            if (_capturePresentation &&
                (_presentationFrame == _presentationCaptureFrame ||
                 _presentationCaptureFrames.Contains(_presentationFrame)))
                capture = $"frame_{_presentationFrame:000000}";
            texture = _presentationRenderer.Render(sourceTexture, sourceWidth, sourceHeight,
                output.w, output.h, fxaa, capture);
        }
        OutputPanel.SetTexture(texture, output.w, output.h, aspect);
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        gl.Viewport(0, 0, (uint)framebuffer.X, (uint)framebuffer.Y);
    }

    static void ProbeDisplay(Gpu gpu, int w, int h)
    {
        string? captureLabel = _requestedDisplayCapture;
        _requestedDisplayCapture = null;
        bool periodicProbe = _displayProbeInterval > 0 &&
            ++_displayProbeFrame % _displayProbeInterval == 1;
        if (!periodicProbe && string.IsNullOrEmpty(captureLabel)) return;

        RecordDisplayProbe(w, h, captureLabel);
    }

    static void ProbeHleDisplay(Hle.IGpuBackend backend, Gpu gpu, int w, int h)
    {
        string? captureLabel = _requestedDisplayCapture;
        _requestedDisplayCapture = null;
        bool periodicProbe = _displayProbeInterval > 0 &&
            ++_displayProbeFrame % _displayProbeInterval == 1;
        if (!periodicProbe && string.IsNullOrEmpty(captureLabel)) return;

        int pixels = w * h;
        if (_hleDisplay.Length < pixels) _hleDisplay = new ushort[pixels];
        if (_rgbDisplay.Length < pixels * 3) _rgbDisplay = new byte[pixels * 3];
        backend.ReadVram(gpu.DisplayX, gpu.DisplayY, w, h, _hleDisplay.AsSpan(0, pixels));
        for (int i = 0, o = 0; i < pixels; i++)
        {
            ushort px = _hleDisplay[i];
            _rgbDisplay[o++] = (byte)((px & 0x1F) << 3);
            _rgbDisplay[o++] = (byte)(((px >> 5) & 0x1F) << 3);
            _rgbDisplay[o++] = (byte)(((px >> 10) & 0x1F) << 3);
        }

        RecordDisplayProbe(w, h, captureLabel);
    }

    static void RecordDisplayProbe(int w, int h, string? captureLabel)
    {

        int pixels = w * h;
        int nonzero = 0;
        uint hash = 2166136261u;
        for (int i = 0; i < pixels; i++)
        {
            int o = i * 3;
            uint rgb = (uint)(_rgbDisplay[o] | (_rgbDisplay[o + 1] << 8) | (_rgbDisplay[o + 2] << 16));
            if (rgb != 0) nonzero++;
            hash = (hash ^ rgb) * 16777619u;
        }

        if (hash != _lastDisplayHash)
        {
            _lastDisplayHash = hash;
            Console.WriteLine($"[GPU] framebuffer {w}x{h} nonzero={nonzero} hash=0x{hash:X8}");
            WriteDisplayPpm("recompone_vram_latest.ppm", w, h, pixels);
        }

        if (!string.IsNullOrEmpty(captureLabel))
        {
            string path = $"recompone_capture_{captureLabel}.ppm";
            WriteDisplayPpm(path, w, h, pixels);
            Console.WriteLine($"[GPU] captured stage '{captureLabel}' to {path}");
            if (_capturePresentation)
                _pendingPresentationCapture = captureLabel;
        }
    }

    static void WriteDisplayPpm(string path, int w, int h, int pixels)
    {
        using var dump = File.Create(path);
        byte[] header = System.Text.Encoding.ASCII.GetBytes($"P6\n{w} {h}\n255\n");
        dump.Write(header);
        dump.Write(_rgbDisplay, 0, pixels * 3);
    }

    static ushort[] _vramView = new ushort[Gpu.VramWidth * Gpu.VramHeight];
    static void UploadVramTexture(GL gl, Gpu gpu)
    {
        const int sz = Gpu.VramWidth * Gpu.VramHeight * 3;
        if (_rgbVram.Length < sz) _rgbVram = new byte[sz];
        ushort[] src;
        if (Hle.GpuHle.Active && _glBackend is { Ready: true })
        {
            _glBackend.ReadVram(0, 0, Gpu.VramWidth, Gpu.VramHeight, _vramView);
            src = _vramView;
        }
        else src = gpu.Vram;
        ConvertVramToBuffer(src, _rgbVram);
        gl.BindTexture(TextureTarget.Texture2D, _vramTex);
        gl.TexImage2D<byte>(TextureTarget.Texture2D, 0, InternalFormat.Rgb, Gpu.VramWidth, Gpu.VramHeight, 0, PixelFormat.Rgb, PixelType.UnsignedByte, _rgbVram.AsSpan(0, sz));
        VramViewerPanel.SetTexture(_vramTex, Gpu.VramWidth, Gpu.VramHeight);
    }

    static void QueueRamConvert()
    {
        if (_ramTask is { IsCompleted: false }) return;
        if (++_ramFrame < 6) return;
        _ramFrame = 0;
        var psMem = Runtime.Mem as Memory.PSMemory;
        if (psMem == null) return;
        var ram = psMem.RamBuffer;
        var back = _ramBack;
        _ramTask = Task.Run(() => Runtime.RamLog.BuildTexture(ram, back))
            .ContinueWith(_ =>
            {
                (_ramFront, _ramBack) = (_ramBack, _ramFront);
                _ramReady = true;
            }, TaskContinuationOptions.ExecuteSynchronously);
    }

    static void FlushRamTexture(GL gl)
    {
        _ramReady = false;
        gl.BindTexture(TextureTarget.Texture2D, _ramTex);
        gl.TexImage2D<byte>(TextureTarget.Texture2D, 0, InternalFormat.Rgba,
            Memory.RamLogger.Width, Memory.RamLogger.Height, 0,
            PixelFormat.Rgba, PixelType.UnsignedByte, _ramFront);
        RamMapPanel.SetTexture(_ramTex);
    }

    static void ConvertDisplay(Gpu gpu, int w, int h)
    {
        var vram = gpu.Vram;
        int dx = gpu.DisplayX, dy = gpu.DisplayY;
        int o = 0;
        if (gpu.Display24Bit)
        {
            for (int y = 0; y < h; y++)
            {
                int lineByte = ((dy + y) * Gpu.VramWidth + dx) * 2;
                for (int x = 0; x < w; x++)
                {
                    int bo = lineByte + x * 3;
                    _rgbDisplay[o++] = VramByte(vram, bo);
                    _rgbDisplay[o++] = VramByte(vram, bo + 1);
                    _rgbDisplay[o++] = VramByte(vram, bo + 2);
                }
            }
        }
        else
        {
            for (int y = 0; y < h; y++)
            {
                int line = ((dy + y) & (Gpu.VramHeight - 1)) * Gpu.VramWidth;
                for (int x = 0; x < w; x++)
                {
                    ushort px = vram[line + ((dx + x) & (Gpu.VramWidth - 1))];
                    _rgbDisplay[o++] = (byte)((px & 0x1F) << 3);
                    _rgbDisplay[o++] = (byte)(((px >> 5) & 0x1F) << 3);
                    _rgbDisplay[o++] = (byte)(((px >> 10) & 0x1F) << 3);
                }
            }
        }
    }

    static void ConvertVramToBuffer(ushort[] vram, byte[] output)
    {
        int o = 0;
        for (int y = 0; y < Gpu.VramHeight; y++)
        for (int x = 0; x < Gpu.VramWidth; x++)
        {
            ushort px = vram[y * Gpu.VramWidth + x];
            output[o++] = (byte)((px & 0x1F) << 3);
            output[o++] = (byte)(((px >> 5) & 0x1F) << 3);
            output[o++] = (byte)(((px >> 10) & 0x1F) << 3);
        }
    }

    static byte VramByte(ushort[] vram, int byteOffset)
    {
        int hw = (byteOffset >> 1) & (Gpu.VramWidth * Gpu.VramHeight - 1);
        ushort v = vram[hw];
        return (byte)((byteOffset & 1) == 0 ? v & 0xFF : v >> 8);
    }
}
