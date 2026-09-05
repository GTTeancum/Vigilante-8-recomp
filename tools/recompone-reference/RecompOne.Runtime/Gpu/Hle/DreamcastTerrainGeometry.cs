using System.Numerics;

namespace RecompOne.Runtime.Hle;

// HIGH: control flow follows Dreamcast 8C1020A6, 8C10238C and 8C1026B4.
// The outer four-unit walker is subdivided into two/one-unit leaves. All
// inputs are authored level data and camera state; no content identities.
public static class DreamcastTerrainGeometry
{
    public readonly record struct Sample(
        Vector3 View, float Height, Vector3 Ramp, Vector3 Flat);

    public readonly record struct Vertex(
        Vector3 View, float Height, float DecisionDepth,
        Vector3 Ramp, Vector3 Flat, float TextureFade);

    public readonly record struct Leaf(int X, int Z, int Size, bool Textured);

    public static int Build(
        ReadOnlySpan<Sample> samples, Vector3 heightAxis, bool alternate,
        Span<Vertex> vertices, Span<Leaf> leaves)
    {
        if (samples.Length != 25 || vertices.Length < 25 || leaves.Length < 16)
            throw new ArgumentException("Terrain patch requires 25 samples and 16 leaf slots.");
        float textureEnd = alternate ? 19.6f : 25.2f;
        ReadOnlySpan<int> corners = stackalloc int[] { 0, 4, 20, 24 };
        foreach (int index in corners)
        {
            Sample s = samples[index];
            vertices[index] = new Vertex(s.View, s.Height, s.View.Z / 256f,
                s.Ramp, s.Flat, Fade(s.View.Z / 256f, textureEnd, 5f));
        }
        int count = 0;
        Visit(samples, heightAxis, alternate, vertices, leaves,
            0, 0, 4, 2, ref count);
        return count;
    }

    static float Fade(float depth, float end, float span) =>
        Math.Clamp((depth - (end - span)) / span, 0f, 1f);

    static void Visit(
        ReadOnlySpan<Sample> samples, Vector3 heightAxis, bool alternate,
        Span<Vertex> vertices, Span<Leaf> leaves,
        int x, int z, int size, int state, ref int count)
    {
        int tl = x * 5 + z, tr = (x + size) * 5 + z;
        int bl = x * 5 + z + size, br = (x + size) * 5 + z + size;
        float minimum = MathF.Min(MathF.Min(vertices[tl].DecisionDepth,
            vertices[tr].DecisionDepth), MathF.Min(vertices[bl].DecisionDepth,
            vertices[br].DecisionDepth));
        float maximum = MathF.Max(MathF.Max(vertices[tl].DecisionDepth,
            vertices[tr].DecisionDepth), MathF.Max(vertices[bl].DecisionDepth,
            vertices[br].DecisionDepth));
        // The SH-4 maximum-depth rejection is at root entry and textured
        // submission only. A two-unit patch can contain visible authored
        // interior vertices even when all four parent corners are behind.
        if ((state == 2 || state == 0) && maximum <= 0.015f)
            return;
        float end = state == 2 ? 34.3f : alternate ? 19.6f : 25.2f;
        if (state == 0 || minimum > end)
        {
            leaves[count++] = new Leaf(x, z, size, state == 0);
            return;
        }
        int half = size / 2;
        float span = state == 2 ? alternate ? 6f : 3f : 5f;
        Midpoint(samples, heightAxis, alternate, vertices,
            (x + half) * 5 + z, tl, tr, state, end, span);
        Midpoint(samples, heightAxis, alternate, vertices,
            x * 5 + z + half, tl, bl, state, end, span);
        Midpoint(samples, heightAxis, alternate, vertices,
            (x + half) * 5 + z + half, tr, bl, state, end, span);
        Midpoint(samples, heightAxis, alternate, vertices,
            (x + size) * 5 + z + half, tr, br, state, end, span);
        Midpoint(samples, heightAxis, alternate, vertices,
            (x + half) * 5 + z + size, bl, br, state, end, span);

        // Native work stack: first TL, then BR, BL, TR (three LIFO entries).
        Visit(samples, heightAxis, alternate, vertices, leaves,
            x, z, half, state - 1, ref count);
        Visit(samples, heightAxis, alternate, vertices, leaves,
            x + half, z + half, half, state - 1, ref count);
        Visit(samples, heightAxis, alternate, vertices, leaves,
            x, z + half, half, state - 1, ref count);
        Visit(samples, heightAxis, alternate, vertices, leaves,
            x + half, z, half, state - 1, ref count);
    }

    static void Midpoint(
        ReadOnlySpan<Sample> samples, Vector3 heightAxis, bool alternate,
        Span<Vertex> vertices, int index, int a, int b,
        int state, float end, float span)
    {
        Sample own = samples[index];
        float parentHeight = (vertices[a].Height + vertices[b].Height) * 0.5f;
        Vector3 parentView = own.View + heightAxis * (parentHeight - own.Height);
        float depth = parentView.Z / 256f;
        float factor = Fade(depth, end, span);
        // 8C101EC0 changes geometry only in the last quarter of the band.
        float weight = factor > 0.75f ? (4f * factor - 3f) * factor : 0f;
        float height = own.Height + (parentHeight - own.Height) * weight;
        Vector3 view = own.View + heightAxis * (height - own.Height);
        Vector3 parentFlat = (samples[a].Flat + samples[b].Flat) * 0.5f;
        Vector3 flat = factor > 0f
            ? own.Flat * (1f - factor) + parentFlat * factor
            : state == 2 ? own.Flat : parentFlat;
        float textureFade = state == 2
            ? factor > 0f ? 1f : Fade(depth, alternate ? 19.6f : 25.2f, 5f)
            : factor;
        vertices[index] = new Vertex(view, height, depth, own.Ramp, flat, textureFade);
    }
}
