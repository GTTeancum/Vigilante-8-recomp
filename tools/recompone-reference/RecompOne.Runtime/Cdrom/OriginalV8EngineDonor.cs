using System.Buffers.Binary;
using System.Security.Cryptography;

namespace RecompOne.Runtime.Cdrom;

internal sealed record OriginalV8EngineRevision(
    ushort Id, int BootLength, string BootSha256, int ArmsLba, int ArmsLength, string ArmsSha256,
    string EntriesSha256);

internal sealed record OriginalV8EngineLoop(
    string StableId, int DonorOffset, ushort Pitch, ReadOnlyMemory<byte> Adpcm);

internal sealed record OriginalV8EnginePack(
    OriginalV8EngineRevision Revision, int ArmsLba, IReadOnlyList<OriginalV8EngineLoop> Loops);

// Strict bounded reader for an explicitly selected original setup disc.
internal static class OriginalV8EngineDonor
{
    internal const int MaxArmsBytes = 4 * 1024 * 1024;
    internal const int MaxSampleBytes = 128 * 1024;
    internal const int SampleCount = 13;

    // Metadata only. Verified against the current updater and a local USA Rev 1
    // disc; a matching title ID alone is not enough to identify a revision.
    internal static IReadOnlyList<OriginalV8EngineRevision> SupportedRevisions { get; } =
        Array.AsReadOnly<OriginalV8EngineRevision>([
            new(1, 358400, "48FEBA840D7E16E134BD0A12D6D50A29C7FEBC29023E2A02840C3A583B190E60",
                105969, 168426, "FC6636A88A6EAE4F8AB9C28A2ACF61DD606F73F528A694FD7E081A8887E0B641",
                "8EE0C1A536594423B39FCF9975355735B1CE97E1301E467EE747FCA0007263D9")
        ]);

    // Original HUD/ARMS.EXP XOBFs are in the first game's driver order, not
    // the Second Offense guest roster order. These IDs match guest_audio.py.
    static readonly string[] StableIds = [
        "guest.v8.chassey_blue", "guest.v8.slick_clyde", "guest.v8.sheila",
        "guest.v8.john_torque", "guest.v8.dave", "guest.v8.convoy",
        "guest.v8.loki", "guest.v8.houston_3", "guest.v8.boogie",
        "guest.v8.beezwax", "guest.v8.molo", "guest.v8.sid_burn",
        "guest.v8.y_the_alien"
    ];

    internal static string StableId(int index) => StableIds[index];

    internal static OriginalV8EnginePack ExtractFromCue(
        string cuePath, CancellationToken cancellation = default) =>
        ExtractFromCue(cuePath, SupportedRevisions, cancellation);

    // The linked-source fixture supplies a separate synthetic revision list.
    internal static OriginalV8EnginePack ExtractFromCue(
        string cuePath, IReadOnlyList<OriginalV8EngineRevision> revisions,
        CancellationToken cancellation = default)
    {
        cancellation.ThrowIfCancellationRequested();
        using var disc = OriginalV8EngineDisc.OpenCue(cuePath);
        var bootSector = disc.ReadDonorFiles(revisions, cancellation);
        var loops = DecodeArms(bootSector.Arms, cancellation);
        var pack = new OriginalV8EnginePack(bootSector.Revision, bootSector.ArmsLba, loops);
        OriginalV8EngineSidecar.Encode(pack, revisions); // Verify exact ordered loop metadata too.
        return pack;
    }

