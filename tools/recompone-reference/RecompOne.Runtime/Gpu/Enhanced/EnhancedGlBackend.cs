using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Globalization;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hle;
using System.Linq;
using Silk.NET.OpenGL;

namespace RecompOne.Runtime.Enhanced;

/// <summary>
/// Modern renderer used by the Enhanced preset.  This backend owns its OpenGL
/// resources, scene depth, material passes and presentation targets.  The
/// original PS1 software rasterizer remains in <see cref="Gpu"/> and is never
/// entered from this class; switching renderers is therefore a host-level
/// choice rather than a collection of fidelity branches inside one rasterizer.
/// </summary>
public sealed class EnhancedGlBackend : Hle.IGpuBackend
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
        public float ViewX, ViewY, ViewZ;
        public float ProjectionCenterX, ProjectionCenterY, ProjectionScale;
        public float HasViewSpace;
        public int Material;
        public float ReplacementX, ReplacementY, ReplacementW, ReplacementH;
        public float ReplacementScaleR, ReplacementScaleG, ReplacementScaleB;
        public float ReplacementBiasR, ReplacementBiasG, ReplacementBiasB;
    }

    const int MaxVerts = 0x40000;
    static readonly bool TraceDepth =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_DEPTH") == "1";
    static readonly bool TraceEnhancedRenderer =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_ENHANCED_RENDERER") == "1";
    static readonly bool TraceConvertedSurfaces =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_CONVERTED_SURFACES") == "1";
    static readonly bool TraceTerrainVram =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VRAM") == "1";
    static readonly bool TraceTextureRegions =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TEXTURE_REGIONS") == "1";
    static readonly bool TraceVehicleTextureReplacements =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_VEHICLE_TEXTURE_REPLACEMENTS") == "1";
    static readonly bool DisableRasterDepth =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_RASTER_DEPTH") == "1";
    static readonly bool DisableProjectiveTextures =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_PROJECTIVE_TEXTURES") == "1";
    static readonly bool TerrainPacketProjection =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_PACKET_PROJECTION") == "1";
    static readonly float TerrainSeamGuardPixels =
        ReadTerrainSeamGuardPixels();
    static readonly bool DebugTerrainCoverage =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_DEBUG_TERRAIN_COVERAGE") == "1";
    // A/B control for the arena-backdrop fog target. With it off the shader
    // falls back to the previous synthesized haze, which is what the
    // before/after acceptance captures compare against.
    static readonly bool FogAtmosphereColor =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_FOG_ATMOSPHERE") != "0";
    // Per-presented-frame terrain volume. Terrain dropping out on alternating
    // frames is invisible to a still capture and to any counter averaged over
    // a window, which is how a packet-arena collision shipped. This is the
    // signal that defect actually lives in, so it is emitted per frame and
    // gated by tools/recompone-v8-2/analyze_terrain_flicker.py.
    static readonly bool TraceTerrainFrames =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_TERRAIN_FRAME") == "1";
    static readonly bool TraceFog =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_FOG") == "1";
    static readonly bool DisableStockPaintCorrection =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_STOCK_PAINT_CORRECTION") == "1";
    static readonly bool DisableVehicleTriangles =
        Environment.GetEnvironmentVariable("RECOMPONE_DISABLE_VEHICLE_TRIANGLES") == "1";
    static readonly HashSet<string> DisabledVehicleMaterials =
        (Environment.GetEnvironmentVariable(
            "RECOMPONE_DISABLE_VEHICLE_MATERIALS") ?? "")
        .Split(
            ';',
            StringSplitOptions.RemoveEmptyEntries |
            StringSplitOptions.TrimEntries)
        .ToHashSet(StringComparer.OrdinalIgnoreCase);
    static readonly HashSet<string> DisabledTriangleSignatures =
        (Environment.GetEnvironmentVariable(
            "RECOMPONE_DISABLE_TRIANGLE_SIGNATURES") ?? "")
        .Split(
            ';',
            StringSplitOptions.RemoveEmptyEntries |
            StringSplitOptions.TrimEntries)
        .ToHashSet(StringComparer.OrdinalIgnoreCase);

    static bool IsOpaqueVehicleGlassMaterial(in PrimFlags f) =>
        f.Material == HleMaterialKind.OpaqueVehicleGlass;

    static int _modalRectLines;
    static int _modalTriLines;
    static readonly bool TraceModalRects =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_MODAL_RECTS") == "1";
    static readonly bool TraceRectangles =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_RECTANGLES") == "1";
    static readonly bool TraceHud =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_HUD") == "1";
    static readonly bool TraceLoadingUiTextures =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_LOADING_UI_TEXTURES") == "1";
    static readonly bool TraceEnhancedFallbacks =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_ENHANCED_FALLBACKS") == "1";
    static readonly bool TracePerformance =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_ENHANCED_PERFORMANCE") == "1";
    static readonly bool TracePresentationFrames =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_PRESENTATION_FRAMES") == "1";
    static readonly bool TraceVehicleMaterials =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_VEHICLE_MATERIALS") == "1";
    static readonly bool TraceSelectorRenderState =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_SELECTOR_RENDER_STATE") == "1";
    static readonly (int Start, int End)? TraceVehicleMaterialTicks =
        ParseTickRange(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_GAMEPLAY_TICKS"));
    static readonly (float X, float Y)? TriangleProbe =
        ParseTriangleProbe(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_TRIANGLE_PROBE"));
    static readonly HashSet<string> TriangleProbeLabels =
        (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_TRIANGLE_LABELS") ?? "")
        .Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)
        .ToHashSet(StringComparer.OrdinalIgnoreCase);
    static readonly bool TraceTriangleProbeContinuously =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TRIANGLE_PROBE_CONTINUOUS") == "1";
    static readonly (int Start, int End)? TraceTriangleProbeFrames =
        ParseTickRange(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_TRIANGLE_PROBE_FRAMES"));
    static readonly HashSet<string> TraceTerrainCells =
        (Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TERRAIN_CELL_TRIANGLES") ?? "")
        .Split(
            ';',
            StringSplitOptions.RemoveEmptyEntries |
            StringSplitOptions.TrimEntries)
        .Select(cell => $"terrain-cell={cell},")
        .ToHashSet(StringComparer.Ordinal);
    static readonly (int Start, int End)? TraceTerrainCellTicks =
        ParseTickRange(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_TERRAIN_CELL_TICKS"));
    static readonly bool TraceTerrainShade =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TERRAIN_SHADE") == "1";
    static readonly bool TraceTerrainDetail =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_TERRAIN_DETAIL") == "1";
    static readonly bool TraceNearClipping =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_NEAR_CLIP") == "1";
    static readonly int[] TraceTerrainScanlines =
        ParseScanlines(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_TERRAIN_SCANLINE"));
    readonly GL _gl;
    readonly GlVram _vram;
    TextureReplacementAtlas? _textureReplacements;
    readonly HashSet<ulong> _vehicleReplacementHits = [];
    readonly HashSet<ulong> _vehicleReplacementMisses = [];
    HudSvgAtlas? _hudSvg;
    readonly GlDisplayRt?[] _rts = new GlDisplayRt?[2];
    long _rtStamp;
    long _frame;
    int _backdropTriangles;
    float _backdropMinX = float.PositiveInfinity;
    float _backdropMaxX = float.NegativeInfinity;
    int _backdropMovedLeft;
    int _backdropMovedRight;
    int _backdropMovedWrongSide;
    readonly HashSet<uint> _backdropPackets = [];
    readonly List<(int A, int B, int C, uint Packet)> _backdropPending = [];
    long _terrainVramTraceFrame = long.MinValue / 2;
    long _traceOpaqueTriangles;
    long _traceTransparentTriangles;
    long _traceDepthTestedTriangles;
    long _tracePainterOrderedWorldTriangles;
    long _tracePainterOrderedScreenTriangles;
    long _traceTransparentWorldDepthTriangles;
    long _traceProjectiveTriangles;
    long _traceMissingOtTriangles;
    long _traceEnhancedTriangles;
    long _traceDirectViewSpaceTriangles;
    long _traceReconstructedViewSpaceTriangles;
    long _traceFallbackTriangles;
    long _traceGlassTriangles;
    long _traceTerrainRouteTriangles;
    long _traceTerrainRouteOpaqueTriangles;
    long _traceTerrainRouteTransparentTriangles;
    long _traceTerrainRouteDepthWriteTriangles;
    long _traceTerrainRouteDepthTestTriangles;
    long _traceTerrainRouteDepthCompareWriteTriangles;
    long _traceTerrainRouteSourceTextured;
    long _traceTerrainRouteSourceCoarse;
    long _traceTerrainRouteSourceTransition;
    long _traceTerrainRouteSourceUnresolved;
    long _terrainFrameTriangles;
    long _terrainFrameWorldTriangles;
    // Non-terrain world geometry binned by where it lands across the widened
    // target, so objects being culled at the outer edges shows up as a band
    // that empties while the middle stays full.
    long _objectFrameLeft, _objectFrameMid, _objectFrameRight;
    readonly HashSet<(int, int, int, int)> _clipRectsLogged = [];
    long _straddleNearPlane, _behindCamera;
    int _severedLogged;
    int _traceNearClipTerrainTriangles;
    int _traceNearClipVehicleTriangles;
    int _traceNearClipObjectTriangles;
    // Coverage of non-terrain world geometry along one scanline. A wall cut in
    // half leaves a gap here with a real primitive on one side of it, which
    // names the thing that stopped being drawn without needing anyone to be
    // parked in front of it.
    // Use each vertex's own camera-space depth when its provenance is exact.
    // A solid object cut mid-surface leaves a long straight run of triangle
    // edges that belong to only one triangle - an open boundary - sitting well
    // inside the frame. Real model silhouettes are short, irregular, or lie at
    // the frame border. This decides whether geometry is being severed without
    // anyone needing to stand in front of it and describe what they see.
    // A wall segment that stops being drawn loses its screen coverage between
    // one frame and the next while the camera moves smoothly. Coverage per
    // object group, compared frame to frame, is the only signal that measures
    // that directly: severed-edge counts and object-pop counts both rise when
    // more geometry is kept, so neither can score a fix.
    static readonly bool TraceVertexHistogram =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_VERTEX_COLUMNS") == "1";
    // A clip line and a real vertical edge both put many vertices in one
    // column. They differ in depth: geometry at one column sits at one
    // distance, while a clip line collects whatever happened to cross it, so
    // its vertices span the whole depth range. Carry the depth spread so the
    // two can be told apart without needing the scene on screen.
    readonly Dictionary<int, (int Count, List<uint> Packets,
        float MinZ, float MaxZ, int Approx)> _vertexColumns = [];
    // One-frame geometry dump. Aggregate metrics cannot localise a defect the
    // harness will not reproduce, and paired screenshots are useless because
    // two identical runs differ across most of the frame. Dumping every drawn
    // triangle of one chosen frame - the frame the player is looking at when
    // they press the key - gives an exact, offline-inspectable record of what
    // was and was not submitted, with no aggregation in between.
    // Where does submitted wall/object geometry actually stop getting near
    // the camera? A dump samples one frame; this accumulates every drawn
    // primitive so the near end of the distribution is not guesswork.
    static readonly long[] NearDepthHistogram = new long[10];
    static readonly float[] NearDepthEdges =
        [20f, 40f, 60f, 80f, 100f, 110f, 120f, 130f, 150f, 200f];

    public static string ConsumeNearDepthHistogram()
    {
        var parts = new List<string>();
        for (int i = 0; i < NearDepthHistogram.Length; i++)
        {
            parts.Add($"<{NearDepthEdges[i]:F0}={NearDepthHistogram[i]}");
            NearDepthHistogram[i] = 0;
        }
        return string.Join(" ", parts);
    }

    // Per-primitive diagnostic; superseded by the band-coverage metric, which
    // reads the geometry dump offline instead of counting during rendering.

    static readonly bool BackdropFill =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_BACKDROP_FILL") != "0";
    static readonly bool TraceBackdropCoverage =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_BACKDROP_COVERAGE") == "1";

    /// <summary>
    /// Reconstructed screen X - what the shader actually draws for a vertex
    /// carrying view-space provenance. The packed <c>X</c> is ignored there,
    /// so moving a backdrop quad means moving <c>ViewX</c>.
    /// </summary>
    static float ReconstructedY(in GlVertex v) =>
        v.HasViewSpace >= 1f && v.ViewZ > 0f && v.ProjectionScale != 0f
            ? v.ProjectionCenterY + v.ViewY * v.ProjectionScale / v.ViewZ
            : v.Y;

    static readonly bool RendererCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_RENDERER_CULL") == "1";
    static readonly bool PacketNclipCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_PACKET_NCLIP_CULL") != "0";
    static readonly bool TracePacketNclipCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_PACKET_NCLIP_CULL") == "1";
    long _packetNclipFrontFaces;
    long _packetNclipBackFaces;
    long _packetNclipRescuedFaces;
    long _packetNclipVehicleFrontFaces;
    long _packetNclipVehicleBackFaces;
    long _packetNclipVehicleRescuedFaces;
    long _packetNclipObjectFrontFaces;
    long _packetNclipObjectBackFaces;
    long _packetNclipObjectRescuedFaces;
    long _packetNclipMissing;

    static float ReconstructedX(in GlVertex v) =>
        v.HasViewSpace >= 1f && v.ViewZ > 0f && v.ProjectionScale != 0f
            ? v.ProjectionCenterX + v.ViewX * v.ProjectionScale / v.ViewZ
            : v.X;

    static void SetReconstructedX(ref GlVertex v, float target)
    {
        if (v.HasViewSpace >= 1f && v.ViewZ > 0f && v.ProjectionScale != 0f)
            v.ViewX = (target - v.ProjectionCenterX) * v.ViewZ /
                v.ProjectionScale;
        v.X = target;
    }

    /// <summary>
    /// Pushes a panorama quad's outermost vertices out to the frame edge,
    /// advancing U at the rate the quad's own vertices establish so the
    /// texture continues instead of stretching.
    /// </summary>
    static bool ExtendBackdropTriangleOuterEdge(
        ref GlVertex a, ref GlVertex b, ref GlVertex c,
        float stripLeft, float stripRight,
        float left, float right)
    {
        Span<float> xs = [ReconstructedX(a), ReconstructedX(b),
            ReconstructedX(c)];
        Span<float> us = [a.U, b.U, c.U];
        float xlo = MathF.Min(xs[0], MathF.Min(xs[1], xs[2]));
        float xhi = MathF.Max(xs[0], MathF.Max(xs[1], xs[2]));

        // The caller measures the complete two-quad panorama strip.  A
        // triangle may move only when it owns that strip's global minimum or
        // maximum.  This is deliberately not based on screen side: the whole
        // strip can rotate far enough that both quad centres lie on the same
        // half of the widescreen target, while their shared join must still
        // remain untouched.
        bool extendLeft = stripLeft > left &&
            MathF.Abs(xlo - stripLeft) < 0.5f;
        bool extendRight = stripRight < right &&
            MathF.Abs(xhi - stripRight) < 0.5f;
        if (!extendLeft && !extendRight)
            return false;

        int p = 0, q = 0;
        float widest = 0f;
        for (int i = 0; i < 3; i++)
            for (int j = i + 1; j < 3; j++)
            {
                float d = MathF.Abs(xs[i] - xs[j]);
                if (d > widest) { widest = d; p = i; q = j; }
            }
        if (widest < 1f)
            return false;
        float dudx = (us[q] - us[p]) / (xs[q] - xs[p]);

        void Push(ref GlVertex v, float x)
        {
            float target = x;
            if (extendLeft && MathF.Abs(x - xlo) < 0.5f) target = left;
            else if (extendRight && MathF.Abs(x - xhi) < 0.5f) target = right;
            if (target == x)
                return;
            v.U += (target - x) * dudx;
            SetReconstructedX(ref v, target);
        }
        Push(ref a, xs[0]);
        Push(ref b, xs[1]);
        Push(ref c, xs[2]);
        return true;
    }

    void ApplyBackdropBatchOuterEdges()
    {
        if (_backdropPending.Count == 0 ||
            _kTarget is not { Margin: > 0 } target)
            return;

        float left = -target.Margin;
        float right = target.Wide1x - target.Margin;
        float stripLeft = float.PositiveInfinity;
        float stripRight = float.NegativeInfinity;
        void Measure(int index)
        {
            float x = ReconstructedX(_verts[index]);
            stripLeft = MathF.Min(stripLeft, x);
            stripRight = MathF.Max(stripRight, x);
        }
        foreach (var pending in _backdropPending)
        {
            Measure(pending.A);
            Measure(pending.B);
            Measure(pending.C);
        }

        // A single panorama quad is narrower than the target.  Requiring a
        // target-width strip makes an unexpected material flush fail closed
        // instead of stretching one half across the display.
        if (stripRight - stripLeft < (right - left) * 0.9f)
        {
            _backdropMovedWrongSide += _backdropPending.Count;
            _backdropPending.Clear();
            return;
        }

        foreach (var pending in _backdropPending)
        {
            ref GlVertex a = ref _verts[pending.A];
            ref GlVertex b = ref _verts[pending.B];
            ref GlVertex c = ref _verts[pending.C];
            float beforeMin = MathF.Min(ReconstructedX(a),
                MathF.Min(ReconstructedX(b), ReconstructedX(c)));
            float beforeMax = MathF.Max(ReconstructedX(a),
                MathF.Max(ReconstructedX(b), ReconstructedX(c)));
            ExtendBackdropTriangleOuterEdge(
                ref a, ref b, ref c,
                stripLeft, stripRight, left, right);
            float afterMin = MathF.Min(ReconstructedX(a),
                MathF.Min(ReconstructedX(b), ReconstructedX(c)));
            float afterMax = MathF.Max(ReconstructedX(a),
                MathF.Max(ReconstructedX(b), ReconstructedX(c)));
            if (afterMin < beforeMin - 0.25f)
                _backdropMovedLeft++;
            if (afterMax > beforeMax + 0.25f)
                _backdropMovedRight++;
            _backdropMinX = MathF.Min(_backdropMinX, afterMin);
            _backdropMaxX = MathF.Max(_backdropMaxX, afterMax);
        }
        _backdropPending.Clear();
    }

    static readonly bool TraceNearDepths =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_NEAR_DEPTHS") == "1";

    // The soak harness deletes stray recompone_present_*.ppm captures unless
    // the label matches one of the patterns it preserves, so paired captures
    // are labelled gameplay_NNN and the index is logged next to the frame.
    static int _geometryDumpIndex;
    static int _drawnFrameCounter;

    static bool _geometryDumpRequested;
    static bool _geometryDumping;
    static System.Text.StringBuilder? _geometryDump;

    public static void RequestGeometryDump() => _geometryDumpRequested = true;

    // Lets the dump be armed without a keypress so it can be exercised in the
    // headless harness.
    static readonly int GeometryDumpFrame =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_GEOMETRY_DUMP_FRAME"),
            out int geometryDumpFrame) ? geometryDumpFrame : 0;

    // Sweeping for an artifact means dumping many frames, not one guessed
    // frame - most frame numbers are 60Hz repeats that draw nothing.
    static readonly int GeometryDumpEvery =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_GEOMETRY_DUMP_EVERY"),
            out int geometryDumpEvery) ? geometryDumpEvery : 0;

    static readonly bool TraceSegmentPop =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_SEGMENT_POP") == "1";
    Dictionary<uint, (float Area, float SumX, float SumY)> _segmentArea = [];
    // The engine alternates between two packet arenas, so an object's packet
    // address changes every drawn frame. Compare against two frames back,
    // which is the same arena and therefore the same identity.
    Dictionary<uint, (float Area, float SumX, float SumY)> _segmentAreaPrevious
        = [];
    Dictionary<uint, (float Area, float SumX, float SumY)> _segmentAreaOlder
        = [];
    int _segmentPops;
    static readonly bool TraceSeveredGeometry =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_SEVERED") == "1";
    readonly List<(uint Packet, float Ax, float Ay, float Bx, float By,
        float Cx, float Cy)> _severedTriangles = [];
    static readonly bool TrueVertexDepth =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRUE_DEPTH") == "1";
    static readonly float? WorldGapScanline =
        float.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TRACE_WORLD_GAP"),
            NumberStyles.Float, CultureInfo.InvariantCulture,
            out float worldGapScanline)
            ? worldGapScanline
            : null;
    readonly List<(float Min, float Max, uint Packet, HleMaterialKind Material)>
        _worldGapSpans = [];
    // How close to the outer edges object geometry ever gets. If the engine
    // culls objects before they reach the edge of the widened view, these stop
    // short of 0 and 1 no matter what is on screen.
    float _objectFrameMinU = 2f, _objectFrameMaxU = -1f;
    long _traceWorldTriangles;
    long _traceWorldFallbackTriangles;
    long _traceEffectFallbackTriangles;
    long _traceVisibleWorldTriangles;
    long _traceVisibleWorldFallbackTriangles;
    long _traceConvertedSurfaceTriangles;
    long _traceVisibleConvertedSurfaceTriangles;
    long _traceDepthTestedConvertedSurfaceTriangles;
    long _traceProjectiveConvertedSurfaceTriangles;
    long _traceModernConvertedSurfaceTriangles;
    double _traceConvertedSurfaceArea;
    double _traceConvertedSurfaceMaxArea;
    long _traceVehicleTriangles;
    long _traceVehicleFallbackTriangles;
    long _traceVisibleVehicleTriangles;
    long _traceVisibleVehicleFallbackTriangles;
    long _traceOpaqueFallbackTriangles;
    long _traceAlphaTestFallbackTriangles;
    long _traceGlassFallbackTriangles;
    long _traceNativeOverspanRejectedTriangles;
    long _traceModernOverspanTriangles;
    long _traceFlushes;
    long _traceMsaaResolves;
    long _tracePresentReallocations;
    long _traceVramFallbackPresents;
    long _traceWideRtPresents;
    long _tracePresentationSourceSwitches;
    long _tracePresentationExtentSwitches;
    int _traceVehicleMaterialTriangles;
    readonly Dictionary<HleMaterialKind, int> _selectorMaterialTriangles = [];
    int _selectorRenderGuest = -1;
    int _selectorRenderGeneration = -1;
    int _selectorRenderStableFrame = -1;
    uint _selectorRenderObject;
    int _selectorOpaqueReflections;
    int _selectorGlossReflections;
    double _selectorTriangleArea;
    double _selectorReflectionArea;
    double _selectorMaxTriangleArea;
    float _selectorMaxSpanX;
    float _selectorMaxSpanY;
    int _traceTerrainCellTriangles;
    int _traceTerrainShadePackets;
    int _traceTerrainDetailPackets;
    readonly Dictionary<string, int> _traceTerrainDetailSourceCounts = [];
    int _traceTerrainScanlineTriangles;
    int _traceRectangleProbeHits;
    long _tracePresentTicks;
    long _tracePresentMaxTicks;
    long _traceFrameIntervalTicks;
    long _traceFrameIntervalMaxTicks;
    long _traceLastPresentStarted;
    int _traceFrameIntervals;
    string _lastPresentationSource = "";
    string _lastPresentationExtent = "";
    long _traceCheckMaskFlushes;
    long _traceSetMaskFlushes;
    long _tracePartialWritebacks;
    long _tracePartialWritebackPixels;
    int _traceMinOt = int.MaxValue;
    int _traceMaxOt;
    readonly HashSet<string> _traceRectangleShapes = [];
    readonly HashSet<string> _traceHudPackets = [];
    readonly HashSet<string> _traceLoadingUiTextures = [];
    readonly HashSet<string> _traceLoadingUiTextureResolves = [];
    readonly HashSet<string> _traceLoadingFontResolves = [];
    readonly HashSet<string> _traceFallbackShapes = [];
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

    static (int Start, int End)? ParseTickRange(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        string[] parts = value.Split('-', 2);
        if (!int.TryParse(parts[0], out int start)) return null;
        int end = start;
        if (parts.Length == 2 && !int.TryParse(parts[1], out end))
            return null;
        return (Math.Min(start, end), Math.Max(start, end));
    }

    static int[] ParseScanlines(string? value)
    {
        if (string.IsNullOrWhiteSpace(value))
            return [];
        string[] parts = value.Split('-', 2);
        if (!int.TryParse(parts[0], out int start))
            return [];
        int end = start;
        if (parts.Length == 2 && !int.TryParse(parts[1], out end))
            return [];
        start = Math.Clamp(start, -512, 1024);
        end = Math.Clamp(end, -512, 1024);
        int minimum = Math.Min(start, end);
        int maximum = Math.Max(start, end);
        return Enumerable.Range(minimum, maximum - minimum + 1).ToArray();
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
    bool _kDepthWrite;
    bool _kSourceDepthCompareWrite;
    HleMaterialKind _kMaterial;
    int _kBlend, _kSetMask, _kCheckMask;
    int _kTwAndX, _kTwAndY, _kTwOrX, _kTwOrY;
    int _kClipX0, _kClipY0, _kClipX1, _kClipY1, _kTextureSmoothing;
    readonly record struct DeferredBatch(
        GlVertex[] Vertices,
        GlDisplayRt? Target,
        bool Transparent,
        bool DepthTest,
        bool DepthWrite,
        bool SourceDepthCompareWrite,
        HleMaterialKind Material,
        int Blend,
        int SetMask,
        int CheckMask,
        int TwAndX,
        int TwAndY,
        int TwOrX,
        int TwOrY,
        int ClipX0,
        int ClipY0,
        int ClipX1,
        int ClipY1,
        int TextureSmoothing);
    readonly List<DeferredBatch> _deferredLoadingPrompt = [];
    int _uTexWindow, _uBlend, _uBlendOpaque, _uSetMask, _uCheckMask, _uPosBias, _uFbInv;
    int _uTextureSmoothing, _uTextureMipmaps, _uAnisotropy;
    int _uEnhancedShadows, _uEnhancedParticles, _uEnhancedFog;
    int _uFogColor, _uFogColorValid;
    // The arena's own horizon colour, harvested from the full-display backdrop
    // quad the engine draws behind every gameplay frame. Distance fog has to
    // converge on this, not on a synthetic haze, or far geometry never joins
    // the sky it is standing against.
    float _fogColorR, _fogColorG, _fogColorB;
    bool _hasFogColor;
    long _fogResetFrame = long.MinValue;
    long _fogColorFrame = long.MinValue;
    int _fogColorOt = int.MinValue;
    float _fogColorSpan;
    bool _fogColorNearWhite;
    int _fogColorLogged;
    int _uPerspectiveCorrectTextures, _uPerspectiveCorrectColors, _uTrueColor;
    int _uVectorFonts, _uVectorIcons;
    int _uPresentOrigin, _uPresentSize, _uPresentTexSize, _uPresent24Origin, _uPresent24Size;

    public bool Ready { get; private set; }

    public EnhancedGlBackend(GL gl) { _gl = gl; _vram = new GlVram(gl); }

    public void ResetTransientState()
    {
        _count = 0;
        _kTarget = null;
        _backdropPending.Clear();
        _backdropPackets.Clear();
        _deferredLoadingPrompt.Clear();
        ResetAtmosphereState();
    }

    public void ResetAtmosphereState()
    {
        _hasFogColor = false;
        _fogColorR = 0f;
        _fogColorG = 0f;
        _fogColorB = 0f;
        _fogResetFrame = _frame;
        _fogColorFrame = long.MinValue;
        _fogColorOt = int.MinValue;
        _fogColorSpan = 0f;
        _fogColorNearWhite = false;
        _fogColorLogged = 0;
        if (TraceFog)
            Console.Error.WriteLine(
                $"[EnhancedFogReset] frame={_fogResetFrame}");
    }

    void TraceSelectorTriangle(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        in PrimFlags f)
    {
        if (!TraceSelectorRenderState || GpuHle.GameplayActive || !f.Vehicle)
            return;
        int guest = Sdk.V82VehicleRegistry.NativeSelectorGuestIndex;
        if (guest < 0)
            return;

        _selectorRenderGuest = guest;
        _selectorRenderGeneration =
            Sdk.V82VehicleRegistry.NativeSelectorGeneration;
        _selectorRenderStableFrame =
            Sdk.V82VehicleRegistry.NativeSelectorStableFrame;
        _selectorRenderObject =
            Sdk.V82VehicleRegistry.NativeSelectorPreviewObject;
        _selectorMaterialTriangles[f.Material] =
            _selectorMaterialTriangles.GetValueOrDefault(f.Material) + 1;
        if (f.Material == HleMaterialKind.VehicleReflection)
        {
            if (f.SemiTrans)
                _selectorGlossReflections++;
            else
                _selectorOpaqueReflections++;
        }

        double twiceArea = Math.Abs(
            (b.X - a.X) * (c.Y - a.Y) -
            (c.X - a.X) * (b.Y - a.Y));
        double area = twiceArea * 0.5;
        _selectorTriangleArea += area;
        if (f.Material == HleMaterialKind.VehicleReflection)
            _selectorReflectionArea += area;
        _selectorMaxTriangleArea = Math.Max(_selectorMaxTriangleArea, area);
        _selectorMaxSpanX = Math.Max(
            _selectorMaxSpanX,
            Math.Max(a.X, Math.Max(b.X, c.X)) -
            Math.Min(a.X, Math.Min(b.X, c.X)));
        _selectorMaxSpanY = Math.Max(
            _selectorMaxSpanY,
            Math.Max(a.Y, Math.Max(b.Y, c.Y)) -
            Math.Min(a.Y, Math.Min(b.Y, c.Y)));
    }

    void FlushSelectorRenderTrace()
    {
        if (_selectorMaterialTriangles.Count == 0)
            return;
        int triangleCount = _selectorMaterialTriangles.Values.Sum();
        string materials = string.Join(
            ',',
            _selectorMaterialTriangles
                .OrderBy(pair => pair.Key.ToString(), StringComparer.Ordinal)
                .Select(pair => $"{pair.Key}:{pair.Value}"));
        Console.Error.WriteLine(
            "[V82SelectorRenderFrame] " +
            $"present={_frame} guest={_selectorRenderGuest} " +
            $"generation={_selectorRenderGeneration} " +
            $"stable={_selectorRenderStableFrame} " +
            $"object=0x{_selectorRenderObject:X8} " +
            $"triangles={triangleCount} " +
            $"reflection-opaque={_selectorOpaqueReflections} " +
            $"reflection-gloss={_selectorGlossReflections} " +
            $"area={_selectorTriangleArea:F3} " +
            $"reflection-area={_selectorReflectionArea:F3} " +
            $"max-area={_selectorMaxTriangleArea:F3} " +
            $"max-span={_selectorMaxSpanX:F3},{_selectorMaxSpanY:F3} " +
            $"materials={materials}");
        _selectorMaterialTriangles.Clear();
        _selectorRenderGuest = -1;
        _selectorRenderGeneration = -1;
        _selectorRenderStableFrame = -1;
        _selectorRenderObject = 0u;
        _selectorOpaqueReflections = 0;
        _selectorGlossReflections = 0;
        _selectorTriangleArea = 0.0;
        _selectorReflectionArea = 0.0;
        _selectorMaxTriangleArea = 0.0;
        _selectorMaxSpanX = 0f;
        _selectorMaxSpanY = 0f;
    }

    public void ApplyResolutionScale(
        int scale, ReadOnlySpan<ushort> nativeVram)
    {
        scale = Math.Clamp(scale, 1, 4);
        if (scale == GlVram.Scale) return;

        Flush();
        for (int index = 0; index < _rts.Length; index++)
        {
            _rts[index]?.Destroy(_gl);
            _rts[index] = null;
        }
        _kTarget = null;
        _vram.ReinitializeScale(scale, nativeVram);
        _textureReplacements?.Reset(nativeVram);
        _readCacheValid = false;
        _presentW = 0;
        _presentH = 0;

        _gl.UseProgram(_progPrim);
        _gl.Uniform1(
            _gl.GetUniformLocation(_progPrim, "uScale"), GlVram.Scale);
        _gl.UseProgram(_progPresent24);
        _gl.Uniform1(
            _gl.GetUniformLocation(_progPresent24, "uScale"),
            GlVram.Scale);
        _gl.UseProgram(0);
    }

    public unsafe void InitGl()
    {
        _vram.Init();

        _progPrim = GlShaders.Build(_gl, GlShaders.PrimVs, GlShaders.PrimFs, "prim");
        _progPresent = GlShaders.Build(_gl, GlShaders.FullscreenVs, GlShaders.PresentFs, "present");
        _progPresent24 = GlShaders.Build(_gl, GlShaders.FullscreenVs, GlShaders.Present24Fs, "present24");
        if (_progPrim == 0 || _progPresent == 0 || _progPresent24 == 0) return;
        _hudSvg = new HudSvgAtlas(_gl);
        _textureReplacements = new TextureReplacementAtlas(_gl);

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
        _uFogColor = _gl.GetUniformLocation(_progPrim, "uFogColor");
        _uFogColorValid = _gl.GetUniformLocation(_progPrim, "uFogColorValid");
        _uPerspectiveCorrectTextures =
            _gl.GetUniformLocation(_progPrim, "uPerspectiveCorrectTextures");
        _uPerspectiveCorrectColors =
            _gl.GetUniformLocation(_progPrim, "uPerspectiveCorrectColors");
        _uTrueColor = _gl.GetUniformLocation(_progPrim, "uTrueColor");
        _uVectorFonts = _gl.GetUniformLocation(_progPrim, "uVectorFonts");
        _uVectorIcons = _gl.GetUniformLocation(_progPrim, "uVectorIcons");
        _uPosBias = _gl.GetUniformLocation(_progPrim, "uPosBias");
        _uFbInv = _gl.GetUniformLocation(_progPrim, "uFbInv");

        _gl.UseProgram(_progPrim);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uVram"), 0);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uDest"), 1);
        _gl.Uniform1(_gl.GetUniformLocation(_progPrim, "uHudSvg"), 2);
        _gl.Uniform1(
            _gl.GetUniformLocation(_progPrim, "uReplacementAtlas"), 3);
        _gl.Uniform2(
            _gl.GetUniformLocation(_progPrim, "uReplacementAtlasSize"),
            (float)(_textureReplacements?.Width ?? 1),
            (float)(_textureReplacements?.Height ?? 1));
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
        _gl.EnableVertexAttribArray(10); _gl.VertexAttribPointer(10, 3, VertexAttribPointerType.Float, false, stride, (void*)68);
        _gl.EnableVertexAttribArray(11); _gl.VertexAttribPointer(11, 3, VertexAttribPointerType.Float, false, stride, (void*)80);
        _gl.EnableVertexAttribArray(12); _gl.VertexAttribPointer(12, 1, VertexAttribPointerType.Float, false, stride, (void*)92);
        _gl.EnableVertexAttribArray(13); _gl.VertexAttribIPointer(13, 1, VertexAttribIType.Int, stride, (void*)96);
        _gl.EnableVertexAttribArray(14); _gl.VertexAttribPointer(14, 4, VertexAttribPointerType.Float, false, stride, (void*)100);
        _gl.EnableVertexAttribArray(15); _gl.VertexAttribPointer(15, 3, VertexAttribPointerType.Float, false, stride, (void*)116);
        _gl.EnableVertexAttribArray(16); _gl.VertexAttribPointer(16, 3, VertexAttribPointerType.Float, false, stride, (void*)128);

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
        Resolve(rt);
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

    void Resolve(GlDisplayRt rt)
    {
        if (rt.MsaaFbo != 0 && rt.NeedsResolve && TracePerformance)
            _traceMsaaResolves++;
        rt.Resolve(_gl);
    }

    void ResolveCheckMaskRegion(GlDisplayRt rt)
    {
        if (rt.MsaaFbo == 0 || !rt.NeedsResolve)
            return;
        int x0 = _kClipX0 - rt.X + rt.Margin;
        int y0 = _kClipY0 - rt.Y;
        int x1 = _kClipX1 - rt.X + rt.Margin + 1;
        int y1 = _kClipY1 - rt.Y + 1;
        if (rt.Margin > 0 &&
            _kClipX0 <= rt.X &&
            _kClipX1 >= rt.X + rt.W - 1)
        {
            x0 = 0;
            x1 = rt.Wide1x;
        }
        if (TracePerformance)
            _traceMsaaResolves++;
        rt.ResolveRegion(_gl, x0, y0, x1, y1);
    }

    void WritebackRegion(GlDisplayRt rt, int x, int y, int w, int h)
    {
        int x0 = Math.Max(x, rt.X);
        int y0 = Math.Max(y, rt.Y);
        int x1 = Math.Min(x + w, rt.X + rt.W);
        int y1 = Math.Min(y + h, rt.Y + rt.H);
        if (x0 >= x1 || y0 >= y1) return;
        if (x0 == rt.X && y0 == rt.Y &&
            x1 == rt.X + rt.W && y1 == rt.Y + rt.H)
        {
            Writeback(rt);
            return;
        }

        int localX0 = x0 - rt.X + rt.Margin;
        int localY0 = y0 - rt.Y;
        int localX1 = x1 - rt.X + rt.Margin;
        int localY1 = y1 - rt.Y;
        rt.ResolveRegion(_gl, localX0, localY0, localX1, localY1);
        int s = GlVram.Scale;
        _gl.Disable(EnableCap.ScissorTest);
        _gl.BindFramebuffer(
            FramebufferTarget.ReadFramebuffer, rt.Fbo);
        _gl.BindFramebuffer(
            FramebufferTarget.DrawFramebuffer, _vram.Fbo);
        _gl.BlitFramebuffer(
            localX0 * s, localY0 * s, localX1 * s, localY1 * s,
            x0 * s, y0 * s, x1 * s, y1 * s,
            ClearBufferMask.ColorBufferBit,
            BlitFramebufferFilter.Nearest);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
        if (TracePerformance)
        {
            _tracePartialWritebacks++;
            _tracePartialWritebackPixels +=
                (long)(x1 - x0) * (y1 - y0);
        }
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
        rt.CopyRegionToMsaa(
            _gl,
            x0 - rt.X + rt.Margin,
            y0 - rt.Y,
            x1 - rt.X + rt.Margin,
            y1 - rt.Y);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
    }

    void WritebackDirtyIntersecting(int x, int y, int w, int h)
    {
        foreach (var rt in _rts)
            if (rt is { Dirty: true } && rt.Intersects(x, y, w, h))
                WritebackRegion(rt, x, y, w, h);
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

    void CheckTextureFeedback(
        in PrimFlags f,
        int minU,
        int minV,
        int maxU,
        int maxV)
    {
        if (!f.Textured) return;
        int px = (f.TPage & 0xF) * 64;
        int py = ((f.TPage >> 4) & 1) * 256;
        int depth = (f.TPage >> 7) & 3;
        int texelsPerWord = depth == 0 ? 4 : depth == 1 ? 2 : 1;
        minU = Math.Clamp(minU, 0, 255);
        minV = Math.Clamp(minV, 0, 255);
        maxU = Math.Clamp(maxU, minU, 255);
        maxV = Math.Clamp(maxV, minV, 255);
        int x0 = px + minU / texelsPerWord;
        int x1 = px + (maxU + texelsPerWord) / texelsPerWord;
        int y0 = py + minV;
        int y1 = py + maxV + 1;
        foreach (var rt in _rts)
            if (rt is { Dirty: true } &&
                rt.Intersects(x0, y0, x1 - x0, y1 - y0))
            {
                Flush();
                WritebackRegion(rt, x0, y0, x1 - x0, y1 - y0);
            }
    }

    bool DesiredMatches(
        bool transparent,
        int blend,
        bool depthTest,
        bool depthWrite,
        bool sourceDepthCompareWrite,
        HleMaterialKind material)
    {
        int twAndX = ~(_env.TwMaskX * 8) & 0xFF, twAndY = ~(_env.TwMaskY * 8) & 0xFF;
        int twOrX = (_env.TwOffX & _env.TwMaskX) * 8, twOrY = (_env.TwOffY & _env.TwMaskY) * 8;
        return _kTransparent == transparent && _kBlend == blend &&
            _kDepthTest == depthTest &&
            _kDepthWrite == depthWrite &&
            _kSourceDepthCompareWrite == sourceDepthCompareWrite
            && _kMaterial == material
            && _kSetMask == (_env.SetMask ? 1 : 0) && _kCheckMask == (_env.CheckMask ? 1 : 0)
            && _kTwAndX == twAndX && _kTwAndY == twAndY && _kTwOrX == twOrX && _kTwOrY == twOrY
            && _kClipX0 == _env.ClipX0 && _kClipY0 == _env.ClipY0 && _kClipX1 == _env.ClipX1 && _kClipY1 == _env.ClipY1
            && _kTextureSmoothing == (ConfigManager.View.TextureSmoothing ? 1 : 0);
    }

    void Begin(
        in PrimFlags f,
        int vertsNeeded,
        bool depthTest = false,
        bool depthWrite = false,
        bool sourceDepthCompareWrite = false)
    {
        _readCacheValid = false;
        bool opaqueVehicleGlass = IsOpaqueVehicleGlassMaterial(f);
        bool transparent = f.SemiTrans && !opaqueVehicleGlass;
        int blend = f.BlendMode;
        HleMaterialKind material = f.Material;
        var target = Classify();
        if (_count > 0 &&
            (target != _kTarget ||
             !DesiredMatches(
                 transparent,
                 blend,
                 depthTest,
                 depthWrite,
                 sourceDepthCompareWrite,
                 material))) Flush();
        if (_count + vertsNeeded > MaxVerts) Flush();
        _kTarget = target;
        _kTransparent = transparent; _kBlend = blend;
        _kDepthTest = depthTest;
        _kDepthWrite = depthWrite;
        _kSourceDepthCompareWrite = sourceDepthCompareWrite;
        _kMaterial = material;
        _kSetMask = _env.SetMask ? 1 : 0; _kCheckMask = _env.CheckMask ? 1 : 0;
        _kTwAndX = ~(_env.TwMaskX * 8) & 0xFF; _kTwAndY = ~(_env.TwMaskY * 8) & 0xFF;
        _kTwOrX = (_env.TwOffX & _env.TwMaskX) * 8; _kTwOrY = (_env.TwOffY & _env.TwMaskY) * 8;
        _kClipX0 = _env.ClipX0; _kClipY0 = _env.ClipY0; _kClipX1 = _env.ClipX1; _kClipY1 = _env.ClipY1;
        _kTextureSmoothing = ConfigManager.View.TextureSmoothing ? 1 : 0;
    }

    bool DitherOf(in PrimFlags f) =>
        ConfigManager.View.Ps1Dithering &&
        _env.Dither &&
        (f.Gouraud || (f.Textured && !f.RawTexture));

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
        bool exactSemitransEffect = f.Material is
            HleMaterialKind.Additive or
            HleMaterialKind.Subtractive;
        if (ConfigManager.View.TextureSmoothing && f.Textured &&
            !exactSemitransEffect &&
            (!f.RawTexture || !uiTexture))
            tpage |= 0x800;
        if (uiTexture) tpage |= 0x1000;
        if (exactSemitransEffect && f.Textured && f.SemiTrans)
            tpage |= 0x800000;
        if (f.Vehicle) tpage |= 0x80000;
        if (f.N64RouteColor) tpage |= 0x200000;
        float perspectiveW =
            perspectiveCorrect ? MathF.Max(1f, v.PerspectiveW) : 1f;
        float fogDepth = v.HasProjectiveW
            ? MathF.Max(1f, v.PerspectiveW)
            : v.HasGteZ
                ? MathF.Max(1f, v.Z)
                : MathF.Max(1f, f.OtIndex * 8f);
        float screenX = v.X;
        float screenY = v.Y;
        float projectionCenterX = v.ProjectionCenterX;
        float projectionCenterY = v.ProjectionCenterY;
        float projectionScale = v.ProjectionScale;
        if (!uiTexture &&
            GpuHle.GameplayActive &&
            _kTarget is { Margin: > 0 } wideTarget)
        {
            // Preserve the native vertical FOV and projection scale. The
            // enhanced target owns real pixels on both sides of the authored
            // 4:3 viewport, so centering the recovered projection in that
            // target exposes additional horizontal view instead of stretching
            // or vertically cropping the original camera.
            screenX += wideTarget.Margin;
            projectionCenterX += wideTarget.Margin;
        }
        return new GlVertex
        {
            X = screenX, Y = screenY,
            Color = color,
            Clut = f.Clut & 0x7FFF,
            Texpage = tpage,
            U = v.U, V = v.V,
            PerspectiveW = perspectiveW,
            Depth = fogDepth,
            RasterDepth = rasterDepth,
            ViewX = v.ViewX,
            ViewY = v.ViewY,
            ViewZ = v.ViewZ,
            ProjectionCenterX = projectionCenterX,
            ProjectionCenterY = projectionCenterY,
            ProjectionScale = projectionScale,
            // 2 = exact address/value GTE provenance, 1 = a complete
            // primitive reconstructed from coherent GTE depth, 0 = stable
            // screen-space fallback.  The shader accepts both 1 and 2 for
            // modern clipping/projection; telemetry keeps them distinct so
            // an OT approximation can never masquerade as exact geometry.
            HasViewSpace = v.HasViewSpace
                ? (v.ReconstructedViewSpace ? 1f : 2f)
                : 0f,
            Material = (int)f.Material,
        };
    }

    // Geometry passing beside or behind the camera has vertices at or behind
    // the eye. Those project to a non-positive w, and the hardware then clips
    // the triangle against w=0 using a depth this renderer derives from the
    // ordering table rather than from the projection - so the cut lands
    // somewhere arbitrary and reads as a wall ending in mid-air. Clip in view
    // space first, where the geometry is exact, and hand the hardware only
    // triangles that are wholly in front.
    static readonly float NearPlaneViewZ =
        float.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_ENHANCED_NEAR_PLANE"),
            NumberStyles.Float,
            CultureInfo.InvariantCulture,
            out float configuredNearPlane)
            ? Math.Clamp(configuredNearPlane, 1f, 256f)
            : 1f;
    static readonly bool NearPlaneClipping =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_NEAR_CLIP") != "0";
    bool _clippingNearPlane;

    void ReportSegmentPops()
    {
        if (!TraceSegmentPop)
            return;
        if ((_frame % 120) == 0)
        {
            Console.Error.WriteLine(
                $"[V82SegmentPopTotal] frame={_frame} pops={_segmentPops}");
            Sdk.V82Compat.ReportObjectRenderGates();
        }
        if (TraceVertexHistogram && (_frame % 600) == 0 &&
            _vertexColumns.Count != 0)
        {
            foreach (var e in _vertexColumns.OrderBy(e => e.Key))
                Console.Error.WriteLine(
                    $"[V82VertexColumn] frame={_frame} " +
                    $"x={e.Key / 2f:F1} vertices={e.Value.Count} " +
                    $"distinctPackets={e.Value.Packets.Distinct().Count()} " +
                    $"minZ={(e.Value.MinZ == float.MaxValue ? 0f : e.Value.MinZ):F0} " +
                    $"maxZ={(e.Value.MaxZ == float.MinValue ? 0f : e.Value.MaxZ):F0} " +
                    $"approx={e.Value.Approx}");
            _vertexColumns.Clear();
        }
        // A present that drew nothing is the 60Hz repeat of the previous
        // image, not every object vanishing at once.
        if (_segmentArea.Count == 0)
            return;
        var target = _kTarget;
        float width = target?.Wide1x ?? 428;
        float height = target?.H ?? 240;
        foreach (var (group, was) in _segmentAreaOlder)
        {
            if (was.Area < 200f)
                continue;
            _segmentArea.TryGetValue(group, out var now);
            if (now.Area >= was.Area * 0.30f)
                continue;
            // Only count it when the geometry was comfortably inside the view,
            // so an object genuinely leaving the frame is not a pop.
            float cx = was.SumX / was.Area;
            float cy = was.SumY / was.Area;
            if (cx < width * 0.06f || cx > width * 0.94f ||
                cy < height * 0.06f || cy > height * 0.94f)
                continue;
            _segmentPops++;
            if (_segmentPops <= 25)
                Console.Error.WriteLine(
                    $"[V82SegmentPop] frame={_frame} group=0x{group:X8} " +
                    $"area={was.Area:F0}->{now.Area:F0} " +
                    $"centre=({cx:F0},{cy:F0})");
        }
        var recycled = _segmentAreaOlder;
        _segmentAreaOlder = _segmentAreaPrevious;
        _segmentAreaPrevious = _segmentArea;
        _segmentArea = recycled;
        _segmentArea.Clear();
    }

    void ReportSeveredGeometry()
    {
        if (!TraceSeveredGeometry || _severedTriangles.Count == 0)
        {
            _severedTriangles.Clear();
            return;
        }
        var target = _kTarget;
        float width = target?.Wide1x ?? 428;
        float height = target?.H ?? 240;

        // Edges shared by two triangles of the same object are interior; the
        // ones left over are the object's boundary.
        var edges = new Dictionary<(uint, long, long, long, long), int>();
        static long Q(float v) => (long)MathF.Round(v * 4f);
        foreach (var t in _severedTriangles)
        {
            void Add(float x0, float y0, float x1, float y1)
            {
                var k1 = (Q(x0), Q(y0));
                var k2 = (Q(x1), Q(y1));
                var key = k1.CompareTo(k2) <= 0
                    ? (t.Packet & 0xFFFFF000u, k1.Item1, k1.Item2, k2.Item1, k2.Item2)
                    : (t.Packet & 0xFFFFF000u, k2.Item1, k2.Item2, k1.Item1, k1.Item2);
                edges[key] = edges.TryGetValue(key, out int n) ? n + 1 : 1;
            }
            Add(t.Ax, t.Ay, t.Bx, t.By);
            Add(t.Bx, t.By, t.Cx, t.Cy);
            Add(t.Cx, t.Cy, t.Ax, t.Ay);
        }

        foreach (var (key, count) in edges)
        {
            if (count != 1) continue;
            float x0 = key.Item2 / 4f, y0 = key.Item3 / 4f;
            float x1 = key.Item4 / 4f, y1 = key.Item5 / 4f;
            float length = MathF.Sqrt(
                (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
            if (length < 60f) continue;
            // Ignore anything touching the frame border: that is the view
            // clipping the object, which is correct.
            float margin = 6f;
            if (MathF.Min(x0, x1) < margin ||
                MathF.Max(x0, x1) > width - margin ||
                MathF.Min(y0, y1) < margin ||
                MathF.Max(y0, y1) > height - margin) continue;
            bool vertical = MathF.Abs(x1 - x0) < MathF.Abs(y1 - y0) * 0.3f;
            if (!vertical) continue;
            if (_severedLogged++ >= 100000) return;
            // Name the exact primitive that owns this open edge, and say where
            // that edge sits in the packed coordinate space retail clips in.
            uint owner = 0;
            foreach (var t in _severedTriangles)
                if ((t.Packet & 0xFFFFF000u) == key.Item1 &&
                    (MathF.Abs(t.Ax - x0) < 0.3f && MathF.Abs(t.Ay - y0) < 0.3f ||
                     MathF.Abs(t.Bx - x0) < 0.3f && MathF.Abs(t.By - y0) < 0.3f ||
                     MathF.Abs(t.Cx - x0) < 0.3f && MathF.Abs(t.Cy - y0) < 0.3f))
                { owner = t.Packet; break; }
            float margin2 = target?.Margin ?? 0;
            float nativeX = x0 - margin2;
            float packedX = 160f + (nativeX - 160f) * 0.75f;
            Console.Error.WriteLine(
                $"[V82SeveredEdge] frame={_frame} " +
                $"packet=0x{owner:X8} " +
                $"owner=\"{GpuHle.DescribePacketOwner(owner)}\" " +
                $"edge=({x0:F1},{y0:F1})..({x1:F1},{y1:F1}) " +
                $"length={length:F1} nativeX={nativeX:F1} " +
                $"packedX={packedX:F1}");
        }
        _severedTriangles.Clear();
    }

    void ReportWorldGaps()
    {
        if (WorldGapScanline is null || _worldGapSpans.Count == 0)
        {
            _worldGapSpans.Clear();
            return;
        }
        _worldGapSpans.Sort((l, r) => l.Min.CompareTo(r.Min));
        float reach = _worldGapSpans[0].Max;
        uint before = _worldGapSpans[0].Packet;
        for (int i = 1; i < _worldGapSpans.Count; i++)
        {
            var span = _worldGapSpans[i];
            if (span.Min - reach >= 6f && reach > 2f && span.Min < 420f)
                Console.Error.WriteLine(
                    $"[V82WorldGap] frame={_frame} y={WorldGapScanline:F0} " +
                    $"gap={reach:F1}..{span.Min:F1} " +
                    $"width={span.Min - reach:F1} " +
                    $"leftPacket=0x{before:X8} rightPacket=0x{span.Packet:X8} " +
                    $"rightMaterial={span.Material}");
            if (span.Max > reach)
            {
                reach = span.Max;
                before = span.Packet;
            }
        }
        _worldGapSpans.Clear();
    }

    static HleVertex LerpVertex(in HleVertex from, in HleVertex to, float t)
    {
        HleVertex result = from;
        result.ViewX = from.ViewX + (to.ViewX - from.ViewX) * t;
        result.ViewY = from.ViewY + (to.ViewY - from.ViewY) * t;
        result.ViewZ = from.ViewZ + (to.ViewZ - from.ViewZ) * t;
        result.X = from.X + (to.X - from.X) * t;
        result.Y = from.Y + (to.Y - from.Y) * t;
        result.Z = from.Z + (to.Z - from.Z) * t;
        result.PerspectiveW =
            from.PerspectiveW + (to.PerspectiveW - from.PerspectiveW) * t;
        result.U = (short)MathF.Round(from.U + (to.U - from.U) * t);
        result.V = (short)MathF.Round(from.V + (to.V - from.V) * t);
        result.R = (byte)Math.Clamp(
            (int)MathF.Round(from.R + (to.R - from.R) * t), 0, 255);
        result.G = (byte)Math.Clamp(
            (int)MathF.Round(from.G + (to.G - from.G) * t), 0, 255);
        result.B = (byte)Math.Clamp(
            (int)MathF.Round(from.B + (to.B - from.B) * t), 0, 255);
        return result;
    }

    /// <summary>
    /// Clips a triangle against the camera-space near plane and redraws the
    /// visible remainder. Returns true when it handled the primitive.
    /// </summary>
    bool ClipAgainstNearPlane(
        in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f)
    {
        if (!NearPlaneClipping || _clippingNearPlane ||
            !a.HasViewSpace || !b.HasViewSpace || !c.HasViewSpace)
            return false;
        if (a.ViewZ >= NearPlaneViewZ &&
            b.ViewZ >= NearPlaneViewZ &&
            c.ViewZ >= NearPlaneViewZ)
            return false;
        if (a.ViewZ < NearPlaneViewZ &&
            b.ViewZ < NearPlaneViewZ &&
            c.ViewZ < NearPlaneViewZ)
        {
            if (ShouldTraceNearClip(f))
                Console.Error.WriteLine(
                    $"[EnhancedNearClip] frame={_frame} " +
                    $"tick={GpuHle.DebugGameplayTick} " +
                    $"packet=0x{f.PacketAddress:X8} " +
                    $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                    $"material={f.Material} vehicle={(f.Vehicle ? 1 : 0)} " +
                    $"view-z={a.ViewZ:F3},{b.ViewZ:F3},{c.ViewZ:F3} " +
                    "result=dropped-behind-camera");
            return true;
        }

        Span<HleVertex> source = [a, b, c];
        Span<HleVertex> kept = stackalloc HleVertex[4];
        int count = 0;
        for (int i = 0; i < 3; i++)
        {
            HleVertex current = source[i];
            HleVertex next = source[(i + 1) % 3];
            bool currentIn = current.ViewZ >= NearPlaneViewZ;
            bool nextIn = next.ViewZ >= NearPlaneViewZ;
            if (currentIn)
                kept[count++] = current;
            if (currentIn != nextIn)
            {
                float span = next.ViewZ - current.ViewZ;
                float t = MathF.Abs(span) < 0.0001f
                    ? 0f
                    : (NearPlaneViewZ - current.ViewZ) / span;
                kept[count++] = LerpVertex(current, next, Math.Clamp(t, 0f, 1f));
            }
        }

        if (ShouldTraceNearClip(f))
            Console.Error.WriteLine(
                $"[EnhancedNearClip] frame={_frame} " +
                $"tick={GpuHle.DebugGameplayTick} " +
                $"packet=0x{f.PacketAddress:X8} " +
                $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                $"material={f.Material} vehicle={(f.Vehicle ? 1 : 0)} " +
                $"view-z={a.ViewZ:F3},{b.ViewZ:F3},{c.ViewZ:F3} " +
                $"result=clipped vertices={count}");

        _clippingNearPlane = true;
        try
        {
            if (count >= 3)
                DrawTriCore(kept[0], kept[1], kept[2], f);
            if (count == 4)
                DrawTriCore(kept[0], kept[2], kept[3], f);
        }
        finally
        {
            _clippingNearPlane = false;
        }
        return true;
    }

    bool ApplyTextureReplacement(
        ref GlVertex a, ref GlVertex b, ref GlVertex c,
        in PrimFlags f,
        float minU, float minV, float maxU, float maxV,
        out ulong textureKey,
        out string resolution,
        bool allowUiReplacement = false)
    {
        textureKey = 0;
        resolution = "kind=disabled source-label=\"none\" shadow=\"none\"";
        bool uiMaterial = f.Material is
            HleMaterialKind.Ui or HleMaterialKind.ScreenEffect;
        if (!ConfigManager.View.HighResolutionTextures ||
            !f.Textured || _textureReplacements == null)
            return false;
        int sourceMinU = (int)MathF.Floor(minU);
        int sourceMinV = (int)MathF.Floor(minV);
        int sourceMaxU = (int)MathF.Ceiling(maxU);
        int sourceMaxV = (int)MathF.Ceiling(maxV);
        TextureReplacementAtlas.Rect rect =
            _textureReplacements.ResolveFontFile(
                f.TPage,
                sourceMinU, sourceMinV, sourceMaxU, sourceMaxV,
                _kTwAndX, _kTwAndY, _kTwOrX, _kTwOrY,
                out string fontPath, out bool fileFont);
        if (fileFont)
        {
            resolution = rect.Valid
                ? $"kind=file-font path=\"{fontPath}\" " +
                  $"replacement-scale={rect.W / Math.Max(1f, sourceMaxU - sourceMinU + 1f):F3}x" +
                  $"{rect.H / Math.Max(1f, sourceMaxV - sourceMinV + 1f):F3} " +
                  "shadow=\"none\""
                : $"kind=file-font-fallback path=\"{fontPath}\" " +
                  "source-label=\"original FNT\" shadow=\"none\"";
            if (!rect.Valid)
                return false;
            ApplyReplacementRect(ref a, ref b, ref c, rect);
            // A filename-resolved FNT is a font at every screen position and
            // size, including the upper half of native pause modals.
            a.Texpage |= 0x3800;
            b.Texpage |= 0x3800;
            c.Texpage |= 0x3800;
            return true;
        }
        if (
            f.Material is HleMaterialKind.Particle or
                HleMaterialKind.Additive or
                HleMaterialKind.Subtractive or
                HleMaterialKind.VehicleReflection or
                HleMaterialKind.OpaqueVehicleGlass ||
            (uiMaterial && !allowUiReplacement))
            return false;
        rect = _textureReplacements.Resolve(
            f.TPage, f.Clut,
            sourceMinU, sourceMinV, sourceMaxU, sourceMaxV,
            _kTwAndX, _kTwAndY, _kTwOrX, _kTwOrY,
            f.Material, out textureKey);
        int sourceWidth =
            (int)MathF.Ceiling(maxU) - (int)MathF.Floor(minU) + 1;
        int sourceHeight =
            (int)MathF.Ceiling(maxV) - (int)MathF.Floor(minV) + 1;
        resolution = _textureReplacements.DescribeResolution(
            textureKey, sourceWidth, sourceHeight);
        if (f.Vehicle && TraceVehicleTextureReplacements)
        {
            HashSet<ulong> keys = rect.Valid
                ? _vehicleReplacementHits
                : _vehicleReplacementMisses;
            if (keys.Add(textureKey))
                Console.WriteLine(
                    $"[TexturePack] vehicle {(rect.Valid ? "hit" : "miss")} " +
                    $"key={textureKey:x16} " +
                    $"size={sourceWidth}x{sourceHeight} " +
                    $"unique={_vehicleReplacementHits.Count}/" +
                    $"{_vehicleReplacementHits.Count + _vehicleReplacementMisses.Count}");
        }
        if (TraceLoadingUiTextures &&
            GpuHle.GameplayActive &&
            GpuHle.DebugGameplayTick == 0 &&
            uiMaterial &&
            _traceLoadingUiTextureResolves.Count < 2048)
        {
            float x0 = MathF.Min(a.X, MathF.Min(b.X, c.X));
            float y0 = MathF.Min(a.Y, MathF.Min(b.Y, c.Y));
            float x1 = MathF.Max(a.X, MathF.Max(b.X, c.X));
            float y1 = MathF.Max(a.Y, MathF.Max(b.Y, c.Y));
            int iu0 = (int)MathF.Floor(minU);
            int iv0 = (int)MathF.Floor(minV);
            int iu1 = (int)MathF.Ceiling(maxU);
            int iv1 = (int)MathF.Ceiling(maxV);
            string packet =
                $"frame={_frame} key={textureKey:x16} hit={(rect.Valid ? 1 : 0)} " +
                $"screen={x0:F1},{y0:F1}-{x1:F1},{y1:F1} " +
                $"size={iu1 - iu0 + 1}x{iv1 - iv0 + 1} " +
                $"uv={iu0},{iv0}-{iu1},{iv1} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"rect={rect.X:F0},{rect.Y:F0},{rect.W:F0},{rect.H:F0} " +
                $"allow-ui={(allowUiReplacement ? 1 : 0)} material={f.Material} " +
                resolution;
            if (_traceLoadingUiTextureResolves.Add(packet))
                Console.Error.WriteLine($"[V82LoadingUiResolve] {packet}");
        }
        if (TraceTextureRegions && f.Material == HleMaterialKind.TerrainRoute)
        {
            float x0 = MathF.Min(a.X, MathF.Min(b.X, c.X));
            float y0 = MathF.Min(a.Y, MathF.Min(b.Y, c.Y));
            float x1 = MathF.Max(a.X, MathF.Max(b.X, c.X));
            float y1 = MathF.Max(a.Y, MathF.Max(b.Y, c.Y));
            Console.WriteLine(
                $"[TextureRegion] frame={_frame} key={textureKey:x16} " +
                $"screen={x0:F1},{y0:F1}-{x1:F1},{y1:F1} " +
                $"uv={minU:F1},{minV:F1}-{maxU:F1},{maxV:F1} " +
                $"hit={(rect.Valid ? 1 : 0)} {resolution}");
        }
        if (!rect.Valid) return false;
        ApplyReplacementRect(ref a, ref b, ref c, rect);
        return true;
    }

    static void ApplyReplacementRect(
        ref GlVertex a, ref GlVertex b, ref GlVertex c,
        TextureReplacementAtlas.Rect rect)
    {
        a.ReplacementX = b.ReplacementX = c.ReplacementX = rect.X;
        a.ReplacementY = b.ReplacementY = c.ReplacementY = rect.Y;
        a.ReplacementW = b.ReplacementW = c.ReplacementW = rect.W;
        a.ReplacementH = b.ReplacementH = c.ReplacementH = rect.H;
        a.ReplacementScaleR = b.ReplacementScaleR = c.ReplacementScaleR = rect.ColorScale.X;
        a.ReplacementScaleG = b.ReplacementScaleG = c.ReplacementScaleG = rect.ColorScale.Y;
        a.ReplacementScaleB = b.ReplacementScaleB = c.ReplacementScaleB = rect.ColorScale.Z;
        a.ReplacementBiasR = b.ReplacementBiasR = c.ReplacementBiasR = rect.ColorBias.X;
        a.ReplacementBiasG = b.ReplacementBiasG = c.ReplacementBiasG = rect.ColorBias.Y;
        a.ReplacementBiasB = b.ReplacementBiasB = c.ReplacementBiasB = rect.ColorBias.Z;
    }

    bool ShouldTraceNearClip(in PrimFlags flags)
    {
        if (!TraceNearClipping)
            return false;
        if (flags.Vehicle)
            return _traceNearClipVehicleTriangles++ < 8192;
        if (flags.Material == HleMaterialKind.TerrainRoute)
            return _traceNearClipTerrainTriangles++ < 1024;
        return _traceNearClipObjectTriangles++ < 8192;
    }

    // The retail modal's border is drawn as flat 2D polygons, not rectangles,
    // so it never reaches the rectangle path. Its vertices come straight from
    // the packet and never touch the GTE, which separates them cleanly from
    // the world still being drawn behind the pause: of ~3900 triangles in a
    // paused frame only ~38 carry neither view space nor a GTE Z.
    public void DrawTri(in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f)
    {
        GpuHle.CoarseTerrainPacket coarseTerrain = default;
        bool hasCoarseTerrain =
            !f.Textured &&
            GpuHle.TryGetCoarseTerrainPacket(
                f.PacketAddress, out coarseTerrain);
        GpuHle.TerrainTransitionPacket terrainTransition = default;
        bool hasTerrainTransition =
            !f.Textured &&
            GpuHle.TryGetTerrainTransitionPacket(
                f.PacketAddress, out terrainTransition);
        if (TraceEnhancedRenderer &&
            f.Material == HleMaterialKind.TerrainRoute)
        {
            if (f.Textured)
                _traceTerrainRouteSourceTextured++;
            else if (hasCoarseTerrain)
                _traceTerrainRouteSourceCoarse++;
            else if (hasTerrainTransition)
                _traceTerrainRouteSourceTransition++;
            else
                _traceTerrainRouteSourceUnresolved++;
        }
        if (TraceTerrainDetail &&
            f.Material == HleMaterialKind.TerrainRoute)
        {
            string source = f.Textured
                ? "detail"
                : hasCoarseTerrain
                    ? "coarse"
                    : hasTerrainTransition ? "transition" : "unresolved";
            string cell = hasCoarseTerrain
                ? $"{coarseTerrain.X},{coarseTerrain.Z}"
                : hasTerrainTransition
                    ? $"{terrainTransition.X},{terrainTransition.Z}"
                    : "unknown";
            string sourceLabel = hasCoarseTerrain
                ? coarseTerrain.Source
                : hasTerrainTransition
                    ? terrainTransition.Source
                    : GpuHle.DescribePacketOwner(f.PacketAddress);
            int sourceCount = _traceTerrainDetailSourceCounts.GetValueOrDefault(
                source);
            _traceTerrainDetailSourceCounts[source] = sourceCount + 1;
            _traceTerrainDetailPackets++;
            // Preserve a deep sample for every path without allowing the
            // common detail path to drown the diagnostically rarer coarse and
            // transition rows in hundreds of megabytes of duplicate lines.
            if (sourceCount < 8192)
                Console.Error.WriteLine(
                    "[V82TerrainPath] " +
                    $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                    $"packet=0x{f.PacketAddress:X8} source={source} " +
                    $"source-sample={sourceCount + 1}/8192 " +
                    $"source-label=\"{sourceLabel}\" cell={cell} " +
                    $"rgb={a.R},{a.G},{a.B};{b.R},{b.G},{b.B};{c.R},{c.G},{c.B} " +
                    $"view-z={a.ViewZ:F3},{b.ViewZ:F3},{c.ViewZ:F3} " +
                    $"gte-z={a.Z:F3},{b.Z:F3},{c.Z:F3} " +
                    $"textured={(f.Textured ? 1 : 0)} raw={(f.RawTexture ? 1 : 0)} " +
                    $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4}");
        }
        if (hasCoarseTerrain &&
            f.Material == HleMaterialKind.TerrainRoute)
        {
            DrawTexturedCoarseTerrainHalf(a, b, c, f, coarseTerrain);
            return;
        }
        if (hasTerrainTransition &&
            f.Material == HleMaterialKind.TerrainRoute)
        {
            DrawTexturedTerrainTransition(
                a, b, c, f, terrainTransition);
            return;
        }

        HleVertex authoredA = a, authoredB = b, authoredC = c;
        float authoredMinU = MathF.Min(a.U, MathF.Min(b.U, c.U));
        float authoredMinV = MathF.Min(a.V, MathF.Min(b.V, c.V));
        float authoredMaxU = MathF.Max(a.U, MathF.Max(b.U, c.U));
        float authoredMaxV = MathF.Max(a.V, MathF.Max(b.V, c.V));
        authoredA.HasAuthoredUvBounds =
            authoredB.HasAuthoredUvBounds =
            authoredC.HasAuthoredUvBounds = true;
        authoredA.AuthoredMinU = authoredB.AuthoredMinU =
            authoredC.AuthoredMinU = authoredMinU;
        authoredA.AuthoredMinV = authoredB.AuthoredMinV =
            authoredC.AuthoredMinV = authoredMinV;
        authoredA.AuthoredMaxU = authoredB.AuthoredMaxU =
            authoredC.AuthoredMaxU = authoredMaxU;
        authoredA.AuthoredMaxV = authoredB.AuthoredMaxV =
            authoredC.AuthoredMaxV = authoredMaxV;
        if (GpuHle.NativeModalActive && _kTarget is { Margin: > 0 } modalTri &&
            !a.HasViewSpace && !b.HasViewSpace && !c.HasViewSpace &&
            !a.HasGteZ && !b.HasGteZ && !c.HasGteZ)
        {
            HleVertex sa = authoredA, sb = authoredB, sc = authoredC;
            sa.X -= modalTri.Margin;
            sb.X -= modalTri.Margin;
            sc.X -= modalTri.Margin;
            DrawTriCore(sa, sb, sc, f);
            return;
        }
        DrawTriCore(authoredA, authoredB, authoredC, f);
    }

    void DrawTexturedCoarseTerrainHalf(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        in PrimFlags flags,
        in GpuHle.CoarseTerrainPacket coarse)
    {
        GpuHle.TerrainCellTextures textures = coarse.Textures;
        int size = textures.GridSize;
        if (TraceTerrainShade && _traceTerrainShadePackets++ < 4096)
        {
            int minimumR = 255, minimumG = 255, minimumB = 255;
            int maximumR = 0, maximumG = 0, maximumB = 0;
            var textureIds = new System.Text.StringBuilder();
            for (int sampleX = 0; sampleX < size; sampleX++)
            {
                for (int sampleZ = 0; sampleZ < size; sampleZ++)
                {
                    GpuHle.TerrainTextureDescriptor sample =
                        textures.Get(sampleX, sampleZ);
                    minimumR = Math.Min(minimumR, sample.AverageR);
                    minimumG = Math.Min(minimumG, sample.AverageG);
                    minimumB = Math.Min(minimumB, sample.AverageB);
                    maximumR = Math.Max(maximumR, sample.AverageR);
                    maximumG = Math.Max(maximumG, sample.AverageG);
                    maximumB = Math.Max(maximumB, sample.AverageB);
                    if (textureIds.Length != 0)
                        textureIds.Append(',');
                    textureIds.Append(sample.TextureId);
                }
            }
            Console.Error.WriteLine(
                "[V82TerrainShade] " +
                $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                $"cell={coarse.X},{coarse.Z} source={coarse.Source} " +
                $"half={(coarse.SecondHalf ? 1 : 0)} grid={size} " +
                $"packet-rgb={a.R},{a.G},{a.B};{b.R},{b.G},{b.B};" +
                $"{c.R},{c.G},{c.B} " +
                $"packet-xy={a.X:F2},{a.Y:F2};{b.X:F2},{b.Y:F2};" +
                $"{c.X:F2},{c.Y:F2} " +
                $"view-z={a.ViewZ:F2},{b.ViewZ:F2},{c.ViewZ:F2} " +
                $"xtin-average={minimumR},{minimumG},{minimumB}.." +
                $"{maximumR},{maximumG},{maximumB} " +
                $"textures={textureIds}");
        }
        for (int x = 0; x < size; x++)
        {
            for (int z = 0; z < size; z++)
            {
                GpuHle.TerrainTextureDescriptor texture =
                    textures.Get(x, z);
                if ((texture.Flags & 1) != 0)
                    continue;

                if (!coarse.SecondHalf)
                {
                    int diagonal = x + z;
                    if (diagonal == size - 1)
                        DrawTerrainTextureTriangle(
                            FirstTerrainGridPoint(a, b, c, size, x, z),
                            texture.Uv00,
                            FirstTerrainGridPoint(a, b, c, size, x + 1, z),
                            texture.Uv01,
                            FirstTerrainGridPoint(a, b, c, size, x, z + 1),
                            texture.Uv10,
                            flags, texture);
                    else if (diagonal < size - 1)
                        DrawTerrainTextureSquare(
                            FirstTerrainGridPoint(a, b, c, size, x, z),
                            FirstTerrainGridPoint(a, b, c, size, x + 1, z),
                            FirstTerrainGridPoint(a, b, c, size, x, z + 1),
                            FirstTerrainGridPoint(
                                a, b, c, size, x + 1, z + 1),
                            flags, texture);
                    continue;
                }

                int secondDiagonal = x + z;
                if (secondDiagonal == size - 1)
                    DrawTerrainTextureTriangle(
                        SecondTerrainGridPoint(
                            a, b, c, size, x + 1, z + 1),
                        texture.Uv11,
                        SecondTerrainGridPoint(a, b, c, size, x + 1, z),
                        texture.Uv01,
                        SecondTerrainGridPoint(a, b, c, size, x, z + 1),
                        texture.Uv10,
                        flags, texture);
                else if (secondDiagonal >= size)
                    DrawTerrainTextureSquare(
                        SecondTerrainGridPoint(a, b, c, size, x, z),
                        SecondTerrainGridPoint(a, b, c, size, x + 1, z),
                        SecondTerrainGridPoint(a, b, c, size, x, z + 1),
                        SecondTerrainGridPoint(
                            a, b, c, size, x + 1, z + 1),
                        flags, texture);
            }
        }
    }

    void DrawTerrainTextureSquare(
        in HleVertex topLeft,
        in HleVertex topRight,
        in HleVertex bottomLeft,
        in HleVertex bottomRight,
        in PrimFlags flags,
        in GpuHle.TerrainTextureDescriptor texture)
    {
        if ((texture.Flags & 2) != 0)
        {
            DrawTerrainTextureTriangle(
                topLeft, texture.Uv00,
                topRight, texture.Uv01,
                bottomRight, texture.Uv11,
                flags, texture);
            DrawTerrainTextureTriangle(
                topLeft, texture.Uv00,
                bottomLeft, texture.Uv10,
                bottomRight, texture.Uv11,
                flags, texture);
            return;
        }

        DrawTerrainTextureTriangle(
            topLeft, texture.Uv00,
            topRight, texture.Uv01,
            bottomLeft, texture.Uv10,
            flags, texture);
        DrawTerrainTextureTriangle(
            bottomRight, texture.Uv11,
            topRight, texture.Uv01,
            bottomLeft, texture.Uv10,
            flags, texture);
    }

    readonly record struct TerrainClipVertex(
        HleVertex Vertex,
        float X,
        float Z);

    void DrawTexturedTerrainTransition(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        in PrimFlags flags,
        in GpuHle.TerrainTransitionPacket transition)
    {
        GpuHle.TerrainCellTextures textures = transition.Textures;
        List<TerrainClipVertex> source =
        [
            new TerrainClipVertex(a, transition.Ax, transition.Az),
            new TerrainClipVertex(b, transition.Bx, transition.Bz),
            new TerrainClipVertex(c, transition.Cx, transition.Cz),
        ];
        for (int x = 0; x < textures.GridSize; x++)
        {
            for (int z = 0; z < textures.GridSize; z++)
            {
                GpuHle.TerrainTextureDescriptor texture =
                    textures.Get(x, z);
                if ((texture.Flags & 1) != 0)
                    continue;

                List<TerrainClipVertex> clipped =
                    ClipTerrainTransition(source, xAxis: true, x, keepHigh: true);
                clipped = ClipTerrainTransition(
                    clipped, xAxis: true, x + 1, keepHigh: false);
                clipped = ClipTerrainTransition(
                    clipped, xAxis: false, z, keepHigh: true);
                clipped = ClipTerrainTransition(
                    clipped, xAxis: false, z + 1, keepHigh: false);
                if (clipped.Count < 3)
                    continue;

                for (int index = 1; index + 1 < clipped.Count; index++)
                    DrawTerrainTextureTriangle(
                        clipped[0],
                        clipped[index],
                        clipped[index + 1],
                        x, z,
                        flags,
                        texture);
            }
        }
    }

    static List<TerrainClipVertex> ClipTerrainTransition(
        List<TerrainClipVertex> source,
        bool xAxis,
        float boundary,
        bool keepHigh)
    {
        if (source.Count == 0)
            return source;

        var output = new List<TerrainClipVertex>(source.Count + 1);
        TerrainClipVertex previous = source[^1];
        float previousCoordinate = xAxis ? previous.X : previous.Z;
        bool previousInside = keepHigh
            ? previousCoordinate >= boundary
            : previousCoordinate <= boundary;
        foreach (TerrainClipVertex current in source)
        {
            float currentCoordinate = xAxis ? current.X : current.Z;
            bool currentInside = keepHigh
                ? currentCoordinate >= boundary
                : currentCoordinate <= boundary;
            if (currentInside != previousInside)
            {
                float span = currentCoordinate - previousCoordinate;
                float amount = MathF.Abs(span) < 0.0001f
                    ? 0f
                    : (boundary - previousCoordinate) / span;
                output.Add(new TerrainClipVertex(
                    LerpVertex(
                        previous.Vertex,
                        current.Vertex,
                        Math.Clamp(amount, 0f, 1f)),
                    previous.X + (current.X - previous.X) * amount,
                    previous.Z + (current.Z - previous.Z) * amount));
            }
            if (currentInside)
                output.Add(current);
            previous = current;
            previousCoordinate = currentCoordinate;
            previousInside = currentInside;
        }
        return output;
    }

    void DrawTerrainTextureTriangle(
        in TerrainClipVertex a,
        in TerrainClipVertex b,
        in TerrainClipVertex c,
        int tileX,
        int tileZ,
        in PrimFlags flags,
        in GpuHle.TerrainTextureDescriptor texture)
    {
        HleVertex texturedA = WithTerrainUv(
            PreserveCoarseTerrainDetail(a.Vertex, texture),
            texture, a.X - tileX, a.Z - tileZ);
        HleVertex texturedB = WithTerrainUv(
            PreserveCoarseTerrainDetail(b.Vertex, texture),
            texture, b.X - tileX, b.Z - tileZ);
        HleVertex texturedC = WithTerrainUv(
            PreserveCoarseTerrainDetail(c.Vertex, texture),
            texture, c.X - tileX, c.Z - tileZ);
        PrimFlags texturedFlags = flags;
        texturedFlags.Textured = true;
        texturedFlags.RawTexture = false;
        texturedFlags.Gouraud = true;
        texturedFlags.TPage = texture.TPage;
        texturedFlags.Clut = texture.Clut;
        DrawTri(texturedA, texturedB, texturedC, texturedFlags);
    }

    static HleVertex FirstTerrainGridPoint(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        int size,
        int x,
        int z)
    {
        float row = z / (float)size;
        HleVertex left = LerpVertex(a, c, row);
        HleVertex right = LerpVertex(b, c, row);
        int rowWidth = size - z;
        return rowWidth == 0
            ? left
            : LerpVertex(left, right, x / (float)rowWidth);
    }

    static HleVertex SecondTerrainGridPoint(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        int size,
        int x,
        int z)
    {
        float row = z / (float)size;
        HleVertex left = LerpVertex(a, b, row);
        HleVertex right = LerpVertex(a, c, row);
        int rowWidth = z;
        return rowWidth == 0
            ? left
            : LerpVertex(
                left, right, (x - (size - z)) / (float)rowWidth);
    }

    void DrawTerrainTextureTriangle(
        in HleVertex a,
        ushort uvA,
        in HleVertex b,
        ushort uvB,
        in HleVertex c,
        ushort uvC,
        in PrimFlags flags,
        in GpuHle.TerrainTextureDescriptor texture)
    {
        HleVertex texturedA = WithTerrainUv(
            PreserveCoarseTerrainDetail(a, texture), uvA);
        HleVertex texturedB = WithTerrainUv(
            PreserveCoarseTerrainDetail(b, texture), uvB);
        HleVertex texturedC = WithTerrainUv(
            PreserveCoarseTerrainDetail(c, texture), uvC);
        PrimFlags texturedFlags = flags;
        texturedFlags.Textured = true;
        texturedFlags.RawTexture = false;
        texturedFlags.Gouraud = true;
        texturedFlags.TPage = texture.TPage;
        texturedFlags.Clut = texture.Clut;
        DrawTri(texturedA, texturedB, texturedC, texturedFlags);
    }

    static HleVertex PreserveCoarseTerrainDetail(
        in HleVertex source,
        in GpuHle.TerrainTextureDescriptor texture)
    {
        // Native coarse terrain stores the intended final surface colour in
        // an untextured Gouraud packet. Reusing that byte directly as textured
        // modulation is wrong: 128 is neutral in the PS1 texture equation, so
        // the old clamp forced every bright distant tile to the texture's raw
        // average and produced a hard white LOD ring. Convert the authored
        // final colour to the modulation needed for this tile's measured
        // average instead. This retains the original distance lighting while
        // restoring local texture detail, and remains continuous when the
        // native emitter changes between detail, transition and coarse paths.
        HleVertex result = source;
        result.R = TerrainTargetToModulation(source.R, texture.AverageR);
        result.G = TerrainTargetToModulation(source.G, texture.AverageG);
        result.B = TerrainTargetToModulation(source.B, texture.AverageB);
        return result;
    }

    static byte TerrainTargetToModulation(byte target, byte textureAverage)
    {
        if (textureAverage == 0)
            return Math.Min(target, (byte)128);
        int modulation = (target * 128 + textureAverage / 2) / textureAverage;
        return (byte)Math.Clamp(modulation, 0, 255);
    }

    static HleVertex WithTerrainUv(in HleVertex source, ushort packedUv)
    {
        HleVertex result = source;
        result.U = (byte)packedUv;
        result.V = (byte)(packedUv >> 8);
        result.HasAuthoredUvBounds = false;
        return result;
    }

    static HleVertex WithTerrainUv(
        in HleVertex source,
        in GpuHle.TerrainTextureDescriptor texture,
        float x,
        float z)
    {
        x = Math.Clamp(x, 0f, 1f);
        z = Math.Clamp(z, 0f, 1f);
        float topU = (byte)texture.Uv00 +
            ((byte)texture.Uv01 - (byte)texture.Uv00) * x;
        float bottomU = (byte)texture.Uv10 +
            ((byte)texture.Uv11 - (byte)texture.Uv10) * x;
        float topV = (byte)(texture.Uv00 >> 8) +
            ((byte)(texture.Uv01 >> 8) - (byte)(texture.Uv00 >> 8)) * x;
        float bottomV = (byte)(texture.Uv10 >> 8) +
            ((byte)(texture.Uv11 >> 8) - (byte)(texture.Uv10 >> 8)) * x;
        HleVertex result = source;
        result.U = (short)Math.Clamp(
            (int)MathF.Round(topU + (bottomU - topU) * z), 0, 255);
        result.V = (short)Math.Clamp(
            (int)MathF.Round(topV + (bottomV - topV) * z), 0, 255);
        result.HasAuthoredUvBounds = false;
        return result;
    }

    void DrawTriCore(in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f)
    {
        if (CullByPacketNclip(a, b, c, f)) return;
        if (ClipAgainstNearPlane(a, b, c, f)) return;
        if (TraceModalRects && GpuHle.NativeModalActive && _modalTriLines++ < 300)
        {
            float lo = Math.Min(a.X, Math.Min(b.X, c.X));
            float hi = Math.Max(a.X, Math.Max(b.X, c.X));
            float ty = Math.Min(a.Y, Math.Min(b.Y, c.Y));
            Console.Error.WriteLine(
                $"[V82ModalTri] x={lo:F0}..{hi:F0} y={ty:F0} " +
                $"tex={(f.Textured ? 1 : 0)} semi={(f.SemiTrans ? 1 : 0)} " +
                $"tpage=0x{f.TPage:X3} ot={f.OtIndex} " +
                $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\"");
        }
        float spanX =
            Math.Max(a.X, Math.Max(b.X, c.X)) -
            Math.Min(a.X, Math.Min(b.X, c.X));
        float spanY =
            Math.Max(a.Y, Math.Max(b.Y, c.Y)) -
            Math.Min(a.Y, Math.Min(b.Y, c.Y));
        string? tracedTerrainOwner = null;
        if (TraceTerrainCells.Count != 0 &&
            (TraceTerrainCellTicks is not { } terrainTicks ||
             (GpuHle.DebugGameplayTick >= terrainTicks.Start &&
              GpuHle.DebugGameplayTick <= terrainTicks.End)))
        {
            string owner = GpuHle.DescribePacketOwner(f.PacketAddress);
            if (TraceTerrainCells.Any(prefix =>
                    owner.StartsWith(prefix, StringComparison.Ordinal)))
                tracedTerrainOwner = owner;
        }
        if (tracedTerrainOwner is not null &&
            _traceTerrainCellTriangles++ < 20000)
        {
            Console.Error.WriteLine(
                "[TerrainCellTriangle] " +
                $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                $"owner=\"{tracedTerrainOwner}\" material={f.Material} " +
                $"xy=({a.X},{a.Y})({b.X},{b.Y})({c.X},{c.Y}) " +
                $"span={spanX},{spanY} " +
                $"overspan={((spanX > 1023 || spanY > 511) ? 1 : 0)} " +
                $"z=({a.Z},{b.Z},{c.Z}) " +
                $"gte=({(a.HasGteZ ? 1 : 0)},{(b.HasGteZ ? 1 : 0)}," +
                $"{(c.HasGteZ ? 1 : 0)})");
        }
        // Match RasterTriangle and the native PS1 polygon limits. Vertices
        // projected across/behind the camera can wrap into enormous screen
        // spans; the hardware drops those primitives instead of clipping them
        // into the long terrain-like wedges OpenGL would otherwise draw.
        bool modernGeometry =
            a.HasViewSpace && b.HasViewSpace && c.HasViewSpace;
        // The PS1 rasterizer rejects polygons whose packet-space coordinate
        // span exceeds its hardware limits. Preserve that rule for fallback
        // packets, where the saturated 2D coordinates are all we have. Exact
        // camera-space geometry has already passed ClipAgainstNearPlane and
        // the shader projects those view-space vertices instead of these
        // packet coordinates; dropping it here cuts valid terrain/object
        // triangles out at the screen edge and exposes the water plane below.
        // The earlier long wedges came from malformed partial morph records,
        // which the N64 converter now expands from the group's static tail.
        if (spanX > 1023 || spanY > 511)
        {
            if (!modernGeometry)
            {
                _traceNativeOverspanRejectedTriangles++;
                return;
            }
            _traceModernOverspanTriangles++;
        }
        TraceSelectorTriangle(a, b, c, f);
        if (TraceVehicleMaterials &&
            f.Vehicle &&
            (TraceVehicleMaterialTicks is not { } materialTicks ||
             (GpuHle.DebugGameplayTick >= materialTicks.Start &&
              GpuHle.DebugGameplayTick <= materialTicks.End)) &&
            _traceVehicleMaterialTriangles++ < 200000)
        {
            string ps1ZeroKey = f.Material == HleMaterialKind.VehicleReflection
                ? "discard"
                : "standard";
            Console.Error.WriteLine(
                "[VehicleMaterialTriangle] " +
                $"gameplay={(GpuHle.GameplayActive ? 1 : 0)} " +
                $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                $"material={f.Material} tex={(f.Textured ? 1 : 0)} " +
                $"semi={(f.SemiTrans ? 1 : 0)} " +
                $"raw={(f.RawTexture ? 1 : 0)} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"tw={_kTwAndX:X2},{_kTwAndY:X2}," +
                    $"{_kTwOrX:X2},{_kTwOrY:X2} " +
                $"ps1-zero-key={ps1ZeroKey} " +
                $"xy=({a.X},{a.Y})({b.X},{b.Y})({c.X},{c.Y}) " +
                $"uv=({a.U},{a.V})({b.U},{b.V})({c.U},{c.V}) " +
                $"rgb=({a.R},{a.G},{a.B})({b.R},{b.G},{b.B})" +
                $"({c.R},{c.G},{c.B}) " +
                $"view-z=({a.ViewZ:F3},{b.ViewZ:F3},{c.ViewZ:F3}) " +
                $"view=({a.ViewX:F3},{a.ViewY:F3});" +
                    $"({b.ViewX:F3},{b.ViewY:F3});" +
                    $"({c.ViewX:F3},{c.ViewY:F3}) " +
                $"view-space=({(a.HasViewSpace ? 1 : 0)}," +
                    $"{(b.HasViewSpace ? 1 : 0)}," +
                    $"{(c.HasViewSpace ? 1 : 0)}) " +
                $"gte=({(a.HasGteZ ? 1 : 0)},{(b.HasGteZ ? 1 : 0)}," +
                $"{(c.HasGteZ ? 1 : 0)})");
        }
        if (DisableVehicleTriangles && f.Vehicle)
            return;
        if (f.Vehicle &&
            DisabledVehicleMaterials.Contains(f.Material.ToString()))
            return;
        string triangleSignature =
            $"{f.Material}:{f.TPage & 0x1FF:X3}:{f.Clut & 0x7FFF:X4}";
        if (DisabledTriangleSignatures.Contains(triangleSignature))
        {
            if (TriangleProbe is not null && GpuHle.GameplayActive)
                Console.Error.WriteLine(
                    "[V8TriangleSignatureSuppressed] " +
                    $"frame={_frame} packet=0x{f.PacketAddress:X8} " +
                    $"signature={triangleSignature} " +
                    $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\"");
            return;
        }

        // Use per-vertex GTE depth only when all three packet vertices were
        // correlated to one native projection group. Independent XY lookups
        // can select unrelated overlapping surfaces; that was the source of
        // both object flicker and translucent world geometry leaking through
        // opaque land.
        bool coherentRasterDepth =
            modernGeometry ||
            a.HasCoherentGteZ &&
            b.HasCoherentGteZ &&
            c.HasCoherentGteZ;
        bool depthEligible =
            !DisableRasterDepth &&
            ConfigManager.View.EnhancedDepthBuffer &&
            GpuHle.GameplayActive &&
            f.Material is not
                (HleMaterialKind.Ui or HleMaterialKind.ScreenEffect) &&
            (ConfigManager.View.HighResolution3D ||
             ConfigManager.View.PerspectiveCorrectTextures) &&
            (modernGeometry || f.OtIndex > 0);
        // Preserve the PS1 ordering table for opaque world geometry. Source
        // route surfaces that explicitly carry an RDP compare/update contract
        // populate the shared depth image; semitransparent world primitives
        // test it without writing it.
        bool depthWrite =
            depthEligible &&
            (!f.SemiTrans || IsOpaqueVehicleGlassMaterial(f));
        bool sourceOpaqueDepthTest =
            f.N64RouteDepthCompare &&
            !f.SemiTrans &&
            coherentRasterDepth;
        bool depthTest =
            depthEligible &&
            (sourceOpaqueDepthTest || f.SemiTrans);
        bool sourceDepthCompareWrite =
            sourceOpaqueDepthTest && depthTest && depthWrite;
        GlDisplayRt? backdropTarget =
            _kTarget is { Margin: > 0 } candidateBackdrop
                ? candidateBackdrop
                : null;
        GlDisplayRt? fullWidthTarget = _kTarget;
        bool fullWidthGameplayFill =
            GpuHle.GameplayActive &&
            fullWidthTarget != null &&
            f.Material == HleMaterialKind.Ui &&
            !f.Textured &&
            !f.SemiTrans &&
            Math.Min(a.X, Math.Min(b.X, c.X)) <= fullWidthTarget.X &&
            Math.Max(a.X, Math.Max(b.X, c.X)) >=
                fullWidthTarget.X + fullWidthTarget.W;
        CheckTextureFeedback(
            f,
            Math.Min(a.U, Math.Min(b.U, c.U)),
            Math.Min(a.V, Math.Min(b.V, c.V)),
            Math.Max(a.U, Math.Max(b.U, c.U)),
            Math.Max(a.V, Math.Max(b.V, c.V)));
        Begin(
            f,
            3,
            depthTest,
            depthWrite,
            sourceDepthCompareWrite);
        bool dith = DitherOf(f);
        bool hasDepth = f.Textured && a.HasGteZ && b.HasGteZ && c.HasGteZ;
        bool hasProjectiveW =
            f.Textured &&
            (modernGeometry ||
             a.HasProjectiveW && b.HasProjectiveW && c.HasProjectiveW);
        bool perspectiveCorrect =
            !DisableProjectiveTextures &&
            f.Material is not
                (HleMaterialKind.Ui or HleMaterialKind.ScreenEffect) &&
            hasProjectiveW;
        if (TraceConvertedSurfaces && GpuHle.GameplayActive &&
            !f.Vehicle && f.Textured && f.SemiTrans &&
            f.Material == HleMaterialKind.Additive &&
            a.R > 0 && a.R <= 48 && a.G > 0 && a.G <= 48 &&
            a.B > 0 && a.B <= 48 &&
            b.R > 0 && b.R <= 48 && b.G > 0 && b.G <= 48 &&
            b.B > 0 && b.B <= 48 &&
            c.R > 0 && c.R <= 48 && c.G > 0 && c.G <= 48 &&
            c.B > 0 && c.B <= 48)
        {
            float minX = Math.Min(a.X, Math.Min(b.X, c.X));
            float maxX = Math.Max(a.X, Math.Max(b.X, c.X));
            float minY = Math.Min(a.Y, Math.Min(b.Y, c.Y));
            float maxY = Math.Max(a.Y, Math.Max(b.Y, c.Y));
            bool visible =
                maxX >= _env.ClipX0 && minX <= _env.ClipX1 &&
                maxY >= _env.ClipY0 && minY <= _env.ClipY1;
            double area = Math.Abs(
                (b.X - a.X) * (c.Y - a.Y) -
                (b.Y - a.Y) * (c.X - a.X)) * 0.5;
            _traceConvertedSurfaceTriangles++;
            if (visible && area > 0.0)
                _traceVisibleConvertedSurfaceTriangles++;
            if (depthTest)
                _traceDepthTestedConvertedSurfaceTriangles++;
            if (perspectiveCorrect)
                _traceProjectiveConvertedSurfaceTriangles++;
            if (modernGeometry)
                _traceModernConvertedSurfaceTriangles++;
            _traceConvertedSurfaceArea += area;
            _traceConvertedSurfaceMaxArea = Math.Max(
                _traceConvertedSurfaceMaxArea, area);
        }
        if (TraceEnhancedRenderer && GpuHle.GameplayActive &&
            f.Material != HleMaterialKind.Ui)
        {
            _traceEnhancedTriangles++;
            bool worldMaterial = f.Material is
                HleMaterialKind.Opaque or
                HleMaterialKind.AlphaTest or
                HleMaterialKind.Glass or
                HleMaterialKind.VehicleReflection or
                HleMaterialKind.OpaqueVehicleGlass or
                HleMaterialKind.TerrainRoute;
            float minX = Math.Min(a.X, Math.Min(b.X, c.X));
            float maxX = Math.Max(a.X, Math.Max(b.X, c.X));
            float minY = Math.Min(a.Y, Math.Min(b.Y, c.Y));
            float maxY = Math.Max(a.Y, Math.Max(b.Y, c.Y));
            bool visibleWorld =
                worldMaterial &&
                maxX >= _env.ClipX0 && minX <= _env.ClipX1 &&
                maxY >= _env.ClipY0 && minY <= _env.ClipY1;
            if (worldMaterial)
                _traceWorldTriangles++;
            if (visibleWorld)
                _traceVisibleWorldTriangles++;
            if (f.Vehicle)
            {
                _traceVehicleTriangles++;
                if (visibleWorld)
                    _traceVisibleVehicleTriangles++;
            }
            if (modernGeometry)
            {
                bool reconstructed =
                    a.ReconstructedViewSpace ||
                    b.ReconstructedViewSpace ||
                    c.ReconstructedViewSpace;
                if (reconstructed)
                    _traceReconstructedViewSpaceTriangles++;
                else
                    _traceDirectViewSpaceTriangles++;
            }
            else
            {
                _traceFallbackTriangles++;
                if (worldMaterial)
                    _traceWorldFallbackTriangles++;
                else
                    _traceEffectFallbackTriangles++;
                if (visibleWorld)
                    _traceVisibleWorldFallbackTriangles++;
                if (f.Vehicle)
                {
                    _traceVehicleFallbackTriangles++;
                    if (visibleWorld)
                        _traceVisibleVehicleFallbackTriangles++;
                }
                switch (f.Material)
                {
                    case HleMaterialKind.Opaque:
                        _traceOpaqueFallbackTriangles++;
                        break;
                    case HleMaterialKind.AlphaTest:
                        _traceAlphaTestFallbackTriangles++;
                        break;
                    case HleMaterialKind.Glass:
                        _traceGlassFallbackTriangles++;
                        break;
                    case HleMaterialKind.VehicleReflection:
                    case HleMaterialKind.OpaqueVehicleGlass:
                        _traceOpaqueFallbackTriangles++;
                        break;
                }
            }
            if (f.Material == HleMaterialKind.Glass)
                _traceGlassTriangles++;
            if (f.Material == HleMaterialKind.TerrainRoute)
            {
                _traceTerrainRouteTriangles++;
                if (f.SemiTrans)
                    _traceTerrainRouteTransparentTriangles++;
                else
                    _traceTerrainRouteOpaqueTriangles++;
                if (depthWrite)
                    _traceTerrainRouteDepthWriteTriangles++;
                if (depthTest)
                    _traceTerrainRouteDepthTestTriangles++;
                if (sourceDepthCompareWrite)
                    _traceTerrainRouteDepthCompareWriteTriangles++;
            }
        }
        if (TraceEnhancedFallbacks && GpuHle.GameplayActive &&
            !modernGeometry && _traceFallbackShapes.Count < 512)
        {
            string shape =
                $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                $"material={f.Material} vehicle={(f.Vehicle ? 1 : 0)} " +
                $"tex={(f.Textured ? 1 : 0)} semi={(f.SemiTrans ? 1 : 0)} " +
                $"xy=({a.X:F3},{a.Y:F3})({b.X:F3},{b.Y:F3})" +
                $"({c.X:F3},{c.Y:F3}) " +
                $"source=(0x{a.SourceAddress:X8},0x{b.SourceAddress:X8}," +
                $"0x{c.SourceAddress:X8}) " +
                $"z=({a.Z:F0},{b.Z:F0},{c.Z:F0}) " +
                $"gte=({(a.HasGteZ ? 1 : 0)}," +
                $"{(b.HasGteZ ? 1 : 0)},{(c.HasGteZ ? 1 : 0)}) " +
                $"projective=({(a.HasProjectiveW ? 1 : 0)}," +
                $"{(b.HasProjectiveW ? 1 : 0)}," +
                $"{(c.HasProjectiveW ? 1 : 0)})";
            if (_traceFallbackShapes.Add(shape))
                Console.Error.WriteLine($"[EnhancedFallback] {shape}");
        }
        if (TraceDepth && GpuHle.GameplayActive)
        {
            if (f.SemiTrans) _traceTransparentTriangles++;
            else _traceOpaqueTriangles++;
            if (depthTest) _traceDepthTestedTriangles++;
            if (f.SemiTrans && !depthTest)
            {
                if (f.Material is HleMaterialKind.Ui or
                    HleMaterialKind.ScreenEffect)
                    _tracePainterOrderedScreenTriangles++;
                else
                    _tracePainterOrderedWorldTriangles++;
            }
            if (depthTest && f.SemiTrans)
                _traceTransparentWorldDepthTriangles++;
            if (perspectiveCorrect) _traceProjectiveTriangles++;
            if (f.OtIndex <= 0) _traceMissingOtTriangles++;
            else
            {
                _traceMinOt = Math.Min(_traceMinOt, f.OtIndex);
                _traceMaxOt = Math.Max(_traceMaxOt, f.OtIndex);
            }
        }
        bool particle = f.Material == HleMaterialKind.Particle;
        bool shadow = !f.Textured && f.SemiTrans && a.HasGteZ && b.HasGteZ && c.HasGteZ &&
            a.R < 96 && a.G < 96 && a.B < 96 && b.R < 96 && b.G < 96 && b.B < 96 &&
            c.R < 96 && c.G < 96 && c.B < 96;
        static float DepthOf(
            in HleVertex vertex, int otDepth, bool useCoherentDepth)
        {
            // The ordering table is the game's authoritative visibility
            // contract.  Recovered per-vertex GTE samples are excellent for
            // perspective-correct UVs and sub-pixel XY, but they are not
            // globally unique: independently projected terrain and vehicle
            // vertices can reuse the same screen coordinate.  Mixing those
            // samples in one hardware Z buffer caused whole vehicle sections
            // to fall behind the road.  Give every primitive its native OT
            // bucket depth instead.  This retains exact far-to-near game
            // ordering while still allowing the Enhanced renderer to use
            // modern projection and texture interpolation.
            //
            // That reasoning holds for depth recovered by screen-space
            // correlation. Where the renderer has the vertex's own exact
            // camera-space Z from the GTE seam, the bucket is strictly worse:
            // a wall long enough to span two buckets gets a single flat depth
            // per primitive, and the half whose bucket sits behind the terrain
            // is discarded - a straight vertical cut through a solid object.
            if (TrueVertexDepth && useCoherentDepth &&
                vertex.HasViewSpace &&
                !vertex.ReconstructedViewSpace &&
                vertex.ViewZ >= 1f)
                return Math.Clamp(vertex.ViewZ, 1f, 65535f) / 65535f;
            return Math.Clamp(otDepth, 1, 0x1FFF) / 8192f;
        }
        bool screenSpacePrimitive = f.Material is
            HleMaterialKind.Ui or HleMaterialKind.ScreenEffect;
        float triMinY = MathF.Min(a.Y, MathF.Min(b.Y, c.Y));
        bool topGameplayHudTriangle =
            GpuHle.GameplayActive &&
            screenSpacePrimitive &&
            _kTarget is { } triHudTarget &&
            triMinY - triHudTarget.Y < triHudTarget.H * 0.42f;
        HleVertex drawA = a;
        HleVertex drawB = b;
        HleVertex drawC = c;
        if (TerrainPacketProjection &&
            f.Material == HleMaterialKind.TerrainRoute &&
            backdropTarget != null)
        {
            // Diagnostic watertightness control. The native packet XY values
            // share exact integer endpoints across adjacent terrain cells.
            // Undo the deliberate widescreen SXY compression in screen space,
            // but keep each vertex's PerspectiveW for projective UVs. This
            // isolates traversal completeness from per-packet camera-space
            // correlation/reprojection, which can otherwise give one shared
            // terrain endpoint two slightly different sub-pixel positions.
            float centerX =
                backdropTarget.X + backdropTarget.W * 0.5f;
            float wideRatio =
                GpuHle.WideAspect / GpuHle.BaseAspect;
            StabilizeTerrainPacketVertex(
                ref drawA, centerX, wideRatio);
            StabilizeTerrainPacketVertex(
                ref drawB, centerX, wideRatio);
            StabilizeTerrainPacketVertex(
                ref drawC, centerX, wideRatio);
        }
        if (TerrainSeamGuardPixels > 0f &&
            f.Material == HleMaterialKind.TerrainRoute &&
            drawA.HasViewSpace && drawB.HasViewSpace && drawC.HasViewSpace)
        {
            // Terrain is rasterized from camera space, so coverage has to be
            // widened there. The previous guard edited the packet XY, which
            // the terrain path never reads, and only appeared to work because
            // it also forced the whole primitive onto the packet-space
            // projection - taking the compressed widescreen X with it and
            // reopening the outer edges.
            ApplyTerrainSeamGuardViewSpace(
                ref drawA, ref drawB, ref drawC, TerrainSeamGuardPixels);
        }
        if (fullWidthGameplayFill)
            RecordAtmosphereColor(a, b, c, f);
        bool fullWidthGameplayBackdrop =
            fullWidthGameplayFill &&
            backdropTarget != null;
        if (fullWidthGameplayBackdrop)
        {
            // Native V8 clears/fills the authored 320-pixel display with
            // full-width flat polygons.  The expanded render target owns real
            // pixels outside that rectangle, so leaving these endpoints at
            // x=0/320 retains stale colour in both widescreen margins.  Extend
            // only the exact full-display backdrop edges; HUD and ordinary
            // screen-space primitives keep their native authored positions.
            ExpandBackdropEdge(ref drawA, backdropTarget!);
            ExpandBackdropEdge(ref drawB, backdropTarget!);
            ExpandBackdropEdge(ref drawC, backdropTarget!);
        }
        var va = V(drawA, f, dith, perspectiveCorrect, screenSpacePrimitive,
            DepthOf(a, f.OtIndex, coherentRasterDepth)); va.BaryX = 1f;
        var vb = V(drawB, f, dith, perspectiveCorrect, screenSpacePrimitive,
            DepthOf(b, f.OtIndex, coherentRasterDepth)); vb.BaryY = 1f;
        var vc = V(drawC, f, dith, perspectiveCorrect, screenSpacePrimitive,
            DepthOf(c, f.OtIndex, coherentRasterDepth)); vc.BaryZ = 1f;
        // The panorama is two big textured quads at a constant far depth,
        // written to a small static packet buffer. They are positioned for the
        // authored 320-wide window, so in widescreen the strip can start
        // inside the frame and leave a wedge of bare background at the edge -
        // 630 units of strip for a 428-wide frame, just offset. Carry the
        // outer edge out to the frame boundary, moving the texture coordinate
        // with it at the rate the quad itself establishes, so the sky
        // continues rather than stretches.
        // Backface culling, done here instead of by the engine. The engine's
        // NCLIP test decides this from coordinates clamped to +/-1024 and a
        // divide that overflows below depth 128, which is why it discards
        // polygons of anything the camera gets close to. The same decision
        // from the reconstructed camera-space projection is exact: float
        // arithmetic, no clamp, no overflow.
        //
        // Terrain is excluded. Its winding convention differs and culling it
        // here renders the ground as a checkerboard of undersides.
        if (RendererCull && GpuHle.GameplayActive && !f.Vehicle &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest &&
            va.HasViewSpace >= 1f && vb.HasViewSpace >= 1f &&
            vc.HasViewSpace >= 1f)
        {
            float ax = ReconstructedX(va), ay = ReconstructedY(va);
            float bx = ReconstructedX(vb), by = ReconstructedY(vb);
            float cx = ReconstructedX(vc), cy = ReconstructedY(vc);
            float area = ax * (by - cy) + bx * (cy - ay) + cx * (ay - by);
            // Sign convention determined empirically: culling the opposite
            // sign removed geometry that should be visible.
            if (area >= 0f)
                return;
        }

        if (BackdropFill && GpuHle.GameplayActive &&
            _kTarget is { Margin: > 0 } bdTarget &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest)
        {
            float lo = MathF.Min(va.ViewZ, MathF.Min(vb.ViewZ, vc.ViewZ));
            float hi = MathF.Max(va.ViewZ, MathF.Max(vb.ViewZ, vc.ViewZ));
            float rx0 = ReconstructedX(va);
            float rx1 = ReconstructedX(vb);
            float rx2 = ReconstructedX(vc);
            float span = MathF.Max(rx0, MathF.Max(rx1, rx2)) -
                MathF.Min(rx0, MathF.Min(rx1, rx2));
            // Far, flat in depth and very wide: the panorama, and nothing
            // else in the scene looks like that.
            if (lo > 2500f && hi < 4200f && (hi - lo) < 200f &&
                span > bdTarget.Wide1x * 0.4f)
            {
                _backdropTriangles++;
                _backdropPackets.Add(f.PacketAddress);
                _backdropPending.Add(
                    (_count, _count + 1, _count + 2, f.PacketAddress));
            }
        }

        if (TraceTerrainFrames && GpuHle.GameplayActive)
        {
            if (f.Material == HleMaterialKind.TerrainRoute)
                _terrainFrameTriangles++;
            if (f.Material is HleMaterialKind.Opaque or
                HleMaterialKind.AlphaTest or
                HleMaterialKind.TerrainRoute)
                _terrainFrameWorldTriangles++;
            if (f.Material is HleMaterialKind.Opaque or
                HleMaterialKind.AlphaTest or
                HleMaterialKind.TerrainRoute)
            {
                float minZ = MathF.Min(a.ViewZ, MathF.Min(b.ViewZ, c.ViewZ));
                float maxZ = MathF.Max(a.ViewZ, MathF.Max(b.ViewZ, c.ViewZ));
                if (a.HasViewSpace && b.HasViewSpace && c.HasViewSpace)
                {
                    if (minZ <= 0f && maxZ > 0f) _straddleNearPlane++;
                    else if (minZ <= 0f) _behindCamera++;
                }
            }
            if (f.Material is HleMaterialKind.Opaque or
                    HleMaterialKind.AlphaTest &&
                _kTarget is { Margin: > 0 } bandTarget)
            {
                float scale = a.ProjectionScale;
                float centre;
                if (a.HasViewSpace && scale > 0.0001f)
                {
                    centre = (
                        Project(a, scale) +
                        Project(b, scale) +
                        Project(c, scale)) / 3f;
                    static float Project(in HleVertex v, float scale) =>
                        v.ProjectionCenterX +
                        v.ViewX * scale / MathF.Max(v.ViewZ, 0.0001f);
                }
                else
                {
                    centre = (a.X + b.X + c.X) / 3f;
                }
                float span = bandTarget.Wide1x;
                float u = (centre + bandTarget.Margin) / MathF.Max(span, 1f);
                if (u < 0.12f) _objectFrameLeft++;
                else if (u > 0.88f) _objectFrameRight++;
                else _objectFrameMid++;
                _objectFrameMinU = MathF.Min(_objectFrameMinU, u);
                _objectFrameMaxU = MathF.Max(_objectFrameMaxU, u);
            }
        }
        if (DebugTerrainCoverage &&
            f.Material == HleMaterialKind.TerrainRoute)
        {
            va.Texpage |= 0x100000;
            vb.Texpage |= 0x100000;
            vc.Texpage |= 0x100000;
        }
        if (TraceNearDepths &&
            GpuHle.GameplayActive &&
            !f.Vehicle &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest)
        {
            float nearest = MathF.Min(
                va.ViewZ > 1f ? va.ViewZ : float.MaxValue,
                MathF.Min(vb.ViewZ > 1f ? vb.ViewZ : float.MaxValue,
                          vc.ViewZ > 1f ? vc.ViewZ : float.MaxValue));
            if (nearest < float.MaxValue)
                for (int i = 0; i < NearDepthEdges.Length; i++)
                    if (nearest < NearDepthEdges[i])
                    {
                        NearDepthHistogram[i]++;
                        break;
                    }
        }

        if (_geometryDumping && _geometryDump is { } dump)
        {
            // Which engine subsystem submitted this packet. The object gates
            // demonstrably do not affect the arena walls, so naming the
            // submitter is the thing that decides where to look next.
            // Record the operands of the cull test that let this triangle
            // through, next to the packet it produced. Comparing the two
            // establishes whether the renderer sees the same vertex order the
            // engine's test did.
            dump.Append("nclip=")
                .Append(Gte.NclipX0).Append(',').Append(Gte.NclipY0).Append(';')
                .Append(Gte.NclipX1).Append(',').Append(Gte.NclipY1).Append(';')
                .Append(Gte.NclipX2).Append(',').Append(Gte.NclipY2).Append(' ');
            dump.Append(f.PacketAddress.ToString("X8")).Append(' ')
                .Append(GpuHle.DescribePacketOwner(f.PacketAddress)
                    .Replace(' ', '_')).Append(' ')
                .Append((int)f.Material).Append(' ')
                .Append(f.Clut & 0x7FFF).Append(' ')
                .Append(va.Texpage).Append(' ');
            foreach (var vv in new[] { va, vb, vc })
            {
                // The packed X/Y here are the widescreen-compressed integers.
                // What actually lands on screen is the shader's
                // centre + view * scale / depth, and each vertex carries the
                // projection snapshot taken when it was projected - so record
                // both, plus the terms, or a mismatch between two panels
                // sharing an edge is invisible in the dump.
                float sx = vv.ViewZ > 0f
                    ? vv.ProjectionCenterX +
                      vv.ViewX * vv.ProjectionScale / vv.ViewZ
                    : float.NaN;
                float sy = vv.ViewZ > 0f
                    ? vv.ProjectionCenterY +
                      vv.ViewY * vv.ProjectionScale / vv.ViewZ
                    : float.NaN;
                dump.Append(vv.X.ToString("F0")).Append(',')
                    .Append(vv.Y.ToString("F0")).Append(',')
                    .Append(sx.ToString("F3")).Append(',')
                    .Append(sy.ToString("F3")).Append(',')
                    .Append(vv.ViewZ.ToString("F2")).Append(',')
                    .Append(vv.ProjectionCenterX.ToString("F3")).Append(',')
                    .Append(vv.ProjectionScale.ToString("F3")).Append(',')
                    .Append(vv.HasViewSpace.ToString("F0")).Append(' ');
            }
            dump.Append('\n');
        }

        if (TraceVertexHistogram &&
            GpuHle.GameplayActive &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest &&
            _kTarget is { Margin: > 0 })
        {
            // Clipping lands vertices exactly on a boundary, so a clip shows
            // up as one x shared by triangles from unrelated packets.
            void Tally(in GlVertex v, uint packet)
            {
                int bucket = (int)MathF.Round(v.X * 2f);
                if (bucket < -200 || bucket > 1200) return;
                if (!_vertexColumns.TryGetValue(bucket, out var entry))
                    entry = (0, [], float.MaxValue, float.MinValue, 0);
                entry.Count++;
                if (entry.Packets.Count < 64) entry.Packets.Add(packet);
                if (v.ViewZ > 0f)
                {
                    entry.MinZ = MathF.Min(entry.MinZ, v.ViewZ);
                    entry.MaxZ = MathF.Max(entry.MaxZ, v.ViewZ);
                }
                if (v.HasViewSpace < 2f) entry.Approx++;
                _vertexColumns[bucket] = entry;
            }
            Tally(va, f.PacketAddress);
            Tally(vb, f.PacketAddress);
            Tally(vc, f.PacketAddress);
        }
        if (TraceSegmentPop &&
            GpuHle.GameplayActive &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest &&
            _kTarget is { Margin: > 0 } popTarget)
        {
            uint group = f.PacketAddress & 0xFFFFF000u;
            float area = MathF.Abs(
                (vb.X - va.X) * (vc.Y - va.Y) -
                (vb.Y - va.Y) * (vc.X - va.X)) * 0.5f;
            float cx = (va.X + vb.X + vc.X) / 3f;
            float cy = (va.Y + vb.Y + vc.Y) / 3f;
            ref var slot = ref CollectionsMarshal.GetValueRefOrAddDefault(
                _segmentArea, group, out _);
            slot.Area += area;
            slot.SumX += cx * area;
            slot.SumY += cy * area;
        }
        if (TraceSeveredGeometry &&
            GpuHle.GameplayActive &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest &&
            _kTarget is { Margin: > 0 })
        {
            _severedTriangles.Add((
                f.PacketAddress,
                va.X, va.Y, vb.X, vb.Y, vc.X, vc.Y));
        }
        if (WorldGapScanline is { } gapY &&
            GpuHle.GameplayActive &&
            f.Material is HleMaterialKind.Opaque or HleMaterialKind.AlphaTest &&
            _kTarget is { Margin: > 0 } gapTarget &&
            TryTriangleScanlineInterval(va, vb, vc, gapY,
                out float gapMinX, out float gapMaxX) &&
            gapMaxX - gapMinX > 0.5f)
        {
            _worldGapSpans.Add((gapMinX, gapMaxX, f.PacketAddress, f.Material));
        }
        if (TraceTerrainScanlines.Length != 0 &&
            f.Material == HleMaterialKind.TerrainRoute &&
            (TraceTerrainCellTicks is not { } scanTicks ||
             (GpuHle.DebugGameplayTick >= scanTicks.Start &&
              GpuHle.DebugGameplayTick <= scanTicks.End)))
        {
            foreach (int scanline in TraceTerrainScanlines)
            {
                if (_traceTerrainScanlineTriangles >= 200000 ||
                    !TryTriangleScanlineInterval(
                        va,
                        vb,
                        vc,
                        scanline,
                        out float scanMinX,
                        out float scanMaxX))
                    continue;
                _traceTerrainScanlineTriangles++;
                Console.Error.WriteLine(
                    "[TerrainScanline] " +
                    $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                    $"y={scanline} x={scanMinX:F3}..{scanMaxX:F3} " +
                    $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                    $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                    $"draw-xy=({va.X},{va.Y})({vb.X},{vb.Y})({vc.X},{vc.Y})");
            }
        }
        if (TriangleProbe is { } probe &&
            GpuHle.GameplayActive &&
            _pendingProbeTriangles.Count < 1024)
        {
            float targetProbeX = (_kTarget?.X ?? 0) + probe.X;
            float targetProbeY = (_kTarget?.Y ?? 0) + probe.Y;
            float area2 = MathF.Abs(
                (vb.X - va.X) * (vc.Y - va.Y) -
                (vb.Y - va.Y) * (vc.X - va.X));
            // A collapsed triangle makes all three edge tests zero, which
            // the point-in-triangle helper correctly treats as lying on every
            // edge but is useless for coverage attribution.  Exclude it from
            // the probe rather than reporting every zero-area packet as an
            // owner of the sampled pixel.
            bool coversProbe = area2 > 0.001f && ContainsPoint(
                va, vb, vc,
                targetProbeX,
                targetProbeY);
            if (coversProbe)
            {
                string triangle =
                    $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                    $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                    $"material={f.Material} vehicle={(f.Vehicle ? 1 : 0)} " +
                    $"depth-test={(depthTest ? 1 : 0)} " +
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
                    $"{(c.HasGteZ ? 1 : 0)}) " +
                    $"coherent=({(a.HasCoherentGteZ ? 1 : 0)}," +
                    $"{(b.HasCoherentGteZ ? 1 : 0)}," +
                    $"{(c.HasCoherentGteZ ? 1 : 0)})";
                _pendingProbeTriangles.Add(triangle);
            }
        }

        static bool TryTriangleScanlineInterval(
            in GlVertex a,
            in GlVertex b,
            in GlVertex c,
            float y,
            out float minX,
            out float maxX)
        {
            float[] intersections = new float[6];
            int count = 0;
            AddEdge(a.X, a.Y, b.X, b.Y);
            AddEdge(b.X, b.Y, c.X, c.Y);
            AddEdge(c.X, c.Y, a.X, a.Y);
            if (count == 0)
            {
                minX = 0f;
                maxX = 0f;
                return false;
            }

            minX = intersections[0];
            maxX = intersections[0];
            for (int index = 1; index < count; index++)
            {
                minX = Math.Min(minX, intersections[index]);
                maxX = Math.Max(maxX, intersections[index]);
            }
            return true;

            void AddEdge(float x0, float y0, float x1, float y1)
            {
                float edgeMinY = Math.Min(y0, y1);
                float edgeMaxY = Math.Max(y0, y1);
                if (y < edgeMinY || y > edgeMaxY)
                    return;
                if (Math.Abs(y1 - y0) < 0.0001f)
                {
                    intersections[count++] = x0;
                    intersections[count++] = x1;
                    return;
                }
                float t = (y - y0) / (y1 - y0);
                intersections[count++] = x0 + (x1 - x0) * t;
            }
        }

        static void ExpandBackdropEdge(
            ref HleVertex vertex,
            GlDisplayRt target)
        {
            if (vertex.X <= target.X)
                vertex.X -= target.Margin;
            else if (vertex.X >= target.X + target.W)
                vertex.X += target.Margin;
        }
        float uvMinX = a.HasAuthoredUvBounds
            ? a.AuthoredMinU
            : MathF.Min(a.U, MathF.Min(b.U, c.U));
        float uvMinY = a.HasAuthoredUvBounds
            ? a.AuthoredMinV
            : MathF.Min(a.V, MathF.Min(b.V, c.V));
        float uvMaxX = a.HasAuthoredUvBounds
            ? a.AuthoredMaxU
            : MathF.Max(a.U, MathF.Max(b.U, c.U));
        float uvMaxY = a.HasAuthoredUvBounds
            ? a.AuthoredMaxV
            : MathF.Max(a.V, MathF.Max(b.V, c.V));
        va.UvMinX = vb.UvMinX = vc.UvMinX = uvMinX;
        va.UvMinY = vb.UvMinY = vc.UvMinY = uvMinY;
        va.UvMaxX = vb.UvMaxX = vc.UvMaxX = uvMaxX;
        va.UvMaxY = vb.UvMaxY = vc.UvMaxY = uvMaxY;
        ApplyTextureReplacement(
            ref va, ref vb, ref vc, f,
            uvMinX, uvMinY, uvMaxX, uvMaxY,
            out _, out _,
            allowUiReplacement:
                screenSpacePrimitive &&
                !topGameplayHudTriangle);
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

    bool CullByPacketNclip(
        in HleVertex a,
        in HleVertex b,
        in HleVertex c,
        in PrimFlags flags)
    {
        if (!PacketNclipCull || !GpuHle.GameplayActive ||
            flags.Material is HleMaterialKind.TerrainRoute or
                HleMaterialKind.Ui or HleMaterialKind.ScreenEffect)
            return false;

        if (!GpuHle.TryGetTriangleNclipPacket(
                flags.PacketAddress, out GpuHle.TriangleNclipPacket packet))
        {
            if (TracePacketNclipCull)
                _packetNclipMissing++;
            return false;
        }

        double preciseArea;
        if (packet.HasPreciseArea)
        {
            preciseArea = packet.PreciseArea;
        }
        else if (packet.HasPacketOrder &&
                 a.HasViewSpace && b.HasViewSpace && c.HasViewSpace)
        {
            Span<HleVertex> vertices = [a, b, c];
            HleVertex p0 = vertices[packet.Gte0PacketIndex];
            HleVertex p1 = vertices[packet.Gte1PacketIndex];
            HleVertex p2 = vertices[packet.Gte2PacketIndex];
            double x0 = ReconstructedHleX(p0);
            double y0 = ReconstructedHleY(p0);
            double x1 = ReconstructedHleX(p1);
            double y1 = ReconstructedHleY(p1);
            double x2 = ReconstructedHleX(p2);
            double y2 = ReconstructedHleY(p2);
            preciseArea =
                x0 * (y1 - y2) +
                x1 * (y2 - y0) +
                x2 * (y0 - y1);
        }
        else
        {
            if (TracePacketNclipCull)
                _packetNclipMissing++;
            return false;
        }

        if (preciseArea > 0d)
        {
            if (TracePacketNclipCull)
            {
                _packetNclipFrontFaces++;
                if (flags.Vehicle)
                    _packetNclipVehicleFrontFaces++;
                else
                    _packetNclipObjectFrontFaces++;
                if (packet.PackedArea <= 0)
                {
                    _packetNclipRescuedFaces++;
                    if (flags.Vehicle)
                        _packetNclipVehicleRescuedFaces++;
                    else
                        _packetNclipObjectRescuedFaces++;
                }
            }
            return false;
        }

        if (TracePacketNclipCull)
        {
            _packetNclipBackFaces++;
            if (flags.Vehicle)
                _packetNclipVehicleBackFaces++;
            else
                _packetNclipObjectBackFaces++;
        }
        return true;
    }

    static double ReconstructedHleX(in HleVertex vertex) =>
        vertex.HasViewSpace && vertex.ViewZ > 0f &&
        vertex.ProjectionScale != 0f
            ? vertex.ProjectionCenterX +
                vertex.ViewX * vertex.ProjectionScale / vertex.ViewZ
            : vertex.X;

    static double ReconstructedHleY(in HleVertex vertex) =>
        vertex.HasViewSpace && vertex.ViewZ > 0f &&
        vertex.ProjectionScale != 0f
            ? vertex.ProjectionCenterY +
                vertex.ViewY * vertex.ProjectionScale / vertex.ViewZ
            : vertex.Y;

    /// <summary>
    /// Remembers the flat full-display quad the engine lays down behind each
    /// gameplay frame. That quad is the arena's authored horizon colour, so it
    /// is the only correct target for distance fog: fading towards anything
    /// else leaves far geometry standing out against the sky instead of
    /// dissolving into it. Each arena supplies its own, and it is retained
    /// across frames so a frame that skips the fill still fogs correctly.
    /// </summary>
    void RecordAtmosphereColor(
        in HleVertex a, in HleVertex b, in HleVertex c, in PrimFlags f)
    {
        if (!FogAtmosphereColor)
            return;

        float r = (a.R + b.R + c.R) / (3f * 255f);
        float g = (a.G + b.G + c.G) / (3f * 255f);
        float blue = (a.B + b.B + c.B) / (3f * 255f);
        // A black or near-black fill is a framebuffer clear, not a sky.
        if (r + g + blue < 0.09f)
            return;

        // The sky is laid down at the far end of the ordering table before
        // anything else in the frame. A nearer full-display fill is a flash,
        // fade, or damage overlay and must not be read as the horizon.
        //
        // The backdrop itself arrives as stacked bands - Route 66 uses a
        // narrow upper band over one tall band that runs down past the
        // horizon and behind all the terrain. Fog has to converge on the band
        // the terrain actually disappears into, so take the tallest one.
        float span =
            MathF.Max(a.Y, MathF.Max(b.Y, c.Y)) -
            MathF.Min(a.Y, MathF.Min(b.Y, c.Y));
        float minimum = MathF.Min(r, MathF.Min(g, blue));
        float maximum = MathF.Max(r, MathF.Max(g, blue));
        bool nearWhite = minimum > 0.94f && maximum - minimum < 0.04f;
        if (_fogColorFrame != _frame)
        {
            _fogColorFrame = _frame;
            _fogColorOt = int.MinValue;
            _fogColorSpan = 0f;
            _fogColorNearWhite = false;
        }
        if (f.OtIndex < _fogColorOt ||
            (f.OtIndex == _fogColorOt &&
             nearWhite && !_fogColorNearWhite) ||
            (f.OtIndex == _fogColorOt &&
             nearWhite == _fogColorNearWhite &&
             span < _fogColorSpan))
            return;
        _fogColorOt = f.OtIndex;
        _fogColorSpan = span;
        _fogColorNearWhite = nearWhite;

        _fogColorR = r;
        _fogColorG = g;
        _fogColorB = blue;
        _hasFogColor = true;
        if (_fogColorLogged < 8)
        {
            _fogColorLogged++;
            Console.Error.WriteLine(
                "[EnhancedFog] atmosphere candidate " +
                $"rgb=({a.R},{a.G},{a.B}) ot={f.OtIndex} frame={_frame} " +
                $"y={MathF.Min(a.Y, MathF.Min(b.Y, c.Y)):F0}.." +
                $"{MathF.Max(a.Y, MathF.Max(b.Y, c.Y)):F0}");
        }
    }

    static void StabilizeTerrainPacketVertex(
        ref HleVertex vertex,
        float centerX,
        float wideRatio)
    {
        vertex.X =
            centerX + (vertex.X - centerX) * wideRatio;
        vertex.HasViewSpace = false;
        vertex.ReconstructedViewSpace = false;
    }

    static float ReadTerrainSeamGuardPixels()
    {
        string? text = Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_SEAM_GUARD_PIXELS");
        return float.TryParse(
            text,
            NumberStyles.Float,
            CultureInfo.InvariantCulture,
            out float value)
            ? Math.Clamp(value, 0f, 2f)
            : 0f;
    }

    /// <summary>
    /// Widens a terrain triangle's coverage by <paramref name="guard"/> logical
    /// pixels, in the projected space the terrain path is actually rasterized
    /// from.
    ///
    /// Terrain approaching the horizon quantizes to strips a fraction of a
    /// native pixel tall, and adjacent strips can fail to meet once drawn at
    /// several times the authored resolution. The renderer projects terrain
    /// from camera space as
    /// <c>centre + view.xy * (scale / view.z)</c>, so displacing a vertex by
    /// <c>d</c> projected pixels is exactly <c>view.xy += d * view.z / scale</c>.
    /// Nothing else about the vertex changes: depth, UVs and colour are
    /// untouched.
    /// </summary>
    static void ApplyTerrainSeamGuardViewSpace(
        ref HleVertex a,
        ref HleVertex b,
        ref HleVertex c,
        float guard)
    {
        float scale = a.ProjectionScale;
        if (scale <= 0.0001f)
            return;

        Span<float> px = stackalloc float[3];
        Span<float> py = stackalloc float[3];
        Span<float> vz = stackalloc float[3];
        vz[0] = MathF.Max(a.ViewZ, 0.0001f);
        vz[1] = MathF.Max(b.ViewZ, 0.0001f);
        vz[2] = MathF.Max(c.ViewZ, 0.0001f);
        px[0] = a.ProjectionCenterX + a.ViewX * scale / vz[0];
        px[1] = b.ProjectionCenterX + b.ViewX * scale / vz[1];
        px[2] = c.ProjectionCenterX + c.ViewX * scale / vz[2];
        py[0] = a.ProjectionCenterY + a.ViewY * scale / vz[0];
        py[1] = b.ProjectionCenterY + b.ViewY * scale / vz[1];
        py[2] = c.ProjectionCenterY + c.ViewY * scale / vz[2];

        float cx = (px[0] + px[1] + px[2]) / 3f;
        float cy = (py[0] + py[1] + py[2]) / 3f;

        Span<float> dx = stackalloc float[3];
        Span<float> dy = stackalloc float[3];
        for (int i = 0; i < 3; i++)
        {
            float ox = px[i] - cx;
            float oy = py[i] - cy;
            float length = MathF.Sqrt(ox * ox + oy * oy);
            if (length < 0.0001f)
            {
                dx[i] = 0f;
                dy[i] = 0f;
                continue;
            }
            dx[i] = ox / length * guard;
            dy[i] = oy / length * guard;
        }

        // A strip that quantized flat gains nothing from a radial push: every
        // offset lies along the strip. Give it height instead, by lifting the
        // middle vertex clear of the line through the other two.
        float ax = px[2] - px[0];
        float ay = py[2] - py[0];
        float bx = px[1] - px[0];
        float by = py[1] - py[0];
        float area = MathF.Abs(ax * by - ay * bx) * 0.5f;
        float longest = MathF.Max(
            MathF.Sqrt(ax * ax + ay * ay),
            MathF.Max(
                MathF.Sqrt(bx * bx + by * by),
                MathF.Sqrt(
                    (px[2] - px[1]) * (px[2] - px[1]) +
                    (py[2] - py[1]) * (py[2] - py[1]))));
        if (longest > 0.0001f && area < guard * longest * 0.5f)
        {
            float ux = ax, uy = ay;
            float ulength = MathF.Sqrt(ux * ux + uy * uy);
            if (ulength > 0.0001f)
            {
                ux /= ulength;
                uy /= ulength;
                float nx = -uy;
                float ny = ux;
                // Whichever vertex sits between the other two along the strip
                // is the one that can open it up.
                int middle = 1;
                float t0 = 0f;
                float t2 = ax * ux + ay * uy;
                float t1 = bx * ux + by * uy;
                if (t1 < MathF.Min(t0, t2) || t1 > MathF.Max(t0, t2))
                    middle = t0 >= MathF.Min(t1, t2) && t0 <= MathF.Max(t1, t2)
                        ? 0
                        : 2;
                float side = (px[middle] - cx) * nx + (py[middle] - cy) * ny;
                float sign = side >= 0f ? 1f : -1f;
                dx[middle] += nx * sign * guard;
                dy[middle] += ny * sign * guard;
                for (int i = 0; i < 3; i++)
                {
                    if (i == middle) continue;
                    dx[i] -= nx * sign * guard * 0.5f;
                    dy[i] -= ny * sign * guard * 0.5f;
                }
            }
        }

        a.ViewX += dx[0] * vz[0] / scale;
        a.ViewY += dy[0] * vz[0] / scale;
        b.ViewX += dx[1] * vz[1] / scale;
        b.ViewY += dy[1] * vz[1] / scale;
        c.ViewX += dx[2] * vz[2] / scale;
        c.ViewY += dy[2] * vz[2] / scale;
    }

    void DrawCleanModalPanel(in HleRect r, in PrimFlags f)
    {
        Begin(f, 6);
        PrimFlags panelFlags = f;
        float left = r.X - 7, top = r.Y - 9;
        int width = r.W + 14, height = r.H + 18;
        GlVertex Corner(int x, int y)
        {
            var source = new HleVertex { X = left + x, Y = top + y };
            var vertex = V(source, panelFlags, false, false, true);
            vertex.Texpage |= 0x01000000;
            vertex.U = x; vertex.V = y;
            vertex.UvMaxX = width; vertex.UvMaxY = height;
            return vertex;
        }
        var a = Corner(0, 0); var b = Corner(width, 0);
        var c = Corner(0, height); var d = Corner(width, height);
        _verts[_count++] = a; _verts[_count++] = b; _verts[_count++] = c;
        _verts[_count++] = b; _verts[_count++] = d; _verts[_count++] = c;
    }

    public void DrawRect(in HleRect r, in PrimFlags f)
    {
        if (GpuHle.IsNativeModalPanel(f.PacketAddress))
        {
            DrawCleanModalPanel(r, f);
            return;
        }
        int sourceU1 = r.U + (r.FlipX ? -r.W : r.W);
        int sourceV1 = r.V + (r.FlipY ? -r.H : r.H);
        CheckTextureFeedback(
            f,
            Math.Min(r.U, sourceU1),
            Math.Min(r.V, sourceV1),
            Math.Max(r.U, sourceU1),
            Math.Max(r.V, sourceV1));
        bool loadingStartPromptGlyph =
            f.Textured &&
            f.TPage == 0x0A5 &&
            f.Clut == 0x7800 &&
            r.Y == 209 &&
            r.H == 15;
        if (loadingStartPromptGlyph)
            Flush();
        Begin(f, 6);
        if (TraceRectangles && GpuHle.GameplayActive &&
            _traceRectangleShapes.Count < 256)
        {
            string shape =
                $"ot={f.OtIndex} xy={r.X},{r.Y} wh={r.W}x{r.H} " +
                $"uv={r.U},{r.V} tex={(f.Textured ? 1 : 0)} " +
                $"semi={(f.SemiTrans ? 1 : 0)} raw={(f.RawTexture ? 1 : 0)} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"packet=0x{f.PacketAddress:X8}";
            if (_traceRectangleShapes.Add(shape))
                Console.WriteLine($"[V82Rect] {shape}");
        }
        bool loadingScreenUi =
            GpuHle.GameplayActive && GpuHle.DebugGameplayTick == 0;
        bool topGameplayHud =
            GpuHle.GameplayActive &&
            !GpuHle.NativeModalActive &&
            !loadingScreenUi &&
            _kTarget is { } hudTarget &&
            r.Y - hudTarget.Y < hudTarget.H * 0.42f;
        float drawX = r.X;
        float drawY = r.Y;
        if (loadingStartPromptGlyph)
            drawY -= 17f;
        int drawW = r.W;
        short drawU = r.U;
        GlDisplayRt? fullDisplayTarget =
            _kTarget is { Margin: > 0 } candidateFullDisplay
                ? candidateFullDisplay
                : null;
        bool fullDisplayGameplayRect =
            GpuHle.GameplayActive &&
            !f.Textured &&
            fullDisplayTarget != null &&
            r.X <= fullDisplayTarget.X &&
            r.X + r.W >= fullDisplayTarget.X + fullDisplayTarget.W &&
            r.Y <= fullDisplayTarget.Y &&
            r.Y + r.H >= fullDisplayTarget.Y + fullDisplayTarget.H;
        if (fullDisplayGameplayRect)
        {
            GlDisplayRt displayTarget = fullDisplayTarget!;
            // Native fades and lighting passes cover the complete 320x240
            // display. The widened target owns real pixels outside that
            // rectangle, so extend the command itself across both margins.
            // Treating it as ordinary top HUD only moves it left and leaves
            // the right widescreen region completely unaffected.
            drawX = displayTarget.X - displayTarget.Margin;
            drawW = displayTarget.W + displayTarget.Margin * 2;
        }
        int resolvedU =
            (r.U & (~(_env.TwMaskX * 8) & 0xFF)) |
            ((_env.TwOffX & _env.TwMaskX) * 8);
        int resolvedV =
            (r.V & (~(_env.TwMaskY * 8) & 0xFF)) |
            ((_env.TwOffY & _env.TwMaskY) * 8);
        // The retail status rectangle starts at x=80, but its live vehicle
        // portrait is x=76..116 and its complete armor bar is x=80..112.
        // Extend only the vector backing six native pixels left: the copied
        // 44-pixel weapon-panel silhouette then lands at x=74..118, centering
        // the portrait at 2/2 pixels and the armor bar at 6/6 pixels.
        // Guest target banks replace the packet's UV, page, and CLUT, so the
        // authored screen rectangle is the stable identity for this backing.
        float statusLocalY = _kTarget is { } statusTarget
            ? r.Y - statusTarget.Y
            : float.NaN;
        bool statusHudBacking =
            ConfigManager.View.VectorIcons &&
            topGameplayHud &&
            f.Textured && f.RawTexture && f.SemiTrans &&
            r.X == 80 && statusLocalY == 20f &&
            r.W == 84 && r.H == 34 &&
            f.Material is HleMaterialKind.Ui or HleMaterialKind.ScreenEffect;
        if (statusHudBacking)
        {
            drawX -= 6f;
            drawW += 6;
            drawU -= 6;
        }
        float anchor = 0f;
        if (TraceModalRects && GpuHle.NativeModalActive &&
            _kTarget is { } modalTarget && _modalRectLines++ < 400)
            Console.Error.WriteLine(
                $"[V82Modal] x={r.X} w={r.W} y={r.Y} h={r.H} " +
                $"tex={(f.Textured ? 1 : 0)} tpage=0x{f.TPage:X3} " +
                $"clut=0x{f.Clut:X4} " +
                $"target=({modalTarget.X},{modalTarget.Y}," +
                $"{modalTarget.W}x{modalTarget.H}) margin={modalTarget.Margin}");
        // The modal's rectangles -- panel, caption, entries, arrows -- already
        // sit where they belong once HUD anchoring stops moving them per
        // piece. Only its border needs correcting, and that is drawn as flat
        // 2D triangles; see DrawTri. Measured against the 4:3 layout, content
        // sits +10.8% of border width right of the border centre, and this
        // pairing reproduces it (+9.7%) with the border itself centred.
        if (GpuHle.NativeModalActive && _kTarget is { Margin: > 0 })
            anchor = 0f;
        else if (fullDisplayGameplayRect)
            anchor = 0f;
        else if (ConfigManager.View.HudAnchoring && GpuHle.GameplayActive &&
            _kTarget is { Margin: > 0 } target)
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
            bool smallText =
                f.Textured && !topGameplayHud &&
                r.W <= 32 && r.H <= 32;
            bool lowerLeftHud =
                (!smallText && localCenter < target.W / 3f) ||
                locationCaption;
            if (topHud || lowerLeftHud)
                anchor = -target.Margin;
        }
        if (TriangleProbe is { } rectangleProbe &&
            GpuHle.GameplayActive &&
            _kTarget is { } rectangleTarget)
        {
            float targetProbeX = rectangleTarget.X + rectangleProbe.X;
            float targetProbeY = rectangleTarget.Y + rectangleProbe.Y;
            float rectangleX0 = Math.Min(drawX + anchor, drawX + drawW + anchor);
            float rectangleY0 = Math.Min(drawY, drawY + r.H);
            float rectangleX1 = Math.Max(drawX + anchor, drawX + drawW + anchor);
            float rectangleY1 = Math.Max(drawY, drawY + r.H);
            if (targetProbeX >= rectangleX0 &&
                targetProbeX < rectangleX1 &&
                targetProbeY >= rectangleY0 &&
                targetProbeY < rectangleY1 &&
                _traceRectangleProbeHits++ < 4096)
                Console.Error.WriteLine(
                    "[V8RectangleProbe] " +
                    $"frame={_frame} packet=0x{f.PacketAddress:X8} " +
                    $"ot={f.OtIndex} " +
                    $"owner=\"{GpuHle.DescribePacketOwner(f.PacketAddress)}\" " +
                    $"material={f.Material} " +
                    $"vehicle={(f.Vehicle ? 1 : 0)} " +
                    $"tex={(f.Textured ? 1 : 0)} " +
                    $"semi={(f.SemiTrans ? 1 : 0)} " +
                    $"raw={(f.RawTexture ? 1 : 0)} " +
                    $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                    $"source-xy={r.X},{r.Y} wh={r.W}x{r.H} " +
                    $"draw-xy={drawX + anchor},{drawY} " +
                    $"draw-wh={drawW}x{r.H} uv={drawU},{r.V} " +
                    $"probe={targetProbeX},{targetProbeY} " +
                    $"target={rectangleTarget.X},{rectangleTarget.Y}");
        }
        short drawU1 = (short)(drawU + (r.FlipX ? -drawW : drawW));
        short drawV1 = (short)(r.V + (r.FlipY ? -r.H : r.H));
        var a = new HleVertex { X = drawX + anchor, Y = drawY, R = r.R, G = r.G, B = r.B, U = drawU, V = r.V };
        var b = new HleVertex { X = drawX + drawW + anchor, Y = drawY, R = r.R, G = r.G, B = r.B, U = drawU1, V = r.V };
        var c = new HleVertex { X = drawX + anchor, Y = drawY + r.H, R = r.R, G = r.G, B = r.B, U = drawU, V = drawV1 };
        var d = new HleVertex { X = drawX + drawW + anchor, Y = drawY + r.H, R = r.R, G = r.G, B = r.B, U = drawU1, V = drawV1 };
        // The compact top HUD uses tightly packed atlas cells. Keep their
        // authored binary silhouettes exact; sampling across a cell boundary
        // can pull neighboring digits into the ammo counter. Larger gameplay
        // labels and menu text remain eligible for contour reconstruction.
        bool fontLike = f.Textured && !topGameplayHud && r.W <= 32 && r.H <= 32;
        bool radarPlate =
            topGameplayHud && f.Textured && r.W == 64 && r.H == 55;
        // Only the exact authored raw/semitransparent status record owns the
        // three-shape SVG. Same-sized active-weapon contents retain their
        // game-driven texture.
        bool mainHudPlate = statusHudBacking;
        bool healthHudPlate =
            topGameplayHud && f.Textured && r.W == 16 && r.H == 49;
        bool hudBackgroundPlate =
            radarPlate || mainHudPlate || healthHudPlate;
        bool iconLike =
            f.Textured && !fontLike &&
            r.W <= 96 && r.H <= 96 &&
            (!topGameplayHud || hudBackgroundPlate);
        bool highResolutionUiFonts =
            fontLike &&
            (ConfigManager.View.VectorFonts ||
             ConfigManager.View.HighResolutionTextures);
        int uiFlags = highResolutionUiFonts ? 0x2800 :
            iconLike && ConfigManager.View.VectorIcons ? 0x4800 : 0;
        bool topHudKeyedMagenta =
            topGameplayHud &&
            f.Textured && f.RawTexture && f.SemiTrans &&
            r.W == 40 && r.H == 16 &&
            r.X >= 70 && r.X <= 130;
        if (topHudKeyedMagenta)
            uiFlags |= 0x400000;
        if (ConfigManager.View.VectorIcons)
        {
            if (radarPlate) uiFlags |= 0x10000;
            if (hudBackgroundPlate) uiFlags |= 0x20000;
            if (healthHudPlate) uiFlags |= 0x40000;
        }
        if (TraceHud && topGameplayHud && _traceHudPackets.Count < 512)
        {
            string packet =
                $"packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                $"xy={r.X},{r.Y} wh={r.W}x{r.H} uv={r.U},{r.V} " +
                $"flip-x={(r.FlipX ? 1 : 0)} flip-y={(r.FlipY ? 1 : 0)} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"textured={(f.Textured ? 1 : 0)} " +
                $"semi={(f.SemiTrans ? 1 : 0)} raw={(f.RawTexture ? 1 : 0)} " +
                $"status-backing={(statusHudBacking ? 1 : 0)} " +
                $"radar={(radarPlate ? 1 : 0)} health={(healthHudPlate ? 1 : 0)} " +
                $"resolved-uv={resolvedU},{resolvedV} " +
                $"draw-x={drawX} draw-w={drawW} draw-u={drawU},{drawU1} " +
                $"ui-flags=0x{uiFlags:X}";
            if (_traceHudPackets.Add(packet))
                Console.Error.WriteLine($"[V82HudPacket] {packet}");
        }
        var va = V(a, f, false, false, true); va.Texpage |= uiFlags;
        var vb = V(b, f, false, false, true); vb.Texpage |= uiFlags;
        var vc = V(c, f, false, false, true); vc.Texpage |= uiFlags;
        var vd = V(d, f, false, false, true); vd.Texpage |= uiFlags;
        // Large authored menu/loading rectangles stay in the native 4:3
        // composition, but receive the same bounded palette-resolved
        // reconstruction as world textures. Small glyphs and compact icons
        // retain their dedicated contour paths and never bleed across atlas
        // cells.
        bool largeUiArtwork =
            ConfigManager.View.TextureSmoothing &&
            f.Textured &&
            !topGameplayHud &&
            !fontLike &&
            r.W >= 64 && r.H >= 48;
        if (largeUiArtwork)
        {
            va.Texpage |= 0x800;
            vb.Texpage |= 0x800;
            vc.Texpage |= 0x800;
            vd.Texpage |= 0x800;
        }
        float uvMinX = Math.Min(drawU, drawU1);
        float uvMinY = Math.Min(r.V, drawV1);
        float uvMaxX = Math.Max(drawU, drawU1) - 1f;
        float uvMaxY = Math.Max(r.V, drawV1) - 1f;
        if (TraceLoadingUiTextures &&
            GpuHle.GameplayActive &&
            GpuHle.DebugGameplayTick == 0 &&
            f.Textured &&
            _traceLoadingUiTextures.Count < 1024)
        {
            string packet =
                $"frame={_frame} packet=0x{f.PacketAddress:X8} ot={f.OtIndex} " +
                $"xy={drawX + anchor:F1},{drawY:F1} wh={drawW}x{r.H} " +
                $"uv={uvMinX:F1},{uvMinY:F1}-{uvMaxX:F1},{uvMaxY:F1} " +
                $"raw-uv={r.U},{r.V} tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"font-like={(fontLike ? 1 : 0)} icon-like={(iconLike ? 1 : 0)} " +
                $"large-ui={(largeUiArtwork ? 1 : 0)} " +
                $"color={r.R},{r.G},{r.B} semi={(f.SemiTrans ? 1 : 0)} " +
                $"raw={(f.RawTexture ? 1 : 0)} material={f.Material}";
            if (_traceLoadingUiTextures.Add(packet))
                Console.Error.WriteLine($"[V82LoadingUiTexture] {packet}");
        }
        va.UvMinX = vb.UvMinX = vc.UvMinX = vd.UvMinX = uvMinX;
        va.UvMinY = vb.UvMinY = vc.UvMinY = vd.UvMinY = uvMinY;
        va.UvMaxX = vb.UvMaxX = vc.UvMaxX = vd.UvMaxX = uvMaxX;
        va.UvMaxY = vb.UvMaxY = vc.UvMaxY = vd.UvMaxY = uvMaxY;
        bool replacementHit = ApplyTextureReplacement(
            ref va, ref vb, ref vc, f,
            uvMinX, uvMinY, uvMaxX, uvMaxY,
            out ulong loadingTextureKey,
            out string loadingTextureResolution,
            allowUiReplacement:
                (f.Material is HleMaterialKind.Ui or
                    HleMaterialKind.ScreenEffect) &&
                !topGameplayHud);
        if (TraceLoadingUiTextures &&
            GpuHle.GameplayActive &&
            GpuHle.DebugGameplayTick == 0 &&
            fontLike &&
            _traceLoadingFontResolves.Count < 2048)
        {
            string packet =
                $"frame={_frame} key={loadingTextureKey:x16} " +
                $"hit={(replacementHit ? 1 : 0)} " +
                $"size={r.W}x{r.H} uv={uvMinX:F0},{uvMinY:F0}-" +
                $"{uvMaxX:F0},{uvMaxY:F0} " +
                $"tpage=0x{f.TPage:X3} clut=0x{f.Clut:X4} " +
                $"rect={va.ReplacementX:F0},{va.ReplacementY:F0}," +
                $"{va.ReplacementW:F0},{va.ReplacementH:F0} " +
                loadingTextureResolution;
            if (_traceLoadingFontResolves.Add(packet))
                Console.Error.WriteLine($"[V82LoadingFontResolve] {packet}");
        }
        vd.ReplacementX = va.ReplacementX;
        vd.ReplacementY = va.ReplacementY;
        vd.ReplacementW = va.ReplacementW;
        vd.Texpage = va.Texpage;
        vd.ReplacementH = va.ReplacementH;
        vd.ReplacementScaleR = va.ReplacementScaleR;
        vd.ReplacementScaleG = va.ReplacementScaleG;
        vd.ReplacementScaleB = va.ReplacementScaleB;
        vd.ReplacementBiasR = va.ReplacementBiasR;
        vd.ReplacementBiasG = va.ReplacementBiasG;
        vd.ReplacementBiasB = va.ReplacementBiasB;
        if (topGameplayHud)
        {
            va.Texpage &= ~0x800;
            vb.Texpage &= ~0x800;
            vc.Texpage &= ~0x800;
            vd.Texpage &= ~0x800;
        }
        if (loadingStartPromptGlyph)
        {
            _deferredLoadingPrompt.Add(CaptureDeferredBatch(
                [va, vb, vc, vb, vd, vc]));
        }
        else
        {
            _verts[_count++] = va; _verts[_count++] = vb;
            _verts[_count++] = vc; _verts[_count++] = vb;
            _verts[_count++] = vd; _verts[_count++] = vc;
        }
    }

    public void DrawLine(in HleVertex a, in HleVertex b, in PrimFlags f)
    {
        Begin(f, 6);
        bool dith = ConfigManager.View.Ps1Dithering && _env.Dither;
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
        _verts[_count++] = V(
            v, f, dither, false,
            f.Material == HleMaterialKind.Ui);
    }

    public void FillRect(int x, int y, int w, int h, ushort color15)
    {
        _readCacheValid = false;
        Flush();
        _vram.Fill(x, y, w, h, color15);
        _textureReplacements?.Fill(x, y, w, h, color15);
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

    public int DiscardWidescreenDisplayTargets()
    {
        _readCacheValid = false;
        Flush();
        int discarded = 0;
        for (int index = 0; index < _rts.Length; index++)
        {
            GlDisplayRt? rt = _rts[index];
            if (rt is not { Margin: > 0 })
                continue;
            // The expanded gameplay target has no authored 4:3 destination.
            // Do not write it back over the shell's native VRAM when leaving
            // gameplay; simply retire this host-only surface.
            if (ReferenceEquals(_kTarget, rt))
                _kTarget = null;
            rt.Destroy(_gl);
            _rts[index] = null;
            discarded++;
        }
        return discarded;
    }

    void FillRtFull(GlDisplayRt rt, ushort color15)
    {
        float r = (color15 & 0x1F) / 31f, g = ((color15 >> 5) & 0x1F) / 31f, b = ((color15 >> 10) & 0x1F) / 31f;
        float a = (color15 & 0x8000) != 0 ? 1f : 0f;
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, rt.DrawFbo);
        _gl.Disable(EnableCap.ScissorTest);
        _gl.ClearColor(r, g, b, a);
        _gl.Clear(ClearBufferMask.ColorBufferBit);
        rt.NeedsResolve = true;
        Resolve(rt);
        _gl.BindFramebuffer(FramebufferTarget.Framebuffer, 0);
    }

    public void CopyVram(int sx, int sy, int dx, int dy, int w, int h)
    {
        _readCacheValid = false;
        Flush();
        WritebackDirtyWrappedIntersecting(sx, sy, w, h);
        _vram.CopyRect(sx, sy, dx, dy, w, h);
        _textureReplacements?.Copy(sx, sy, dx, dy, w, h);
        SyncWrappedRtsFromVram(dx, dy, w, h);
    }

    public void WriteVram(int x, int y, int w, int h, ReadOnlySpan<ushort> px)
    {
        _readCacheValid = false;
        Flush();
        _vram.WriteRect(x, y, w, h, px);
        _textureReplacements?.Write(x, y, w, h, px);
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
        ApplyBackdropBatchOuterEdges();
        if (TracePerformance)
        {
            _traceFlushes++;
            if (_kCheckMask != 0) _traceCheckMaskFlushes++;
            if (_kSetMask != 0) _traceSetMaskFlushes++;
        }
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
            // uDest is read only for the native check-mask operation. Keep
            // multisampled colour resident across ordinary batches and
            // resolve only at an actual feedback boundary; resolving the full
            // 3x/8x gameplay target after every material batch made dense
            // levels GPU-bound.
            if (_kCheckMask != 0)
                ResolveCheckMaskRegion(rt);
        }
        _vram.Barrier();

        if (_kSourceDepthCompareWrite)
        {
            // Converted N64 XRTP route surfaces retain the source RDP's
            // opaque Z-compare/Z-update contract. A single color draw must
            // both test and update depth so later route triangles see earlier
            // route triangles in source submission order.
            Debug.Assert(_kDepthTest && _kDepthWrite && !_kTransparent);
            _gl.Enable(EnableCap.DepthTest);
            _gl.DepthFunc(DepthFunction.Lequal);
            _gl.DepthMask(true);
        }
        else if (_kDepthTest)
        {
            _gl.Enable(EnableCap.DepthTest);
            _gl.DepthFunc(DepthFunction.Lequal);
            _gl.DepthMask(false);
        }
        else if (_kDepthWrite)
        {
            _gl.Disable(EnableCap.DepthTest);
            _gl.DepthMask(false);
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
            bool spansDisplay =
                _kClipX0 <= rt.X && _kClipX1 >= rt.X + rt.W - 1;
            if (rt.Margin > 0 && spansDisplay) { cx0 = 0; cx1 = rt.Wide1x - 1; }
            if (TraceTerrainFrames && rt.Margin > 0 && !spansDisplay &&
                GpuHle.GameplayActive &&
                _clipRectsLogged.Add((_kClipX0, _kClipX1, rt.X, rt.W)))
                Console.Error.WriteLine(
                    $"[V82NarrowClip] clip={_kClipX0}..{_kClipX1} " +
                    $"display={rt.X}..{rt.X + rt.W - 1} " +
                    "-> scissored to the authored window, cutting anything " +
                    "the widened view adds");
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
        _gl.ActiveTexture(TextureUnit.Texture3);
        _gl.BindTexture(
            TextureTarget.Texture2D,
            _textureReplacements?.Texture ?? 0);
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
            _gl.Uniform3(_uFogColor, _fogColorR, _fogColorG, _fogColorB);
            _gl.Uniform1(
                _uFogColorValid,
                _hasFogColor ? 1 : 0);
            _gl.Uniform1(
                _uPerspectiveCorrectTextures,
                ConfigManager.View.PerspectiveCorrectTextures ? 1 : 0);
            _gl.Uniform1(
                _uPerspectiveCorrectColors,
                ConfigManager.View.PerspectiveCorrectColors ? 1 : 0);
            _gl.Uniform1(_uTrueColor, ConfigManager.View.TrueColor ? 1 : 0);
            _gl.Uniform1(
                _uVectorFonts,
                ConfigManager.View.VectorFonts ||
                ConfigManager.View.HighResolutionTextures ? 1 : 0);
            _gl.Uniform1(_uVectorIcons, ConfigManager.View.VectorIcons ? 1 : 0);

        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        _gl.BufferSubData<GlVertex>(BufferTargetARB.ArrayBuffer, 0, _verts.AsSpan(0, _count));

        void DrawBatch()
        {
            _gl.DrawArrays(PrimitiveType.Triangles, 0, (uint)_count);
        }

        if (!_kTransparent)
        {
            _gl.Disable(EnableCap.Blend);
            DrawBatch();
            if (_kDepthWrite && !_kSourceDepthCompareWrite)
            {
                // The PS1 has no Z buffer: its opaque colour contract is the
                // ordering-table painter order above. Build a separate
                // nearest-surface depth image only for later transparent
                // tests, without allowing it to reject opaque colour.
                _gl.ColorMask(false, false, false, false);
                _gl.Enable(EnableCap.DepthTest);
                _gl.DepthFunc(DepthFunction.Lequal);
                _gl.DepthMask(true);
                DrawBatch();
                _gl.ColorMask(true, true, true, true);
            }
        }
        else
        {
            _gl.Enable(EnableCap.Blend);
            if (_kMaterial == HleMaterialKind.Glass)
            {
                _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
                _gl.BlendFunc(
                    BlendingFactor.SrcAlpha,
                    BlendingFactor.OneMinusSrcAlpha);
                DrawBatch();
            }
            else
            {
            _gl.BlendFuncSeparate(BlendingFactor.Src1Color, BlendingFactor.Src1Alpha, BlendingFactor.One, BlendingFactor.Zero);
            if (_kBlend == 2)
            {
                _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
                SetBlend(0f, 1f);
                DrawBatch();

                _vram.Barrier();
                _gl.BlendEquationSeparate(BlendEquationModeEXT.FuncReverseSubtract, BlendEquationModeEXT.FuncAdd);
                SetBlend(1f, 1f);
                _gl.Uniform4(_uBlendOpaque, 0f, 0f, 0f, 1f);
                DrawBatch();
            }
            else
            {
                _gl.BlendEquation(BlendEquationModeEXT.FuncAdd);
                SetBlend(_kBlend switch { 0 => 0.5f, 3 => 0.25f, _ => 1f }, _kBlend == 0 ? 0.5f : 1f);
                DrawBatch();
            }
            }
        }
        _gl.Disable(EnableCap.ScissorTest);
        _gl.DepthMask(true);
        if (rt != null)
        {
            rt.NeedsResolve = true;
            rt.Dirty = true;
            rt.LastDrawFrame = _frame;
        }
        _count = 0;
    }

    DeferredBatch CaptureDeferredBatch(GlVertex[] vertices) => new(
        vertices,
        _kTarget,
        _kTransparent,
        _kDepthTest,
        _kDepthWrite,
        _kSourceDepthCompareWrite,
        _kMaterial,
        _kBlend,
        _kSetMask,
        _kCheckMask,
        _kTwAndX,
        _kTwAndY,
        _kTwOrX,
        _kTwOrY,
        _kClipX0,
        _kClipY0,
        _kClipX1,
        _kClipY1,
        _kTextureSmoothing);

    void ReplayDeferredBatches(List<DeferredBatch> batches)
    {
        if (batches.Count == 0)
            return;

        foreach (var batch in batches)
        {
            Debug.Assert(batch.Vertices.Length <= MaxVerts);
            batch.Vertices.CopyTo(_verts, 0);
            _count = batch.Vertices.Length;
            _kTarget = batch.Target;
            _kTransparent = batch.Transparent;
            _kDepthTest = batch.DepthTest;
            _kDepthWrite = batch.DepthWrite;
            _kSourceDepthCompareWrite = batch.SourceDepthCompareWrite;
            _kMaterial = batch.Material;
            _kBlend = batch.Blend;
            _kSetMask = batch.SetMask;
            _kCheckMask = batch.CheckMask;
            _kTwAndX = batch.TwAndX;
            _kTwAndY = batch.TwAndY;
            _kTwOrX = batch.TwOrX;
            _kTwOrY = batch.TwOrY;
            _kClipX0 = batch.ClipX0;
            _kClipY0 = batch.ClipY0;
            _kClipX1 = batch.ClipX1;
            _kClipY1 = batch.ClipY1;
            _kTextureSmoothing = batch.TextureSmoothing;
            Flush();
        }
        batches.Clear();
    }

    void SetBlend(float src, float dst) => _gl.Uniform4(_uBlend, src, src, src, dst);

    public void Present(in HleDispEnv disp) => PresentDisplay(disp.X, disp.Y, disp.W, disp.H, disp.Rgb24);

    public unsafe (uint tex, int w, int h, float aspect) PresentDisplay(int dispX, int dispY, int w, int h, bool rgb24 = false, int outW = 0, int outH = 0)
    {
        if (!Ready || w <= 0 || h <= 0) return (0, 0, 0, GpuHle.OutputAspect);
        long presentStarted = Stopwatch.GetTimestamp();
        if (TracePerformance && _traceLastPresentStarted != 0)
        {
            long interval = presentStarted - _traceLastPresentStarted;
            _traceFrameIntervalTicks += interval;
            _traceFrameIntervalMaxTicks =
                Math.Max(_traceFrameIntervalMaxTicks, interval);
            _traceFrameIntervals++;
        }
        _traceLastPresentStarted = presentStarted;
        if (TraceBackdropCoverage && _backdropTriangles > 0 &&
            _kTarget is { Margin: > 0 } backdropTraceTarget)
        {
            float targetLeft = -backdropTraceTarget.Margin;
            float targetRight = backdropTraceTarget.Wide1x -
                backdropTraceTarget.Margin;
            bool covered =
                _backdropMinX <= targetLeft + 0.5f &&
                _backdropMaxX >= targetRight - 0.5f;
            Console.Error.WriteLine(
                $"[BackdropCoverage] frame={_frame} " +
                $"triangles={_backdropTriangles} " +
                $"packets={string.Join(',', _backdropPackets.Order())} " +
                $"span={_backdropMinX:F3}..{_backdropMaxX:F3} " +
                $"target={targetLeft:F3}..{targetRight:F3} " +
                $"moved-left={_backdropMovedLeft} " +
                $"moved-right={_backdropMovedRight} " +
                $"wrong-side={_backdropMovedWrongSide} " +
                $"covered={(covered ? 1 : 0)}");
        }
        _backdropTriangles = 0;
        _backdropMinX = float.PositiveInfinity;
        _backdropMaxX = float.NegativeInfinity;
        _backdropMovedLeft = 0;
        _backdropMovedRight = 0;
        _backdropMovedWrongSide = 0;
        _backdropPackets.Clear();
        // A repeat present draws nothing; consuming the cell counters there
        // would drain them before the present that actually drew reports.
        if (TraceTerrainFrames && GpuHle.GameplayActive &&
            _terrainFrameWorldTriangles > 0)
        {
            var cells = Sdk.V82Compat.ConsumeTerrainCellCounts();
            var objCull = Sdk.V82Compat.ConsumeObjectCullCounts();
            var nclip = Gte.ConsumeNclipCorrections();
            var rtp = Gte.ConsumeRtpNearCounts();
            Console.Error.WriteLine(
                $"[TerrainFrame] frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                $"terrain={_terrainFrameTriangles} " +
                $"world={_terrainFrameWorldTriangles} " +
                $"cells={cells.Submitted} emitted={cells.Emitted} " +
                $"objL={_objectFrameLeft} objM={_objectFrameMid} " +
                $"objR={_objectFrameRight} " +
                $"objMinU={_objectFrameMinU:F3} objMaxU={_objectFrameMaxU:F3} " +
                $"satX={Gte.ConsumeSaturatedVertices()} " +
                $"satObj={Gte.ConsumeSaturatedObjectVertices()} " +
                $"nearReject={Gte.ConsumeDivideSaturations()} " +
                $"straddle={_straddleNearPlane} behind={_behindCamera} " +
                $"objTested={objCull.Tested} objCulled={objCull.Culled} " +
                $"nclipT={nclip.Terrain} nclipO={nclip.Object} " +
                $"rtp={rtp.Total} rtpNear100={rtp.Near100} " +
                $"flagReads={Gte.FlagRegisterReads} " +
                $"polyDrops[{Sdk.V82Compat.DescribePolygonDrops()}] " +
                $"nearPolys={Sdk.V82Compat.EmitterNearTotal} " +
                $"nearPolysDropped={Sdk.V82Compat.EmitterNearDropped} " +
                $"poolDropped={Sdk.V82Compat.PoolDropped} " +
                $"poolPeak={Sdk.V82Compat.PoolHighWater} " +
                $"orderingNearRemaps=" +
                $"{Sdk.V82Compat.GeometryOrderingNearDepthRemapped} " +
                $"flagErrors={Gte.FlagRegisterErrors} " +
                $"depths[{ConsumeNearDepthHistogram()}] " +
                $"rtpNear60={rtp.Near60} H={rtp.H} " +
                $"nclipRescued={nclip.Rescued}");
        }
        // Half the presents are 60Hz repeats that draw nothing. Flushing on
        // one writes an empty dump and consumes the request, so hold the dump
        // open until a present actually draws.
        if (_geometryDumping && (_geometryDump?.Length ?? 0) > 0)
        {
            string path =
                $"recompone_geometry_frame{_frame}.txt";
            string? dir = Environment.GetEnvironmentVariable(
                "RECOMPONE_CAPTURE_DIR");
            if (!string.IsNullOrWhiteSpace(dir))
            {
                dir = System.IO.Path.GetFullPath(dir);
                System.IO.Directory.CreateDirectory(dir);
                path = System.IO.Path.Combine(dir, path);
            }
            var census = Sdk.V82Compat.EndObjectCensus();
            var header = new System.Text.StringBuilder();
            var cm = Sdk.V82Compat.CamMatrix;
            header.Append("# camera world=")
                  .Append(Sdk.V82Compat.CamX).Append(',')
                  .Append(Sdk.V82Compat.CamY).Append(',')
                  .Append(Sdk.V82Compat.CamZ).Append(" matrix=");
            for (int i = 0; i < cm.Length; i++)
            {
                if (i > 0) header.Append(',');
                header.Append(cm[i]);
            }
            header.AppendLine();
            header.Append("# objects considered this frame: ")
                  .Append(census.Count)
                  .Append(" hookCalls=")
                  .Append(Sdk.V82Compat.CensusHookCalls)
                  .Append(" rejected=")
                  .Append(Sdk.V82Compat.CensusRejected)
                  .Append(" entries=")
                  .Append(Sdk.V82Compat.ObjectRenderEntries)
                  .Append(" exits=")
                  .Append(Sdk.V82Compat.ObjectRenderExits).AppendLine();
            foreach (var o in census)
                header.Append("# object ")
                      .Append(o.Address.ToString("X8")).Append(' ')
                      .Append(o.Outcome).Append(" world=")
                      .Append(o.X).Append(',').Append(o.Y).Append(',')
                      .Append(o.Z).Append(" radius=").Append(o.Radius)
                      .Append(" lastFrame=").Append(o.Frame)
                      .AppendLine();
            foreach (var r in Sdk.V82Compat.RejectedTriangles)
                header.Append("# rejected ")
                      .Append(r.Emitter.ToString("X8")).Append(' ')
                      .Append(r.X0).Append(',').Append(r.Y0).Append(',')
                      .Append(r.Z0).Append(' ')
                      .Append(r.X1).Append(',').Append(r.Y1).Append(',')
                      .Append(r.Z1).Append(' ')
                      .Append(r.X2).Append(',').Append(r.Y2).Append(',')
                      .Append(r.Z2).Append(" flags=")
                      .Append(r.Flags.ToString("X8")).Append(" mac0=")
                      .Append(r.Mac0).AppendLine();
            System.IO.File.WriteAllText(
                path, header.ToString() + (_geometryDump?.ToString() ?? ""));
            Sdk.V82Compat.RejectedTriangles.Clear();
            _geometryDumpIndex++;
            Console.WriteLine(
                $"[V82GeometryDump] frame={_frame} " +
                $"index={_geometryDumpIndex:D3} " +
                $"triangles={_terrainFrameWorldTriangles} wrote {path}");
            // Pair a picture with every dump. The artifact is episodic, so a
            // capture at a fixed point usually misses it; pairing lets the
            // frames the metric flags be pulled out as images afterwards.
            Host.HostWindow.RequestDisplayCapture(
                $"gameplay_{_geometryDumpIndex:D3}");
            _geometryDumping = false;
            _geometryDump = null;
        }
        else if (_geometryDumpRequested ||
                 (GeometryDumpFrame > 0 && _frame == GeometryDumpFrame) ||
                 (GeometryDumpEvery > 0 && GpuHle.GameplayActive &&
                  _terrainFrameWorldTriangles > 0 &&
                  // Count drawn frames rather than testing _frame: it advances
                  // by two during gameplay and can sit permanently on odd
                  // values, so a modulo of an even interval never fires.
                  (++_drawnFrameCounter % GeometryDumpEvery) == 0))
        {
            _geometryDumpRequested = false;
            _geometryDumping = true;
            _geometryDump = new System.Text.StringBuilder(1 << 20);
        }
        Sdk.V82Compat.CensusFrame = _frame;
        if (Gte.MarkRescuedNclip)
        {
            if ((_frame % 120) == 0)
                Console.Error.WriteLine(
                    $"[V82NclipMark] frame={_frame} " +
                    $"rescuedTriangles={Gte.RescuedTriangleCount} " +
                    $"markedPrimitives={Gte.MarkedPrimitives} " +
                    $"tested={Gte.TestedPrimitives} " +
                    $"candidates={Gte.CandidatePrimitives}");
            Gte.MarkedPrimitives = 0;
            Gte.TestedPrimitives = 0;
            Gte.CandidatePrimitives = 0;
            Gte.ClearNclipRescued();
        }
        _terrainFrameTriangles = 0;
        _terrainFrameWorldTriangles = 0;
        _objectFrameLeft = 0;
        _objectFrameMid = 0;
        _objectFrameRight = 0;
        _objectFrameMinU = 2f;
        _objectFrameMaxU = -1f;
        _straddleNearPlane = 0;
        _behindCamera = 0;
        ReportWorldGaps();
        ReportSeveredGeometry();
        ReportSegmentPops();
        if (TraceFog && GpuHle.GameplayActive &&
            (_frame < 8 || (_frame % 60) == 0))
            Console.Error.WriteLine(
                "[EnhancedFogFrame] " +
                $"frame={_frame} tick={GpuHle.DebugGameplayTick} " +
                $"valid={(_hasFogColor ? 1 : 0)} " +
                $"reset-frame={_fogResetFrame} " +
                $"selected-frame={_fogColorFrame} ot={_fogColorOt} " +
                $"span={_fogColorSpan:F1} " +
                $"near-white={(_fogColorNearWhite ? 1 : 0)} " +
                $"rgb={_fogColorR:F6},{_fogColorG:F6},{_fogColorB:F6}");
        if (TracePacketNclipCull && (_frame % 60) == 0)
        {
            Console.Error.WriteLine(
                $"[PacketNclipCull] frames={_frame - 59}-{_frame} " +
                $"front={_packetNclipFrontFaces} " +
                $"rescued={_packetNclipRescuedFaces} " +
                $"back={_packetNclipBackFaces} " +
                $"vehicle={_packetNclipVehicleFrontFaces}/" +
                    $"{_packetNclipVehicleRescuedFaces}/" +
                    $"{_packetNclipVehicleBackFaces} " +
                $"object={_packetNclipObjectFrontFaces}/" +
                    $"{_packetNclipObjectRescuedFaces}/" +
                    $"{_packetNclipObjectBackFaces} " +
                $"missing={_packetNclipMissing}");
            _packetNclipFrontFaces = 0;
            _packetNclipRescuedFaces = 0;
            _packetNclipBackFaces = 0;
            _packetNclipVehicleFrontFaces = 0;
            _packetNclipVehicleRescuedFaces = 0;
            _packetNclipVehicleBackFaces = 0;
            _packetNclipObjectFrontFaces = 0;
            _packetNclipObjectRescuedFaces = 0;
            _packetNclipObjectBackFaces = 0;
            _packetNclipMissing = 0;
        }
        FlushSelectorRenderTrace();
        _frame++;
        // Re-armed each frame the modal is drawn, so it follows the overlay
        // rather than lingering after it closes.
        if (GpuHle.NativeModalHold > 0) GpuHle.NativeModalHold--;
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
        if (TraceTriangleProbeContinuously &&
            GpuHle.GameplayActive &&
            (TraceTriangleProbeFrames is not { } probeFrames ||
             (_frame >= probeFrames.Start && _frame <= probeFrames.End)))
            foreach (string triangle in _pendingProbeTriangles)
                Console.Error.WriteLine(
                    $"[V8TriangleProbeContinuous] frame={_frame} {triangle}");
        _pendingProbeTriangles.Clear();
        Flush();
        ReplayDeferredBatches(_deferredLoadingPrompt);
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
                $"painter-world={_tracePainterOrderedWorldTriangles} " +
                $"painter-screen={_tracePainterOrderedScreenTriangles} " +
                $"world-transparent-depth=" +
                    $"{_traceTransparentWorldDepthTriangles} " +
                $"projective={_traceProjectiveTriangles} " +
                $"missing-ot={_traceMissingOtTriangles} " +
                $"ot-range={minOt}..{_traceMaxOt}");
            _traceOpaqueTriangles = 0;
            _traceTransparentTriangles = 0;
            _traceDepthTestedTriangles = 0;
            _tracePainterOrderedWorldTriangles = 0;
            _tracePainterOrderedScreenTriangles = 0;
            _traceTransparentWorldDepthTriangles = 0;
            _traceProjectiveTriangles = 0;
            _traceMissingOtTriangles = 0;
            _traceMinOt = int.MaxValue;
            _traceMaxOt = 0;
        }
        if (TraceConvertedSurfaces && (_frame % 60) == 0)
        {
            Console.Error.WriteLine(
                $"[ConvertedSurface] frames={_frame - 59}-{_frame} " +
                $"tick={GpuHle.DebugGameplayTick} " +
                $"triangles={_traceConvertedSurfaceTriangles} " +
                $"visible={_traceVisibleConvertedSurfaceTriangles} " +
                $"depth-tested={_traceDepthTestedConvertedSurfaceTriangles} " +
                $"projective={_traceProjectiveConvertedSurfaceTriangles} " +
                $"modern={_traceModernConvertedSurfaceTriangles} " +
                $"screen-area={_traceConvertedSurfaceArea:F3} " +
                $"max-area={_traceConvertedSurfaceMaxArea:F3}");
            _traceConvertedSurfaceTriangles = 0;
            _traceVisibleConvertedSurfaceTriangles = 0;
            _traceDepthTestedConvertedSurfaceTriangles = 0;
            _traceProjectiveConvertedSurfaceTriangles = 0;
            _traceModernConvertedSurfaceTriangles = 0;
            _traceConvertedSurfaceArea = 0.0;
            _traceConvertedSurfaceMaxArea = 0.0;
        }
        if (TraceEnhancedRenderer && (_frame % 60) == 0)
        {
            long viewSpaceTriangles =
                _traceDirectViewSpaceTriangles +
                _traceReconstructedViewSpaceTriangles;
            double viewSpaceCoverage = _traceEnhancedTriangles > 0
                ? 100.0 * viewSpaceTriangles / _traceEnhancedTriangles
                : 0.0;
            double directCoverage = _traceEnhancedTriangles > 0
                ? 100.0 * _traceDirectViewSpaceTriangles /
                    _traceEnhancedTriangles
                : 0.0;
            Console.Error.WriteLine(
                $"[EnhancedRenderer] frames={_frame - 59}-{_frame} " +
                $"triangles={_traceEnhancedTriangles} " +
                $"direct={_traceDirectViewSpaceTriangles} " +
                $"reconstructed={_traceReconstructedViewSpaceTriangles} " +
                $"fallback={_traceFallbackTriangles} " +
                $"world={_traceWorldTriangles} " +
                $"world-fallback={_traceWorldFallbackTriangles} " +
                $"visible-world={_traceVisibleWorldTriangles} " +
                $"visible-world-fallback=" +
                    $"{_traceVisibleWorldFallbackTriangles} " +
                $"vehicle={_traceVehicleTriangles} " +
                $"vehicle-fallback={_traceVehicleFallbackTriangles} " +
                $"visible-vehicle={_traceVisibleVehicleTriangles} " +
                $"visible-vehicle-fallback=" +
                    $"{_traceVisibleVehicleFallbackTriangles} " +
                $"opaque-fallback={_traceOpaqueFallbackTriangles} " +
                $"alpha-fallback={_traceAlphaTestFallbackTriangles} " +
                $"glass-fallback={_traceGlassFallbackTriangles} " +
                $"effect-fallback={_traceEffectFallbackTriangles} " +
                $"coverage={viewSpaceCoverage:F2}% " +
                $"direct-coverage={directCoverage:F2}% " +
                $"glass={_traceGlassTriangles} " +
                $"terrain-route={_traceTerrainRouteTriangles} " +
                $"route-opaque={_traceTerrainRouteOpaqueTriangles} " +
                $"route-transparent={_traceTerrainRouteTransparentTriangles} " +
                $"route-depth-write={_traceTerrainRouteDepthWriteTriangles} " +
                $"route-depth-test={_traceTerrainRouteDepthTestTriangles} " +
                $"route-depth-compare-write={_traceTerrainRouteDepthCompareWriteTriangles} " +
                $"route-source-textured={_traceTerrainRouteSourceTextured} " +
                $"route-source-coarse={_traceTerrainRouteSourceCoarse} " +
                $"route-source-transition={_traceTerrainRouteSourceTransition} " +
                $"route-source-unresolved={_traceTerrainRouteSourceUnresolved} " +
                $"modern-overspan={_traceModernOverspanTriangles} " +
                $"native-overspan-rejected=" +
                    $"{_traceNativeOverspanRejectedTriangles}");
            _traceEnhancedTriangles = 0;
            _traceDirectViewSpaceTriangles = 0;
            _traceReconstructedViewSpaceTriangles = 0;
            _traceFallbackTriangles = 0;
            _traceGlassTriangles = 0;
            _traceTerrainRouteTriangles = 0;
            _traceTerrainRouteOpaqueTriangles = 0;
            _traceTerrainRouteTransparentTriangles = 0;
            _traceTerrainRouteDepthWriteTriangles = 0;
            _traceTerrainRouteDepthTestTriangles = 0;
            _traceTerrainRouteDepthCompareWriteTriangles = 0;
            _traceTerrainRouteSourceTextured = 0;
            _traceTerrainRouteSourceCoarse = 0;
            _traceTerrainRouteSourceTransition = 0;
            _traceTerrainRouteSourceUnresolved = 0;
            _traceWorldTriangles = 0;
            _traceWorldFallbackTriangles = 0;
            _traceEffectFallbackTriangles = 0;
            _traceVisibleWorldTriangles = 0;
            _traceVisibleWorldFallbackTriangles = 0;
            _traceVehicleTriangles = 0;
            _traceVehicleFallbackTriangles = 0;
            _traceVisibleVehicleTriangles = 0;
            _traceVisibleVehicleFallbackTriangles = 0;
            _traceOpaqueFallbackTriangles = 0;
            _traceAlphaTestFallbackTriangles = 0;
            _traceGlassFallbackTriangles = 0;
            _traceModernOverspanTriangles = 0;
            _traceNativeOverspanRejectedTriangles = 0;
        }
        for (int i = 0; i < _rts.Length; i++)
        {
            if (_rts[i] is not { } rt) continue;
            if (_frame - rt.LastDrawFrame > 300)
            {
                if (rt.Dirty) Writeback(rt);
                rt.Destroy(_gl);
                _rts[i] = null;
            }
        }

        GlDisplayRt? src = null;
        bool preTickLoadingPresentation =
            GpuHle.GameplayActive &&
            GpuHle.DebugGameplayTick == 0;
        bool requireWideRt =
            GpuHle.GameplayActive &&
            !preTickLoadingPresentation &&
            GpuHle.WideAspect > GpuHle.BaseAspect + 0.001f;
        if (!rgb24)
            foreach (var rt in _rts)
            {
                if (rt == null) continue;
                if ((!GpuHle.GameplayActive || preTickLoadingPresentation) &&
                    rt.Margin > 0)
                    continue;
                if (requireWideRt && rt.Margin <= 0) continue;
                if (dispX < rt.X || dispY < rt.Y || dispX + w > rt.X + rt.W || dispY + h > rt.Y + rt.H) continue;
                if (src == null || rt.LastDrawFrame > src.LastDrawFrame) src = rt;
            }
        if (src == null && TracePerformance)
            _traceVramFallbackPresents++;
        else if (src is { Margin: > 0 } && TracePerformance)
            _traceWideRtPresents++;
        if (TracePresentationFrames && GpuHle.GameplayActive)
            Console.Error.WriteLine(
                $"[EnhancedPresentFrame] frame={_frame} " +
                $"tick={GpuHle.DebugGameplayTick} " +
                $"source={(src == null ? "vram" : "rt")} " +
                $"slot={(src == null ? -1 : Array.IndexOf(_rts, src))} " +
                $"xy={src?.X ?? dispX},{src?.Y ?? dispY} " +
                $"size={src?.W ?? w}x{src?.H ?? h} " +
                $"wide={src?.Wide1x ?? w}x{src?.H ?? h} " +
                $"last-draw={src?.LastDrawFrame ?? -1} " +
                $"capture={GpuHle.DebugCaptureLabel ?? "-"}");
        if (TracePerformance)
        {
            string source = src == null
                ? $"vram:{w}x{h}"
                : $"rt:{Array.IndexOf(_rts, src)}:{src.Wide1x}x{src.H}";
            string extent = src == null
                ? $"vram:{w}x{h}"
                : $"rt:{src.Wide1x}x{src.H}";
            if (_lastPresentationSource.Length != 0 &&
                !source.Equals(
                    _lastPresentationSource,
                    StringComparison.Ordinal))
                _tracePresentationSourceSwitches++;
            if (_lastPresentationExtent.Length != 0 &&
                !extent.Equals(
                    _lastPresentationExtent,
                    StringComparison.Ordinal))
                _tracePresentationExtentSwitches++;
            if (!extent.Equals(
                    _lastPresentationExtent,
                    StringComparison.Ordinal))
                Console.Error.WriteLine(
                    $"[EnhancedPresentExtent] frame={_frame} source={source} " +
                    $"extent={extent} " +
                    $"last-draw={src?.LastDrawFrame ?? -1} " +
                    $"gameplay={(GpuHle.GameplayActive ? 1 : 0)}");
            _lastPresentationSource = source;
            _lastPresentationExtent = extent;
        }
        if (src != null)
            Resolve(src);

        int w1x = src != null ? w + src.Margin * 2 : w;
        int h1x = h;
        float aspect = preTickLoadingPresentation
            ? GpuHle.BaseAspect
            : src is { Margin: > 0 }
                ? GpuHle.WideAspect
                : GpuHle.OutputAspect;


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
        if (TracePerformance)
        {
            long ticks = Stopwatch.GetTimestamp() - presentStarted;
            _tracePresentTicks += ticks;
            _tracePresentMaxTicks = Math.Max(_tracePresentMaxTicks, ticks);
            if ((_frame % 60) == 0)
            {
                var wide = Gte.ConsumeWidescreenProjectionMetrics();
                double tickMs = 1000.0 / Stopwatch.Frequency;
                double frameMeanMs = _traceFrameIntervals == 0
                    ? 0.0
                    : _traceFrameIntervalTicks * tickMs /
                        _traceFrameIntervals;
                double effectiveFps = frameMeanMs <= 0.0
                    ? 0.0
                    : 1000.0 / frameMeanMs;
                Console.Error.WriteLine(
                    $"[EnhancedPerformance] frames={_frame - 59}-{_frame} " +
                    $"frame-mean-ms={frameMeanMs:F3} " +
                    $"frame-max-ms={_traceFrameIntervalMaxTicks * tickMs:F3} " +
                    $"effective-fps={effectiveFps:F2} " +
                    $"present-mean-ms={_tracePresentTicks * tickMs / 60.0:F3} " +
                    $"present-max-ms={_tracePresentMaxTicks * tickMs:F3} " +
                    $"flushes={_traceFlushes} " +
                    $"msaa-resolves={_traceMsaaResolves} " +
                    $"check-mask-flushes={_traceCheckMaskFlushes} " +
                    $"set-mask-flushes={_traceSetMaskFlushes} " +
                    $"partial-writebacks={_tracePartialWritebacks} " +
                    $"partial-writeback-pixels={_tracePartialWritebackPixels} " +
                    $"present-reallocations={_tracePresentReallocations} " +
                    $"presentation-source-switches=" +
                        $"{_tracePresentationSourceSwitches} " +
                    $"presentation-extent-switches=" +
                        $"{_tracePresentationExtentSwitches} " +
                    $"wide-rt-presents={_traceWideRtPresents} " +
                    $"vram-fallback-presents={_traceVramFallbackPresents} " +
                    $"wide-projection-vertices={wide.Vertices} " +
                    $"wide-expanded-vertices={wide.ExpandedVertices}");
                _traceFlushes = 0;
                _traceMsaaResolves = 0;
                _tracePresentReallocations = 0;
                _traceVramFallbackPresents = 0;
                _traceWideRtPresents = 0;
                _tracePresentationSourceSwitches = 0;
                _tracePresentationExtentSwitches = 0;
                _tracePresentTicks = 0;
                _tracePresentMaxTicks = 0;
                _traceFrameIntervalTicks = 0;
                _traceFrameIntervalMaxTicks = 0;
                _traceFrameIntervals = 0;
                _traceCheckMaskFlushes = 0;
                _traceSetMaskFlushes = 0;
                _tracePartialWritebacks = 0;
                _tracePartialWritebackPixels = 0;
            }
        }
        return (_presentTex, fbW, fbH, aspect);
    }

    unsafe void EnsurePresentSize(int w, int h, bool nearest)
    {
        if (w == _presentW && h == _presentH && nearest == _presentNearest) return;
        if (TracePerformance)
            _tracePresentReallocations++;
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
        _textureReplacements?.Dispose();
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
