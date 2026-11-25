using System.Buffers.Binary;
using System.Security.Cryptography;

namespace RecompOne.Runtime.Cdrom;

internal enum OriginalV8EngineCacheResult { Created, Unchanged, Replaced }

// V8EA v1: little-endian metadata, 13 ordered pitch+ADPCM records, SHA-256
// footer. The compiled donor allowlist additionally authenticates the exact
// record section: an edited sample plus a freshly computed footer is rejected.
// This class only stores data; it does not enable engines or change settings.
internal static class OriginalV8EngineSidecar
{
    internal const string FileName = "original-v8-engines.v8e";
    const int HeaderBytes = 84, RecordBytes = 12, DigestBytes = 32;
    const int MinFileBytes = HeaderBytes + OriginalV8EngineDonor.SampleCount * (RecordBytes + 32) + DigestBytes;
    internal const int MaxFileBytes = HeaderBytes + OriginalV8EngineDonor.SampleCount *
        (RecordBytes + OriginalV8EngineDonor.MaxSampleBytes) + DigestBytes;

    internal static byte[] Encode(OriginalV8EnginePack pack,
        IReadOnlyList<OriginalV8EngineRevision> revisions)
    {
        var rev = LookupRevision(pack.Revision.Id, revisions);
        if (pack.Revision != rev || pack.Loops.Count != OriginalV8EngineDonor.SampleCount)
            throw new InvalidDataException("Unknown original engine source or missing drivers.");
        CheckArmsExtent(pack.ArmsLba, rev);
        int previous = 0, totalBytes = HeaderBytes + DigestBytes;
        for (int i = 0; i < pack.Loops.Count; i++)
        {
            var loop = pack.Loops[i];
            if (loop.StableId != OriginalV8EngineDonor.StableId(i))
                throw new InvalidDataException("Unexpected original engine driver order.");
            previous = CheckRecord(loop.DonorOffset, loop.Adpcm.Span, loop.Pitch, previous, rev.ArmsLength);
            totalBytes += RecordBytes + loop.Adpcm.Length;
        }
        byte[] bytes = new byte[totalBytes];
        Span<byte> output = bytes;
        "V8EA"u8.CopyTo(output);
        Write16(output[4..], 1);
        Write16(output[6..], rev.Id);
        Write16(output[8..], OriginalV8EngineDonor.SampleCount);
        Write32(output[12..], pack.ArmsLba);
        Write32(output[16..], rev.ArmsLength);
        Convert.FromHexString(rev.BootSha256).CopyTo(output[20..52]);
        Convert.FromHexString(rev.ArmsSha256).CopyTo(output[52..84]);

        int cursor = HeaderBytes;
        foreach (var loop in pack.Loops)
        {
            Write32(output[cursor..], loop.DonorOffset);
            Write32(output[(cursor + 4)..], loop.Adpcm.Length);
            Write16(output[(cursor + 8)..], loop.Pitch);
            loop.Adpcm.Span.CopyTo(output[(cursor + RecordBytes)..]);
            cursor += RecordBytes + loop.Adpcm.Length;
        }
        CheckEntriesHash(output[HeaderBytes..cursor], rev);
        SHA256.HashData(output[..cursor], output[cursor..]);
        return bytes;
    }

