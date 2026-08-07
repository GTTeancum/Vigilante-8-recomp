using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hle;

namespace RecompOne.Runtime;

//old soft raster
public sealed partial class Gpu
{
    struct Vert
    {
        public uint SourceAddress;
        public int X, Y, RawX, RawY, R, G, B, U, V, Z;
        public float PerspectiveW, PreciseX, PreciseY;
        public float ViewX, ViewY, ViewZ;
        public float ProjectionCenterX, ProjectionCenterY, ProjectionScale;
        public bool HasGteZ, HasPreciseGteZ, HasCoherentGteZ;
        public bool HasProjectiveW, HasPrecisePosition, HasViewSpace;
        public bool ReconstructedViewSpace;
    }

    static readonly bool TraceProjection =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_PROJECTION") == "1";
    static readonly bool TraceSoftwareRectangles =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_SOFTWARE_RECTANGLES") == "1";
    static readonly (int X, int Y)? RasterPixelProbe =
        ParseRasterPixelProbe(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_TRACE_RASTER_PIXEL"));
    long _projectionQuadGte;
    long _projectionQuadAffine;
    long _projectionTriPrecise;
    long _projectionTriGte;
    long _projectionTriOtCorrelated;
    long _projectionTriAffine;
    long _projectionWideReconstructedVertices;
    long _projectionFullViewSpacePrimitives;
    long _projectionPartialViewSpaceFallbackPrimitives;
    long _projectionIncoherentViewSpaceFallbackPrimitives;
    int _projectionGameplayFrames;
    int _softwareRectangleTraceCount;
    readonly Dictionary<ulong, ProjectiveDepthCacheEntry>
        _projectiveDepthCache = [];

    readonly record struct ProjectiveDepthCacheEntry(
        ushort Depth, int OtDepth, uint PacketAddress);

    static (int X, int Y)? ParseRasterPixelProbe(string? value)
    {
        if (string.IsNullOrWhiteSpace(value))
            return null;
        string[] parts = value.Split(
            ',',
            StringSplitOptions.RemoveEmptyEntries |
            StringSplitOptions.TrimEntries);
        return parts.Length == 2 &&
            int.TryParse(parts[0], out int x) &&
            int.TryParse(parts[1], out int y)
                ? (x, y)
                : null;
    }

    static bool RasterPixelProbeMatches(int x, int y)
    {
        if (RasterPixelProbe is not { } probe || x != probe.X)
            return false;
        // V8 alternates native 320x240 draw buffers at VRAM Y=0 and Y=240.
        // The probe is expressed in display coordinates and follows either
        // buffer so both halves of the renderer's frame cadence are audited.
        return y == probe.Y || y == probe.Y + 240;
    }

    static bool RasterPixelProbeTickEnabled()
    {
        if (RasterPixelProbe is null || !GpuHle.GameplayActive)
            return false;
        int tick = GpuHle.DebugGameplayTick;
        return TraceGameplayTicks is not { } range ||
            tick >= range.Start && tick <= range.End;
    }

    void TraceRasterPixel(
        int x,
        int y,
        in Vert a,
        in Vert b,
        in Vert c,
        bool tex,
        bool semi,
        bool raw,
        int clut,
        int u,
        int v,
        ushort texel,
        ushort before,
        ushort after,
        string outcome)
    {
        Console.Error.WriteLine(
            $"[V8RasterPixel] tick={GpuHle.DebugGameplayTick} " +
            $"xy={x},{y} packet=0x{_currentOtPacketAddress:X8} " +
            $"ot={_currentOtDepth} outcome={outcome} " +
            $"tex={(tex ? 1 : 0)} semi={(semi ? 1 : 0)} " +
            $"raw={(raw ? 1 : 0)} tpage=0x{CurTPage():X3} " +
            $"clut=0x{clut:X4} uv={u},{v} texel=0x{texel:X4} " +
            $"before=0x{before:X4} after=0x{after:X4} " +
            $"tri=({a.X},{a.Y})({b.X},{b.Y})({c.X},{c.Y}) " +
            $"tri-uv=({a.U},{a.V})({b.U},{b.V})({c.U},{c.V}) " +
            $"rgb=({a.R},{a.G},{a.B})({b.R},{b.G},{b.B})" +
            $"({c.R},{c.G},{c.B})");
    }

    static readonly int[,] Dither =
    {
        { -4,  0, -3,  1 },
        {  2, -2,  3, -1 },
        { -3,  1, -4,  0 },
        {  3, -1,  2, -2 },
    };

    void DrawPolygon()
    {
        uint cmd = _fifo[0];
        bool gouraud = (cmd & (1u << 28)) != 0;
        bool quad = (cmd & (1u << 27)) != 0;
        bool tex = (cmd & (1u << 26)) != 0;
        bool semi = (cmd & (1u << 25)) != 0;
        bool raw = (cmd & (1u << 24)) != 0;
        int n = quad ? 4 : 3;

        Span<Vert> v = stackalloc Vert[4];
        int idx = 1;
        int clut = 0;
        int cr = (int)(cmd & 0xFF), cg = (int)((cmd >> 8) & 0xFF), cb = (int)((cmd >> 16) & 0xFF);

        for (int i = 0; i < n; i++)
        {
            if (gouraud && i > 0)
            {
                uint cw = _fifo[idx++];
                cr = (int)(cw & 0xFF); cg = (int)((cw >> 8) & 0xFF); cb = (int)((cw >> 16) & 0xFF);
            }
            v[i].R = cr; v[i].G = cg; v[i].B = cb;

            int vertexWordIndex = idx;
            uint vw = _fifo[idx++];
            int rawX = CoordX(vw);
            int rawY = CoordY(vw);
            v[i].RawX = rawX;
            v[i].RawY = rawY;
            v[i].X = _drawOffsetX + rawX;
            v[i].Y = _drawOffsetY + rawY;
            uint sourceAddress = FifoSource(vertexWordIndex);
            v[i].SourceAddress = sourceAddress;
            if (sourceAddress != UnknownFifoSource &&
                Runtime.Mem != null &&
                Runtime.Mem.TryGetPreciseGteVertex(
                    sourceAddress, vw,
                    out var precise))
            {
                v[i].Z = precise.Depth;
                v[i].PerspectiveW = precise.PerspectiveW;
                v[i].PreciseX = _drawOffsetX + precise.PreciseX;
                v[i].PreciseY = _drawOffsetY + precise.PreciseY;
                v[i].ViewX = precise.ViewX;
                v[i].ViewY = precise.ViewY;
                v[i].ViewZ = precise.ViewZ;
                v[i].ProjectionCenterX =
                    _drawOffsetX + precise.ProjectionCenterX;
                v[i].ProjectionCenterY =
                    _drawOffsetY + precise.ProjectionCenterY;
                v[i].ProjectionScale = precise.ProjectionScale;
                v[i].HasGteZ = true;
                v[i].HasPreciseGteZ = true;
                v[i].HasPrecisePosition =
                    MathF.Abs(precise.PreciseX - rawX) <= 1.5f &&
                    MathF.Abs(precise.PreciseY - rawY) <= 1.5f;
                v[i].HasViewSpace = precise.Valid;
            }
            else if (Gte.TryGetScreenDepth(
                    rawX, rawY, _currentOtDepth, out ushort z) ||
                Gte.TryGetScreenDepth(
                    v[i].X, v[i].Y, _currentOtDepth, out z))
            {
                v[i].Z = z;
                v[i].HasGteZ = true;
            }

            if (tex)
            {
                uint uvw = _fifo[idx++];
                v[i].U = (int)(uvw & 0xFF);
                v[i].V = (int)((uvw >> 8) & 0xFF);
                if (i == 0) clut = (int)((uvw >> 16) & 0xFFFF);
                else if (i == 1) SetTexpageFromWord((uvw >> 16) & 0xFFFF);
            }
        }

        bool coherentPrecisePosition = true;
        for (int i = 0; i < n; i++)
            coherentPrecisePosition &= v[i].HasPrecisePosition;
        if (!coherentPrecisePosition)
            for (int i = 0; i < n; i++)
                v[i].HasPrecisePosition = false;

        if (!quad &&
            !(v[0].HasPreciseGteZ &&
              v[1].HasPreciseGteZ &&
              v[2].HasPreciseGteZ))
            TryRecoverCoherentTriangleDepth(v);

        if (HleOn && !tex && !quad &&
            !(v[0].HasGteZ && v[1].HasGteZ && v[2].HasGteZ) &&
            TryRecoverOtConstrainedTriangleDepth(
                v, clut: 0, textured: false,
                out ushort flatZ0, out ushort flatZ1, out ushort flatZ2))
        {
            SetCoherentTriangleDepth(v, flatZ0, flatZ1, flatZ2);
        }

        if (tex && quad && ConfigManager.View.PerspectiveCorrectTextures)
        {
            // Emulator-grade PGXP behavior: use perspective W only when every
            // packet vertex retains a validated RAM-address/GTE association.
            // Never infer W from the quad's screen shape; that approximation
            // changes as the camera turns and recreates the visible swimming.
            if (TryProjectiveQuadGteW(v))
            {
                _projectionQuadGte++;
            }
            else
            {
                _projectionQuadAffine++;
            }
        }
        else if (tex && !quad && ConfigManager.View.PerspectiveCorrectTextures)
        {
            if (TryProjectiveTriangleW(v, clut, out bool grouped))
            {
                if (v[0].HasPreciseGteZ &&
                    v[1].HasPreciseGteZ &&
                    v[2].HasPreciseGteZ)
                    _projectionTriPrecise++;
                else if (grouped)
                    _projectionTriGte++;
                else
                    _projectionTriOtCorrelated++;
            }
            else
                _projectionTriAffine++;
        }

        if (Gte.MarkRescuedNclip && GpuHle.GameplayActive)
            Gte.CandidatePrimitives++;
        if (Gte.MarkRescuedNclip &&
            GpuHle.GameplayActive &&
            v[0].HasViewSpace && v[1].HasViewSpace &&
            v[2].HasViewSpace &&
            v[0].ViewZ > 0f && v[1].ViewZ > 0f && v[2].ViewZ > 0f)
        {
            // Proof aid, not a game feature. Recompute the backface sign the
            // way the engine did - from the widescreen-compressed integer
            // coordinates - and the way it is computed now, from the
            // fractional projection. Where they disagree and the fractional
            // one says front-facing, this primitive is on screen only because
            // of the NCLIP fix; before it, the rounded test deleted it.
            Gte.TestedPrimitives++;
            long packedArea =
                (long)v[0].RawX * (v[1].RawY - v[2].RawY) +
                (long)v[1].RawX * (v[2].RawY - v[0].RawY) +
                (long)v[2].RawX * (v[0].RawY - v[1].RawY);
            Span<double> px = stackalloc double[3];
            Span<double> py = stackalloc double[3];
            for (int i = 0; i < 3; i++)
            {
                px[i] = v[i].ProjectionCenterX +
                    v[i].ViewX * v[i].ProjectionScale / v[i].ViewZ;
                py[i] = v[i].ProjectionCenterY +
                    v[i].ViewY * v[i].ProjectionScale / v[i].ViewZ;
            }
            double preciseArea =
                px[0] * (py[1] - py[2]) +
                px[1] * (py[2] - py[0]) +
                px[2] * (py[0] - py[1]);
            if ((packedArea > 0) != (preciseArea > 0d))
            {
                for (int i = 0; i < n; i++)
                {
                    v[i].R = 255; v[i].G = 32; v[i].B = 32;
                }
                tex = false;
                gouraud = false;
                raw = false;
                Gte.MarkedPrimitives++;
            }
        }

        if (HleOn)
        {
            PopulateEnhancedViewSpace(v, n);
            NormalizeEnhancedPrimitiveViewSpace(v, n);
            HleTri(v[0], v[1], v[2], tex, gouraud, semi, raw, clut);
            if (quad) HleTri(v[1], v[2], v[3], tex, gouraud, semi, raw, clut);
        }
        else
        {
            RasterTriangle(v[0], v[1], v[2], tex, gouraud, semi, raw, clut);
            if (quad) RasterTriangle(v[1], v[2], v[3], tex, gouraud, semi, raw, clut);
        }
    }

    void PopulateEnhancedViewSpace(Span<Vert> vertices, int count)
    {
        if (!GpuHle.GameplayActive)
            return;
        if (!Gte.TryGetProjectionState(
                out float centerX,
                out float centerY,
                out float projectionScale))
            return;

        centerX += _drawOffsetX;
        centerY += _drawOffsetY;
        for (int index = 0; index < count; index++)
        {
            ref Vert vertex = ref vertices[index];
            if (vertex.HasViewSpace)
                continue;

            float viewZ =
                vertex.HasProjectiveW &&
                float.IsFinite(vertex.PerspectiveW) &&
                vertex.PerspectiveW > 0
                    ? vertex.PerspectiveW
                    : vertex.HasGteZ && vertex.Z > 0
                        ? vertex.Z
                        : 0f;
            if (!float.IsFinite(viewZ) || viewZ <= 0)
                continue;

            float screenX =
                vertex.HasPrecisePosition ? vertex.PreciseX : vertex.X;
            float screenY =
                vertex.HasPrecisePosition ? vertex.PreciseY : vertex.Y;
            if (ConfigManager.View.HighResolution3D &&
                GpuHle.WideAspect > GpuHle.BaseAspect + 0.001f)
            {
                // GTE.Rtp deliberately compresses the retail-visible SXY
                // result so the original 320-pixel clip tests retain geometry
                // from the expanded widescreen frustum. Exact correlated
                // vertices carry the uncompressed camera-space position.
                // Undo that known compression before reconstructing a missing
                // vertex, or one triangle can mix two projection spaces and
                // stretch into jagged terrain/water-like overlays.
                float inverseWideRatio =
                    GpuHle.WideAspect / GpuHle.BaseAspect;
                screenX =
                    centerX + (screenX - centerX) * inverseWideRatio;
                _projectionWideReconstructedVertices++;
            }
            vertex.ViewX =
                (screenX - centerX) * viewZ / projectionScale;
            vertex.ViewY =
                (screenY - centerY) * viewZ / projectionScale;
            vertex.ViewZ = viewZ;
            vertex.ProjectionCenterX = centerX;
            vertex.ProjectionCenterY = centerY;
            vertex.ProjectionScale = projectionScale;
            vertex.PerspectiveW = viewZ;
            vertex.HasProjectiveW = true;
            vertex.HasViewSpace =
                float.IsFinite(vertex.ViewX) &&
                float.IsFinite(vertex.ViewY);
            vertex.ReconstructedViewSpace = vertex.HasViewSpace;
        }
    }

    void NormalizeEnhancedPrimitiveViewSpace(
        Span<Vert> vertices,
        int count)
    {
        if (!GpuHle.GameplayActive || count <= 0)
            return;

        int viewSpaceCount = 0;
        for (int index = 0; index < count; index++)
            if (vertices[index].HasViewSpace)
                viewSpaceCount++;

        if (viewSpaceCount == 0)
            return;

        bool coherent = viewSpaceCount == count;
        if (coherent)
        {
            float centerX = vertices[0].ProjectionCenterX;
            float centerY = vertices[0].ProjectionCenterY;
            float scale = vertices[0].ProjectionScale;
            for (int index = 1; index < count; index++)
            {
                ref Vert vertex = ref vertices[index];
                if (MathF.Abs(vertex.ProjectionCenterX - centerX) > 1.5f ||
                    MathF.Abs(vertex.ProjectionCenterY - centerY) > 1.5f ||
                    MathF.Abs(vertex.ProjectionScale - scale) > 1.5f)
                {
                    coherent = false;
                    break;
                }
            }
        }

        if (coherent)
        {
            _projectionFullViewSpacePrimitives++;
            return;
        }

        if (viewSpaceCount != count)
            _projectionPartialViewSpaceFallbackPrimitives++;
        else
            _projectionIncoherentViewSpaceFallbackPrimitives++;

        // Projection is selected by the vertex shader per endpoint. A
        // primitive must never combine camera-space and packet screen-space
        // vertices. Preserve the native packet as one coherent fallback when
        // a complete camera-space primitive cannot be established.
        for (int index = 0; index < count; index++)
        {
            vertices[index].HasViewSpace = false;
            vertices[index].ReconstructedViewSpace = false;
        }
    }

    public void EndProjectionFrame(bool gameplayActive)
    {
        if (!TraceProjection) return;
        if (!gameplayActive)
        {
            ResetProjectionCounters();
            return;
        }

        _projectionGameplayFrames++;
        if ((_projectionGameplayFrames % 60) != 0) return;
        Console.Error.WriteLine(
            $"[Projection] frames={_projectionGameplayFrames - 59}-" +
            $"{_projectionGameplayFrames} quad-gte={_projectionQuadGte} " +
            $"quad-affine={_projectionQuadAffine} tri-gte={_projectionTriGte} " +
            $"tri-precise={_projectionTriPrecise} " +
            $"tri-ot={_projectionTriOtCorrelated} " +
            $"tri-affine={_projectionTriAffine} " +
            $"wide-reconstructed-vertices=" +
            $"{_projectionWideReconstructedVertices} " +
            $"full-viewspace-primitives={_projectionFullViewSpacePrimitives} " +
            $"partial-viewspace-fallback-primitives=" +
            $"{_projectionPartialViewSpaceFallbackPrimitives} " +
            $"incoherent-viewspace-fallback-primitives=" +
            $"{_projectionIncoherentViewSpaceFallbackPrimitives} " +
            $"gte-points={Gte.ScreenDepthCount}");
        ResetProjectionCounters();
    }

    void ResetProjectionCounters()
    {
        _projectionQuadGte = 0;
        _projectionQuadAffine = 0;
        _projectionTriPrecise = 0;
        _projectionTriGte = 0;
        _projectionTriOtCorrelated = 0;
        _projectionTriAffine = 0;
        _projectionWideReconstructedVertices = 0;
        _projectionFullViewSpacePrimitives = 0;
        _projectionPartialViewSpaceFallbackPrimitives = 0;
        _projectionIncoherentViewSpaceFallbackPrimitives = 0;
    }

    bool TryProjectiveQuadGteW(Span<Vert> v)
    {
        if (!HasValidPreciseDepths(v, 4))
            return false;

        for (int i = 0; i < 4; i++)
        {
            v[i].HasProjectiveW = true;
        }
        return true;
    }

    bool HasValidPreciseDepths(Span<Vert> v, int count)
    {
        for (int i = 0; i < count; i++)
        {
            if (!v[i].HasPreciseGteZ ||
                v[i].Z <= 0 ||
                !float.IsFinite(v[i].PerspectiveW) ||
                v[i].PerspectiveW <= 0)
                return false;
        }
        return true;
    }

    bool TryRecoverCoherentTriangleDepth(Span<Vert> v)
    {
        bool recovered = Gte.TryGetGroupedTriangleScreenDepth(
            v[0].RawX, v[0].RawY,
            v[1].RawX, v[1].RawY,
            v[2].RawX, v[2].RawY,
            _currentOtDepth,
            out ushort z0, out ushort z1, out ushort z2);
        if (!recovered)
            recovered = Gte.TryGetGroupedTriangleScreenDepth(
                v[0].X, v[0].Y,
                v[1].X, v[1].Y,
                v[2].X, v[2].Y,
                _currentOtDepth,
                out z0, out z1, out z2);
        if (!recovered)
            return false;

        SetCoherentTriangleDepth(v, z0, z1, z2);
        return true;
    }

    static void SetCoherentTriangleDepth(
        Span<Vert> v, ushort z0, ushort z1, ushort z2)
    {
        v[0].Z = z0;
        v[1].Z = z1;
        v[2].Z = z2;
        v[0].HasGteZ = v[0].HasCoherentGteZ = true;
        v[1].HasGteZ = v[1].HasCoherentGteZ = true;
        v[2].HasGteZ = v[2].HasCoherentGteZ = true;
    }

    bool TryProjectiveTriangleW(Span<Vert> v, int clut, out bool grouped)
    {
        grouped = false;
        bool allPrecise =
            v[0].HasPreciseGteZ &&
            v[1].HasPreciseGteZ &&
            v[2].HasPreciseGteZ;
        bool precise = allPrecise && HasValidPreciseDepths(v, 3);
        // Match the emulator's primitive-wide fallback: do not replace a
        // rejected exact-address set with a second, screen-coordinate guess.
        if (allPrecise && !precise)
            return false;
        grouped = precise ||
            v[0].HasCoherentGteZ &&
            v[1].HasCoherentGteZ &&
            v[2].HasCoherentGteZ;
        bool recovered = grouped;
        ushort z0 = checked((ushort)v[0].Z);
        ushort z1 = checked((ushort)v[1].Z);
        ushort z2 = checked((ushort)v[2].Z);
        if (!recovered)
        {
            // Do not combine independent screen-coordinate lookups. A busy
            // scene can project unrelated surfaces through the same pixel
            // over the retained GTE generations, and mixing those depths
            // makes vertical textures swim as the camera moves. Recover the
            // complete triangle from one RTPT group or one tightly consecutive
            // RTPS run instead.
            recovered = Gte.TryGetTriangleScreenDepth(
                v[0].RawX, v[0].RawY,
                v[1].RawX, v[1].RawY,
                v[2].RawX, v[2].RawY,
                _currentOtDepth,
                out z0, out z1, out z2);
            if (!recovered)
                recovered = Gte.TryGetTriangleScreenDepth(
                    v[0].X, v[0].Y,
                    v[1].X, v[1].Y,
                    v[2].X, v[2].Y,
                    _currentOtDepth,
                    out z0, out z1, out z2);
            if (!recovered)
            {
                // V8 also emits mesh triangles after projecting cached
                // vertices one at a time. Those projections can be separated
                // by enough unrelated GTE work that they are not one strict
                // RTPS run. The packet's OTZ is AVSZ3 (average SZ / 8), so it
                // gives us an independent coherence check and, when exactly
                // one sample is absent, the correct missing-depth equation:
                //
                //     missing SZ = 3 * (OTZ * 8) - known SZ0 - known SZ1
                //
                // Previously the missing vertex received the average itself.
                // That bends reciprocal depth across the triangle and is the
                // source of the visible affine-like swimming on buildings.
                recovered = TryRecoverOtConstrainedTriangleDepth(
                    v, clut, textured: true, out z0, out z1, out z2);
            }
            if (!recovered)
                return false;
        }

        if (!precise)
        {
            v[0].PerspectiveW = z0;
            v[1].PerspectiveW = z1;
            v[2].PerspectiveW = z2;
        }
        v[0].HasProjectiveW = true;
        v[1].HasProjectiveW = true;
        v[2].HasProjectiveW = true;
        RememberProjectiveTriangleDepth(v, clut, z0, z1, z2);
        return true;
    }

    bool TryRecoverOtConstrainedTriangleDepth(
        Span<Vert> v, int clut, bool textured,
        out ushort z0, out ushort z1, out ushort z2)
    {
        z0 = z1 = z2 = 0;
        // Enhanced is a separate renderer, so both 4-bit model/building
        // textures and 8-bit terrain/world textures use the same
        // primitive-wide recovery contract.  This remains conservative:
        // accept only two exact or same-material cached depths and derive the
        // third from the packet's native AVSZ3 ordering-table value.  Stock
        // never enters this path because perspective correction is disabled.
        if (_currentOtDepth <= 0 ||
            (textured && _texDepth is not (0 or 1)))
            return false;

        int expected = Math.Clamp(_currentOtDepth * 8, 1, ushort.MaxValue);
        Span<int> depth = stackalloc int[3];
        int present = 0;
        long knownSum = 0;
        for (int i = 0; i < 3; i++)
        {
            if (v[i].HasGteZ && v[i].Z > 0)
                depth[i] = v[i].Z;
            else if (!textured ||
                     !TryGetCachedProjectiveDepth(
                         v[i], clut, expected, out depth[i]))
                continue;
            present++;
            knownSum += depth[i];
        }

        if (present < 2)
            return false;

        // OT bins are only eight camera-space units wide, but the game's
        // ordering-table setup and near-plane saturation introduce a small
        // bias. Keep the mean check tight enough to reject samples from an
        // overlapping object while allowing the native fixed-point rounding.
        int meanTolerance = Math.Max(96, expected / 8);
        if (present == 3)
        {
            int mean = (int)((knownSum + 1) / 3);
            if (Math.Abs(mean - expected) > meanTolerance)
                return false;
        }
        else
        {
            long inferred = 3L * expected - knownSum;
            if (inferred is <= 0 or > ushort.MaxValue)
                return false;
            for (int i = 0; i < 3; i++)
            {
                if (!v[i].HasGteZ)
                {
                    depth[i] = (int)inferred;
                    break;
                }
            }
        }

        int minimum = Math.Min(depth[0], Math.Min(depth[1], depth[2]));
        int maximum = Math.Max(depth[0], Math.Max(depth[1], depth[2]));
        if (minimum <= 0 || maximum > minimum * 8)
            return false;

        // Every accepted sample must remain plausible for this OT bucket.
        // The slope allowance is deliberately wider than the mean allowance:
        // a wall viewed obliquely legitimately spans a broad depth range.
        int vertexTolerance = Math.Max(512, expected);
        for (int i = 0; i < 3; i++)
            if (Math.Abs(depth[i] - expected) > vertexTolerance)
                return false;

        z0 = checked((ushort)depth[0]);
        z1 = checked((ushort)depth[1]);
        z2 = checked((ushort)depth[2]);
        return true;
    }

    ulong ProjectiveDepthCacheKey(in Vert vertex, int clut) =>
        (ushort)vertex.RawX |
        ((ulong)(ushort)vertex.RawY << 16) |
        ((ulong)(ushort)clut << 32) |
        ((ulong)(ushort)CurTPage() << 48);

    bool TryGetCachedProjectiveDepth(
        in Vert vertex, int clut, int expected, out int depth)
    {
        depth = 0;
        if (!_projectiveDepthCache.TryGetValue(
                ProjectiveDepthCacheKey(vertex, clut), out var entry))
            return false;

        long packetDistance = Math.Abs(
            (long)_currentOtPacketAddress - entry.PacketAddress);
        int otTolerance = Math.Max(64, _currentOtDepth / 4);
        int depthTolerance = Math.Max(512, expected);
        if (_currentOtPacketAddress == 0 || entry.PacketAddress == 0 ||
            packetDistance > 0x1000 ||
            Math.Abs(_currentOtDepth - entry.OtDepth) > otTolerance ||
            Math.Abs(entry.Depth - expected) > depthTolerance)
            return false;

        depth = entry.Depth;
        return true;
    }

    void RememberProjectiveTriangleDepth(
        Span<Vert> v, int clut, ushort z0, ushort z1, ushort z2)
    {
        if (_texDepth is not (0 or 1) || _currentOtPacketAddress == 0)
            return;
        Span<ushort> depth = stackalloc ushort[3] { z0, z1, z2 };
        for (int i = 0; i < 3; i++)
            _projectiveDepthCache[ProjectiveDepthCacheKey(v[i], clut)] =
                new(depth[i], _currentOtDepth, _currentOtPacketAddress);
    }

    void RasterTriangle(Vert a, Vert b, Vert c, bool tex, bool gouraud, bool semi, bool raw, int clut)
    {
        int spanX = Math.Max(a.X, Math.Max(b.X, c.X)) - Math.Min(a.X, Math.Min(b.X, c.X));
        int spanY = Math.Max(a.Y, Math.Max(b.Y, c.Y)) - Math.Min(a.Y, Math.Min(b.Y, c.Y));
        if (spanX > 1023 || spanY > 511) return;

        long area = (long)(b.X - a.X) * (c.Y - a.Y) - (long)(b.Y - a.Y) * (c.X - a.X);
        if (area == 0) return;
        if (area < 0) { (b, c) = (c, b); area = -area; }

        int minX = Math.Max(_drawAreaLeft, Math.Min(a.X, Math.Min(b.X, c.X)));
        int maxX = Math.Min(_drawAreaRight, Math.Max(a.X, Math.Max(b.X, c.X)));
        int minY = Math.Max(_drawAreaTop, Math.Min(a.Y, Math.Min(b.Y, c.Y)));
        int maxY = Math.Min(_drawAreaBottom, Math.Max(a.Y, Math.Max(b.Y, c.Y)));
        if (minX > maxX || minY > maxY) return;

        if (tex && semi)
            TraceImportedShadowSourceSoftware(a, b, c, clut);

        int bias0 = IsTopLeft(b, c) ? 0 : -1;
        int bias1 = IsTopLeft(c, a) ? 0 : -1;
        int bias2 = IsTopLeft(a, b) ? 0 : -1;
        bool ditherTex = DitherEnabled && !raw;

        int sx0 = b.Y - c.Y, sy0 = c.X - b.X;
        int sx1 = c.Y - a.Y, sy1 = a.X - c.X;
        int sx2 = a.Y - b.Y, sy2 = b.X - a.X;

        long w0Row = (long)(c.X - b.X) * (minY - b.Y) - (long)(c.Y - b.Y) * (minX - b.X);
        long w1Row = (long)(a.X - c.X) * (minY - c.Y) - (long)(a.Y - c.Y) * (minX - c.X);
        long w2Row = (long)(b.X - a.X) * (minY - a.Y) - (long)(b.Y - a.Y) * (minX - a.X);

        for (int y = minY; y <= maxY; y++, w0Row += sy0, w1Row += sy1, w2Row += sy2)
        {
            long w0 = w0Row, w1 = w1Row, w2 = w2Row;
            for (int x = minX; x <= maxX; x++, w0 += sx0, w1 += sx1, w2 += sx2)
            {
                if (w0 + bias0 < 0 || w1 + bias1 < 0 || w2 + bias2 < 0) continue;

                int r, g, bl;
                if (gouraud)
                {
                    r = (int)((w0 * a.R + w1 * b.R + w2 * c.R) / area);
                    g = (int)((w0 * a.G + w1 * b.G + w2 * c.G) / area);
                    bl = (int)((w0 * a.B + w1 * b.B + w2 * c.B) / area);
                }
                else { r = a.R; g = a.G; bl = a.B; }

                if (tex)
                {
                    int u = (int)((w0 * a.U + w1 * b.U + w2 * c.U) / area);
                    int tv = (int)((w0 * a.V + w1 * b.V + w2 * c.V) / area);
                    ushort texel = FetchTexel(u, tv, clut);
                    bool tracePixel =
                        RasterPixelProbeTickEnabled() &&
                        RasterPixelProbeMatches(x, y);
                    ushort before = tracePixel
                        ? Vram[y * VramWidth + x]
                        : (ushort)0;
                    if (texel == 0)
                    {
                        if (tracePixel)
                            TraceRasterPixel(
                                x, y, a, b, c, tex, semi, raw, clut,
                                u, tv, texel, before, before,
                                "transparent-texel");
                        continue;
                    }
                    bool stp = (texel & 0x8000) != 0;
                    int tr = (texel & 0x1F) << 3, tg = ((texel >> 5) & 0x1F) << 3, tb = ((texel >> 10) & 0x1F) << 3;
                    if (!raw) { tr = tr * r >> 7; tg = tg * g >> 7; tb = tb * bl >> 7; }
                    Plot(x, y, tr, tg, tb, semi && stp, ditherTex, stp);
                    if (tracePixel)
                        TraceRasterPixel(
                            x, y, a, b, c, tex, semi, raw, clut,
                            u, tv, texel, before,
                            Vram[y * VramWidth + x],
                            "write");
                }
                else
                {
                    bool tracePixel =
                        RasterPixelProbeTickEnabled() &&
                        RasterPixelProbeMatches(x, y);
                    ushort before = tracePixel
                        ? Vram[y * VramWidth + x]
                        : (ushort)0;
                    Plot(x, y, r, g, bl, semi, DitherEnabled && gouraud);
                    if (tracePixel)
                        TraceRasterPixel(
                            x, y, a, b, c, tex, semi, raw, clut,
                            0, 0, 0, before,
                            Vram[y * VramWidth + x],
                            "write");
                }
            }
        }
    }

    static bool IsTopLeft(in Vert p0, in Vert p1)
    {
        int dy = p1.Y - p0.Y, dx = p1.X - p0.X;
        return dy < 0 || (dy == 0 && dx > 0);
    }

    void DrawRectangle()
    {
        uint cmd = _fifo[0];
        int sz = (int)((cmd >> 27) & 3);
        bool tex = (cmd & (1u << 26)) != 0;
        bool semi = (cmd & (1u << 25)) != 0;
        bool raw = (cmd & (1u << 24)) != 0;
        int cr = (int)(cmd & 0xFF), cg = (int)((cmd >> 8) & 0xFF), cb = (int)((cmd >> 16) & 0xFF);

        int idx = 1;
        uint vw = _fifo[idx++];
        int x = _drawOffsetX + CoordX(vw);
        int y = _drawOffsetY + CoordY(vw);

        int u0 = 0, v0 = 0, clut = 0;
        if (tex)
        {
            uint uvw = _fifo[idx++];
            u0 = (int)(uvw & 0xFF); v0 = (int)((uvw >> 8) & 0xFF);
            clut = (int)((uvw >> 16) & 0xFFFF);
        }

        int w, h;
        if (sz == 0) { uint wh = _fifo[idx]; w = (int)(wh & 0xFFFF); h = (int)((wh >> 16) & 0xFFFF); }
        else { w = h = sz == 1 ? 1 : sz == 2 ? 8 : 16; }

        if (TraceSoftwareRectangles &&
            GpuHle.GameplayActive &&
            _softwareRectangleTraceCount++ < 8192)
            Console.Error.WriteLine(
                "[SoftwareRect] " +
                $"tick={GpuHle.DebugGameplayTick} " +
                $"packet=0x{_currentOtPacketAddress:X8} " +
                $"ot={_currentOtDepth} xy={x},{y} wh={w}x{h} " +
                $"uv={u0},{v0} tex={(tex ? 1 : 0)} " +
                $"semi={(semi ? 1 : 0)} raw={(raw ? 1 : 0)} " +
                $"rgb={cr},{cg},{cb} tpage=0x{CurTPage():X3} " +
                $"clut=0x{clut:X4}");

        if (HleOn) { HleRect(x, y, w, h, u0, v0, clut, cr, cg, cb, tex, semi, raw); return; }

        for (int dy = 0; dy < h; dy++)
            for (int dx = 0; dx < w; dx++)
            {
                int px = x + dx, py = y + dy;
                if (px < _drawAreaLeft || px > _drawAreaRight || py < _drawAreaTop || py > _drawAreaBottom) continue;
                if (tex)
                {
                    int sampleU = u0 + (_texFlipX ? -dx : dx);
                    int sampleV = v0 + (_texFlipY ? -dy : dy);
                    ushort texel = FetchTexel(sampleU & 0xFF, sampleV & 0xFF, clut);
                    if (texel == 0) continue;
                    bool stp = (texel & 0x8000) != 0;
                    int tr = (texel & 0x1F) << 3, tg = ((texel >> 5) & 0x1F) << 3, tb = ((texel >> 10) & 0x1F) << 3;
                    if (!raw) { tr = tr * cr >> 7; tg = tg * cg >> 7; tb = tb * cb >> 7; }
                    Plot(px, py, tr, tg, tb, semi && stp, false, stp);
                }
                else Plot(px, py, cr, cg, cb, semi, false);
            }
    }

    void DrawLine()
    {
        uint cmd = _fifo[0];
        bool gouraud = (cmd & (1u << 28)) != 0;
        bool semi = (cmd & (1u << 25)) != 0;
        int idx = 1;

        int r0 = (int)(cmd & 0xFF), g0 = (int)((cmd >> 8) & 0xFF), b0 = (int)((cmd >> 16) & 0xFF);
        uint v0w = _fifo[idx++];
        int r1 = r0, g1 = g0, b1 = b0;
        if (gouraud) { uint cw = _fifo[idx++]; r1 = (int)(cw & 0xFF); g1 = (int)((cw >> 8) & 0xFF); b1 = (int)((cw >> 16) & 0xFF); }
        uint v1w = _fifo[idx++];

        LineSegment(CoordX(v0w), CoordY(v0w), r0, g0, b0, CoordX(v1w), CoordY(v1w), r1, g1, b1, semi, gouraud);
    }

    void ExecutePolyline()
    {
        uint cmd = _fifo[0];
        bool gouraud = (cmd & (1u << 28)) != 0;
        bool semi = (cmd & (1u << 25)) != 0;

        var pts = new List<(int X, int Y, int R, int G, int B)>();
        int idx = 1;
        int r = (int)(cmd & 0xFF), g = (int)((cmd >> 8) & 0xFF), b = (int)((cmd >> 16) & 0xFF);
        bool first = true;
        while (idx < _fifo.Count)
        {
            if (gouraud && !first) { uint cw = _fifo[idx++]; r = (int)(cw & 0xFF); g = (int)((cw >> 8) & 0xFF); b = (int)((cw >> 16) & 0xFF); }
            if (idx >= _fifo.Count) break;
            uint vw = _fifo[idx++];
            pts.Add((CoordX(vw), CoordY(vw), r, g, b));
            first = false;
        }

        for (int i = 0; i + 1 < pts.Count; i++)
            LineSegment(pts[i].X, pts[i].Y, pts[i].R, pts[i].G, pts[i].B,
                        pts[i + 1].X, pts[i + 1].Y, pts[i + 1].R, pts[i + 1].G, pts[i + 1].B, semi, gouraud);
    }

    void LineSegment(int x0, int y0, int r0, int g0, int b0, int x1, int y1, int r1, int g1, int b1, bool semi, bool gouraud)
    {
        x0 += _drawOffsetX; y0 += _drawOffsetY;
        x1 += _drawOffsetX; y1 += _drawOffsetY;
        if (HleOn) { HleLine(x0, y0, r0, g0, b0, x1, y1, r1, g1, b1, semi, gouraud); return; }
        int dx = Math.Abs(x1 - x0), dy = Math.Abs(y1 - y0);
        int steps = Math.Max(dx, dy);
        if (steps == 0) { Plot(x0, y0, r0, g0, b0, semi, DitherEnabled); return; }
        for (int i = 0; i <= steps; i++)
        {
            double t = (double)i / steps;
            int x = (int)Math.Round(x0 + (x1 - x0) * t);
            int y = (int)Math.Round(y0 + (y1 - y0) * t);
            int r = (int)(r0 + (r1 - r0) * t);
            int g = (int)(g0 + (g1 - g0) * t);
            int b = (int)(b0 + (b1 - b0) * t);
            if (x < _drawAreaLeft || x > _drawAreaRight || y < _drawAreaTop || y > _drawAreaBottom) continue;
            Plot(x, y, r, g, b, semi, DitherEnabled);
        }
    }

    ushort FetchTexel(int u, int v, int clut)
    {
        u = (u & ~(_texWinMaskX * 8)) | ((_texWinOffX & _texWinMaskX) * 8);
        v = (v & ~(_texWinMaskY * 8)) | ((_texWinOffY & _texWinMaskY) * 8);
        u &= 0xFF; v &= 0xFF;

        int row = (_texPageY + v) & (VramHeight - 1);
        if (_texDepth == 2 || _texDepth == 3)
            return Vram[row * VramWidth + ((_texPageX + u) & (VramWidth - 1))];

        int clutX = (clut & 0x3F) * 16;
        int clutY = (clut >> 6) & 0x1FF;
        int index;
        if (_texDepth == 0)
        {
            ushort block = Vram[row * VramWidth + ((_texPageX + (u >> 2)) & (VramWidth - 1))];
            index = (block >> ((u & 3) * 4)) & 0xF;
        }
        else
        {
            ushort block = Vram[row * VramWidth + ((_texPageX + (u >> 1)) & (VramWidth - 1))];
            index = (block >> ((u & 1) * 8)) & 0xFF;
        }
        return Vram[(clutY & (VramHeight - 1)) * VramWidth + ((clutX + index) & (VramWidth - 1))];
    }

    void Plot(int x, int y, int r, int g, int b, bool semi, bool dither, bool maskBit = false)
    {
        if (x < _drawAreaLeft || x > _drawAreaRight || y < _drawAreaTop || y > _drawAreaBottom) return;
        if (x < 0 || x >= VramWidth || y < 0 || y >= VramHeight) return;

        int idx = y * VramWidth + x;
        ushort bg = Vram[idx];
        if (_checkMask && (bg & 0x8000) != 0) return;

        if (dither)
        {
            int d = Dither[y & 3, x & 3];
            r = Clamp255(r + d); g = Clamp255(g + d); b = Clamp255(b + d);
        }

        int fr = Math.Min(31, r >> 3), fg = Math.Min(31, g >> 3), fb = Math.Min(31, b >> 3);

        if (semi)
        {
            int br = bg & 0x1F, bgn = (bg >> 5) & 0x1F, bbl = (bg >> 10) & 0x1F;
            switch (_blendMode)
            {
                case 0: fr = (br + fr) >> 1; fg = (bgn + fg) >> 1; fb = (bbl + fb) >> 1; break;
                case 1: fr = Math.Min(31, br + fr); fg = Math.Min(31, bgn + fg); fb = Math.Min(31, bbl + fb); break;
                case 2: fr = Math.Max(0, br - fr); fg = Math.Max(0, bgn - fg); fb = Math.Max(0, bbl - fb); break;
                default: fr = Math.Min(31, br + (fr >> 2)); fg = Math.Min(31, bgn + (fg >> 2)); fb = Math.Min(31, bbl + (fb >> 2)); break;
            }
        }

        ushort outp = (ushort)(fr | (fg << 5) | (fb << 10));
        if (_setMask || maskBit) outp |= 0x8000;
        Vram[idx] = outp;
    }

    void SetTexpageFromWord(uint tp)
    {
        _texPageX = (int)(tp & 0xF) * 64;
        _texPageY = (int)((tp >> 4) & 1) * 256;
        _blendMode = (int)((tp >> 5) & 3);
        _texDepth = (int)((tp >> 7) & 3);
        _texDisable = (tp & (1u << 11)) != 0;
    }

    static int CoordX(uint w) { int x = (int)(w & 0x7FF); return (x & 0x400) != 0 ? x - 0x800 : x; }
    static int CoordY(uint w) { int y = (int)((w >> 16) & 0x7FF); return (y & 0x400) != 0 ? y - 0x800 : y; }
    static ushort To15(int r, int g, int b) => (ushort)(((r >> 3) & 0x1F) | (((g >> 3) & 0x1F) << 5) | (((b >> 3) & 0x1F) << 10));
    static int Clamp255(int v) => v < 0 ? 0 : v > 255 ? 255 : v;
}
