using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Diagnostics;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime;
using Recompiled;

AppDomain.CurrentDomain.ProcessExit += (_, _) =>
    Console.Error.WriteLine(
        $"[Host] process exit requested (code={Environment.ExitCode})");

string launchDirectory = Environment.CurrentDirectory;
string executableDirectory = Runtime.ExecutableDirectory;
// ViewConfig has sequel-specific Enhanced defaults (notably Maximum LOD).
// Establish the game identity before any command, probe, or configuration
// migration. Runtime.Initialize reaffirms this title when a window is needed.
Runtime.ConfigureGameTitle("Vigilante 8: 2nd Offense PC");
ConsoleMirror.Install();

string? guestVehicle = null;
string? explicitLoose = null;
var launchArguments = args.ToList();
for (int index = 0; index < launchArguments.Count; index++)
{
    if (launchArguments[index].Equals(
            "--muted", StringComparison.OrdinalIgnoreCase))
    {
        // Keep automated/headless runs silent without mutating the player's
        // saved audio settings. Runtime.Initialize consumes this override.
        Environment.SetEnvironmentVariable("RECOMPONE_MUTE", "1");
        launchArguments.RemoveAt(index);
        index--;
        continue;
    }
    if (launchArguments[index].Equals(
            "--loose", StringComparison.OrdinalIgnoreCase))
    {
        if (index + 1 >= launchArguments.Count)
            throw new ArgumentException("--loose requires a directory");
        explicitLoose = Path.GetFullPath(
            launchArguments[index + 1], launchDirectory);
        launchArguments.RemoveRange(index, 2);
        index--;
        continue;
    }
    if (!launchArguments[index].Equals(
            "--guest-vehicle", StringComparison.OrdinalIgnoreCase))
        continue;
    if (index + 1 >= launchArguments.Count)
        throw new ArgumentException("--guest-vehicle requires a stable ID");
    guestVehicle = launchArguments[index + 1];
    launchArguments.RemoveRange(index, 2);
    break;
}
args = launchArguments.ToArray();

if (args.Length == 3 &&
    args[0].Equals("--extract-loose", StringComparison.OrdinalIgnoreCase))
{
    string cue = Path.GetFullPath(args[1], launchDirectory);
    string output = Path.GetFullPath(args[2], launchDirectory);
    V82LooseImporter.Import(cue, output, ReportLooseImport);
    return 0;
}

if (args.Length == 1 &&
    args[0].Equals("--probe-native-controls", StringComparison.OrdinalIgnoreCase))
{
    try
    {
        var modern = InputProfiles.CreatePad(InputProfiles.Modern);
        var triggerDrive =
            InputProfiles.CreatePad(InputProfiles.TriggerDrive);
        var classic = InputProfiles.CreatePad(InputProfiles.Classic);
        var southpaw = InputProfiles.CreatePad(InputProfiles.Southpaw);
        if (!InputProfiles.ValidateModernBrakeReverse(modern))
            throw new InvalidOperationException("Modern Square is not routed to native brake/reverse");
        if (!InputProfiles.ValidateTriggerDrive(triggerDrive))
            throw new InvalidOperationException(
                "Trigger Drive physical-to-native gameplay mapping is incomplete");
        if (!ReferenceEquals(
                InputBindingResolver.ResolvePad(
                    InputProfiles.TriggerDrive, triggerDrive,
                    gameplayActive: true, nativeGameplayMenuActive: false),
                triggerDrive))
            throw new InvalidOperationException(
                "Trigger Drive gameplay mapping was replaced");
        var triggerMenu = InputBindingResolver.ResolvePad(
            InputProfiles.TriggerDrive, triggerDrive,
            gameplayActive: false, nativeGameplayMenuActive: false);
        var triggerPause = InputBindingResolver.ResolvePad(
            InputProfiles.TriggerDrive, triggerDrive,
            gameplayActive: true, nativeGameplayMenuActive: true);
        if (!InputProfiles.IsClassicDefaults(triggerMenu) ||
            !InputProfiles.IsClassicDefaults(triggerPause))
            throw new InvalidOperationException(
                "Trigger Drive menu fallback is not the stock DualShock layout");
        if (!classic.Square.Contains(2) || classic.Down.Contains(2))
            throw new InvalidOperationException("Classic retail Square mapping changed");
        if (!southpaw.Down.Contains(2) || !southpaw.Down.Contains(109))
            throw new InvalidOperationException("Southpaw reverse mapping is incomplete");
        Console.WriteLine(
            "[NativeControlsProbe] PASS " +
            "presets=Modern,TriggerDrive,Classic,Southpaw " +
            "trigger_drive=RT_Gas,LT_BrakeReverse,A_SelectedWeapon," +
            "X_MachineGun,LB_RB_Swap menu_face_buttons=stock " +
            "modern_square=Down modern_stick_down=Down");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine($"[NativeControlsProbe] FAILED: {exception}");
        return 1;
    }
}