    internal static IReadOnlyList<OriginalV8EngineLoop> DecodeArms(
        ReadOnlySpan<byte> arms, CancellationToken cancellation = default)
    {
        if (arms.IsEmpty || arms.Length > MaxArmsBytes)
            throw new InvalidDataException("HUD/ARMS.EXP exceeds the donor size bounds.");
        int pos = 0;
        var terr = Take(arms, ref pos, arms.Length, "FORM"u8);
        if (pos != arms.Length || terr.Length < 4 ||
            !arms.Slice(terr.Offset, 4).SequenceEqual("TERR"u8))
            throw new InvalidDataException("HUD/ARMS.EXP must be a complete TERR form.");
        int cursor = terr.Offset + 4;
        var head = Take(arms, ref cursor, terr.End, "HEAD"u8);
        if (head.Length == 0)
            throw new InvalidDataException("HUD/ARMS.EXP has no TERR header.");
        var loops = new OriginalV8EngineLoop[SampleCount];
        for (int i = 0; i < loops.Length; i++)
        {
            cancellation.ThrowIfCancellationRequested();
            var form = Take(arms, ref cursor, terr.End, "FORM"u8);
            if (form.Length < 4 || !arms.Slice(form.Offset, 4).SequenceEqual("XOBF"u8))
                throw new InvalidDataException("Expected 13 original driver XOBF forms in order.");
            int child = form.Offset + 4;
            _ = Take(arms, ref child, form.End, "BIN "u8); // Skip unrelated object bytes.
            var snd = Take(arms, ref child, form.End, "SND "u8);
            if (child != form.End || snd.Length < 8)
                throw new InvalidDataException("Original XOBF must contain one complete SND chunk.");
            var header = arms.Slice(snd.Offset, 8);
            int bytes = BinaryPrimitives.ReadUInt16LittleEndian(header[2..]) * 8;
            if (BinaryPrimitives.ReadUInt16LittleEndian(header) != 1 ||
                BinaryPrimitives.ReadUInt16LittleEndian(header[4..]) != 0 ||
                bytes != snd.Length - 8)
                throw new InvalidDataException("Original SND must be a single unpadded sample at offset zero.");
            ushort pitch = BinaryPrimitives.ReadUInt16LittleEndian(header[6..]);
            int sourceOffset = snd.Offset + 8;
            var payload = arms.Slice(sourceOffset, bytes);
            ValidateLoop(payload, pitch);
            loops[i] = new(StableIds[i], sourceOffset, pitch, payload.ToArray());
        }
        var palette = Take(arms, ref cursor, terr.End, "PLTX"u8);
        if (palette.Length != 0 || cursor != terr.End)
            throw new InvalidDataException("Unexpected extra original driver or trailing donor data.");
        return Array.AsReadOnly(loops);
    }

    internal static void ValidateLoop(ReadOnlySpan<byte> bytes, ushort pitch)
    {
        if (pitch is 0 or > 0x3FFF || bytes.Length < 32 ||
            bytes.Length > MaxSampleBytes || (bytes.Length & 15) != 0)
            throw new InvalidDataException("Invalid original engine ADPCM length or pitch.");
        int starts = 0;
        for (int i = 0; i < bytes.Length; i += 16)
        {
            byte predictor = bytes[i], flag = bytes[i + 1];
            bool last = i + 16 == bytes.Length;
            if ((predictor >> 4) > 4 || (predictor & 15) > 12 || flag > 7 ||
                (last ? flag != 3 : (flag & 1) != 0))
                throw new InvalidDataException("Invalid original engine ADPCM block or loop end.");
            if ((flag & 4) != 0) starts++;
        }
        if (starts != 1)
            throw new InvalidDataException("An original engine sample must have exactly one loop start.");
    }

    internal static bool MatchesHash(ReadOnlySpan<byte> bytes, string hex) =>
        CryptographicOperations.FixedTimeEquals(SHA256.HashData(bytes), Convert.FromHexString(hex));

    readonly record struct Chunk(int Offset, int Length)
    {
        internal int End => Offset + Length;
    }

    static Chunk Take(ReadOnlySpan<byte> data, ref int cursor, int bound, ReadOnlySpan<byte> tag)
    {
        if (bound - cursor < 8 || !data.Slice(cursor, 4).SequenceEqual(tag))
            throw new InvalidDataException("Missing or unexpected original engine IFF chunk.");
        uint size = BinaryPrimitives.ReadUInt32BigEndian(data[(cursor + 4)..]);
        int start = cursor + 8;
        // Include the IFF word pad in bounds; it is not part of the payload.
        if ((ulong)size + (size & 1u) > (uint)(bound - start))
            throw new InvalidDataException("Original engine IFF chunk exceeds its parent.");
        cursor = start + (int)size + (int)(size & 1u);
        return new(start, (int)size);
    }
}
