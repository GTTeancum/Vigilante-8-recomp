namespace RecompOne.Runtime.Cdrom;

// Optional, local to the installation. Kept beside game_data so it remains
// available when the first disc is provided before the sequel is installed.
// This never writes an installed roster, config, or game-data file.
internal static class OriginalV8EngineInstallation
{
    internal static string CacheDirectory(string gameDataRoot) =>
        Path.TrimEndingDirectorySeparator(Path.GetFullPath(gameDataRoot)) + ".v8-engine-audio";

    internal static bool TryImport(string cuePath, string gameDataRoot,
        CancellationToken cancellation, Action<string> diagnostic) =>
        TryImport(cuePath, gameDataRoot, OriginalV8EngineDonor.SupportedRevisions, cancellation, diagnostic);

    // The source-linked contract exercises the same publication path with a
    // synthetic revision list. Production always uses the pinned list above.
    internal static bool TryImport(string cuePath, string gameDataRoot,
        IReadOnlyList<OriginalV8EngineRevision> revisions, CancellationToken cancellation,
        Action<string> diagnostic)
    {
        try
        {
            OriginalV8EnginePack pack = OriginalV8EngineDonor.ExtractFromCue(cuePath, revisions, cancellation);
            string directory = CacheDirectory(gameDataRoot);
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
        string directory = CacheDirectory(gameDataRoot);
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
