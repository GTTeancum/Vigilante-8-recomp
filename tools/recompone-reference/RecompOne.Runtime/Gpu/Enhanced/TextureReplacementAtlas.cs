using System.Globalization;
using System.Numerics;
using System.Text.Json;
using RecompOne.Runtime.Hle;
using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

/// <summary>
/// Content-addressed high-resolution texture replacements for the Enhanced
/// renderer. PS1 texture pages are decoded on the CPU with their active CLUT;
/// the decoded texels, dimensions and transparency/STP bits form the key.
/// This keeps replacements stable when the retail VRAM allocator moves an
/// image and makes a missing replacement an exact per-primitive fallback.
/// </summary>
internal sealed class TextureReplacementAtlas : IDisposable
{
    internal readonly record struct Rect(float X, float Y, float W, float H)
    {
        public bool Valid => W > 0f && H > 0f;
        public Vector3 ColorScale { get; init; } = Vector3.One;
        public Vector3 ColorBias { get; init; } = Vector3.Zero;
    }

    readonly record struct Signature(
        int TPage, int Clut,
        int MinU, int MinV, int MaxU, int MaxV,
        int TwAndX, int TwAndY, int TwOrX, int TwOrY,
        HleMaterialKind Material);
    readonly record struct Cached(ulong Revision, Rect Rect, ulong Key);
    readonly record struct PendingEntry(
        ulong Key, string Image, int X, int Y, int Width, int Height,
        string Source);
    readonly record struct EntryInfo(
        string Image, int X, int Y, int Width, int Height, string Source);
    readonly record struct PendingTerrainAtlas(
        string Name, string Image, int Width, int Height,
        int ImageX, int ImageY, int Depth, ulong IndexHash,
        byte[] Indices, ushort[] Palette);
    readonly record struct PendingFontFile(
        string Path, string Image, int SourceWidth, int SourceHeight,
        int UploadWidthWords, int UploadHeight);
    readonly record struct TerrainTileKey(int Width, int Height, ulong Hash);
    readonly record struct TerrainAnchor(
        int LiveX, int LiveY, int SourceX, int SourceY);
    sealed record LooseImage(string Name, int Width, int Height, byte[] Rgba);
    sealed class TerrainAtlas
    {
        public required string Name { get; init; }
        public required LooseImage Image { get; init; }
        public required int SourceWidth { get; init; }
        public required int SourceHeight { get; init; }
        public required int ImageX { get; init; }
        public required int ImageY { get; init; }
        public required int Depth { get; init; }
        public required ulong IndexHash { get; init; }
        public required byte[] SourceIndices { get; init; }
        public required ushort[] Palette { get; init; }
        public required int PackedX { get; init; }
        public required int PackedY { get; init; }
        public required Dictionary<TerrainTileKey, (int X, int Y)> Tiles { get; init; }
        public List<TerrainAnchor> Anchors { get; } = [];
    }
    sealed class FileFont
    {
        public required string Path { get; init; }
        public required LooseImage Image { get; init; }
        public required int SourceWidth { get; init; }
        public required int SourceHeight { get; init; }
        public required int UploadWidthWords { get; init; }
        public required int UploadHeight { get; init; }
        public required int PackedX { get; init; }
        public required int PackedY { get; init; }
    }

    readonly GL _gl;
    readonly ushort[] _native = new ushort[VramShadow.Width * VramShadow.Height];
    readonly ulong[] _blockRevision = new ulong[16 * 8];
    readonly Dictionary<Signature, Cached> _cache = [];
    readonly Dictionary<ulong, Rect> _entries = [];
    readonly Dictionary<ulong, EntryInfo> _entryInfo = [];
    readonly Dictionary<ulong, string> _entryShadows = [];
    readonly HashSet<ulong> _routeEntries = [];
    readonly List<TerrainAtlas> _terrainAtlases = [];
    readonly List<FileFont> _fileFonts = [];
    readonly HashSet<string> _loggedFileFontHits =
        new(StringComparer.OrdinalIgnoreCase);
    readonly HashSet<string> _loggedFileFontFallbacks =
        new(StringComparer.OrdinalIgnoreCase);
    TerrainAtlas? _activeTerrainAtlas;
    readonly HashSet<ulong> _dumped = [];
    readonly HashSet<ulong> _loggedTerrain = [];
    readonly string? _dumpDirectory;
    readonly bool _dumpTerrainOnly;
    readonly bool _traceTerrainAtlasFragments;
    int _terrainAtlasCatalogTraceCount;
    int _terrainAtlasAnchorTraceCount;
    int _terrainAtlasMissTraceCount;
    long _resolves, _hits;
    long _terrainResolves, _terrainAtlasHits;
    long _terrainAtlasCatalogHits, _terrainAtlasAnchorHits;
    long _terrainAtlasMisses;
    ulong _revision = 1;
    uint _texture;
    int _width = 1, _height = 1;

    public uint Texture => _texture;
    public int Width => _width;
    public int Height => _height;
    public int Count => _entries.Count + _terrainAtlases.Count + _fileFonts.Count;

    public TextureReplacementAtlas(GL gl)
    {
        _gl = gl;
        _dumpDirectory = Environment.GetEnvironmentVariable(
            "RECOMPONE_TEXTURE_DUMP_DIR");
        _dumpTerrainOnly = Environment.GetEnvironmentVariable(
            "RECOMPONE_TEXTURE_DUMP_TERRAIN_ONLY") == "1";
        _traceTerrainAtlasFragments = Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TERRAIN_ATLAS_FRAGMENTS") == "1";
        if (!string.IsNullOrWhiteSpace(_dumpDirectory))
            Directory.CreateDirectory(Path.GetFullPath(_dumpDirectory));
        Load();
    }

