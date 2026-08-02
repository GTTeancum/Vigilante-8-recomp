using RecompOne.Runtime.Config;
using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using Recompiled;

string launchDirectory = Environment.CurrentDirectory;
string executableDirectory =
    Path.GetDirectoryName(Environment.ProcessPath) ??
    AppContext.BaseDirectory;
Environment.CurrentDirectory = executableDirectory;

if (args.Length == 3 &&
    args[0].Equals(
        "--probe-native-selector-records",
        StringComparison.OrdinalIgnoreCase))
{
    string loose = Path.GetFullPath(args[1], launchDirectory);
    string outputDirectory = Path.GetFullPath(args[2], launchDirectory);
    Directory.CreateDirectory(outputDirectory);
    var memory = new PSMemory();
    LoadPsxExecutable(memory, Path.Combine(loose, "SLUS_005.10"));
    memory.LoadBytes(
        0x80100000u,
        File.ReadAllBytes(Path.Combine(loose, "SHELL", "SHELL.DLL")));
    Dispatcher.Register("main", new MainDispatchTable());
    Dispatcher.Register("SHELL", new SHELLDispatchTable());
    Dispatcher.Load("main");
    Dispatcher.Load("SHELL");
    var context = new CpuContext
    {
        GP = 0u,
        SP = 0x801FF000u,
        RA = 0x80012340u,
    };
    RecompOne.Runtime.Runtime.SetMode(RecompOne.Runtime.RunMode.Devkit);
    RecompOne.Runtime.Runtime.SetContext(context, memory);
    RecompOne.Runtime.Bios.Bios.Init(memory);
    const uint heap = 0x80140000u;
    const uint heapEnd = 0x801F0000u;
    memory.WriteU32(0x8005ED4Cu, heap);
    memory.WriteU32(heap, heap);
    memory.WriteU32(heap + 4u, (heapEnd - heap) >> 3);

    byte[] table = File.ReadAllBytes(
        Path.Combine(loose, "SHELL", "CHARSEL1.TBL"));
    int count = System.Buffers.Binary.BinaryPrimitives
        .ReadInt32LittleEndian(table.AsSpan(0, 4));
    if (count < 12)
        throw new InvalidDataException(
            $"CHARSEL1.TBL contains only {count} records");
    const uint recordAddress = 0x80120000u;
    var portraits = new List<(int Width, int Height, byte[] Rgb)>();
    int probeLimit = int.TryParse(
        Environment.GetEnvironmentVariable("RECOMPONE_SELECTOR_PROBE_LIMIT"),
        out int requestedLimit)
            ? Math.Clamp(requestedLimit, 1, 12)
            : 12;
    for (int index = 0; index < probeLimit; index++)
    {
        int start = System.Buffers.Binary.BinaryPrimitives
            .ReadInt32LittleEndian(table.AsSpan(4 + index * 4, 4));
        int end = System.Buffers.Binary.BinaryPrimitives
            .ReadInt32LittleEndian(table.AsSpan(8 + index * 4, 4));
        byte[] record = table[start..end];
        memory.LoadBytes(recordAddress, record);
        context.GP = 0u;
        context.SP = 0x801FF000u;
        context.RA = 0x80012340u;
        context.A0 = recordAddress;
        context.A1 = 16u;
        context.A2 = 16u;
        context.A3 = 0u;
        Console.Error.WriteLine(
            $"[V8SelectorRecordProbe] index={index} phase=loader begin " +
            $"bytes={record.Length}");
        var timer = System.Diagnostics.Stopwatch.StartNew();
        Vigilante8PC.FUN_8010dc18(context, memory);
        timer.Stop();
        int width = memory.ReadU16(recordAddress);
        int height = memory.ReadU16(recordAddress + 2u);
        byte[] rgb = CaptureVramRgb(16, 16, width, height);
        portraits.Add((width, height, rgb));
        WritePpm(
            Path.Combine(outputDirectory, $"selector_{index:00}.ppm"),
            width, height, rgb);
        Console.WriteLine(
            $"[V8SelectorRecordProbe] index={index} bytes={record.Length} " +
            $"size={width}x{height} loader_ms={timer.ElapsedMilliseconds}");
    }
    string contact = Path.Combine(
        outputDirectory, "selector_native_v8_contact.ppm");
    WritePortraitContactSheet(contact, portraits);
    Console.WriteLine($"[V8SelectorRecordProbe] visual={contact}");
    return 0;
}