if (args.Length is 2 or 3 &&
    args[0].Equals(
        "--probe-selector-portraits", StringComparison.OrdinalIgnoreCase))
{
    try
    {
        string package = Path.GetFullPath(args[1], launchDirectory);
        string? portraitOutput = args.Length == 3
            ? Path.GetFullPath(args[2], launchDirectory)
            : null;
        if (portraitOutput != null)
            Directory.CreateDirectory(portraitOutput);
        var decodedPortraits =
            new List<(int Width, int Height, byte[] Rgb)>();
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] phase=package-validation begin");
        string validation = V82VehicleRegistry.ValidatePackage(package);
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] phase=package-validation complete");
        Runtime.SetMode(RunMode.Devkit);
        var memory = new PSMemory();
        LoadPsxExecutable(
            memory, Path.Combine(package, "SLUS_008.68"));
        string shell = Path.Combine(package, "SHELL");
        memory.LoadBytes(
            0x80100000u,
            File.ReadAllBytes(Path.Combine(shell, "SHELL.DLL")));
        Dispatcher.Register("main", new MainDispatchTable());
        Dispatcher.Register("SHELL_SHELL", new SHELL_SHELLDispatchTable());
        Dispatcher.Load("main");
        Dispatcher.Load("SHELL_SHELL");
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] phase=dispatcher-init complete");
        var context = new CpuContext
        {
            GP = 0x8006A7F0u,
            SP = 0x801FF000u,
            RA = 0x80012340u,
        };
        Runtime.SetContext(context, memory);
        RecompOne.Runtime.Bios.Bios.Init(memory);
        context.A0 = 0u;
        Vigilante82PC.func_8005BC0C(context, memory);
        context.GP = 0x8006A7F0u;
        context.SP = 0x801FF000u;
        context.RA = 0x80012340u;
        // A fresh harness has no prior VRAM tree to release; the retail boot
        // call passes one so func_800206A8 establishes allocator bookkeeping.
        context.A0 = 1u;
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] phase=vram-init begin");
        Vigilante82PC.func_8002091C(context, memory);
        // SHELL subsequently switches the initialized allocator to its
        // 640x480 front-end layout through the ordinary reset-existing path.
        context.A0 = 0u;
        Vigilante82PC.func_800209C8(context, memory);
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] phase=vram-init complete");

        const uint recordAddress = 0x80120000u;
        byte[] poses = File.ReadAllBytes(Path.Combine(shell, "POSES.TBL"));
        int retailStart = checked((int)System.Buffers.Binary.BinaryPrimitives
            .ReadUInt32LittleEndian(poses.AsSpan(4, 4)));
        int retailEnd = checked((int)System.Buffers.Binary.BinaryPrimitives
            .ReadUInt32LittleEndian(poses.AsSpan(8, 4)));
        byte[] retailRecord = poses[retailStart..retailEnd];
        for (int offset = 0; offset < retailRecord.Length; offset++)
            memory.WriteU8(
                recordAddress + (uint)offset, retailRecord[offset]);
        context.GP = 0x8006A7F0u;
        context.SP = 0x801FF000u;
        context.RA = 0x80012340u;
        context.A0 = recordAddress;
        context.A1 = 16u;
        context.A2 = 16u;
        context.A3 = 0u;
        Console.Error.WriteLine(
            $"[SelectorPortraitProbe] retail=0 phase=loader begin " +
            $"bytes={retailRecord.Length}");
        Vigilante82PC.func_801109FC(context, memory);
        Console.Error.WriteLine(
            "[SelectorPortraitProbe] retail=0 phase=loader complete");

        var durations = new List<long>();
        for (int index = 0;
             index < V82VehicleRegistry.Roster().Length;
             index++)
        {
            string path = Path.Combine(shell, $"SELECTOR_{index:00}.PPM");
            ushort[] pixels =
                V82VehicleRegistry.BuildSelectorPortraitPixelsForProbe(path);
            Console.Error.WriteLine(
                $"[SelectorPortraitProbe] index={index} phase=upload " +
                $"pixels={pixels.Length}");
            Console.Error.WriteLine(
                $"[SelectorPortraitProbe] index={index} phase=upload begin");
            var timer = System.Diagnostics.Stopwatch.StartNew();
            V82VehicleRegistry.UploadSelectorPortraitForProbe(
                16, 16, pixels);
            timer.Stop();
            durations.Add(timer.ElapsedMilliseconds);
            const int width = 240;
            const int height = 421;
            byte[] rgb = CaptureVramRgb(16, 16, width, height);
            decodedPortraits.Add((width, height, rgb));
            if (portraitOutput != null)
                WritePpm(
                    Path.Combine(
                        portraitOutput, $"selector_{index:00}.ppm"),
                    width, height, rgb);
            Console.WriteLine(
                $"[SelectorPortraitProbe] index={index} pixels={pixels.Length} " +
                $"loader_ms={timer.ElapsedMilliseconds}");
        }
        if (portraitOutput != null)
        {
            string contact = Path.Combine(
                portraitOutput, "selector_native_decoder_contact.ppm");
            WritePortraitContactSheet(contact, decodedPortraits);
            Console.WriteLine(
                $"[SelectorPortraitProbe] visual={contact}");
        }
        Console.WriteLine(
            $"[SelectorPortraitProbe] {validation} returned=" +
            $"{durations.Count}/{V82VehicleRegistry.Roster().Length} " +
            $"max_ms={durations.Max()}");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(
            $"[SelectorPortraitProbe] FAILED: {exception}");
        return 1;
    }
}

