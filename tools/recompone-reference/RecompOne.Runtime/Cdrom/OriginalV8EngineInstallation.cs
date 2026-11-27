namespace RecompOne.Runtime.Cdrom;

// Disc-derived cache. Stage it beside game_data only while the sequel import
// is incomplete; the sequel importer needs to create its own game_data root.
// Never write to the installed mod roster or user settings.
internal static class OriginalV8EngineInstallation
{
    internal static string StagedDirectory(string gameDataRoot) =>
        Path.TrimEndingDirectorySeparator(Path.GetFullPath(gameDataRoot)) + ".v8-engine-audio";

    internal static bool TryImport(string cuePath, string gameDataRoot,
        CancellationToken cancellation, Action<string> diagnostic, bool sequelInstalled = false) =>
        TryImport(cuePath, gameDataRoot, OriginalV8EngineDonor.SupportedRevisions, cancellation, diagnostic, sequelInstalled);

    // The source-linked contract exercises the same publication path with a
    // synthetic revision list. Production always uses the pinned list above.
    internal static bool TryImport(string cuePath, string gameDataRoot,
        IReadOnlyList<OriginalV8EngineRevision> revisions, CancellationToken cancellation,
        Action<string> diagnostic, bool sequelInstalled = false)
    {
        try
        {
            OriginalV8EnginePack pack = OriginalV8EngineDonor.ExtractFromCue(cuePath, revisions, cancellation);
            string directory = sequelInstalled ? Path.GetFullPath(gameDataRoot) : StagedDirectory(gameDataRoot);
            cancellation.ThrowIfCancellationRequested();
            if (OperatingSystem.IsWindows())
                Directory.CreateDirectory(directory);
            else
                Directory.CreateDirectory(directory, UnixFileMode.UserRead |
                    UnixFileMode.UserWrite | UnixFileMode.UserExecute);
            RequireOrdinaryDirectory(directory);
            var result = OriginalV8EngineSidecar.WriteAtomic(directory, pack, revisions, cancellation: cancellation);
            diagnostic($"Original Vigilante 8 engine cache {result.ToString().ToLowerInvariant()} ({pack.Loops.Count} verified loops).");
            return true;
        }
        catch (Exception ex) when (ex is IOException or InvalidDataException or
            UnauthorizedAccessException or System.Text.DecoderFallbackException)
        {
            diagnostic($"Original Vigilante 8 engines were not added: {ex.Message}");
            return false;
        }
    }

    internal static OriginalV8EnginePack? TryRead(string? gameDataRoot, Action<string> diagnostic) =>
        TryRead(gameDataRoot, OriginalV8EngineDonor.SupportedRevisions, diagnostic);

    internal static OriginalV8EnginePack? TryRead(string? gameDataRoot,
        IReadOnlyList<OriginalV8EngineRevision> revisions, Action<string> diagnostic)
    {
        if (gameDataRoot == null)
            return null;
        return ReadCache(Path.GetFullPath(gameDataRoot), revisions, diagnostic)
            ?? ReadCache(StagedDirectory(gameDataRoot), revisions, diagnostic);
    }

    // Only call for a committed sequel installation. Publish with a no-overwrite
    // rename; on failure the legacy read path still works and the cache is intact.
    internal static void AdoptStaged(string gameDataRoot, Action<string> diagnostic) =>
        AdoptStaged(gameDataRoot, OriginalV8EngineDonor.SupportedRevisions, diagnostic);

    internal static void AdoptStaged(string gameDataRoot,
        IReadOnlyList<OriginalV8EngineRevision> revisions, Action<string> diagnostic)
    {
        string root = Path.GetFullPath(gameDataRoot);
        string staged = StagedDirectory(root);
        string source = Path.Combine(staged, OriginalV8EngineSidecar.FileName);
        string target = Path.Combine(root, OriginalV8EngineSidecar.FileName);
        if (!File.Exists(source) || File.Exists(target))
            return;
        try
        {
            RequireOrdinaryDirectory(root);
            RequireOrdinaryDirectory(staged);
            _ = OriginalV8EngineSidecar.Read(staged, revisions);
            File.Move(source, target, overwrite: false);
            diagnostic("Moved the original Vigilante 8 engine cache into game_data.");
            // Leave anything unrelated under the old directory untouched.
            if (!Directory.EnumerateFileSystemEntries(staged).Any())
                Directory.Delete(staged, recursive: false);
        }
        catch (Exception ex) when (ex is IOException or InvalidDataException or UnauthorizedAccessException)
        {
            diagnostic($"Could not clean up the staged original engine cache: {ex.Message}");
        }
    }

    static OriginalV8EnginePack? ReadCache(string directory,
        IReadOnlyList<OriginalV8EngineRevision> revisions, Action<string> diagnostic)
    {
        if (!Directory.Exists(directory))
            return null;
        try
        {
            RequireOrdinaryDirectory(directory);
            return OriginalV8EngineSidecar.Read(directory, revisions);
        }
        catch (FileNotFoundException) { return null; }
        catch (Exception ex) when (ex is IOException or InvalidDataException or UnauthorizedAccessException)
        {
            diagnostic($"Ignoring the original Vigilante 8 engine cache: {ex.Message}");
            return null;
        }
    }

    static void RequireOrdinaryDirectory(string path)
    {
        if ((File.GetAttributes(path) & (FileAttributes.Directory | FileAttributes.ReparsePoint)) !=
            FileAttributes.Directory)
            throw new IOException("The engine cache directory cannot be a link.");
    }
}
