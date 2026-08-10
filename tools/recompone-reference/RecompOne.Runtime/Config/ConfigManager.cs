using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using ImGuiNET;
using RecompOne.Runtime.Host.Window;
using RecompOne.Runtime.Serialization;

namespace RecompOne.Runtime.Config;

static file class PanelDefaults
{
    public static bool IsOpenByDefault(IPanel p) => p.Name == "Output";
}

public static class ConfigManager
{
    static readonly JsonSerializerOptions _opts = new()
    {
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.Never,
    };

    // settings.json used to be a bare relative path, so it resolved against
    // whatever directory the game happened to be launched from. Several copies
    // exist across a working tree and a staged install, holding different
    // input profiles, which meant the bindings actually in force depended on
    // the launch directory rather than on what the player configured -- a
    // Classic copy silently leaves brake/reverse on the stick and D-pad only.
    // Resolve it next to the executable instead, and migrate a config found
    // beside the launch directory the first time so nothing is lost.
    static readonly string GameConfigPath =
        Path.Combine(AppContext.BaseDirectory, "settings.json");
    const string LegacyGameConfigPath = "settings.json";
    const string InterfaceFile = "interface.ini";

    public static GameConfig Game { get; private set; } = new();
    public static ViewConfig  View { get; private set; } = new();

    static string? _pendingImGuiIni;

    public static void Load()
    {
        bool saveGame = false;
        string? source =
            File.Exists(GameConfigPath) ? GameConfigPath
            : File.Exists(LegacyGameConfigPath) ? Path.GetFullPath(LegacyGameConfigPath)
            : null;
        if (source != null)
        {
            try { Game = JsonSerializer.Deserialize(File.ReadAllText(source), RuntimeJsonContext.Default.GameConfig) ?? new(); }
            catch { Game = new(); saveGame = true; }
            if (!string.Equals(source, GameConfigPath, StringComparison.OrdinalIgnoreCase))
                saveGame = true;
        }
        else
        {
            Game = new();
            saveGame = true;
        }

        if (Game.InputBindingsVersion < 1)
        {
            // Pad2 was historically serialized as an all-empty legacy default.
            // Migrate only that legacy shape; any customized non-empty mapping
            // is retained byte-for-byte by the serializer.
            if (!Game.Pad2.HasAnyBinding()) Game.Pad2 = new GamepadBindings();
            Game.InputBindingsVersion = 1;
            saveGame = true;
        }
        if (Game.InputBindingsVersion < 2)
        {
            bool isV82 = Runtime.GameTitle.Contains("2nd Offense", StringComparison.Ordinal);
            bool legacyDefaults =
                InputProfiles.IsClassicDefaults(Game.Pad) &&
                InputProfiles.IsClassicDefaults(Game.Pad2);
            if (isV82 && legacyDefaults)
                InputProfiles.Apply(Game, InputProfiles.Modern);
            else if (legacyDefaults)
                Game.InputProfile = InputProfiles.Classic;
            else
                Game.InputProfile = InputProfiles.Custom;
            Game.InputBindingsVersion = 2;
            saveGame = true;
        }
        if (saveGame) SaveGame();
        Console.Error.WriteLine(
            $"[Config] settings={GameConfigPath} " +
            $"profile={Game.InputProfile} " +
            $"down=[{string.Join(',', Game.Pad.Down)}]" +
            (source != null && !string.Equals(source, GameConfigPath,
                StringComparison.OrdinalIgnoreCase)
                ? $" (migrated from {source})" : string.Empty));

        if (File.Exists(InterfaceFile))
        {
            var (view, imguiIni) = ParseInterfaceFile(File.ReadAllText(InterfaceFile));
            view.ReconcileNamedGraphicsPreset();
            View = view;
            _pendingImGuiIni = imguiIni;
        }

        // V8:2 has no render-scale choice in its in-game menu, because the
        // scale caps at 4x and that is 1280x960 -- below every output
        // resolution the menu offers except 720p. Anything lower is strictly
        // worse with nothing to show for it: the fill rate at that size is
        // negligible and VRAM readback is scale-independent, because ReadRect
        // blits down to an unscaled staging target before reading. So the
        // port pins it, and MSAA remains the knob for a slow GPU. The host
        // panel can still change it for the rest of a session.
        if (Runtime.GameTitle.Contains("2nd Offense", StringComparison.Ordinal))
        {
            View.InternalResolutionScale = 4;
            View.HighResolution3D = true;
        }
    }

    
    public static bool ApplyImGuiLayout()
    {
        if (_pendingImGuiIni == null) return false;
        ImGui.LoadIniSettingsFromMemory(_pendingImGuiIni);
        _pendingImGuiIni = null;
        return true;
    }