if (args.Length == 1 &&
    args[0].Equals(
        "--probe-graphics-config", StringComparison.OrdinalIgnoreCase))
{
    ConfigManager.Load();
    ViewConfig view = ConfigManager.View;
    string resolved = view.ResolveGraphicsPreset();
    if (!view.GraphicsPreset.Equals(
            resolved, StringComparison.OrdinalIgnoreCase))
        throw new InvalidDataException(
            $"graphics preset label '{view.GraphicsPreset}' does not match " +
            $"configured values '{resolved}'");
    Console.WriteLine(
        $"[GraphicsConfig] preset={resolved} " +
        $"hle={(view.HighResolution3D ? "enhanced" : "software")} " +
        $"scale={(view.HighResolution3D ? view.InternalResolutionScale : 1)}x " +
        $"projection={(view.PerspectiveCorrectTextures ? "perspective" : "affine")} " +
        $"lod={view.LevelOfDetail} dithering={view.Ps1Dithering} " +
        $"smoothing={view.TextureSmoothing} aa={view.AntiAliasing} " +
        $"msaa={view.MsaaSamples} anisotropy={view.AnisotropicFiltering} " +
        $"mipmaps={view.TextureMipmaps} widescreen={view.Widescreen} " +
        $"world-texture-class={ViewConfig.MaximumWorldTextureClass} " +
        $"ui-texture-class={ViewConfig.MaximumUiTextureClass}");
    return 0;
}

