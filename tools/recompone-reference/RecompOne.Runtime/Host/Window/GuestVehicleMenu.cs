using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using ImGuiNET;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Host.Window;

/// <summary>
/// Package management belongs to the wrapper; actual V8:2 vehicle selection
/// belongs to the native character-select flow and its integrated guest page.
/// </summary>
internal static class GuestVehicleMenu
{
    public static IPanel PackagePanel { get; } =
        new GuestVehiclePackagePanel();

    public static void Draw()
    {
        if (Runtime.GameTitle.Contains(
                "2nd Offense", StringComparison.Ordinal))
        {
            ImGui.TextDisabled("Vigilante 8: 2nd Offense");
            if (V82VehicleRegistry.HasPackage)
                ImGui.TextDisabled(
                    $"{V82VehicleRegistry.Count} vehicle" +
                    $"{(V82VehicleRegistry.Count == 1 ? "" : "s")} loaded");
            else
                ImGui.TextDisabled("No guest package loaded");

            if (ImGui.MenuItem("Manage test package..."))
                PackagePanel.IsOpen = true;
            return;
        }

        if (!V8VehicleRegistry.HasPackage)
        {
            ImGui.TextDisabled("No independent vehicle package loaded");
            return;
        }

        ImGui.TextDisabled("Vigilante 8");
        int selected = V8VehicleRegistry.SelectedType;
        if (ImGui.MenuItem(
                "Built-in roster selection##v8", null, selected < 0))
            V8VehicleRegistry.SelectType(-1);
        foreach (VehicleRosterItem item in V8VehicleRegistry.Roster())
        {
            string label =
                $"{item.DisplayName}  [{item.StableId}]##v8_{item.Type}";
            if (ImGui.MenuItem(label, null, selected == item.Type))
                V8VehicleRegistry.SelectType(item.Type);
        }
    }
}

internal sealed class GuestVehiclePackagePanel : IPanel
{
    const int PathBufferSize = 2048;
    readonly byte[] _pathBuffer = new byte[PathBufferSize];
    string _status = "";
    bool _statusIsError;
    bool _wasOpen;

    public string Name => "Guest Vehicle Package";
    public bool IsOpen { get; set; }

    public void Draw()
    {
        if (!IsOpen)
        {
            _wasOpen = false;
            return;
        }
        if (!_wasOpen)
        {
            SetPath(
                ConfigManager.Game.V82VehiclePackagePath.Length != 0
                    ? ConfigManager.Game.V82VehiclePackagePath
                    : V82VehicleRegistry.LoadedPackageRoot ?? "");
            _status = "";
            _wasOpen = true;
        }

        ImGui.SetNextWindowSize(
            new Vector2(720f, 430f), ImGuiCond.FirstUseEver);
        bool open = IsOpen;
        if (!ImGui.Begin(
                "V8:2 Guest Vehicle Packages",
                ref open,
                ImGuiWindowFlags.NoCollapse))
        {
            IsOpen = open;
            ImGui.End();
            return;
        }
        IsOpen = open;

        DrawHeading();
        ImGui.Spacing();
        DrawLoadedPackage();
        ImGui.Spacing();
        ImGui.Separator();
        ImGui.Spacing();

        ImGui.TextUnformatted("TEST PACKAGE FOR NEXT LAUNCH");
        ImGui.TextWrapped(
            "Point the wrapper at a compatible vehicle package. The normal " +
            "in-game character-select screen chooses among every vehicle " +
            "inside it; this window only changes which package is installed.");
        ImGui.Spacing();

        const float browseWidth = 92f;
        float spacing = ImGui.GetStyle().ItemSpacing.X;
        ImGui.SetNextItemWidth(
            ImGui.GetContentRegionAvail().X - browseWidth - spacing);
        ImGui.InputText(
            "##vehicle-package-path",
            _pathBuffer,
            PathBufferSize);
        ImGui.SameLine();
        if (ImGui.Button("Browse...", new Vector2(browseWidth, 0f)))
            Browse();

        ImGui.TextDisabled(
            "Select VEHICLES.V8R, CUSTOM.EXP, or their containing folder.");
        ImGui.Spacing();

        if (ImGui.Button(
                "Use Package Next Launch", new Vector2(210f, 34f)))
            Apply();
        ImGui.SameLine();
        if (ImGui.Button(
                "Use Loose Mods Autodiscovery", new Vector2(230f, 34f)))
            ClearOverride();

        if (_status.Length != 0)
        {
            ImGui.Spacing();
            ImGui.PushStyleColor(
                ImGuiCol.Text,
                _statusIsError
                    ? new Vector4(1f, 0.38f, 0.28f, 1f)
                    : new Vector4(0.71f, 0.91f, 0.42f, 1f));
            ImGui.TextWrapped(_status);
            ImGui.PopStyleColor();
        }

        ImGui.Spacing();
        ImGui.TextDisabled(
            "A package is validated and loaded at startup. Restart after " +
            "changing this setting.");
        ImGui.End();
    }

    static void DrawHeading()
    {
        ImGui.PushStyleColor(
            ImGuiCol.Text, new Vector4(0.93f, 0.68f, 0.24f, 1f));
        ImGui.TextUnformatted("GUEST VEHICLE TEST BAY");
        ImGui.PopStyleColor();
        ImGui.SameLine();
        ImGui.TextDisabled(" / VIGILANTE 8: 2ND OFFENSE");
    }

