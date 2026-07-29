using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Host.Window;

// Final host-only presentation pass. The source texture is always the completed
// PS1 framebuffer; this class cannot affect emulated VRAM or game state.
internal sealed class PresentationRenderer : IDisposable
{
    const string UpscaleFs = """
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uSource;
        uniform sampler2D uBanner;
        uniform vec2 uSourceSize;
        uniform ivec2 uBannerSize;
        uniform int uBannerActive;
        out vec4 oColor;
        void main() {
            ivec2 size = ivec2(uSourceSize);
            ivec2 p = clamp(ivec2(vUv * uSourceSize), ivec2(0), size - 1);
            ivec2 nativeP = clamp(
                ivec2(vUv * vec2(640.0, 480.0)),
                ivec2(0), ivec2(639, 479));
            vec3 base = texelFetch(uSource, p, 0).rgb;
            if (uBannerActive != 0) {
                if (all(greaterThanEqual(nativeP, ivec2(0))) &&
                    all(lessThan(nativeP, uBannerSize)))
                    base = texelFetch(uBanner, nativeP, 0).rgb;
                // Imported V8 vehicles are standard-only. The rest of the
                // native V8:2 control strip remains pixel-for-pixel intact.
                if (nativeP.x >= 340 && nativeP.x < 455 &&
                    nativeP.y >= 423)
                    base = vec3(0.0);
            }
            oColor = vec4(base, 1.0);
        }
        """;

    const string FxaaFs = """
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uSource;
        uniform vec2 uSourceSize;
        uniform vec2 uInvResolution;
        out vec4 oColor;

        vec3 sampleLinear(vec2 uv) {
            vec2 p = clamp(uv, vec2(0.0), vec2(1.0)) * uSourceSize - 0.5;
            ivec2 i0 = ivec2(floor(p));
            vec2 f = fract(p);
            ivec2 hi = ivec2(uSourceSize) - 1;
            i0 = clamp(i0, ivec2(0), hi);
            ivec2 i1 = min(i0 + 1, hi);
            vec3 a = mix(texelFetch(uSource, ivec2(i0.x, i0.y), 0).rgb,
                         texelFetch(uSource, ivec2(i1.x, i0.y), 0).rgb, f.x);
            vec3 b = mix(texelFetch(uSource, ivec2(i0.x, i1.y), 0).rgb,
                         texelFetch(uSource, ivec2(i1.x, i1.y), 0).rgb, f.x);
            return mix(a, b, f.y);
        }

        float luma(vec3 rgb) { return dot(rgb, vec3(0.299, 0.587, 0.114)); }

        void main() {
            vec3 nw = sampleLinear(vUv + vec2(-1.0, -1.0) * uInvResolution);
            vec3 ne = sampleLinear(vUv + vec2( 1.0, -1.0) * uInvResolution);
            vec3 sw = sampleLinear(vUv + vec2(-1.0,  1.0) * uInvResolution);
            vec3 se = sampleLinear(vUv + vec2( 1.0,  1.0) * uInvResolution);
            vec3 m  = sampleLinear(vUv);

            float lumaNW = luma(nw), lumaNE = luma(ne);
            float lumaSW = luma(sw), lumaSE = luma(se), lumaM = luma(m);
            float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
            float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

            vec2 dir;
            dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
            dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
            float reduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 / 8.0), 1.0 / 128.0);
            float reciprocal = 1.0 / (min(abs(dir.x), abs(dir.y)) + reduce);
            dir = clamp(dir * reciprocal, vec2(-8.0), vec2(8.0)) * uInvResolution;

            vec3 a = 0.5 * (sampleLinear(vUv + dir * (1.0 / 3.0 - 0.5)) +
                            sampleLinear(vUv + dir * (2.0 / 3.0 - 0.5)));
            vec3 b = a * 0.5 + 0.25 * (sampleLinear(vUv + dir * -0.5) +
                                      sampleLinear(vUv + dir *  0.5));
            float lumaB = luma(b);
            oColor = vec4((lumaB < lumaMin || lumaB > lumaMax) ? a : b, 1.0);
        }
        """;

