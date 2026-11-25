using System.Buffers.Binary;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace RecompOne.Runtime.Cdrom;

// Deliberately not CueFs.ReadFile: for provenance, never fall back to a
// basename elsewhere on the disc or consult loose game files/mods.
internal sealed class OriginalV8EngineDisc : IDisposable
{
    const int RawSector = 2352, LogicalSector = 2048;
    const int MaxCueBytes = 64 * 1024, MaxDirectoryBytes = 1024 * 1024;
    internal const int MaxDiscSectors = 450000;
    static ReadOnlySpan<byte> Sync => [0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0];
    static readonly Regex FileLine = new("^FILE\\s+\"([^\"]+)\"\\s+BINARY$",
        RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

    readonly FileStream _data;
    readonly int _start, _trackSectors;
    int _volumeSectors;

    OriginalV8EngineDisc(FileStream data, int start, int trackSectors)
        => (_data, _start, _trackSectors, _volumeSectors) = (data, start, trackSectors, trackSectors);

    sealed class Track(string path, string mode)
    {
        internal string Path { get; } = path;
        internal string Mode { get; } = mode;
        internal int? Start, Pregap;
    }

    internal static OriginalV8EngineDisc OpenCue(string cuePath)
    {
        cuePath = Path.GetFullPath(cuePath);
        if (!Path.GetExtension(cuePath).Equals(".cue", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("Select the original USA disc's CUE.");
        string directory = Path.GetDirectoryName(cuePath)!;
        // Check size before opening too, so a non-file such as a zero-length
        // named pipe is never awaited as if it were a disc.
        CheckCueLength(new FileInfo(cuePath).Length);
        using var cue = File.OpenRead(cuePath);
        CheckCueLength(cue.Length);
        byte[] cueBytes = new byte[(int)cue.Length];
        cue.ReadExactly(cueBytes);
        string text = new UTF8Encoding(false, true).GetString(cueBytes).TrimStart('\uFEFF');
        var tracks = new List<Track>();
        string? file = null;
        foreach (string rawLine in text.Split('\n'))
        {
            string line = rawLine.Trim();
            string[] fields = line.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (fields.Length == 0) continue;
            switch (fields[0].ToUpperInvariant())
            {
                case "FILE":
                    Match match = FileLine.Match(line);
                    if (!match.Success)
                        throw new InvalidDataException("Original disc CUE requires a quoted BINARY FILE.");
                    file = LocalPath(directory, match.Groups[1].Value);
                    break;
                case "TRACK":
                    if (file == null || fields.Length != 3 || tracks.Count == 99 ||
                        fields[1] != (tracks.Count + 1).ToString("00", CultureInfo.InvariantCulture))
                        throw new InvalidDataException("Original disc CUE tracks must start at 01 and be ordered.");
                    tracks.Add(new(file, fields[2].ToUpperInvariant()));
                    break;
                case "INDEX":
                    if (tracks.Count == 0 || fields.Length != 3)
                        throw new InvalidDataException("Invalid original disc CUE index.");
                    ref int? index = ref (fields[1] == "01" ? ref tracks[^1].Start : ref tracks[^1].Pregap);
                    if (fields[1] is not ("00" or "01") || index.HasValue)
                        throw new InvalidDataException("Original disc CUE uses duplicate or unsupported indexes.");
                    index = Frames(fields[2]);
                    break;
            }
        }
        if (tracks.Count == 0 || tracks[0].Mode != "MODE2/2352" || tracks[0].Start is not int start ||
            tracks[0].Pregap > start || !tracks[0].Path.EndsWith(".bin", StringComparison.OrdinalIgnoreCase) ||
            tracks.Skip(1).Any(t => t.Mode != "AUDIO"))
            throw new InvalidDataException("Original disc requires Track 01 MODE2/2352 with INDEX 01.");

        RejectLinkedSource(directory, tracks[0].Path);
        CheckTrackLength(new FileInfo(tracks[0].Path).Length);
        var data = new FileStream(tracks[0].Path, FileMode.Open, FileAccess.Read, FileShare.Read);
        try
        {
            CheckTrackLength(data.Length);
            int fileSectors = (int)(data.Length / RawSector);
            int end = fileSectors, previousStart = start;
            var comparison = OperatingSystem.IsWindows() ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;
            foreach (var audio in tracks.Skip(1))
            {
                // Data-track-only imports do not need to open or stat any CDDA
                // BINs. In a combined BIN, stop before the first audio pregap.
                if (!audio.Path.Equals(tracks[0].Path, comparison)) continue;
                if (audio.Start is not int audioStart || audio.Pregap > audioStart ||
                    audioStart >= fileSectors || (audio.Pregap ?? audioStart) <= previousStart)
                    throw new InvalidDataException("Invalid audio boundary in combined original BIN.");
                end = Math.Min(end, audio.Pregap ?? audioStart);
                previousStart = audioStart;
            }
            if (end - start <= 16)
                throw new InvalidDataException("Original data track ends before its volume descriptor.");
            return new(data, start, end - start);
        }
        catch
        {
            data.Dispose();
            throw;
        }
    }

    static void CheckCueLength(long length)
    {
        if (length <= 0 || length > MaxCueBytes)
            throw new InvalidDataException("Original disc CUE exceeds size bounds.");
    }

    static void CheckTrackLength(long length)
    {
        if (length <= 16L * RawSector || length % RawSector != 0 || length > (long)RawSector * MaxDiscSectors)
            throw new InvalidDataException("Original data track is not a bounded raw-sector BIN.");
    }

    static string LocalPath(string directory, string path)
    {
        path = path.Replace('\\', '/');
        if (Path.IsPathRooted(path) || path.Contains(':'))
            throw new InvalidDataException("Keep the original disc BINs beside or below their CUE.");
        string full = Path.GetFullPath(Path.Combine(directory, path));
        string relative = Path.GetRelativePath(directory, full);
        if (relative is "." or ".." || relative.StartsWith(".." + Path.DirectorySeparatorChar, StringComparison.Ordinal))
            throw new InvalidDataException("Original disc CUE may not reference files outside its folder.");
        return full;
    }

    static void RejectLinkedSource(string directory, string target)
    {
        foreach (string part in Path.GetRelativePath(directory, target).Split(Path.DirectorySeparatorChar))
        {
            directory = Path.Combine(directory, part);
            if ((File.GetAttributes(directory) & FileAttributes.ReparsePoint) != 0)
                throw new InvalidDataException("Original disc CUE may not follow BIN or subfolder links.");
        }
    }

    static int Frames(string value)
    {
        if (value.Length != 8 || value[2] != ':' || value[5] != ':' ||
            !int.TryParse(value.AsSpan(0, 2), NumberStyles.None, CultureInfo.InvariantCulture, out int mm) ||
            !int.TryParse(value.AsSpan(3, 2), NumberStyles.None, CultureInfo.InvariantCulture, out int ss) ||
            !int.TryParse(value.AsSpan(6, 2), NumberStyles.None, CultureInfo.InvariantCulture, out int ff) ||
            ss >= 60 || ff >= 75)
            throw new InvalidDataException("Invalid original disc CUE timecode.");
        return (mm * 60 + ss) * 75 + ff;
    }

    internal (OriginalV8EngineRevision Revision, int ArmsLba, byte[] Arms) ReadDonorFiles(
        IReadOnlyList<OriginalV8EngineRevision> revisions, CancellationToken cancellation)
    {
        Span<byte> pvd = stackalloc byte[LogicalSector];
        ReadSector(16, pvd);
        uint volume = Both32(pvd[80..]);
        if (pvd[0] != 1 || !pvd.Slice(1, 5).SequenceEqual("CD001"u8) || pvd[6] != 1 ||
            Both16(pvd[128..]) != LogicalSector || volume is <= 16 or > MaxDiscSectors)
            throw new InvalidDataException("Invalid original disc ISO9660 volume descriptor.");
        _volumeSectors = (int)volume; // May include redbook placeholders beyond Track 01.
        var root = Record(pvd.Slice(156, pvd[156]), directory: true);
        byte[] rootBytes = ReadExtent(root, MaxDirectoryBytes, cancellation);
        var cnf = Find(rootBytes, "SYSTEM.CNF;1"u8, directory: false);
        if (!IsOriginalBoot(ReadExtent(cnf, 4096, cancellation)))
            throw new InvalidDataException("Wrong disc: expected original Vigilante 8 USA (SLUS_005.10).");

        var boot = Find(rootBytes, "SLUS_005.10;1"u8, directory: false);
        if (!revisions.Any(item => item.BootLength == boot.Length))
            throw new InvalidDataException("Unverified original Vigilante 8 USA executable revision.");
        byte[] program = ReadExtent(boot, 1024 * 1024, cancellation);
        if (!program.AsSpan(0, Math.Min(8, program.Length)).SequenceEqual("PS-X EXE"u8))
            throw new InvalidDataException("Invalid original Vigilante 8 boot executable.");
        var revision = revisions.SingleOrDefault(item => item.BootLength == program.Length &&
            OriginalV8EngineDonor.MatchesHash(program, item.BootSha256));
        if (revision == null)
            throw new InvalidDataException("Unverified original Vigilante 8 USA executable revision.");

        var hud = Find(rootBytes, "HUD"u8, directory: true);
        var bank = Find(ReadExtent(hud, MaxDirectoryBytes, cancellation), "ARMS.EXP;1"u8, directory: false);
        if (bank.Length != revision.ArmsLength)
            throw new InvalidDataException("HUD/ARMS.EXP does not match the verified original revision.");
        byte[] arms = ReadExtent(bank, OriginalV8EngineDonor.MaxArmsBytes, cancellation);
        if (bank.Lba != revision.ArmsLba || !OriginalV8EngineDonor.MatchesHash(arms, revision.ArmsSha256))
            throw new InvalidDataException("HUD/ARMS.EXP does not match the verified original revision.");
        return (revision, bank.Lba, arms);
    }

    static bool IsOriginalBoot(byte[] cnf)
    {
        int boots = 0;
        foreach (string line in Encoding.ASCII.GetString(cnf).Split('\n'))
        {
            int eq = line.IndexOf('=');
            if (eq < 0 || !line[..eq].Trim().Equals("BOOT", StringComparison.OrdinalIgnoreCase)) continue;
            string value = line[(eq + 1)..].Trim();
            if (!value.StartsWith("cdrom:", StringComparison.OrdinalIgnoreCase) ||
                !value[6..].TrimStart('\\', '/').Equals("SLUS_005.10;1", StringComparison.OrdinalIgnoreCase))
                return false;
            boots++;
        }
        return boots == 1;
    }

    record struct Extent(int Lba, int Length);

    static Extent Find(ReadOnlySpan<byte> records, ReadOnlySpan<byte> name, bool directory)
    {
        Extent? match = null;
        int cursor = 0;
        while (cursor < records.Length)
        {
            int remaining = LogicalSector - (cursor % LogicalSector);
            int length = records[cursor];
            if (length == 0)
            {
                cursor += remaining;
                continue;
            }
            if (length < 34 || length > remaining || length > records.Length - cursor)
                throw new InvalidDataException("Truncated original ISO9660 directory record.");
            var record = records.Slice(cursor, length);
            int nameLength = record[32];
            if (nameLength == 0 || nameLength > length - 33)
                throw new InvalidDataException("Invalid original ISO9660 directory identifier.");
            if (record.Slice(33, nameLength).SequenceEqual(name))
            {
                if (match.HasValue)
                    throw new InvalidDataException("Duplicate exact original ISO9660 path.");
                match = Record(record, directory);
            }
            cursor += length;
        }
        return match ?? throw new InvalidDataException($"Missing exact original ISO9660 entry: {Encoding.ASCII.GetString(name)}.");
    }

    static Extent Record(ReadOnlySpan<byte> record, bool directory)
    {
        if (record.Length < 34 || record[1] != 0 || record[26] != 0 || record[27] != 0 ||
            (record[25] & 0x80) != 0 || ((record[25] & 2) != 0) != directory || Both16(record[28..]) != 1)
            throw new InvalidDataException("Unsupported original ISO9660 extent layout.");
        uint lba = Both32(record[2..]);
        uint length = Both32(record[10..]);
        if (lba is 0 or >= MaxDiscSectors || length == 0 || length > int.MaxValue)
            throw new InvalidDataException("Invalid original ISO9660 extent location or size.");
        return new((int)lba, (int)length);
    }

    byte[] ReadExtent(Extent extent, int maxBytes, CancellationToken cancellation)
    {
        if (extent.Length > maxBytes)
            throw new InvalidDataException("Original ISO9660 extent exceeds its size bound.");
        int count = (extent.Length + (LogicalSector - 1)) / LogicalSector;
        if (extent.Lba + count > Math.Min(_trackSectors, _volumeSectors))
            throw new InvalidDataException("Original ISO9660 extent is outside Track 01.");
        byte[] data = new byte[extent.Length];
        Span<byte> sector = stackalloc byte[LogicalSector];
        for (int i = 0; i < count; i++)
        {
            cancellation.ThrowIfCancellationRequested();
            ReadSector(extent.Lba + i, sector);
            int offset = i * LogicalSector;
            sector[..Math.Min(LogicalSector, data.Length - offset)].CopyTo(data.AsSpan(offset));
        }
        return data;
    }

    void ReadSector(int lba, Span<byte> data)
    {
        Span<byte> raw = stackalloc byte[RawSector];
        _data.Position = (long)(_start + lba) * RawSector;
        _data.ReadExactly(raw);
        if (!raw[..12].SequenceEqual(Sync) || raw[15] != 2 ||
            !raw.Slice(16, 4).SequenceEqual(raw.Slice(20, 4)) || (raw[18] & 0x20) != 0)
            throw new InvalidDataException("Original ISO9660 data must be in raw Mode 2 Form 1 sectors.");
        raw.Slice(24, LogicalSector).CopyTo(data);
    }

    static uint Both32(ReadOnlySpan<byte> bytes)
    {
        uint value = BinaryPrimitives.ReadUInt32LittleEndian(bytes);
        return BinaryPrimitives.ReadUInt32BigEndian(bytes[4..]) == value
            ? value : throw new InvalidDataException("Original ISO9660 extent endian copies disagree.");
    }

    static ushort Both16(ReadOnlySpan<byte> bytes)
    {
        ushort value = BinaryPrimitives.ReadUInt16LittleEndian(bytes);
        return BinaryPrimitives.ReadUInt16BigEndian(bytes[2..]) == value
            ? value : throw new InvalidDataException("Original ISO9660 field endian copies disagree.");
    }

    public void Dispose() => _data.Dispose();
}