    static void DrawLoadedPackage()
    {
        ImGui.TextUnformatted("CURRENT RUN");
        if (!V82VehicleRegistry.HasPackage)
        {
            ImGui.TextDisabled("No guest vehicle package was loaded.");
            return;
        }

        ImGui.TextWrapped(
            V82VehicleRegistry.LoadedPackageRoot ?? "(package path unknown)");
        foreach (VehicleRosterItem item in V82VehicleRegistry.Roster())
        {
            ImGui.Bullet();
            ImGui.SameLine();
            ImGui.TextUnformatted(item.DisplayName);
            ImGui.SameLine();
            ImGui.TextDisabled($"[{item.StableId}]");
        }
    }

    void Browse()
    {
        string initial = ReadPath();
        if (NativeVehiclePackageDialog.TrySelect(
                initial, out string selected))
        {
            SetPath(selected);
            _status = "";
        }
    }

    void Apply()
    {
        try
        {
            string root = NormalizePackageRoot(ReadPath());
            ProbePackage(root);
            ConfigManager.Game.V82VehiclePackagePath = root;
            ConfigManager.SaveGame();
            SetPath(root);
            _statusIsError = false;
            _status =
                "Package selected. Restart the game to validate it fully and " +
                "add its entries to the in-game character-select roster.";
        }
        catch (Exception ex)
        {
            _statusIsError = true;
            _status = ex.Message;
        }
    }

    void ClearOverride()
    {
        ConfigManager.Game.V82VehiclePackagePath = "";
        ConfigManager.SaveGame();
        SetPath("");
        _statusIsError = false;
        _status =
            "Package override cleared. The next launch will scan the loose " +
            "files mods folder.";
    }

    static string NormalizePackageRoot(string value)
    {
        if (string.IsNullOrWhiteSpace(value))
            throw new InvalidDataException(
                "Choose a vehicle package before applying.");
        string path = Path.GetFullPath(value.Trim().Trim('"'));
        if (File.Exists(path))
            path = Path.GetDirectoryName(path)
                ?? throw new InvalidDataException(
                    "The selected package path has no parent directory.");
        return path;
    }

    static void ProbePackage(string root)
    {
        string registry = Path.Combine(root, "VEHICLES.V8R");
        string archive = Path.Combine(root, "CUSTOM.EXP");
        if (!File.Exists(registry) || !File.Exists(archive))
            throw new FileNotFoundException(
                "A vehicle package requires both VEHICLES.V8R and " +
                $"CUSTOM.EXP in {root}");

        byte[] header = new byte[20];
        using FileStream stream = File.OpenRead(registry);
        if (stream.Read(header, 0, header.Length) != header.Length ||
            !header.AsSpan(0, 4).SequenceEqual("V8VR"u8) ||
            header[6] != 2)
            throw new InvalidDataException(
                "VEHICLES.V8R is not a V8:2 guest-vehicle registry.");
    }

    string ReadPath() =>
        Encoding.UTF8.GetString(_pathBuffer).TrimEnd('\0').Trim();

    void SetPath(string value)
    {
        Array.Clear(_pathBuffer);
        byte[] bytes = Encoding.UTF8.GetBytes(value);
        Array.Copy(
            bytes,
            _pathBuffer,
            Math.Min(bytes.Length, _pathBuffer.Length - 1));
    }
}

internal static class NativeVehiclePackageDialog
{
    const int OfnPathMustExist = 0x00000800;
    const int OfnFileMustExist = 0x00001000;
    const int OfnNoChangeDir = 0x00000008;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct OpenFileName
    {
        public int StructSize;
        public IntPtr Owner;
        public IntPtr Instance;
        public string Filter;
        public string? CustomFilter;
        public int MaxCustomFilter;
        public int FilterIndex;
        public StringBuilder File;
        public int MaxFile;
        public string? FileTitle;
        public int MaxFileTitle;
        public string? InitialDir;
        public string? Title;
        public int Flags;
        public short FileOffset;
        public short FileExtension;
        public string? DefaultExtension;
        public IntPtr CustomData;
        public IntPtr Hook;
        public string? TemplateName;
        public IntPtr Reserved;
        public int Reserved2;
        public int FlagsEx;
    }

    [DllImport(
        "comdlg32.dll",
        CharSet = CharSet.Unicode,
        SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool GetOpenFileName(ref OpenFileName value);

    [DllImport("user32.dll")]
    static extern IntPtr GetActiveWindow();

    public static bool TrySelect(
        string initialPath, out string selectedDirectory)
    {
        string? initialDirectory = null;
        if (Directory.Exists(initialPath))
            initialDirectory = initialPath;
        else if (File.Exists(initialPath))
            initialDirectory = Path.GetDirectoryName(initialPath);

        var file = new StringBuilder(2048);
        var value = new OpenFileName
        {
            StructSize = Marshal.SizeOf<OpenFileName>(),
            Owner = GetActiveWindow(),
            Filter =
                "V8:2 vehicle package (VEHICLES.V8R;CUSTOM.EXP)\0" +
                "VEHICLES.V8R;CUSTOM.EXP\0All files\0*.*\0\0",
            FilterIndex = 1,
            File = file,
            MaxFile = file.Capacity,
            InitialDir = initialDirectory,
            Title = "Choose a V8:2 guest vehicle package",
            Flags =
                OfnPathMustExist | OfnFileMustExist | OfnNoChangeDir,
        };

        if (!GetOpenFileName(ref value))
        {
            selectedDirectory = "";
            return false;
        }

        selectedDirectory =
            Path.GetDirectoryName(file.ToString()) ?? "";
        return selectedDirectory.Length != 0;
    }
}
