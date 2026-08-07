namespace RecompOne.Runtime;

using RecompOne.Runtime.Config;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Memory;

public static class Gte
{
    static long _wideProjectionVertices;
    static long _wideProjectionExpandedVertices;
    static long _wideProjectionSaturatedVertices;
    static long _wideProjectionSaturatedObjectVertices;
    static long _divideSaturations;
    static long _nclipTerrainCorrections;
    static long _nclipObjectCorrections;
    static long _nclipRescued;
    // Is near geometry never projected, or projected and then discarded? The
    // renderer only sees what the engine submits, so counting projections the
    // GTE actually performed at close range separates an engine-side gate
    // from a projection that never happens.
    static long _rtpNear100, _rtpNear60, _rtpTotal;
    // These sit in the per-vertex projection path (11.5M calls a run) and in
    // every FLAG read. They answered what they were added for - H is 256, so
    // the divide saturates below depth 128 - and are gated off by default so
    // the JIT can drop them entirely.
    static readonly bool TraceNearProjection =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_NEAR_PROJECTION") == "1";
    static long _projectionFlagsWithheld;
    static int _lastH;
    /// <summary>Depth of the most recent projection, for emitter attribution.</summary>
    public static int LastProjectedDepth;
    static bool _lastDivideSaturated;
    // func_80022... reads the GTE FLAG register and skips the primitive when
    // the error summary bit is set:
    //     c.At = Gte.ReadControl(31);
    //     if ((int)c.At < 0) { ...skip... }
    // Bits 13..18 all feed that summary: packed X/Y saturation (13, 14),
    // MAC0 overflow (15, 16) and divide overflow (17). Geometry passing close
    // beside the camera raises all of them, so withholding only bit 17 - which
    // is what SuppressNearRejection did - never stopped the primitive being
    // thrown away. Retail is right to reject: it cannot draw what it cannot
    // represent. Enhanced reconstructs these vertices from camera space and
    // clips them at its own near plane, so the only thing the flags cost us is
    // the primitive itself. Withhold the whole group; no value changes.
    const uint ProjectionErrorFlags = 0x0007E000u;
    // On: this is what lets geometry passing close beside the camera be
    // drawn at all. The engine reads the GTE FLAG register and skips the
    // primitive when the error summary bit is set; a wall the car is driving
    // alongside trips it, and the wall is sliced off where it passes closest.
    // Measured on the near-cutoff metric (Wild West, 700 frames): the nearest
    // submitted wall vertex moves from depth 72.9 to 26.2 with this on.
    // Enhanced reconstructs these vertices from camera space and clips them at
    // its own near plane, so the flags cost only the primitive itself.
    static readonly bool NearFlagSuppression =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_NEAR_FLAGS") != "0";
    // Geometry passing close beside the camera projects far outside the range
    // a PS1 coordinate can hold, so the packed result saturates and the engine
    // throws the primitive away - correct for hardware that can only draw what
    // it can represent. Enhanced reconstructs this geometry from camera space
    // and never reads the packed value, so folding the saturated coordinate
    // back into a representable window only stops the primitive being
    // discarded. Nothing that is drawn moves.
    // Measured inert. Normalised against each run's own drawn geometry it
    // changes nothing: 32.4 near primitives per million drawn with it, 32.0
    // without. The apparent doubling seen first came from comparing raw counts
    // between soak runs that had diverged. Left off; it alters game-visible
    // packed coordinates for no demonstrated benefit.
    // On, and not widescreen-scoped. The artifact occurs at 4:3 as well, so
    // gating this on a wide aspect meant it never ran in any test. The
    // projection divide saturates below depth H/2 = 128, and the observed
    // floor on drawn geometry is 110-115 in every capture taken so far.
    static readonly bool NearKeepGeometry =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_NEAR_KEEP") != "0";

    public static (long Total, long Near100, long Near60, int H)
        ConsumeRtpNearCounts()
    {
        var v = (_rtpTotal, _rtpNear100 + _projectionFlagsWithheld * 0,
            _rtpNear60, _lastH);
        _rtpTotal = _rtpNear100 = _rtpNear60 = 0;
        return v;
    }

    // Proof aid, not a game feature. The soak harness is not deterministic
    // frame-to-frame - two identical runs differ across most of the frame -
    // so a before/after screenshot pair cannot show what this fix restored.
    // Record the polygons whose front-facing sign the rounded integer test
    // erased and let the rasteriser paint them, so one frame shows exactly
    // the geometry that used to be missing.
    public static readonly bool MarkRescuedNclip =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_MARK_NCLIP") == "1";
    static readonly HashSet<ulong> RescuedTriangles = [];
    public static long MarkedPrimitives;
    public static long TestedPrimitives;
    public static long CandidatePrimitives;
    public static int RescuedTriangleCount => RescuedTriangles.Count;

    static ulong RescueHash(int x, int y) =>
        (((ulong)(ushort)(short)x << 16) | (ushort)(short)y)
            * 0x9E3779B97F4A7C15UL;

    /// <summary>Order-independent identity for a projected triangle.</summary>
    public static ulong RescueKey(
        int x0, int y0, int x1, int y1, int x2, int y2) =>
        RescueHash(x0, y0) ^ RescueHash(x1, y1) ^ RescueHash(x2, y2);

    public static bool WasNclipRescued(ulong key) =>
        RescuedTriangles.Contains(key);

    public static void ClearNclipRescued() => RescuedTriangles.Clear();


    /// <summary>
    /// Polygons whose backface sign the widescreen-compressed integer NCLIP
    /// got wrong, split into the terrain transform and everything else.
    /// </summary>
    public static (long Terrain, long Object, long Rescued)
        ConsumeNclipCorrections()
    {
        var value = (_nclipTerrainCorrections, _nclipObjectCorrections,
            _nclipRescued);
        _nclipTerrainCorrections = 0;
        _nclipObjectCorrections = 0;
        _nclipRescued = 0;
        return value;
    }

    static readonly bool SuppressNearRejection =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_NEAR_REJECTION") != "0";

    public static long ConsumeDivideSaturations()
    {
        long value = _divideSaturations;
        _divideSaturations = 0;
        return value;
    }

    public static long ConsumeSaturatedObjectVertices()
    {
        long value = _wideProjectionSaturatedObjectVertices;
        _wideProjectionSaturatedObjectVertices = 0;
        return value;
    }

