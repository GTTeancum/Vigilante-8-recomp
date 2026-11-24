using System.Buffers.Binary;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;

static class TerrainWindowCases
{
    const int W = 40, H = W * 4;
    const ulong atlasKey = 0x1234abUL;
    static readonly ushort[] Colors = [0, 0x801f, 0x83e0, 0xfc00, 0xffff];
    static byte Index(int x, int y) => (byte)(1 + ((x >> 3) & 1) + 2 * ((y >> 3) & 1));

    public static object Definition(DirectoryInfo fixture)
    {
        byte[] image = new byte[128 + W * H * 4], indices = new byte[W * H];
        void Word(int offset, uint v) =>
            BinaryPrimitives.WriteUInt32LittleEndian(image.AsSpan(offset), v);
        Word(0, 0x20534444); Word(4, 124); Word(8, 0x100f); Word(12, H); Word(16, W); Word(20, W*4);
        Word(76, 32); Word(80, 0x41); Word(88, 32); Word(92,255); Word(96, 0xff00);
        Word(100, 0xff0000); Word(104, 0xff000000); Word(108, 0x1000);
        for (int y=0; y<H; y++)
        for (int x=0; x<W; x++)
        {
            byte index = indices[y*W+x] = Index(x,y);
            uint rgba = index switch { 1 => 0xff0000ff, 2 => 0xff00ff00, 3 => 0xffff0000, _ => 0xffffffff };
            Word(128 + 4*(y*W+x), rgba);
        }
        File.WriteAllBytes(Path.Combine(fixture.FullName, "checker.dds"), image);
        return new {
            name = "checker", image = "checker.dds", width = W, height = H,
            imageX = 0, imageY = 0, depth = 1, indexHash = atlasKey.ToString("x16"),
            indices = Convert.ToBase64String(indices),
            palette = Enumerable.Range(0,256).Select(i => (i < Colors.Length ? Colors[i] : 0).ToString("x4"))
        };
    }

    public static void Run(TextureReplacementAtlas atlas, Action<bool,string> check)
    {
        // Upload the same 8bpp terrain tile twice, offset by 64 texels. A single
        // DDS crop is valid for a translated contiguous window, but cannot
        // reproduce a PS1 window that repeats an 8-texel band inside the tile.
        const int tpage = 128 + 5, pageX = 320, cy = 333, clut = cy << 6;
        atlas.Write(0, cy, Colors.Length, 1, Colors);
        ushort[] packed = new ushort[W/2*W];
        for (int y=0; y<W; y++)
        for (int x=0; x<W; x+=2)
            packed[y*(W/2)+x/2] = (ushort)(Index(x,y) | (Index(x+1,y) << 8));
        atlas.Write(pageX, 0, W/2, W, packed);
        atlas.Write(pageX + 32, 0, W/2, W, packed);
        void CheckWindow(int ax, int ay, int ox, int oy, bool straight, string label)
        {
            var rect = atlas.Resolve(tpage, clut, 0,0,W-1,W-1, ax,ay,ox,oy,
                HleMaterialKind.TerrainRoute, out ulong key);
            atlas.Resolve(tpage, clut, 0,0,W-1,W-1, ax,ay,ox,oy,
                HleMaterialKind.Opaque, out ulong decodedKey);
            check(straight ? rect.Valid && key == atlasKey : !rect.Valid && key == decodedKey,
                $"{label}: straight={straight}, valid={rect.Valid}, key={key:x16}, decoded={decodedKey:x16}");
        }
        CheckWindow(255,255,0,0,true, "identity terrain crop");
        CheckWindow(191,255,64,0,true, "translated terrain crop");
        CheckWindow(247,255,0,0,false, "folded terrain U");
        CheckWindow(255,247,0,0,false, "folded terrain V");
        CheckWindow(247,247,8,8,false, "terrain folding both axes high");
    }
}
