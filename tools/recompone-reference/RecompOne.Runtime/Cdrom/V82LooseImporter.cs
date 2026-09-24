using System.Buffers.Binary;
using System.Text.RegularExpressions;
using OggVorbisEncoder;
using TextEncoding = System.Text.Encoding;

namespace RecompOne.Runtime.Cdrom;

public readonly record struct LooseImportProgress(
    string Phase, int Current, int Total, string Item);

public static class V82LooseImporter
{
    const int CookedSectorSize = 2048;
    const int StreamSectorSize = 2336;
    const int AudioSectorSize = 2352;
    const int AudioFramesPerSector = 588;
    const int AudioSampleRate = 44100;
    const int AudioChannels = 2;
    const int AudioSectorsPerChunk = 32;
    const int VorbisLookaheadFrames = 1024;
    const float VorbisQuality = 0.6f;
    const string CompletionMarker = ".recompone-import-complete";
    public static string DefaultRoot =>
        Path.Combine(global::RecompOne.Runtime.Runtime.ExecutableDirectory,
            "game_data");

    public static bool IsComplete(string root) =>
        File.Exists(Path.Combine(Path.GetFullPath(root), CompletionMarker)) &&
        File.Exists(Path.Combine(Path.GetFullPath(root), "SYSTEM.CNF")) &&
        File.Exists(Path.Combine(Path.GetFullPath(root), "SLUS_008.68"));

    public static void Import(
        string cuePath,
        string outputRoot,
        Action<LooseImportProgress>? report = null, CancellationToken cancellation = default)
    {
        cuePath = Path.GetFullPath(cuePath);
        outputRoot = Path.GetFullPath(outputRoot);
        string partialRoot = outputRoot + ".partial";
        if (IsComplete(outputRoot))
            return;
        if (Directory.Exists(outputRoot))
            throw new IOException(
                $"Incomplete import directory already exists: {outputRoot}");

        var manifest = V8LooseManifest.LoadEmbedded("V82LooseManifest.json");
        using var disc = CueBin.Open(cuePath);
        report?.Invoke(new("validating", 0, 1, "Checking Second Offense title ID"));
        using (var files = CueFs.Open(cuePath))
        {
            string system = TextEncoding.ASCII.GetString(files.ReadFile("SYSTEM.CNF"));
            if (!Regex.IsMatch(system, @"(?im)^\s*BOOT\s*=\s*cdrom:[\\/]*SLUS_008\.68;1\s*$"))
                throw new InvalidDataException("Please select Vigilante 8: 2nd Offense (USA), title ID SLUS-00868.");
            // Locate the selected disc's files, not a particular dump's LBAs or hashes.
            foreach (var file in manifest.Files)
            {
                if (!files.Locate(file.Path, out int lba, out uint size))
                    throw new InvalidDataException($"The disc is missing required file {file.Path}.");
                file.Lba = lba; file.Size = size;
            }
        }
        foreach (var track in manifest.Tracks.Where(t => t.Number > 1))
        {
            if (!disc.TryGetTrackStartLba(track.Number, out int start) ||
                !disc.TryReadAudioSector(start, out _, out _, out int end))
                throw new InvalidDataException($"The CUE is missing audio track {track.Number:00}. Keep all BIN files beside the CUE.");
            track.StartLba = start; track.EndLba = end;
        }
        EnsureFreeSpace(outputRoot, manifest);
        Directory.CreateDirectory(partialRoot);

        int total = manifest.Files.Count +
            manifest.Tracks.Count(track => track.Number > 1);
        int current = 0;
        foreach (var file in manifest.Files)
        {
            cancellation.ThrowIfCancellationRequested();
            report?.Invoke(new("files", current, total, file.Path));
            ExtractFile(disc, file, partialRoot, cancellation);
            current++;
        }

        foreach (var track in manifest.Tracks.Where(track => track.Number > 1))
        {
            if (string.IsNullOrWhiteSpace(track.Source))
                throw new InvalidDataException(
                    $"Track {track.Number:00} has no loose audio target");
            cancellation.ThrowIfCancellationRequested();
            report?.Invoke(new("music", current, total, track.Source));
            EncodeTrack(disc, track, partialRoot, cancellation);
            current++;
        }

        File.WriteAllText(
            Path.Combine(partialRoot, CompletionMarker),
            $"Vigilante 8: 2nd Offense\nsource={Path.GetFileName(cuePath)}\n" +
            $"completed={DateTimeOffset.Now:o}\n",
            TextEncoding.UTF8);
        CommitImport(partialRoot, outputRoot);
        report?.Invoke(new("complete", total, total, outputRoot));
    }