    public static void ApplyViewToPanels(IReadOnlyList<IPanel> panels)
    {
        foreach (var p in panels)
        {
            if (View.Panels.TryGetValue(p.Name, out var state))
                p.IsOpen = state.Open;
        }
    }

    public static void SaveView(IReadOnlyList<IPanel> panels)
    {
        foreach (var p in panels)
            View.Panels[p.Name] = new PanelState { Open = p.IsOpen };

        // The in-game menus save view settings too, and they run in contexts
        // with no ImGui context at all -- headless, and any launch where the
        // host window never came up. Asking ImGui to serialise itself there
        // takes the process down inside native code with no managed exception
        // to catch. Carry the layout already on disk across instead of writing
        // an empty section, which would silently discard the window layout of
        // anyone who changes a setting from the in-game menus.
        string imguiIni = ImGui.GetCurrentContext() != IntPtr.Zero
            ? ImGui.SaveIniSettingsToMemory()
            : ReadStoredImGuiLayout();
        var sb = new StringBuilder();
        sb.AppendLine("[RecompOne]");
        foreach (var (key, value) in View.Values)
            sb.AppendLine($"{key}={value}");
        foreach (var (name, state) in View.Panels)
            sb.AppendLine($"Panels.{name}={state.Open}");
        sb.AppendLine();
        sb.Append(imguiIni);
        File.WriteAllText(InterfaceFile, sb.ToString());
    }

    /// <summary>
    /// Everything in interface.ini from the first section that is not
    /// [RecompOne] onwards, which is exactly the ImGui layout this writer
    /// appends after its own key/value block.
    /// </summary>
    static string ReadStoredImGuiLayout()
    {
        if (!File.Exists(InterfaceFile))
            return string.Empty;
        try
        {
            string[] lines = File.ReadAllLines(InterfaceFile);
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];
                if (!line.StartsWith('[') ||
                    line.Equals("[RecompOne]", StringComparison.Ordinal))
                    continue;
                return string.Join(Environment.NewLine, lines[i..]) +
                    Environment.NewLine;
            }
        }
        catch (IOException) { }
        return string.Empty;
    }

    public static void ResetView(IReadOnlyList<IPanel> panels)
    {
        View = new();
        foreach (var p in panels)
            p.IsOpen = PanelDefaults.IsOpenByDefault(p);
        ImGui.LoadIniSettingsFromMemory("");
        SaveView(panels);
    }

    public static void SaveGame()
    {
        File.WriteAllText(GameConfigPath, JsonSerializer.Serialize(Game, RuntimeJsonContext.Default.GameConfig));
    }

    static (ViewConfig view, string imguiIni) ParseInterfaceFile(string content)
    {
        var view = new ViewConfig();
        var imguiLines = new List<string>();
        bool inRecompOne = false;

        foreach (var rawLine in content.Split('\n'))
        {
            var line = rawLine.TrimEnd('\r');

            if (line == "[RecompOne]")
            {
                inRecompOne = true;
                continue;
            }

            if (line.StartsWith('['))
                inRecompOne = false;

            if (inRecompOne)
            {
                if (line.Length == 0) continue;
                int eq = line.IndexOf('=');
                if (eq <= 0) continue;
                var key = line[..eq];
                var value = line[(eq + 1)..];
                if (key.StartsWith("Panels."))
                {
                    var panelName = key[7..];
                    var open = bool.TryParse(value, out var b) && b;
                    view.Panels[panelName] = new PanelState { Open = open };
                }
                else
                {
                    view.Values[key] = value;
                }
            }
            else
            {
                imguiLines.Add(line);
            }
        }

        return (view, string.Join('\n', imguiLines));
    }
}
