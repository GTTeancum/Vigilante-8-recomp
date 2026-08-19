using System.Buffers.Binary;
using System.Security.Cryptography;
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
    static readonly IReadOnlyDictionary<string, string> SupportedFileHashes =
        new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["SLUS_008.68"] =
                "9ECFD0A7986CEE816EF79284A6635EAADBA435183362F8B796F667B8FB5A3EB6",
            ["SHELL/SHELL.DLL"] =
                "FAD8848CADEE00C519AE043A344B74AD9253F6272795B822334AEEC9A360C995",
            ["SHELL/POSES.TBL"] =
                "976148F86AEF4880F3C884DE15EB1A5CC11C532CE766001EF2A848D69696FFF2",
            ["QUEST.BIN"] =
                "4414144FA777111308C499E62B13BC37575D33B7365ABD598080BEC5EB56E898",
        };

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
        Action<LooseImportProgress>? report = null)
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
        ValidateDisc(disc, manifest);
        EnsureFreeSpace(outputRoot, manifest);
        Directory.CreateDirectory(partialRoot);

        int total = manifest.Files.Count +
            manifest.Tracks.Count(track => track.Number > 1);
        int current = 0;
        foreach (var file in manifest.Files)
        {
            report?.Invoke(new(
                "files", ++current, total, file.Path));
            ExtractFile(disc, file, partialRoot);
        }

        foreach (var track in manifest.Tracks.Where(track => track.Number > 1))
        {
            if (string.IsNullOrWhiteSpace(track.Source))
                throw new InvalidDataException(
                    $"Track {track.Number:00} has no loose audio target");
            report?.Invoke(new(
                "music", ++current, total, track.Source));
            EncodeTrack(disc, track, partialRoot);
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

    static void ValidateDisc(CueBin disc, V8LooseManifest manifest)
    {
        if (disc.FirstTrackNumber != 1 ||
            disc.LastTrackNumber != manifest.Tracks.Max(track => track.Number))
            throw new InvalidDataException(
                "The selected CUE does not contain the expected V8:2 track layout");

        foreach (var track in manifest.Tracks)
        {
            if (!disc.TryGetTrackStartLba(track.Number, out int start) ||
                start != track.StartLba)
                throw new InvalidDataException(
                    $"Track {track.Number:00} starts at LBA {start}; " +
                    $"expected {track.StartLba}");
        }

        var system = manifest.Files.Single(file =>
            file.Path.Equals("SYSTEM.CNF", StringComparison.OrdinalIgnoreCase));
        string text = TextEncoding.ASCII.GetString(ReadLogicalFile(disc, system));
        if (!text.Contains("SLUS_008.68", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException(
                "The selected disc is not Vigilante 8: 2nd Offense (USA)");

        foreach (var (path, expected) in SupportedFileHashes)
        {
            var file = manifest.Files.Single(candidate =>
                candidate.Path.Equals(path, StringComparison.OrdinalIgnoreCase));
            string actual = Convert.ToHexString(
                SHA256.HashData(ReadLogicalFile(disc, file)));
            if (!actual.Equals(expected, StringComparison.OrdinalIgnoreCase))
                throw new InvalidDataException(
                    "Unsupported or modified Vigilante 8: 2nd Offense disc: " +
                    $"{path} SHA-256 is {actual}, expected {expected}");
        }
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

    static void ExtractFile(CueBin disc, V8LooseFile file, string root)
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
                    output.Write(disc.ReadSectorData(
                        file.Lba + index, StreamSectorSize));
            }
            else
            {
                uint remaining = file.Size;
                int lba = file.Lba;
                while (remaining > 0)
                {
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

    static void EncodeTrack(CueBin disc, V8LooseTrack track, string root)
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
