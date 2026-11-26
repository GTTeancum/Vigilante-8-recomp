using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

internal sealed class GlResourceSetupException(string message) : InvalidOperationException(message);

// Allocation-time checks, not per-draw polling. Do not publish a render target
// until the driver has accepted its storage and completed its framebuffer.
internal static class GlResourceSetup
{
    internal static void RequireTextureSize(GL gl, int width, int height, string target)
    {
        gl.GetInteger(GLEnum.MaxTextureSize, out int limit);
        if (width <= 0 || height <= 0 || width > limit || height > limit)
            throw new GlResourceSetupException(
                $"{target}: requested texture {width}x{height}; OpenGL limit is {limit}.");
    }

    internal static void RequireNoError(GL gl, string stage)
    {
        var error = gl.GetError();
        if (error != GLEnum.NoError)
            throw new GlResourceSetupException($"{stage}: OpenGL error {error} (0x{(uint)error:X}).");
    }

    // The caller owns the texture and the framebuffer handles. This method
    // binds the FBO only after level zero has storage.
    internal static void AttachColor(GL gl, uint fbo, uint texture)
    {
        gl.BindFramebuffer(FramebufferTarget.Framebuffer, fbo);
        gl.FramebufferTexture2D(FramebufferTarget.Framebuffer,
            FramebufferAttachment.ColorAttachment0, TextureTarget.Texture2D, texture, 0);
    }

    internal static void RequireComplete(GL gl, string target)
    {
        RequireNoError(gl, $"{target} attachment");
        var status = gl.CheckFramebufferStatus(FramebufferTarget.Framebuffer);
        if (status != GLEnum.FramebufferComplete)
            throw new GlResourceSetupException(
                $"{target}: incomplete OpenGL framebuffer ({status}, 0x{(uint)status:X}).");
    }
}
