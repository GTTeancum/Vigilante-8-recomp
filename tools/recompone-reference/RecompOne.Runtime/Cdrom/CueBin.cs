namespace RecompOne.Runtime.Cdrom;

public sealed class CueBin : IDisposable
{
    private sealed class TrackBuilder
    {
        public required string BinPath;
        public required int Number;
        public required string Mode;
        public int Index0 = -1;
        public int Index1 = -1;
    }

    private record Track(
        string BinPath,
        int Number,
        string Mode,
        int SectorSize,
        int DataOffset,
        long FileOffset,
        int FileBaseLba,
        int Index0Lba,
        int Index1Lba,
        int EndLba);

    private readonly List<Track> _tracks = [];
    private readonly Dictionary<string, FileStream> _files = [];
    private readonly object _ioGate = new();

    private CueBin() {}

    public static CueBin Open(string cuePath)
    {
        var cb = new CueBin();
        cb.Parse(cuePath);
        return cb;
    }

    private void Parse(string cuePath)
    {
        string dir = Path.GetDirectoryName(Path.GetFullPath(cuePath)) ?? "";
        string? currentFile = null;
        TrackBuilder? currentTrack = null;
        var builders = new List<TrackBuilder>();

        foreach (var raw in File.ReadLines(cuePath))
        {
            var line = raw.Trim();
            if (line.StartsWith("FILE ", StringComparison.OrdinalIgnoreCase))
            {
                int a = line.IndexOf('"') + 1;
                int b = line.LastIndexOf('"');
                currentFile = Path.Combine(dir, line[a..b]);
            }
            else if (line.StartsWith("TRACK ", StringComparison.OrdinalIgnoreCase))
            {
                var p = line.Split(' ', 3, StringSplitOptions.RemoveEmptyEntries);
                currentTrack = new TrackBuilder
                {
                    BinPath = currentFile ?? throw new InvalidDataException("TRACK appeared before FILE"),
                    Number = int.Parse(p[1]),
                    Mode = p[2],
                };
                builders.Add(currentTrack);
            }
            else if (line.StartsWith("INDEX ", StringComparison.OrdinalIgnoreCase))
            {
                if (currentTrack == null) throw new InvalidDataException("INDEX appeared before TRACK");
                var p = line.Split(' ', 3, StringSplitOptions.RemoveEmptyEntries);
                int sectors = checked((int)MsfToSectors(p[2]));
                if (p[1] == "00") currentTrack.Index0 = sectors;
                else if (p[1] == "01") currentTrack.Index1 = sectors;
            }
        }

        int fileBaseLba = 0;
        foreach (var fileGroup in builders.GroupBy(builder => builder.BinPath))
        {
            var fileTracks = fileGroup.ToList();
            int sectorSize = GetSectorSize(fileTracks[0].Mode);
            long fileBytes = new FileInfo(fileGroup.Key).Length;
            int fileSectors = checked((int)(fileBytes / sectorSize));
            for (int i = 0; i < fileTracks.Count; i++)
            {
                var builder = fileTracks[i];
                int index0 = builder.Index0 >= 0 ? builder.Index0 : Math.Max(0, builder.Index1);
                int index1 = builder.Index1 >= 0 ? builder.Index1 : 0;
                int endInFile = fileSectors;
                if (i + 1 < fileTracks.Count)
                {
                    var next = fileTracks[i + 1];
                    endInFile = next.Index0 >= 0 ? next.Index0 : Math.Max(0, next.Index1);
                }
                int builderSectorSize = GetSectorSize(builder.Mode);
                _tracks.Add(new Track(
                    builder.BinPath,
                    builder.Number,
                    builder.Mode,
                    builderSectorSize,
                    GetDataOffset(builder.Mode),
                    (long)index1 * builderSectorSize,
                    fileBaseLba,
                    fileBaseLba + index0,
                    fileBaseLba + index1,
                    fileBaseLba + endInFile));
            }
            fileBaseLba += fileSectors;
        }
    }

    public int FirstTrackNumber => _tracks.Count == 0 ? 0 : _tracks.Min(track => track.Number);
    public int LastTrackNumber => _tracks.Count == 0 ? 0 : _tracks.Max(track => track.Number);
    public int LeadOutLba => _tracks.Count == 0 ? 0 : _tracks.Max(track => track.EndLba);

    public bool TryGetTrackStartLba(int trackNumber, out int lba)
    {
        var track = _tracks.Find(candidate => candidate.Number == trackNumber);
        lba = track?.Index1Lba ?? 0;
        return track != null;
    }

    public bool TryReadAudioSector(
        int discLba,
        out byte[] sector,
        out int trackNumber,
        out int trackEndLba)
    {
        var track = _tracks.Find(candidate =>
            candidate.Mode.Equals("AUDIO", StringComparison.OrdinalIgnoreCase) &&
            discLba >= candidate.Index0Lba && discLba < candidate.EndLba);
        if (track == null)
        {
            sector = [];
            trackNumber = 0;
            trackEndLba = 0;
            return false;
        }

        var stream = GetStream(track.BinPath);
        long pos = (long)(discLba - track.FileBaseLba) * track.SectorSize;
        sector = new byte[2352];
        lock (_ioGate)
        {
            stream.Seek(pos, SeekOrigin.Begin);
            stream.ReadExactly(sector);
        }
        trackNumber = track.Number;
        trackEndLba = track.EndLba;
        return true;
    }

    public byte[] ReadSector(int lba) => ReadSectorData(lba, 2048);

    public byte[] ReadSectorData(int lba, int size)
    {
        var t = DataTrack();
        var stream = GetStream(t.BinPath);
        int offset = t.SectorSize == 2352
            ? size switch { >= 2340 => 12, >= 2329 => 16, _ => 24 }
            : t.DataOffset;
        long pos = t.FileOffset + (long)lba * t.SectorSize + offset;
        var buf = new byte[size];
        if (lba < 0) return buf;
        int want = Math.Min(size, t.SectorSize - offset);
        lock (_ioGate)
        {
            if (pos >= stream.Length) return buf;
            int avail = (int)Math.Min(want, stream.Length - pos);
            stream.Seek(pos, SeekOrigin.Begin);
            stream.ReadExactly(buf, 0, avail);
        }
        return buf;
    }

    private Track DataTrack() => _tracks.Find(t => !t.Mode.Equals("AUDIO", StringComparison.OrdinalIgnoreCase)) ?? throw new InvalidOperationException("no data track was found in cue sheet");

    private FileStream GetStream(string path)
    {
        lock (_ioGate)
        {
            if (!_files.TryGetValue(path, out var s))
                _files[path] = s = File.OpenRead(path);
            return s;
        }
    }

    private static long MsfToSectors(string msf)
    {
        var p = msf.Split(':');
        return long.Parse(p[0]) * 60 * 75 + long.Parse(p[1]) * 75 + long.Parse(p[2]);
    }

    private static int GetSectorSize(string mode) => mode switch
    {
        "MODE1/2048" => 2048,
        "MODE2/2336" => 2336,
        _ => 2352,
    };

    private static int GetDataOffset(string mode) => mode switch
    {
        "MODE1/2352" => 16,
        "MODE2/2352" => 24,
        "MODE2/2336" => 8,
        _ => 0,
    };

    public void Dispose()
    {
        foreach (var s in _files.Values) s.Dispose();
        _files.Clear();
    }
}
