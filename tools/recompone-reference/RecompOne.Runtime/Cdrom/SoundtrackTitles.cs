using System.Text.Json;
using RecompOne.Runtime.Serialization;

namespace RecompOne.Runtime.Cdrom;

/// <summary>
/// Song-list titles for soundtrack files the game's own title table does not
/// cover, keyed by the manifest's source path. The files keep the names the
/// first game's disc gives them (REDBOOK/BEEZEWAX.RAW and REDBOOK/SHELIA.RAW
/// are misspelled on the disc), while the list shows the characters' names.
/// The table is data (SoundtrackTitles.json), not code, so a corrected or
/// added title needs no runtime change.
/// </summary>
public static class SoundtrackTitles
{
    static readonly Lazy<Dictionary<string, string>> _titles = new(Load);

    /// <summary>The title for a source such as "music/BOOGIE.ogg", or null.</summary>
    public static string? For(string? source) =>
        source != null && _titles.Value.TryGetValue(source.Replace('\\', '/'), out string? title)
            ? title
            : null;

    static Dictionary<string, string> Load()
    {
        var assembly = typeof(SoundtrackTitles).Assembly;
        string resource = assembly.GetManifestResourceNames().Single(name =>
            name.EndsWith("SoundtrackTitles.json", StringComparison.Ordinal));
        using Stream stream = assembly.GetManifestResourceStream(resource)!;
        var titles = JsonSerializer.Deserialize(stream, RuntimeJsonContext.Default.DictionaryStringString) ?? [];
        return new Dictionary<string, string>(titles, StringComparer.OrdinalIgnoreCase);
    }
}
