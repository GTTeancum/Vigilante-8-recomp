// Source runtime, synthetic 5551 pixels and actual GL only. No guest or assets.
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;
using TransferSegment = (int X, int Y, int BufferX, int BufferY, int Width, int Height);

bool baseline = args.Contains("--baseline");
int samples = args.Contains("--msaa4") ? 4 : 0;
int scale = args.Contains("--scale4") ? 4 : 1;
var cases = new[]
{
    new Case("zero-w", 1022, 510, 0, 3, []),
    new Case("zero-h", 1022, 510, 3, 0, []),
    new Case("negative", 1022, 510, -4, 3, []),
    new Case("center", 15, 24, 2, 3, [(15,24,0,0,2,3)]),
    new Case("exact-edge", 1021, 508, 3, 4, [(1021,508,0,0,3,4)]),
    new Case("X-only", 1022, 28, 5, 3, [(1022,28,0,0,2,3), (0,28,2,0,3,3)]),
    new Case("Y-only", 28, 510, 3, 5, [(28,510,0,0,3,2), (28,0,0,2,3,3)]),
    new Case("X+Y", 1023, 510, 4, 5, [(1023,510,0,0,1,2), (0,510,1,0,3,2), (1023,0,0,2,1,3), (0,0,1,2,3,3)]),
    new Case("aliased", -1, 1022, 4, 5, [(1023,510,0,0,1,2), (0,510,1,0,3,2), (1023,0,0,2,1,3), (0,0,1,2,3,3)])
};
if (!baseline)
{
    // Bind the actual private helper rather than reproducing its arithmetic.
    // Calling via the typed delegate keeps the span purely on the stack.
    var split = typeof(GlVram).GetMethod("WrappedSegments", BindingFlags.Static | BindingFlags.NonPublic)!
        .CreateDelegate<Splitter>();
    Span<TransferSegment> scratch = stackalloc TransferSegment[4];
    foreach (var c in cases)
        Check(split(c.X, c.Y, c.W, c.H, scratch).SequenceEqual(c.Order),
            $"{c.Name}: incorrect segment order or buffer offsets");
    const int repeats = 10_000;
    for (int i = 0; i < 100; i++) _ = split(1023, 510, 4, 5, scratch);
    long before = GC.GetAllocatedBytesForCurrentThread();
    int sum = 0;
    for (int i = 0; i < repeats; i++)
        sum += split(1023, 510, 4, 5, scratch).Length;
    long bytes = GC.GetAllocatedBytesForCurrentThread() - before;
    Check(sum == 4 * repeats && bytes == 0, $"Splitter allocated {bytes} bytes");
    Console.WriteLine($"PASS exact order; {repeats} splits allocated {bytes} managed bytes");
}

