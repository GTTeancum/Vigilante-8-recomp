using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Hle;

public sealed class GlVram
{
    public static int Scale { get; set; } = 4;
    public static int Width => VramShadow.Width * Scale;
    public static int Height => VramShadow.Height * Scale;

    readonly GL _gl;
    uint _tex, _fbo;
    uint _stageTex, _stageFbo;
    uint _scratchTex;
    byte[] _uploadRgba = [];
    byte[] _readRgba = [];

    public uint Texture => _tex;
    public uint Fbo => _fbo;

    public GlVram(GL gl) => _gl = gl;

    public void Init()
    {
        _tex = CreateTex(Width, Height);
        _fbo = CreateFbo(_tex);
        _stageTex = CreateTex(VramShadow.Width, VramShadow.Height);
        _stageFbo = CreateFbo(_stageTex);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
    }

    uint CreateTex(int w, int h)
    {
        uint t = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, t);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        _gl.TexImage2D<byte>(TextureTarget.Texture2D, 0, InternalFormat.Rgba8, (uint)w, (uint)h, 0,
            PixelFormat.Rgba, PixelType.UnsignedByte, new byte[w * h * 4].AsSpan());
        return t;
    }

    uint CreateFbo(uint tex)
    {
        uint f = _gl.GenFramebuffer();
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, f);
        _gl.FramebufferTexture2D(FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0,
            TextureTarget.Texture2D, tex, 0);
        return f;
    }

    public void BindDraw()
    {
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
        _gl.Viewport(0, 0, (uint)Width, (uint)Height);
    }

    public void Barrier() => _gl.TextureBarrier();

    public void WriteRect(int x, int y, int w, int h, ReadOnlySpan<ushort> px)
    {
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        _gl.BindTexture(TextureTarget.Texture2D, _stageTex);
        int count = w * h;
        if (_uploadRgba.Length < count * 4)
            _uploadRgba = new byte[count * 4];
        for (int i = 0, o = 0; i < count; i++, o += 4)
        {
            ushort p = px[i];
            _uploadRgba[o + 0] = Expand5(p & 0x1F);
            _uploadRgba[o + 1] = Expand5((p >> 5) & 0x1F);
            _uploadRgba[o + 2] = Expand5((p >> 10) & 0x1F);
            _uploadRgba[o + 3] = (byte)((p & 0x8000) != 0 ? 255 : 0);
        }
        _gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
        _gl.TexSubImage2D<byte>(TextureTarget.Texture2D, 0, x, y, (uint)w, (uint)h,
            PixelFormat.Rgba, PixelType.UnsignedByte, _uploadRgba.AsSpan(0, count * 4));

        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _stageFbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _fbo);
        _gl.BlitFramebuffer(x, y, x + w, y + h,
            x * Scale, y * Scale, (x + w) * Scale, (y + h) * Scale,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
    }

    public void Fill(int x, int y, int w, int h, ushort color15)
    {
        float r = (color15 & 0x1F) / 31f, g = ((color15 >> 5) & 0x1F) / 31f, b = ((color15 >> 10) & 0x1F) / 31f;
        float a = (color15 & 0x8000) != 0 ? 1f : 0f;
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
        _gl.Enable(EnableCap.ScissorTest);
        _gl.Scissor(x * Scale, y * Scale, (uint)Math.Max(0, w * Scale), (uint)Math.Max(0, h * Scale));
        _gl.ClearColor(r, g, b, a);
        _gl.Clear(ClearBufferMask.ColorBufferBit);
        _gl.Disable(EnableCap.ScissorTest);
    }

    public void CopyRect(int sx, int sy, int dx, int dy, int w, int h)
    {
        int sw = w * Scale, sh = h * Scale;
        bool overlap = sx < dx + w && dx < sx + w && sy < dy + h && dy < sy + h;
        if (!overlap)
        {
            _gl.CopyImageSubData(_tex, CopyImageSubDataTarget.Texture2D, 0, sx * Scale, sy * Scale, 0,
                _tex, CopyImageSubDataTarget.Texture2D, 0, dx * Scale, dy * Scale, 0, (uint)sw, (uint)sh, 1);
            return;
        }

        EnsureScratch();
        _gl.CopyImageSubData(_tex, CopyImageSubDataTarget.Texture2D, 0, sx * Scale, sy * Scale, 0,
            _scratchTex, CopyImageSubDataTarget.Texture2D, 0, 0, 0, 0, (uint)sw, (uint)sh, 1);
        _gl.CopyImageSubData(_scratchTex, CopyImageSubDataTarget.Texture2D, 0, 0, 0, 0,
            _tex, CopyImageSubDataTarget.Texture2D, 0, dx * Scale, dy * Scale, 0, (uint)sw, (uint)sh, 1);
    }

    public void ReadRect(int x, int y, int w, int h, Span<ushort> dst)
    {
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _fbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _stageFbo);
        _gl.BlitFramebuffer(x * Scale, y * Scale, (x + w) * Scale, (y + h) * Scale,
            x, y, x + w, y + h, ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);

        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _stageFbo);
        int count = w * h;
        if (_readRgba.Length < count * 4)
            _readRgba = new byte[count * 4];
        _gl.PixelStore(PixelStoreParameter.PackAlignment, 1);
        _gl.ReadPixels<byte>(x, y, (uint)w, (uint)h, PixelFormat.Rgba, PixelType.UnsignedByte,
            _readRgba.AsSpan(0, count * 4));
        for (int i = 0, o = 0; i < count; i++, o += 4)
        {
            int r = _readRgba[o + 0] >> 3;
            int g = _readRgba[o + 1] >> 3;
            int b = _readRgba[o + 2] >> 3;
            int a = _readRgba[o + 3] >= 128 ? 0x8000 : 0;
            dst[i] = (ushort)(r | (g << 5) | (b << 10) | a);
        }
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
    }

    static byte Expand5(int value) => (byte)((value << 3) | (value >> 2));

    void EnsureScratch()
    {
        if (_scratchTex != 0) return;
        _scratchTex = CreateTex(Width, Height);
    }

    public void Dispose()
    {
        if (_fbo != 0) _gl.DeleteFramebuffer(_fbo);
        if (_stageFbo != 0) _gl.DeleteFramebuffer(_stageFbo);
        if (_tex != 0) _gl.DeleteTexture(_tex);
        if (_stageTex != 0) _gl.DeleteTexture(_stageTex);
        if (_scratchTex != 0) _gl.DeleteTexture(_scratchTex);
    }
}
