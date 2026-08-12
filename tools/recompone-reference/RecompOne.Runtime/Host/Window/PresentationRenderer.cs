using Silk.NET.OpenGL;
using RecompOne.Runtime.Config;

namespace RecompOne.Runtime.Host.Window;

// Final host-only presentation pass. The source texture is always the completed
// PS1 framebuffer; this class cannot affect emulated VRAM or game state.
internal sealed class PresentationRenderer : IDisposable
{
    const string UpscaleFs = """
        #version 330 core
        in vec2 vUv;
        uniform sampler2D uSource;
        uniform vec2 uSourceSize;
        uniform int uDedither;
        uniform int uDeditherStep;
        uniform int uLinearFilter;
        uniform int uLoadingUiRestore;
        uniform sampler2D uLoadingCard;
        uniform int uLoadingCardOverlay;
        uniform vec4 uLoadingCardRect;
        out vec4 oColor;

        vec3 sourcePixel(ivec2 p) {
            return texelFetch(
                uSource,
                clamp(p, ivec2(0), ivec2(uSourceSize) - 1),
                0).rgb;
        }

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

        void main() {
            ivec2 size = ivec2(uSourceSize);
            ivec2 p = clamp(ivec2(vUv * uSourceSize), ivec2(0), size - 1);
            vec3 center = uLinearFilter != 0
                ? sampleLinear(vUv)
                : sourcePixel(p);
            bool loadingCardPixel = false;
            if (uLoadingCardOverlay != 0) {
                vec2 innerUv = (vUv - uLoadingCardRect.xy) / uLoadingCardRect.zw;
                if (innerUv.x >= 0.0 && innerUv.x <= 1.0 &&
                    innerUv.y >= 0.0 && innerUv.y <= 1.0) {
                    bool preserveTitle =
                        vUv.x < 0.55 && vUv.y < 0.20;
                    if (!preserveTitle) {
                        vec2 cardUv =
                            vec2(80.0 / 1280.0, 32.0 / 384.0) +
                            innerUv * vec2(1120.0 / 1280.0, 320.0 / 384.0);
                        center = texture(uLoadingCard, cardUv).rgb;
                        loadingCardPixel = true;
                    }
                }
            }
            if (uDedither == 0) {
                if (uLoadingUiRestore != 0 && !loadingCardPixel) {
                    vec2 texel = 1.0 / uSourceSize;
                    vec3 n  = sampleLinear(vUv + vec2( 0.0, -texel.y));
                    vec3 e  = sampleLinear(vUv + vec2( texel.x,  0.0));
                    vec3 s  = sampleLinear(vUv + vec2( 0.0,  texel.y));
                    vec3 w  = sampleLinear(vUv + vec2(-texel.x,  0.0));
                    vec3 ne = sampleLinear(vUv + vec2( texel.x, -texel.y));
                    vec3 se = sampleLinear(vUv + vec2( texel.x,  texel.y));
                    vec3 sw = sampleLinear(vUv + vec2(-texel.x,  texel.y));
                    vec3 nw = sampleLinear(vUv + vec2(-texel.x, -texel.y));

                    vec3 lo = min(center, min(min(n, e), min(s, w)));
                    lo = min(lo, min(min(ne, se), min(sw, nw)));
                    vec3 hi = max(center, max(max(n, e), max(s, w)));
                    hi = max(hi, max(max(ne, se), max(sw, nw)));
                    float localRange = max(
                        hi.r - lo.r,
                        max(hi.g - lo.g, hi.b - lo.b));

                    vec3 average =
                        (center * 4.0 + n + e + s + w +
                         (ne + se + sw + nw) * 0.5) / 10.0;
                    float restore = 1.0 - smoothstep(0.09, 0.32, localRange);
                    vec3 denoised = mix(center, average, restore * 0.65);
                    vec3 blur = (denoised * 4.0 + n + e + s + w) / 8.0;
                    vec3 sharpened = denoised + (denoised - blur) *
                        (0.45 * restore);
                    oColor = vec4(clamp(sharpened, lo - 0.025, hi + 0.025), 1.0);
                    return;
                }
                oColor = vec4(center, 1.0);
                return;
            }

            int s = max(uDeditherStep, 1);
            vec3 n  = sourcePixel(p + ivec2( 0, -s));
            vec3 ne = sourcePixel(p + ivec2( s, -s));
            vec3 e  = sourcePixel(p + ivec2( s,  0));
            vec3 se = sourcePixel(p + ivec2( s,  s));
            vec3 so = sourcePixel(p + ivec2( 0,  s));
            vec3 sw = sourcePixel(p + ivec2(-s,  s));
            vec3 w  = sourcePixel(p + ivec2(-s,  0));
            vec3 nw = sourcePixel(p + ivec2(-s, -s));

            vec3 lo = min(center, min(min(n, ne), min(e, se)));
            lo = min(lo, min(min(so, sw), min(w, nw)));
            vec3 hi = max(center, max(max(n, ne), max(e, se)));
            hi = max(hi, max(max(so, sw), max(w, nw)));
            float localRange = max(
                hi.r - lo.r,
                max(hi.g - lo.g, hi.b - lo.b));

            // A PS1 ordered-dither cell is a low-amplitude, high-frequency
            // variation. Average that variation only inside low-contrast
            // regions; geometry, text, HUD edges and texture detail exceed
            // the upper threshold and remain untouched.
            vec3 average = (center * 4.0 + n + ne + e + se + so + sw + w + nw)
                / 12.0;
            float strength =
                1.0 - smoothstep(0.045, 0.14, localRange);
            oColor = vec4(mix(center, average, strength), 1.0);
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
    uint _upscaleTexture, _fxaaTexture, _upscaleFbo, _fxaaFbo;
    uint _loadingCardTexture;
    int _loadingCardWidth, _loadingCardHeight;
    int _width, _height;
    int _lastSourceWidth, _lastSourceHeight, _lastOutputWidth, _lastOutputHeight;
    bool _lastFxaa;
    int _upscaleSourceSize, _upscaleDedither, _upscaleDeditherStep;
    int _upscaleLinearFilter, _upscaleLoadingUiRestore;
    int _upscaleLoadingCardOverlay, _upscaleLoadingCardRect;
    int _fxaaSourceSize, _fxaaInvResolution;

    public bool Ready { get; private set; }

    public PresentationRenderer(GL gl) => _gl = gl;

    public unsafe void Initialize()
    {
        _upscaleProgram = Enhanced.GlShaders.Build(
            _gl,
            Enhanced.GlShaders.FullscreenVs,
            UpscaleFs,
            "presentation-upscale");
        _fxaaProgram = Enhanced.GlShaders.Build(
            _gl,
            Enhanced.GlShaders.FullscreenVs,
            FxaaFs,
            "presentation-fxaa");
        if (_upscaleProgram == 0 || _fxaaProgram == 0) return;

        _gl.UseProgram(_upscaleProgram);
        _gl.Uniform1(_gl.GetUniformLocation(_upscaleProgram, "uSource"), 0);
        _gl.Uniform1(_gl.GetUniformLocation(_upscaleProgram, "uLoadingCard"), 1);
        _upscaleSourceSize = _gl.GetUniformLocation(_upscaleProgram, "uSourceSize");
        _upscaleDedither =
            _gl.GetUniformLocation(_upscaleProgram, "uDedither");
        _upscaleDeditherStep =
            _gl.GetUniformLocation(_upscaleProgram, "uDeditherStep");
        _upscaleLinearFilter =
            _gl.GetUniformLocation(_upscaleProgram, "uLinearFilter");
        _upscaleLoadingUiRestore =
            _gl.GetUniformLocation(_upscaleProgram, "uLoadingUiRestore");
        _upscaleLoadingCardOverlay =
            _gl.GetUniformLocation(_upscaleProgram, "uLoadingCardOverlay");
        _upscaleLoadingCardRect =
            _gl.GetUniformLocation(_upscaleProgram, "uLoadingCardRect");
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
        LoadLoadingCardOverlay();
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

        PreparePass(_upscaleFbo, _upscaleProgram, sourceTexture);
        _gl.Uniform2(_upscaleSourceSize, (float)sourceWidth, sourceHeight);
        bool dedither =
            !ConfigManager.View.Ps1Dithering &&
            Runtime.GameTitle.Contains(
                "2nd Offense", StringComparison.Ordinal) &&
            RecompOne.Runtime.Hle.GpuHle.Active &&
            RecompOne.Runtime.Hle.GpuHle.GameplayActive;
        bool offGameplayV82Ui =
            ConfigManager.View.HighResolutionTextures &&
            Runtime.GameTitle.Contains(
                "2nd Offense", StringComparison.Ordinal) &&
            RecompOne.Runtime.Hle.GpuHle.Active &&
            !RecompOne.Runtime.Hle.GpuHle.GameplayActive;
        bool preTickLoadingCard =
            ConfigManager.View.HighResolutionTextures &&
            Runtime.GameTitle.Contains(
                "2nd Offense", StringComparison.Ordinal) &&
            RecompOne.Runtime.Hle.GpuHle.Active &&
            RecompOne.Runtime.Hle.GpuHle.GameplayActive &&
            RecompOne.Runtime.Hle.GpuHle.DebugGameplayTick == 0 &&
            sourceWidth >= 1200 && sourceHeight >= 900;
        bool uiPresentation = offGameplayV82Ui || preTickLoadingCard;
        bool loadingUiSource =
            uiPresentation && sourceWidth >= 1200 && sourceHeight >= 900;
        _gl.Uniform1(_upscaleDedither, dedither ? 1 : 0);
        _gl.Uniform1(
            _upscaleDeditherStep,
            ConfigManager.View.HighResolution3D
                ? ConfigManager.View.InternalResolutionScale
                : 1);
        _gl.Uniform1(_upscaleLinearFilter, 0);
        _gl.Uniform1(_upscaleLoadingUiRestore, loadingUiSource ? 1 : 0);
        bool loadingCardOverlay = preTickLoadingCard && _loadingCardTexture != 0;
        _gl.Uniform1(_upscaleLoadingCardOverlay, loadingCardOverlay ? 1 : 0);
        _gl.Uniform4(
            _upscaleLoadingCardRect,
            224f / 1280f,
            90f / 720f,
            832f / 1280f,
            240f / 720f);
        if (loadingCardOverlay)
        {
            _gl.ActiveTexture(TextureUnit.Texture1);
            _gl.BindTexture(TextureTarget.Texture2D, _loadingCardTexture);
            _gl.ActiveTexture(TextureUnit.Texture0);
        }
        _gl.DrawArrays(PrimitiveType.TriangleStrip, 0, 4);

        uint finalTexture = _upscaleTexture;
        uint finalFbo = _upscaleFbo;
        bool finalFxaa = fxaa && !uiPresentation;
        if (finalFxaa)
        {
            PreparePass(_fxaaFbo, _fxaaProgram, _upscaleTexture);
            _gl.Uniform2(_fxaaSourceSize, (float)outputWidth, outputHeight);
            _gl.Uniform2(_fxaaInvResolution, 1f / outputWidth, 1f / outputHeight);
            _gl.DrawArrays(PrimitiveType.TriangleStrip, 0, 4);
            finalTexture = _fxaaTexture;
            finalFbo = _fxaaFbo;
        }

        if (!string.IsNullOrEmpty(captureLabel))
            CapturePpm(finalFbo, outputWidth, outputHeight, captureLabel, finalFxaa);

        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        return finalTexture;
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

    void LoadLoadingCardOverlay()
    {
        string? explicitPath =
            Environment.GetEnvironmentVariable("RECOMPONE_ROUTE66_LOADING_CARD_PPM");
        string? looseRoot = Runtime.ResolveLoosePath();
        string? path = !string.IsNullOrWhiteSpace(explicitPath)
            ? explicitPath
            : looseRoot == null
                ? null
                : Path.Combine(
                    looseRoot,
                    "mods",
                    "enhanced_textures_2x",
                    "loading_cards",
                    "route66_loading_card_4x.ppm");
        if (string.IsNullOrWhiteSpace(path) || !File.Exists(path)) return;

        try
        {
            (int width, int height, byte[] rgb) = ReadP6Ppm(path);
            uint texture = _gl.GenTexture();
            _gl.BindTexture(TextureTarget.Texture2D, texture);
            _gl.TexParameter(
                TextureTarget.Texture2D,
                TextureParameterName.TextureMinFilter,
                (int)GLEnum.Linear);
            _gl.TexParameter(
                TextureTarget.Texture2D,
                TextureParameterName.TextureMagFilter,
                (int)GLEnum.Linear);
            _gl.TexParameter(
                TextureTarget.Texture2D,
                TextureParameterName.TextureWrapS,
                (int)GLEnum.ClampToEdge);
            _gl.TexParameter(
                TextureTarget.Texture2D,
                TextureParameterName.TextureWrapT,
                (int)GLEnum.ClampToEdge);
            _gl.PixelStore(PixelStoreParameter.UnpackAlignment, 1);
            _gl.TexImage2D<byte>(
                TextureTarget.Texture2D,
                0,
                InternalFormat.Rgb8,
                (uint)width,
                (uint)height,
                0,
                PixelFormat.Rgb,
                PixelType.UnsignedByte,
                rgb);
            _loadingCardTexture = texture;
            _loadingCardWidth = width;
            _loadingCardHeight = height;
            Console.WriteLine(
                $"[TexturePack] loaded Route 66 loading card overlay " +
                $"{width}x{height}: {path}");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(
                $"[TexturePack] ignored Route 66 loading card overlay " +
                $"{path}: {ex.Message}");
        }
    }

    static (int Width, int Height, byte[] Rgb) ReadP6Ppm(string path)
    {
        byte[] data = File.ReadAllBytes(path);
        int cursor = 0;
        string magic = NextPpmToken(data, ref cursor);
        if (magic != "P6")
            throw new InvalidDataException("not a binary PPM");
        int width = int.Parse(NextPpmToken(data, ref cursor));
        int height = int.Parse(NextPpmToken(data, ref cursor));
        int max = int.Parse(NextPpmToken(data, ref cursor));
        if (width <= 0 || height <= 0 || max != 255)
            throw new InvalidDataException("unsupported PPM header");
        if (cursor >= data.Length || data[cursor] > 32)
            throw new InvalidDataException("missing PPM header separator");
        cursor += cursor + 1 < data.Length &&
            data[cursor] == '\r' &&
            data[cursor + 1] == '\n'
                ? 2
                : 1;
        int bytes = checked(width * height * 3);
        if (data.Length - cursor < bytes)
            throw new InvalidDataException("short PPM payload");
        byte[] rgb = new byte[bytes];
        System.Buffer.BlockCopy(data, cursor, rgb, 0, bytes);
        return (width, height, rgb);
    }

    static string NextPpmToken(byte[] data, ref int cursor)
    {
        while (cursor < data.Length)
        {
            byte b = data[cursor];
            if (b == '#')
            {
                while (cursor < data.Length && data[cursor] != '\n') cursor++;
                continue;
            }
            if (b > 32) break;
            cursor++;
        }
        int start = cursor;
        while (cursor < data.Length && data[cursor] > 32) cursor++;
        if (start == cursor)
            throw new InvalidDataException("truncated PPM header");
        return System.Text.Encoding.ASCII.GetString(data, start, cursor - start);
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
        string? captureDirectory =
            Environment.GetEnvironmentVariable("RECOMPONE_CAPTURE_DIR");
        if (!string.IsNullOrWhiteSpace(captureDirectory))
        {
            captureDirectory = Path.GetFullPath(captureDirectory);
            Directory.CreateDirectory(captureDirectory);
            path = Path.Combine(captureDirectory, path);
        }
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
        if (_loadingCardTexture != 0) _gl.DeleteTexture(_loadingCardTexture);
        if (_upscaleFbo != 0) _gl.DeleteFramebuffer(_upscaleFbo);
        if (_fxaaFbo != 0) _gl.DeleteFramebuffer(_fxaaFbo);
    }
}
