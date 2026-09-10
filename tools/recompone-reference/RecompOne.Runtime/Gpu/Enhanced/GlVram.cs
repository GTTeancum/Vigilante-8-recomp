using Silk.NET.OpenGL;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime.Enhanced;

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
    uint _probeTex, _probeFbo, _probeBuffer;
    int _probeCount;
    readonly byte[] _probeRgba = new byte[4096 * 4];

    public int ProbeCount => _probeCount;

    public void BeginProbes() => _probeCount = 0;

    // Snapshot each original nearest-filtered native pixel now; download
    // after preparing the next camera. Later draws cannot alter snapshots.
    public int QueueProbes(ReadOnlySpan<int> xy)
    {
        int count = xy.Length / 2;
        if (_probeCount + count > 4096) throw new InvalidOperationException("probe staging capacity");
        if (_probeFbo == 0) { _probeTex = CreateTex(1024, 4); _probeFbo = CreateFbo(_probeTex); }
        int start = _probeCount;
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _fbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _probeFbo);
        for (int i = 0; i < count; i++)
        {
            int x = xy[2*i] & 1023, y = xy[2*i+1] & 511;
            int index = _probeCount++;
            _gl.BlitFramebuffer(x*Scale, y*Scale, (x+1)*Scale, (y+1)*Scale,
                index%1024, index/1024, index%1024+1, index/1024+1,
                ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        }
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
        return start;
    }

    public unsafe void IssueProbeReadback()
    {
        if (_probeCount == 0) return;
        if (_probeBuffer == 0)
        {
            _probeBuffer = _gl.GenBuffer();
            _gl.BindBuffer(BufferTargetARB.PixelPackBuffer, _probeBuffer);
            _gl.BufferData(BufferTargetARB.PixelPackBuffer, 4096*4, null, BufferUsageARB.StreamRead);
        }
        else _gl.BindBuffer(BufferTargetARB.PixelPackBuffer, _probeBuffer);
        int width = Math.Min(_probeCount,1024), height = (_probeCount+1023)/1024;
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _probeFbo);
        _gl.PixelStore(PixelStoreParameter.PackAlignment,1);
        _gl.ReadPixels(0,0,(uint)width,(uint)height,PixelFormat.Rgba,PixelType.UnsignedByte,(void*)0);
        _gl.BindBuffer(BufferTargetARB.PixelPackBuffer,0);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer,_fbo);
        // Let the GPU transfer while the CPU prepares the following camera.
        _gl.Flush();
    }

    public void ReadProbes(Span<ushort> dst)
    {
        if (_probeCount == 0) return;
        int width = Math.Min(_probeCount,1024), height = (_probeCount+1023)/1024;
        _gl.BindBuffer(BufferTargetARB.PixelPackBuffer,_probeBuffer);
        _gl.GetBufferSubData<byte>(BufferTargetARB.PixelPackBuffer,0,_probeRgba.AsSpan(0,width*height*4));
        _gl.BindBuffer(BufferTargetARB.PixelPackBuffer,0);
        for (int i = 0; i < _probeCount; i++)
        {
            int o = i*4;
            dst[i] = (ushort)((_probeRgba[o]>>3) | ((_probeRgba[o+1]>>3)<<5) |
                ((_probeRgba[o+2]>>3)<<10) | (_probeRgba[o+3]>=128 ? 0x8000 : 0));
        }
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
    }

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

    public void ReinitializeScale(
        int scale, ReadOnlySpan<ushort> nativeVram)
    {
        scale = Math.Clamp(scale, 1, 4);
        if (scale == Scale) return;
        if (nativeVram.Length != VramShadow.Width * VramShadow.Height)
            throw new ArgumentException(
                "native VRAM snapshot has the wrong size",
                nameof(nativeVram));

        if (_fbo != 0) _gl.DeleteFramebuffer(_fbo);
        if (_tex != 0) _gl.DeleteTexture(_tex);
        if (_scratchTex != 0) _gl.DeleteTexture(_scratchTex);
        _fbo = 0;
        _tex = 0;
        _scratchTex = 0;

        Scale = scale;
        _tex = CreateTex(Width, Height);
        _fbo = CreateFbo(_tex);
        WriteRect(
            0, 0, VramShadow.Width, VramShadow.Height, nativeVram);
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
        if (w <= 0 || h <= 0) return;
        x &= VramShadow.Width - 1;
        y &= VramShadow.Height - 1;

        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        _gl.BindTexture(TextureTarget.Texture2D, _stageTex);
        _gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
        _gl.Disable(EnableCap.ScissorTest);

        int firstW = Math.Min(w, VramShadow.Width - x);
        int wrappedW = w - firstW;
        int firstH = Math.Min(h, VramShadow.Height - y);
        int wrappedH = h - firstH;

        WriteSegment(px, w, 0, 0, x, y, firstW, firstH);
        WriteSegment(px, w, firstW, 0, 0, y, wrappedW, firstH);
        WriteSegment(px, w, 0, firstH, x, 0, firstW, wrappedH);
        WriteSegment(px, w, firstW, firstH, 0, 0, wrappedW, wrappedH);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
    }

    void WriteSegment(
        ReadOnlySpan<ushort> source,
        int sourceWidth,
        int sourceX,
        int sourceY,
        int destX,
        int destY,
        int width,
        int height)
    {
        if (width <= 0 || height <= 0) return;
        int count = width * height;
        if (_uploadRgba.Length < count * 4)
            _uploadRgba = new byte[count * 4];
        int output = 0;
        for (int row = 0; row < height; row++)
        {
            int input = (sourceY + row) * sourceWidth + sourceX;
            for (int col = 0; col < width; col++, output += 4)
            {
                ushort p = source[input + col];
                _uploadRgba[output + 0] = Expand5(p & 0x1F);
                _uploadRgba[output + 1] = Expand5((p >> 5) & 0x1F);
                _uploadRgba[output + 2] = Expand5((p >> 10) & 0x1F);
                _uploadRgba[output + 3] =
                    (byte)((p & 0x8000) != 0 ? 255 : 0);
            }
        }

        _gl.BindTexture(TextureTarget.Texture2D, _stageTex);
        _gl.TexSubImage2D<byte>(
            TextureTarget.Texture2D, 0, destX, destY,
            (uint)width, (uint)height,
            PixelFormat.Rgba, PixelType.UnsignedByte, _uploadRgba.AsSpan(0, count * 4));

        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _stageFbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _fbo);
        _gl.BlitFramebuffer(
            destX, destY, destX + width, destY + height,
            destX * Scale, destY * Scale,
            (destX + width) * Scale, (destY + height) * Scale,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
    }

    public void Fill(int x, int y, int w, int h, ushort color15)
    {
        if (w <= 0 || h <= 0) return;
        x &= VramShadow.Width - 1;
        y &= VramShadow.Height - 1;
        float r = (color15 & 0x1F) / 31f, g = ((color15 >> 5) & 0x1F) / 31f, b = ((color15 >> 10) & 0x1F) / 31f;
        float a = (color15 & 0x8000) != 0 ? 1f : 0f;
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _fbo);
        _gl.Enable(EnableCap.ScissorTest);
        _gl.ClearColor(r, g, b, a);
        int firstW = Math.Min(w, VramShadow.Width - x);
        int wrappedW = w - firstW;
        int firstH = Math.Min(h, VramShadow.Height - y);
        int wrappedH = h - firstH;
        FillSegment(x, y, firstW, firstH);
        FillSegment(0, y, wrappedW, firstH);
        FillSegment(x, 0, firstW, wrappedH);
        FillSegment(0, 0, wrappedW, wrappedH);
        _gl.Disable(EnableCap.ScissorTest);
    }

    void FillSegment(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0) return;
        _gl.Scissor(
            x * Scale, y * Scale,
            (uint)(w * Scale), (uint)(h * Scale));
        _gl.Clear(ClearBufferMask.ColorBufferBit);
    }

    public void CopyRect(int sx, int sy, int dx, int dy, int w, int h)
    {
        if (w <= 0 || h <= 0) return;
        sx &= VramShadow.Width - 1;
        sy &= VramShadow.Height - 1;
        dx &= VramShadow.Width - 1;
        dy &= VramShadow.Height - 1;
        int sw = w * Scale, sh = h * Scale;
        bool wraps =
            sx + w > VramShadow.Width ||
            dx + w > VramShadow.Width ||
            sy + h > VramShadow.Height ||
            dy + h > VramShadow.Height;
        bool overlap =
            sx < dx + w && dx < sx + w &&
            sy < dy + h && dy < sy + h;
        if (!wraps && !overlap)
        {
            _gl.CopyImageSubData(_tex, CopyImageSubDataTarget.Texture2D, 0, sx * Scale, sy * Scale, 0,
                _tex, CopyImageSubDataTarget.Texture2D, 0, dx * Scale, dy * Scale, 0, (uint)sw, (uint)sh, 1);
            return;
        }

        EnsureScratch();
        int sourceFirstW = Math.Min(w, VramShadow.Width - sx);
        int sourceWrappedW = w - sourceFirstW;
        int sourceFirstH = Math.Min(h, VramShadow.Height - sy);
        int sourceWrappedH = h - sourceFirstH;
        CopySegment(
            _tex, sx, sy, _scratchTex, 0, 0,
            sourceFirstW, sourceFirstH);
        CopySegment(
            _tex, 0, sy, _scratchTex, sourceFirstW, 0,
            sourceWrappedW, sourceFirstH);
        CopySegment(
            _tex, sx, 0, _scratchTex, 0, sourceFirstH,
            sourceFirstW, sourceWrappedH);
        CopySegment(
            _tex, 0, 0, _scratchTex, sourceFirstW, sourceFirstH,
            sourceWrappedW, sourceWrappedH);

        int destFirstW = Math.Min(w, VramShadow.Width - dx);
        int destWrappedW = w - destFirstW;
        int destFirstH = Math.Min(h, VramShadow.Height - dy);
        int destWrappedH = h - destFirstH;
        CopySegment(
            _scratchTex, 0, 0, _tex, dx, dy,
            destFirstW, destFirstH);
        CopySegment(
            _scratchTex, destFirstW, 0, _tex, 0, dy,
            destWrappedW, destFirstH);
        CopySegment(
            _scratchTex, 0, destFirstH, _tex, dx, 0,
            destFirstW, destWrappedH);
        CopySegment(
            _scratchTex, destFirstW, destFirstH, _tex, 0, 0,
            destWrappedW, destWrappedH);
    }

    void CopySegment(
        uint sourceTexture,
        int sourceX,
        int sourceY,
        uint destTexture,
        int destX,
        int destY,
        int width,
        int height)
    {
        if (width <= 0 || height <= 0) return;
        _gl.CopyImageSubData(
            sourceTexture, CopyImageSubDataTarget.Texture2D, 0,
            sourceX * Scale, sourceY * Scale, 0,
            destTexture, CopyImageSubDataTarget.Texture2D, 0,
            destX * Scale, destY * Scale, 0,
            (uint)(width * Scale), (uint)(height * Scale), 1);
    }

    public void ReadRect(int x, int y, int w, int h, Span<ushort> dst)
    {
        if (w <= 0 || h <= 0) return;
        x &= VramShadow.Width - 1;
        y &= VramShadow.Height - 1;
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _fbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _stageFbo);
        int firstW = Math.Min(w, VramShadow.Width - x);
        int wrappedW = w - firstW;
        int firstH = Math.Min(h, VramShadow.Height - y);
        int wrappedH = h - firstH;
        ReadSegment(x, y, 0, 0, firstW, firstH);
        ReadSegment(0, y, firstW, 0, wrappedW, firstH);
        ReadSegment(x, 0, 0, firstH, firstW, wrappedH);
        ReadSegment(0, 0, firstW, firstH, wrappedW, wrappedH);

        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _stageFbo);
        int count = w * h;
        if (_readRgba.Length < count * 4)
            _readRgba = new byte[count * 4];
        _gl.PixelStore(PixelStoreParameter.PackAlignment, 1);
        _gl.ReadPixels<byte>(0, 0, (uint)w, (uint)h, PixelFormat.Rgba, PixelType.UnsignedByte,
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

    void ReadSegment(
        int sourceX,
        int sourceY,
        int destX,
        int destY,
        int width,
        int height)
    {
        if (width <= 0 || height <= 0) return;
        _gl.BlitFramebuffer(
            sourceX * Scale, sourceY * Scale,
            (sourceX + width) * Scale, (sourceY + height) * Scale,
            destX, destY, destX + width, destY + height,
            ClearBufferMask.ColorBufferBit,
            BlitFramebufferFilter.Nearest);
    }

    static byte Expand5(int value) => (byte)((value << 3) | (value >> 2));

    void EnsureScratch()
    {
        if (_scratchTex != 0) return;
        _scratchTex = CreateTex(Width, Height);
    }

    public void Dispose()
    {
        if (_probeBuffer != 0) _gl.DeleteBuffer(_probeBuffer);
        if (_probeFbo != 0) _gl.DeleteFramebuffer(_probeFbo);
        if (_probeTex != 0) _gl.DeleteTexture(_probeTex);
        if (_fbo != 0) _gl.DeleteFramebuffer(_fbo);
        if (_stageFbo != 0) _gl.DeleteFramebuffer(_stageFbo);
        if (_tex != 0) _gl.DeleteTexture(_tex);
        if (_stageTex != 0) _gl.DeleteTexture(_stageTex);
        if (_scratchTex != 0) _gl.DeleteTexture(_scratchTex);
    }
}
