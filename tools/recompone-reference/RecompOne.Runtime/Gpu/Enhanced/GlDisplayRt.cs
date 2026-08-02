using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

public sealed class GlDisplayRt
{
    public int X, Y, W, H;
    public int Margin;
    public uint Tex, Fbo, Depth, MsaaFbo, MsaaColor, MsaaDepth;
    public int Samples;
    public bool Dirty;
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

    public void Create(GL gl)
    {
        Samples = Math.Clamp(Config.ConfigManager.View.MsaaSamples, 0, 8);
        if (Samples == 1) Samples = 0;
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
            RenderbufferTarget.Renderbuffer, InternalFormat.DepthComponent24,
            (uint)TexW, (uint)TexH);
        gl.FramebufferRenderbuffer(
            FramebufferTarget.Framebuffer, FramebufferAttachment.DepthAttachment,
            RenderbufferTarget.Renderbuffer, Depth);
        gl.ClearColor(0f, 0f, 0f, 0f);
        gl.ClearDepth(1.0);
        gl.Disable(EnableCap.ScissorTest);
        gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

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
                InternalFormat.DepthComponent24, (uint)TexW, (uint)TexH);
            gl.FramebufferRenderbuffer(
                FramebufferTarget.Framebuffer,
                FramebufferAttachment.DepthAttachment,
                RenderbufferTarget.Renderbuffer, MsaaDepth);
            gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
        }
    }

    public uint DrawFbo => MsaaFbo != 0 ? MsaaFbo : Fbo;

    public void Resolve(GL gl)
    {
        if (MsaaFbo == 0) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, MsaaFbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, Fbo);
        gl.BlitFramebuffer(0, 0, TexW, TexH, 0, 0, TexW, TexH,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
    }

    public void CopyResolveToMsaa(GL gl)
    {
        if (MsaaFbo == 0) return;
        gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, Fbo);
        gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, MsaaFbo);
        gl.BlitFramebuffer(0, 0, TexW, TexH, 0, 0, TexW, TexH,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
    }

    public void ClearDepth(GL gl)
    {
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, DrawFbo);
        gl.Disable(EnableCap.ScissorTest);
        gl.DepthMask(true);
        gl.ClearDepth(1.0);
        gl.Clear(ClearBufferMask.DepthBufferBit);
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
    }
}