if (args.Length == 2 &&
    args[0].Equals(
        "--probe-result-voice-proxies", StringComparison.OrdinalIgnoreCase))
{
    try
    {
        string package = Path.GetFullPath(args[1], launchDirectory);
        string validation = V82VehicleRegistry.ValidatePackage(package);
        var memory = new PSMemory();
        LoadPsxExecutable(
            memory, Path.Combine(package, "SLUS_008.68"));
        VehicleRosterItem[] roster = V82VehicleRegistry.Roster();
        var resolved = new List<string>(roster.Length);
        foreach (VehicleRosterItem item in roster)
        {
            foreach (bool defeated in new[] { false, true })
            {
                string? stem = V82VehicleRegistry.ResultVoiceStem(
                    item.Type, defeated);
                if (stem == null)
                    throw new InvalidDataException(
                        $"result voice bank missing for {item.StableId}");
                int channel = item.Type - 64;
                string expected =
                    $"V8VOICE\\{(defeated ? "D" : "V")}{channel:00}";
                if (!stem.Equals(expected, StringComparison.Ordinal))
                    throw new InvalidDataException(
                        $"result voice stem for {item.StableId} was " +
                        $"'{stem}', expected '{expected}'");
                resolved.Add(
                    $"{item.Type}:{item.StableId}:{(defeated ? "defeat" : "victory")}" +
                    $"={stem}");
            }
        }
        Console.WriteLine(
            $"[ResultVoiceProxy] {validation} resolved={roster.Length} " +
            $"{string.Join(',', resolved)}");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(
            $"[ResultVoiceProxy] FAILED: {exception}");
        return 1;
    }
}

if (args.Length == 2 &&
    args[0].Equals(
        "--probe-selector-lifecycle", StringComparison.OrdinalIgnoreCase))
{
    try
    {
        string package = Path.GetFullPath(args[1], launchDirectory);
        string validation = V82VehicleRegistry.ValidatePackage(package);
        var memory = new PSMemory();
        var context = new CpuContext
        {
            A1 = 1u,
            FP = 0u,
        };

        V82VehicleRegistry.BeginNativeSelector(context, memory);
        uint initial = V82VehicleRegistry.ResolveNativeSelectorSlot(
            context, memory);
        memory.WriteU32(0x8006B508u, 0x80000000u);
        context.FP = 17u;
        uint wrapped = V82VehicleRegistry.ResolveNativeSelectorSlot(
            context, memory);
        int guest = V82VehicleRegistry.NativeSelectorGuestIndex;
        if (initial != 0u || wrapped != 0u || guest != 11)
            throw new InvalidDataException(
                $"left wrap did not select Sid: initial={initial} " +
                $"wrapped={wrapped} guest={guest}");

        context.V0 = wrapped;
        V82VehicleRegistry.EndNativeSelector(context, memory);
        if (V82VehicleRegistry.SelectedType != 75)
            throw new InvalidDataException(
                $"Sid selection did not persist: " +
                $"type={V82VehicleRegistry.SelectedType}");

        context.A1 = 0u;
        V82VehicleRegistry.BeginNativeSelector(context, memory);
        context.RA = 0x80106A00u;
        context.A0 = 0x81234560u;
        bool nativeEnemyPortrait =
            V82VehicleRegistry.BeginNativeSelectorPortrait(context, memory);
        if (!nativeEnemyPortrait || context.A0 != 0x81234560u)
            throw new InvalidDataException(
                "enemy selector context replaced the native portrait");
        V82VehicleRegistry.EndNativeSelector(context, memory);
        if (V82VehicleRegistry.SelectedType != 75)
            throw new InvalidDataException(
                "enemy selector context displaced player-one Sid selection");

        // The enemy editor keeps retail proxy bytes in its four fixed rows.
        // Crossing left from retail slot zero must enter the final registered
        // guest, and the gameplay participant handoff must translate every
        // matching proxy occurrence without changing player one.
        context.A1 = 1u;
        V82VehicleRegistry.BeginNativeSelector(context, memory);
        context.RA = 0x80107A1Cu;
        context.A1 = 0x801008E8u;
        V82VehicleRegistry.ObserveNativeSelectorCall(context, memory);
        context.S3 = 0u;
        memory.WriteU8(0x8006B8F6u, (byte)0);
        // Reserve Sid's ordinary wrap proxy in row one. The guest row must
        // move to a distinct retail identity so participant expansion cannot
        // silently convert row one's stock enemy as well.
        memory.WriteU8(0x8006B8F7u, (byte)17);
        memory.WriteU32(0x8006B508u, 0u);
        V82VehicleRegistry.ApplyNativeEnemySelectorSlot(context, memory);
        memory.WriteU8(0x8006B8F6u, (byte)17);
        memory.WriteU32(0x8006B508u, 0x80000000u);
        V82VehicleRegistry.ApplyNativeEnemySelectorSlot(context, memory);
        int npcProxy = memory.ReadU8(0x8006B8F6u);
        if (V82VehicleRegistry.SelectedNpcTypeForSlot(0) != 75 ||
            npcProxy == 17)
            throw new InvalidDataException(
                "enemy selector did not isolate Sid's retail proxy");
        const uint participantBase = 0x80061104u;
        memory.WriteU8(participantBase, (byte)0);
        memory.WriteU8(participantBase + 1u, (byte)0xFF);
        memory.WriteU8(participantBase + 2u, checked((byte)npcProxy));
        memory.WriteU8(participantBase + 3u, (byte)17);
        V82VehicleRegistry.ApplySelectedNpcTypes(memory, participantBase);
        if (memory.ReadU8(participantBase) != 0u ||
            memory.ReadU8(participantBase + 2u) != 75u ||
            memory.ReadU8(participantBase + 3u) != 17u)
            throw new InvalidDataException(
                "isolated enemy proxy changed an unrelated stock participant");
        V82VehicleRegistry.EndNativeSelector(context, memory);

        Console.WriteLine(
            $"[SelectorLifecycle] {validation} left_wrap=Sid " +
            $"type=75 enemy_portrait=native player_selection=preserved " +
            $"npc_type=75 npc_proxy={npcProxy} stock_proxy=17_preserved");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(
            $"[SelectorLifecycle] FAILED: {exception}");
        return 1;
    }
}

