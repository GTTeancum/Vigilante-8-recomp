using System.Buffers.Binary;
using System.Text;

namespace RecompOne.Runtime.Sdk;

readonly record struct NativeVramAllocation(
    uint Width,
    uint Height,
    uint AlignWidth,
    uint AlignHeight,
    uint LimitWidth,
    uint LimitHeight);

/// <summary>
/// File-backed slice of one strict native XOBF bank. Package discovery keeps
/// only offsets resident; BIN/ANM payloads are read when the selected vehicle
/// is validated or built.
/// </summary>
sealed class NativeVehicleBankSource(
    string path,
    long binOffset,
    int binLength,
    long animationOffset,
    int animationLength)
{
    public const int MaximumSourceBytes = 256 * 1024;

    public byte[] ReadBin() => ReadSlice(path, binOffset, binLength);

    public byte[]? ReadAnimation() =>
        animationOffset < 0
            ? null
            : ReadSlice(path, animationOffset, animationLength);

    /// <summary>
    /// Returns the native PS1 allocator requests made by the retail XOBF
    /// texture loader. Keeping these reservations live while COMMON and the
    /// arena load prevents late guest banks from being starved by unrelated
    /// allocations. The payload remains file-backed and is discarded after
    /// this descriptor-only scan.
    /// </summary>
    public IReadOnlyList<NativeVramAllocation> ReadVramAllocations(
        bool palettesFirst)
    {
        byte[] data = ReadBin();
        RequireRange(data, 0, 0x1C, "XOBF BIN header");
        uint textureCount = U32(data, 0x10);
        uint textureTable = U32(data, 0x14);
        if (textureCount > 256)
            throw new InvalidDataException(
                $"native vehicle bank has {textureCount} textures; limit is 256");
        RequireRange(
            data, textureTable, checked(textureCount * 4u),
            "XOBF texture table");

        var palettes = new List<NativeVramAllocation>();
        var images = new List<(int Index, NativeVramAllocation Request)>();
        for (uint index = 0; index < textureCount; index++)
        {
            uint descriptor = RelativeTarget(
                data, textureTable, index, "XOBF texture descriptor");
            RequireRange(data, descriptor, 0x14, "XOBF texture descriptor");
            uint flags = U32(data, descriptor + 4u);
            uint depth = flags & 3u;
            if (depth > 1u)
                continue;

            uint paletteWidth = U16(data, descriptor + 0x10u);
            uint paletteHeight = U16(data, descriptor + 0x12u);
            if (paletteWidth == 0u || paletteHeight == 0u ||
                paletteWidth * paletteHeight > 256u)
                throw new InvalidDataException(
                    $"texture {index} has an invalid native CLUT rectangle");
            palettes.Add(new NativeVramAllocation(
                paletteWidth, paletteHeight,
                16u, 1u, paletteWidth, 1u));

            uint image = checked(descriptor + U32(data, descriptor + 8u));
            RequireRange(data, image, 0x14, "XOBF texture image descriptor");
            uint widthWords = U16(data, image + 0x10u);
            uint height = U16(data, image + 0x12u);
            if (widthWords == 0u || height == 0u)
                throw new InvalidDataException(
                    $"texture {index} has an invalid native image rectangle");
            images.Add((
                checked((int)index),
                new NativeVramAllocation(
                    widthWords, height,
                    64u, 256u, 64u << checked((int)depth), 256u)));
        }

        var result = new List<NativeVramAllocation>(
            checked(palettes.Count + images.Count));
        if (palettesFirst)
        {
            result.AddRange(palettes);
            foreach (var image in images
                .OrderByDescending(item => item.Request.Height)
                .ThenBy(item => item.Index))
                result.Add(image.Request);
        }
        else
        {
            for (int index = 0; index < images.Count; index++)
            {
                result.Add(palettes[index]);
                result.Add(images[index].Request);
            }
        }
        return result;
    }

    public int SourceBytes => checked(binLength + animationLength);
    public int BinLength => binLength;

    public static List<NativeVehicleBankSource> Open(string archivePath)
    {
        string path = Path.GetFullPath(archivePath);
        using var stream = new FileStream(
            path, FileMode.Open, FileAccess.Read, FileShare.Read);
        var banks = new List<NativeVehicleBankSource>();
        var header = new byte[8];
        var formType = new byte[4];
        var childHeader = new byte[8];
        while (stream.Position < stream.Length)
        {
            long formStart = stream.Position;
            ReadExact(stream, header, "top-level IFF chunk");
            if (!header[..4].SequenceEqual("FORM"u8))
                throw new InvalidDataException(
                    $"CUSTOM.EXP contains a non-FORM chunk at 0x{formStart:X}");
            uint formSize = BinaryPrimitives.ReadUInt32BigEndian(header[4..]);
            long bodyStart = stream.Position;
            long formEnd = checked(bodyStart + formSize);
            if (formSize < 4 || formEnd > stream.Length)
                throw new InvalidDataException("XOBF form is truncated");

            ReadExact(stream, formType, "XOBF form type");
            if (!formType.SequenceEqual("XOBF"u8))
                throw new InvalidDataException(
                    $"CUSTOM.EXP FORM at 0x{formStart:X} is not XOBF");

            long binOffset = -1;
            int binLength = 0;
            long animationOffset = -1;
            int animationLength = 0;
            while (stream.Position < formEnd)
            {
                ReadExact(stream, childHeader, "XOBF child header");
                uint childSize =
                    BinaryPrimitives.ReadUInt32BigEndian(childHeader[4..]);
                if (childSize > int.MaxValue ||
                    stream.Position + childSize > formEnd)
                    throw new InvalidDataException("XOBF child is truncated");
                long payloadOffset = stream.Position;
                if (childHeader[..4].SequenceEqual("BIN "u8))
                {
                    if (binOffset >= 0)
                        throw new InvalidDataException(
                            "XOBF contains duplicate BIN chunks");
                    binOffset = payloadOffset;
                    binLength = (int)childSize;
                }
                else if (childHeader[..4].SequenceEqual("ANM "u8))
                {
                    if (animationOffset >= 0)
                        throw new InvalidDataException(
                            "XOBF contains duplicate ANM chunks");
                    animationOffset = payloadOffset;
                    animationLength = (int)childSize;
                }
                else
                {
                    throw new InvalidDataException(
                        "XOBF contains unsupported chunk " +
                        Encoding.ASCII.GetString(childHeader[..4]));
                }
                stream.Position = checked(
                    payloadOffset + childSize + (childSize & 1u));
            }
            if (stream.Position != formEnd || binOffset < 0)
                throw new InvalidDataException(
                    "XOBF has invalid padding or no BIN chunk");
            int sourceBytes = checked(binLength + animationLength);
            if (sourceBytes > MaximumSourceBytes)
                throw new InvalidDataException(
                    $"native vehicle bank uses {sourceBytes} source bytes; " +
                    $"limit is {MaximumSourceBytes}");
            banks.Add(new NativeVehicleBankSource(
                path,
                binOffset,
                binLength,
                animationOffset,
                animationLength));
            stream.Position = checked(formEnd + (formSize & 1u));
        }
        if (stream.Position != stream.Length || banks.Count == 0)
            throw new InvalidDataException(
                "CUSTOM.EXP is empty or has trailing data");
        return banks;
    }

    static byte[] ReadSlice(string path, long offset, int length)
    {
        using var stream = new FileStream(
            path, FileMode.Open, FileAccess.Read, FileShare.Read);
        stream.Position = offset;
        var data = new byte[length];
        ReadExact(stream, data, "native vehicle bank payload");
        return data;
    }

    static ushort U16(byte[] data, uint offset)
    {
        RequireRange(data, offset, 2, "native u16");
        return BinaryPrimitives.ReadUInt16LittleEndian(
            data.AsSpan(checked((int)offset), 2));
    }

    static uint U32(byte[] data, uint offset)
    {
        RequireRange(data, offset, 4, "native u32");
        return BinaryPrimitives.ReadUInt32LittleEndian(
            data.AsSpan(checked((int)offset), 4));
    }

    static uint RelativeTarget(
        byte[] data, uint table, uint index, string context)
    {
        uint entry = checked(table + index * 4u);
        return checked(table + U32(data, entry));
    }

    static void RequireRange(
        byte[] data, uint offset, uint size, string context)
    {
        uint length = checked((uint)data.Length);
        if (offset > length || size > length - offset)
            throw new InvalidDataException($"{context} is truncated");
    }

    static void ReadExact(Stream stream, Span<byte> destination, string context)
    {
        int read = 0;
        while (read < destination.Length)
        {
            int count = stream.Read(destination[read..]);
            if (count == 0)
                throw new InvalidDataException($"{context} is truncated");
            read += count;
        }
    }
}
