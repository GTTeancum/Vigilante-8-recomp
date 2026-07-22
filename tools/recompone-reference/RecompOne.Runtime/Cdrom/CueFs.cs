namespace RecompOne.Runtime.Cdrom;

public sealed class CueFs : IDisposable
{
    private record Entry(int Lba, uint Size, bool IsDir, string Name);
    private enum LooseStorage { Cooked2048, Raw2336 }
    private record LooseEntry(
        int Lba,
        uint DiscSize,
        string DiscPath,
        string HostPath,
        long HostSize,
        uint LogicalSize,
        LooseStorage Storage);

    private readonly CueBin? _bin;
    private readonly V8LooseManifest? _manifest;
    private readonly Dictionary<string, V8LooseFile> _manifestFiles =
        new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, string> _looseFiles =
        new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<int, LooseEntry> _looseByStartLba = [];
    private readonly LooseEntry[] _looseByLba;
    private readonly Dictionary<string, FileStream> _looseStreams =
        new(StringComparer.OrdinalIgnoreCase);
    private readonly object _looseIoGate = new();
    private readonly LooseCdda? _looseCdda;

    private CueFs(CueBin bin, string? looseRoot)
    {
        _bin = bin;
        IndexLooseFiles(looseRoot);
        _looseByLba = IndexCueOverrides();
        if (_looseByLba.Length > 0)
            Console.WriteLine(
                $"[CD] CUE overrides={_looseByLba.Length} " +
                $"root={Path.GetFullPath(looseRoot!)}");
    }

    private CueFs(string looseRoot)
    {
        string root = Path.GetFullPath(looseRoot);
        if (!Directory.Exists(root))
            throw new DirectoryNotFoundException($"Loose asset root not found: {root}");
        _manifest = V8LooseManifest.Load();
        foreach (var file in _manifest.Files)
            _manifestFiles[NormalizeDiscPath(file.Path)] = file;
        IndexLooseFiles(root);
        _looseByLba = IndexStandaloneFiles();
        _looseCdda = new LooseCdda(root, _manifest.Tracks);
        Console.WriteLine(
            $"[CD] standalone loose files={_looseByLba.Length} " +
            $"volume={_manifest.Volume} root={root}");
    }

    public static CueFs Open(string cuePath) => new(CueBin.Open(cuePath), null);

    public static CueFs Open(string cuePath, string? looseRoot) =>
        new(CueBin.Open(cuePath), looseRoot);

    public static CueFs OpenLoose(string looseRoot) => new(looseRoot);

    public int LooseOverrideCount => _looseByLba.Length;
    public bool IsStandaloneLoose => _manifest != null;

    public byte[] ReadFile(string path)
    {
        var located = LocateEntryWithPath(path) ??
            throw new FileNotFoundException($"File not found: {path}");
        if (TryGetLoose(located.Entry.Lba, out var loose))
            return ReadLooseLogicalRange(loose, 0, checked((int)loose.LogicalSize));
        if (_bin != null)
            return ReadExtent(located.Entry.Lba, checked((int)located.Entry.Size));
        throw MissingLooseAsset(located.Path);
    }

    private static string StripVersion(string name)
    {
        int semi = name.IndexOf(';');
        return semi >= 0 ? name[..semi] : name;
    }

    public bool Exists(string path)
    {
        try { ReadFile(path); return true; }
        catch { return false; }
    }

    public string? FindFile(string name)
    {
        if (_manifest != null)
            return FindUniqueManifestPath(name)?.Path;
        return Search(Root(), "", name.ToUpperInvariant());
    }

    public bool Locate(string name, out int lba, out uint size)
    {
        lba = 0;
        size = 0;
        var located = LocateEntryWithPath(name);
        if (located == null) return false;
        lba = located.Value.Entry.Lba;
        size = TryGetLoose(lba, out var loose)
            ? loose.LogicalSize
            : located.Value.Entry.Size;
        return true;
    }

