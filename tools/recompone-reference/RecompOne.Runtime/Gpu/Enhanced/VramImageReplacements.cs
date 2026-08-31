using System.Globalization;
using System.Numerics;
using System.Text.Json;

namespace RecompOne.Runtime.Enhanced;

/// <summary>
/// Optional, content-addressed replacements for complete CPU-to-VRAM images.
/// This covers decoded still images that never pass through the textured-
/// primitive replacement atlas. A manifest identifies each native upload by
/// its pixels, dimensions and destination, so no game address or screen name
/// is embedded in the renderer.
/// </summary>
internal static class VramImageReplacements
{
    readonly record struct UploadKey(
        uint Hash, int DestinationX, int DestinationY,
        int Width, int Height);
    sealed record DdsImage(int Width, int Height, byte[] Rgba);
    sealed record Replacement(string Mod, string Image, ushort[] Pixels);

    static readonly Lazy<Dictionary<UploadKey, Replacement>> s_entries =
        new(Load, LazyThreadSafetyMode.ExecutionAndPublication);
    static readonly HashSet<UploadKey> s_loggedHits = [];
    static readonly object s_logLock = new();

    public static ushort[]? Resolve(
        int destinationX, int destinationY, int width, int height,
        ReadOnlySpan<ushort> nativePixels)
    {
        Dictionary<UploadKey, Replacement> entries = s_entries.Value;
        if (entries.Count == 0 || width <= 0 || height <= 0 ||
            nativePixels.Length != width * height)
            return null;

        uint hash = Hash(nativePixels);
        var key = new UploadKey(
            hash, destinationX, destinationY, width, height);
        if (!entries.TryGetValue(key, out Replacement? replacement))
            return null;

        lock (s_logLock)
        {
            if (s_loggedHits.Add(key))
                Console.Error.WriteLine(
                    $"[VramImageMod] hit mod={replacement.Mod} " +
                    $"image={replacement.Image} fnv=0x{hash:X8} " +
                    $"xy={destinationX},{destinationY} " +
                    $"size={width}x{height}");
        }
        return replacement.Pixels;
    }

    static Dictionary<UploadKey, Replacement> Load()
    {
        var entries = new Dictionary<UploadKey, Replacement>();
        string mods = Runtime.ModsDirectory;
        if (!Directory.Exists(mods)) return entries;

        var manifests = new List<string>();
        string rootManifest = Path.Combine(mods, "manifest.json");
        if (File.Exists(rootManifest)) manifests.Add(rootManifest);
        manifests.AddRange(Directory.EnumerateDirectories(mods)
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .Select(path => Path.Combine(path, "manifest.json"))
            .Where(File.Exists));

        int loaded = 0;
        foreach (string manifest in manifests)
        {
            try
            {
                using JsonDocument document = JsonDocument.Parse(
                    File.ReadAllBytes(manifest));
                if (!document.RootElement.TryGetProperty(
                        "vramImages", out JsonElement groups) ||
                    groups.ValueKind != JsonValueKind.Array)
                    continue;

                string directory = Path.GetDirectoryName(manifest)!;
                string mod = document.RootElement.TryGetProperty(
                    "name", out JsonElement nameElement)
                    ? nameElement.GetString() ?? Path.GetFileName(directory)
                    : Path.GetFileName(directory);
                var images = new Dictionary<string, DdsImage>(
                    StringComparer.OrdinalIgnoreCase);

                foreach (JsonElement group in groups.EnumerateArray())
                {
                    string relativeImage =
                        group.GetProperty("image").GetString() ?? "";
                    string imagePath = ResolveInside(
                        directory, relativeImage);
                    if (!images.TryGetValue(imagePath, out DdsImage? image))
                    {
                        image = ReadDds(imagePath);
                        images.Add(imagePath, image);
                    }

                    foreach (JsonElement entry in group.GetProperty(
                                 "uploads").EnumerateArray())
                    {
                        string hashText =
                            entry.GetProperty("hash").GetString() ?? "";
                        if (!uint.TryParse(
                                hashText, NumberStyles.HexNumber,
                                CultureInfo.InvariantCulture, out uint hash))
                            throw new InvalidDataException(
                                $"Invalid VRAM-image hash: {hashText}");
                        int destinationX =
                            entry.GetProperty("destinationX").GetInt32();
                        int destinationY =
                            entry.GetProperty("destinationY").GetInt32();
                        int width = entry.GetProperty("width").GetInt32();
                        int height = entry.GetProperty("height").GetInt32();
                        int sourceX = entry.GetProperty("sourceX").GetInt32();
                        int sourceY = entry.GetProperty("sourceY").GetInt32();
                        ushort[] pixels = CropRgb555(
                            image, sourceX, sourceY, width, height);
                        var key = new UploadKey(
                            hash, destinationX, destinationY, width, height);
                        entries[key] = new Replacement(
                            mod, relativeImage.Replace('\\', '/'), pixels);
                        loaded++;
                    }
                }
            }
            catch (Exception error)
            {
                Console.Error.WriteLine(
                    $"[VramImageMod] rejected {manifest}: {error.Message}");
            }
        }

        if (loaded != 0)
            Console.Error.WriteLine(
                $"[VramImageMod] loaded {loaded} upload replacements " +
                $"from {manifests.Count} scanned manifests");
        return entries;
    }

