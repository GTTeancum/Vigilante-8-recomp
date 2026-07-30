using System.Runtime.InteropServices;
using RecompOne.Runtime.Config;
using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Hle;

public sealed class GlBackend : IGpuBackend
{
    [StructLayout(LayoutKind.Sequential)]
    struct GlVertex
    {
        public float X, Y;
        public uint Color;
        public int Clut, Texpage;
        public float U, V, PerspectiveW, Depth, RasterDepth;
        public float BaryX, BaryY, BaryZ;
        public float UvMinX, UvMinY, UvMaxX, UvMaxY;
    }

    const int MaxVerts = 0x40000;
    static readonly bool TraceDepth =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_DEPTH") == "1";
    static readonly bool TraceTerrainVram =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VRAM") == "1";
    static readonly bool DisableRasterDepth =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_RASTER_DEPTH") == "1";
    static readonly bool DisableProjectiveTextures =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_PROJECTIVE_TEXTURES") == "1";
    static readonly bool DisableStockPaintCorrection =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_STOCK_PAINT_CORRECTION") == "1";
    static readonly bool TraceRectangles =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_RECTANGLES") == "1";
    static readonly (float X, float Y)? TriangleProbe =
        ParseTriangleProbe(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_TRIANGLE_PROBE"));
    static readonly HashSet<string> TriangleProbeLabels =
        (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_TRIANGLE_LABELS") ?? "")
        .Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
        .ToHashSet(StringComparer.OrdinalIgnoreCase);
    readonly GL _gl;
    readonly GlVram _vram;
    HudSvgAtlas? _hudSvg;
    readonly GlDisplayRt?[] _rts = new GlDisplayRt?[2];
    long _rtStamp;
    long _frame;
    long _terrainVramTraceFrame = long.MinValue / 2;
    long _traceOpaqueTriangles;
    long _traceTransparentTriangles;
    long _traceDepthTestedTriangles;
    long _traceProjectiveTriangles;
    long _traceMissingOtTriangles;
    int _traceMinOt = int.MaxValue;
    int _traceMaxOt;
    readonly HashSet<string> _traceRectangleShapes = [];
    readonly HashSet<string> _pendingProbeTriangles = [];
    readonly Queue<(long Frame, string[] Triangles)> _probeTriangleHistory = [];

    static (float X, float Y)? ParseTriangleProbe(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        string[] parts = value.Split(',');
        return parts.Length == 2 &&
            float.TryParse(
                parts[0],
                System.Globalization.NumberStyles.Float,
                System.Globalization.CultureInfo.InvariantCulture,
                out float x) &&
            float.TryParse(
                parts[1],
                System.Globalization.NumberStyles.Float,
                System.Globalization.CultureInfo.InvariantCulture,
                out float y)
            ? (x, y)
            : null;
    }

    static uint Fnv1a16(ReadOnlySpan<ushort> values)
    {
        uint hash = 2166136261u;
        foreach (ushort value in values)
        {
            hash = (hash ^ (byte)value) * 16777619u;
            hash = (hash ^ (byte)(value >> 8)) * 16777619u;
        }
        return hash;
    }

    static bool ContainsPoint(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        float x,
        float y)
    {
        static float Edge(
            float ax, float ay, float bx, float by, float px, float py) =>
            (px - ax) * (by - ay) - (py - ay) * (bx - ax);
        float e0 = Edge(a.X, a.Y, b.X, b.Y, x, y);
        float e1 = Edge(b.X, b.Y, c.X, c.Y, x, y);
        float e2 = Edge(c.X, c.Y, a.X, a.Y, x, y);
        return (e0 >= 0f && e1 >= 0f && e2 >= 0f) ||
             (e0 <= 0f && e1 <= 0f && e2 <= 0f);
    }

    static bool ContainsPoint(
        in GlVertex a,
        in GlVertex b,
        in GlVertex c,
        float x,
        float y)
    {
        static float Edge(
            float ax, float ay, float bx, float by, float px, float py) =>
            (px - ax) * (by - ay) - (py - ay) * (bx - ax);
        float e0 = Edge(a.X, a.Y, b.X, b.Y, x, y);
        float e1 = Edge(b.X, b.Y, c.X, c.Y, x, y);
        float e2 = Edge(c.X, c.Y, a.X, a.Y, x, y);
        return (e0 >= 0f && e1 >= 0f && e2 >= 0f) ||
            (e0 <= 0f && e1 <= 0f && e2 <= 0f);
    }

    uint _vao, _vbo, _presentVao, _presentVbo, _progPrim, _progPresent, _progPresent24;
    uint _presentFbo, _presentTex;
    int _presentW, _presentH;
    bool _presentNearest;

    readonly GlVertex[] _verts = new GlVertex[MaxVerts];
    int _count;
    ushort[] _readCache = Array.Empty<ushort>();
    bool _readCacheValid;

    HleDrawEnv _env;

    GlDisplayRt? _kTarget;
    bool _kTransparent;
    bool _kDepthTest;
    int _kBlend, _kSetMask, _kCheckMask;
    int _kTwAndX, _kTwAndY, _kTwOrX, _kTwOrY;
    int _kClipX0, _kClipY0, _kClipX1, _kClipY1, _kTextureSmoothing;
    int _uTexWindow, _uBlend, _uBlendOpaque, _uSetMask, _uCheckMask, _uPosBias, _uFbInv;
    int _uTextureSmoothing, _uTextureMipmaps, _uAnisotropy, _uEnhancedShadows, _uEnhancedParticles, _uEnhancedFog;
    int _uVectorFonts, _uVectorIcons;
    int _uPresentOrigin, _uPresentSize, _uPresentTexSize, _uPresent24Origin, _uPresent24Size;

    public bool Ready { get; private set; }

    public GlBackend(GL gl) { _gl = gl; _vram = new GlVram(gl); }

    public unsafe void InitGl()
    {
        _vram.Init();

        _progPrim = GlShaders.Build(_gl, GlShaders.PrimVs, GlShaders.PrimFs, "prim");
        _progPresent = GlShaders.Build(_gl, GlShaders.FullscreenVs, GlShaders.PresentFs, "present");
        _progPresent24 = GlShaders.Build(_gl, GlShaders.FullscreenVs, GlShaders.Present24Fs, "present24");
        if (_progPrim == 0 || _progPresent == 0 || _progPresent24 == 0) return;
        _hudSvg = new HudSvgAtlas(_gl);

        _uTexWindow = _gl.GetUniformLocation(_progPrim, "uTexWindow");
        _uBlend = _gl.GetUniformLocation(_progPrim, "uBlend");
        _uBlendOpaque = _gl.GetUniformLocation(_progPrim, "uBlendOpaque");
        _uSetMask = _gl.GetUniformLocation(_progPrim, "uSetMask");
        _uCheckMask = _gl.GetUniformLocation(_progPrim, "uCheckMask");
        _uTextureSmoothing = _gl.GetUniformLocation(_progPrim, "uTextureSmoothing");
        _uTextureMipmaps = _gl.GetUniformLocation(_progPrim, "uTextureMipmaps");
        _uAnisotropy = _gl.GetUniformLocation(_progPrim, "uAnisotropy");
        _uEnhancedShadows = _gl.GetUniformLocation(_progPrim, "uEnhancedShadows");
        _uEnhancedParticles = _gl.GetUniformLocation(_progPrim, "uEnhancedParticles");
        _uEnhancedFog = _gl.GetUniformLocation(_progPrim, "uEnhancedFog");
        _uVectorFonts = _gl.GetUniformLocation(_progPrim, "uVectorFonts");
        _uVectorIcons = _gl.GetUniformLocation(_progPrim, "uVectorIcons");
        _uPosBias = _gl.GetUniformLocation(_progPrim, "uPosBias");
        _uFbInv = _gl.GetUniformLocation(_progPrim, "uFbInv");

        _gl.UseProgram(_progPrim);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uVram"), 0);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uDest"), 1);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uHudSvg"), 2);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uScale"), GlVram.Scale);
        bool stockPaintCorrection =
            !DisableStockPaintCorrection &&
            !Runtime.GameTitle.Contains(
                "2nd Offense", StringComparison.Ordinal);
        _gl.Uniform1(
            _gl.GetUniformLocation(_progPrim, "uStockPaintCorrection"),
            stockPaintCorrection ? 1 : 0);

        _uPresentOrigin = _gl.GetUniformLocation(_progPresent, "uOrigin");
        _uPresentSize = _gl.GetUniformLocation(_progPresent, "uSize");
        _uPresentTexSize = _gl.GetUniformLocation(_progPresent, "uTexSize");
        _gl.UseProgram(_progPresent);
        _gl.Uniform1(_gl.GetUniformLocation(_progPresent, "uVram"), 0);

        _uPresent24Origin = _gl.GetUniformLocation(_progPresent24, "uOrigin");
        _uPresent24Size = _gl.GetUniformLocation(_progPresent24, "uSize");
        _gl.UseProgram(_progPresent24);
        _gl.Uniform1(_gl.GetUniformLocation(_progPresent24, "uVram"), 0);
        _gl.Uniform1(_gl.GetUniformLocation(_progPresent24, "uScale"), GlVram.Scale);

        _vao = _gl.GenVertexArray();
        _vbo = _gl.GenBuffer();
        _gl.BindVertexArray(_vao);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(MaxVerts * sizeof(GlVertex)), null, BufferUsageARB.DynamicDraw);
        uint stride = (uint)sizeof(GlVertex);
        _gl.EnableVertexAttribArray(0); _gl.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, stride, (void*)0);
        _gl.EnableVertexAttribArray(1); _gl.VertexAttribIPointer(1, 1, VertexAttribIType.UnsignedInt, stride, (void*)8);
        _gl.EnableVertexAttribArray(2); _gl.VertexAttribIPointer(2, 1, VertexAttribIType.Int, stride, (void*)12);
        _gl.EnableVertexAttribArray(3); _gl.VertexAttribIPointer(3, 1, VertexAttribIType.Int, stride, (void*)16);
        _gl.EnableVertexAttribArray(4); _gl.VertexAttribPointer(4, 2, VertexAttribPointerType.Float, false, stride, (void*)20);
        _gl.EnableVertexAttribArray(5); _gl.VertexAttribPointer(5, 1, VertexAttribPointerType.Float, false, stride, (void*)28);
        _gl.EnableVertexAttribArray(6); _gl.VertexAttribPointer(6, 3, VertexAttribPointerType.Float, false, stride, (void*)40);
        _gl.EnableVertexAttribArray(7); _gl.VertexAttribPointer(7, 4, VertexAttribPointerType.Float, false, stride, (void*)52);
        _gl.EnableVertexAttribArray(8); _gl.VertexAttribPointer(8, 1, VertexAttribPointerType.Float, false, stride, (void*)32);
        _gl.EnableVertexAttribArray(9); _gl.VertexAttribPointer(9, 1, VertexAttribPointerType.Float, false, stride, (void*)36);

        // fullscreen quad for present, real vbo since gl_VertexID without arrays does not draw on mesa for some reason?? or i did it wrong?
        _presentVao = _gl.GenVertexArray();
        _presentVbo = _gl.GenBuffer();
        _gl.BindVertexArray(_presentVao);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _presentVbo);
        float[] quad = { -1f, -1f, 1f, -1f, -1f, 1f, 1f, 1f };
        fixed (float* qp = quad)
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(quad.Length * sizeof(float)), qp, BufferUsageARB.StaticDraw);
        _gl.EnableVertexAttribArray(0);
        _gl.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), (void*)0);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, 0);

        _presentTex = _gl.GenTexture();
        _gl.BindTexture(TextureTarget.Texture2D, _presentTex);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.Linear);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Linear);
        _presentFbo = _gl.GenFramebuffer();
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _presentFbo);
        _gl.FramebufferTexture2D(FramebufferTarget.Framebuffer, FramebufferAttachment.ColorAttachment0, TextureTarget.Texture2D, _presentTex, 0);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);

        _kClipX1 = 1023; _kClipY1 = 511;
        Ready = true;
    }

    public void SetDrawEnv(in HleDrawEnv env) => _env = env;

    const int FbSlackW = 64;
    const int FbSlackH = 32;

    GlDisplayRt? Classify()
    {
        int clipX = _env.ClipX0, clipY = _env.ClipY0;
        int clipW = _env.ClipX1 - _env.ClipX0 + 1, clipH = _env.ClipY1 - _env.ClipY0 + 1;
        if (clipW <= 0 || clipH <= 0) return null;

        long bestStamp = -1;
        int fbX = 0, fbY = 0, fbW = 0, fbH = 0;
        for (int i = 0; i < GpuHle.RectCount; i++)
        {
            var r = GpuHle.GetRect(i);
            if (!r.Valid || r.W <= 0 || r.H <= 0 || r.Stamp <= bestStamp) continue;

            bool clipInside = clipX >= r.X && clipX + clipW <= r.X + r.W &&
                              clipY >= r.Y && clipY + clipH <= r.Y + r.H;
            bool clipIsFb = clipX <= r.X && clipX + clipW >= r.X + r.W &&
                            clipY <= r.Y && clipY + clipH >= r.Y + r.H &&
                            clipW - r.W <= FbSlackW && clipH - r.H <= FbSlackH;
            if (clipInside) { bestStamp = r.Stamp; fbX = r.X; fbY = r.Y; fbW = r.W; fbH = r.H; }
            else if (clipIsFb) { bestStamp = r.Stamp; fbX = clipX; fbY = clipY; fbW = clipW; fbH = clipH; }
        }
        return bestStamp < 0 ? null : GetOrCreateRt(fbX, fbY, fbW, fbH);
    }

    GlDisplayRt GetOrCreateRt(int fbX, int fbY, int fbW, int fbH)
    {
        int slot = -1;
        for (int i = 0; i < _rts.Length; i++)
            if (_rts[i] is { } rt && rt.X == fbX && rt.Y == fbY)
            {
                bool sameW = rt.W == fbW;
                bool fitsH = rt.H >= fbH && rt.H - fbH <= FbSlackH;
                int samples = Math.Clamp(ConfigManager.View.MsaaSamples, 0, 8);
                if (samples == 1) samples = 0;
                if (sameW && fitsH && rt.Margin == GpuHle.WideMargin(rt.W) && rt.Samples == samples)
                {
                    rt.Stamp = ++_rtStamp;
                    return rt;
                }
                slot = i;
                break;
            }

        if (slot < 0)
        {
            slot = 0;
            for (int i = 1; i < _rts.Length; i++)
            {
                if (_rts[i] == null) { slot = i; break; }
                if (_rts[slot] != null && _rts[i]!.Stamp < _rts[slot]!.Stamp) slot = i;
            }
        }

        if (_rts[slot] is { } old)
        {
            if (old.Dirty) Writeback(old);
            old.Destroy(_gl);
        }

        var fresh = new GlDisplayRt { X = fbX, Y = fbY, W = fbW, H = fbH, Margin = GpuHle.WideMargin(fbW), Stamp = ++_rtStamp, LastDrawFrame = _frame };
        fresh.Create(_gl);
        _rts[slot] = fresh;
        SyncRtFromVram(fresh, fbX, fbY, fbW, fbH);
        if (TraceTerrainVram)
            Console.Error.WriteLine(
                $"[V82GlRt] create frame={_frame} slot={slot} " +
                $"xy={fbX},{fbY} size={fbW}x{fbH} " +
                $"clip={_env.ClipX0},{_env.ClipY0}-" +
                $"{_env.ClipX1},{_env.ClipY1}");
        return fresh;
    }

    void Writeback(GlDisplayRt rt)
    {
        rt.Resolve(_gl);
        int s = GlVram.Scale;
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, rt.Fbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, _vram.Fbo);
        _gl.BlitFramebuffer(rt.Margin * s, 0, (rt.Margin + rt.W) * s, rt.H * s,
            rt.X * s, rt.Y * s, (rt.X + rt.W) * s, (rt.Y + rt.H) * s,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        rt.Dirty = false;
    }

    void SyncRtFromVram(GlDisplayRt rt, int rx, int ry, int rw, int rh)
    {
        int x0 = Math.Max(rx, rt.X), y0 = Math.Max(ry, rt.Y);
        int x1 = Math.Min(rx + rw, rt.X + rt.W), y1 = Math.Min(ry + rh, rt.Y + rt.H);
        if (x0 >= x1 || y0 >= y1) return;
        int s = GlVram.Scale;
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(FramebufferTarget.ReadFramebuffer, _vram.Fbo);
        _gl.BindFramebuffer(FramebufferTarget.DrawFramebuffer, rt.Fbo);
        _gl.BlitFramebuffer(x0 * s, y0 * s, x1 * s, y1 * s,
            (x0 - rt.X + rt.Margin) * s, (y0 - rt.Y) * s, (x1 - rt.X + rt.Margin) * s, (y1 - rt.Y) * s,
            ClearBufferMask.ColorBufferBit, BlitFramebufferFilter.Nearest);
        rt.CopyResolveToMsaa(_gl);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
    }

    void WritebackDirtyIntersecting(int x, int y, int w, int h)
    {
        foreach (var rt in _rts)
            if (rt is { Dirty: true } && rt.Intersects(x, y, w, h)) Writeback(rt);
    }

    void WritebackDirtyWrappedIntersecting(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0) return;
        x &= VramShadow.Width - 1;
        y &= VramShadow.Height - 1;
        int firstW = Math.Min(w, VramShadow.Width - x);
        int wrappedW = w - firstW;
        int firstH = Math.Min(h, VramShadow.Height - y);
        int wrappedH = h - firstH;
        WritebackDirtyIntersecting(x, y, firstW, firstH);
        WritebackDirtyIntersecting(0, y, wrappedW, firstH);
        WritebackDirtyIntersecting(x, 0, firstW, wrappedH);
        WritebackDirtyIntersecting(0, 0, wrappedW, wrappedH);
    }

    void SyncRtsFromVram(int x, int y, int w, int h)
    {
        foreach (var rt in _rts)
            if (rt != null && rt.Intersects(x, y, w, h)) SyncRtFromVram(rt, x, y, w, h);
    }

    void SyncWrappedRtsFromVram(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0) return;
        x &= VramShadow.Width - 1;
        y &= VramShadow.Height - 1;
        int firstW = Math.Min(w, VramShadow.Width - x);
        int wrappedW = w - firstW;
        int firstH = Math.Min(h, VramShadow.Height - y);
        int wrappedH = h - firstH;
        SyncRtsFromVram(x, y, firstW, firstH);
        SyncRtsFromVram(0, y, wrappedW, firstH);
        SyncRtsFromVram(x, 0, firstW, wrappedH);
        SyncRtsFromVram(0, 0, wrappedW, wrappedH);
    }

    void CheckTextureFeedback(in PrimFlags f)
    {
        if (!f.Textured) return;
        int px = (f.TPage & 0xF) * 64;
        int py = ((f.TPage >> 4) & 1) * 256;
        int depth = (f.TPage >> 7) & 3;
        int pw = depth == 0 ? 64 : depth == 1 ? 128 : 256;
        foreach (var rt in _rts)
            if (rt is { Dirty: true } && rt.Intersects(px, py, pw, 256))
            {
                Flush();
                Writeback(rt);
            }
    }

    bool DesiredMatches(bool transparent, int blend, bool depthTest)
    {
        int twAndX = ~(_env.TwMaskX * 8) & 0xFF, twAndY = ~(_env.TwMaskY * 8) & 0xFF;
        int twOrX = (_env.TwOffX & _env.TwMaskX) * 8, twOrY = (_env.TwOffY & _env.TwMaskY) * 8;
        return _kTransparent == transparent && _kBlend == blend &&
            _kDepthTest == depthTest
            && _kSetMask == (_env.SetMask ? 1 : 0) && _kCheckMask == (_env.CheckMask ? 1 : 0)
            && _kTwAndX == twAndX && _kTwAndY == twAndY && _kTwOrX == twOrX && _kTwOrY == twOrY
            && _kClipX0 == _env.ClipX0 && _kClipY0 == _env.ClipY0 && _kClipX1 == _env.ClipX1 && _kClipY1 == _env.ClipY1
            && _kTextureSmoothing == (ConfigManager.View.TextureSmoothing ? 1 : 0);
    }

    void Begin(in PrimFlags f, int vertsNeeded, bool depthTest = false)
    {
        _readCacheValid = false;
        bool transparent = f.SemiTrans;
        int blend = f.BlendMode;
        var target = Classify();
        if (_count > 0 &&
            (target != _kTarget ||
             !DesiredMatches(transparent, blend, depthTest))) Flush();
        if (_count + vertsNeeded > MaxVerts) Flush();
        CheckTextureFeedback(f);

        _kTarget = target;
        _kTransparent = transparent; _kBlend = blend;
        _kDepthTest = depthTest;
        _kSetMask = _env.SetMask ? 1 : 0; _kCheckMask = _env.CheckMask ? 1 : 0;
        _kTwAndX = ~(_env.TwMaskX * 8) & 0xFF; _kTwAndY = ~(_env.TwMaskY * 8) & 0xFF;
        _kTwOrX = (_env.TwOffX & _env.TwMaskX) * 8; _kTwOrY = (_env.TwOffY & _env.TwMaskY) * 8;
        _kClipX0 = _env.ClipX0; _kClipY0 = _env.ClipY0; _kClipX1 = _env.ClipX1; _kClipY1 = _env.ClipY1;
        _kTextureSmoothing = ConfigManager.View.TextureSmoothing ? 1 : 0;
    }

    bool DitherOf(in PrimFlags f) => _env.Dither && (f.Gouraud || (f.Textured && !f.RawTexture));

    GlVertex V(
        in HleVertex v,
        in PrimFlags f,
        bool dither,
        bool perspectiveCorrect,
        bool uiTexture = false,
        float rasterDepth = 0f)
    {
        uint color = (f.Textured && f.RawTexture) ? 0x808080u : (uint)(v.R | (v.G << 8) | (v.B << 16));
        int tpage = f.Textured ? (f.TPage & 0x1FF) : 0x8000;
        if (dither) tpage |= 0x400;
        // "Raw texture" only disables vertex-colour modulation on the PS1;
        // it does not identify a UI primitive. Terrain and several vehicle
        // materials use raw-texture packets, so excluding them left the most
        // visibly pixelated surfaces untouched.
        if (ConfigManager.View.TextureSmoothing && f.Textured &&
            (!f.RawTexture || !uiTexture))
            tpage |= 0x800;
        if (uiTexture) tpage |= 0x1000;
        if (f.Vehicle) tpage |= 0x80000;
        float perspectiveW =
            perspectiveCorrect ? MathF.Max(1f, v.PerspectiveW) : 1f;
        float screenX = v.X;
        float screenY = v.Y;
        if (!uiTexture &&
            GpuHle.GameplayActive &&
            _kTarget is { Margin: > 0 } wideTarget)
        {
            // The retail game culls world sectors for a 4:3 camera. Exposing
            // a wider horizontal frustum reveals unsubmitted terrain edges as
            // blinking sky wedges. Expand both axes uniformly instead: this
            // fills 16:9 without stretching models or exposing uncullable
            // off-map geometry (the vertical field is cropped accordingly).
            float wideScale = wideTarget.Wide1x / (float)wideTarget.W;
            screenX = wideTarget.X - wideTarget.Margin +
                (v.X - wideTarget.X) * wideScale;
            float centerY = wideTarget.Y + wideTarget.H * 0.5f;
            screenY = centerY + (v.Y - centerY) * wideScale;
        }
        return new GlVertex
        {
            X = screenX, Y = screenY,
            Color = color,
            Clut = f.Clut & 0x7FFF,
            Texpage = tpage,
            U = v.U, V = v.V,
            PerspectiveW = perspectiveW,
            Depth = v.HasGteZ ? MathF.Max(1f, v.Z) : 1f,
            RasterDepth = rasterDepth,
        };
    }

    public void DrawTri(in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f)
    {
        float spanX =
            Math.Max(a.X, Math.Max(b.X, c.X)) -
            Math.Min(a.X, Math.Min(b.X, c.X));
        float spanY =
            Math.Max(a.Y, Math.Max(b.Y, c.Y)) -
            Math.Min(a.Y, Math.Min(b.Y, c.Y));
        // Match RasterTriangle and the native PS1 polygon limits. Vertices
        // projected across/behind the camera can wrap into enormous screen
        // spans; the hardware drops those primitives instead of clipping them
        // into the long terrain-like wedges OpenGL would otherwise draw.
        if (spanX > 1023 || spanY > 511)
            return;

        // Exact per-vertex GTE depth is needed only for transparent world
        // planes (notably Dreamland water) so opaque terrain can occlude them
        // fragment by fragment. Applying the recovered samples to every
        // opaque object made unrelated projections at shared screen pixels
        // change an object's depth from frame to frame, which caused the
        // widespread prop/vehicle polygon flicker. Opaque primitives retain
        // one native ordering-table depth for the complete packet.
        bool worldTransparent =
            f.SemiTrans &&
            a.HasGteZ && b.HasGteZ && c.HasGteZ &&
            !f.Vehicle;
        bool depthTest =
            !DisableRasterDepth &&
            GpuHle.GameplayActive &&
            (ConfigManager.View.HighResolution3D ||
             ConfigManager.View.PerspectiveCorrectTextures) &&
            (!f.SemiTrans || worldTransparent) &&
            f.OtIndex > 0;
        Begin(f, 3, depthTest);
        bool dith = DitherOf(f);
        bool hasDepth = f.Textured && a.HasGteZ && b.HasGteZ && c.HasGteZ;
        bool hasProjectiveW =
            f.Textured &&
            a.HasProjectiveW && b.HasProjectiveW && c.HasProjectiveW;
        bool perspectiveCorrect =
            !DisableProjectiveTextures &&
            ConfigManager.View.PerspectiveCorrectTextures && hasProjectiveW;
        if (TraceDepth && GpuHle.GameplayActive)
        {
            if (f.SemiTrans) _traceTransparentTriangles++;
            else _traceOpaqueTriangles++;
            if (depthTest) _traceDepthTestedTriangles++;
            if (perspectiveCorrect) _traceProjectiveTriangles++;
            if (f.OtIndex <= 0) _traceMissingOtTriangles++;
            else
            {
                _traceMinOt = Math.Min(_traceMinOt, f.OtIndex);
                _traceMaxOt = Math.Max(_traceMaxOt, f.OtIndex);
            }
        }
        bool particle = f.Textured && f.SemiTrans && hasDepth;
        bool shadow = !f.Textured && f.SemiTrans && a.HasGteZ && b.HasGteZ && c.HasGteZ &&
            a.R < 96 && a.G < 96 && a.B < 96 && b.R < 96 && b.G < 96 && b.B < 96 &&
            c.R < 96 && c.G < 96 && c.B < 96;
        static float DepthOf(
            in HleVertex vertex, int otDepth, bool useGteDepth)
        {
            // GTE projection recovery gives each world vertex its native
            // camera-space SZ value. Preserve that value through OpenGL so
            // transparent world planes are clipped fragment-by-fragment by
            // opaque terrain instead of comparing one OT bucket for the
            // complete primitive. The latter made Dreamland water cross hills
            // as the bucket changed while the camera turned.
            if (useGteDepth && vertex.HasGteZ)
                return Math.Clamp(vertex.Z, 1f, ushort.MaxValue) /
                    ushort.MaxValue;

            // V8 bins camera-space GTE SZ at approximately SZ/8. Recover the
            // same normalized depth for vertices whose exact projection could
            // not be correlated, instead of placing them twice as far away.
            return Math.Clamp(otDepth, 1, 0x1FFF) / 8192f;
        }
        var va = V(a, f, dith, perspectiveCorrect, false,
            DepthOf(a, f.OtIndex, worldTransparent)); va.BaryX = 1f;
        var vb = V(b, f, dith, perspectiveCorrect, false,
            DepthOf(b, f.OtIndex, worldTransparent)); vb.BaryY = 1f;
        var vc = V(c, f, dith, perspectiveCorrect, false,
            DepthOf(c, f.OtIndex, worldTransparent)); vc.BaryZ = 1f;
        if (TriangleProbe is { } probe &&
            GpuHle.GameplayActive &&
            _pendingProbeTriangles.Count < 1024)
        {
            float targetProbeX = (_kTarget?.X ?? 0) + probe.X;
            float targetProbeY = (_kTarget?.Y ?? 0) + probe.Y;
            float area2 = MathF.Abs(
                (vb.X - va.X) * (vc.Y - va.Y) -
                (vb.Y - va.Y) * (vc.X - va.X));
            bool largeTexturedTriangle = f.Textured;
            bool coversProbe = ContainsPoint(
                va, vb, vc,
                targetProbeX,
                targetProbeY);
            if (coversProbe || largeTexturedTriangle)
            {
                string triangle =
                    $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                    $"tex={(f.Textured ? 1 : 0)} " +
                    $"semi={(f.SemiTrans ? 1 : 0)} raw={(f.RawTexture ? 1 : 0)} " +
                    $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                    $"source-xy=({a.X},{a.Y})({b.X},{b.Y})({c.X},{c.Y}) " +
                    $"draw-xy=({va.X},{va.Y})({vb.X},{vb.Y})({vc.X},{vc.Y}) " +
                    $"uv=({a.U},{a.V})({b.U},{b.V})({c.U},{c.V}) " +
                    $"rgb=({a.R},{a.G},{a.B})({b.R},{b.G},{b.B})" +
                    $"({c.R},{c.G},{c.B}) " +
                    $"area2={area2} probe={(coversProbe ? 1 : 0)} " +
                    $"target={_kTarget?.X ?? -1},{_kTarget?.Y ?? -1} " +
                    $"z=({a.Z},{b.Z},{c.Z}) " +
                    $"gte=({(a.HasGteZ ? 1 : 0)},{(b.HasGteZ ? 1 : 0)}," +
                    $"{(c.HasGteZ ? 1 : 0)})";
                _pendingProbeTriangles.Add(triangle);
            }
        }
        float uvMinX = MathF.Min(a.U, MathF.Min(b.U, c.U));
        float uvMinY = MathF.Min(a.V, MathF.Min(b.V, c.V));
        float uvMaxX = MathF.Max(a.U, MathF.Max(b.U, c.U));
        float uvMaxY = MathF.Max(a.V, MathF.Max(b.V, c.V));
        va.UvMinX = vb.UvMinX = vc.UvMinX = uvMinX;
        va.UvMinY = vb.UvMinY = vc.UvMinY = uvMinY;
        va.UvMaxX = vb.UvMaxX = vc.UvMaxX = uvMaxX;
        va.UvMaxY = vb.UvMaxY = vc.UvMaxY = uvMaxY;
        if (particle) { va.Texpage |= 0x2000; vb.Texpage |= 0x2000; vc.Texpage |= 0x2000; }
        if (shadow)
        {
            float oppositeA = MathF.Pow(b.X - c.X, 2f) + MathF.Pow(b.Y - c.Y, 2f);
            float oppositeB = MathF.Pow(a.X - c.X, 2f) + MathF.Pow(a.Y - c.Y, 2f);
            float oppositeC = MathF.Pow(a.X - b.X, 2f) + MathF.Pow(a.Y - b.Y, 2f);
            int longest = oppositeA >= oppositeB && oppositeA >= oppositeC ? 0 :
                oppositeB >= oppositeC ? 1 : 2;
            va.Texpage |= 0x4000; vb.Texpage |= 0x4000; vc.Texpage |= 0x4000;
            va.Clut = vb.Clut = vc.Clut = longest;
        }
        _verts[_count++] = va; _verts[_count++] = vb; _verts[_count++] = vc;
    }

    public void DrawRect(in HleRect r, in PrimFlags f)
    {
        Begin(f, 6);
        if (TraceRectangles && GpuHle.GameplayActive &&
            _traceRectangleShapes.Count < 256)
        {
            string shape =
                $"ot={f.OtIndex} xy={r.X},{r.Y} wh={r.W}x{r.H} " +
                $"uv={r.U},{r.V} tex={(f.Textured ? 1 : 0)} " +
                $"semi={(f.SemiTrans ? 1 : 0)} raw={(f.RawTexture ? 1 : 0)} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4}";
            if (_traceRectangleShapes.Add(shape))
                Console.WriteLine($"[V82Rect] {shape}");
        }
        bool topGameplayHud =
            GpuHle.GameplayActive &&
            _kTarget is { } hudTarget &&
            r.Y - hudTarget.Y < hudTarget.H * 0.42f;
        float drawX = r.X;
        int drawW = r.W;
        short drawU = r.U;
        // The retail status rectangle starts at x=80, but its live vehicle
        // portrait is x=76..116 and its complete armor bar is x=80..112.
        // Extend only the vector backing six native pixels left: the copied
        // 44-pixel weapon-panel silhouette then lands at x=74..118, centering
        // the portrait at 2/2 pixels and the armor bar at 6/6 pixels.
        bool statusHudBacking =
            ConfigManager.View.VectorIcons &&
            topGameplayHud &&
            f.Textured && f.RawTexture && f.SemiTrans &&
            r.W == 84 && r.H == 34;
        if (statusHudBacking)
        {
            drawX -= 6f;
            drawW += 6;
            drawU -= 6;
        }
        float anchor = 0f;
        if (ConfigManager.View.HudAnchoring && GpuHle.GameplayActive && _kTarget is { Margin: > 0 } target)
        {
            float localCenter = drawX + drawW * 0.5f - target.X;
            float localTop = r.Y - target.Y;

            // V8:2 builds the radar, armor/health panel and weapon panel from
            // many adjacent rectangles. Classifying each tile by horizontal
            // thirds split those widgets and opened visible gaps. Keep every
            // top-HUD tile in one left-anchored group. Lower HUD art is moved
            // only when it was already authored in the left third; projected
            // names and central prompts retain their world/screen position.
            bool topHud = localTop < target.H * 0.42f;
            bool locationCaption =
                localTop >= target.H * 0.52f &&
                localTop < target.H * 0.9f;
            bool lowerLeftHud =
                localCenter < target.W / 3f || locationCaption;
            if (topHud || lowerLeftHud)
                anchor = -target.Margin;
        }
        var a = new HleVertex { X = drawX + anchor, Y = r.Y, R = r.R, G = r.G, B = r.B, U = drawU, V = r.V };
        var b = new HleVertex { X = drawX + drawW + anchor, Y = r.Y, R = r.R, G = r.G, B = r.B, U = (short)(drawU + drawW), V = r.V };
        var c = new HleVertex { X = drawX + anchor, Y = r.Y + r.H, R = r.R, G = r.G, B = r.B, U = drawU, V = (short)(r.V + r.H) };
        var d = new HleVertex { X = drawX + drawW + anchor, Y = r.Y + r.H, R = r.R, G = r.G, B = r.B, U = (short)(drawU + drawW), V = (short)(r.V + r.H) };
        // The compact top HUD uses tightly packed atlas cells. Keep their
        // authored binary silhouettes exact; sampling across a cell boundary
        // can pull neighboring digits into the ammo counter. Larger gameplay
        // labels and menu text remain eligible for contour reconstruction.
        bool fontLike = f.Textured && !topGameplayHud && r.W <= 32 && r.H <= 32;
        bool radarPlate =
            topGameplayHud && f.Textured && r.W == 64 && r.H == 55;
        bool mainHudPlate =
            topGameplayHud && f.Textured && r.W == 84 && r.H == 34;
        bool healthHudPlate =
            topGameplayHud && f.Textured && r.W == 16 && r.H == 49;
        bool hudBackgroundPlate =
            radarPlate || mainHudPlate || healthHudPlate;
        bool iconLike =
            f.Textured && !fontLike &&
            r.W <= 96 && r.H <= 96 &&
            (!topGameplayHud || hudBackgroundPlate);
        int uiFlags = fontLike && ConfigManager.View.VectorFonts ? 0x2800 :
            iconLike && ConfigManager.View.VectorIcons ? 0x4800 : 0;
        if (ConfigManager.View.VectorIcons)
        {
            if (radarPlate) uiFlags |= 0x10000;
            if (hudBackgroundPlate) uiFlags |= 0x20000;
            if (healthHudPlate) uiFlags |= 0x40000;
        }
        var va = V(a, f, false, false, true); va.Texpage |= uiFlags;
        var vb = V(b, f, false, false, true); vb.Texpage |= uiFlags;
        var vc = V(c, f, false, false, true); vc.Texpage |= uiFlags;
        var vd = V(d, f, false, false, true); vd.Texpage |= uiFlags;
        float uvMinX = Math.Min(drawU, drawU + drawW);
        float uvMinY = Math.Min(r.V, r.V + r.H);
        float uvMaxX = Math.Max(drawU, drawU + drawW) - 1f;
        float uvMaxY = Math.Max(r.V, r.V + r.H) - 1f;
        va.UvMinX = vb.UvMinX = vc.UvMinX = vd.UvMinX = uvMinX;
        va.UvMinY = vb.UvMinY = vc.UvMinY = vd.UvMinY = uvMinY;
        va.UvMaxX = vb.UvMaxX = vc.UvMaxX = vd.UvMaxX = uvMaxX;
        va.UvMaxY = vb.UvMaxY = vc.UvMaxY = vd.UvMaxY = uvMaxY;
        if (topGameplayHud)
        {
            va.Texpage &= ~0x800;
            vb.Texpage &= ~0x800;
            vc.Texpage &= ~0x800;
            vd.Texpage &= ~0x800;
        }
        _verts[_count++] = va; _verts[_count++] = vb; _verts[_count++] = vc;
        _verts[_count++] = vb; _verts[_count++] = vd; _verts[_count++] = vc;
    }

    public void DrawLine(in HleVertex a, in HleVertex b, in PrimFlags f)
    {
        Begin(f, 6);
        bool dith = _env.Dither;
        float x1 = a.X, y1 = a.Y;
        float x2 = b.X, y2 = b.Y;
        float dx = x2 - x1, dy = y2 - y1;

        if (dx == 0 && dy == 0)
        {
            LineVert(x1, y1, a, f, dith); LineVert(x1 + 1, y1, a, f, dith); LineVert(x1 + 1, y1 + 1, a, f, dith);
            LineVert(x1 + 1, y1 + 1, a, f, dith); LineVert(x1, y1 + 1, a, f, dith); LineVert(x1, y1, a, f, dith);
            return;
        }

        float xo, yo;
        if (Math.Abs(dx) > Math.Abs(dy)) { xo = 0; yo = 1; if (dx > 0) x2++; else x1++; }
        else { xo = 1; yo = 0; if (dy > 0) y2++; else y1++; }

        LineVert(x1, y1, a, f, dith); LineVert(x2, y2, b, f, dith); LineVert(x2 + xo, y2 + yo, b, f, dith);
        LineVert(x2 + xo, y2 + yo, b, f, dith); LineVert(x1 + xo, y1 + yo, a, f, dith); LineVert(x1, y1, a, f, dith);
    }

    void LineVert(float x, float y, in HleVertex src, in PrimFlags f, bool dither)
    {
        var v = src; v.X = x; v.Y = y;
        _verts[_count++] = V(v, f, dither, false);
    }

    public void FillRect(int x, int y, int w, int h, ushort color15)
    {
        _readCacheValid = false;
        Flush();
        _vram.Fill(x, y, w, h, color15);
        if (x + w > VramShadow.Width ||
            y + h > VramShadow.Height)
        {
            SyncWrappedRtsFromVram(x, y, w, h);
            return;
        }
        foreach (var rt in _rts)
        {
            if (rt == null || !rt.Intersects(x, y, w, h)) continue;
            if (rt.Covers(x, y, x + w - 1, y + h - 1))
            {
                FillRtFull(rt, color15);
                rt.Dirty = false;
                rt.LastDrawFrame = _frame;
            }
            else SyncRtFromVram(rt, x, y, w, h);
        }
    }

    void FillRtFull(GlDisplayRt rt, ushort color15)
    {
        float r = (color15 & 0x1F) / 31f, g = ((color15 >> 5) & 0x1F) / 31f, b = ((color15 >> 10) & 0x1F) / 31f;
        float a = (color15 & 0x8000) != 0 ? 1f : 0f;
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, rt.DrawFbo);
        _gl.Disable(EnableCap.ScissorTest);
        _gl.ClearColor(r, g, b, a);
        _gl.Clear(ClearBufferMask.ColorBufferBit);
        rt.Resolve(_gl);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
    }

    public void CopyVram(int sx, int sy, int dx, int dy, int w, int h)
    {
        _readCacheValid = false;
        Flush();
        WritebackDirtyWrappedIntersecting(sx, sy, w, h);
        _vram.CopyRect(sx, sy, dx, dy, w, h);
        SyncWrappedRtsFromVram(dx, dy, w, h);
    }

    public void WriteVram(int x, int y, int w, int h, ReadOnlySpan<ushort> px)
    {
        _readCacheValid = false;
        Flush();
        _vram.WriteRect(x, y, w, h, px);
        SyncWrappedRtsFromVram(x, y, w, h);
    }

    public void ReadVram(int x, int y, int w, int h, Span<ushort> px)
    {
        Flush();
        if (!_readCacheValid && (long)w * h > 64)
        {
            WritebackDirtyWrappedIntersecting(x, y, w, h);
            _vram.ReadRect(x, y, w, h, px);
            return;
        }

        int cacheWidth = VramShadow.Width;
        int cacheHeight = VramShadow.Height;
        int cachePixels = cacheWidth * cacheHeight;
        if (!_readCacheValid)
        {
            if (_readCache.Length < cachePixels)
                _readCache = new ushort[cachePixels];
            WritebackDirtyIntersecting(0, 0, cacheWidth, cacheHeight);
            _vram.ReadRect(
                0, 0, cacheWidth, cacheHeight,
                _readCache.AsSpan(0, cachePixels));
            _readCacheValid = true;
        }

        int rows = Math.Min(h, px.Length / Math.Max(1, w));
        if (x >= 0 && y >= 0 &&
            x + w <= cacheWidth && y + rows <= cacheHeight)
        {
            for (int row = 0; row < rows; row++)
            {
                _readCache.AsSpan((y + row) * cacheWidth + x, w)
                    .CopyTo(px.Slice(row * w, w));
            }
            return;
        }

        for (int row = 0; row < rows; row++)
        {
            int sourceY = (y + row) & (cacheHeight - 1);
            for (int col = 0; col < w; col++)
            {
                int sourceX = (x + col) & (cacheWidth - 1);
                px[row * w + col] =
                    _readCache[sourceY * cacheWidth + sourceX];
            }
        }
    }

    public unsafe void Flush()
    {
        if (_count == 0) return;
        _readCacheValid = false;

        var rt = _kTarget;
        uint destTex;
        if (rt == null)
        {
            _vram.BindDraw();
            destTex = _vram.Texture;
        }
        else
        {
            _gl.BindFramebuffer(FramebufferTarget.Framebuffer, rt.DrawFbo);
            _gl.Viewport(0, 0, (uint)rt.TexW, (uint)rt.TexH);
            destTex = rt.Tex;
        }
        _vram.Barrier();

        if (_kDepthTest)
        {
            _gl.Enable(EnableCap.DepthTest);
            _gl.DepthFunc(DepthFunction.Lequal);
            _gl.DepthMask(!_kTransparent);
        }
        else
        {
            _gl.Disable(EnableCap.DepthTest);
            _gl.DepthMask(false);
        }
        _gl.Disable(EnableCap.CullFace);
        _gl.Enable(EnableCap.ScissorTest);
        int s = GlVram.Scale;
        if (rt == null)
        {
            int sw = _kClipX1 - _kClipX0 + 1, sh = _kClipY1 - _kClipY0 + 1;
            _gl.Scissor(_kClipX0 * s, _kClipY0 * s, (uint)Math.Max(0, sw * s), (uint)Math.Max(0, sh * s));
        }
        else
        {
            int cx0 = _kClipX0 - rt.X + rt.Margin, cy0 = _kClipY0 - rt.Y;
            int cx1 = _kClipX1 - rt.X + rt.Margin, cy1 = _kClipY1 - rt.Y;
            if (rt.Margin > 0 && _kClipX0 <= rt.X && _kClipX1 >= rt.X + rt.W - 1) { cx0 = 0; cx1 = rt.Wide1x - 1; }
            _gl.Scissor(cx0 * s, cy0 * s, (uint)Math.Max(0, (cx1 - cx0 + 1) * s), (uint)Math.Max(0, (cy1 - cy0 + 1) * s));
        }

        _gl.UseProgram(_progPrim);
        _gl.BindVertexArray(_vao);
        _gl.ActiveTexture(TextureUnit.Texture0);
        _gl.BindTexture(TextureTarget.Texture2D, _vram.Texture);
        _gl.ActiveTexture(TextureUnit.Texture1);
        _gl.BindTexture(TextureTarget.Texture2D, destTex);
        _gl.ActiveTexture(TextureUnit.Texture2);
        _gl.BindTexture(TextureTarget.Texture2D, _hudSvg?.Texture ?? 0);
        _gl.ActiveTexture(TextureUnit.Texture0);
        if (rt != null)
        {
            _gl.Uniform2(_uPosBias, (float)(rt.Margin - rt.X), (float)(-rt.Y));
            _gl.Uniform2(_uFbInv, 2f / rt.Wide1x, 2f / rt.H);
        }
        else
        {
            _gl.Uniform2(_uPosBias, 0f, 0f);
            _gl.Uniform2(_uFbInv, 2f / VramShadow.Width, 2f / VramShadow.Height);
        }
        _gl.Uniform4(_uTexWindow, _kTwAndX, _kTwAndY, _kTwOrX, _kTwOrY);
        _gl.Uniform1(_uSetMask, _kSetMask == 1 ? 1f : 0f);
        _gl.Uniform1(_uCheckMask, _kCheckMask);
        _gl.Uniform4(_uBlendOpaque, 1f, 1f, 1f, 0f);
            _gl.Uniform1(_uTextureSmoothing, _kTextureSmoothing);
            _gl.Uniform1(_uTextureMipmaps, ConfigManager.View.TextureMipmaps ? 1 : 0);
            _gl.Uniform1(_uAnisotropy, Math.Clamp(ConfigManager.View.AnisotropicFiltering, 1, 16));
            _gl.Uniform1(_uEnhancedShadows, ConfigManager.View.EnhancedShadows ? 1 : 0);
            _gl.Uniform1(
                _uEnhancedParticles,
                ConfigManager.View.EnhancedParticles ? 1 : 0);
            _gl.Uniform1(_uEnhancedFog, ConfigManager.View.EnhancedFog ? 1 : 0);
            _gl.Uniform1(_uVectorFonts, ConfigManager.View.VectorFonts ? 1 : 0);
            _gl.Uniform1(_uVectorIcons, ConfigManager.View.VectorIcons ? 1 : 0);

        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        _gl.BufferSubData<GlVertex>(BufferTargetARB.ArrayBuffer, 0, _verts.AsSpan(0, _count));

        if (!_kTransparent)
        {
            _gl.Disable(EnableCap.Blend);
            _gl.DrawArrays(PrimitiveType.Triangles, 0, (uint)_count);
        }
        else
        {
            _gl.Enable(EnableCap.Blend);
            _gl.BlendFuncSeparate(BlendingFactor.Src1Color, BlendingFactor.Src1Alpha, BlendingFactor.One, BlendingFactor.Zero);
            if (_kBlend == 2)
            {
                _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
                SetBlend(0f, 1f);
                _gl.DrawArrays(PrimitiveType.Triangles, 0, (uint)_count);

                _vram.Barrier();
                _gl.BlendEquationSeparate(BlendEquationModeEXT.FuncReverseSubtract, BlendEquationModeEXT.FuncAdd);
                SetBlend(1f, 1f);
                _gl.Uniform4(_uBlendOpaque, 0f, 0f, 0f, 1f);
                _gl.DrawArrays(PrimitiveType.Triangles, 0, (uint)_count);
            }
            else
            {
                _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
                SetBlend(_kBlend switch { 0 => 0.5f, 3 => 0.25f, _ => 1f }, _kBlend == 0 ? 0.5f : 1f);
                _gl.DrawArrays(PrimitiveType.Triangles, 0, (uint)_count);
            }
        }

        _gl.Disable(EnableCap.ScissorTest);
        _gl.DepthMask(true);
        if (rt != null)
        {
            rt.Resolve(_gl);
            rt.Dirty = true;
            rt.LastDrawFrame = _frame;
        }
        _count = 0;
    }

    void SetBlend(float src, float dst) => _gl.Uniform4(_uBlend, src, src, src, dst);

    public void Present(in HleDispEnv disp) => PresentDisplay(disp.X, disp.Y, disp.W, disp.H, disp.Rgb24);

    public unsafe (uint tex, int w, int h, float aspect) PresentDisplay(int dispX, int dispY, int w, int h, bool rgb24 = false, int outW = 0, int outH = 0)
    {
        if (!Ready || w <= 0 || h <= 0) return (0, 0, 0, GpuHle.OutputAspect);
        _frame++;
        string? captureLabel = GpuHle.DebugCaptureLabel;
        _probeTriangleHistory.Enqueue(
            (_frame, _pendingProbeTriangles.ToArray()));
        while (_probeTriangleHistory.Count > 4)
            _probeTriangleHistory.Dequeue();
        if (TriangleProbe is not null && !string.IsNullOrEmpty(captureLabel))
            Console.Error.WriteLine(
                $"[V8TriangleProbeFrame] label={captureLabel} frame={_frame} " +
                $"candidates={_probeTriangleHistory.Sum(entry => entry.Triangles.Length)}");
        if (!string.IsNullOrEmpty(captureLabel) &&
            (TriangleProbeLabels.Count == 0 ||
             TriangleProbeLabels.Contains(captureLabel)))
            foreach (var entry in _probeTriangleHistory)
                foreach (string triangle in entry.Triangles)
                    Console.Error.WriteLine(
                        $"[V8TriangleProbe] label={captureLabel} " +
                        $"capture-frame={_frame} draw-frame={entry.Frame} {triangle}");
        _pendingProbeTriangles.Clear();
        Flush();
        if (TraceTerrainVram && GpuHle.GameplayActive &&
            _frame - _terrainVramTraceFrame >= 120)
        {
            ushort[] atlas = new ushort[240 * 192];
            ushort[] palettes = new ushort[232 * 30];
            _vram.ReadRect(640, 0, 240, 192, atlas);
            _vram.ReadRect(32, 482, 232, 30, palettes);
            string paletteHashes = string.Join(
                ' ',
                Enumerable.Range(0, 30).Select(row =>
                    $"{482 + row}:" +
                    $"{Fnv1a16(palettes.AsSpan(row * 232, 232)):X8}"));
            Console.Error.WriteLine(
                $"[V82GlVramFinal] atlas=640,0 240x192 " +
                $"fnv=0x{Fnv1a16(atlas):X8} " +
                $"palette-rows={paletteHashes}");
            _terrainVramTraceFrame = _frame;
        }
        if (TraceDepth && (_frame % 60) == 0)
        {
            int minOt = _traceMinOt == int.MaxValue ? 0 : _traceMinOt;
            Console.Error.WriteLine(
                $"[Depth] frames={_frame - 59}-{_frame} " +
                $"opaque-tris={_traceOpaqueTriangles} " +
                $"transparent-tris={_traceTransparentTriangles} " +
                $"depth-tested={_traceDepthTestedTriangles} " +
                $"projective={_traceProjectiveTriangles} " +
                $"missing-ot={_traceMissingOtTriangles} " +
                $"ot-range={minOt}..{_traceMaxOt}");
            _traceOpaqueTriangles = 0;
            _traceTransparentTriangles = 0;
            _traceDepthTestedTriangles = 0;
            _traceProjectiveTriangles = 0;
            _traceMissingOtTriangles = 0;
            _traceMinOt = int.MaxValue;
            _traceMaxOt = 0;
        }

        for (int i = 0; i < _rts.Length; i++)
        {
            if (_rts[i] is not { } rt) continue;
            if (rt.Dirty) Writeback(rt);
            if (_frame - rt.LastDrawFrame > 300)
            {
                rt.Destroy(_gl);
                _rts[i] = null;
            }
        }

        GlDisplayRt? src = null;
        if (!rgb24)
            foreach (var rt in _rts)
            {
                if (rt == null || _frame - rt.LastDrawFrame > 4) continue;
                if (dispX < rt.X || dispY < rt.Y || dispX + w > rt.X + rt.W || dispY + h > rt.Y + rt.H) continue;
                if (src == null || rt.LastDrawFrame > src.LastDrawFrame) src = rt;
            }

        int w1x = src != null ? w + src.Margin * 2 : w;
        int h1x = h;
        float aspect = src is { Margin: > 0 } ? GpuHle.WideAspect : GpuHle.OutputAspect;


        int presentScale = GpuHle.NativeResolution ? 1 : GlVram.Scale;
        int fbW = w1x * presentScale;
        int fbH = h1x * presentScale;
        EnsurePresentSize(fbW, fbH, GpuHle.NativeResolution);

        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, _presentFbo);
        _gl.Viewport(0, 0, (uint)fbW, (uint)fbH);
        _gl.Disable(EnableCap.DepthTest);
        _gl.DepthMask(true);
        _gl.Disable(EnableCap.Blend);
        _gl.Disable(EnableCap.ScissorTest);
        _gl.Disable(EnableCap.CullFace);

        _gl.UseProgram(rgb24 ? _progPresent24 : _progPresent);
        _gl.BindVertexArray(_presentVao);
        _gl.ActiveTexture(TextureUnit.Texture0);
        _gl.BindTexture(TextureTarget.Texture2D, src?.Tex ?? _vram.Texture);
        if (rgb24)
        {
            _gl.Uniform2(_uPresent24Origin, (float)dispX, dispY);
            _gl.Uniform2(_uPresent24Size, (float)w, h);
        }
        else if (src != null)
        {
            _gl.Uniform2(_uPresentOrigin, (float)(dispX - src.X), dispY - src.Y);
            _gl.Uniform2(_uPresentSize, (float)w1x, h1x);
            _gl.Uniform2(_uPresentTexSize, (float)src.Wide1x, src.H);
        }
        else
        {
            _gl.Uniform2(_uPresentOrigin, (float)dispX, dispY);
            _gl.Uniform2(_uPresentSize, (float)w, h);
            _gl.Uniform2(_uPresentTexSize, (float)VramShadow.Width, VramShadow.Height);
        }
        _gl.DrawArrays(PrimitiveType.TriangleStrip, 0, 4);

        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        foreach (var rt in _rts)
            rt?.ClearDepth(_gl);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        GpuHle.DebugCaptureLabel = null;
        return (_presentTex, fbW, fbH, aspect);
    }

    unsafe void EnsurePresentSize(int w, int h, bool nearest)
    {
        if (w == _presentW && h == _presentH && nearest == _presentNearest) return;
        _gl.BindTexture(TextureTarget.Texture2D, _presentTex);
        _gl.TexImage2D(TextureTarget.Texture2D, 0, InternalFormat.Rgba8, (uint)w, (uint)h, 0, PixelFormat.Rgba, PixelType.UnsignedByte, null);
        var filter = nearest ? GLEnum.Nearest : GLEnum.Linear;
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)filter);
        _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)filter);
        _presentW = w; _presentH = h; _presentNearest = nearest;
    }

    public void Dispose()
    {
        foreach (var rt in _rts) rt?.Destroy(_gl);
        _hudSvg?.Dispose();
        _vram.Dispose();
        if (_vbo != 0) _gl.DeleteBuffer(_vbo);
        if (_presentVbo != 0) _gl.DeleteBuffer(_presentVbo);
        if (_vao != 0) _gl.DeleteVertexArray(_vao);
        if (_presentVao != 0) _gl.DeleteVertexArray(_presentVao);
        if (_progPrim != 0) _gl.DeleteProgram(_progPrim);
        if (_progPresent != 0) _gl.DeleteProgram(_progPresent);
        if (_progPresent24 != 0) _gl.DeleteProgram(_progPresent24);
        if (_presentTex != 0) _gl.DeleteTexture(_presentTex);
        if (_presentFbo != 0) _gl.DeleteFramebuffer(_presentFbo);
    }
}
