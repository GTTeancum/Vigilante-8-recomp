using System.Numerics;
using System.Text;
using ImGuiNET;
using Silk.NET.Input;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.OpenGL.Extensions.ImGui;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;
using RecompOne.Runtime.Cdrom;

namespace RecompOne.Runtime.Host;

// Owns only the setup window. The game creates its ordinary host after this
// window and GL/ImGui context are disposed. Import never blocks the UI thread.
public static class FirstRunSetup
{
    public readonly record struct DiscStatus(bool SecondOffense, bool Original)
    {
        public bool Ready => SecondOffense && Original;
    }

    public static void MigrateEngineCache(string destination)
    {
        if (V82LooseImporter.IsComplete(destination))
            OriginalV8EngineInstallation.AdoptStaged(destination, Console.Error.WriteLine);
    }

    public static DiscStatus Inspect(string destination)
    {
        bool sequel = V82LooseImporter.IsComplete(destination);
        string albumRoot = sequel ? destination : V8SoundtrackImporter.StagingRoot(destination);
        bool album = V8LooseManifest.LoadEmbedded("V8LooseManifest.json").Tracks
            .Where(track => track.Number > 1).All(track =>
            {
                try
                {
                    using var file = File.OpenRead(Path.Combine(albumRoot, track.Source!));
                    Span<byte> magic = stackalloc byte[4];
                    return file.Length > 27 && file.Read(magic) == 4 && magic.SequenceEqual("OggS"u8);
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException) { return false; }
            });
        bool original = album && OriginalV8EngineInstallation.TryRead(destination,
            message => Console.Error.WriteLine($"[Setup] {message}")) != null;
        return new(sequel, original);
    }

    public static DiscStatus ImportDisc(string source, string destination,
        Action<LooseImportProgress> Report, CancellationToken cancellation)
    {
        string cue = ResolveCue(source);
        var title = V8SoundtrackImporter.BootExecutable(cue);
        if (V8SoundtrackImporter.IsFirstGameTitle(title))
        {
            string why = "Unknown source error.";
            Report(new("original-engines", 0, 1, "Verifying the Rev 1 disc and its engine samples"));
            if (!OriginalV8EngineInstallation.TryImport(cue, destination, cancellation,
                text => { why = text; Console.Error.WriteLine($"[OriginalV8Engines] {text}"); },
                sequelInstalled: V82LooseImporter.IsComplete(destination)))
                throw new InvalidDataException("Use your original Vigilante 8 USA Rev 1 CUE. " + why);
            V8SoundtrackImporter.Import(cue, destination, Report, cancellation);
        }
        else if (title == "SLUS_008.68")
        {
            V82LooseImporter.Import(cue, destination, Report, cancellation);
        }
        else
        {
            throw new InvalidDataException("That is not one of the required USA PlayStation discs.");
        }
        return Inspect(destination);
    }

