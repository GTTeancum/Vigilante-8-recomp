namespace RecompOne.Runtime.Enhanced;

/// <summary>
/// Carries a loaded .FNT filename from the CD read buffer to the native GPU
/// image upload. The Enhanced renderer can then resolve an HD font atlas by
/// file identity and source coordinates; it never identifies glyphs by VRAM
/// content or a generated crop hash.
/// </summary>
internal static class FontFileProvenance
{
    static readonly bool TraceAllFileSources =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_FONT_FILE_SOURCES") == "1";
    readonly record struct RamExtent(
        uint Start, uint End, string Path, int FileOffset);
    internal readonly record struct VramExtent(
        string Path, int XWords, int Y, int WidthWords, int Height,
        int SourceXWords, int SourceY,
        int SourceWidthWords, int SourceHeight);

    static readonly object Gate = new();
    static readonly List<RamExtent> RamExtents = [];
    static readonly List<VramExtent> VramExtents = [];
    static readonly HashSet<string> TrackedPaths =
        new(StringComparer.OrdinalIgnoreCase);
    static int _uploadX, _uploadY, _uploadWidth, _uploadHeight;
    static string? _uploadPath;
    static bool _uploadSawData;
    static bool _uploadMixedSource;

    internal static string Normalize(string path) =>
        path.Replace('\\', '/').TrimStart('/').ToUpperInvariant();

    internal static void RegisterTrackedPath(string path)
    {
        lock (Gate)
            TrackedPaths.Add(Normalize(path));
    }

    internal static void TrackFileRead(
        string path, uint address, int fileOffset, int byteCount)
    {
        if (byteCount <= 0)
            return;
        uint end = checked(address + (uint)byteCount);
        lock (Gate)
        {
            // Every disc-file read invalidates stale ownership at the
            // destination. Only FNT reads establish new font ownership.
            RamExtents.RemoveAll(extent =>
                address < extent.End && end > extent.Start);
            string normalized = Normalize(path);
            if (!TraceAllFileSources &&
                !Path.GetExtension(normalized).Equals(
                    ".FNT", StringComparison.OrdinalIgnoreCase) &&
                !TrackedPaths.Contains(normalized))
                return;
            RamExtents.Add(new RamExtent(
                address, end, normalized, fileOffset));
            if (RamExtents.Count > 256)
                RamExtents.RemoveRange(0, RamExtents.Count - 256);
        }
    }

    internal static void BeginUpload(int x, int y, int width, int height)
    {
        lock (Gate)
        {
            _uploadX = x;
            _uploadY = y;
            _uploadWidth = width;
            _uploadHeight = height;
            _uploadPath = null;
            _uploadSawData = false;
            _uploadMixedSource = false;
        }
    }

    internal static void TrackUploadWord(uint sourceAddress)
    {
        if (sourceAddress == uint.MaxValue)
            return;
        lock (Gate)
        {
            RamExtent? match = null;
            for (int index = RamExtents.Count - 1; index >= 0; index--)
            {
                RamExtent extent = RamExtents[index];
                if (sourceAddress >= extent.Start && sourceAddress < extent.End)
                {
                    match = extent;
                    break;
                }
            }
            if (match is not RamExtent source)
                return;
            _uploadSawData = true;
            if (_uploadPath == null)
                _uploadPath = source.Path;
            else if (!_uploadPath.Equals(
                         source.Path, StringComparison.OrdinalIgnoreCase))
                _uploadMixedSource = true;
        }
    }

