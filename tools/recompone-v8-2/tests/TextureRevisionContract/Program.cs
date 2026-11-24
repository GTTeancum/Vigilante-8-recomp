using System.Buffers.Binary;
using System.Text.Json;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

// Two dummy replacements inside a tiny DDS. No retail textures or disc required.
const ushort red = 0x801f, green = 0x83e0, blue = 0xfc00;
static ulong Key(ushort color, int width = 1, int height = 1)
{
    ulong hash = 14695981039346656037UL;
    void Add(byte b) => hash = unchecked((hash ^ b) * 1099511628211UL);
    Add((byte)width); Add((byte)(width >> 8)); Add((byte)height); Add((byte)(height >> 8));
    static byte Expand(int n) => (byte)(((n & 31) << 3) | ((n & 31) >> 2));
    for (int i = 0; i < width * height; i++)
    {
        Add(Expand(color)); Add(Expand(color >> 5)); Add(Expand(color >> 10));
        Add((byte)((color & 0x8000) == 0 ? 0 : 255));
    }
    return hash;
}

var fixture = Directory.CreateTempSubdirectory("v8-texture-revisions-");
try
{
    byte[] dds = new byte[128 + 8];
    void U32(int i, uint v) => BinaryPrimitives.WriteUInt32LittleEndian(dds.AsSpan(i), v);
    U32(0, 0x20534444); U32(4, 124); U32(8, 0x100f); U32(12, 1); U32(16, 2); U32(20, 8);
    U32(76, 32); U32(80, 0x41); U32(88, 32); U32(92, 0xff); U32(96, 0xff00);
    U32(100, 0xff0000); U32(104, 0xff000000); U32(108, 0x1000);
    U32(128, 0xff0000ff); U32(132, 0xff00ff00);
    File.WriteAllBytes(Path.Combine(fixture.FullName, "colors.dds"), dds);
    var entries = new[] { red, green }.Select((color, x) => new {
        key = Key(color).ToString("x16"), image = "colors.dds", x, y = 0, width = 1, height = 1
    });
    File.WriteAllText(Path.Combine(fixture.FullName, "manifest.json"),
        JsonSerializer.Serialize(new { format = 2, entries }));
    Environment.SetEnvironmentVariable("RECOMPONE_TEXTURE_PACK_DIR", fixture.FullName);
    Environment.SetEnvironmentVariable("RECOMPONE_USER_DATA_DIR", fixture.FullName);
    Environment.SetEnvironmentVariable("RECOMPONE_TRACE_ENHANCED_PERFORMANCE", "1");

    GlfwWindowing.Use();
    using var window = Window.Create(WindowOptions.Default with {
        Title = "TextureRevisionContract", Size = new Vector2D<int>(64, 64), IsVisible = false,
        API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
    });
    window.Initialize();
    using var gl = GL.GetApi(window);
    Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} | {gl.GetStringS(StringName.Version)}");
    using var atlas = new TextureReplacementAtlas(gl);
    if (atlas.Count != 2 || gl.GetError() != GLEnum.NoError)
        throw new Exception("Synthetic replacement pack failed to upload");

    int failures = 0, checks = 0;
    void Check(bool ok, string what)
    {
        checks++;
        if (!ok) { failures++; Console.Error.WriteLine("FAIL " + what); }
    }
    int Page(int x, int y) => 0x100 | ((x & 1023) >> 6) | (((y & 511) >> 8) << 4);
    void Expect(ushort expected, string label, int tpage, int u, int v, int clut = 0,
        int ax = 255, int ay = 255, int ox = 0, int oy = 0)
    {
        var rect = atlas.Resolve(tpage, clut, u, v, u, v, ax, ay, ox, oy,
            HleMaterialKind.Glass, out ulong key);
        bool replaced = expected is red or green;
        Check(key == Key(expected) && rect.Valid == replaced &&
            (!replaced || rect.X == (expected == red ? 2 : 3)),
            $"{label}: expected {Key(expected):x16}/replacement={replaced}, got {key:x16}/x={rect.X}");
    }
    void Direct(ushort expected, string label, int x, int y) =>
        Expect(expected, label, Page(x,y), x & 63, y & 255);
    void Corners(ushort expected, string label, int x, int y)
    {
        for (int yy = 0; yy < 2; yy++)
        for (int xx = 0; xx < 2; xx++)
            Direct(expected, $"{label} ({x+xx},{y+yy})", x+xx, y+yy);
    }

    // Each 2x2 crosses four 64-word by 64-line blocks, including both VRAM seams.
    foreach (var (x,y) in new[] { (63,63), (1023,511) })
    {
        atlas.Fill(x,y,2,2,red); Corners(red, "prime", x,y);
        atlas.Write(x,y,2,2,[green,green,green,green]); Corners(green, "write", x,y);
        atlas.Fill(x,y,2,2,blue); Corners(blue, "native fallback", x,y);
        atlas.Fill(320,128,2,2,red); atlas.Copy(320,128,x,y,2,2);
        Corners(red, "VRAM copy", x,y);
    }
    // Do not fix a stale corner by clearing the cache on every write.
    Direct(red, "prime unrelated page", 320,128);
    atlas.ConsumePerformanceMetrics();
    atlas.Fill(63,63,2,2,green); Direct(red, "unrelated page", 320,128);
    Check(atlas.ConsumePerformanceMetrics().CacheHits == 1, "unrelated write evicted a cache hit");

    // A PS1 texture window remaps texels before packing 4bpp/8bpp/16bpp.
    // The last page wraps high U horizontally in direct/8bpp modes.
    // An 8bpp CLUT at X=1008 likewise wraps the test entry (index 200).
    foreach (int depth in new[] { 0, 1, 2 })
    {
        int tpage = depth * 128 + 15, shift = 2-depth, x = (960 + (128 >> shift)) & 1023;
        int clut = (333 << 6) | 63, index = depth == 0 ? 7 : 200;
        if (depth < 2) atlas.Fill(1008+index,333,1,1,red);
        atlas.Fill(x,128,1,1,depth == 2 ? red : (ushort)index);
        void Probe(ushort color) =>
            Expect(color, $"windowed depth={depth}", tpage,0,0,clut,127,127,128,128);
        Probe(red);
        if (depth == 2) atlas.Fill(x,128,1,1,green);
        else
        {
            atlas.Fill(1008+index-1,333,1,1,green);
            Probe(red); // new palette revision, old texel index
            atlas.Fill(x,128,1,1,(ushort)(index-1));
        }
        Probe(green);
        if (depth < 2) { atlas.Fill(1008+index-1,333,1,1,blue); Probe(blue); }
    }
    // Both 0 and 128 map to 0, but an interior texel in the next block can change.
    atlas.Fill(0,192,128,1,red);
    ulong SpanKey() {
        atlas.Resolve(0x100,0,0,192,128,192,127,255,0,0,
            HleMaterialKind.TerrainRoute, out ulong key);
        return key;
    }
    ulong before = SpanKey();
    Check(before == Key(red,129), "folded window uses the expected red source");
    atlas.Fill(100,192,1,1,green);
    Check(SpanKey() != before, "folded window ignored interior block");

    Console.WriteLine($"TextureRevisionContract: {checks-failures}/{checks} passed");
    return failures == 0 ? 0 : 1;
}
finally { fixture.Delete(recursive:true); }