    public static unsafe bool Run(string destination, string? initialSource = null)
    {
        GlfwWindowing.Use();
        using var window = Silk.NET.Windowing.Window.Create(WindowOptions.Default with
        {
            Title = "Vigilante 8 Classic - Setup", Size = new Vector2D<int>(900, 570),
            WindowBorder = WindowBorder.Fixed,
            IsVisible = Environment.GetEnvironmentVariable("RECOMPONE_WINDOW_VISIBLE") != "0",
            VSync = true,
            API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
        });
        GL? gl = null;
        IInputContext? input = null;
        ImGuiController? ui = null;
        using var cancel = new CancellationTokenSource();
        Task? import = null;
        Task<string?>? choosing = null;
        var gate = new object();
        var progress = new LooseImportProgress("welcome", 0, 1, "");
        string error = "", notice = "", selected = initialSource ?? "";
        string enteredPath = "";
        bool success = false;
        int frames = 0;
        DiscStatus status = Inspect(destination);
        ImFontPtr body = default, heading = default, strong = default;
        var gold = new Vector4(1f, .77f, .25f, 1);
        var muted = new Vector4(.65f, .70f, .76f, 1);
        var ready = new Vector4(.41f, .90f, .69f, 1);
        string? fixture = Environment.GetEnvironmentVariable("RECOMPONE_SETUP_TEST_SOURCE");
        string? captures = Environment.GetEnvironmentVariable("RECOMPONE_SETUP_CAPTURE_DIR");
        var captured = new HashSet<string>();

        void Start(string source)
        {
            selected = source;
            lock (gate) { error = ""; notice = ""; progress = new("validating", 0, 1, "Checking disc title ID"); }
            import = Task.Run(() =>
            {
                try
                {
                    void Report(LooseImportProgress p)
                    {
                        lock (gate) progress = p;
                        Console.Error.WriteLine($"[Setup] {p.Phase} {p.Current}/{p.Total} {p.Item}");
                    }
                    DiscStatus verified = ImportDisc(source, destination, Report, cancel.Token);
                    lock (gate)
                    {
                        status = verified;
                        notice = verified.Ready ? "Both discs are ready. You can now launch the game."
                            : verified.SecondOffense ? "Second Offense imported. Now add the original Vigilante 8 Rev 1 disc."
                            : "Original Vigilante 8 imported. Now add your Second Offense disc.";
                        progress = new("verified", 1, 1, "");
                    }
                }
                catch (OperationCanceledException) { }
                catch (Exception ex)
                {
                    lock (gate) { error = ex.Message; progress = new("error", 0, 1, ""); }
                    Console.Error.WriteLine($"[Setup] failed: {ex}");
                }
            });
        }

        window.Load += () =>
        {
            gl = GL.GetApi(window); input = window.CreateInput();
            ui = new ImGuiController(gl, window, input, () =>
            {
                // Use the desktop clipboard, not ImGui's process-local buffer,
                // on both Windows and Linux.
                // GLFW's UTF-8 get/set APIs match ImGui's callback signatures.
                var io = ImGui.GetIO();
                string systemFonts = Environment.GetFolderPath(Environment.SpecialFolder.Fonts);
                string regular = Path.Combine(systemFonts, "segoeui.ttf");
                string bold = Path.Combine(systemFonts, "segoeuib.ttf");
                if (!OperatingSystem.IsWindows())
                {
                    regular = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
                    bold = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
                }
                if (File.Exists(regular) && File.Exists(bold))
                {
                    body = io.Fonts.AddFontFromFileTTF(regular, 17);
                    strong = io.Fonts.AddFontFromFileTTF(bold, 18);
                    heading = io.Fonts.AddFontFromFileTTF(bold, 34);
                    io.NativePtr->FontDefault = body.NativePtr;
                }
                var glfw = Silk.NET.GLFW.GlfwProvider.GLFW.Value.Context;
                io.GetClipboardTextFn = glfw.GetProcAddress("glfwGetClipboardString");
                io.SetClipboardTextFn = glfw.GetProcAddress("glfwSetClipboardString");
                io.ClipboardUserData = window.Handle;

                // Silk 2.22 sends physical modifier keys, but ImGui 1.90's
                // shortcuts also need ModCtrl/ModShift events. Queue them in
                // input order so a complete chord between frames is not lost.
                if (input.Keyboards.Count > 0)
                {
                    var keyboard = input.Keyboards[0];
                    keyboard.KeyDown += ForwardSetupModifiers;
                    keyboard.KeyUp += ForwardSetupModifiers;
                }
            });
            ImGui.GetIO().NativePtr->IniFilename = null;
            ImGui.StyleColorsDark();
            var style = ImGui.GetStyle();
            style.WindowPadding = new(28, 21);
            style.FramePadding = new(11, 7);
            style.ItemSpacing = new(10, 8);
            style.FrameRounding = 6;
            style.ChildRounding = 9;
            style.Colors[(int)ImGuiCol.WindowBg] = new(.055f, .070f, .095f, 1);
            style.Colors[(int)ImGuiCol.ChildBg] = new(.094f, .120f, .156f, 1);
            style.Colors[(int)ImGuiCol.Border] = new(.20f, .25f, .31f, 1);
            style.Colors[(int)ImGuiCol.Button] = new(.16f, .22f, .29f, 1);
            style.Colors[(int)ImGuiCol.ButtonHovered] = new(.22f, .30f, .39f, 1);
            style.Colors[(int)ImGuiCol.FrameBg] = new(.11f, .15f, .20f, 1);
            style.Colors[(int)ImGuiCol.PlotHistogram] = gold;
        };
        window.Render += dt =>
        {
            ui!.Update((float)Math.Max(dt, 0.001));
            gl!.Viewport(0, 0, (uint)window.FramebufferSize.X, (uint)window.FramebufferSize.Y);
            gl.ClearColor(0.07f, 0.07f, 0.08f, 1); gl.Clear(ClearBufferMask.ColorBufferBit);
            if (choosing is { IsCompleted: true })
            {
                // Any failure here must reach the window, not escape the
                // render callback: an exception from Render ends the process.
                try
                {
                    string? file = choosing.GetAwaiter().GetResult();
                    if (file != null) Start(file);
                }
                catch (Exception ex)
                {
                    lock (gate) error = ex.Message;
                    Console.Error.WriteLine($"[Setup] file selection failed: {ex}");
                }
                choosing = null;
            }
            LooseImportProgress p; string message, info; DiscStatus discs;
            lock (gate) { p = progress; message = error; info = notice; discs = status; }
            bool busy = import != null && !import.IsCompleted;
            ImGui.SetNextWindowPos(Vector2.Zero);
            ImGui.SetNextWindowSize(new Vector2(window.Size.X, window.Size.Y));
            ImGui.Begin("Setup", ImGuiWindowFlags.NoDecoration | ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoSavedSettings);
            ImGui.TextColored(gold, "PC EDITION  /  FIRST-RUN INSTALLATION");
            if (heading.NativePtr != null) ImGui.PushFont(heading);
            ImGui.TextUnformatted("Vigilante 8 Classic");
            if (heading.NativePtr != null) ImGui.PopFont();
            ImGui.TextColored(muted, "Both USA PlayStation discs are required. Add them in either order.");
            ImGui.Dummy(new(0, 8));
            void DiscCard(string id, string name, string detail, bool installed)
            {
                ImGui.BeginChild("disc" + id, new(0, 65), ImGuiChildFlags.Border, ImGuiWindowFlags.NoScrollbar);
                ImGui.TextColored(installed ? ready : gold, id + (installed ? "   VERIFIED" : "   REQUIRED"));
                ImGui.SameLine(155);
                if (strong.NativePtr != null) ImGui.PushFont(strong);
                ImGui.TextUnformatted(name);
                if (strong.NativePtr != null) ImGui.PopFont();
                ImGui.SetCursorPosX(155);
                ImGui.TextColored(muted, detail);
                ImGui.EndChild();
            }
            DiscCard("01", "Vigilante 8: 2nd Offense", "USA / SLUS-00868  |  Base game and Second Offense music", discs.SecondOffense);
            DiscCard("02", "Vigilante 8", "USA Rev 1 / SLUS-00510  |  Original music and guest-car engines", discs.Original);
            ImGui.Dummy(new(0, 3));
            if (!busy && choosing == null && !discs.Ready)
            {
                ImGui.TextUnformatted("Select a disc image");
                if (OperatingSystem.IsWindows())
                {
                    if (ImGui.Button("Browse for disc...", new(154, 40)))
                        choosing = NativeFileDialog.OpenAsync("Choose a required Vigilante 8 disc",
                            "PlayStation disc (BIN/CUE)\0*.cue;*.bin\0\0", owner: NativeFileDialog.ActiveWindow());
                    ImGui.SameLine();
                }
                ImGui.SetNextItemWidth(-125);
                bool submitted = ImGui.InputTextWithHint("##setup-disc-path", "Or paste the full CUE path",
                    ref enteredPath, 4096, ImGuiInputTextFlags.EnterReturnsTrue);
                ImGui.SameLine();
                if ((ImGui.Button("Use path", new(110, 40)) || submitted) && choosing == null)
                {
                    string path = NormalizeTypedDiscPath(enteredPath);
                    if (!string.IsNullOrWhiteSpace(path)) Start(path);
                }
                ImGui.TextColored(muted, "Keep each CUE beside every BIN it references.");
            }
            else if (busy)
            {
                ImGui.TextUnformatted(p.Phase switch
                {
                    "music" => "Preparing Second Offense music",
                    "soundtrack" => "Preparing original Vigilante 8 music",
                    "original-engines" => "Verifying original Vigilante 8 engine audio",
                    "validating" => "Checking disc image",
                    _ => "Importing game data",
                });
                ImGui.ProgressBar((float)p.Current / Math.Max(1, p.Total), new(-1, 20));
                ImGui.TextWrapped(Path.GetFileName(selected) + "  |  " + p.Item);
            }
            else if (choosing != null) ImGui.TextColored(muted, "Waiting for the file picker...");
            if (message.Length != 0)
            {
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1, .66f, .52f, 1));
                ImGui.TextWrapped(message);
                ImGui.PopStyleColor();
            }
            else if (!busy && info.Length != 0) ImGui.TextColored(ready, info);
            ImGui.Dummy(new(0, 4));
            if (choosing == null && ImGui.Button(busy ? "Cancel installation" : "Exit setup", new(158, 40)))
                window.Close();
            if (discs.Ready && !busy && choosing == null)
            {
                ImGui.SameLine(ImGui.GetWindowWidth() - 211);
                ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(.97f, .72f, .22f, 1));
                ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(1, .81f, .43f, 1));
                ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(.09f, .10f, .13f, 1));
                if (ImGui.Button("Launch game", new(182, 40))) { success = Inspect(destination).Ready; window.Close(); }
                ImGui.PopStyleColor(3);
            }
            ImGui.End(); ui.Render();
            string phase = p.Phase is "files" or "music" or "soundtrack"
                ? p.Phase + "-" + (p.Current * 4 / Math.Max(1, p.Total)) : p.Phase;
            if (captures != null && frames > 1 && captured.Add(phase)) Capture(gl, window, captures, phase);
            // A process-local fixture calls the same selection/import action as
            // the button. It never sends input to Windows or another process.
            if (++frames == 5 && (fixture != null || initialSource != null)) Start(fixture ?? initialSource!);
            if (fixture != null && p.Phase == "verified" && captures != null && captured.Contains("verified"))
            { success = discs.Ready; window.Close(); }
            if (fixture != null && p.Phase == "error" && captured.Contains("error")) window.Close();
        };
        // Silk destroys the context before Run returns. Release GPU objects
        // while Closing still has the setup context current.
        window.Closing += () =>
        {
            ui?.Dispose(); ui = null;
            input?.Dispose(); input = null;
            gl?.Dispose(); gl = null;
        };
        try { window.Run(); }
        finally
        {
            cancel.Cancel(); import?.GetAwaiter().GetResult();
            ui?.Dispose(); input?.Dispose(); gl?.Dispose();
        }
        return success;
    }

    static void ForwardSetupModifiers(IKeyboard keyboard, Key key, int scancode)
    {
        var io = ImGui.GetIO();
        io.AddKeyEvent(ImGuiKey.ModCtrl,
            keyboard.IsKeyPressed(Key.ControlLeft) || keyboard.IsKeyPressed(Key.ControlRight));
        io.AddKeyEvent(ImGuiKey.ModShift,
            keyboard.IsKeyPressed(Key.ShiftLeft) || keyboard.IsKeyPressed(Key.ShiftRight));
    }

    public static string ResolveCue(string source)
    {
        source = Path.GetFullPath(source);
        if (!File.Exists(source)) throw new FileNotFoundException("The selected disc file was not found.", source);
        if (Path.GetExtension(source).Equals(".cue", StringComparison.OrdinalIgnoreCase)) return source;
        if (!Path.GetExtension(source).Equals(".bin", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("Select a CUE file or one of its BIN files.");
        var pathComparison = OperatingSystem.IsWindows()
            ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;
        var matches = Directory.EnumerateFiles(Path.GetDirectoryName(source)!)
            .Where(cue => Path.GetExtension(cue).Equals(".cue", StringComparison.OrdinalIgnoreCase))
            .Where(cue => File.ReadLines(cue).Any(line =>
            {
                var text = line.Trim();
                int first = text.IndexOf('"'), last = text.LastIndexOf('"');
                return text.StartsWith("FILE ", StringComparison.OrdinalIgnoreCase) && first >= 0 && last > first &&
                    Path.GetFullPath(Path.Combine(Path.GetDirectoryName(cue)!, text[(first + 1)..last])).Equals(source, pathComparison);
            })).ToArray();
        if (matches.Length != 1) throw new InvalidDataException("Select the matching CUE file. A BIN alone does not describe all game and audio tracks.");
        return matches[0];
    }

    public static string NormalizeTypedDiscPath(string source)
    {
        // Explorer's "Copy as path" surrounds the whole path with quotes.
        // Do not apply this to paths from the picker or command line.
        string path = source.Trim();
        return path.Length >= 2 && path[0] == '"' && path[^1] == '"' ? path[1..^1] : path;
    }

    static unsafe void Capture(GL gl, IWindow window, string directory, string phase)
    {
        int width = window.FramebufferSize.X, height = window.FramebufferSize.Y;
        byte[] pixels = new byte[width * height * 3];
        gl.PixelStore(PixelStoreParameter.PackAlignment, 1);
        fixed (byte* ptr = pixels) gl.ReadPixels(0, 0, (uint)width, (uint)height, PixelFormat.Rgb, PixelType.UnsignedByte, ptr);
        Directory.CreateDirectory(directory);
        using var file = File.Create(Path.Combine(directory, "setup-" + phase + ".ppm"));
        file.Write(Encoding.ASCII.GetBytes($"P6\n{width} {height}\n255\n"));
        for (int y = height - 1; y >= 0; y--) file.Write(pixels, y * width * 3, width * 3);
    }
}