    void Load()
    {
        string executableRoot = Runtime.ExecutableDirectory;
        string looseRoot = Runtime.ResolveLoosePath() ?? executableRoot;
        string? overrideDirectory = Environment.GetEnvironmentVariable(
            "RECOMPONE_TEXTURE_PACK_DIR");
        string modsDirectory = Path.Combine(
            Runtime.ModsDirectory, "enhanced_textures_2x");
        string legacyDirectory = Path.Combine(looseRoot, "textures", "2x");
        string directory = overrideDirectory ??
            (File.Exists(Path.Combine(modsDirectory, "manifest.json"))
                ? modsDirectory
                : legacyDirectory);
        directory = Path.GetFullPath(directory);
        string manifestPath = Path.Combine(directory, "manifest.json");
        if (!File.Exists(manifestPath))
        {
            Console.WriteLine("[TexturePack] no loose 2x DDS pack found");
            return;
        }

        using JsonDocument document = JsonDocument.Parse(
            File.ReadAllBytes(manifestPath));
        JsonElement rootElement = document.RootElement;
        int format = rootElement.GetProperty("format").GetInt32();
        if (format is not (2 or 3))
            throw new InvalidDataException(
                "Texture replacement manifest is not loose DDS format 2 or 3");
        var sourceLabels = new Dictionary<ulong, string>();
        if (rootElement.TryGetProperty("sources", out JsonElement sources))
        foreach (JsonProperty source in sources.EnumerateObject())
        {
            if (!ulong.TryParse(
                    source.Name, NumberStyles.HexNumber,
                    CultureInfo.InvariantCulture, out ulong key))
                continue;
            sourceLabels[key] = source.Value.ValueKind == JsonValueKind.Array
                ? string.Join(" | ", source.Value.EnumerateArray().Select(
                    value => value.GetString() ?? ""))
                : source.Value.ToString();
        }
        var pending = new List<PendingEntry>();
        foreach (JsonElement entry in rootElement.GetProperty("entries").EnumerateArray())
        {
            string keyText = entry.GetProperty("key").GetString() ?? "";
            if (!ulong.TryParse(
                    keyText, NumberStyles.HexNumber,
                    CultureInfo.InvariantCulture, out ulong key))
                throw new InvalidDataException(
                    $"Texture replacement key is not hexadecimal: {keyText}");
            pending.Add(new PendingEntry(
                key,
                entry.GetProperty("image").GetString() ?? "",
                entry.GetProperty("x").GetInt32(),
                entry.GetProperty("y").GetInt32(),
                entry.GetProperty("width").GetInt32(),
                entry.GetProperty("height").GetInt32(),
                sourceLabels.GetValueOrDefault(key, "unlabelled")));
            if (pending[^1].Image.StartsWith(
                    "images/route/", StringComparison.OrdinalIgnoreCase))
                _routeEntries.Add(key);
        }
        var pendingTerrain = new List<PendingTerrainAtlas>();
        if (rootElement.TryGetProperty("terrainAtlases", out JsonElement terrain))
        foreach (JsonElement entry in terrain.EnumerateArray())
        {
            string hashText = entry.GetProperty("indexHash").GetString() ?? "";
            if (!ulong.TryParse(
                    hashText, NumberStyles.HexNumber,
                    CultureInfo.InvariantCulture, out ulong indexHash))
                throw new InvalidDataException(
                    $"Terrain atlas index hash is not hexadecimal: {hashText}");
            ushort[] palette = entry.GetProperty("palette")
                .EnumerateArray()
                .Select(value =>
                {
                    string text = value.GetString() ?? "";
                    if (!ushort.TryParse(
                            text, NumberStyles.HexNumber,
                            CultureInfo.InvariantCulture, out ushort pixel))
                        throw new InvalidDataException(
                            $"Terrain palette value is not hexadecimal: {text}");
                    return pixel;
                })
                .ToArray();
            if (palette.Length != 256)
                throw new InvalidDataException(
                    "Terrain atlas palette must contain 256 colors");
            pendingTerrain.Add(new PendingTerrainAtlas(
                entry.GetProperty("name").GetString() ?? "terrain",
                entry.GetProperty("image").GetString() ?? "",
                entry.GetProperty("width").GetInt32(),
                entry.GetProperty("height").GetInt32(),
                entry.GetProperty("imageX").GetInt32(),
                entry.GetProperty("imageY").GetInt32(),
                entry.GetProperty("depth").GetInt32(),
                indexHash,
                Convert.FromBase64String(
                    entry.GetProperty("indices").GetString() ?? ""),
                palette));
        }
        var pendingFonts = new List<PendingFontFile>();
        if (rootElement.TryGetProperty("fontFiles", out JsonElement fontFiles))
        foreach (JsonElement entry in fontFiles.EnumerateArray())
        {
            string path = FontFileProvenance.Normalize(
                entry.GetProperty("path").GetString() ?? "");
            int uploadWidthWords = entry.TryGetProperty(
                "uploadWidthWords", out JsonElement uploadWidthElement)
                ? uploadWidthElement.GetInt32() : 0;
            int uploadHeight = entry.TryGetProperty(
                "uploadHeight", out JsonElement uploadHeightElement)
                ? uploadHeightElement.GetInt32() : 0;
            if ((uploadWidthWords == 0) != (uploadHeight == 0) ||
                uploadWidthWords < 0 || uploadHeight < 0)
                throw new InvalidDataException(
                    $"Font replacement has an invalid upload extent: {path}");
            pendingFonts.Add(new PendingFontFile(
                path,
                entry.GetProperty("image").GetString() ?? "",
                entry.GetProperty("sourceWidth").GetInt32(),
                entry.GetProperty("sourceHeight").GetInt32(),
                uploadWidthWords,
                uploadHeight));
            FontFileProvenance.RegisterTrackedPath(path);
        }

        string directoryPrefix = directory.TrimEnd(
            Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) +
            Path.DirectorySeparatorChar;
        var images = new Dictionary<string, LooseImage>(
            StringComparer.OrdinalIgnoreCase);
        foreach (string relative in pending.Select(e => e.Image)
                     .Concat(pendingTerrain.Select(e => e.Image))
                     .Concat(pendingFonts.Select(e => e.Image))
                     .Distinct(StringComparer.OrdinalIgnoreCase))
        {
            string normalized = relative.Replace(
                '/', Path.DirectorySeparatorChar);
            string path = Path.GetFullPath(Path.Combine(directory, normalized));
            if (!path.StartsWith(directoryPrefix, StringComparison.OrdinalIgnoreCase))
                throw new InvalidDataException(
                    $"Texture replacement escapes its mod directory: {relative}");
            images[relative] = ReadDds(path, relative);
        }

        Dictionary<string, (int X, int Y)>? placements = null;
        int atlasSize = 1024;
        while (atlasSize <= 16384 && placements == null)
        {
            placements = TryLayout(images.Values, atlasSize, padding: 2);
            if (placements == null)
                atlasSize = atlasSize < 8192
                    ? atlasSize * 2
                    : atlasSize + 1024;
        }
        if (placements == null)
            throw new InvalidDataException(
                "Loose texture replacements exceed a 16384x16384 GPU atlas");
        _width = _height = atlasSize;
        byte[] rgba = new byte[checked(_width * _height * 4)];
        foreach (LooseImage image in images.Values)
        {
            (int x, int y) = placements[image.Name];
            BlitWithGutter(rgba, _width, image, x, y, padding: 2);
        }
        foreach (PendingEntry entry in pending)
        {
            LooseImage image = images[entry.Image];
            if (entry.Width <= 0 || entry.Height <= 0 ||
                entry.X < 0 || entry.Y < 0 ||
                entry.X + entry.Width > image.Width ||
                entry.Y + entry.Height > image.Height)
                throw new InvalidDataException(
                    $"Texture replacement crop is outside {entry.Image}");
            (int x, int y) = placements[entry.Image];
            if (_entryInfo.TryGetValue(entry.Key, out EntryInfo previous))
            {
                string relation =
                    entry.Width * entry.Height < previous.Width * previous.Height
                        ? "lower-resolution-winner"
                        : entry.Width * entry.Height > previous.Width * previous.Height
                            ? "higher-resolution-winner"
                            : "equal-resolution-winner";
                _entryShadows[entry.Key] =
                    $"{relation}:previous={previous.Image}@" +
                    $"{previous.Width}x{previous.Height}:winner=" +
                    $"{entry.Image}@{entry.Width}x{entry.Height}";
            }
            _entries[entry.Key] = new Rect(
                x + entry.X, y + entry.Y,
                entry.Width, entry.Height);
            _entryInfo[entry.Key] = new EntryInfo(
                entry.Image, entry.X, entry.Y,
                entry.Width, entry.Height, entry.Source);
        }
        foreach (PendingTerrainAtlas entry in pendingTerrain)
        {
            LooseImage image = images[entry.Image];
            int bytesPerPixel = entry.Depth == 2 ? 2 : 1;
            int tileSize = entry.Height / 4;
            if (entry.Width <= 0 || entry.Height <= 0 ||
                entry.Depth is < 0 or > 2 ||
                entry.Height % 4 != 0 ||
                tileSize <= 0 || entry.Width % tileSize != 0 ||
                entry.Indices.Length !=
                    entry.Width * entry.Height * bytesPerPixel ||
                image.Width % entry.Width != 0 ||
                image.Height % entry.Height != 0 ||
                image.Width / entry.Width != image.Height / entry.Height)
                throw new InvalidDataException(
                    $"Terrain DDS has an invalid integer scale: {entry.Image}");
            (int x, int y) = placements[entry.Image];
            _terrainAtlases.Add(new TerrainAtlas {
                Name = entry.Name,
                Image = image,
                SourceWidth = entry.Width,
                SourceHeight = entry.Height,
                ImageX = entry.ImageX,
                ImageY = entry.ImageY,
                Depth = entry.Depth,
                IndexHash = entry.IndexHash,
                SourceIndices = entry.Indices,
                Palette = entry.Palette,
                PackedX = x,
                PackedY = y,
                Tiles = BuildTerrainTiles(
                    entry.Indices, entry.Width, entry.Height,
                    entry.Depth, tileSize),
            });
        }
        foreach (PendingFontFile entry in pendingFonts)
        {
            LooseImage image = images[entry.Image];
            if (entry.SourceWidth <= 0 || entry.SourceHeight <= 0 ||
                image.Width % entry.SourceWidth != 0 ||
                image.Height % entry.SourceHeight != 0 ||
                image.Width / entry.SourceWidth !=
                    image.Height / entry.SourceHeight)
                throw new InvalidDataException(
                    $"Font DDS has an invalid integer scale: {entry.Image}");
            (int x, int y) = placements[entry.Image];
            _fileFonts.Add(new FileFont {
                Path = entry.Path,
                Image = image,
                SourceWidth = entry.SourceWidth,
                SourceHeight = entry.SourceHeight,
                UploadWidthWords = entry.UploadWidthWords,
                UploadHeight = entry.UploadHeight,
                PackedX = x,
                PackedY = y,
            });
        }

        _texture = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, _texture);
        _gl.TexParameter(
            TextureTarget.Texture2D,
            TextureParameterName.TextureMinFilter,
            (int)GLEnum.Linear);
        _gl.TexParameter(
            TextureTarget.Texture2D,
            TextureParameterName.TextureMagFilter,
            (int)GLEnum.Linear);
        _gl.TexParameter(
            TextureTarget.Texture2D,
            TextureParameterName.TextureWrapS,
            (int)GLEnum.ClampToEdge);
        _gl.TexParameter(
            TextureTarget.Texture2D,
            TextureParameterName.TextureWrapT,
            (int)GLEnum.ClampToEdge);
        _gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
        _gl.TexImage2D<byte>(
            TextureTarget.Texture2D, 0, InternalFormat.Rgba8,
            (uint)_width, (uint)_height, 0,
            PixelFormat.Rgba, PixelType.UnsignedByte, rgba);
        Console.WriteLine(
            $"[TexturePack] loaded {images.Count} loose DDS files / " +
            $"{_entries.Count} regions / {_routeEntries.Count} route regions / " +
            $"{_terrainAtlases.Count} terrain atlases / " +
            $"{_fileFonts.Count} file-font atlases " +
            $"from {directory} " +
            $"runtime-atlas={_width}x{_height}");
        Console.WriteLine(
            $"[TexturePack] manifest key shadows={_entryShadows.Count}");
    }

    public Rect ResolveFontFile(
        int tpage,
        int minU, int minV, int maxU, int maxV,
        int twAndX, int twAndY, int twOrX, int twOrY,
        out string sourcePath, out bool recognized)
    {
        sourcePath = "";
        recognized = false;
        if (!FontFileProvenance.TryResolve(
                tpage, minU, minV, maxU, maxV,
                twAndX, twAndY, twOrX, twOrY,
                out string path, out int x, out int y,
                out int width, out int height,
                out int sourceWidthWords, out int sourceHeight))
            return default;
        recognized = true;
        sourcePath = path;
        FileFont? font = _fileFonts.FirstOrDefault(candidate =>
            candidate.Path.Equals(path, StringComparison.OrdinalIgnoreCase) &&
            (candidate.UploadWidthWords == 0 ||
             candidate.UploadWidthWords == sourceWidthWords) &&
            (candidate.UploadHeight == 0 ||
             candidate.UploadHeight == sourceHeight));
        if (font == null)
        {
            string identity = $"{path}:{sourceWidthWords}x{sourceHeight}";
            if (_loggedFileFontFallbacks.Add(identity))
                Console.WriteLine(
                    $"[FontFiles] fallback original path={path} " +
                    $"upload={sourceWidthWords}x{sourceHeight} " +
                    "reason=no-hd-sidecar-for-subresource");
            return default;
        }
        if (x < 0 || y < 0 || x + width > font.SourceWidth ||
            y + height > font.SourceHeight)
        {
            if (_loggedFileFontFallbacks.Add(path + ":bounds"))
                Console.WriteLine(
                    $"[FontFiles] fallback original path={path} " +
                    $"reason=crop-outside-source crop={x},{y},{width},{height} " +
                    $"source={font.SourceWidth}x{font.SourceHeight}");
            return default;
        }
        int scale = font.Image.Width / font.SourceWidth;
        string hitIdentity =
            $"{path}:{font.UploadWidthWords}x{font.UploadHeight}";
        if (_loggedFileFontHits.Add(hitIdentity))
            Console.WriteLine(
                $"[FontFiles] HD replacement path={path} " +
                $"upload={sourceWidthWords}x{sourceHeight} " +
                $"source={font.SourceWidth}x{font.SourceHeight} " +
                $"dds={font.Image.Width}x{font.Image.Height} scale={scale}x");
        return new Rect(
            font.PackedX + x * scale,
            font.PackedY + y * scale,
            width * scale,
            height * scale);
    }

    public string DescribeResolution(
        ulong key, int sourceWidth, int sourceHeight)
    {
        if (_entryInfo.TryGetValue(key, out EntryInfo info))
        {
            float scaleX = info.Width / (float)Math.Max(1, sourceWidth);
            float scaleY = info.Height / (float)Math.Max(1, sourceHeight);
            string shadow = _entryShadows.GetValueOrDefault(key, "none");
            return
                $"kind=entry image=\"{info.Image}\" " +
                $"source-label=\"{info.Source.Replace("\"", "'")}\" " +
                $"source-crop={info.X},{info.Y},{info.Width},{info.Height} " +
                $"replacement-scale={scaleX:F3}x{scaleY:F3} " +
                $"shadow=\"{shadow}\"";
        }

        TerrainAtlas? terrain = _terrainAtlases.FirstOrDefault(
            atlas => atlas.IndexHash == key);
        if (terrain != null)
        {
            float scale = terrain.Image.Width / (float)terrain.SourceWidth;
            return
                $"kind=terrain-atlas image=\"{terrain.Image.Name}\" " +
                $"source-label=\"{terrain.Name}\" " +
                $"source-atlas={terrain.SourceWidth}x{terrain.SourceHeight} " +
                $"replacement-atlas={terrain.Image.Width}x{terrain.Image.Height} " +
                $"replacement-scale={scale:F3}x{scale:F3} shadow=\"none\"";
        }

        return key == 0
            ? "kind=none source-label=\"none\" shadow=\"none\""
            : $"kind=unmatched key={key:x16} source-label=\"none\" shadow=\"none\"";
    }

    static Dictionary<TerrainTileKey, (int X, int Y)> BuildTerrainTiles(
        byte[] indices, int width, int height, int depth, int tileSize)
    {
        var result = new Dictionary<TerrainTileKey, (int X, int Y)>();
        // V8:2 maps use complete 40x40 cells. Original-V8 maps normally use
        // 48x48 cells, but Valley Farm also addresses aligned 24x24 quadrants
        // from those cells. Catalog both authored cell sizes so the runtime
        // resolves the actual indexed tile instead of falling back to a
        // duplicated per-polygon texture.
        int minimumSize = tileSize % 2 == 0 ? tileSize / 2 : tileSize;
        for (int size = tileSize; size >= minimumSize; size /= 2)
        {
        for (int y = 0; y + size <= height; y += size)
        for (int x = 0; x + size <= width; x += size)
        {
            ulong hash = HashSourceIndices(
                indices, width, depth, x, y, size, size);
            result.TryAdd(
                new TerrainTileKey(size, size, hash), (x, y));
        }
        if (size == 1) break;
        }
        return result;
    }

    static ulong HashSourceIndices(
        byte[] indices, int sourceWidth, int depth,
        int x0, int y0, int width, int height)
    {
        int bytesPerPixel = depth == 2 ? 2 : 1;
        ulong hash = 14695981039346656037UL;
        AddHash(ref hash, (byte)width);
        AddHash(ref hash, (byte)(width >> 8));
        AddHash(ref hash, (byte)height);
        AddHash(ref hash, (byte)(height >> 8));
        for (int y = 0; y < height; y++)
        {
            int source = ((y0 + y) * sourceWidth + x0) * bytesPerPixel;
            for (int byteIndex = 0;
                 byteIndex < width * bytesPerPixel;
                 byteIndex++)
                AddHash(ref hash, indices[source + byteIndex]);
        }
        return hash;
    }

    static Dictionary<string, (int X, int Y)>? TryLayout(
        IEnumerable<LooseImage> source, int size, int padding)
    {
        var result = new Dictionary<string, (int X, int Y)>(
            StringComparer.OrdinalIgnoreCase);
        int x = 0, y = 0, rowHeight = 0;
        foreach (LooseImage image in source
                     .OrderByDescending(image => image.Height)
                     .ThenByDescending(image => image.Width)
                     .ThenBy(image => image.Name, StringComparer.Ordinal))
        {
            int packedWidth = image.Width + padding * 2;
            int packedHeight = image.Height + padding * 2;
            if (packedWidth > size || packedHeight > size)
                return null;
            if (x + packedWidth > size)
            {
                x = 0;
                y += rowHeight;
                rowHeight = 0;
            }
            if (y + packedHeight > size)
                return null;
            result[image.Name] = (x + padding, y + padding);
            x += packedWidth;
            rowHeight = Math.Max(rowHeight, packedHeight);
        }
        return result;
    }

    static void BlitWithGutter(
        byte[] atlas, int atlasWidth, LooseImage image,
        int innerX, int innerY, int padding)
    {
        for (int y = -padding; y < image.Height + padding; y++)
        for (int x = -padding; x < image.Width + padding; x++)
        {
            int sx = Math.Clamp(x, 0, image.Width - 1);
            int sy = Math.Clamp(y, 0, image.Height - 1);
            int source = (sy * image.Width + sx) * 4;
            int target = ((innerY + y) * atlasWidth + innerX + x) * 4;
            System.Buffer.BlockCopy(image.Rgba, source, atlas, target, 4);
        }
    }

    static LooseImage ReadDds(string path, string name)
    {
        byte[] bytes = File.ReadAllBytes(path);
        if (bytes.Length < 128 || Read32(bytes, 0) != 0x20534444u ||
            Read32(bytes, 4) != 124u || Read32(bytes, 76) != 32u)
            throw new InvalidDataException($"Invalid DDS texture: {name}");
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
                $"DDS must be uncompressed 32-bit RGBA: {name}");
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
        return new LooseImage(name, width, height, rgba);
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

    public Rect Resolve(
        int tpage, int clut,
        int minU, int minV, int maxU, int maxV,
        int twAndX, int twAndY, int twOrX, int twOrY,
        HleMaterialKind material,
        out ulong textureKey)
    {
        textureKey = 0;
        if ((tpage & 0x180) == 0x180 ||
            minU < 0 || minV < 0 || maxU > 255 || maxV > 255 ||
            minU > maxU || minV > maxV)
            return default;
        var signature = new Signature(
            tpage & 0x1FF, clut & 0x7FFF,
            minU, minV, maxU, maxV,
            twAndX, twAndY, twOrX, twOrY,
            material);
        ulong regionRevision = RevisionOf(signature);
        if (_cache.TryGetValue(signature, out Cached cached) &&
            cached.Revision == regionRevision)
        {
            textureKey = cached.Key;
            return cached.Rect;
        }

        bool terrain = material == HleMaterialKind.TerrainRoute;
        if (terrain) _terrainResolves++;
        byte[]? decoded = null;
        ulong hash = 0;
        int width = maxU - minU + 1;
        int height = maxV - minV + 1;
        if (terrain && _routeEntries.Count != 0)
        {
            decoded = Decode(signature);
            hash = Hash(decoded, width, height);
            if (_routeEntries.Contains(hash) &&
                _entries.TryGetValue(hash, out Rect routeRect))
            {
                textureKey = hash;
                _resolves++;
                _hits++;
                _cache[signature] = new Cached(
                    regionRevision, routeRect, hash);
                if (_loggedTerrain.Count < 128 && _loggedTerrain.Add(hash))
                    Console.WriteLine(
                        $"[TexturePack] route DDS hit key={hash:x16} " +
                        $"size={width}x{height} " +
                        $"window={twAndX:x2},{twAndY:x2},{twOrX:x2},{twOrY:x2}");
                return routeRect;
            }
        }
        if (terrain && TryResolveTerrain(
                signature, out Rect terrainRect, out ulong terrainKey))
        {
            textureKey = terrainKey;
            _resolves++;
            _hits++;
            _terrainAtlasHits++;
            if ((_terrainResolves & 0xFF) == 0)
                LogTerrainAtlasCoverage();
            _cache[signature] = new Cached(
                regionRevision, terrainRect, terrainKey);
            if (_loggedTerrain.Count < 128 && _loggedTerrain.Add(terrainKey))
                Console.WriteLine(
                    $"[TexturePack] terrain atlas hit key={terrainKey:x16} " +
                    $"size={maxU - minU + 1}x{maxV - minV + 1} " +
                    $"window={twAndX:x2},{twAndY:x2},{twOrX:x2},{twOrY:x2}");
            return terrainRect;
        }

        byte[] rgba = decoded ?? Decode(signature);
        if (hash == 0)
            hash = Hash(rgba, width, height);
        textureKey = hash;
        _resolves++;
        Rect rect = _entries.TryGetValue(hash, out Rect found)
            ? found
            : default;
        if (rect.Valid) _hits++;
        if (material == HleMaterialKind.TerrainRoute &&
            _loggedTerrain.Count < 128 && _loggedTerrain.Add(hash))
            Console.WriteLine(
                $"[TexturePack] terrain {(rect.Valid ? "hit" : "miss")} " +
                $"key={hash:x16} size={width}x{height} " +
                $"window={twAndX:x2},{twAndY:x2},{twOrX:x2},{twOrY:x2}");
        if ((_resolves & 0x3FF) == 0)
        {
            Console.WriteLine(
                $"[TexturePack] runtime hits={_hits}/{_resolves} " +
                $"({(100.0 * _hits / _resolves):F1}%)");
            Console.WriteLine(
                $"[TexturePack] unique terrain atlas coverage=" +
                $"{_terrainAtlasHits}/{_terrainResolves} " +
                $"({(100.0 * _terrainAtlasHits /
                    Math.Max(1, _terrainResolves)):F1}%)");
        }
        _cache[signature] = new Cached(regionRevision, rect, hash);
        if (_dumpDirectory != null &&
            (!_dumpTerrainOnly || material == HleMaterialKind.TerrainRoute) &&
            _dumped.Add(hash))
            Dump(hash, width, height, rgba);
        return rect;
    }

    bool TryResolveTerrain(Signature s, out Rect rect, out ulong key)
    {
        rect = default;
        key = 0;
        int depth = (s.TPage >> 7) & 3;
        if (depth > 2) return false;
        int u0 = ((s.MinU & s.TwAndX) | s.TwOrX) & 0xFF;
        int v0 = ((s.MinV & s.TwAndY) | s.TwOrY) & 0xFF;
        int u1 = ((s.MaxU & s.TwAndX) | s.TwOrX) & 0xFF;
        int v1 = ((s.MaxV & s.TwAndY) | s.TwOrY) & 0xFF;
        if (u1 - u0 != s.MaxU - s.MinU ||
            v1 - v0 != s.MaxV - s.MinV)
            return false;
        int pixelsPerWord = depth == 0 ? 4 : depth == 1 ? 2 : 1;
        int pageX = (s.TPage & 0xF) * 64;
        int pageY = ((s.TPage >> 4) & 1) * 256;
        int globalX = pageX * pixelsPerWord + u0;
        int globalY = pageY + v0;
        int sourceWidth = u1 - u0 + 1;
        int sourceHeight = v1 - v0 + 1;
        ulong liveHash = HashNativeIndices(
            depth, globalX, globalY, sourceWidth, sourceHeight);
        var tileKey = new TerrainTileKey(
            sourceWidth, sourceHeight, liveHash);

        TerrainAtlas? match = null;
        (int X, int Y) tile = default;
        if (_activeTerrainAtlas is { } active &&
            active.Depth == depth &&
            active.Tiles.TryGetValue(tileKey, out tile))
            match = active;
        else
        {
            foreach (TerrainAtlas candidate in _terrainAtlases)
            {
                if (candidate.Depth == depth &&
                    candidate.Tiles.TryGetValue(tileKey, out tile))
                {
                    match = candidate;
                    break;
                }
            }
        }
        bool anchored = false;
        if (match == null && _activeTerrainAtlas is { } activeAnchor &&
            activeAnchor.Depth == depth &&
            TryResolveTerrainAnchor(
                activeAnchor,
                globalX, globalY,
                sourceWidth, sourceHeight,
                liveHash,
                out tile))
        {
            match = activeAnchor;
            anchored = true;
        }
        if (match == null)
        {
            foreach (TerrainAtlas candidate in _terrainAtlases)
            {
                if (candidate.Depth == depth &&
                    TryResolveTerrainAnchor(
                        candidate,
                        globalX, globalY,
                        sourceWidth, sourceHeight,
                        liveHash,
                        out tile))
                {
                    match = candidate;
                    anchored = true;
                    break;
                }
            }
        }
        if (match == null)
        {
            _terrainAtlasMisses++;
            if (_traceTerrainAtlasFragments &&
                _terrainAtlasMissTraceCount++ < 512)
                Console.WriteLine(
                    "[TerrainAtlasFragment] miss " +
                    $"depth={depth} live={globalX},{globalY} " +
                    $"size={sourceWidth}x{sourceHeight} " +
                    $"indexHash={liveHash:x16} anchors=" +
                    string.Join(',', _terrainAtlases.Select(
                        atlas => $"{atlas.Name}:{atlas.Anchors.Count}")));
            return false;
        }
        if (anchored)
            _terrainAtlasAnchorHits++;
        else
            _terrainAtlasCatalogHits++;
        if (!anchored)
        {
            var anchor = new TerrainAnchor(
                globalX, globalY, tile.X, tile.Y);
            if (!match.Anchors.Contains(anchor) && match.Anchors.Count < 64)
                match.Anchors.Add(anchor);
        }
        if (!ReferenceEquals(_activeTerrainAtlas, match))
        {
            _activeTerrainAtlas = match;
            Console.WriteLine(
                $"[TexturePack] active terrain atlas={match.Name} " +
                $"source={match.SourceWidth}x{match.SourceHeight} " +
                $"dds={match.Image.Width}x{match.Image.Height}");
        }
        int scale = match.Image.Width / match.SourceWidth;
        (Vector3 colorScale, Vector3 colorBias) =
            TerrainPaletteTransform(
                match, s, globalX, globalY, sourceWidth, sourceHeight);
        rect = new Rect(
            match.PackedX + tile.X * scale,
            match.PackedY + tile.Y * scale,
            sourceWidth * scale,
            sourceHeight * scale) {
            ColorScale = colorScale,
            ColorBias = colorBias,
        };
        key = match.IndexHash;
        if (_traceTerrainAtlasFragments &&
            (anchored
                ? _terrainAtlasAnchorTraceCount++ < 512
                : _terrainAtlasCatalogTraceCount++ < 128))
            Console.WriteLine(
                "[TerrainAtlasFragment] hit " +
                $"mode={(anchored ? "anchor" : "catalog")} " +
                $"atlas={match.Name} depth={depth} " +
                $"live={globalX},{globalY} source={tile.X},{tile.Y} " +
                $"size={sourceWidth}x{sourceHeight} " +
                $"rect={rect.X:F0},{rect.Y:F0},{rect.W:F0},{rect.H:F0} " +
                $"scale={colorScale.X:F4},{colorScale.Y:F4},{colorScale.Z:F4} " +
                $"bias={colorBias.X:F4},{colorBias.Y:F4},{colorBias.Z:F4} " +
                $"indexHash={liveHash:x16} anchors={match.Anchors.Count}");
        return true;
    }

    static bool TryResolveTerrainAnchor(
        TerrainAtlas atlas,
        int globalX,
        int globalY,
        int width,
        int height,
        ulong liveHash,
        out (int X, int Y) source)
    {
        foreach (TerrainAnchor anchor in atlas.Anchors)
        {
            int sourceX = globalX - anchor.LiveX + anchor.SourceX;
            int sourceY = globalY - anchor.LiveY + anchor.SourceY;
            if (sourceX < 0 || sourceY < 0 ||
                sourceX + width > atlas.SourceWidth ||
                sourceY + height > atlas.SourceHeight)
                continue;
            ulong sourceHash = HashSourceIndices(
                atlas.SourceIndices,
                atlas.SourceWidth,
                atlas.Depth,
                sourceX,
                sourceY,
                width,
                height);
            if (sourceHash != liveHash)
                continue;
            source = (sourceX, sourceY);
            return true;
        }
        source = default;
        return false;
    }

    ulong HashNativeIndices(
        int depth, int globalX, int globalY, int width, int height)
    {
        int pixelsPerWord = depth == 0 ? 4 : depth == 1 ? 2 : 1;
        ulong hash = 14695981039346656037UL;
        AddHash(ref hash, (byte)width);
        AddHash(ref hash, (byte)(width >> 8));
        AddHash(ref hash, (byte)height);
        AddHash(ref hash, (byte)(height >> 8));
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            ushort packed = Read(
                (globalX + x) / pixelsPerWord,
                globalY + y);
            if (depth == 0)
                AddHash(
                    ref hash,
                    (byte)((packed >> (((globalX + x) & 3) * 4)) & 0xF));
            else if (depth == 1)
                AddHash(
                    ref hash,
                    (byte)((packed >> (((globalX + x) & 1) * 8)) & 0xFF));
            else
            {
                AddHash(ref hash, (byte)packed);
                AddHash(ref hash, (byte)(packed >> 8));
            }
        }
        return hash;
    }

    (Vector3 Scale, Vector3 Bias) TerrainPaletteTransform(
        TerrainAtlas atlas, Signature s,
        int globalX, int globalY, int width, int height)
    {
        if (atlas.Depth == 2) return (Vector3.One, Vector3.Zero);
        int pixelsPerWord = atlas.Depth == 0 ? 4 : 2;
        int clutX = (s.Clut & 0x3F) * 16;
        int clutY = (s.Clut >> 6) & 0x1FF;
        var indices = new HashSet<int>();
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            ushort packed = Read(
                (globalX + x) / pixelsPerWord,
                globalY + y);
            int index = atlas.Depth == 0
                ? (packed >> (((globalX + x) & 3) * 4)) & 0xF
                : (packed >> (((globalX + x) & 1) * 8)) & 0xFF;
            indices.Add(index);
        }
        if (indices.Count == 0) return (Vector3.One, Vector3.Zero);
        var source = new List<Vector3>(indices.Count);
        var target = new List<Vector3>(indices.Count);
        foreach (int index in indices)
        {
            source.Add(ColorOf(atlas.Palette[index]));
            target.Add(ColorOf(Read(clutX + index, clutY)));
        }
        Vector3 scale = new(
            Fit(source, target, 0).Scale,
            Fit(source, target, 1).Scale,
            Fit(source, target, 2).Scale);
        Vector3 bias = new(
            Fit(source, target, 0).Bias,
            Fit(source, target, 1).Bias,
            Fit(source, target, 2).Bias);
        return (scale, bias);
    }

    static (float Scale, float Bias) Fit(
        List<Vector3> source, List<Vector3> target, int channel)
    {
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (int index = 0; index < source.Count; index++)
        {
            double x = channel == 0 ? source[index].X :
                channel == 1 ? source[index].Y : source[index].Z;
            double y = channel == 0 ? target[index].X :
                channel == 1 ? target[index].Y : target[index].Z;
            sx += x;
            sy += y;
            sxx += x * x;
            sxy += x * y;
        }
        double count = source.Count;
        double denominator = count * sxx - sx * sx;
        if (Math.Abs(denominator) < 1e-9)
            return (1f, (float)((sy - sx) / count));
        double scale = (count * sxy - sx * sy) / denominator;
        double bias = (sy - scale * sx) / count;
        return ((float)scale, (float)bias);
    }

    static Vector3 ColorOf(ushort pixel) => new(
        Expand5(pixel & 0x1F) / 255f,
        Expand5((pixel >> 5) & 0x1F) / 255f,
        Expand5((pixel >> 10) & 0x1F) / 255f);

    byte[] Decode(Signature s)
    {
        int width = s.MaxU - s.MinU + 1;
        int height = s.MaxV - s.MinV + 1;
        byte[] rgba = new byte[width * height * 4];
        int pageX = (s.TPage & 0xF) * 64;
        int pageY = ((s.TPage >> 4) & 1) * 256;
        int depth = (s.TPage >> 7) & 3;
        int clutX = (s.Clut & 0x3F) * 16;
        int clutY = (s.Clut >> 6) & 0x1FF;
        int output = 0;
        for (int y = s.MinV; y <= s.MaxV; y++)
        for (int x = s.MinU; x <= s.MaxU; x++, output += 4)
        {
            int u = ((x & s.TwAndX) | s.TwOrX) & 0xFF;
            int v = ((y & s.TwAndY) | s.TwOrY) & 0xFF;
            ushort pixel;
            if (depth == 0)
            {
                ushort packed = Read(pageX + (u >> 2), pageY + v);
                int index = (packed >> ((u & 3) * 4)) & 0xF;
                pixel = Read(clutX + index, clutY);
            }
            else if (depth == 1)
            {
                ushort packed = Read(pageX + (u >> 1), pageY + v);
                int index = (packed >> ((u & 1) * 8)) & 0xFF;
                pixel = Read(clutX + index, clutY);
            }
            else
            {
                pixel = Read(pageX + u, pageY + v);
            }
            rgba[output] = Expand5(pixel & 0x1F);
            rgba[output + 1] = Expand5((pixel >> 5) & 0x1F);
            rgba[output + 2] = Expand5((pixel >> 10) & 0x1F);
            rgba[output + 3] = (byte)((pixel & 0x8000) != 0 ? 255 : 0);
        }
        return rgba;
    }

    ulong RevisionOf(Signature s)
    {
        int pageX = (s.TPage & 0xF) * 64;
        int pageY = ((s.TPage >> 4) & 1) * 256;
        int depth = (s.TPage >> 7) & 3;
        int perWord = depth == 0 ? 4 : depth == 1 ? 2 : 1;
        int x0 = pageX + s.MinU / perWord;
        int x1 = pageX + s.MaxU / perWord;
        ulong revision = BlocksRevision(x0, pageY + s.MinV, x1, pageY + s.MaxV);
        if (depth < 2)
        {
            int clutX = (s.Clut & 0x3F) * 16;
            int clutY = (s.Clut >> 6) & 0x1FF;
            int colors = depth == 0 ? 16 : 256;
            revision = Math.Max(
                revision,
                BlocksRevision(clutX, clutY, clutX + colors - 1, clutY));
        }
        return revision;
    }

    void LogTerrainAtlasCoverage()
    {
        Console.WriteLine(
            $"[TexturePack] terrain atlas coverage=" +
            $"{_terrainAtlasHits}/{_terrainResolves} " +
            $"({(100.0 * _terrainAtlasHits /
                Math.Max(1, _terrainResolves)):F1}%) " +
            $"catalog={_terrainAtlasCatalogHits} " +
            $"anchor={_terrainAtlasAnchorHits} " +
            $"miss={_terrainAtlasMisses}");
    }

    ulong BlocksRevision(int x0, int y0, int x1, int y1)
    {
        ulong revision = 0;
        for (int by = y0 >> 6; by <= y1 >> 6; by++)
        for (int bx = x0 >> 6; bx <= x1 >> 6; bx++)
            revision = Math.Max(
                revision,
                _blockRevision[((by & 7) * 16) + (bx & 15)]);
        return revision;
    }

    public void Reset(ReadOnlySpan<ushort> pixels)
    {
        pixels.CopyTo(_native);
        Mark(0, 0, VramShadow.Width, VramShadow.Height);
        _cache.Clear();
    }

    public void Write(int x, int y, int width, int height, ReadOnlySpan<ushort> pixels)
    {
        if (width <= 0 || height <= 0) return;
        for (int row = 0; row < height; row++)
        for (int column = 0; column < width; column++)
            _native[(((y + row) & 511) * 1024) + ((x + column) & 1023)] =
                pixels[row * width + column];
        Mark(x, y, width, height);
    }

    public void Fill(int x, int y, int width, int height, ushort value)
    {
        if (width <= 0 || height <= 0) return;
        for (int row = 0; row < height; row++)
        for (int column = 0; column < width; column++)
            _native[(((y + row) & 511) * 1024) + ((x + column) & 1023)] = value;
        Mark(x, y, width, height);
    }

    public void Copy(int sx, int sy, int dx, int dy, int width, int height)
    {
        if (width <= 0 || height <= 0) return;
        ushort[] temporary = new ushort[width * height];
        for (int row = 0; row < height; row++)
        for (int column = 0; column < width; column++)
            temporary[row * width + column] = Read(sx + column, sy + row);
        Write(dx, dy, width, height, temporary);
    }

    void Mark(int x, int y, int width, int height)
    {
        ulong revision = ++_revision;
        for (int row = 0; row < height; row += 64)
        for (int column = 0; column < width; column += 64)
        {
            int bx = ((x + column) & 1023) >> 6;
            int by = ((y + row) & 511) >> 6;
            _blockRevision[by * 16 + bx] = revision;
        }
        int endBx = ((x + width - 1) & 1023) >> 6;
        int endBy = ((y + height - 1) & 511) >> 6;
        _blockRevision[endBy * 16 + endBx] = revision;
    }

    void Dump(ulong hash, int width, int height, byte[] rgba)
    {
        string stem = $"{hash:x16}_{width}x{height}";
        string path = Path.Combine(Path.GetFullPath(_dumpDirectory!), stem + ".rgba");
        if (!File.Exists(path))
            File.WriteAllBytes(path, rgba);
    }

    static ulong Hash(ReadOnlySpan<byte> rgba, int width, int height)
    {
        ulong hash = 14695981039346656037UL;
        static void Add(ref ulong h, byte value)
        {
            h ^= value;
            h *= 1099511628211UL;
        }
        Add(ref hash, (byte)width);
        Add(ref hash, (byte)(width >> 8));
        Add(ref hash, (byte)height);
        Add(ref hash, (byte)(height >> 8));
        foreach (byte value in rgba) Add(ref hash, value);
        return hash;
    }

    static void AddHash(ref ulong hash, byte value)
    {
        hash ^= value;
        hash *= 1099511628211UL;
    }

    ushort Read(int x, int y) =>
        _native[((y & 511) * 1024) + (x & 1023)];

    static byte Expand5(int value) => (byte)((value << 3) | (value >> 2));

    public void Dispose()
    {
        if (_entries.Count != 0 || _terrainAtlases.Count != 0)
        {
            Console.WriteLine(
                $"[TexturePack] resolve summary hits={_hits}/{_resolves} " +
                $"({(100.0 * _hits / Math.Max(1, _resolves)):F1}%)");
            Console.WriteLine(
                $"[TexturePack] unique terrain atlas coverage=" +
                $"{_terrainAtlasHits}/{_terrainResolves} " +
                $"({(100.0 * _terrainAtlasHits /
                    Math.Max(1, _terrainResolves)):F1}%)");
            Console.WriteLine(
                $"[TexturePack] terrain atlas resolution " +
                $"catalog={_terrainAtlasCatalogHits} " +
                $"anchor={_terrainAtlasAnchorHits} " +
                $"miss={_terrainAtlasMisses}");
        }
        if (_texture != 0) _gl.DeleteTexture(_texture);
        _texture = 0;
    }
}
