using System.Text.RegularExpressions;
using TextEncoding = System.Text.Encoding;

namespace RecompOne.Runtime.Cdrom;

/// <summary>
/// Imports the original Vigilante 8 album from the first game's own disc:
/// only its 12 CD-audio tracks, never its game data. SoundtrackDisc plays them
/// beside the Second Offense music under the names V8LooseManifest gives.
///
/// Before Second Offense is installed there is no game_data to put them in,
/// and its import refuses an existing half-filled one, so the album waits in a
/// staging folder that the Second Offense import adopts when it commits.
/// </summary>
public static class V8SoundtrackImporter
{
    // Vigilante 8 (USA). Discs and tools write it SLUS_005.10 (the CD file
    // name), SLUS_00510 or SLUS-00510, so only letters and digits are compared.
    const string TitleId = "SLUS00510";
    // Encoded OGG is roughly a fifth of the CD audio; keep a generous margin.
    const long FreeSpaceMargin = 256L * 1024 * 1024;

    /// <summary>Where the album waits until Second Offense is installed.</summary>
    public static string StagingRoot(string gameDataRoot) =>
        Path.GetFullPath(gameDataRoot).TrimEnd(
            Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + ".v8-soundtrack";

    /// <summary>The executable SYSTEM.CNF boots, such as SLUS_005.10.</summary>
    public static string? BootExecutable(string cuePath)
    {
        using var files = CueFs.Open(cuePath);
        string system = TextEncoding.ASCII.GetString(files.ReadFile("SYSTEM.CNF"));
        var match = Regex.Match(system, @"(?im)^\s*BOOT\s*=\s*cdrom:[\\/]*([^;\s]+);1\s*$");
        return match.Success ? match.Groups[1].Value.ToUpperInvariant() : null;
    }

    /// <summary>True for SLUS_005.10, SLUS_00510, SLUS-00510 and the like.</summary>
    public static bool IsFirstGameTitle(string? titleId) =>
        titleId != null &&
        string.Concat(titleId.Where(char.IsLetterOrDigit)).Equals(
            TitleId, StringComparison.OrdinalIgnoreCase);

    /// <summary>
    /// True for the first game's USA disc. An unreadable disc is not one, so
    /// the Second Offense import still sees it and reports as it always has.
    /// </summary>
    public static bool IsFirstGameDisc(string cuePath)
    {
        try { return IsFirstGameTitle(BootExecutable(cuePath)); }
        catch (Exception ex) when (ex is IOException or InvalidDataException or UnauthorizedAccessException)
        {
            return false;
        }
    }

    /// <summary>
    /// Encodes tracks 2-13 into the installed game's music folder, or into
    /// the staging folder when Second Offense is not installed yet. Returns
    /// true when the album went straight into the installed game.
    /// </summary>
    public static bool Import(
        string cuePath, string gameDataRoot,
        Action<LooseImportProgress>? report = null, CancellationToken cancellation = default)
    {
        cuePath = Path.GetFullPath(cuePath);
        gameDataRoot = Path.GetFullPath(gameDataRoot);
        report?.Invoke(new("validating", 0, 1, "Checking Vigilante 8 title ID"));
        if (!IsFirstGameTitle(BootExecutable(cuePath)))
            throw new InvalidDataException(
                "Please select Vigilante 8 (USA), title ID SLUS-00510, to add its soundtrack.");

        var tracks = V8LooseManifest.LoadEmbedded("V8LooseManifest.json").Tracks
            .Where(track => track.Number > 1).ToList();
        using var disc = CueBin.Open(cuePath);
        foreach (var track in tracks)
        {
            if (!disc.TryGetTrackStartLba(track.Number, out int start) ||
                !disc.TryReadAudioSector(start, out _, out _, out int end))
                throw new InvalidDataException(
                    $"The CUE is missing audio track {track.Number:00}. Keep all BIN files beside the CUE.");
            track.StartLba = start; track.EndLba = end;
        }
        EnsureFreeSpace(gameDataRoot, tracks);

        // Encode into a scratch folder first, so a cancelled or failed import
        // never leaves part of an album where the game would play it.
        string work = gameDataRoot + ".v8-soundtrack.partial";
        if (Directory.Exists(work)) Directory.Delete(work, true);
        Directory.CreateDirectory(work);
        for (int index = 0; index < tracks.Count; index++)
        {
            var track = tracks[index];
            cancellation.ThrowIfCancellationRequested();
            report?.Invoke(new("soundtrack", index, tracks.Count, track.Source!));
            V82LooseImporter.EncodeTrack(
                disc, track, work, $"Vigilante 8 Track {track.Number:00}", cancellation);
        }

        bool installed = V82LooseImporter.IsComplete(gameDataRoot);
        MoveMusic(work, installed ? gameDataRoot : StagingRoot(gameDataRoot));
        Directory.Delete(work, true);
        Console.WriteLine(
            $"[Soundtrack] imported Vigilante 8 album ({tracks.Count} tracks) " +
            (installed ? $"into {gameDataRoot}" : $"staged at {StagingRoot(gameDataRoot)}"));
        return installed;
    }

    /// <summary>
    /// Moves an album staged before Second Offense was installed into the
    /// import being committed. The album is optional, so a failure here is
    /// reported and never fails the game installation.
    /// </summary>
    public static void AdoptStaged(string gameDataRoot, string partialRoot)
    {
        string staged = StagingRoot(gameDataRoot);
        if (!Directory.Exists(Path.Combine(staged, "music"))) return;
        try
        {
            MoveMusic(staged, partialRoot);
            Directory.Delete(staged, true);
            Console.WriteLine($"[Soundtrack] adopted staged Vigilante 8 album from {staged}");
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            Console.Error.WriteLine($"[Soundtrack] could not adopt staged album {staged}: {ex.Message}");
        }
    }

    static void MoveMusic(string fromRoot, string toRoot)
    {
        string target = Path.Combine(toRoot, "music");
        Directory.CreateDirectory(target);
        foreach (string file in Directory.EnumerateFiles(Path.Combine(fromRoot, "music"), "*.ogg"))
            File.Move(file, Path.Combine(target, Path.GetFileName(file)), true);
    }

    static void EnsureFreeSpace(string gameDataRoot, IEnumerable<V8LooseTrack> tracks)
    {
        long rawAudioBytes = tracks.Sum(track => (long)(track.EndLba - track.StartLba) * 2352);
        long need = rawAudioBytes / 4 + FreeSpaceMargin;
        string? volume = Path.GetPathRoot(gameDataRoot);
        if (volume == null) return;
        long available = new DriveInfo(volume).AvailableFreeSpace;
        if (available < need)
            throw new IOException(
                $"The soundtrack needs about {need / (1024d * 1024d):0} MiB free; " +
                $"only {available / (1024d * 1024d):0} MiB is available on {volume}");
    }
}