if (args.Length == 2 &&
    args[0].Equals(
        "--probe-defeat-quit-lifetime", StringComparison.OrdinalIgnoreCase))
{
    try
    {
        string package = Path.GetFullPath(args[1], launchDirectory);
        string validation = V82VehicleRegistry.ValidatePackage(package);
        var memory = new PSMemory();
        var context = new CpuContext();
        string lifecycle =
            V82Compat.ProbeGuestIdentityLifetime(context, memory);
        Console.WriteLine(
            $"[DefeatQuitLifetime] {validation} {lifecycle}");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(
            $"[DefeatQuitLifetime] FAILED: {exception}");
        return 1;
    }
}

if (args.Length == 2 &&
    args[0].Equals(
        "--probe-vehicle-package", StringComparison.OrdinalIgnoreCase))
{
    string package = Path.GetFullPath(args[1], launchDirectory);
    string validation = V82VehicleRegistry.ValidatePackage(package);
    VehicleRosterItem[] roster = V82VehicleRegistry.Roster();
    if (roster.Length == 0)
        throw new InvalidDataException("vehicle package has an empty guest roster");
    V82VehicleRegistry.SelectType(roster[0].Type);
    if (V82VehicleRegistry.SelectedType != roster[0].Type)
        throw new InvalidOperationException("guest roster selection did not persist");
    V82VehicleRegistry.SelectType(-1);
    Console.WriteLine(
        $"[VehiclePackage] {validation} roster=" +
        $"{string.Join(',', roster.Select(item => $"{item.Type}:{item.StableId}"))}");
    return 0;
}

if (guestVehicle != null)
    V82VehicleRegistry.RequestSelection(guestVehicle);

string? explicitSource = args.Length switch
{
    0 => null,
    1 => Path.GetFullPath(args[0], launchDirectory),
    _ => throw new ArgumentException(
        "usage: Vigilante82PC [disc.cue|directory] [--loose <directory>] " +
        "[--muted] " +
        "[--probe-vehicle-package <directory>] [--probe-graphics-config] " +
        "[--probe-result-voice-proxies <directory>] " +
        "[--probe-defeat-quit-lifetime <directory>] " +
        "[--probe-selector-lifecycle <directory>] " +
        "[--probe-selector-portraits <directory> [output-directory]] " +
        "[--guest-vehicle <stable-id>]")
};

