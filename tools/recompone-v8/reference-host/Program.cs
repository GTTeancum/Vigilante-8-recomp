using RecompOne.Runtime.Config;
using RecompOne.Runtime.Cdrom;
using RecompOne.Runtime.Memory;
using Recompiled;

Environment.CurrentDirectory = AppContext.BaseDirectory;
if (Environment.GetEnvironmentVariable("RECOMPONE_V8_GAME_VOLUME") == null)
    Environment.SetEnvironmentVariable("RECOMPONE_V8_GAME_VOLUME", "0");

string? explicitCue = null;
bool probeSource = false;
for (int i = 0; i < args.Length; i++)
{
    if (args[i].Equals("--probe-source", StringComparison.OrdinalIgnoreCase))
    {
        probeSource = true;
        continue;
    }
    if (args[i].Equals("--loose", StringComparison.OrdinalIgnoreCase) && i + 1 < args.Length)
    {
        Environment.SetEnvironmentVariable(
            "RECOMPONE_LOOSE_DIR", Path.GetFullPath(args[++i]));
        continue;
    }
    if (args[i].Equals("--no-loose", StringComparison.OrdinalIgnoreCase))
    {
        Environment.SetEnvironmentVariable("RECOMPONE_LOOSE_DIR", "0");
        continue;
    }
    if (explicitCue != null)
    {
        Console.Error.WriteLine(
            "usage: Vigilante8PC [disc.cue] [--loose <directory>] [--no-loose] [--probe-source]");
        return 1;
    }
    explicitCue = Path.GetFullPath(args[i]);
}

ConfigManager.Load();
string? cuePath = ResolveCue(explicitCue, ConfigManager.Game.CdPath);
if (cuePath != null)
{
    ConfigManager.Game.CdPath = cuePath;
    ConfigManager.SaveGame();
    Console.WriteLine($"[Host] disc={cuePath}");
}
else
{
    ConfigManager.Game.CdPath = "";
    Console.WriteLine("[Host] no neighboring CUE found; opening the disc picker");
}

if (probeSource)
{
    if (cuePath == null) return 2;
    string? loosePath = RecompOne.Runtime.Runtime.ResolveLoosePath(cuePath);
    using var source = CueFs.Open(cuePath, loosePath);
    byte[] system = source.ReadFile("SYSTEM.CNF");
    Console.WriteLine(
        $"[SourceProbe] cue={cuePath} loose={loosePath ?? "disabled"} " +
        $"overrides={source.LooseOverrideCount} systemCnfBytes={system.Length}");
    return 0;
}

PreloadBundledNative("SDL2.dll");
Entry.Run(new PSMemory(), cuePath);
return 0;

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

static string? ResolveCue(string? explicitCue, string? savedCue)
{
    if (!string.IsNullOrWhiteSpace(explicitCue))
    {
        if (File.Exists(explicitCue)) return Path.GetFullPath(explicitCue);
        if (Directory.Exists(explicitCue)) return FindCue(explicitCue);
        throw new FileNotFoundException($"Disc CUE not found: {explicitCue}");
    }

    string? adjacent = FindCue(AppContext.BaseDirectory);
    if (adjacent != null) return adjacent;
    string siblingBinCue = Path.GetFullPath(
        Path.Combine(AppContext.BaseDirectory, "..", "BINCUE"));
    string? sibling = Directory.Exists(siblingBinCue) ? FindCue(siblingBinCue) : null;
    if (sibling != null) return sibling;
    return !string.IsNullOrWhiteSpace(savedCue) && File.Exists(savedCue)
        ? Path.GetFullPath(savedCue)
        : null;
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