string? explicitSource = null;
string? explicitLoose = null;
string? explicitOverrides = null;
bool disableLoose = false;
bool probeSource = false;
string? probeFile = null;
string? probeVehiclePackage = null;
string? guestVehicle = null;
string? guestArena = null;
for (int i = 0; i < args.Length; i++)
{
    if (args[i].Equals("--guest-arena", StringComparison.OrdinalIgnoreCase) &&
        i + 1 < args.Length)
    {
        guestArena = args[++i];
        continue;
    }
    if (args[i].Equals("--guest-vehicle", StringComparison.OrdinalIgnoreCase) &&
        i + 1 < args.Length)
    {
        guestVehicle = args[++i];
        continue;
    }
    if (args[i].Equals(
            "--probe-vehicle-package", StringComparison.OrdinalIgnoreCase) &&
        i + 1 < args.Length)
    {
        probeVehiclePackage = ResolveLaunchPath(args[++i]);
        continue;
    }
    if (args[i].Equals("--probe-source", StringComparison.OrdinalIgnoreCase))
    {
        probeSource = true;
        continue;
    }
    if (args[i].Equals("--probe-file", StringComparison.OrdinalIgnoreCase) && i + 1 < args.Length)
    {
        probeSource = true;
        probeFile = args[++i];
        continue;
    }
    if (args[i].Equals("--loose", StringComparison.OrdinalIgnoreCase) && i + 1 < args.Length)
    {
        explicitLoose = ResolveLaunchPath(args[++i]);
        continue;
    }
    if (args[i].Equals("--overrides", StringComparison.OrdinalIgnoreCase) &&
        i + 1 < args.Length)
    {
        explicitOverrides = ResolveLaunchPath(args[++i]);
        continue;
    }
    if (args[i].Equals("--no-loose", StringComparison.OrdinalIgnoreCase))
    {
        disableLoose = true;
        continue;
    }
    if (explicitSource != null)
    {
        PrintUsage();
        return 1;
    }
    explicitSource = ResolveLaunchPath(args[i]);
}

if (probeVehiclePackage != null)
{
    string validation =
        V8VehicleRegistry.ValidatePackage(probeVehiclePackage);
    VehicleRosterItem[] roster = V8VehicleRegistry.Roster();
    if (roster.Length == 0)
        throw new InvalidDataException("vehicle package has an empty guest roster");
    V8VehicleRegistry.SelectType(roster[0].Type);
    if (V8VehicleRegistry.SelectedType != roster[0].Type)
        throw new InvalidOperationException("guest roster selection did not persist");
    V8VehicleRegistry.SelectType(-1);
    Console.WriteLine(
        $"[VehiclePackage] {validation} roster=" +
        $"{string.Join(',', roster.Select(item => $"{item.Type}:{item.StableId}"))}");
    return 0;
}

if ((explicitLoose != null || explicitOverrides != null) && disableLoose)
{
    PrintUsage();
    return 1;
}

if (guestVehicle != null)
    V8VehicleRegistry.RequestSelection(guestVehicle);
if (guestArena != null)
    V8ArenaRegistry.RequestSelection(guestArena);

ConfigManager.Load();
(string? cuePath, string? loosePath) = ResolveSource(
    explicitSource, explicitLoose, disableLoose, executableDirectory);
string? overridePath = null;
if (cuePath != null && !disableLoose)
{
    if (explicitOverrides != null)
    {
        if (!Directory.Exists(explicitOverrides))
            throw new DirectoryNotFoundException(
                $"Loose override root not found: {explicitOverrides}");
        overridePath = Path.GetFullPath(explicitOverrides);
    }
    else if (Directory.Exists(Path.Combine(executableDirectory, "TERRAIN")))
    {
        overridePath = executableDirectory;
    }
}
else if (explicitOverrides != null)
{
    throw new InvalidOperationException(
        "--overrides requires a CUE source; standalone loose mode already " +
        "uses its complete asset root");
}
if (loosePath != null)
{
    Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", loosePath);
    ConfigManager.Game.CdPath = "";
    ConfigManager.SaveGame();
    Console.WriteLine($"[Host] standalone-loose={loosePath}");
}
else if (cuePath != null)
{
    Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", "0");
    ConfigManager.Game.CdPath = cuePath;
    ConfigManager.SaveGame();
    Console.WriteLine(
        $"[Host] disc={cuePath}" +
        (overridePath == null ? "" : $" overrides={overridePath}"));
}
else
{
    throw new FileNotFoundException(
        "No game source found. Put Vigilante8PC.exe beside the extracted loose " +
        "SYSTEM.CNF tree, or beside/pass a retail CUE file.");
}

if (probeSource)
{
    using var source = loosePath != null
        ? CueFs.OpenLoose(loosePath)
        : CueFs.Open(cuePath!, overridePath);
    byte[] system = source.ReadFile("SYSTEM.CNF");
    Console.WriteLine(
        $"[SourceProbe] mode={(source.IsStandaloneLoose ? "standalone-loose" : "cue")} " +
        $"root={loosePath ?? cuePath} files={source.LooseOverrideCount} " +
        $"systemCnfBytes={system.Length}");
    if (!string.IsNullOrWhiteSpace(probeFile))
    {
        if (!source.Locate(probeFile, out int lba, out uint size))
            throw new FileNotFoundException($"Game file not found: {probeFile}");
        byte[] file = source.ReadFile(probeFile);
        byte[] cooked = source.ReadSectorData(lba, 2048);
        byte[] stream = source.ReadSectorData(lba, 2336);
        byte[] raw = source.ReadSectorData(lba, 2352);
        static string Sha256(byte[] data) => Convert.ToHexString(
            System.Security.Cryptography.SHA256.HashData(data));
        Console.WriteLine(
            $"[SourceProbeFile] path={probeFile} lba={lba} size={size} " +
            $"bytes={file.Length} sha256={Sha256(file)} " +
            $"sector2048={Sha256(cooked)} sector2336={Sha256(stream)} " +
            $"sector2352={Sha256(raw)}");
    }
    return 0;
}