ConfigManager.Load();
string? loosePath =
    explicitLoose ?? ResolveLooseSource(explicitSource ?? executableDirectory);
string importedLooseRoot = V82LooseImporter.DefaultRoot;
if (loosePath == null && explicitSource == null &&
    V82LooseImporter.IsComplete(importedLooseRoot))
    loosePath = importedLooseRoot;
string? cuePath = loosePath == null
    ? ResolveCue(explicitSource ?? executableDirectory)
    : null;
if (loosePath == null)
{
    Console.WriteLine(
        $"[Import] preparing standalone game data from {cuePath}");
    V82LooseImporter.Import(cuePath!, importedLooseRoot, ReportLooseImport);
    loosePath = V82LooseImporter.IsComplete(importedLooseRoot)
        ? importedLooseRoot
        : throw new InvalidDataException(
            $"Disc import did not complete: {importedLooseRoot}");
    cuePath = null;
}
if (loosePath != null)
{
    ConfigManager.Game.CdPath = "";
    Runtime.SetLoosePath(loosePath);
    Console.WriteLine($"[Host] standalone-loose={loosePath}");
}
else
{
    Runtime.SetLoosePath(null);
    ConfigManager.Game.CdPath = cuePath!;
    ConfigManager.SaveGame();
    Console.WriteLine($"[Host] disc={cuePath}");
}

PreloadBundledNative("SDL2.dll");
Runtime.SetMode(RunMode.Devkit);
Entry.Run(new PSMemory(), cuePath, loosePath);
Console.Error.WriteLine("[Host] emulation entry returned");
return 0;

static byte[] CaptureVramRgb(int x, int y, int width, int height)
{
    ushort[] vram = Runtime.Gpu?.Vram ??
        throw new InvalidOperationException("GPU is unavailable");
    byte[] rgb = new byte[checked(width * height * 3)];
    int output = 0;
    for (int row = 0; row < height; row++)
    for (int column = 0; column < width; column++)
    {
        ushort pixel = vram[
            ((y + row) & (Gpu.VramHeight - 1)) * Gpu.VramWidth +
            ((x + column) & (Gpu.VramWidth - 1))];
        rgb[output++] = Expand5(pixel & 31);
        rgb[output++] = Expand5(pixel >> 5 & 31);
        rgb[output++] = Expand5(pixel >> 10 & 31);
    }
    return rgb;
}

static void ReportLooseImport(LooseImportProgress progress)
{
    Console.WriteLine(
        $"[Import] {progress.Phase} {progress.Current}/{progress.Total} " +
        progress.Item);
}

static byte Expand5(int value) =>
    checked((byte)(value << 3));

static void WritePpm(string path, int width, int height, byte[] rgb)
{
    using FileStream output = File.Create(path);
    byte[] header = System.Text.Encoding.ASCII.GetBytes(
        $"P6\n{width} {height}\n255\n");
    output.Write(header);
    output.Write(rgb);
}

static void WritePortraitContactSheet(
    string path, List<(int Width, int Height, byte[] Rgb)> portraits)
{
    const int columns = 4;
    const int cellWidth = 260;
    const int cellHeight = 422;
    int rows = (portraits.Count + columns - 1) / columns;
    int width = columns * cellWidth;
    int height = rows * cellHeight;
    byte[] contact = new byte[checked(width * height * 3)];
    Array.Fill(contact, (byte)16);
    for (int index = 0; index < portraits.Count; index++)
    {
        var portrait = portraits[index];
        int cellX = index % columns * cellWidth;
        int cellY = index / columns * cellHeight;
        int startX = cellX + (cellWidth - portrait.Width) / 2;
        int startY = cellY + (cellHeight - portrait.Height) / 2;
        for (int row = 0; row < portrait.Height; row++)
        {
            int source = row * portrait.Width * 3;
            int destination =
                ((startY + row) * width + startX) * 3;
            Array.Copy(
                portrait.Rgb, source, contact, destination,
                portrait.Width * 3);
        }
        for (int x = cellX; x < cellX + cellWidth; x++)
        {
            SetContactPixel(contact, width, x, cellY, 248, 192, 0);
            SetContactPixel(
                contact, width, x, cellY + cellHeight - 1, 248, 192, 0);
        }
        for (int y = cellY; y < cellY + cellHeight; y++)
        {
            SetContactPixel(contact, width, cellX, y, 248, 192, 0);
            SetContactPixel(
                contact, width, cellX + cellWidth - 1, y, 248, 192, 0);
        }
    }
    WritePpm(path, width, height, contact);
}

