using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

public sealed class GlDisplayRt
{
    public int X, Y, W, H;
    public int Margin;
    public uint Tex, Fbo, Depth, MsaaFbo, MsaaColor, MsaaDepth;
    public int Samples;
    public bool Dirty;
    public bool NeedsResolve;
    public long Stamp;
    public long LastDrawFrame;

    public int Wide1x => W + Margin * 2;
    public int TexW => Wide1x * GlVram.Scale;
    public int TexH => H * GlVram.Scale;

    public bool Contains(int cx0, int cy0, int cx1, int cy1)
        => cx0 >= X && cx1 <= X + W - 1 && cy0 >= Y && cy1 <= Y + H - 1;

    public bool Covers(int cx0, int cy0, int cx1, int cy1)
        => cx0 <= X && cx1 >= X + W - 1 && cy0 <= Y && cy1 >= Y + H - 1;

    public bool Intersects(int rx, int ry, int rw, int rh)
        => rx < X + W && X < rx + rw && ry < Y + H && Y < ry + rh;

    // A multisample framebuffer needs the SAME supported count on its
    // colour and depth/stencil renderbuffers. The global GL_MAX_SAMPLES
    // alone is not a guarantee for either internal format.
    internal static int[] SupportedSampleCounts(GL gl)
    {
        int[] Counts(GLEnum format)
        {
            gl.GetInternalformat(GLEnum.Renderbuffer, format, GLEnum.NumSampleCounts, 1, out int count);
            if (count <= 0) return [];
            int[] counts = new int[count];
            gl.GetInternalformat(GLEnum.Renderbuffer, format, GLEnum.Samples, (uint)count, counts.AsSpan());
            return counts;
        }
        return Counts(GLEnum.Rgba8).Intersect(Counts(GLEnum.Depth24Stencil8))
            .Where(n => n is >= 2 and <= 8).Order().ToArray();
    }

    internal static int ResolveSampleCount(int requested, ReadOnlySpan<int> supported)
    {
        if (requested <= 1 || supported.IsEmpty) return 0;
        for (int i = supported.Length - 1; i >= 0; i--)
            if (supported[i] <= requested) return supported[i];
        // Some drivers expose only 4x and round a 2x request up to 4x.
        return supported[0];
    }

    public void Create(GL gl) => Create(gl,
        ResolveSampleCount(Config.ConfigManager.View.MsaaSamples, SupportedSampleCounts(gl)));

