using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Memory;
using RecompOne.Runtime.Sdk;
using RecompOne.Runtime;
using Recompiled;

AppDomain.CurrentDomain.ProcessExit += (_, _) =>
    Console.Error.WriteLine(
        $"[Host] process exit requested (code={Environment.ExitCode})");

string launchDirectory = Environment.CurrentDirectory;
Environment.CurrentDirectory = AppContext.BaseDirectory;

string? guestVehicle = null;
var launchArguments = args.ToList();
for (int index = 0; index < launchArguments.Count; index++)
{
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
        "usage: Vigilante82PC [disc.cue|directory] " +
        "[--probe-vehicle-package <directory>] [--guest-vehicle <stable-id>]")
};

ConfigManager.Load();
string cuePath = ResolveCue(explicitSource ?? AppContext.BaseDirectory);
ConfigManager.Game.CdPath = cuePath;
ConfigManager.SaveGame();
Console.WriteLine($"[Host] disc={cuePath}");

PreloadBundledNative("SDL2.dll");
Runtime.SetMode(RunMode.Devkit);
if (string.IsNullOrWhiteSpace(
        Environment.GetEnvironmentVariable("RECOMPONE_LOOSE_DIR")))
    Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", "0");
Entry.Run(new PSMemory(), cuePath);
Console.Error.WriteLine("[Host] emulation entry returned");
return 0;

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