static void SetContactPixel(
    byte[] output, int width, int x, int y,
    byte red, byte green, byte blue)
{
    int offset = (y * width + x) * 3;
    output[offset] = red;
    output[offset + 1] = green;
    output[offset + 2] = blue;
}

static string? ResolveLooseSource(string source)
{
    if (!Directory.Exists(source) ||
        !File.Exists(Path.Combine(source, "SYSTEM.CNF")))
        return null;
    return Path.GetFullPath(source);
}

static string ResolveCue(string source)
{
    if (File.Exists(source))
    {
        if (!Path.GetExtension(source).Equals(".cue", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException($"Expected a CUE file: {source}");
        return Path.GetFullPath(source);
    }

    if (!Directory.Exists(source))
        throw new FileNotFoundException($"Game source was not found: {source}");

    string[] cues = Directory
        .EnumerateFiles(source, "*.cue", SearchOption.TopDirectoryOnly)
        .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
        .ToArray();
    if (cues.Length == 0)
    {
        string? nearbyCue = FindNearbySequelCue(source);
        if (nearbyCue != null)
            return nearbyCue;
        throw new FileNotFoundException(
            $"No Vigilante 8: 2nd Offense CUE file was found in or near: {source}");
    }
    return Path.GetFullPath(
        cues.FirstOrDefault(path => Path.GetFileName(path).Contains(
            "2nd Offensive", StringComparison.OrdinalIgnoreCase)) ?? cues[0]);
}

static string? FindNearbySequelCue(string source)
{
    DirectoryInfo? cursor = new(Path.GetFullPath(source));
    for (int depth = 0; depth < 8 && cursor != null; depth++, cursor = cursor.Parent)
    {
        string candidateDirectory = Path.Combine(cursor.FullName, "V8_2_BINCUE");
        if (!Directory.Exists(candidateDirectory))
            continue;

        string? cue = Directory
            .EnumerateFiles(candidateDirectory, "*.cue", SearchOption.TopDirectoryOnly)
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault(path => Path.GetFileName(path).Contains(
                "2nd Offensive", StringComparison.OrdinalIgnoreCase));
        if (cue != null)
            return Path.GetFullPath(cue);
    }
    return null;
}

static void PreloadBundledNative(string fileName)
{
    string? searchDirectories =
        AppContext.GetData("NATIVE_DLL_SEARCH_DIRECTORIES") as string;
    if (string.IsNullOrWhiteSpace(searchDirectories))
        return;

    foreach (string directory in searchDirectories.Split(
                 Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
    {
        string candidate = Path.Combine(directory, fileName);
        if (!File.Exists(candidate))
            continue;
        System.Runtime.InteropServices.NativeLibrary.Load(candidate);
        Console.WriteLine($"[Host] preloaded bundled native library: {fileName}");
        return;
    }
}

static void LoadPsxExecutable(PSMemory memory, string path)
{
    byte[] executable = File.ReadAllBytes(path);
    if (executable.Length < 0x800 ||
        !executable.AsSpan(0, 8).SequenceEqual("PS-X EXE"u8))
        throw new InvalidDataException($"Not a PS-X executable: {path}");
    uint address = System.Buffers.Binary.BinaryPrimitives
        .ReadUInt32LittleEndian(executable.AsSpan(0x18, 4));
    uint length = System.Buffers.Binary.BinaryPrimitives
        .ReadUInt32LittleEndian(executable.AsSpan(0x1C, 4));
    if (length > executable.Length - 0x800)
        throw new InvalidDataException(
            $"PS-X executable payload is truncated: {path}");
    for (uint offset = 0; offset < length; offset++)
        memory.WriteU8(address + offset, executable[0x800 + offset]);
}