    readonly GL _gl;
    uint _vao, _vbo, _upscaleProgram, _fxaaProgram;
    uint _upscaleTexture, _fxaaTexture, _upscaleFbo, _fxaaFbo, _bannerTexture;
    int _width, _height;
    int _bannerWidth = 1, _bannerHeight = 1;
    string? _bannerPath;
    int _lastSourceWidth, _lastSourceHeight, _lastOutputWidth, _lastOutputHeight;
    bool _lastFxaa;
    int _upscaleSourceSize;
    int _upscaleBannerSize, _upscaleBannerActive;
    int _fxaaSourceSize, _fxaaInvResolution;

    public bool Ready { get; private set; }

    public PresentationRenderer(GL gl) => _gl = gl;

    public unsafe void Initialize()
    {
        _upscaleProgram = Hle.GlShaders.Build(_gl, Hle.GlShaders.FullscreenVs, UpscaleFs, "presentation-upscale");
        _fxaaProgram = Hle.GlShaders.Build(_gl, Hle.GlShaders.FullscreenVs, FxaaFs, "presentation-fxaa");
        if (_upscaleProgram == 0 || _fxaaProgram == 0) return;

        _gl.UseProgram(_upscaleProgram);
        _gl.Uniform1(_gl.GetUniformLocation(_upscaleProgram, "uSource"), 0);
        _gl.Uniform1(_gl.GetUniformLocation(_upscaleProgram, "uBanner"), 1);
        _upscaleSourceSize = _gl.GetUniformLocation(_upscaleProgram, "uSourceSize");
        _upscaleBannerSize = _gl.GetUniformLocation(_upscaleProgram, "uBannerSize");
        _upscaleBannerActive = _gl.GetUniformLocation(_upscaleProgram, "uBannerActive");
        _gl.UseProgram(_fxaaProgram);
        _gl.Uniform1(_gl.GetUniformLocation(_fxaaProgram, "uSource"), 0);
        _fxaaSourceSize = _gl.GetUniformLocation(_fxaaProgram, "uSourceSize");
        _fxaaInvResolution = _gl.GetUniformLocation(_fxaaProgram, "uInvResolution");

        _vao = _gl.GenVertexArray();
        _vbo = _gl.GenBuffer();
        _gl.BindVertexArray(_vao);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        float[] quad = [-1f, -1f, 1f, -1f, -1f, 1f, 1f, 1f];
        fixed (float* vertices = quad)
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(quad.Length * sizeof(float)), vertices, BufferUsageARB.StaticDraw);
        _gl.EnableVertexAttribArray(0);
        _gl.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), (void*)0);

        (_upscaleTexture, _upscaleFbo) = CreateTarget();
        (_fxaaTexture, _fxaaFbo) = CreateTarget();
        _bannerTexture = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, _bannerTexture);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        byte[] black = [0, 0, 0];
        _gl.TexImage2D<byte>(TextureTarget.Texture2D, 0, InternalFormat.Rgb, 1, 1, 0,
            PixelFormat.Rgb, PixelType.UnsignedByte, black);
        EnsureSize(1, 1);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        Ready = true;
    }

    (uint texture, uint fbo) CreateTarget()
    {
        uint texture = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, texture);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Nearest);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.ClampToEdge);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.ClampToEdge);
        uint fbo = _gl.GenFramebuffer();
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, fbo);
        _gl.FramebufferTexture2D(FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0,
            TextureTarget.Texture2D, texture, 0);
        return (texture, fbo);
    }

    unsafe void EnsureSize(int width, int height)
    {
        if (width == _width && height == _height) return;
        foreach (uint texture in new[] { _upscaleTexture, _fxaaTexture })
        {
            _gl.BindTexture(TextureTarget.Texture2D, texture);
            _gl.TexImage2D(TextureTarget.Texture2D, 0, InternalFormat.Rgba8, (uint)width, (uint)height, 0,
                PixelFormat.Rgba, PixelType.UnsignedByte, null);
        }
        _width = width;
        _height = height;
    }

    public uint Render(uint sourceTexture, int sourceWidth, int sourceHeight,
        int outputWidth, int outputHeight, bool fxaa, string? captureLabel = null)
    {
        if (!Ready || sourceTexture == 0 || sourceWidth <= 0 || sourceHeight <= 0)
            return sourceTexture;

        outputWidth = Math.Clamp(outputWidth, 1, 8192);
        outputHeight = Math.Clamp(outputHeight, 1, 8192);
        EnsureSize(outputWidth, outputHeight);
        if (sourceWidth != _lastSourceWidth || sourceHeight != _lastSourceHeight ||
            outputWidth != _lastOutputWidth || outputHeight != _lastOutputHeight || fxaa != _lastFxaa)
        {
            Console.WriteLine($"[Host] presentation source={sourceWidth}x{sourceHeight} output={outputWidth}x{outputHeight} aa={(fxaa ? "FXAA" : "Off")}");
            _lastSourceWidth = sourceWidth;
            _lastSourceHeight = sourceHeight;
            _lastOutputWidth = outputWidth;
            _lastOutputHeight = outputHeight;
            _lastFxaa = fxaa;
        }

        bool bannerActive = UpdateNativeSelectorBanner();
        PreparePass(_upscaleFbo, _upscaleProgram, sourceTexture);
        _gl.Uniform2(_upscaleSourceSize, (float)sourceWidth, sourceHeight);
        _gl.Uniform2(_upscaleBannerSize, _bannerWidth, _bannerHeight);
        _gl.Uniform1(_upscaleBannerActive, bannerActive ? 1 : 0);
        _gl.ActiveTexture(TextureUnit.Texture1);
        _gl.BindTexture(TextureTarget.Texture2D, _bannerTexture);
        _gl.ActiveTexture(TextureUnit.Texture0);
        _gl.DrawArrays(PrimitiveType.TriangleStrip, 0, 4);

        uint finalTexture = _upscaleTexture;
        uint finalFbo = _upscaleFbo;
        if (fxaa)
        {
            PreparePass(_fxaaFbo, _fxaaProgram, _upscaleTexture);
            _gl.Uniform2(_fxaaSourceSize, (float)outputWidth, outputHeight);
            _gl.Uniform2(_fxaaInvResolution, 1f / outputWidth, 1f / outputHeight);
            _gl.DrawArrays(PrimitiveType.TriangleStrip, 0, 4);
            finalTexture = _fxaaTexture;
            finalFbo = _fxaaFbo;
        }

        if (!string.IsNullOrEmpty(captureLabel))
            CapturePpm(finalFbo, outputWidth, outputHeight, captureLabel, fxaa);

        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        return finalTexture;
    }

    bool UpdateNativeSelectorBanner()
    {
        string? path = Sdk.V82VehicleRegistry.NativeSelectorBannerPath;
        if (string.IsNullOrEmpty(path) || !File.Exists(path))
            return false;
        if (path.Equals(_bannerPath, StringComparison.OrdinalIgnoreCase))
            return true;

        byte[] file = File.ReadAllBytes(path);
        int cursor = 0;
        string magic = ReadPpmToken(file, ref cursor);
        int width = int.Parse(ReadPpmToken(file, ref cursor));
        int height = int.Parse(ReadPpmToken(file, ref cursor));
        int maximum = int.Parse(ReadPpmToken(file, ref cursor));
        if (cursor >= file.Length || !char.IsWhiteSpace((char)file[cursor]))
            throw new InvalidDataException(
                $"native selector banner has no PPM header terminator: {path}");
        cursor++;
        int length = checked(width * height * 3);
        if (magic != "P6" || width != 260 || height != 422 ||
            maximum != 255 || cursor + length != file.Length)
            throw new InvalidDataException(
                $"native selector banner has invalid PPM layout: {path}");

        _gl.ActiveTexture(TextureUnit.Texture1);
        _gl.BindTexture(TextureTarget.Texture2D, _bannerTexture);
        _gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
        _gl.TexImage2D<byte>(
            TextureTarget.Texture2D, 0, InternalFormat.Rgb,
            (uint)width, (uint)height, 0, PixelFormat.Rgb,
            PixelType.UnsignedByte, file.AsSpan(cursor, length));
        _gl.ActiveTexture(TextureUnit.Texture0);
        _bannerWidth = width;
        _bannerHeight = height;
        _bannerPath = path;
        Console.WriteLine(
            $"[Host] loaded exact native V8 selector banner '{Path.GetFileName(path)}'");
        return true;
    }

    static string ReadPpmToken(byte[] data, ref int cursor)
    {
        while (cursor < data.Length && char.IsWhiteSpace((char)data[cursor]))
            cursor++;
        int start = cursor;
        while (cursor < data.Length && !char.IsWhiteSpace((char)data[cursor]))
            cursor++;
        if (start == cursor)
            throw new InvalidDataException("native selector PPM is truncated");
        return System.Text.Encoding.ASCII.GetString(data, start, cursor - start);
    }

    void PreparePass(uint fbo, uint program, uint sourceTexture)
    {
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, fbo);
        _gl.Viewport(0, 0, (uint)_width, (uint)_height);
        _gl.Disable(EnableCap.DepthTest);
        _gl.Disable(EnableCap.Blend);
        _gl.Disable(EnableCap.ScissorTest);
        _gl.Disable(EnableCap.CullFace);
        _gl.UseProgram(program);
        _gl.BindVertexArray(_vao);
        _gl.ActiveTexture(TextureUnit.Texture0);
        _gl.BindTexture(TextureTarget.Texture2D, sourceTexture);
    }

    void CapturePpm(uint fbo, int width, int height, string label, bool fxaa)
    {
        // The source upload stores its first (top) scanline at texture row zero.
        // The two fullscreen passes preserve that convention, so GL readback is
        // already in the top-to-bottom order expected by PPM.
        byte[] pixels = new byte[width * height * 3];
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, fbo);
        _gl.PixelStore(PixelStoreParameter.PackAlignment, 1);
        _gl.ReadPixels(0, 0, (uint)width, (uint)height, PixelFormat.Rgb, PixelType.UnsignedByte, pixels.AsSpan());

        string mode = fxaa ? "fxaa" : "off";
        string path = $"recompone_present_{label}_{width}x{height}_{mode}.ppm";
        using var output = File.Create(path);
        byte[] header = System.Text.Encoding.ASCII.GetBytes($"P6\n{width} {height}\n255\n");
        output.Write(header);
        output.Write(pixels);
        Console.WriteLine($"[Host] captured presentation '{label}' at {width}x{height} aa={mode} to {path}");
    }

    public void Dispose()
    {
        if (_vbo != 0) _gl.DeleteBuffer(_vbo);
        if (_vao != 0) _gl.DeleteVertexArray(_vao);
        if (_upscaleProgram != 0) _gl.DeleteProgram(_upscaleProgram);
        if (_fxaaProgram != 0) _gl.DeleteProgram(_fxaaProgram);
        if (_upscaleTexture != 0) _gl.DeleteTexture(_upscaleTexture);
        if (_fxaaTexture != 0) _gl.DeleteTexture(_fxaaTexture);
        if (_bannerTexture != 0) _gl.DeleteTexture(_bannerTexture);
        if (_upscaleFbo != 0) _gl.DeleteFramebuffer(_upscaleFbo);
        if (_fxaaFbo != 0) _gl.DeleteFramebuffer(_fxaaFbo);
    }
}