    internal static OriginalV8EnginePack Decode(ReadOnlySpan<byte> source,
        IReadOnlyList<OriginalV8EngineRevision> revisions)
    {
        if (source.Length is < MinFileBytes or > MaxFileBytes ||
            !source[..4].SequenceEqual("V8EA"u8) || Read16(source[4..]) != 1 ||
            Read16(source[8..]) != OriginalV8EngineDonor.SampleCount || Read16(source[10..]) != 0)
            throw new InvalidDataException("Unrecognized or incomplete original engine cache.");
        var revision = LookupRevision(Read16(source[6..]), revisions);
        int armsLba = Read32(source[12..]);
        CheckArmsExtent(armsLba, revision);
        if (Read32(source[16..]) != revision.ArmsLength ||
            !source[20..52].SequenceEqual(Convert.FromHexString(revision.BootSha256)) ||
            !source[52..84].SequenceEqual(Convert.FromHexString(revision.ArmsSha256)) ||
            !CryptographicOperations.FixedTimeEquals(SHA256.HashData(source[..^DigestBytes]), source[^DigestBytes..]))
            throw new InvalidDataException("Original engine cache metadata or checksum does not match.");
        ReadOnlySpan<byte> records = source[HeaderBytes..^DigestBytes];
        CheckEntriesHash(records, revision);
        int previous = 0;
        var loops = new OriginalV8EngineLoop[OriginalV8EngineDonor.SampleCount];
        for (int i = 0; i < loops.Length; i++)
        {
            if (records.Length < RecordBytes || Read16(records[10..]) != 0)
                throw new InvalidDataException("Original engine cache has an invalid driver record.");
            int offset = Read32(records), length = Read32(records[4..]);
            ushort pitch = Read16(records[8..]);
            if (length < 0 || length > records.Length - RecordBytes)
                throw new InvalidDataException("Original engine cache has a truncated sample.");
            var payload = records.Slice(RecordBytes, length);
            previous = CheckRecord(offset, payload, pitch, previous, revision.ArmsLength);
            loops[i] = new(OriginalV8EngineDonor.StableId(i), offset, pitch, payload.ToArray());
            records = records[(RecordBytes + length)..];
        }
        if (!records.IsEmpty)
            throw new InvalidDataException("Original engine cache contains trailing driver data.");
        return new(revision, armsLba, Array.AsReadOnly(loops));
    }

    internal static OriginalV8EnginePack Read(string directory) =>
        Read(directory, OriginalV8EngineDonor.SupportedRevisions);

    internal static OriginalV8EnginePack Read(string directory, IReadOnlyList<OriginalV8EngineRevision> revisions)
    {
        string path = Path.Combine(Path.GetFullPath(directory), FileName);
        return Decode(Existing(path) ?? throw new FileNotFoundException("Original engine cache is not installed."),
            revisions);
    }

    // Caller must explicitly choose a local cache directory. No registry, mod
    // path or settings write happens, and an unknown existing file isn't ours.
    internal static OriginalV8EngineCacheResult WriteAtomic(string directory, OriginalV8EnginePack pack,
        bool replaceExisting = false, CancellationToken cancellation = default) =>
        WriteAtomic(directory, pack, OriginalV8EngineDonor.SupportedRevisions, replaceExisting, cancellation);

    internal static OriginalV8EngineCacheResult WriteAtomic(string directory, OriginalV8EnginePack pack,
        IReadOnlyList<OriginalV8EngineRevision> revisions, bool replaceExisting = false,
        CancellationToken cancellation = default)
    {
        cancellation.ThrowIfCancellationRequested();
        byte[] bytes = Encode(pack, revisions);
        directory = Path.GetFullPath(directory);
        string target = Path.Combine(directory, FileName);
        byte[]? existing = Existing(target);
        if (existing != null)
        {
            _ = Decode(existing, revisions);
            if (bytes.AsSpan().SequenceEqual(existing))
                return OriginalV8EngineCacheResult.Unchanged;
            if (!replaceExisting)
                throw new IOException("An original engine cache already exists; replacement must be requested.");
        }
        string temporary = Path.Combine(directory, $".{FileName}.{Guid.NewGuid():N}.tmp");
        bool ownsTemporary = false;
        try
        {
            var options = new FileStreamOptions
            {
                Mode = FileMode.CreateNew, Access = FileAccess.ReadWrite, Share = FileShare.None,
            };
            if (!OperatingSystem.IsWindows())
                options.UnixCreateMode = UnixFileMode.UserRead | UnixFileMode.UserWrite;
            using (var stream = new FileStream(temporary, options))
            {
                ownsTemporary = true;
                stream.Write(bytes);
                stream.Flush(flushToDisk: true);
                stream.Position = 0;
                byte[] readback = ReadBounded(stream);
                _ = Decode(readback, revisions);
                if (!bytes.AsSpan().SequenceEqual(readback))
                    throw new InvalidDataException("Original engine cache changed during verification.");
            }
            cancellation.ThrowIfCancellationRequested();
            if (existing == null)
            {
                File.Move(temporary, target, overwrite: false);
            }
            else
            {
                var current = Existing(target);
                if (current == null || !current.AsSpan().SequenceEqual(existing))
                    throw new IOException("Original engine cache changed while preparing its replacement.");
                File.Replace(temporary, target, destinationBackupFileName: null);
            }
            ownsTemporary = false;
            return existing == null ? OriginalV8EngineCacheResult.Created : OriginalV8EngineCacheResult.Replaced;
        }
        finally
        {
            if (ownsTemporary)
            {
                try { File.Delete(temporary); }
                catch (IOException) { } // Ignore uncommitted scratch on sharing failure.
                catch (UnauthorizedAccessException) { }
            }
        }
    }

