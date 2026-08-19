using System.Reflection;
using System.Text.Json;
using RecompOne.Runtime.Serialization;

namespace RecompOne.Runtime.Cdrom;

internal sealed class V8LooseManifest
{
    public int FormatVersion { get; set; }
    public string Volume { get; set; } = "";
    public int LeadOutLba { get; set; }
    public Dictionary<int, string> MetadataSectors { get; set; } = [];
    public List<V8LooseFile> Files { get; set; } = [];
    public List<V8LooseTrack> Tracks { get; set; } = [];

    public static V8LooseManifest Load(string looseRoot)
    {
        string systemPath = Path.Combine(looseRoot, "SYSTEM.CNF");
        string system = File.ReadAllText(systemPath);
        string manifestFile = system.Contains(
            "SLUS_008.68", StringComparison.OrdinalIgnoreCase)
            ? "V82LooseManifest.json"
            : system.Contains("SLUS_005.10", StringComparison.OrdinalIgnoreCase)
                ? "V8LooseManifest.json"
                : throw new InvalidDataException(
                    $"Unsupported loose SYSTEM.CNF: {systemPath}");
        return LoadEmbedded(manifestFile);
    }

    internal static V8LooseManifest LoadEmbedded(string manifestFile)
    {
        var assembly = typeof(V8LooseManifest).Assembly;
        string resourceName = assembly.GetManifestResourceNames().Single(name =>
            name.EndsWith(manifestFile, StringComparison.Ordinal));
        using Stream stream = assembly.GetManifestResourceStream(resourceName) ??
            throw new InvalidOperationException(
                $"Embedded loose-disc manifest is missing: {resourceName}");
        var manifest = JsonSerializer.Deserialize(stream, RuntimeJsonContext.Default.V8LooseManifest) ??
            throw new InvalidDataException("Loose-disc manifest is empty");
        if (manifest.FormatVersion != 1 || manifest.Files.Count == 0 || manifest.Tracks.Count == 0)
            throw new InvalidDataException(
                $"Unsupported loose-disc manifest version {manifest.FormatVersion}");
        return manifest;
    }
}

internal sealed class V8LooseFile
{
    public string Path { get; set; } = "";
    public int Lba { get; set; }
    public uint Size { get; set; }
}

internal sealed class V8LooseTrack
{
    public int Number { get; set; }
    public int Index0Lba { get; set; }
    public int StartLba { get; set; }
    public int EndLba { get; set; }
    public string? Source { get; set; }
}