PreloadBundledNative("SDL2.dll");
V8DreamlandCompat.RegisterHostFunctions();
Entry.Run(new PSMemory(), cuePath, loosePath, overridePath);
return 0;

string ResolveLaunchPath(string path) => Path.GetFullPath(path, launchDirectory);

static void PrintUsage() => Console.Error.WriteLine(
    "usage: Vigilante8PC [disc.cue|loose-directory] [--loose <directory>] " +
    "[--overrides <directory>] " +
    "[--no-loose] [--probe-source] [--probe-file <game-path>] " +
    "[--probe-vehicle-package <directory>] [--guest-vehicle <stable-id>] " +
    "[--guest-arena <stable-id>]");

static void PreloadBundledNative(string fileName)
{
    string? searchDirectories =
        AppContext.GetData("NATIVE_DLL_SEARCH_DIRECTORIES") as string;
    if (string.IsNullOrWhiteSpace(searchDirectories)) return;

    foreach (string directory in searchDirectories.Split(
                 Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
    {
        string candidate = Path.Combine(directory, fileName);
        if (!File.Exists(candidate)) continue;
        System.Runtime.InteropServices.NativeLibrary.Load(candidate);
        Console.WriteLine($"[Host] preloaded bundled native library: {fileName}");
        return;
    }
}

static byte[] CaptureVramRgb(int x, int y, int width, int height)
{
    ushort[] vram = RecompOne.Runtime.Runtime.Gpu?.Vram ??
        throw new InvalidOperationException("GPU is unavailable");
    byte[] rgb = new byte[checked(width * height * 3)];
    int output = 0;
    for (int row = 0; row < height; row++)
    for (int column = 0; column < width; column++)
    {
        ushort pixel = vram[
            ((y + row) & (RecompOne.Runtime.Gpu.VramHeight - 1)) *
            RecompOne.Runtime.Gpu.VramWidth +
            ((x + column) & (RecompOne.Runtime.Gpu.VramWidth - 1))];
        rgb[output++] = Expand5(pixel & 31);
        rgb[output++] = Expand5(pixel >> 5 & 31);
        rgb[output++] = Expand5(pixel >> 10 & 31);
    }
    return rgb;
}

static byte Expand5(int value) =>
    checked((byte)((value << 3) | value >> 2));

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
    const int cellHeight = 380;
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
    }
    WritePpm(path, width, height, contact);
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
    memory.LoadBytes(
        address,
        executable.AsSpan(0x800, checked((int)length)).ToArray());
}

static (string? CuePath, string? LoosePath) ResolveSource(
    string? explicitSource,
    string? explicitLoose,
    bool disableLoose,
    string executableDirectory)
{
    if (explicitLoose != null)
        return (null, RequireLooseRoot(explicitLoose));

    if (explicitSource != null)
    {
        if (Directory.Exists(explicitSource))
        {
            if (!disableLoose && File.Exists(Path.Combine(explicitSource, "SYSTEM.CNF")))
                return (null, RequireLooseRoot(explicitSource));
            string? cue = FindCue(explicitSource);
            if (cue != null) return (cue, null);
            throw new FileNotFoundException($"No loose SYSTEM.CNF or CUE found: {explicitSource}");
        }
        if (!File.Exists(explicitSource))
            throw new FileNotFoundException($"Game source not found: {explicitSource}");
        if (!Path.GetExtension(explicitSource).Equals(".cue", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException($"Expected a CUE file or loose directory: {explicitSource}");
        return (Path.GetFullPath(explicitSource), null);
    }

    if (!disableLoose && File.Exists(Path.Combine(executableDirectory, "SYSTEM.CNF")))
        return (null, RequireLooseRoot(executableDirectory));
    return (FindCue(executableDirectory), null);
}

static string RequireLooseRoot(string directory)
{
    string root = Path.GetFullPath(directory);
    if (!Directory.Exists(root) || !File.Exists(Path.Combine(root, "SYSTEM.CNF")))
        throw new DirectoryNotFoundException(
            $"Standalone loose root must contain SYSTEM.CNF: {root}");
    return root;
}

static string? FindCue(string directory)
{
    var cues = Directory.EnumerateFiles(directory, "*.cue", SearchOption.TopDirectoryOnly)
        .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
        .ToArray();
    if (cues.Length == 0) return null;
    return Path.GetFullPath(
        cues.FirstOrDefault(path =>
            Path.GetFileName(path).Contains("Vigilante 8", StringComparison.OrdinalIgnoreCase))
        ?? cues[0]);
}