    static byte[]? Existing(string target)
    {
        FileAttributes attrs;
        try { attrs = File.GetAttributes(target); }
        catch (FileNotFoundException) { return null; }
        if ((attrs & (FileAttributes.Directory | FileAttributes.ReparsePoint)) != 0)
            throw new IOException("The original engine cache cannot be a directory or link.");
        CheckFileLength(new FileInfo(target).Length);
        using var stream = File.OpenRead(target);
        return ReadBounded(stream);
    }

    static byte[] ReadBounded(Stream stream)
    {
        long length = stream.Length;
        CheckFileLength(length);
        byte[] data = new byte[(int)length];
        stream.ReadExactly(data);
        if (stream.ReadByte() != -1)
            throw new InvalidDataException("Original engine cache grew during read.");
        return data;
    }

    static void CheckFileLength(long bytes)
    {
        if (bytes is < MinFileBytes or > MaxFileBytes)
            throw new InvalidDataException("Original engine cache exceeds size bounds.");
    }

    static OriginalV8EngineRevision LookupRevision(ushort id, IReadOnlyList<OriginalV8EngineRevision> revisions) =>
        revisions.SingleOrDefault(revision => revision.Id == id) ??
        throw new InvalidDataException("Unverified original engine donor revision.");

    static void CheckArmsExtent(int lba, OriginalV8EngineRevision revision)
    {
        int bytes = revision.ArmsLength;
        if (lba != revision.ArmsLba || bytes <= 0 || bytes > OriginalV8EngineDonor.MaxArmsBytes || lba <= 0 ||
            (long)lba + (bytes + 2047) / 2048 > OriginalV8EngineDisc.MaxDiscSectors)
            throw new InvalidDataException("Original engine cache contains an invalid donor extent.");
    }

    static int CheckRecord(int offset, ReadOnlySpan<byte> payload, ushort pitch, int previous, int extentLength)
    {
        if (offset <= 0 || offset < previous || offset > extentLength || payload.Length > extentLength - offset)
            throw new InvalidDataException("Original engine cache driver offsets exceed the donor.");
        OriginalV8EngineDonor.ValidateLoop(payload, pitch);
        return offset + payload.Length;
    }

    static void CheckEntriesHash(ReadOnlySpan<byte> records, OriginalV8EngineRevision revision)
    {
        if (!OriginalV8EngineDonor.MatchesHash(records, revision.EntriesSha256))
            throw new InvalidDataException("Original engine samples do not match the verified donor.");
    }

    static ushort Read16(ReadOnlySpan<byte> data) => BinaryPrimitives.ReadUInt16LittleEndian(data);
    static int Read32(ReadOnlySpan<byte> data) => BinaryPrimitives.ReadInt32LittleEndian(data);
    static void Write16(Span<byte> data, ushort value) => BinaryPrimitives.WriteUInt16LittleEndian(data, value);
    static void Write32(Span<byte> data, int value) => BinaryPrimitives.WriteInt32LittleEndian(data, value);
}
