using RecompOne.Runtime.Config;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;
using Silk.NET.Maths;
using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Windowing.Glfw;

// Exercise the actual frame-scoped setting, draw to a registered display
// target, resolve it back into native VRAM, and read the pixel the game sees.
GlfwWindowing.Use();
using var window = Window.Create(WindowOptions.Default with
{
    Title = "MsaaTargetContract", Size = new Vector2D<int>(64, 64), IsVisible = false,
    API = new GraphicsAPI(ContextAPI.OpenGL, ContextProfile.Core, ContextFlags.Default, new APIVersion(4, 5))
});
window.Initialize();
using var gl = GL.GetApi(window);
gl.GetInteger(GLEnum.MaxSamples, out int limit);
int[] Modes(GLEnum format)
{
    gl.GetInternalformat(GLEnum.Renderbuffer, format, GLEnum.NumSampleCounts, 1, out int count);
    int[] values = new int[count];
    if (count > 0)
        gl.GetInternalformat(GLEnum.Renderbuffer, format, GLEnum.Samples, (uint)count, values.AsSpan());
    return values;
}
var modes = Modes(GLEnum.Rgba8).Intersect(Modes(GLEnum.Depth24Stencil8))
    .Where(n => n is >= 2 and <= 8).ToArray();
Console.WriteLine($"driver={gl.GetStringS(StringName.Renderer)} max-samples={limit} shared-modes=[{string.Join(",", modes)}]");

GlVram.Scale = 1;
ConfigManager.View.MsaaSamples = 0;
var backend = new EnhancedGlBackend(gl);
backend.InitGl();
if (!backend.Ready) throw new Exception("Enhanced GL initialization failed.");
GpuHle.NotifyDisplay(0, 0, 64, 64);
backend.SetDrawEnv(new HleDrawEnv { ClipX1 = 63, ClipY1 = 63 });
int failures = 0, n = 0;
int previousSamples = -1, previousFbo = -1;
foreach (int requested in new[] { 0, 2, 4, 8, 8, 4, 0, 2 })
{
    ConfigManager.View.MsaaSamples = requested;
    backend.PresentDisplay(0, 0, 64, 64);
    GpuHle.NotifyDisplay(0, 0, 64, 64); // Force a fresh target lookup each frame.
    byte red = (n++ % 2 == 0) ? (byte)255 : (byte)0;
    byte green = (byte)(255 - red);
    backend.DrawRect(new HleRect { X=8, Y=8, W=16, H=16, R=red, G=green }, new PrimFlags { Material = HleMaterialKind.Ui });
    backend.Flush();
    var status = gl.CheckFramebufferStatus(FramebufferTarget.Framebuffer);
    gl.GetInteger(GLEnum.Samples, out int samples);
    gl.GetInteger(GLEnum.DrawFramebufferBinding, out int fbo);
    ushort[] pixels = new ushort[2];
    backend.ReadVram(12, 12, 1, 1, pixels.AsSpan(0, 1));
    backend.ReadVram(32, 32, 1, 1, pixels.AsSpan(1));
    var error = gl.GetError();
    // Check the driver's real storage, not only the pixel. Silently turning
    // off AA, or forcing the same quality for all requests, is not a fix.
    int expected = requested == 0 || modes.Length == 0 ? 0 :
        modes.Where(n => n <= requested).DefaultIfEmpty(modes.Min()).Max();
    bool ok = (pixels[0] & 0x7fff) == (red == 255 ? 0x001f : 0x03e0) &&
              (pixels[1] & 0x7fff) == 0 && error == GLEnum.NoError &&
              status == GLEnum.FramebufferComplete && fbo != 0 &&
              samples == expected && samples <= limit &&
              ConfigManager.View.MsaaSamples == requested &&
              (previousSamples != samples || previousFbo == fbo);
    previousSamples = samples; previousFbo = fbo;
    // Drain the previous case's errors so a failure cannot affect the next.
    while (gl.GetError() != GLEnum.NoError) {}
    if (!ok) failures++;
    Console.WriteLine($"{(ok ? "PASS" : "FAIL")} request={requested}x pixel={pixels[0]:x4} outside={pixels[1]:x4} gl={error} actual={samples}x fbo={fbo}:{status}");
}
backend.Dispose();
return failures == 0 ? 0 : 1;
