using System.Numerics;
using System.Runtime.InteropServices;
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
    public static unsafe bool Run(string destination, string? initialSource = null)
    {
        GlfwWindowing.Use();
        using var window = Silk.NET.Windowing.Window.Create(WindowOptions.Default with
        {
            Title = "Vigilante 8 Classic - Setup", Size = new Vector2D<int>(800, 420),
            IsVisible = Environment.GetEnvironmentVariable("RECOMPONE_WINDOW_VISIBLE") != "0",
            VSync = true,
            API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
        });
        GL? gl = null;
        IInputContext? input = null;
        ImGuiController? ui = null;
        using var cancel = new CancellationTokenSource();
        Task? import = null;
        var gate = new object();
        var progress = new LooseImportProgress("welcome", 0, 1, "");
        string error = "", selected = initialSource ?? "";
        bool success = false;
        int frames = 0, completionFrames = 0;
        string? fixture = Environment.GetEnvironmentVariable("RECOMPONE_SETUP_TEST_SOURCE");
        string? captures = Environment.GetEnvironmentVariable("RECOMPONE_SETUP_CAPTURE_DIR");
        var captured = new HashSet<string>();

        void Start(string source)
        {
            selected = source; error = "";
            lock (gate) progress = new("validating", 0, 1, "Checking disc title ID");
            import = Task.Run(() =>
            {
                try
                {
                    string cue = ResolveCue(source);
                    V82LooseImporter.Import(cue, destination, p =>
                    {
                        lock (gate) progress = p;
                        Console.Error.WriteLine($"[Setup] {p.Phase} {p.Current}/{p.Total} {p.Item}");
                    }, cancel.Token);
                    lock (gate) { success = true; progress = new("complete", 1, 1, "Starting your game..."); }
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
            ui = new ImGuiController(gl, window, input);
            ImGui.GetIO().NativePtr->IniFilename = null;
            ImGui.StyleColorsDark();
        };
        window.Render += dt =>
        {
            ui!.Update((float)Math.Max(dt, 0.001));
            gl!.Viewport(0, 0, (uint)window.FramebufferSize.X, (uint)window.FramebufferSize.Y);
            gl.ClearColor(0.07f, 0.07f, 0.08f, 1); gl.Clear(ClearBufferMask.ColorBufferBit);
            LooseImportProgress p; string message; bool done;
            lock (gate) { p = progress; message = error; done = success; }
            bool busy = import != null && !import.IsCompleted && p.Phase != "error";
            ImGui.SetNextWindowPos(Vector2.Zero);
            ImGui.SetNextWindowSize(new Vector2(window.Size.X, window.Size.Y));
            ImGui.Begin("Setup", ImGuiWindowFlags.NoDecoration | ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoSavedSettings);
            ImGui.SetWindowFontScale(1.5f);
            ImGui.TextColored(new Vector4(1, 0.8f, 0.05f, 1), "VIGILANTE 8 CLASSIC");
            ImGui.Separator(); ImGui.Spacing();
            ImGui.TextWrapped("Choose your Vigilante 8: 2nd Offense (USA) CUE or BIN file to install the game.");
            ImGui.TextWrapped("Keep the CUE and all its BIN files together. Required title ID: SLUS-00868.");
            ImGui.Spacing();
            if (selected.Length != 0) ImGui.TextWrapped("Selected: " + Path.GetFileName(selected));
            if (!busy && !done)
            {
                if (ImGui.Button("Choose BIN / CUE...", new Vector2(250, 42)))
                {
                    string? file = ChooseDisc();
                    if (file != null) Start(file);
                }
                if (message.Length != 0)
                {
                    ImGui.Spacing(); ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1, 0.5f, 0.4f, 1));
                    ImGui.TextWrapped(message); ImGui.PopStyleColor();
                }
            }
            if (busy || done)
            {
                ImGui.Text(p.Phase == "complete" ? "Installation complete" : p.Phase == "music" ? "Preparing music..." : p.Phase == "validating" ? "Checking disc..." : "Extracting game files...");
                ImGui.ProgressBar((float)p.Current / Math.Max(1, p.Total), new Vector2(-1, 30));
                ImGui.TextWrapped(p.Item);
            }
            ImGui.Spacing();
            if (!done && ImGui.Button(busy ? "Cancel installation" : "Exit", new Vector2(220, 36))) window.Close();
            ImGui.End(); ui.Render();
            string phase = p.Phase is "files" or "music" ? p.Phase + "-" + (p.Current * 4 / Math.Max(1, p.Total)) : p.Phase;
            if (captures != null && frames > 1 && captured.Add(phase)) Capture(gl, window, captures, phase);
            // A process-local fixture calls the same selection/import action as
            // the button. It never sends input to Windows or another process.
            if (++frames == 5 && (fixture != null || initialSource != null)) Start(fixture ?? initialSource!);
            if (done && ++completionFrames >= 30) window.Close();
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

    public static string ResolveCue(string source)
    {
        source = Path.GetFullPath(source);
        if (!File.Exists(source)) throw new FileNotFoundException("The selected disc file was not found.", source);
        if (Path.GetExtension(source).Equals(".cue", StringComparison.OrdinalIgnoreCase)) return source;
        if (!Path.GetExtension(source).Equals(".bin", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("Select a CUE file or one of its BIN files.");
        var matches = Directory.EnumerateFiles(Path.GetDirectoryName(source)!, "*.cue")
            .Where(cue => File.ReadLines(cue).Any(line =>
            {
                var text = line.Trim();
                int first = text.IndexOf('"'), last = text.LastIndexOf('"');
                return text.StartsWith("FILE ", StringComparison.OrdinalIgnoreCase) && first >= 0 && last > first &&
                    Path.GetFullPath(Path.Combine(Path.GetDirectoryName(cue)!, text[(first + 1)..last])).Equals(source, StringComparison.OrdinalIgnoreCase);
            })).ToArray();
        if (matches.Length != 1) throw new InvalidDataException("Select the matching CUE file. A BIN alone does not describe all game and audio tracks.");
        return matches[0];
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

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct OpenFileName
    {
        public int Size; public nint Owner, Instance; public string Filter;
        public nint CustomFilter; public int MaxCustomFilter, FilterIndex;
        public StringBuilder File; public int MaxFile; public nint FileTitle;
        public int MaxFileTitle; public string? InitialDir, Title; public int Flags;
        public short FileOffset, FileExtension; public nint DefaultExtension, CustomData, Hook, TemplateName, Reserved;
        public int Reserved2, FlagsEx;
    }
    [DllImport("comdlg32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool GetOpenFileName(ref OpenFileName value);

    static string? ChooseDisc()
    {
        var buffer = new StringBuilder(32768);
        var dialog = new OpenFileName
        {
            Size = Marshal.SizeOf<OpenFileName>(), Filter = "PlayStation disc (BIN/CUE)\0*.cue;*.bin\0\0",
            FilterIndex = 1, File = buffer, MaxFile = buffer.Capacity,
            Title = "Select Vigilante 8: 2nd Offense", Flags = 0x1000 | 0x800 | 8
        };
        return GetOpenFileName(ref dialog) ? buffer.ToString() : null;
    }
}