    internal void Create(GL gl, int samples)
    {
        Samples = samples;
        Tex = gl.GenTexture();
        gl.BindTexture(TextureTarget.Texture2D, Tex);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Nearest);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Nearest);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        gl.TexImage2D<ushort>(TextureTarget.Texture2D, 0, InternalFormat.Rgb5A1, (uint)TexW, (uint)TexH, 0,
            PixelFormat.Rgba, PixelType.UnsignedShort1555Rev, new ushort[TexW * TexH].AsSpan());

        Fbo = gl.GenFramebuffer();
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, Fbo);
        gl.FramebufferTexture2D(FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0,
            TextureTarget.Texture2D, Tex, 0);
        Depth = gl.GenRenderbuffer();
        gl.BindRenderbuffer(RenderbufferTarget.Renderbuffer, Depth);
        gl.RenderbufferStorage(
            RenderbufferTarget.Renderbuffer, InternalFormat.Depth24Stencil8,
            (uint)TexW, (uint)TexH);
        gl.FramebufferRenderbuffer(
            FramebufferTarget.Framebuffer, FramebufferAttachment.DepthStencilAttachment,
            RenderbufferTarget.Renderbuffer, Depth);
        gl.ClearColor(0f, 0f, 0f, 0f);
        gl.ClearDepth(1.0);
        gl.ClearStencil(0);
        gl.StencilMask(0xFF);
        gl.Disable(EnableCap.ScissorTest);
        gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit | ClearBufferMask.StencilBufferBit);

        if (Samples > 1)
        {
            MsaaColor = gl.GenRenderbuffer();
            gl.BindRenderbuffer(RenderbufferTarget.Renderbuffer, MsaaColor);
            gl.RenderbufferStorageMultisample(RenderbufferTarget.Renderbuffer, (uint)Samples,
                InternalFormat.Rgba8, (uint)TexW, (uint)TexH);
            MsaaFbo = gl.GenFramebuffer();
            gl.BindFramebuffer(FramebufferTarget.Framebuffer, MsaaFbo);
            gl.FramebufferRenderbuffer(FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0,
                RenderbufferTarget.Renderbuffer, MsaaColor);
            MsaaDepth = gl.GenRenderbuffer();
            gl.BindRenderbuffer(RenderbufferTarget.Renderbuffer, MsaaDepth);
            gl.RenderbufferStorageMultisample(
                RenderbufferTarget.Renderbuffer, (uint)Samples,
                InternalFormat.Depth24Stencil8, (uint)TexW, (uint)TexH);
            gl.FramebufferRenderbuffer(
                FramebufferTarget.Framebuffer,
                FramebufferAttachment.DepthStencilAttachment,
                RenderbufferTarget.Renderbuffer, MsaaDepth);
            gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit | ClearBufferMask.StencilBufferBit);
        }
    }

    public uint DrawFbo => MsaaFbo != 0 ? MsaaFbo : Fbo;

    public void Resolve(GL gl)
    {
        if (MsaaFbo == 0 || !NeedsResolve) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, MsaaFbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, Fbo);
        gl.BlitFramebuffer(0, 0, TexW, TexH, 0, 0, TexW, TexH,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        NeedsResolve = false;
    }

    public void ResolveRegion(GL gl, int x0, int y0, int x1, int y1)
    {
        if (MsaaFbo == 0 || !NeedsResolve || x0 >= x1 || y0 >= y1) return;
        int s = GlVram.Scale;
        x0 = Math.Clamp(x0 * s, 0, TexW);
        y0 = Math.Clamp(y0 * s, 0, TexH);
        x1 = Math.Clamp(x1 * s, 0, TexW);
        y1 = Math.Clamp(y1 * s, 0, TexH);
        if (x0 >= x1 || y0 >= y1) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, MsaaFbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, Fbo);
        gl.BlitFramebuffer(
            x0, y0, x1, y1,
            x0, y0, x1, y1,
            ClearBufferMask.ColorBufferBit,
            BlitFramebufferFilter.Nearest);
    }

    public void CopyResolveToMsaa(GL gl)
    {
        if (MsaaFbo == 0) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, Fbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, MsaaFbo);
        gl.BlitFramebuffer(0, 0, TexW, TexH, 0, 0, TexW, TexH,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        NeedsResolve = false;
    }

    public void CopyRegionToMsaa(
        GL gl, int x0, int y0, int x1, int y1)
    {
        if (MsaaFbo == 0 || x0 >= x1 || y0 >= y1) return;
        int s = GlVram.Scale;
        x0 = Math.Clamp(x0 * s, 0, TexW);
        y0 = Math.Clamp(y0 * s, 0, TexH);
        x1 = Math.Clamp(x1 * s, 0, TexW);
        y1 = Math.Clamp(y1 * s, 0, TexH);
        if (x0 >= x1 || y0 >= y1) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, Fbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, MsaaFbo);
        gl.BlitFramebuffer(
            x0, y0, x1, y1,
            x0, y0, x1, y1,
            ClearBufferMask.ColorBufferBit,
            BlitFramebufferFilter.Nearest);
    }

    public void ClearDepth(GL gl)
    {
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, DrawFbo);
        gl.Disable(EnableCap.ScissorTest);
        gl.DepthMask(true);
        gl.ClearDepth(1.0);
        gl.ClearStencil(0);
        gl.StencilMask(0xFF);
        gl.Clear(ClearBufferMask.DepthBufferBit | ClearBufferMask.StencilBufferBit);
    }

    public void Destroy(GL gl)
    {
        if (Fbo != 0) gl.DeleteFramebuffer(Fbo);
        if (MsaaFbo != 0) gl.DeleteFramebuffer(MsaaFbo);
        if (MsaaColor != 0) gl.DeleteRenderbuffer(MsaaColor);
        if (MsaaDepth != 0) gl.DeleteRenderbuffer(MsaaDepth);
        if (Depth != 0) gl.DeleteRenderbuffer(Depth);
        if (Tex != 0) gl.DeleteTexture(Tex);
        Fbo = Tex = Depth = MsaaFbo = MsaaColor = MsaaDepth = 0;
        NeedsResolve = false;
    }
}