    static string ResolveInside(string directory, string relative)
    {
        if (string.IsNullOrWhiteSpace(relative) || Path.IsPathRooted(relative))
            throw new InvalidDataException(
                "VRAM-image path must be relative to its mod");
        string root = Path.GetFullPath(directory)
            .TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
        string path = Path.GetFullPath(Path.Combine(directory, relative));
        if (!path.StartsWith(root, StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException(
                "VRAM-image path leaves its mod directory");
        return path;
    }

    static ushort[] CropRgb555(
        DdsImage image, int sourceX, int sourceY, int width, int height)
    {
        if (width <= 0 || height <= 0 || sourceX < 0 || sourceY < 0 ||
            sourceX + width > image.Width || sourceY + height > image.Height)
            throw new InvalidDataException(
                "VRAM-image crop is outside the DDS bounds");
        var output = new ushort[checked(width * height)];
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            int source = ((sourceY + y) * image.Width + sourceX + x) * 4;
            int red = (image.Rgba[source] * 31 + 127) / 255;
            int green = (image.Rgba[source + 1] * 31 + 127) / 255;
            int blue = (image.Rgba[source + 2] * 31 + 127) / 255;
            output[y * width + x] =
                (ushort)(red | green << 5 | blue << 10);
        }
        return output;
    }

    static uint Hash(ReadOnlySpan<ushort> pixels)
    {
        uint hash = 2166136261u;
        foreach (ushort pixel in pixels)
        {
            hash = (hash ^ (byte)pixel) * 16777619u;
            hash = (hash ^ (byte)(pixel >> 8)) * 16777619u;
        }
        return hash;
    }

    static DdsImage ReadDds(string path)
    {
        byte[] bytes = File.ReadAllBytes(path);
        if (bytes.Length < 128 || Read32(bytes, 0) != 0x20534444u ||
            Read32(bytes, 4) != 124u || Read32(bytes, 76) != 32u)
            throw new InvalidDataException(
                $"Invalid DDS texture: {Path.GetFileName(path)}");
        int height = checked((int)Read32(bytes, 12));
        int width = checked((int)Read32(bytes, 16));
        int pitch = checked((int)Read32(bytes, 20));
        uint fourCc = Read32(bytes, 84);
        int bits = checked((int)Read32(bytes, 88));
        uint redMask = Read32(bytes, 92);
        uint greenMask = Read32(bytes, 96);
        uint blueMask = Read32(bytes, 100);
        uint alphaMask = Read32(bytes, 104);
        if (width <= 0 || height <= 0 || width > 4096 || height > 4096 ||
            fourCc != 0 || bits != 32 || pitch < width * 4 ||
            bytes.Length < 128L + (long)pitch * height)
            throw new InvalidDataException(
                $"DDS must be uncompressed 32-bit RGBA: " +
                Path.GetFileName(path));
        byte[] rgba = new byte[checked(width * height * 4)];
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            uint pixel = Read32(bytes, 128 + y * pitch + x * 4);
            int output = (y * width + x) * 4;
            rgba[output] = Channel(pixel, redMask, 0);
            rgba[output + 1] = Channel(pixel, greenMask, 0);
            rgba[output + 2] = Channel(pixel, blueMask, 0);
            rgba[output + 3] = Channel(pixel, alphaMask, 255);
        }
        return new DdsImage(width, height, rgba);
    }

    static uint Read32(byte[] bytes, int offset) =>
        (uint)(bytes[offset] |
               bytes[offset + 1] << 8 |
               bytes[offset + 2] << 16 |
               bytes[offset + 3] << 24);

    static byte Channel(uint pixel, uint mask, byte absent)
    {
        if (mask == 0) return absent;
        int shift = BitOperations.TrailingZeroCount(mask);
        uint maximum = mask >> shift;
        uint value = (pixel & mask) >> shift;
        return (byte)((value * 255u + maximum / 2u) / maximum);
    }
}