    static void CommitImport(string partialRoot, string outputRoot)
    {
        const int attempts = 30;
        Exception? lastError = null;
        for (int attempt = 1; attempt <= attempts; attempt++)
        {
            try
            {
                Directory.Move(partialRoot, outputRoot);
                return;
            }
            catch (Exception exception)
                when (exception is IOException or UnauthorizedAccessException)
            {
                if (Directory.Exists(outputRoot))
                    throw new IOException(
                        $"Import destination appeared during finalization: " +
                        outputRoot, exception);
                lastError = exception;
                if (attempt < attempts)
                    Thread.Sleep(1000);
            }
        }

        throw new IOException(
            $"Could not finalize disc import after {attempts} seconds: " +
            $"{partialRoot}", lastError);
    }

    static void EnsureFreeSpace(string outputRoot, V8LooseManifest manifest)
    {
        long fileBytes = manifest.Files.Sum(file =>
            IsRawStream(file.Path)
                ? ((file.Size + CookedSectorSize - 1) / CookedSectorSize) *
                  StreamSectorSize
                : file.Size);
        long rawAudioBytes = manifest.Tracks
            .Where(track => track.Number > 1)
            .Sum(track => (long)(track.EndLba - track.StartLba) *
                          AudioSectorSize);
        long conservativeNeed = fileBytes + rawAudioBytes + 256L * 1024 * 1024;
        string? volume = Path.GetPathRoot(outputRoot);
        if (volume == null) return;
        long available = new DriveInfo(volume).AvailableFreeSpace;
        if (available < conservativeNeed)
            throw new IOException(
                $"Disc import needs up to {FormatBytes(conservativeNeed)} free; " +
                $"only {FormatBytes(available)} is available on {volume}");
    }

    static void ExtractFile(CueBin disc, V8LooseFile file, string root, CancellationToken cancellation)
    {
        string target = ResolveTarget(root, file.Path);
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        string temporary = target + ".recompone-tmp";
        using (var output = new FileStream(
                   temporary, FileMode.Create, FileAccess.Write, FileShare.None))
        {
            if (IsRawStream(file.Path))
            {
                int sectors = checked((int)(
                    (file.Size + CookedSectorSize - 1) / CookedSectorSize));
                for (int index = 0; index < sectors; index++)
                {
                    cancellation.ThrowIfCancellationRequested();
                    output.Write(disc.ReadSectorData(file.Lba + index, StreamSectorSize));
                }
            }
            else
            {
                uint remaining = file.Size;
                int lba = file.Lba;
                while (remaining > 0)
                {
                    cancellation.ThrowIfCancellationRequested();
                    byte[] sector = disc.ReadSector(lba++);
                    int count = checked((int)Math.Min(
                        remaining, (uint)sector.Length));
                    output.Write(sector, 0, count);
                    remaining -= (uint)count;
                }
            }
        }
        File.Move(temporary, target, true);
    }

    static byte[] ReadLogicalFile(CueBin disc, V8LooseFile file)
    {
        using var output = new MemoryStream(checked((int)file.Size));
        uint remaining = file.Size;
        int lba = file.Lba;
        while (remaining > 0)
        {
            byte[] sector = disc.ReadSector(lba++);
            int count = checked((int)Math.Min(remaining, (uint)sector.Length));
            output.Write(sector, 0, count);
            remaining -= (uint)count;
        }
        return output.ToArray();
    }

