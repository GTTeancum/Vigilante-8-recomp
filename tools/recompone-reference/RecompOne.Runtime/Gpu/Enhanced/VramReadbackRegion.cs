namespace RecompOne.Runtime.Enhanced;

/// <summary>A native-resolution read window; no pixel values are approximated.</summary>
public readonly record struct VramReadbackRegion(int X, int Y, int Width, int Height)
{
    public static VramReadbackRegion Full => new(0, 0, 1024, 512);

    public static VramReadbackRegion ForRead(int x, int y, int w, int h)
    {
        x &= 1023;
        y &= 511;
        // Preserve native wrap semantics using the full window at a VRAM seam.
        if (x + w > 1024 || y + h > 512)
            return Full;
        int left = x & ~63, top = y & ~63;
        int right = Math.Min(1024, (x + w + 63) & ~63);
        int bottom = Math.Min(512, (y + h + 63) & ~63);
        return new(left, top, right - left, bottom - top);
    }

    public bool Contains(int x, int y, int w, int h)
    {
        if (this == Full) return true;
        x &= 1023;
        y &= 511;
        return x >= X && y >= Y && x + w <= X + Width && y + h <= Y + Height;
    }

    public void CopyTo(ReadOnlySpan<ushort> source, int x, int y,
        int w, int h, Span<ushort> destination)
    {
        x &= 1023;
        y &= 511;
        int rows = Math.Min(h, destination.Length / Math.Max(1, w));
        for (int row = 0; row < rows; row++)
        {
            int offset = (((y + row) & 511) - Y) * Width;
            if (x + w <= 1024)
                source.Slice(offset + x - X, w).CopyTo(destination.Slice(row * w, w));
            else
                for (int col = 0; col < w; col++)
                    destination[row * w + col] = source[offset + ((x + col) & 1023) - X];
        }
    }
}