    public bool TryDescribeLba(int lba, out string path, out int endLba)
    {
        if (_manifest != null)
        {
            foreach (var file in _manifest.Files.OrderBy(candidate => candidate.Lba))
            {
                uint size = TryGetLoose(file.Lba, out var loose)
                    ? loose.LogicalSize
                    : file.Size;
                int end = file.Lba + checked((int)((size + 2047u) >> 11));
                if (lba < file.Lba || lba >= end) continue;
                path = file.Path;
                endLba = end;
                return true;
            }
            path = $"LBA {lba}";
            endLba = int.MaxValue;
            return false;
        }

        var found = FindByLba(Root(), "", lba);
        if (found != null)
        {
            path = found.Value.Path;
            endLba = found.Value.Entry.Lba +
                checked((int)((found.Value.Entry.Size + 2047u) >> 11));
            return true;
        }

        path = $"LBA {lba}";
        endLba = int.MaxValue;
        return false;
    }

    private (Entry Entry, string Path)? FindByLba(Entry dir, string basePath, int lba)
    {
        foreach (var entry in Entries(dir))
        {
            string path = basePath.Length > 0 ? basePath + "/" + entry.Name : entry.Name;
            if (entry.IsDir)
            {
                var found = FindByLba(entry, path, lba);
                if (found != null) return found;
                continue;
            }

            int endLba = entry.Lba + checked((int)((entry.Size + 2047u) >> 11));
            if (lba >= entry.Lba && lba < endLba)
                return (entry, path);
        }
        return null;
    }

    private (Entry Entry, string Path)? LocateEntryWithPath(string name)
    {
        name = NormalizeDiscPath(name);
        if (_manifest != null)
        {
            if (_manifestFiles.TryGetValue(name, out var exact))
                return (new Entry(exact.Lba, exact.Size, false, Path.GetFileName(exact.Path)), exact.Path);
            var unique = FindUniqueManifestPath(name);
            return unique == null
                ? null
                : (new Entry(unique.Lba, unique.Size, false, Path.GetFileName(unique.Path)), unique.Path);
        }

        try
        {
            var parts = name.Split('/');
            var dir = Root();
            string basePath = "";
            for (int i = 0; i < parts.Length - 1; i++)
            {
                dir = Find(dir, parts[i], true);
                basePath = basePath.Length == 0 ? dir.Name : $"{basePath}/{dir.Name}";
            }
            var file = Find(dir, parts[^1], false);
            return (file, basePath.Length == 0 ? file.Name : $"{basePath}/{file.Name}");
        }
        catch (FileNotFoundException) { }

        int slash = name.LastIndexOf('/');
        string basename = slash >= 0 ? name[(slash + 1)..] : name;
        return SearchEntry(Root(), "", basename);
    }

    private V8LooseFile? FindUniqueManifestPath(string name)
    {
        string basename = name[(name.LastIndexOf('/') + 1)..];
        V8LooseFile? match = null;
        foreach (var file in _manifest!.Files)
        {
            string candidate = NormalizeDiscPath(file.Path);
            string candidateBase = candidate[(candidate.LastIndexOf('/') + 1)..];
            if (!candidateBase.Equals(basename, StringComparison.OrdinalIgnoreCase)) continue;
            if (match != null) return null;
            match = file;
        }
        return match;
    }