    public static long ConsumeSaturatedVertices()
    {
        long value = _wideProjectionSaturatedVertices;
        _wideProjectionSaturatedVertices = 0;
        return value;
    }
    // Native pixels of unused window left on each side of the packed
    // widescreen projection, while the terrain transform is running. Retail
    // packet clipping operates on the packed SXY, so a terrain primitive that
    // straddles the packed window edge is discarded whole even though its
    // visible part is inside the widened viewport, which opens wedges along
    // both outer edges. Enhanced draws terrain from camera space and never
    // reads the packed X, so reserving headroom moves the retail clip
    // boundary outside the real picture instead of through it.
    //
    // Measured on Route 66: outer-edge terrain loss stops falling at 40 native
    // pixels and is identical at 56 and 72, so 48 sits on the plateau with
    // slack for other camera pitches while leaving the packed window ample
    // resolution for retail logic.
    //
    // Confined to the terrain scope. Applying it to every gameplay projection
    // was tried against a report of objects vanishing at the outer edges up
    // close, and made no measurable difference to how much object geometry
    // reaches the edges (11.02% against 10.69%, inside run-to-run noise) or to
    // packed-coordinate saturation. Do not widen this scope again without a
    // reproduction that it demonstrably fixes.
    static readonly int WideClipHeadroom =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_WIDE_CLIP_HEADROOM"),
            out int wideClipHeadroom)
            ? Math.Clamp(wideClipHeadroom, 0, 120)
            : 48;
    static readonly bool TerrainPreciseNclip =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_PRECISE_NCLIP") != "0";
    // Widescreen narrows the PS1-visible SX by BaseAspect/WideAspect before
    // rounding it to an integer. NCLIP's cross product is a sum of x*dy terms,
    // so that scaling shrinks every polygon's measured area by the same ratio
    // and pushes far more of them under one unit, where rounding decides the
    // sign. A tall thin vertical quad - which is exactly what a destructible
    // wall panel is - has an area dominated by sub-pixel x differences, so the
    // rounded test flips it to backfacing at random and the engine drops the
    // whole panel. The result on screen is a full-height vertical slice of
    // wall missing. Measure the area from the same fractional projection the
    // renderer draws from; the sign only changes for polygons the integer test
    // was already getting wrong, because a uniform positive x scale cannot
    // reorder a cross product on its own.
    // On, scoped. All four mesh emitters (func_80022A4C and siblings) run
    // NCLIP, read MAC0 and discard the triangle when the area is <= 0. SatX /
    // SatY clamp packed coordinates to +/-1024, so geometry passing close
    // beside the camera saturates, reads degenerate, and is thrown away - the
    // reported artifact, and why it only happens near objects.
    //
    // The area is computed from SxyUnclamped* for object geometry only.
    // Terrain keeps the clamped projection: feeding it unclamped values
    // inverted terrain backface signs and shredded the ground (T68), and no
    // geometry statistic detected it - only a whole-frame image comparison
    // did (T69). Gate any change here with whole_frame_gate.py, whose control
    // run establishes the ~1.8% noise floor.
    // Retail culling neutralised. All four mesh emitters reject a triangle
    // when NCLIP's area comes out <= 0, computed from coordinates the PS1
    // could represent. Enhanced draws from camera space and does not need that
    // decision made for it, so this forces the result positive and lets the
    // renderer decide what is visible. Diagnostic sledgehammer first: if
    // geometry the engine was dropping still does not appear with this on, the
    // rejection is not NCLIP at all.
    static readonly bool NoRetailCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_NO_RETAIL_CULL") == "1";

    static readonly bool WidePreciseNclip =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_WIDE_PRECISE_NCLIP") != "0";
    static readonly bool WideClipHeadroomGlobal =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_WIDE_CLIP_GLOBAL") == "1";
    static int _terrainProjectionDepth;

    /// <summary>
    /// Brackets the retail terrain transform. Only projections issued inside
    /// take the widescreen clip headroom described on
    /// <see cref="WideClipHeadroom"/>.
    /// </summary>
    public static void BeginTerrainProjection() => _terrainProjectionDepth++;

    public static void EndTerrainProjection()
    {
        if (_terrainProjectionDepth > 0)
            _terrainProjectionDepth--;
    }
    static readonly short[] V = new short[9];
    static byte RGBC_R, RGBC_G, RGBC_B, RGBC_CODE;
    static ushort OTZ;
    static int IR0, IR1, IR2, IR3;
    static readonly short[] SX = new short[3];
    static readonly short[] SY = new short[3];
    // Projection provenance for the three screen-coordinate FIFO entries.
    // The hardware SZ value remains the raster depth. PerspectiveW retains
    // the unsaturated camera-space Z used by PGXP-class renderers, because
    // saturated 16-bit SZ is not sufficient for stable perspective mapping.
    static readonly ushort[] SxyDepth = new ushort[3];
    static readonly float[] SxyPerspectiveW = new float[3];
    static readonly float[] SxyPreciseX = new float[3];
    // The clamped copies above are what a PS1 coordinate can represent.
    // NCLIP's signed area must not be computed from them: geometry passing
    // close beside the camera projects far off-screen, two or three vertices
    // clamp to the same limit, the triangle reads as degenerate and the engine
    // discards it as backfacing. Keep the unclamped projection for that test.
    static readonly float[] SxyUnclampedX = new float[3];
    static readonly float[] SxyUnclampedY = new float[3];
    static readonly float[] SxyPreciseY = new float[3];
    static readonly float[] SxyViewX = new float[3];
    static readonly float[] SxyViewY = new float[3];
    static readonly float[] SxyViewZ = new float[3];
    static readonly float[] SxyProjectionCenterX = new float[3];
    static readonly float[] SxyProjectionCenterY = new float[3];
    static readonly float[] SxyProjectionScale = new float[3];
    static readonly bool[] SxyHasPrecisePosition = new bool[3];
    static readonly ushort[] SZ = new ushort[4];

    /// <summary>
    /// The triangle currently in the projection registers, for recording
    /// geometry the engine rejects. Every capture so far has shown only what
    /// survived, so the cause of a rejection has had to be inferred from an
    /// absence; this makes the rejected triangle itself observable.
    /// </summary>
    public static void ReadProjectedTriangle(
        Span<int> xs, Span<int> ys, Span<int> zs)
    {
        for (int i = 0; i < 3; i++)
        {
            xs[i] = SX[i];
            ys[i] = SY[i];
            zs[i] = SZ[i + 1];
        }
    }

    public static uint CurrentFlags => FLAG;
    public static int CurrentMac0 => MAC0;
    static readonly uint[] RGB = new uint[3];
    // GPU packets contain only projected XY coordinates, so enhanced texture
    // projection correlates them with the GTE's most recent screen/depth
    // results. The former reverse scan through 4096 entries was O(vertices *
    // 4096) and dominated frame time in busy arenas. Two generation maps
    // preserve the required one-frame ordering-table latency and "latest
    // projection wins" behavior with bounded O(1) lookups.
    readonly struct ScreenDepthSample
    {
        public readonly ushort Depth;
        public readonly uint Group;

        public ScreenDepthSample(ushort depth, uint group)
        {
            Depth = depth;
            Group = group;
        }
    }

    readonly struct ScreenDepthSamples
    {
        public readonly ScreenDepthSample A, B, C, D, E, F, G, H;
        public readonly byte Count;

        ScreenDepthSamples(
            ScreenDepthSample a,
            ScreenDepthSample b,
            ScreenDepthSample c,
            ScreenDepthSample d,
            ScreenDepthSample e,
            ScreenDepthSample f,
            ScreenDepthSample g,
            ScreenDepthSample h,
            byte count)
        {
            A = a; B = b; C = c; D = d;
            E = e; F = f; G = g; H = h;
            Count = count;
        }

        ScreenDepthSample At(int index) => index switch
        {
            0 => A,
            1 => B,
            2 => C,
            3 => D,
            4 => E,
            5 => F,
            6 => G,
            _ => H,
        };

        public ScreenDepthSamples Add(ushort depth, uint group)
        {
            var value = new ScreenDepthSample(depth, group);
            for (int index = 0; index < Count; index++)
            {
                ScreenDepthSample existing = At(index);
                if (existing.Depth == depth && existing.Group == group)
                    return this;
            }

            return Count switch
            {
                0 => new(value, default, default, default,
                         default, default, default, default, 1),
                1 => new(A, value, default, default,
                         default, default, default, default, 2),
                2 => new(A, B, value, default,
                         default, default, default, default, 3),
                3 => new(A, B, C, value,
                         default, default, default, default, 4),
                4 => new(A, B, C, D, value,
                         default, default, default, 5),
                5 => new(A, B, C, D, E, value,
                         default, default, 6),
                6 => new(A, B, C, D, E, F, value,
                         default, 7),
                7 => new(A, B, C, D, E, F, G, value, 8),
                // Dense distant meshes routinely quantize more than four
                // distinct vertices to one PS1 pixel. Retain eight recent
                // projections so the OT-constrained lookup can still find
                // the correct surface without an unbounded history.
                _ => new(B, C, D, E, F, G, H, value, 8),
            };
        }

        public void FindClosest(
            int expectedDepth,
            ref ushort closest,
            ref int closestError)
        {
            for (int index = 0; index < Count; index++)
            {
                ScreenDepthSample sample = At(index);
                int error = Math.Abs(sample.Depth - expectedDepth);
                if (error >= closestError) continue;
                closest = sample.Depth;
                closestError = error;
            }
        }

        public int CollectGroups(
            int expectedDepth,
            int maximumError,
            Span<uint> groups,
            int count)
        {
            for (int index = 0; index < Count && count < groups.Length; index++)
            {
                ScreenDepthSample sample = At(index);
                if (Math.Abs(sample.Depth - expectedDepth) > maximumError)
                    continue;
                bool duplicate = false;
                for (int existing = 0; existing < count; existing++)
                    duplicate |= groups[existing] == sample.Group;
                if (!duplicate)
                    groups[count++] = sample.Group;
            }
            return count;
        }

        public bool TryGetGroup(
            uint group,
            int expectedDepth,
            int maximumError,
            out ushort depth)
        {
            depth = 0;
            int closestError = int.MaxValue;
            for (int index = 0; index < Count; index++)
            {
                ScreenDepthSample sample = At(index);
                if (sample.Group != group)
                    continue;
                int error = Math.Abs(sample.Depth - expectedDepth);
                if (error >= closestError)
                    continue;
                depth = sample.Depth;
                closestError = error;
            }
            return closestError <= maximumError;
        }

        public int CollectSamples(
            int expectedDepth,
            int maximumError,
            Span<ScreenDepthSample> samples,
            int count)
        {
            for (int index = 0; index < Count && count < samples.Length; index++)
            {
                ScreenDepthSample sample = At(index);
                if (Math.Abs(sample.Depth - expectedDepth) > maximumError)
                    continue;
                bool duplicate = false;
                for (int existing = 0; existing < count; existing++)
                    duplicate |=
                        samples[existing].Depth == sample.Depth &&
                        samples[existing].Group == sample.Group;
                if (!duplicate)
                    samples[count++] = sample;
            }
            return count;
        }
    }

    static Dictionary<uint, ScreenDepthSamples> ScreenDepthCurrent =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious2 =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious3 =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious4 =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious5 =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious6 =
        new(16384);
    static Dictionary<uint, ScreenDepthSamples> ScreenDepthPrevious7 =
        new(16384);
    static uint ScreenProjectionGroup;
    static uint ActiveProjectionGroup;
    static uint RES1;
    static int MAC0, MAC1, MAC2, MAC3;
    static uint LZCS, LZCR;

    static readonly short[] RT = new short[9];
    static readonly short[] LLM = new short[9];
    static readonly short[] LCM = new short[9];
    static readonly int[] TR = new int[3];
    static readonly int[] BK = new int[3];
    static readonly int[] FC = new int[3];
    static int OFX, OFY;
    static ushort H;
    static short DQA;
    static int DQB;
    static short ZSF3, ZSF4;
    static uint FLAG;
    public static long FlagRegisterReads;
    public static long FlagRegisterErrors;

    static readonly byte[] Unr = BuildUnr();

    static byte[] BuildUnr()
    {
        var t = new byte[0x101];
        for (int i = 0; i < 0x101; i++)
        {
            int v = (0x40000 / (i + 0x100) + 1) / 2 - 0x101;
            t[i] = (byte)(v < 0 ? 0 : v > 0xFF ? 0xFF : v);
        }
        return t;
    }

    static void Flag(int bit) => FLAG |= 1u << bit;

    static int SatIR(int n, int v, bool lm)
    {
        int min = lm ? 0 : -0x8000;
        if (v < min) { v = min; Flag(25 - n); }
        else if (v > 0x7FFF) { v = 0x7FFF; Flag(25 - n); }
        return v;
    }

    static int SatIR0(int v)
    {
        if (v < 0) { Flag(12); return 0; }
        if (v > 0x1000) { Flag(12); return 0x1000; }
        return v;
    }

    static int SatColor(int n, int v)
    {
        if (v < 0) { Flag(21 - n); return 0; }
        if (v > 0xFF) { Flag(21 - n); return 0xFF; }
        return v;
    }

    static int SatSZ(int v)
    {
        if (v < 0) { Flag(18); return 0; }
        if (v > 0xFFFF) { Flag(18); return 0xFFFF; }
        return v;
    }

    static int SatX(int v)
    {
        if (v < -0x400) { Flag(14); return -0x400; }
        if (v > 0x3FF) { Flag(14); return 0x3FF; }
        return v;
    }

    static int SatY(int v)
    {
        if (v < -0x400) { Flag(13); return -0x400; }
        if (v > 0x3FF) { Flag(13); return 0x3FF; }
        return v;
    }

    static long CheckMac0(long v)
    {
        if (v > 0x7FFFFFFFL) Flag(16);
        else if (v < -0x80000000L) Flag(15);
        return v;
    }

    static void CheckMac(int n, long v)
    {
        if (v >= (1L << 43)) Flag(31 - n);
        else if (v < -(1L << 43)) Flag(28 - n);
    }

    static void SetMac(int n, long v, int sf, bool lm)
    {
        CheckMac(n, v);
        int m = (int)(v >> sf);
        if (n == 1) { MAC1 = m; IR1 = SatIR(1, m, lm); }
        else if (n == 2) { MAC2 = m; IR2 = SatIR(2, m, lm); }
        else { MAC3 = m; IR3 = SatIR(3, m, lm); }
    }

    static void MatVec(short[] mx, int t0, int t1, int t2, int vx, int vy, int vz, int sf, bool lm)
    {
        SetMac(1, ((long)t0 << 12) + (long)mx[0] * vx + (long)mx[1] * vy + (long)mx[2] * vz, sf, lm);
        SetMac(2, ((long)t1 << 12) + (long)mx[3] * vx + (long)mx[4] * vy + (long)mx[5] * vz, sf, lm);
        SetMac(3, ((long)t2 << 12) + (long)mx[6] * vx + (long)mx[7] * vy + (long)mx[8] * vz, sf, lm);
    }
    static void PushColor()
    {
        int r = SatColor(0, MAC1 >> 4);
        int g = SatColor(1, MAC2 >> 4);
        int b = SatColor(2, MAC3 >> 4);
        RGB[0] = RGB[1]; RGB[1] = RGB[2];
        RGB[2] = (uint)(r | (g << 8) | (b << 16) | (RGBC_CODE << 24));
    }

    static void Interp(long in1, long in2, long in3, int sf, bool lm)
    {
        IR1 = SatIR(1, (int)((((long)FC[0] << 12) - in1) >> sf), false);
        IR2 = SatIR(2, (int)((((long)FC[1] << 12) - in2) >> sf), false);
        IR3 = SatIR(3, (int)((((long)FC[2] << 12) - in3) >> sf), false);
        SetMac(1, (long)IR1 * IR0 + in1, sf, lm);
        SetMac(2, (long)IR2 * IR0 + in2, sf, lm);
        SetMac(3, (long)IR3 * IR0 + in3, sf, lm);
        PushColor();
    }

    static void Modulate(int sf, bool lm)
    {
        SetMac(1, ((long)RGBC_R * IR1) << 4, sf, lm);
        SetMac(2, ((long)RGBC_G * IR2) << 4, sf, lm);
        SetMac(3, ((long)RGBC_B * IR3) << 4, sf, lm);
        PushColor();
    }

    static int Clz16(uint v)
    {
        int n = 0;
        for (int i = 15; i >= 0 && (v & (1u << i)) == 0; i--) n++;
        return n;
    }

    static uint Divide(uint h, uint sz3)
    {
        if (h >= sz3 * 2)
        {
            _divideSaturations++;
            _lastDivideSaturated = true;
            // Retail rejects the whole primitive when this overflows, which is
            // correct for hardware that can only draw what this produces. A
            // wall segment passing beside the camera trips it and disappears,
            // and widescreen shows a third more of exactly that region, so the
            // gap lands inside the picture as a vertical cut that slides along
            // the wall as you drive.
            //
            // Enhanced projects this geometry from camera space and never
            // reads the result, so withholding the overflow flag only stops
            // the primitive being thrown away. The saturated value is still
            // returned, so anything that does consume it sees what it saw
            // before.
            if (!SuppressNearRejection ||
                !ConfigManager.View.HighResolution3D ||
                !GpuHle.GameplayActive)
                Flag(17);
            return 0x1FFFF;
        }
        _lastDivideSaturated = false;
        int z = Clz16(sz3);
        ulong n = (ulong)h << z;
        ulong d = (ulong)sz3 << z;
        int idx = (int)((d - 0x7FC0) >> 7);
        if (idx < 0) idx = 0; else if (idx > 0x100) idx = 0x100;
        ulong u = (ulong)Unr[idx] + 0x101;
        d = (0x2000080UL - d * u) >> 8;
        d = (0x0000080UL + d * u) >> 8;
        ulong res = (n * d + 0x8000) >> 16;
        return res > 0x1FFFF ? 0x1FFFFu : (uint)res;
    }

    static void Rtp(int vx, int vy, int vz, int sf, bool lm, bool last)
    {
        uint flagBeforeProjection = FLAG;
        long m1 = ((long)TR[0] << 12) + (long)RT[0] * vx + (long)RT[1] * vy + (long)RT[2] * vz;
        long m2 = ((long)TR[1] << 12) + (long)RT[3] * vx + (long)RT[4] * vy + (long)RT[5] * vz;
        long m3 = ((long)TR[2] << 12) + (long)RT[6] * vx + (long)RT[7] * vy + (long)RT[8] * vz;
        CheckMac(1, m1); CheckMac(2, m2); CheckMac(3, m3);
        MAC1 = (int)(m1 >> sf); MAC2 = (int)(m2 >> sf); MAC3 = (int)(m3 >> sf);
        IR1 = SatIR(1, MAC1, lm);
        IR2 = SatIR(2, MAC2, lm);
        int ir3flag = (int)(m3 >> 12);
        if (ir3flag < -0x8000 || ir3flag > 0x7FFF) Flag(22);
        IR3 = MAC3 < (lm ? 0 : -0x8000) ? (lm ? 0 : -0x8000) : MAC3 > 0x7FFF ? 0x7FFF : MAC3;

        int sz = SatSZ((int)(m3 >> 12));
        LastProjectedDepth = sz;
        SZ[0] = SZ[1]; SZ[1] = SZ[2]; SZ[2] = SZ[3]; SZ[3] = (ushort)sz;

        if (TraceNearProjection)
        {
            _lastH = (int)H;
            _rtpTotal++;
            if (SZ[3] < 100) _rtpNear100++;
            if (SZ[3] < 60) _rtpNear60++;
        }
        uint div = Divide(H, SZ[3]);
        bool wideProjection =
            ConfigManager.View.HighResolution3D &&
            GpuHle.GameplayActive &&
            GpuHle.WideAspect > GpuHle.BaseAspect + 0.001f;
        // Geometry beside the camera overflows MAC0 here as well: the divide
        // has already saturated, and multiplying that by a large IR leaves the
        // 32-bit range. Bits 13..18 of FLAG all feed the error summary bit the
        // engine tests, so withholding only the divide's bit 17 still loses
        // the primitive. Withhold the overflow bits from this projection too,
        // under the same conditions - Enhanced never reads MAC0 or the packed
        // result for these vertices, so nothing that is drawn changes.
        bool nearKeep =
            NearKeepGeometry && _lastDivideSaturated &&
            ConfigManager.View.HighResolution3D && GpuHle.GameplayActive;
        // Measured harmful: withholding these dropped near geometry from
        // 32 to 5.5 primitives per million drawn. Left raising the flags.
        long sx = CheckMac0((long)div * IR1 + OFX); MAC0 = (int)sx;
        long sy = CheckMac0((long)div * IR2 + OFY); MAC0 = (int)sy;
        long packedSx = sx;
        if (wideProjection)
        {
            // The renderer keeps the original, uncompressed view-space
            // projection below.  Only the PS1-visible SXY result is narrowed
            // so retail frustum/clip tests cover the wider PC viewport.
            // This is the same projection contract as an emulator's
            // widescreen geometry hack, but it cannot distort Enhanced output
            // because Enhanced reconstructs from SxyView* and the original
            // projection center/scale.
            double ratio = GpuHle.BaseAspect / GpuHle.WideAspect;
            if (WideClipHeadroom > 0 &&
                (WideClipHeadroomGlobal || _terrainProjectionDepth > 0))
            {
                // The packed window is the authored gameplay display, which is
                // where retail clipping happens regardless of output size.
                const double window = 320d;
                double usable = window - 2d * WideClipHeadroom;
                if (usable > 32d)
                    ratio *= usable / window;
            }
            packedSx = OFX + (long)Math.Round(
                (sx - OFX) * ratio,
                MidpointRounding.AwayFromZero);
            _wideProjectionVertices++;

            double center = OFX / 65536.0;
            double originalX = sx / 65536.0;
            double nativeX = packedSx / 65536.0;
            if (Math.Abs(originalX - center) > 160.0 &&
                Math.Abs(nativeX - center) <= 160.0)
                _wideProjectionExpandedVertices++;
        }
        // Count vertices whose packed X leaves the range retail geometry can
        // represent. Those are the ones the retail clipper mishandles, and
        // they are produced by geometry that is both close and far off-axis.
        long packedX = packedSx >> 16;
        if (wideProjection && (packedX < -1023 || packedX > 1023))
        {
            if (_terrainProjectionDepth > 0)
                _wideProjectionSaturatedVertices++;
            else
                _wideProjectionSaturatedObjectVertices++;
        }
        long packedY = sy >> 16;
        if (nearKeep)
        {
            // Keep the direction, drop only the unrepresentable magnitude.
            long cx = OFX >> 16, cy = OFY >> 16;
            packedX = Math.Clamp(packedX, cx - 480, cx + 480);
            packedY = Math.Clamp(packedY, cy - 480, cy + 480);
        }
        int nx = SatX((int)packedX);
        int ny = SatY((int)packedY);
        SX[0] = SX[1]; SX[1] = SX[2]; SX[2] = (short)nx;
        SY[0] = SY[1]; SY[1] = SY[2]; SY[2] = (short)ny;
        SxyDepth[0] = SxyDepth[1];
        SxyDepth[1] = SxyDepth[2];
        SxyDepth[2] = (ushort)sz;
        SxyPerspectiveW[0] = SxyPerspectiveW[1];
        SxyPerspectiveW[1] = SxyPerspectiveW[2];
        float rawViewZ = (float)(m3 / 4096.0);
        float preciseZ = MathF.Max(H * 0.5f, rawViewZ);
        SxyPerspectiveW[2] = preciseZ;
        SxyPreciseX[0] = SxyPreciseX[1];
        SxyPreciseX[1] = SxyPreciseX[2];
        SxyPreciseY[0] = SxyPreciseY[1];
        SxyPreciseY[1] = SxyPreciseY[2];
        SxyUnclampedX[0] = SxyUnclampedX[1];
        SxyUnclampedX[1] = SxyUnclampedX[2];
        SxyUnclampedY[0] = SxyUnclampedY[1];
        SxyUnclampedY[1] = SxyUnclampedY[2];
        SxyViewX[0] = SxyViewX[1];
        SxyViewX[1] = SxyViewX[2];
        SxyViewY[0] = SxyViewY[1];
        SxyViewY[1] = SxyViewY[2];
        SxyViewZ[0] = SxyViewZ[1];
        SxyViewZ[1] = SxyViewZ[2];
        SxyProjectionCenterX[0] = SxyProjectionCenterX[1];
        SxyProjectionCenterX[1] = SxyProjectionCenterX[2];
        SxyProjectionCenterY[0] = SxyProjectionCenterY[1];
        SxyProjectionCenterY[1] = SxyProjectionCenterY[2];
        SxyProjectionScale[0] = SxyProjectionScale[1];
        SxyProjectionScale[1] = SxyProjectionScale[2];
        SxyHasPrecisePosition[0] = SxyHasPrecisePosition[1];
        SxyHasPrecisePosition[1] = SxyHasPrecisePosition[2];
        SxyViewX[2] = (float)(m1 / 4096.0);
        SxyViewY[2] = (float)(m2 / 4096.0);
        SxyViewZ[2] = rawViewZ;
        SxyProjectionCenterX[2] = OFX / 65536.0f;
        SxyProjectionCenterY[2] = OFY / 65536.0f;
        SxyProjectionScale[2] = H;
        float projectionScale = H / preciseZ;
        float unclampedX =
            SxyProjectionCenterX[2] + SxyViewX[2] * projectionScale;
        float unclampedY =
            SxyProjectionCenterY[2] + SxyViewY[2] * projectionScale;
        SxyUnclampedX[2] = float.IsFinite(unclampedX)
            ? Math.Clamp(unclampedX, -1e7f, 1e7f) : 0f;
        SxyUnclampedY[2] = float.IsFinite(unclampedY)
            ? Math.Clamp(unclampedY, -1e7f, 1e7f) : 0f;
        SxyPreciseX[2] = Math.Clamp(unclampedX, -1024f, 1023f);
        SxyPreciseY[2] = Math.Clamp(unclampedY, -1024f, 1023f);
        SxyHasPrecisePosition[2] =
            float.IsFinite(SxyPreciseX[2]) &&
            float.IsFinite(SxyPreciseY[2]) &&
            float.IsFinite(SxyViewX[2]) &&
            float.IsFinite(SxyViewY[2]) &&
            float.IsFinite(SxyViewZ[2]) &&
            SxyProjectionScale[2] > 0;
        RecordScreenDepth((short)nx, (short)ny, (ushort)sz);

        if (last)
        {
            long dp = CheckMac0((long)div * DQA + DQB);
            MAC0 = (int)dp;
            IR0 = SatIR0((int)(dp >> 12));
        }

        if (NearFlagSuppression && wideProjection)
        {
            // Put bits 13..18 back exactly as this projection found them and
            // recompute the summary bit, so a vertex the hardware could not
            // represent no longer costs the engine the whole primitive.
            FLAG = (FLAG & ~ProjectionErrorFlags) |
                   (flagBeforeProjection & ProjectionErrorFlags);
            FLAG &= ~0x80000000u;
            if ((FLAG & 0x7F87E000u) != 0)
                FLAG |= 0x80000000u;
            _projectionFlagsWithheld++;
        }
    }

    public static (long Vertices, long ExpandedVertices)
        ConsumeWidescreenProjectionMetrics()
    {
        long vertices = _wideProjectionVertices;
        long expanded = _wideProjectionExpandedVertices;
        _wideProjectionVertices = 0;
        _wideProjectionExpandedVertices = 0;
        return (vertices, expanded);
    }

    static void RecordScreenDepth(short x, short y, ushort z)
    {
        if (z == 0) return;
        uint key = ScreenKey(x, y);
        ScreenDepthCurrent.TryGetValue(key, out ScreenDepthSamples samples);
        ScreenDepthCurrent[key] = samples.Add(z, ActiveProjectionGroup);
    }

    public static bool TryGetScreenDepth(
        int x, int y, int orderingTableDepth, out ushort z)
    {
        int expectedDepth = Math.Clamp(orderingTableDepth * 8, 1, 0xFFFF);
        if (TryLookupScreenDepth(
                ScreenKey((short)x, (short)y), expectedDepth, out z))
            return true;

        // Packet coordinates can differ by one pixel after the game's fixed-
        // point rounding and draw offset. Recover that benign mismatch without
        // accepting a broad nearest-neighbour depth from another surface.
        for (int radius = 1; radius <= 1; radius++)
        {
            for (int oy = -radius; oy <= radius; oy++)
            {
                for (int ox = -radius; ox <= radius; ox++)
                {
                    if (Math.Max(Math.Abs(ox), Math.Abs(oy)) != radius)
                        continue;
                    if (TryLookupScreenDepth(
                            ScreenKey((short)(x + ox), (short)(y + oy)),
                            expectedDepth,
                            out z))
                        return true;
                }
            }
        }

        z = 0;
        return false;
    }

    /// <summary>
    /// Recovers a triangle's original camera-space depths from either one
    /// RTPT/RTPS group or a tightly consecutive RTPS run. Shared screen pixels
    /// can contain unrelated surfaces, so three independent nearest depths
    /// are not sufficient for safe perspective correction.
    /// </summary>
    public static bool TryGetTriangleScreenDepth(
        int x0, int y0,
        int x1, int y1,
        int x2, int y2,
        int orderingTableDepth,
        out ushort z0,
        out ushort z1,
        out ushort z2)
    {
        if (TryGetGroupedTriangleScreenDepth(
                x0, y0, x1, y1, x2, y2, orderingTableDepth,
                out z0, out z1, out z2))
            return true;

        int expectedDepth = Math.Clamp(orderingTableDepth * 8, 1, 0xFFFF);
        int maximumError = Math.Max(256, expectedDepth / 2);

        // Some original paths project one cached vertex at a time with RTPS
        // instead of one RTPT. Those three operations are consecutive. Match
        // only a tight run of projection IDs so independent nearest-depth
        // samples from overlapping objects cannot be combined.
        Span<ScreenDepthSample> samples0 =
            stackalloc ScreenDepthSample[16];
        Span<ScreenDepthSample> samples1 =
            stackalloc ScreenDepthSample[16];
        Span<ScreenDepthSample> samples2 =
            stackalloc ScreenDepthSample[16];
        int count0 = CollectSamplesAt(
            x0, y0, expectedDepth, maximumError, samples0);
        int count1 = CollectSamplesAt(
            x1, y1, expectedDepth, maximumError, samples1);
        int count2 = CollectSamplesAt(
            x2, y2, expectedDepth, maximumError, samples2);
        int bestScore = int.MaxValue;
        ushort best0 = 0, best1 = 0, best2 = 0;
        for (int i = 0; i < count0; i++)
            for (int j = 0; j < count1; j++)
                for (int k = 0; k < count2; k++)
                {
                    uint minimumGroup = Math.Min(
                        samples0[i].Group,
                        Math.Min(samples1[j].Group, samples2[k].Group));
                    uint maximumGroup = Math.Max(
                        samples0[i].Group,
                        Math.Max(samples1[j].Group, samples2[k].Group));
                    if (maximumGroup - minimumGroup > 4u)
                        continue;
                    int minimumDepth = Math.Min(
                        samples0[i].Depth,
                        Math.Min(samples1[j].Depth, samples2[k].Depth));
                    int maximumDepth = Math.Max(
                        samples0[i].Depth,
                        Math.Max(samples1[j].Depth, samples2[k].Depth));
                    if (minimumDepth == 0 ||
                        maximumDepth > minimumDepth * 16)
                        continue;
                    int score =
                        Math.Abs(samples0[i].Depth - expectedDepth) +
                        Math.Abs(samples1[j].Depth - expectedDepth) +
                        Math.Abs(samples2[k].Depth - expectedDepth) +
                        checked((int)(maximumGroup - minimumGroup) * 64);
                    if (score >= bestScore)
                        continue;
                    bestScore = score;
                    best0 = samples0[i].Depth;
                    best1 = samples1[j].Depth;
                    best2 = samples2[k].Depth;
                }
        if (bestScore != int.MaxValue)
        {
            z0 = best0;
            z1 = best1;
            z2 = best2;
            return true;
        }

        z0 = z1 = z2 = 0;
        return false;
    }

    /// <summary>
    /// Recovers a triangle only when all three vertices belong to one native
    /// RTPT/RTPS projection group. This bounded path is suitable for raster
    /// depth, where accepting three merely nearby samples would corrupt
    /// opaque/transparent occlusion.
    /// </summary>
    public static bool TryGetGroupedTriangleScreenDepth(
        int x0, int y0,
        int x1, int y1,
        int x2, int y2,
        int orderingTableDepth,
        out ushort z0,
        out ushort z1,
        out ushort z2)
    {
        int expectedDepth = Math.Clamp(orderingTableDepth * 8, 1, 0xFFFF);
        int maximumError = Math.Max(256, expectedDepth / 2);
        Span<uint> groups = stackalloc uint[16];
        int count = CollectGroupsAt(
            x0, y0, expectedDepth, maximumError, groups);
        for (int index = 0; index < count; index++)
        {
            uint group = groups[index];
            if (!TryGetGroupAt(
                    x0, y0, group, expectedDepth, maximumError, out z0) ||
                !TryGetGroupAt(
                    x1, y1, group, expectedDepth, maximumError, out z1) ||
                !TryGetGroupAt(
                    x2, y2, group, expectedDepth, maximumError, out z2))
                continue;

            int minimum = Math.Min(z0, Math.Min(z1, z2));
            int maximum = Math.Max(z0, Math.Max(z1, z2));
            if (minimum > 0 && maximum <= minimum * 16)
                return true;
        }

        z0 = z1 = z2 = 0;
        return false;
    }

    static uint ScreenKey(short x, short y) =>
        (ushort)x | ((uint)(ushort)y << 16);

    static bool TryLookupScreenDepth(
        uint key, int expectedDepth, out ushort z)
    {
        z = 0;
        int closestError = int.MaxValue;
        if (ScreenDepthCurrent.TryGetValue(
                key, out ScreenDepthSamples current))
            current.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious.TryGetValue(
                key, out ScreenDepthSamples previous))
            previous.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious2.TryGetValue(
                key, out ScreenDepthSamples previous2))
            previous2.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious3.TryGetValue(
                key, out ScreenDepthSamples previous3))
            previous3.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious4.TryGetValue(
                key, out ScreenDepthSamples previous4))
            previous4.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious5.TryGetValue(
                key, out ScreenDepthSamples previous5))
            previous5.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious6.TryGetValue(
                key, out ScreenDepthSamples previous6))
            previous6.FindClosest(expectedDepth, ref z, ref closestError);
        if (ScreenDepthPrevious7.TryGetValue(
                key, out ScreenDepthSamples previous7))
            previous7.FindClosest(expectedDepth, ref z, ref closestError);

        // V8 bins projected geometry at roughly SZ/8. A screen coordinate
        // shared by unrelated surfaces is common; accepting a depth from a
        // different OT range creates edge wedges and lets transparent planes
        // cross terrain. Keep a generous half-range for sloped polygons and
        // fall back to the primitive's OT depth when no correlated projection
        // is credible.
        int maximumError = Math.Max(512, expectedDepth / 2);
        return closestError <= maximumError;
    }

    static int CollectGroupsAt(
        int x,
        int y,
        int expectedDepth,
        int maximumError,
        Span<uint> groups)
    {
        int count = CollectGroupsAtExact(
            x, y, expectedDepth, maximumError, groups, 0);
        if (count != 0)
            return count;

        for (int oy = -1; oy <= 1 && count < groups.Length; oy++)
            for (int ox = -1; ox <= 1 && count < groups.Length; ox++)
            {
                if (ox == 0 && oy == 0)
                    continue;
                count = CollectGroupsAtExact(
                    x + ox, y + oy, expectedDepth, maximumError,
                    groups, count);
            }
        return count;
    }

    static int CollectSamplesAt(
        int x,
        int y,
        int expectedDepth,
        int maximumError,
        Span<ScreenDepthSample> samples)
    {
        int count = CollectSamplesAtExact(
            x, y, expectedDepth, maximumError, samples, 0);
        if (count != 0)
            return count;

        for (int oy = -1; oy <= 1 && count < samples.Length; oy++)
            for (int ox = -1; ox <= 1 && count < samples.Length; ox++)
            {
                if (ox == 0 && oy == 0)
                    continue;
                count = CollectSamplesAtExact(
                    x + ox, y + oy, expectedDepth, maximumError,
                    samples, count);
            }
        return count;
    }

    static int CollectSamplesAtExact(
        int x,
        int y,
        int expectedDepth,
        int maximumError,
        Span<ScreenDepthSample> samples,
        int count)
    {
        uint key = ScreenKey((short)x, (short)y);
        if (ScreenDepthCurrent.TryGetValue(
                key, out ScreenDepthSamples current))
            count = current.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious.TryGetValue(
                key, out ScreenDepthSamples previous))
            count = previous.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious2.TryGetValue(
                key, out ScreenDepthSamples previous2))
            count = previous2.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious3.TryGetValue(
                key, out ScreenDepthSamples previous3))
            count = previous3.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious4.TryGetValue(
                key, out ScreenDepthSamples previous4))
            count = previous4.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious5.TryGetValue(
                key, out ScreenDepthSamples previous5))
            count = previous5.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious6.TryGetValue(
                key, out ScreenDepthSamples previous6))
            count = previous6.CollectSamples(
                expectedDepth, maximumError, samples, count);
        if (ScreenDepthPrevious7.TryGetValue(
                key, out ScreenDepthSamples previous7))
            count = previous7.CollectSamples(
                expectedDepth, maximumError, samples, count);
        return count;
    }

    static int CollectGroupsAtExact(
        int x,
        int y,
        int expectedDepth,
        int maximumError,
        Span<uint> groups,
        int count)
    {
        uint key = ScreenKey((short)x, (short)y);
        if (ScreenDepthCurrent.TryGetValue(
                key, out ScreenDepthSamples current))
            count = current.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious.TryGetValue(
                key, out ScreenDepthSamples previous))
            count = previous.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious2.TryGetValue(
                key, out ScreenDepthSamples previous2))
            count = previous2.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious3.TryGetValue(
                key, out ScreenDepthSamples previous3))
            count = previous3.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious4.TryGetValue(
                key, out ScreenDepthSamples previous4))
            count = previous4.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious5.TryGetValue(
                key, out ScreenDepthSamples previous5))
            count = previous5.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious6.TryGetValue(
                key, out ScreenDepthSamples previous6))
            count = previous6.CollectGroups(
                expectedDepth, maximumError, groups, count);
        if (ScreenDepthPrevious7.TryGetValue(
                key, out ScreenDepthSamples previous7))
            count = previous7.CollectGroups(
                expectedDepth, maximumError, groups, count);
        return count;
    }

    static bool TryGetGroupAt(
        int x,
        int y,
        uint group,
        int expectedDepth,
        int maximumError,
        out ushort depth)
    {
        if (TryGetGroupAtExact(
                x, y, group, expectedDepth, maximumError, out depth))
            return true;
        for (int oy = -1; oy <= 1; oy++)
            for (int ox = -1; ox <= 1; ox++)
            {
                if (ox == 0 && oy == 0)
                    continue;
                if (TryGetGroupAtExact(
                        x + ox, y + oy, group, expectedDepth,
                        maximumError, out depth))
                    return true;
            }
        depth = 0;
        return false;
    }

    static bool TryGetGroupAtExact(
        int x,
        int y,
        uint group,
        int expectedDepth,
        int maximumError,
        out ushort depth)
    {
        depth = 0;
        uint key = ScreenKey((short)x, (short)y);
        if (ScreenDepthCurrent.TryGetValue(
                key, out ScreenDepthSamples current) &&
            current.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious.TryGetValue(
                key, out ScreenDepthSamples previous) &&
            previous.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious2.TryGetValue(
                key, out ScreenDepthSamples previous2) &&
            previous2.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious3.TryGetValue(
                key, out ScreenDepthSamples previous3) &&
            previous3.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious4.TryGetValue(
                key, out ScreenDepthSamples previous4) &&
            previous4.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious5.TryGetValue(
                key, out ScreenDepthSamples previous5) &&
            previous5.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        if (ScreenDepthPrevious6.TryGetValue(
                key, out ScreenDepthSamples previous6) &&
            previous6.TryGetGroup(
                group, expectedDepth, maximumError, out depth))
            return true;
        return ScreenDepthPrevious7.TryGetValue(
                   key, out ScreenDepthSamples previous7) &&
               previous7.TryGetGroup(
                   group, expectedDepth, maximumError, out depth);
    }

    public static int ScreenDepthCount =>
        ScreenDepthCurrent.Count +
        ScreenDepthPrevious.Count +
        ScreenDepthPrevious2.Count +
        ScreenDepthPrevious3.Count +
        ScreenDepthPrevious4.Count +
        ScreenDepthPrevious5.Count +
        ScreenDepthPrevious6.Count +
        ScreenDepthPrevious7.Count;

    public static void BeginFrame()
    {
        // The game builds one ordering table while the prior table is being
        // presented, so GPU packets can trail their GTE projections across
        // multiple host presents. Retain eight bounded generations without
        // the former 4096-entry linear scan.
        (ScreenDepthPrevious7,
         ScreenDepthPrevious6,
         ScreenDepthPrevious5,
         ScreenDepthPrevious4,
         ScreenDepthPrevious3,
         ScreenDepthPrevious2,
         ScreenDepthPrevious,
         ScreenDepthCurrent) =
            (ScreenDepthPrevious6,
             ScreenDepthPrevious5,
             ScreenDepthPrevious4,
             ScreenDepthPrevious3,
             ScreenDepthPrevious2,
             ScreenDepthPrevious,
             ScreenDepthCurrent,
             ScreenDepthPrevious7);
        ScreenDepthCurrent.Clear();
    }

    public static void Execute(uint cmd)
    {
        FLAG = 0;
        int sf = (cmd & (1u << 19)) != 0 ? 12 : 0;
        bool lm = (cmd & (1u << 10)) != 0;
        int mx = (int)((cmd >> 17) & 3);
        int vn = (int)((cmd >> 15) & 3);
        int cv = (int)((cmd >> 13) & 3);

        switch (cmd & 0x3F)
        {
            case 0x01:
                ActiveProjectionGroup = NextProjectionGroup();
                Rtp(V[0], V[1], V[2], sf, lm, true);
                break;
            case 0x30:
                ActiveProjectionGroup = NextProjectionGroup();
                Rtp(V[0], V[1], V[2], sf, lm, false);
                Rtp(V[3], V[4], V[5], sf, lm, false);
                Rtp(V[6], V[7], V[8], sf, lm, true);
                break;
            case 0x06:
                if (NoRetailCull &&
                    _terrainProjectionDepth == 0 &&
                    ConfigManager.View.HighResolution3D &&
                    GpuHle.GameplayActive)
                {
                    // Diagnostic. Never let the engine reject an object
                    // primitive on its own backface test, so a scene can be
                    // driven to see whether that test is what removes geometry
                    // beside the camera. Terrain is excluded: NCLIP is the only
                    // thing culling terrain back faces, and without it the
                    // ground renders as a checkerboard of undersides.
                    //
                    // Object back faces WILL be drawn with this on. It is not
                    // shippable; it answers one question.
                    MAC0 = 0x01000000;
                    break;
                }
                // NCLIP is game-visible GTE state. The engine branches on
                // MAC0 while traversing terrain, so replacing the native
                // integer-SXY result with host fractional coordinates changes
                // which faces the original game submits. Geometry correction
                // belongs after packet submission in the Enhanced renderer;
                // emulated CPU/GTE behavior remains bit-exact here.
                //
                // The one exception is the terrain transform. Terrain near the
                // horizon is a stack of strips a fraction of a native pixel
                // tall, and integer SXY rounding drives their three vertices
                // exactly collinear, so the engine's own zero-area rejection
                // deletes them. At 1x that removes something genuinely
                // sub-pixel; Enhanced draws the same strip from camera space
                // several real pixels tall, so the rejection punches a visible
                // slit along the horizon instead. Inside the terrain scope
                // only, measure the area with the fractional projection that
                // the renderer is actually going to draw.
                if (((TerrainPreciseNclip && _terrainProjectionDepth > 0) ||
                     (WidePreciseNclip &&
                      ConfigManager.View.HighResolution3D &&
                      GpuHle.GameplayActive &&
                      GpuHle.WideAspect > GpuHle.BaseAspect + 0.001f)) &&
                    SxyHasPrecisePosition[0] &&
                    SxyHasPrecisePosition[1] &&
                    SxyHasPrecisePosition[2])
                {
                    // Terrain keeps the clamped projection. That path was
                    // tuned against it to close the horizon slits, and feeding
                    // it unclamped values changes sign decisions the walker
                    // depends on - it shreds the ground. Only object geometry,
                    // where saturation is what deletes triangles beside the
                    // camera, uses the unclamped projection.
                    bool terrainScope = _terrainProjectionDepth > 0;
                    float[] ax = terrainScope ? SxyPreciseX : SxyUnclampedX;
                    float[] ay = terrainScope ? SxyPreciseY : SxyUnclampedY;
                    double area =
                        (double)ax[0] * (ay[1] - ay[2]) +
                        (double)ax[1] * (ay[2] - ay[0]) +
                        (double)ax[2] * (ay[0] - ay[1]);
                    // A strip whose true area is a fraction of a pixel still
                    // has to survive the engine's integer sign test, so keep
                    // the sign when truncation would erase it.
                    MAC0 = (int)Math.Clamp(
                        area >= 0d ? Math.Ceiling(area) : Math.Floor(area),
                        int.MinValue,
                        int.MaxValue);
                    long packed =
                        (long)SX[0] * (SY[1] - SY[2]) +
                        (long)SX[1] * (SY[2] - SY[0]) +
                        (long)SX[2] * (SY[0] - SY[1]);
                    // Only a polygon whose two tests disagree changes what the
                    // engine submits, so count those and nothing else.
                    if ((packed > 0) != (MAC0 > 0))
                    {
                        if (_terrainProjectionDepth > 0)
                            _nclipTerrainCorrections++;
                        else
                            _nclipObjectCorrections++;
                        // Rounding erased a front face the renderer would have
                        // drawn: this is the direction that punches holes.
                        if (MAC0 > 0)
                        {
                            _nclipRescued++;
                            if (MarkRescuedNclip)
                                RescuedTriangles.Add(RescueKey(
                                    SX[0], SY[0], SX[1], SY[1],
                                    SX[2], SY[2]));
                        }
                    }
                }
                else
                {
                    MAC0 = (int)CheckMac0(
                        (long)SX[0] * (SY[1] - SY[2]) +
                        (long)SX[1] * (SY[2] - SY[0]) +
                        (long)SX[2] * (SY[0] - SY[1]));
                }
                break;
            case 0x2D:
                MAC0 = (int)CheckMac0((long)ZSF3 * (SZ[1] + SZ[2] + SZ[3]));
                OTZ = (ushort)SatSZ(MAC0 >> 12);
                break;
            case 0x2E:
                MAC0 = (int)CheckMac0((long)ZSF4 * (SZ[0] + SZ[1] + SZ[2] + SZ[3]));
                OTZ = (ushort)SatSZ(MAC0 >> 12);
                break;
            case 0x12: Mvmva(sf, lm, mx, vn, cv); break;
            case 0x28:
                SetMac(1, (long)IR1 * IR1, sf, lm);
                SetMac(2, (long)IR2 * IR2, sf, lm);
                SetMac(3, (long)IR3 * IR3, sf, lm);
                break;
            case 0x0C:
                // OP reads all three source IR registers before writing any
                // result. SetMac also updates IRn, so retain the original
                // vector or MAC2/MAC3 incorrectly consume earlier results.
                int opIr1 = IR1, opIr2 = IR2, opIr3 = IR3;
                long opMac1 = (long)RT[4] * opIr3 - (long)RT[8] * opIr2;
                long opMac2 = (long)RT[8] * opIr1 - (long)RT[0] * opIr3;
                long opMac3 = (long)RT[0] * opIr2 - (long)RT[4] * opIr1;
                SetMac(1, opMac1, sf, lm);
                SetMac(2, opMac2, sf, lm);
                SetMac(3, opMac3, sf, lm);
                break;
            case 0x3D:
                SetMac(1, (long)IR0 * IR1, sf, lm);
                SetMac(2, (long)IR0 * IR2, sf, lm);
                SetMac(3, (long)IR0 * IR3, sf, lm);
                PushColor();
                break;
            case 0x3E:
                SetMac(1, ((long)MAC1 << sf) + (long)IR0 * IR1, sf, lm);
                SetMac(2, ((long)MAC2 << sf) + (long)IR0 * IR2, sf, lm);
                SetMac(3, ((long)MAC3 << sf) + (long)IR0 * IR3, sf, lm);
                PushColor();
                break;
            case 0x10: Interp((long)RGBC_R << 16, (long)RGBC_G << 16, (long)RGBC_B << 16, sf, lm); break;
            case 0x2A:
                for (int i = 0; i < 3; i++)
                    Interp((long)(RGB[0] & 0xFF) << 16, (long)((RGB[0] >> 8) & 0xFF) << 16, (long)((RGB[0] >> 16) & 0xFF) << 16, sf, lm);
                break;
            case 0x11: Interp((long)IR1 << 12, (long)IR2 << 12, (long)IR3 << 12, sf, lm); break;
            case 0x29: Interp(((long)RGBC_R * IR1) << 4, ((long)RGBC_G * IR2) << 4, ((long)RGBC_B * IR3) << 4, sf, lm); break;
            case 0x1E: Ncs(0, sf, lm); break;
            case 0x20: Ncs(0, sf, lm); Ncs(1, sf, lm); Ncs(2, sf, lm); break;
            case 0x13: Ncds(0, sf, lm); break;
            case 0x16: Ncds(0, sf, lm); Ncds(1, sf, lm); Ncds(2, sf, lm); break;
            case 0x1B: Nccs(0, sf, lm); break;
            case 0x3F: Nccs(0, sf, lm); Nccs(1, sf, lm); Nccs(2, sf, lm); break;
            case 0x1C:
                MatVec(LCM, BK[0], BK[1], BK[2], IR1, IR2, IR3, sf, lm);
                Modulate(sf, lm);
                break;
            case 0x14:
                MatVec(LCM, BK[0], BK[1], BK[2], IR1, IR2, IR3, sf, lm);
                Interp(((long)RGBC_R * IR1) << 4, ((long)RGBC_G * IR2) << 4, ((long)RGBC_B * IR3) << 4, sf, lm);
                break;
        }

        if ((FLAG & 0x7F87E000u) != 0) FLAG |= 0x80000000u;
    }

    static void Ncs(int vec, int sf, bool lm)
    {
        MatVec(LLM, 0, 0, 0, V[vec * 3], V[vec * 3 + 1], V[vec * 3 + 2], sf, lm);
        MatVec(LCM, BK[0], BK[1], BK[2], IR1, IR2, IR3, sf, lm);
        PushColor();
    }

    static void Ncds(int vec, int sf, bool lm)
    {
        MatVec(LLM, 0, 0, 0, V[vec * 3], V[vec * 3 + 1], V[vec * 3 + 2], sf, lm);
        MatVec(LCM, BK[0], BK[1], BK[2], IR1, IR2, IR3, sf, lm);
        Interp(((long)RGBC_R * IR1) << 4, ((long)RGBC_G * IR2) << 4, ((long)RGBC_B * IR3) << 4, sf, lm);
    }

    static void Nccs(int vec, int sf, bool lm)
    {
        MatVec(LLM, 0, 0, 0, V[vec * 3], V[vec * 3 + 1], V[vec * 3 + 2], sf, lm);
        MatVec(LCM, BK[0], BK[1], BK[2], IR1, IR2, IR3, sf, lm);
        Modulate(sf, lm);
    }

    static void Mvmva(int sf, bool lm, int mx, int vn, int cv)
    {
        short[] mat = mx == 0 ? RT : mx == 1 ? LLM : mx == 2 ? LCM : RT;
        int vx, vy, vz;
        if (vn < 3) { vx = V[vn * 3]; vy = V[vn * 3 + 1]; vz = V[vn * 3 + 2]; }
        else { vx = IR1; vy = IR2; vz = IR3; }

        if (cv == 2)
        {
            SatIR(1, (int)((((long)FC[0] << 12) + (long)mat[0] * vx) >> sf), lm);
            SatIR(2, (int)((((long)FC[1] << 12) + (long)mat[3] * vx) >> sf), lm);
            SatIR(3, (int)((((long)FC[2] << 12) + (long)mat[6] * vx) >> sf), lm);
            SetMac(1, (long)mat[1] * vy + (long)mat[2] * vz, sf, lm);
            SetMac(2, (long)mat[4] * vy + (long)mat[5] * vz, sf, lm);
            SetMac(3, (long)mat[7] * vy + (long)mat[8] * vz, sf, lm);
            return;
        }

        int t0 = 0, t1 = 0, t2 = 0;
        if (cv == 0) { t0 = TR[0]; t1 = TR[1]; t2 = TR[2]; }
        else if (cv == 1) { t0 = BK[0]; t1 = BK[1]; t2 = BK[2]; }
        MatVec(mat, t0, t1, t2, vx, vy, vz, sf, lm);
    }

    public static uint Read(int reg)
    {
        switch (reg)
        {
            case 0: return (uint)((ushort)V[0] | (V[1] << 16));
            case 1: return (uint)V[2];
            case 2: return (uint)((ushort)V[3] | (V[4] << 16));
            case 3: return (uint)V[5];
            case 4: return (uint)((ushort)V[6] | (V[7] << 16));
            case 5: return (uint)V[8];
            case 6: return (uint)(RGBC_R | (RGBC_G << 8) | (RGBC_B << 16) | (RGBC_CODE << 24));
            case 7: return OTZ;
            case 8: return (uint)IR0;
            case 9: return (uint)IR1;
            case 10: return (uint)IR2;
            case 11: return (uint)IR3;
            case 12: return (uint)((ushort)SX[0] | (SY[0] << 16));
            case 13: return (uint)((ushort)SX[1] | (SY[1] << 16));
            case 14:
            case 15: return (uint)((ushort)SX[2] | (SY[2] << 16));
            case 16: return SZ[0];
            case 17: return SZ[1];
            case 18: return SZ[2];
            case 19: return SZ[3];
            case 20: return RGB[0];
            case 21: return RGB[1];
            case 22: return RGB[2];
            case 23: return RES1;
            case 24: return (uint)MAC0;
            case 25: return (uint)MAC1;
            case 26: return (uint)MAC2;
            case 27: return (uint)MAC3;
            case 28:
            case 29:
                int r = Math.Clamp(IR1 >> 7, 0, 0x1F);
                int g = Math.Clamp(IR2 >> 7, 0, 0x1F);
                int b = Math.Clamp(IR3 >> 7, 0, 0x1F);
                return (uint)(r | (g << 5) | (b << 10));
            case 30: return LZCS;
            case 31: return LZCR;
            default: return 0;
        }
    }

    static uint NextProjectionGroup()
    {
        ScreenProjectionGroup++;
        if (ScreenProjectionGroup == 0)
            ScreenProjectionGroup++;
        return ScreenProjectionGroup;
    }

    public static void Write(int reg, uint val)
    {
        switch (reg)
        {
            case 0: V[0] = (short)val; V[1] = (short)(val >> 16); break;
            case 1: V[2] = (short)val; break;
            case 2: V[3] = (short)val; V[4] = (short)(val >> 16); break;
            case 3: V[5] = (short)val; break;
            case 4: V[6] = (short)val; V[7] = (short)(val >> 16); break;
            case 5: V[8] = (short)val; break;
            case 6: RGBC_R = (byte)val; RGBC_G = (byte)(val >> 8); RGBC_B = (byte)(val >> 16); RGBC_CODE = (byte)(val >> 24); break;
            case 7: OTZ = (ushort)val; break;
            case 8: IR0 = (short)val; break;
            case 9: IR1 = (short)val; break;
            case 10: IR2 = (short)val; break;
            case 11: IR3 = (short)val; break;
            case 12: SX[0] = (short)val; SY[0] = (short)(val >> 16); ClearPreciseFifoEntry(0); break;
            case 13: SX[1] = (short)val; SY[1] = (short)(val >> 16); ClearPreciseFifoEntry(1); break;
            case 14: SX[2] = (short)val; SY[2] = (short)(val >> 16); ClearPreciseFifoEntry(2); break;
            case 15:
                SX[0] = SX[1]; SY[0] = SY[1]; SX[1] = SX[2]; SY[1] = SY[2];
                SxyDepth[0] = SxyDepth[1]; SxyDepth[1] = SxyDepth[2];
                SxyPerspectiveW[0] = SxyPerspectiveW[1];
                SxyPerspectiveW[1] = SxyPerspectiveW[2];
                SxyPreciseX[0] = SxyPreciseX[1];
                SxyPreciseX[1] = SxyPreciseX[2];
                SxyPreciseY[0] = SxyPreciseY[1];
                SxyPreciseY[1] = SxyPreciseY[2];
                SxyViewX[0] = SxyViewX[1];
                SxyViewX[1] = SxyViewX[2];
                SxyViewY[0] = SxyViewY[1];
                SxyViewY[1] = SxyViewY[2];
                SxyViewZ[0] = SxyViewZ[1];
                SxyViewZ[1] = SxyViewZ[2];
                SxyProjectionCenterX[0] = SxyProjectionCenterX[1];
                SxyProjectionCenterX[1] = SxyProjectionCenterX[2];
                SxyProjectionCenterY[0] = SxyProjectionCenterY[1];
                SxyProjectionCenterY[1] = SxyProjectionCenterY[2];
                SxyProjectionScale[0] = SxyProjectionScale[1];
                SxyProjectionScale[1] = SxyProjectionScale[2];
                SxyHasPrecisePosition[0] = SxyHasPrecisePosition[1];
                SxyHasPrecisePosition[1] = SxyHasPrecisePosition[2];
                SX[2] = (short)val; SY[2] = (short)(val >> 16);
                ClearPreciseFifoEntry(2);
                break;
            case 16: SZ[0] = (ushort)val; break;
            case 17: SZ[1] = (ushort)val; break;
            case 18: SZ[2] = (ushort)val; break;
            case 19: SZ[3] = (ushort)val; break;
            case 20: RGB[0] = val; break;
            case 21: RGB[1] = val; break;
            case 22: RGB[2] = val; break;
            case 23: RES1 = val; break;
            case 24: MAC0 = (int)val; break;
            case 25: MAC1 = (int)val; break;
            case 26: MAC2 = (int)val; break;
            case 27: MAC3 = (int)val; break;
            case 28:
                IR1 = (int)((val & 0x1F) << 7);
                IR2 = (int)(((val >> 5) & 0x1F) << 7);
                IR3 = (int)(((val >> 10) & 0x1F) << 7);
                break;
            case 29: break;
            case 30:
                LZCS = val;
                uint test = (val & 0x80000000u) != 0 ? ~val : val;
                LZCR = (uint)(test == 0 ? 32 : System.Numerics.BitOperations.LeadingZeroCount(test));
                break;
            case 31: break;
        }
    }

    public static uint ReadControl(int reg)
    {
        switch (reg)
        {
            case 0: return (uint)((ushort)RT[0] | (RT[1] << 16));
            case 1: return (uint)((ushort)RT[2] | (RT[3] << 16));
            case 2: return (uint)((ushort)RT[4] | (RT[5] << 16));
            case 3: return (uint)((ushort)RT[6] | (RT[7] << 16));
            case 4: return (uint)RT[8];
            case 5: return (uint)TR[0];
            case 6: return (uint)TR[1];
            case 7: return (uint)TR[2];
            case 8: return (uint)((ushort)LLM[0] | (LLM[1] << 16));
            case 9: return (uint)((ushort)LLM[2] | (LLM[3] << 16));
            case 10: return (uint)((ushort)LLM[4] | (LLM[5] << 16));
            case 11: return (uint)((ushort)LLM[6] | (LLM[7] << 16));
            case 12: return (uint)LLM[8];
            case 13: return (uint)BK[0];
            case 14: return (uint)BK[1];
            case 15: return (uint)BK[2];
            case 16: return (uint)((ushort)LCM[0] | (LCM[1] << 16));
            case 17: return (uint)((ushort)LCM[2] | (LCM[3] << 16));
            case 18: return (uint)((ushort)LCM[4] | (LCM[5] << 16));
            case 19: return (uint)((ushort)LCM[6] | (LCM[7] << 16));
            case 20: return (uint)LCM[8];
            case 21: return (uint)FC[0];
            case 22: return (uint)FC[1];
            case 23: return (uint)FC[2];
            case 24: return (uint)OFX;
            case 25: return (uint)OFY;
            case 26: return (uint)(short)H;
            case 27: return (uint)DQA;
            case 28: return (uint)DQB;
            case 29: return (uint)ZSF3;
            case 30: return (uint)ZSF4;
            case 31:
                // The engine skips a primitive when the error summary bit is
                // set at this read. Counting how often that is true is the
                // only direct measure of whether the suppression reaches the
                // path that actually rejects geometry.
                FlagRegisterReads++;
                if ((FLAG & 0x80000000u) != 0u) FlagRegisterErrors++;
                return FLAG;
            default: return 0;
        }
    }

    public static void WriteControl(int reg, uint val)
    {
        switch (reg)
        {
            case 0: RT[0] = (short)val; RT[1] = (short)(val >> 16); break;
            case 1: RT[2] = (short)val; RT[3] = (short)(val >> 16); break;
            case 2: RT[4] = (short)val; RT[5] = (short)(val >> 16); break;
            case 3: RT[6] = (short)val; RT[7] = (short)(val >> 16); break;
            case 4: RT[8] = (short)val; break;
            case 5: TR[0] = (int)val; break;
            case 6: TR[1] = (int)val; break;
            case 7: TR[2] = (int)val; break;
            case 8: LLM[0] = (short)val; LLM[1] = (short)(val >> 16); break;
            case 9: LLM[2] = (short)val; LLM[3] = (short)(val >> 16); break;
            case 10: LLM[4] = (short)val; LLM[5] = (short)(val >> 16); break;
            case 11: LLM[6] = (short)val; LLM[7] = (short)(val >> 16); break;
            case 12: LLM[8] = (short)val; break;
            case 13: BK[0] = (int)val; break;
            case 14: BK[1] = (int)val; break;
            case 15: BK[2] = (int)val; break;
            case 16: LCM[0] = (short)val; LCM[1] = (short)(val >> 16); break;
            case 17: LCM[2] = (short)val; LCM[3] = (short)(val >> 16); break;
            case 18: LCM[4] = (short)val; LCM[5] = (short)(val >> 16); break;
            case 19: LCM[6] = (short)val; LCM[7] = (short)(val >> 16); break;
            case 20: LCM[8] = (short)val; break;
            case 21: FC[0] = (int)val; break;
            case 22: FC[1] = (int)val; break;
            case 23: FC[2] = (int)val; break;
            case 24: OFX = (int)val; break;
            case 25: OFY = (int)val; break;
            case 26: H = (ushort)val; break;
            case 27: DQA = (short)val; break;
            case 28: DQB = (int)val; break;
            case 29: ZSF3 = (short)val; break;
            case 30: ZSF4 = (short)val; break;
            case 31: FLAG = val & 0x7FFFF000u; if ((FLAG & 0x7F87E000u) != 0) FLAG |= 0x80000000u; break;
        }
    }

    public static void LoadWord(int reg, uint val) => Write(reg, val);

    /// <summary>
    /// Loads a COP2 data register from RAM without discarding exact projected
    /// vertex provenance. V8 caches projected SXY words, reloads them into the
    /// GTE FIFO for NCLIP/packet generation, then stores that FIFO into GPU
    /// packets. Treating LWC2 as a raw uint load loses the original
    /// unsaturated view-space vertex even though the address and value still
    /// identify it exactly.
    /// </summary>
    public static void LoadMemoryWord(
        int reg, IMemory memory, uint address)
    {
        uint value = memory.ReadU32(address);
        Write(reg, value);
        if (reg is not (12 or 13 or 14 or 15))
            return;
        if (!memory.TryGetPreciseGteVertex(
                address, value, out var vertex))
            return;

        int index = reg switch
        {
            12 => 0,
            13 => 1,
            _ => 2,
        };
        RestorePreciseFifoEntry(index, vertex);
    }

    static void RestorePreciseFifoEntry(
        int index, in PreciseGteVertexData vertex)
    {
        SxyDepth[index] = vertex.Depth;
        SxyPerspectiveW[index] = vertex.PerspectiveW;
        SxyPreciseX[index] = vertex.PreciseX;
        SxyPreciseY[index] = vertex.PreciseY;
        SxyViewX[index] = vertex.ViewX;
        SxyViewY[index] = vertex.ViewY;
        SxyViewZ[index] = vertex.ViewZ;
        SxyProjectionCenterX[index] = vertex.ProjectionCenterX;
        SxyProjectionCenterY[index] = vertex.ProjectionCenterY;
        SxyProjectionScale[index] = vertex.ProjectionScale;
        SxyHasPrecisePosition[index] = vertex.Valid;
    }

    public static uint StoreWord(int reg) => Read(reg);

    static void ClearPreciseFifoEntry(int index)
    {
        SxyDepth[index] = 0;
        SxyPerspectiveW[index] = 0;
        SxyPreciseX[index] = 0;
        SxyPreciseY[index] = 0;
        SxyViewX[index] = 0;
        SxyViewY[index] = 0;
        SxyViewZ[index] = 0;
        SxyProjectionCenterX[index] = 0;
        SxyProjectionCenterY[index] = 0;
        SxyProjectionScale[index] = 0;
        SxyHasPrecisePosition[index] = false;
    }

    public static bool TryGetStoreVertex(
        int reg, out PreciseGteVertexData vertex)
    {
        uint packedScreenPosition = Read(reg);
        int index = reg switch
        {
            12 => 0,
            13 => 1,
            14 or 15 => 2,
            _ => -1,
        };
        vertex = index >= 0
            ? new PreciseGteVertexData(
                packedScreenPosition,
                SxyDepth[index],
                SxyPerspectiveW[index],
                SxyViewX[index],
                SxyViewY[index],
                SxyViewZ[index],
                SxyPreciseX[index],
                SxyPreciseY[index],
                SxyProjectionCenterX[index],
                SxyProjectionCenterY[index],
                SxyProjectionScale[index])
            : default;
        return index >= 0 &&
            SxyHasPrecisePosition[index] &&
            vertex.Valid;
    }

    public static bool TryGetProjectionState(
        out float centerX,
        out float centerY,
        out float scale)
    {
        centerX = OFX / 65536.0f;
        centerY = OFY / 65536.0f;
        scale = H;
        return H > 0 &&
            float.IsFinite(centerX) &&
            float.IsFinite(centerY);
    }

    public static bool GetCondition() => false;
}