const int D = 32;
GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with
{
    Title = "VramTransferContract", Size = new Vector2D<int>(D, D), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
GlVram.Scale = scale;
ConfigManager.View.MsaaSamples = samples;
ConfigManager.View.HighResolutionTextures = false;
var backend = new EnhancedGlBackend(gl);
try
{
    backend.InitGl();
    Check(backend.Ready, "Actual GL renderer unavailable");
    var raw = (GlVram)Field("_vram");
    Console.WriteLine($"GL driver={gl.GetStringS(StringName.Renderer)} scale={scale} samples={samples}");

    ushort[] single = new ushort[1];
    int[] scissor = new int[4];
    foreach (var c in cases)
    {
        if (c.Order.Length == 0)
        {
            // All three empty operations must preserve state, even the
            // otherwise unconditional FBO, pack and scissor setup.
            gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
            gl.Enable(EnableCap.ScissorTest);
            gl.Scissor(5, 6, 7, 8);
            gl.PixelStore(PixelStoreParameter.PackAlignment, 4);
            gl.PixelStore(PixelStoreParameter.UnpackAlignment, 4);
            single[0] = 0xa1c0;
            raw.WriteRect(c.X, c.Y, c.W, c.H, ReadOnlySpan<ushort>.Empty);
            raw.Fill(c.X, c.Y, c.W, c.H, 0);
            raw.ReadRect(c.X, c.Y, c.W, c.H, single);
            gl.GetInteger(GLEnum.DrawFramebufferBinding, out int bound);
            gl.GetInteger(GLEnum.PackAlignment, out int pack);
            gl.GetInteger(GLEnum.UnpackAlignment, out int unpack);
            gl.GetInteger(GLEnum.ScissorBox, scissor.AsSpan());
            Check(bound == 0 && pack == 4 && unpack == 4 && gl.IsEnabled(EnableCap.ScissorTest)
                && scissor.SequenceEqual(new[] { 5, 6, 7, 8 }) && single[0] == 0xa1c0, $"{c.Name}: empty transfer had effects");
            Console.WriteLine($"GL {c.Name}: no transfer");
            continue;
        }

        ushort[] pixels = Pattern(c.W * c.H);
        ushort[] read = new ushort[pixels.Length];
        raw.WriteRect(c.X, c.Y, c.W, c.H, pixels);
        raw.ReadRect(c.X, c.Y, c.W, c.H, read);
        Check(pixels.SequenceEqual(read), $"{c.Name}: round-trip changed native 5551 data");
        // Single-pixel reads cannot hide matching offset mistakes made by
        // both the segmented write and the segmented read.
        for (int dy = 0; dy < c.H; dy++)
        for (int dx = 0; dx < c.W; dx++)
        {
            raw.ReadRect((c.X + dx) & 1023, (c.Y + dy) & 511, 1, 1, single);
            Check(single[0] == pixels[dy * c.W + dx], $"{c.Name}: wrong physical placement at {dx},{dy}");
        }

        const ushort fill = 0xa652;
        raw.Fill(c.X, c.Y, c.W, c.H, fill);
        var last = c.Order[^1];
        gl.GetInteger(GLEnum.ScissorBox, scissor.AsSpan());
        Check(scissor.SequenceEqual(new[] { last.X * scale, last.Y * scale, last.Width * scale, last.Height * scale })
            && !gl.IsEnabled(EnableCap.ScissorTest), $"{c.Name}: final fill region/state changed");
        raw.ReadRect(c.X, c.Y, c.W, c.H, read);
        Check(read.All(px => px == fill), $"{c.Name}: fill missed native pixels or mask");
        Console.WriteLine($"GL {c.Name}: {Hash(pixels)} {Hash(read)}");
    }

    // Let the actual display target consume a rectangle that wraps into
    // its top-left corner. Make another draw before resolve so the 4x test
    // must preserve that corner in the real multisample draw target.
    GpuHle.NotifyDisplay(0, 0, D, D);
    backend.SetDrawEnv(new HleDrawEnv { ClipX1 = D - 1, ClipY1 = D - 1 });
    void Marker(int x, int y) => backend.DrawRect(new HleRect { X=x, Y=y, W=2, H=2, R=255 },
        new PrimFlags { Material = HleMaterialKind.Ui });
    Marker(8, 8);
    backend.Flush();
    var rt = (GlDisplayRt)Field("_classifiedTarget");
    Check(rt.Samples == samples && (rt.MsaaFbo != 0) == (samples > 0), "wrong display target AA mode");
    ushort[] corner = Pattern(16);
    backend.WriteVram(1022, 510, 4, 4, corner);
    Marker(16, 16);
    backend.Flush();
    rt.Resolve(gl);
    gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, rt.Fbo);
    ushort[] displayed = new ushort[D * D * scale * scale];
    gl.ReadPixels<ushort>(0, 0, (uint)(D * scale), (uint)(D * scale), PixelFormat.Rgba,
        PixelType.UnsignedShort1555Rev, displayed.AsSpan());
    for (int y = 0; y < 2 * scale; y++)
    for (int x = 0; x < 2 * scale; x++)
        Check(displayed[y * D * scale + x] == corner[(y / scale + 2) * 4 + x / scale + 2],
            "wrapped upload failed to reach the resolved display target");
    Check((displayed[(16 * D * scale + 16) * scale] & 0x7fff) == 31, "final draw missing");
    Check(gl.GetError() == GLEnum.NoError, "OpenGL reported a transfer error");
    Console.WriteLine($"GL display: {Hash(displayed)}");
    Console.WriteLine("GL PASS ordered, wrapped and empty transfers");
}
finally
{
    backend.Dispose();
}

object Field(string name) => typeof(EnhancedGlBackend)
    .GetField(name, BindingFlags.Instance | BindingFlags.NonPublic)!.GetValue(backend)!;
static ushort[] Pattern(int n) => Enumerable.Range(0, n)
    .Select(i => unchecked((ushort)(0x2941 + i * 8191))).ToArray();
static string Hash(ushort[] pixels) =>
    Convert.ToHexString(SHA256.HashData(MemoryMarshal.AsBytes(pixels.AsSpan())));
static void Check(bool ok, string why)
{
    if (!ok) throw new InvalidOperationException(why);
}

delegate ReadOnlySpan<TransferSegment> Splitter(int x, int y, int w, int h, Span<TransferSegment> scratch);
sealed record Case(string Name, int X, int Y, int W, int H, TransferSegment[] Order);
