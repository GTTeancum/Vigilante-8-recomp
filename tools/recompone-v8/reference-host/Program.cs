using RecompOne.Runtime.Config;
using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using Recompiled;

string launchDirectory = Environment.CurrentDirectory;
Environment.CurrentDirectory = AppContext.BaseDirectory;
if (Environment.GetEnvironmentVariable("RECOMPONE_V8_GAME_VOLUME") == null)
    Environment.SetEnvironmentVariable("RECOMPONE_V8_GAME_VOLUME", "0");

string? explicitSource = null;
string? explicitLoose = null;
bool disableLoose = false;
bool probeSource = false;
string? probeFile = null;
string? probeVehiclePackage = null;
string? guestVehicle = null;
for (int i = 0; i < args.Length; i++)
{
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

if (explicitLoose != null && disableLoose)
{
    PrintUsage();
    return 1;
}

if (guestVehicle != null)
    V8VehicleRegistry.RequestSelection(guestVehicle);

ConfigManager.Load();
(string? cuePath, string? loosePath) = ResolveSource(
    explicitSource, explicitLoose, disableLoose);
if (loosePath != null)
{
    Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", loosePath);
    ConfigManager.Game.CdPath = "";
    Console.WriteLine($"[Host] standalone-loose={loosePath}");
}
else if (cuePath != null)
{
    Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", "0");
    ConfigManager.Game.CdPath = cuePath;
    ConfigManager.SaveGame();
    Console.WriteLine($"[Host] disc={cuePath}");
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
        : CueFs.Open(cuePath!);
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
Entry.Run(new PSMemory(), cuePath, loosePath);
return 0;

string ResolveLaunchPath(string path) => Path.GetFullPath(path, launchDirectory);

static void PrintUsage() => Console.Error.WriteLine(
    "usage: Vigilante8PC [disc.cue|loose-directory] [--loose <directory>] " +
    "[--no-loose] [--probe-source] [--probe-file <game-path>] " +
    "[--probe-vehicle-package <directory>] [--guest-vehicle <stable-id>]");

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

static (string? CuePath, string? LoosePath) ResolveSource(
    string? explicitSource,
    string? explicitLoose,
    bool disableLoose)
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

    if (!disableLoose && File.Exists(Path.Combine(AppContext.BaseDirectory, "SYSTEM.CNF")))
        return (null, RequireLooseRoot(AppContext.BaseDirectory));
    return (FindCue(AppContext.BaseDirectory), null);
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
