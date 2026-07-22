namespace RecompOne.Runtime.Cdrom;

public sealed class CueFs : IDisposable
{
    private record Entry(int Lba, uint Size, bool IsDir, string Name);
    private record LooseEntry(int Lba, uint DiscSize, string DiscPath, string HostPath, long HostSize);

    private readonly CueBin _bin;
    private readonly Dictionary<string, string> _looseFiles =
        new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<int, LooseEntry> _looseByStartLba = [];
    private readonly LooseEntry[] _looseByLba;

    private CueFs(CueBin bin, string? looseRoot)
    {
        _bin = bin;
        IndexLooseFiles(looseRoot);
        _looseByLba = IndexLooseDiscEntries();
        if (_looseByLba.Length > 0)
            Console.WriteLine(
                $"[CD] loose-file overrides={_looseByLba.Length} root={Path.GetFullPath(looseRoot!)}");
    }

    public static CueFs Open(string cuePath) => new(CueBin.Open(cuePath), null);

    public static CueFs Open(string cuePath, string? looseRoot) =>
        new(CueBin.Open(cuePath), looseRoot);

    public int LooseOverrideCount => _looseByLba.Length;

    public byte[] ReadFile(string path)
    {
        var (file, discPath) = LocateEntryWithPath(path) ??
            throw new FileNotFoundException($"File not found: {path}");
        if (TryGetLoose(file.Lba, out var loose))
            return File.ReadAllBytes(loose.HostPath);
        return ReadExtent(file.Lba, (int)file.Size);
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

    public string? FindFile(string name) => Search(Root(), "", name.ToUpperInvariant());

    public bool Locate(string name, out int lba, out uint size)
    {
        lba = 0;
        size = 0;
        var located = LocateEntryWithPath(name);
        if (located == null) return false;
        lba = located.Value.Entry.Lba;
        size = TryGetLoose(lba, out var loose)
            ? checked((uint)loose.HostSize)
            : located.Value.Entry.Size;
        return true;
    }

    public bool TryDescribeLba(int lba, out string path, out int endLba)
    {
        var found = FindByLba(Root(), "", lba);
        if (found != null)
        {
            path = found.Value.Path;
            endLba = found.Value.Entry.Lba +
                (int)((found.Value.Entry.Size + 2047u) >> 11);
            return true;
        }

        path = $"LBA {lba}";
        endLba = int.MaxValue;
        return false;
    }

    private (Entry Entry, string Path)? FindByLba(Entry dir, string basePath, int lba)
    {
        foreach (var e in Entries(dir))
        {
            string path = basePath.Length > 0 ? basePath + "/" + e.Name : e.Name;
            if (e.IsDir)
            {
                var found = FindByLba(e, path, lba);
                if (found != null) return found;
                continue;
            }

            int endLba = e.Lba + (int)((e.Size + 2047u) >> 11);
            if (lba >= e.Lba && lba < endLba)
                return (e, path);
        }
        return null;
    }

    private (Entry Entry, string Path)? LocateEntryWithPath(string name)
    {
        name = NormalizeDiscPath(name);
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
        var basename = slash >= 0 ? name[(slash + 1)..] : name;
        return SearchEntry(Root(), "", basename);
    }

    private (Entry Entry, string Path)? SearchEntry(Entry dir, string basePath, string name)
    {
        foreach (var e in Entries(dir))
        {
            string path = basePath.Length == 0 ? e.Name : $"{basePath}/{e.Name}";
            if (e.IsDir)
            {
                var found = SearchEntry(e, path, name);
                if (found != null) return found;
            }
            else if (e.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                return (e, path);
        }
        return null;
    }

    public byte[] ReadSector(int lba) => ReadSectorData(lba, 2048);

    public byte[] ReadSectorData(int lba, int size)
    {
        if (size == 2048 && TryReadLooseSector(lba, out var loose))
            return loose;
        return _bin.ReadSectorData(lba, size);
    }

    public bool TryReadLooseFileRange(int startLba, uint offset, int count, out byte[] data)
    {
        data = [];
        if (!TryGetLoose(startLba, out var loose)) return false;
        data = ReadLooseRange(loose, offset, count);
        return true;
    }

    public int FirstTrackNumber => _bin.FirstTrackNumber;
    public int LastTrackNumber => _bin.LastTrackNumber;
    public int LeadOutLba => _bin.LeadOutLba;
    public bool TryGetTrackStartLba(int trackNumber, out int lba) =>
        _bin.TryGetTrackStartLba(trackNumber, out lba);
    public bool TryReadAudioSector(
        int lba,
        out byte[] sector,
        out int trackNumber,
        out int trackEndLba) =>
        _bin.TryReadAudioSector(lba, out sector, out trackNumber, out trackEndLba);

    public byte[] ReadSectors(int lba, int size) => ReadExtent(lba, size);

    private string? Search(Entry dir, string basePath, string name)
    {
        foreach (var e in Entries(dir))
        {
            if (e.IsDir)
            {
                var p = basePath.Length > 0 ? basePath + "/" + e.Name : e.Name;
                var found = Search(e, p, name);
                if (found != null) return found;
            }
            else if (e.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                return basePath.Length > 0 ? basePath + "/" + e.Name : e.Name;
        }
        return null;
    }

    private Entry Root()
    {
        var pvd = _bin.ReadSector(16);
        return ParseEntry(pvd, 156);
    }

    private Entry Find(Entry dir, string name, bool wantDir)
    {
        string upper = name.ToUpperInvariant();
        foreach (var e in Entries(dir))
            if (e.IsDir == wantDir && e.Name.Equals(upper, StringComparison.OrdinalIgnoreCase))
                return e;
        throw new FileNotFoundException($"{(wantDir ? "directory" : "File")} not found: {name}");
    }

    private IEnumerable<Entry> Entries(Entry dir)
    {
        var data = ReadExtent(dir.Lba, (int)dir.Size);
        int i = 0;
        while (i < data.Length)
        {
            byte len = data[i];
            if (len == 0) { i = (i / 2048 + 1) * 2048; continue; }
            var e = ParseEntry(data, i);
            if (e.Name is not ("\x00" or "\x01"))
                yield return e;
            i += len;
        }
    }

    private byte[] ReadExtent(int lba, int size)
    {
        var result = new byte[size];
        int done = 0;
        int cur = lba;
        while (done < size)
        {
            var sector = _bin.ReadSector(cur++);
            int n = Math.Min(2048, size - done);
            sector.AsSpan(0, n).CopyTo(result.AsSpan(done));
            done += n;
        }
        return result;
    }

    private static Entry ParseEntry(byte[] data, int off)
    {
        int lba = BitConverter.ToInt32(data, off + 2);
        uint size = BitConverter.ToUInt32(data, off + 10);
        bool isDir = (data[off + 25] & 0x02) != 0;
        int nameLen = data[off + 32];
        string raw = System.Text.Encoding.ASCII.GetString(data, off + 33, nameLen);
        int semi = raw.IndexOf(';');
        return new Entry(lba, size, isDir, semi >= 0 ? raw[..semi] : raw);
    }

    public void Dispose() => _bin.Dispose();

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

    private LooseEntry[] IndexLooseDiscEntries()
    {
        if (_looseFiles.Count == 0) return [];
        var found = new List<LooseEntry>();
        IndexLooseDiscEntries(Root(), "", found);
        foreach (var entry in found) _looseByStartLba[entry.Lba] = entry;
        return found.OrderBy(entry => entry.Lba).ToArray();
    }

    private void IndexLooseDiscEntries(Entry dir, string basePath, List<LooseEntry> found)
    {
        foreach (var entry in Entries(dir))
        {
            string path = basePath.Length == 0 ? entry.Name : $"{basePath}/{entry.Name}";
            if (entry.IsDir)
            {
                IndexLooseDiscEntries(entry, path, found);
                continue;
            }
            if (!TryResolveLoose(path, out string hostPath)) continue;
            found.Add(new LooseEntry(
                entry.Lba, entry.Size, path, hostPath, new FileInfo(hostPath).Length));
        }
    }

    private bool TryResolveLoose(string discPath, out string hostPath)
    {
        discPath = NormalizeDiscPath(discPath);
        if (_looseFiles.TryGetValue(discPath, out hostPath!)) return true;

        string basename = discPath[(discPath.LastIndexOf('/') + 1)..];
        string? match = null;
        foreach (var (candidate, path) in _looseFiles)
        {
            if (!candidate.EndsWith($"/{basename}", StringComparison.OrdinalIgnoreCase) &&
                !candidate.Equals(basename, StringComparison.OrdinalIgnoreCase))
                continue;
            if (match != null) { hostPath = ""; return false; }
            match = path;
        }
        hostPath = match ?? "";
        return match != null;
    }

    private bool TryGetLoose(int startLba, out LooseEntry entry) =>
        _looseByStartLba.TryGetValue(startLba, out entry!);

    private bool TryReadLooseSector(int lba, out byte[] data)
    {
        foreach (var entry in _looseByLba)
        {
            int sectors = (int)((entry.DiscSize + 2047u) >> 11);
            if (lba < entry.Lba || lba >= entry.Lba + sectors) continue;
            data = ReadLooseRange(entry, (uint)((lba - entry.Lba) * 2048), 2048);
            return true;
        }
        data = [];
        return false;
    }

    private static byte[] ReadLooseRange(LooseEntry entry, uint offset, int count)
    {
        var data = new byte[count];
        using var stream = File.OpenRead(entry.HostPath);
        if (offset >= stream.Length) return data;
        stream.Position = offset;
        int done = 0;
        while (done < count)
        {
            int read = stream.Read(data, done, count - done);
            if (read == 0) break;
            done += read;
        }
        return data;
    }

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