    internal static void CompleteUpload()
    {
        lock (Gate)
        {
            int x0 = _uploadX;
            int y0 = _uploadY;
            int x1 = x0 + _uploadWidth;
            int y1 = y0 + _uploadHeight;
            VramExtents.RemoveAll(extent =>
                x0 < extent.XWords + extent.WidthWords &&
                x1 > extent.XWords &&
                y0 < extent.Y + extent.Height &&
                y1 > extent.Y);
            if (_uploadSawData && !_uploadMixedSource && _uploadPath != null)
            {
                VramExtents.Add(new VramExtent(
                    _uploadPath, _uploadX, _uploadY,
                    _uploadWidth, _uploadHeight, 0, 0,
                    _uploadWidth, _uploadHeight));
                Console.WriteLine(
                    $"[{(Path.GetExtension(_uploadPath).Equals(
                        ".FNT", StringComparison.OrdinalIgnoreCase)
                        ? "FontFiles" : "FontFileSources")}] " +
                    $"upload path={_uploadPath} " +
                    $"vram={_uploadX},{_uploadY} " +
                    $"size={_uploadWidth}x{_uploadHeight}");
            }
        }
    }

    internal static void TrackVramCopy(
        int sourceX, int sourceY, int destinationX, int destinationY,
        int width, int height)
    {
        if (width <= 0 || height <= 0)
            return;
        lock (Gate)
        {
            // Resolve ownership from a snapshot because PS1 MoveImage permits
            // overlapping rectangles. The destination is invalidated in full;
            // only portions copied from a known FNT regain ownership.
            VramExtent[] sources = [.. VramExtents];
            int destinationRight = destinationX + width;
            int destinationBottom = destinationY + height;
            VramExtents.RemoveAll(extent =>
                destinationX < extent.XWords + extent.WidthWords &&
                destinationRight > extent.XWords &&
                destinationY < extent.Y + extent.Height &&
                destinationBottom > extent.Y);

            int sourceRight = sourceX + width;
            int sourceBottom = sourceY + height;
            foreach (VramExtent extent in sources)
            {
                int left = Math.Max(sourceX, extent.XWords);
                int top = Math.Max(sourceY, extent.Y);
                int right = Math.Min(
                    sourceRight, extent.XWords + extent.WidthWords);
                int bottom = Math.Min(
                    sourceBottom, extent.Y + extent.Height);
                if (right <= left || bottom <= top)
                    continue;
                VramExtents.Add(new VramExtent(
                    extent.Path,
                    destinationX + left - sourceX,
                    destinationY + top - sourceY,
                    right - left,
                    bottom - top,
                    extent.SourceXWords + left - extent.XWords,
                    extent.SourceY + top - extent.Y,
                    extent.SourceWidthWords,
                    extent.SourceHeight));
            }
        }
    }

    internal static bool TryResolve(
        int tpage, int minU, int minV, int maxU, int maxV,
        int twAndX, int twAndY, int twOrX, int twOrY,
        out string path, out int localX, out int localY,
        out int width, out int height,
        out int sourceWidthWords, out int sourceHeight)
    {
        path = "";
        localX = localY = width = height = 0;
        sourceWidthWords = sourceHeight = 0;
        int depth = (tpage >> 7) & 3;
        if (depth > 2)
            return false;
        int u0 = ((minU & twAndX) | twOrX) & 0xFF;
        int v0 = ((minV & twAndY) | twOrY) & 0xFF;
        int u1 = ((maxU & twAndX) | twOrX) & 0xFF;
        int v1 = ((maxV & twAndY) | twOrY) & 0xFF;
        if (u1 < u0 || v1 < v0)
            return false;
        int pixelsPerWord = depth == 0 ? 4 : depth == 1 ? 2 : 1;
        int pageXWords = (tpage & 0xF) * 64;
        int pageY = ((tpage >> 4) & 1) * 256;
        int globalX = pageXWords * pixelsPerWord + u0;
        int globalY = pageY + v0;
        width = u1 - u0 + 1;
        height = v1 - v0 + 1;
        lock (Gate)
        {
            for (int index = VramExtents.Count - 1; index >= 0; index--)
            {
                VramExtent extent = VramExtents[index];
                int sourceX = extent.XWords * pixelsPerWord;
                int sourceWidth = extent.WidthWords * pixelsPerWord;
                if (globalX < sourceX || globalY < extent.Y ||
                    globalX + width > sourceX + sourceWidth ||
                    globalY + height > extent.Y + extent.Height)
                    continue;
                path = extent.Path;
                localX =
                    extent.SourceXWords * pixelsPerWord + globalX - sourceX;
                localY = extent.SourceY + globalY - extent.Y;
                sourceWidthWords = extent.SourceWidthWords;
                sourceHeight = extent.SourceHeight;
                return true;
            }
        }
        return false;
    }

    internal static void ResetVram()
    {
        lock (Gate)
            VramExtents.Clear();
    }
}