    private (Entry Entry, string Path)? SearchEntry(Entry dir, string basePath, string name)
    {
        foreach (var entry in Entries(dir))
        {
            string path = basePath.Length == 0 ? entry.Name : $"{basePath}/{entry.Name}";
            if (entry.IsDir)
            {
                var found = SearchEntry(entry, path, name);
                if (found != null) return found;
            }
            else if (entry.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                return (entry, path);
        }
        return null;
    }

    public byte[] ReadSector(int lba) => ReadSectorData(lba, 2048);

    public byte[] ReadSectorData(int lba, int size)
    {
        if (TryReadLooseSector(lba, size, out var loose))
            return loose;
        if (_manifest?.MetadataSectors.TryGetValue(lba, out string? encoded) == true)
            return ExpandCookedSector(Convert.FromBase64String(encoded), size);
        if (_bin != null)
            return _bin.ReadSectorData(lba, size);
        throw new IOException(
            $"Standalone loose mode has no asset mapped to LBA {lba}; BIN/CUE fallback is disabled");
    }

    public bool TryReadLooseFileRange(int startLba, uint offset, int count, out byte[] data)
    {
        data = [];
        if (!TryGetLoose(startLba, out var loose)) return false;
        data = ReadLooseLogicalRange(loose, offset, count);
        return true;
    }

    public int FirstTrackNumber => _manifest?.Tracks.Min(track => track.Number) ??
        _bin!.FirstTrackNumber;
    public int LastTrackNumber => _manifest?.Tracks.Max(track => track.Number) ??
        _bin!.LastTrackNumber;
    public int LeadOutLba => _manifest?.LeadOutLba ?? _bin!.LeadOutLba;

    public bool TryGetTrackStartLba(int trackNumber, out int lba)
    {
        if (_manifest == null)
            return _bin!.TryGetTrackStartLba(trackNumber, out lba);
        var track = _manifest.Tracks.FirstOrDefault(candidate => candidate.Number == trackNumber);
        lba = track?.StartLba ?? 0;
        return track != null;
    }

    public bool TryReadAudioSector(
        int lba,
        out byte[] sector,
        out int trackNumber,
        out int trackEndLba)
    {
        if (_looseCdda != null)
            return _looseCdda.TryReadSector(
                lba, out sector, out trackNumber, out trackEndLba);
        return _bin!.TryReadAudioSector(
            lba, out sector, out trackNumber, out trackEndLba);
    }

    public byte[] ReadSectors(int lba, int size) => ReadExtent(lba, size);

    private string? Search(Entry dir, string basePath, string name)
    {
        foreach (var entry in Entries(dir))
        {
            if (entry.IsDir)
            {
                string path = basePath.Length > 0 ? basePath + "/" + entry.Name : entry.Name;
                var found = Search(entry, path, name);
                if (found != null) return found;
            }
            else if (entry.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                return basePath.Length > 0 ? basePath + "/" + entry.Name : entry.Name;
        }
        return null;
    }

    private Entry Root()
    {
        if (_bin == null)
            throw new InvalidOperationException("ISO directory records are unavailable in loose mode");
        var pvd = _bin.ReadSector(16);
        return ParseEntry(pvd, 156);
    }

    private Entry Find(Entry dir, string name, bool wantDir)
    {
        string upper = name.ToUpperInvariant();
        foreach (var entry in Entries(dir))
            if (entry.IsDir == wantDir &&
                entry.Name.Equals(upper, StringComparison.OrdinalIgnoreCase))
                return entry;
        throw new FileNotFoundException($"{(wantDir ? "directory" : "File")} not found: {name}");
    }

    private IEnumerable<Entry> Entries(Entry dir)
    {
        var data = ReadExtent(dir.Lba, checked((int)dir.Size));
        int offset = 0;
        while (offset < data.Length)
        {
            byte length = data[offset];
            if (length == 0)
            {
                offset = (offset / 2048 + 1) * 2048;
                continue;
            }
            var entry = ParseEntry(data, offset);
            if (entry.Name is not ("\x00" or "\x01"))
                yield return entry;
            offset += length;
        }
    }

    private byte[] ReadExtent(int lba, int size)
    {
        var result = new byte[size];
        int done = 0;
        int current = lba;
        while (done < size)
        {
            var sector = ReadSectorData(current++, 2048);
            int count = Math.Min(2048, size - done);
            sector.AsSpan(0, count).CopyTo(result.AsSpan(done));
            done += count;
        }
        return result;
    }

    private static Entry ParseEntry(byte[] data, int offset)
    {
        int lba = BitConverter.ToInt32(data, offset + 2);
        uint size = BitConverter.ToUInt32(data, offset + 10);
        bool isDir = (data[offset + 25] & 0x02) != 0;
        int nameLength = data[offset + 32];
        string raw = System.Text.Encoding.ASCII.GetString(data, offset + 33, nameLength);
        int semi = raw.IndexOf(';');
        return new Entry(lba, size, isDir, semi >= 0 ? raw[..semi] : raw);
    }

    public void Dispose()
    {
        _looseCdda?.Dispose();
        _bin?.Dispose();
        lock (_looseIoGate)
        {
            foreach (var stream in _looseStreams.Values) stream.Dispose();
            _looseStreams.Clear();
        }
    }

    private void IndexLooseFiles(string? looseRoot)
    {
        if (string.IsNullOrWhiteSpace(looseRoot) || !Directory.Exists(looseRoot)) return;
        string root = Path.GetFullPath(looseRoot);
        foreach (string file in Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories))
        {
            string relative = NormalizeDiscPath(Path.GetRelativePath(root, file));
            if (relative.Length > 0) _looseFiles[relative] = Path.GetFullPath(file);
        }
    }

    private LooseEntry[] IndexCueOverrides()
    {
        if (_looseFiles.Count == 0) return [];
        var found = new List<LooseEntry>();
        IndexCueOverrides(Root(), "", found);
        foreach (var entry in found) _looseByStartLba[entry.Lba] = entry;
        return found.OrderBy(entry => entry.Lba).ToArray();
    }

    private void IndexCueOverrides(Entry dir, string basePath, List<LooseEntry> found)
    {
        foreach (var entry in Entries(dir))
        {
            string path = basePath.Length == 0 ? entry.Name : $"{basePath}/{entry.Name}";
            if (entry.IsDir)
            {
                IndexCueOverrides(entry, path, found);
                continue;
            }
            if (!_looseFiles.TryGetValue(NormalizeDiscPath(path), out string? hostPath)) continue;
            found.Add(CreateLooseEntry(entry.Lba, entry.Size, path, hostPath));
        }
    }

    private LooseEntry[] IndexStandaloneFiles()
    {
        var found = new List<LooseEntry>();
        var missing = new List<string>();
        var malformedStreams = new List<string>();
        foreach (var file in _manifest!.Files)
        {
            string discPath = NormalizeDiscPath(file.Path);
            if (discPath.StartsWith("REDBOOK/", StringComparison.OrdinalIgnoreCase))
                continue;
            if (!_looseFiles.TryGetValue(discPath, out string? hostPath))
            {
                missing.Add(file.Path);
                continue;
            }
            var entry = CreateLooseEntry(file.Lba, file.Size, file.Path, hostPath);
            if (IsStreamPath(file.Path) && entry.Storage != LooseStorage.Raw2336)
            {
                malformedStreams.Add(file.Path);
                continue;
            }
            found.Add(entry);
        }

        if (missing.Count > 0)
            throw new FileNotFoundException(
                "Standalone loose install is missing required assets (no BIN/CUE fallback): " +
                string.Join(", ", missing));
        if (malformedStreams.Count > 0)
            throw new InvalidDataException(
                "Loose STR/XA files must contain 2336-byte Mode 2 sectors: " +
                string.Join(", ", malformedStreams));

        foreach (var entry in found) _looseByStartLba[entry.Lba] = entry;
        return found.OrderBy(entry => entry.Lba).ToArray();
    }

    private static LooseEntry CreateLooseEntry(
        int lba, uint discSize, string discPath, string hostPath)
    {
        long hostSize = new FileInfo(hostPath).Length;
        LooseStorage storage = IsStreamPath(discPath) && hostSize > 0 &&
            hostSize % 2336 == 0
                ? LooseStorage.Raw2336
                : LooseStorage.Cooked2048;
        uint logicalSize = storage == LooseStorage.Raw2336
            ? checked((uint)(hostSize / 2336 * 2048))
            : checked((uint)hostSize);
        return new LooseEntry(
            lba, discSize, discPath, hostPath, hostSize, logicalSize, storage);
    }

    private static bool IsStreamPath(string path)
    {
        string extension = Path.GetExtension(path);
        return extension.Equals(".STR", StringComparison.OrdinalIgnoreCase) ||
            extension.Equals(".XA", StringComparison.OrdinalIgnoreCase);
    }

    private bool TryGetLoose(int startLba, out LooseEntry entry) =>
        _looseByStartLba.TryGetValue(startLba, out entry!);

    private bool TryReadLooseSector(int lba, int size, out byte[] data)
    {
        foreach (var entry in _looseByLba)
        {
            int sectors = checked((int)((entry.LogicalSize + 2047u) >> 11));
            if (lba < entry.Lba || lba >= entry.Lba + sectors) continue;
            int sectorIndex = lba - entry.Lba;
            data = ReadLooseSector(entry, sectorIndex, size);
            return true;
        }
        data = [];
        return false;
    }

    private byte[] ReadLooseSector(LooseEntry entry, int sectorIndex, int size)
    {
        if (entry.Storage == LooseStorage.Raw2336)
        {
            byte[] raw = ReadHostRange(entry, (long)sectorIndex * 2336, 2336);
            return size switch
            {
                2048 => raw.AsSpan(8, 2048).ToArray(),
                2328 => raw.AsSpan(8, 2328).ToArray(),
                2336 => raw,
                2340 => Prefix(raw, 4),
                2352 => Prefix(raw, 16),
                _ => throw new NotSupportedException(
                    $"Loose raw-sector size {size} is not supported"),
            };
        }

        byte[] cooked = ReadLooseLogicalRange(entry, checked((uint)(sectorIndex * 2048)), 2048);
        return size switch
        {
            2048 => cooked,
            2328 => Suffix(cooked, 280),
            2336 => PrefixAndSuffix(cooked, 8, 280),
            2340 => PrefixAndSuffix(cooked, 12, 280),
            2352 => PrefixAndSuffix(cooked, 24, 280),
            _ => throw new NotSupportedException(
                $"Loose cooked-sector size {size} is not supported"),
        };
    }

    private static byte[] Prefix(byte[] source, int count)
    {
        var result = new byte[count + source.Length];
        source.CopyTo(result, count);
        return result;
    }

    private static byte[] Suffix(byte[] source, int count)
    {
        var result = new byte[source.Length + count];
        source.CopyTo(result, 0);
        return result;
    }

    private static byte[] PrefixAndSuffix(byte[] source, int prefix, int suffix)
    {
        var result = new byte[prefix + source.Length + suffix];
        source.CopyTo(result, prefix);
        return result;
    }

    private static byte[] ExpandCookedSector(byte[] cooked, int size) => size switch
    {
        2048 => cooked,
        2328 => Suffix(cooked, 280),
        2336 => PrefixAndSuffix(cooked, 8, 280),
        2340 => PrefixAndSuffix(cooked, 12, 280),
        2352 => PrefixAndSuffix(cooked, 24, 280),
        _ => throw new NotSupportedException(
            $"Loose metadata-sector size {size} is not supported"),
    };

    private byte[] ReadLooseLogicalRange(LooseEntry entry, uint offset, int count)
    {
        var data = new byte[count];
        if (offset >= entry.LogicalSize || count == 0) return data;
        int available = Math.Min(count, checked((int)(entry.LogicalSize - offset)));
        if (entry.Storage == LooseStorage.Cooked2048)
            return ReadHostRange(entry, offset, count);

        int done = 0;
        uint logicalOffset = offset;
        while (done < available)
        {
            int sector = checked((int)(logicalOffset >> 11));
            int inSector = checked((int)(logicalOffset & 0x7FF));
            int take = Math.Min(available - done, 2048 - inSector);
            byte[] part = ReadHostRange(
                entry, (long)sector * 2336 + 8 + inSector, take);
            part.CopyTo(data, done);
            done += take;
            logicalOffset += checked((uint)take);
        }
        return data;
    }

    private byte[] ReadHostRange(LooseEntry entry, long offset, int count)
    {
        var data = new byte[count];
        lock (_looseIoGate)
        {
            if (!_looseStreams.TryGetValue(entry.HostPath, out var stream))
                _looseStreams[entry.HostPath] = stream = File.OpenRead(entry.HostPath);
            if (offset >= stream.Length) return data;
            stream.Position = offset;
            int available = checked((int)Math.Min(count, stream.Length - offset));
            stream.ReadExactly(data, 0, available);
        }
        return data;
    }

    private static FileNotFoundException MissingLooseAsset(string path) => new(
        $"Standalone loose asset is missing: {path}. BIN/CUE fallback is disabled.", path);

    private static string NormalizeDiscPath(string path)
    {
        var parts = path.Trim().TrimStart('/', '\\')
            .Split(['/', '\\'], StringSplitOptions.RemoveEmptyEntries)
            .Select(StripVersion)
            .ToArray();
        if (parts.Any(part => part is "." or ".."))
            throw new InvalidDataException($"Invalid loose-file path: {path}");
        return string.Join('/', parts);
    }
}