    static void EncodeTrack(CueBin disc, V8LooseTrack track, string root, CancellationToken cancellation)
    {
        string target = ResolveTarget(root, track.Source);
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        string temporary = target + ".recompone-tmp";

        using (var output = new FileStream(
                   temporary, FileMode.Create, FileAccess.Write, FileShare.None))
        {
            var info = VorbisInfo.InitVariableBitRate(
                AudioChannels, AudioSampleRate, VorbisQuality);
            var ogg = new OggStream(unchecked(0x56383200 + track.Number));
            var comments = new Comments();
            comments.AddTag("TITLE", $"V8:2 Track {track.Number:00}");
            comments.AddTag("ENCODER", "Vigilante82PC loose-disc importer");
            ogg.PacketIn(HeaderPacketBuilder.BuildInfoPacket(info));
            ogg.PacketIn(HeaderPacketBuilder.BuildCommentsPacket(comments));
            ogg.PacketIn(HeaderPacketBuilder.BuildBooksPacket(info));
            FlushPages(ogg, output, true);

            var state = ProcessingState.Create(info);
            int capacity = AudioSectorsPerChunk * AudioFramesPerSector;
            float[][] samples = [new float[capacity], new float[capacity]];
            for (int lba = track.StartLba; lba < track.EndLba;)
            {
                cancellation.ThrowIfCancellationRequested();
                int sectors = Math.Min(
                    AudioSectorsPerChunk, track.EndLba - lba);
                int frame = 0;
                for (int sectorIndex = 0; sectorIndex < sectors; sectorIndex++)
                {
                    if (!disc.TryReadAudioSector(
                            lba++, out byte[] sector,
                            out int trackNumber, out _) ||
                        trackNumber != track.Number)
                        throw new InvalidDataException(
                            $"Missing CD audio sector in track {track.Number:00}");
                    for (int offset = 0;
                         offset < AudioSectorSize;
                         offset += 4, frame++)
                    {
                        samples[0][frame] =
                            BinaryPrimitives.ReadInt16LittleEndian(
                                sector.AsSpan(offset, 2)) / 32768f;
                        samples[1][frame] =
                            BinaryPrimitives.ReadInt16LittleEndian(
                                sector.AsSpan(offset + 2, 2)) / 32768f;
                    }
                }
                state.WriteData(samples, frame, 0);
                DrainPackets(state, ogg, output);
            }

            float[][] lookahead =
                [new float[VorbisLookaheadFrames],
                 new float[VorbisLookaheadFrames]];
            state.WriteData(lookahead, VorbisLookaheadFrames, 0);
            DrainPackets(state, ogg, output);
            state.WriteEndOfStream();
            DrainPackets(state, ogg, output);
            FlushPages(ogg, output, true);
        }
        File.Move(temporary, target, true);
    }

    static void DrainPackets(
        ProcessingState state, OggStream ogg, Stream output)
    {
        while (!ogg.Finished && state.PacketOut(out OggPacket packet))
        {
            ogg.PacketIn(packet);
            FlushPages(ogg, output, false);
        }
    }

    static void FlushPages(OggStream ogg, Stream output, bool force)
    {
        while (ogg.PageOut(out OggPage page, force))
        {
            output.Write(page.Header, 0, page.Header.Length);
            output.Write(page.Body, 0, page.Body.Length);
        }
    }

    static bool IsRawStream(string path)
    {
        string extension = Path.GetExtension(path);
        return extension.Equals(".STR", StringComparison.OrdinalIgnoreCase) ||
               extension.Equals(".XA", StringComparison.OrdinalIgnoreCase);
    }

    static string ResolveTarget(string root, string relative)
    {
        root = Path.GetFullPath(root).TrimEnd(
            Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
        string target = Path.GetFullPath(Path.Combine(
            root, relative.Replace('/', Path.DirectorySeparatorChar)));
        string prefix = root + Path.DirectorySeparatorChar;
        if (!target.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException(
                $"Disc path escapes the import root: {relative}");
        return target;
    }

    static string FormatBytes(long bytes) =>
        $"{bytes / (1024d * 1024d * 1024d):0.00} GiB";
}
