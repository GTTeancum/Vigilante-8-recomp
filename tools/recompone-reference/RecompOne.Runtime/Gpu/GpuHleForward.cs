using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Config;

namespace RecompOne.Runtime;

public sealed partial class Gpu
{
    static bool HleOn => GpuHle.Active && GpuHle.Backend is { Ready: true };
    static readonly bool TraceVram =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VRAM") == "1";
    static readonly bool TraceTerrainPrims =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_TERRAIN_PRIMS") == "1";
    static readonly (int Start, int End)? TraceGameplayTicks =
        ParseTraceGameplayTicks(
            Environment.GetEnvironmentVariable("RECOMPONE_TRACE_GAMEPLAY_TICKS"));
    static readonly bool TracePacketOwners =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_PACKET_OWNERS") == "1";
    static readonly bool TraceMeshes =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_MESHES") == "1";
    static readonly bool TraceN64RouteColors =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_N64_ROUTE_COLORS") == "1";
    static readonly bool DumpVramLoads =
        Environment.GetEnvironmentVariable("RECOMPONE_DUMP_VRAM_LOADS") == "1";
    static readonly string DumpVramLoadDir =
        Environment.GetEnvironmentVariable("RECOMPONE_VRAM_LOAD_DUMP_DIR") ??
        "vram_load_dump";
    static readonly (int Width, int Height)? DumpVramLoadSize =
        ParseDumpSize(Environment.GetEnvironmentVariable(
            "RECOMPONE_VRAM_LOAD_DUMP_SIZE"));
    static int s_vramLoadDumpIndex;
    static int _terrainPrimTraceCount;
    static bool _terrainCellTraced;
    bool DitherEnabled => _dither && ConfigManager.View.Ps1Dithering;
    int _currentOtDepth;
    uint _currentOtPacketAddress;
    bool _currentOtPacketVehicle;
    bool _currentOtPacketVehicleReflection;
    bool _currentOtPacketTerrainRoute;
    int _traceVehiclePacketHits;
    int _traceVehicleTriangles;
    int _tracePacketOwnershipLookups;
    int _traceResolvedPacketOwnershipLookups;
    int _traceVramLoadSequence;
    uint _traceVramLoadHash;
    ushort _traceVramLoadOr;
    int _traceGameplayTick = -1;
    int _traceGameplayTickPrimitives;
    int _traceGameplayTickLines;
    int _traceN64RouteColorVertices;
    long _n64RouteColorMappedVertices;
    long _n64RouteColorRejectedVertices;
    int _traceN64RouteColorSummaryTick = -1;

    static (int Start, int End)? ParseTraceGameplayTicks(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        string[] parts = value.Split('-', 2);
        if (!int.TryParse(parts[0], out int start)) return null;
        int end = start;
        if (parts.Length == 2 && !int.TryParse(parts[1], out end))
            return null;
        return (Math.Min(start, end), Math.Max(start, end));
    }

    static bool IntersectsVramRect(
        int x, int y, int w, int h,
        int probeX, int probeY, int probeW, int probeH) =>
        x < probeX + probeW && x + w > probeX &&
        y < probeY + probeH && y + h > probeY;

    void TraceVramTransfer(string kind, int x, int y, int w, int h)
    {
        if (!TraceVram) return;
        bool suspectClut =
            IntersectsVramRect(x, y, w, h, 320, 256, 256, 1);
        bool suspectPage =
            IntersectsVramRect(x, y, w, h, 896, 0, 128, 256);
        Console.Error.WriteLine(
            $"[V82VramTransfer] {kind} xy={x},{y} size={w}x{h} " +
            $"suspect-clut={suspectClut} suspect-page={suspectPage}");
    }

    public void BeginOrderingTable()
    {
        _currentOtDepth = 0;
        _projectiveDepthCache.Clear();
    }

    public void SetOrderingTableDepth(int depth) =>
        _currentOtDepth = Math.Max(1, depth);

    public void SetOrderingTablePacket(uint address, uint wordCount = 0)
    {
        _currentOtPacketAddress = address;
        _currentOtPacketVehicle = GpuHle.IsVehiclePacket(address);
        _currentOtPacketVehicleReflection =
            GpuHle.IsVehicleReflectionPacket(address);
        _currentOtPacketTerrainRoute =
            GpuHle.IsTerrainRoutePacket(address);
        if (GpuHle.GameplayActive &&
            TracePacketOwners &&
            wordCount != 0)
        {
            string owner = GpuHle.DescribePacketOwner(address);
            bool resolved = owner != "unresolved";
            if ((resolved && _traceResolvedPacketOwnershipLookups++ < 4096) ||
                (!resolved && _tracePacketOwnershipLookups++ < 4096))
                Console.Error.WriteLine(
                    $"[EnhancedPacketOwner] address=0x{address:X8} " +
                    $"words={wordCount} " +
                    $"vehicle={(_currentOtPacketVehicle ? 1 : 0)} " +
                    $"reflection={(_currentOtPacketVehicleReflection ? 1 : 0)} " +
                    $"terrain-route={(_currentOtPacketTerrainRoute ? 1 : 0)} " +
                    $"owner={owner}");
        }
        if (GpuHle.GameplayActive && _currentOtPacketVehicle &&
            TraceMeshes &&
            _traceVehiclePacketHits++ < 32)
            Console.Error.WriteLine($"[V8VehiclePacketHit] address=0x{address:X8}");
    }

    public void EndOrderingTable()
    {
        if (TraceN64RouteColors &&
            GpuHle.GameplayActive &&
            GpuHle.DebugGameplayTick >= 0 &&
            GpuHle.DebugGameplayTick % 120 == 0 &&
            _traceN64RouteColorSummaryTick != GpuHle.DebugGameplayTick)
        {
            _traceN64RouteColorSummaryTick = GpuHle.DebugGameplayTick;
            Console.Error.WriteLine(
                $"[V8N64RouteColorSummary] " +
                $"tick={GpuHle.DebugGameplayTick} " +
                $"mapped={_n64RouteColorMappedVertices} " +
                $"rejected={_n64RouteColorRejectedVertices}");
        }
        _currentOtDepth = 0;
        _currentOtPacketAddress = 0;
        _currentOtPacketVehicle = false;
        _currentOtPacketVehicleReflection = false;
        _currentOtPacketTerrainRoute = false;
        _projectiveDepthCache.Clear();
    }

    int CurTPage() => ((_texPageX / 64) & 0xf) | (((_texPageY / 256) & 1) << 4)
                    | ((_blendMode & 3) << 5) | ((_texDepth & 3) << 7);

    static bool IsNativeVehicleGlassClut(int clut)
    {
        // Native V8:2 vehicle panes use a dedicated authored CLUT row. Keep
        // this as material provenance; do not infer glass from sampled colour.
        int row = clut >> 6;
        return row == 509;
    }

    HleDrawEnv CurEnv() => new()
    {
        ClipX0 = _drawAreaLeft, ClipY0 = _drawAreaTop, ClipX1 = _drawAreaRight, ClipY1 = _drawAreaBottom,
        TwMaskX = _texWinMaskX, TwMaskY = _texWinMaskY, TwOffX = _texWinOffX, TwOffY = _texWinOffY,
        SetMask = _setMask, CheckMask = _checkMask, Dither = DitherEnabled,
    };

    HleVertex HV(in Vert v, in PrimFlags flags)
    {
        HleVertex result = new()
        {
            SourceAddress = v.SourceAddress,
            // This path exists only for the Enhanced renderer. Stock
            // geometry continues through Gpu.RasterTriangle and therefore
            // retains native integer SXY.
            X = v.HasPrecisePosition ? v.PreciseX : v.X,
            Y = v.HasPrecisePosition ? v.PreciseY : v.Y,
            R = (byte)v.R, G = (byte)v.G, B = (byte)v.B,
            U = (short)v.U, V = (short)v.V,
            Z = v.Z, HasGteZ = v.HasGteZ,
            HasCoherentGteZ = v.HasCoherentGteZ,
            PerspectiveW = v.PerspectiveW,
            HasProjectiveW = v.HasProjectiveW,
            ViewX = v.ViewX, ViewY = v.ViewY, ViewZ = v.ViewZ,
            ProjectionCenterX = v.ProjectionCenterX,
            ProjectionCenterY = v.ProjectionCenterY,
            ProjectionScale = v.ProjectionScale,
            HasViewSpace = v.HasViewSpace,
            ReconstructedViewSpace = v.ReconstructedViewSpace,
        };
        if (!flags.N64RouteColor)
            return result;

        if (GpuHle.TryDecodeTerrainRouteColor(
                result.R,
                result.G,
                result.B,
                out byte mappedRed,
                out byte mappedGreen,
                out byte mappedBlue,
                out int index))
        {
            _n64RouteColorMappedVertices++;
            if (TraceN64RouteColors &&
                _traceN64RouteColorVertices++ < 4096)
                Console.Error.WriteLine(
                    $"[V8N64RouteVertexColor] " +
                    $"tick={GpuHle.DebugGameplayTick} " +
                    $"packet=0x{flags.PacketAddress:X8} " +
                    $"encoded={result.R},{result.G},{result.B} " +
                    $"index={index} substep={result.R & 3} " +
                    $"mapped={mappedRed},{mappedGreen},{mappedBlue}");
            result.R = mappedRed;
            result.G = mappedGreen;
            result.B = mappedBlue;
        }
        else
        {
            _n64RouteColorRejectedVertices++;
            if (TraceN64RouteColors &&
                _n64RouteColorRejectedVertices <= 256)
                Console.Error.WriteLine(
                    $"[V8N64RouteVertexColorReject] " +
                    $"tick={GpuHle.DebugGameplayTick} " +
                    $"packet=0x{flags.PacketAddress:X8} " +
                    $"encoded={result.R},{result.G},{result.B}");
        }
        return result;
    }

    PrimFlags PrimOf(
        bool tex,
        bool semi,
        bool raw,
        int clut,
        bool gouraud = false)
    {
        HleMaterialKind material;
        bool nativeVehicleGlass = tex && IsNativeVehicleGlassClut(clut);
        if (_currentOtPacketTerrainRoute)
            material = HleMaterialKind.TerrainRoute;
        else if (_currentOtPacketVehicle)
            material = _currentOtPacketVehicleReflection && tex
                ? HleMaterialKind.VehicleReflection
                : nativeVehicleGlass
                    ? HleMaterialKind.Glass
                : semi && tex
                ? _blendMode switch
                {
                    // Vehicle render scopes also emit stock projected
                    // shadows and emissive effects. Their tpage blend mode is
                    // the native material contract; treating every
                    // semitransparent vehicle packet as glass made the
                    // subtractive 0x04A shadow atlas into an opaque floor
                    // card beneath imported previews.
                    1 => HleMaterialKind.Additive,
                    2 => HleMaterialKind.Subtractive,
                    _ => HleMaterialKind.Glass,
                }
                : tex
                    ? HleMaterialKind.AlphaTest
                    : HleMaterialKind.Opaque;
        else if (semi && tex)
            material = _blendMode switch
            {
                1 => HleMaterialKind.Additive,
                2 => HleMaterialKind.Subtractive,
                _ => HleMaterialKind.Particle,
            };
        else
            material = tex
                ? HleMaterialKind.AlphaTest
                : HleMaterialKind.Opaque;

        return new PrimFlags
        {
            Textured = tex,
            SemiTrans = semi,
            RawTexture = raw,
            Gouraud = gouraud,
            TPage = (ushort)CurTPage(),
            Clut = (ushort)clut,
            OtIndex = _currentOtDepth,
            PacketAddress = _currentOtPacketAddress,
            Vehicle = _currentOtPacketVehicle,
            TerrainRoute = _currentOtPacketTerrainRoute,
            N64RouteColor =
                _currentOtPacketTerrainRoute &&
                GpuHle.TerrainRouteColorRampActive,
            N64RouteDepthCompare =
                _currentOtPacketTerrainRoute &&
                GpuHle.TerrainRouteColorRampActive,
            Material = material,
        };
    }

    void HleTri(in Vert a, in Vert b, in Vert c, bool tex, bool gouraud, bool semi, bool raw, int clut)
    {
        int spanX = Math.Max(a.X, Math.Max(b.X, c.X)) - Math.Min(a.X, Math.Min(b.X, c.X));
        int spanY = Math.Max(a.Y, Math.Max(b.Y, c.Y)) - Math.Min(a.Y, Math.Min(b.Y, c.Y));
        bool modernGeometry =
            a.HasViewSpace && b.HasViewSpace && c.HasViewSpace;
        if (!modernGeometry && (spanX > 1023 || spanY > 511)) return;

        int gameplayTick = GpuHle.DebugGameplayTick;
        if (_traceGameplayTick != gameplayTick)
        {
            _traceGameplayTick = gameplayTick;
            _traceGameplayTickPrimitives = 0;
            _traceGameplayTickLines = 0;
        }
        if (TraceGameplayTicks is { } ticks &&
            gameplayTick >= ticks.Start &&
            gameplayTick <= ticks.End &&
            _traceGameplayTickPrimitives++ < 4096)
            Console.Error.WriteLine(
                $"[V8TickTriangle] tick={gameplayTick} " +
                $"packet=0x{_currentOtPacketAddress:X8} ot={_currentOtDepth} " +
                $"texdepth={_texDepth} page={_texPageX},{_texPageY} " +
                $"tpage=0x{CurTPage():X4} clut=0x{clut:X4} " +
                $"tex={(tex ? 1 : 0)} semi={(semi ? 1 : 0)} " +
                $"raw={(raw ? 1 : 0)} " +
                $"xy=({a.X},{a.Y}),({b.X},{b.Y}),({c.X},{c.Y}) " +
                $"uv=({a.U},{a.V}),({b.U},{b.V}),({c.U},{c.V}) " +
                $"rgb=({a.R},{a.G},{a.B}),({b.R},{b.G},{b.B}),({c.R},{c.G},{c.B}) " +
                $"z=({a.Z},{b.Z},{c.Z}) " +
                $"gte=({(a.HasGteZ ? 1 : 0)},{(b.HasGteZ ? 1 : 0)}," +
                $"{(c.HasGteZ ? 1 : 0)}) " +
                $"w=({a.PerspectiveW},{b.PerspectiveW},{c.PerspectiveW}) " +
                $"projective=({(a.HasProjectiveW ? 1 : 0)}," +
                $"{(b.HasProjectiveW ? 1 : 0)}," +
                $"{(c.HasProjectiveW ? 1 : 0)}) " +
                $"owner={GpuHle.DescribePacketOwner(_currentOtPacketAddress)}");

        if (_currentOtPacketVehicle &&
            Environment.GetEnvironmentVariable("RECOMPONE_TRACE_MESHES") == "1" &&
            _traceVehicleTriangles++ < 128)
            Console.Error.WriteLine(
                $"[V8VehicleTriangle] xy=({a.X},{a.Y}),({b.X},{b.Y}),({c.X},{c.Y}) " +
                $"tex={(tex ? 1 : 0)} tpage=0x{CurTPage():X4} clut=0x{clut:X4}");

        if (TraceTerrainPrims && tex && _texDepth == 1 &&
            clut is >= 0x4000 and < 0x4800 &&
            _texPageX is >= 704 and <= 896 &&
            _terrainPrimTraceCount++ < 256)
        {
            Console.Error.WriteLine(
                $"[V8TerrainPrim] tpage=0x{CurTPage():X4} page={_texPageX},{_texPageY} " +
                $"clut=0x{clut:X4} uv=" +
                $"({a.U},{a.V}),({b.U},{b.V}),({c.U},{c.V}) " +
                $"xy=({a.X},{a.Y}),({b.X},{b.Y}),({c.X},{c.Y}) " +
                $"rgb=({a.R},{a.G},{a.B}),({b.R},{b.G},{b.B}),({c.R},{c.G},{c.B})");

            int minU = Math.Min(a.U, Math.Min(b.U, c.U));
            int maxU = Math.Max(a.U, Math.Max(b.U, c.U));
            int minV = Math.Min(a.V, Math.Min(b.V, c.V));
            int maxV = Math.Max(a.V, Math.Max(b.V, c.V));
            if (!_terrainCellTraced && minU == 0 && maxU == 47 &&
                maxV - minV == 47)
            {
                _terrainCellTraced = true;
                int clutX = (clut & 0x3F) << 4;
                int clutY = (clut >> 6) & 0x1FF;
                ushort[] palette = new ushort[256];
                ushort[] texels = new ushort[24 * 48];
                GpuHle.Backend!.ReadVram(clutX, clutY, 256, 1, palette);
                GpuHle.Backend.ReadVram(
                    _texPageX, _texPageY + minV, 24, 48, texels);
                byte[] paletteBytes = new byte[palette.Length * 2];
                byte[] texelBytes = new byte[texels.Length * 2];
                Buffer.BlockCopy(palette, 0, paletteBytes, 0, paletteBytes.Length);
                Buffer.BlockCopy(texels, 0, texelBytes, 0, texelBytes.Length);
                Console.Error.WriteLine(
                    $"[V8TerrainCell] page={_texPageX},{_texPageY} " +
                    $"uv=0,{minV},47,{maxV} clut={clutX},{clutY} " +
                    $"palette={Convert.ToBase64String(paletteBytes)} " +
                    $"texels={Convert.ToBase64String(texelBytes)}");
            }
        }

        var be = GpuHle.Backend!;
        be.SetDrawEnv(CurEnv());
        PrimFlags flags = PrimOf(tex, semi, raw, clut, gouraud);
        bool hasWorldProjection =
            a.HasViewSpace || b.HasViewSpace || c.HasViewSpace ||
            a.HasGteZ || b.HasGteZ || c.HasGteZ ||
            a.HasProjectiveW || b.HasProjectiveW || c.HasProjectiveW;
        int minX = Math.Min(a.X, Math.Min(b.X, c.X));
        int maxX = Math.Max(a.X, Math.Max(b.X, c.X));
        int minY = Math.Min(a.Y, Math.Min(b.Y, c.Y));
        int maxY = Math.Max(a.Y, Math.Max(b.Y, c.Y));
        // V8:2's weather/lighting overlay is emitted as a native stack of
        // one-scanline, full-display semitransparent polygons.  It is an
        // explicit screen effect, not world geometry: occasional XY values
        // aliasing old GTE stores must not make those scanlines enter modern
        // projection/depth/fog.
        bool nativeScreenEffect =
            GpuHle.GameplayActive &&
            !flags.Vehicle &&
            semi && !tex &&
            minX <= 0 && maxX >= 320 &&
            (maxY - minY <= 1 || (minY <= 0 && maxY >= 239));
        bool screenSpaceOverlay =
            !flags.Vehicle &&
            (!GpuHle.GameplayActive ||
             (_currentOtDepth >= 0x800 && !hasWorldProjection));
        bool misownedGameplayCaption =
            GpuHle.GameplayActive &&
            flags.Vehicle &&
            _currentOtDepth >= 0x800 &&
            !hasWorldProjection &&
            tex &&
            minY >= 110 &&
            minY <= 230;
        if (screenSpaceOverlay || misownedGameplayCaption)
            flags.Material = HleMaterialKind.Ui;
        else if (nativeScreenEffect)
            flags.Material = HleMaterialKind.ScreenEffect;
        else if (GpuHle.GameplayActive &&
                 flags.Vehicle &&
                 hasWorldProjection &&
                 maxX - minX <= 260 &&
                 maxY - minY <= 180 &&
                flags.Material == HleMaterialKind.Glass)
            flags.Material = HleMaterialKind.OpaqueVehicleGlass;
        be.DrawTri(HV(a, flags), HV(b, flags), HV(c, flags), flags);
    }

    void HleRect(int x, int y, int w, int h, int u, int v, int clut, int r, int g, int b, bool tex, bool semi, bool raw)
    {
        var be = GpuHle.Backend!;
        be.SetDrawEnv(CurEnv());
        PrimFlags flags = PrimOf(tex, semi, raw, clut);
        flags.Material = HleMaterialKind.Ui;
        be.DrawRect(new HleRect
        {
            X = x,
            Y = y,
            W = w,
            H = h,
            U = (short)u,
            V = (short)v,
            R = (byte)r,
            G = (byte)g,
            B = (byte)b,
            FlipX = _texFlipX,
            FlipY = _texFlipY,
        },
            flags);
    }

    void HleLine(int x0, int y0, int r0, int g0, int b0, int x1, int y1, int r1, int g1, int b1, bool semi, bool gouraud)
    {
        if (Math.Abs(x1 - x0) > 1023 || Math.Abs(y1 - y0) > 511) return;

        int gameplayTick = GpuHle.DebugGameplayTick;
        if (_traceGameplayTick != gameplayTick)
        {
            _traceGameplayTick = gameplayTick;
            _traceGameplayTickPrimitives = 0;
            _traceGameplayTickLines = 0;
        }
        if (TraceGameplayTicks is { } ticks &&
            gameplayTick >= ticks.Start &&
            gameplayTick <= ticks.End &&
            _traceGameplayTickLines++ < 4096)
            Console.Error.WriteLine(
                $"[V8TickLine] tick={gameplayTick} " +
                $"packet=0x{_currentOtPacketAddress:X8} ot={_currentOtDepth} " +
                $"xy=({x0},{y0}),({x1},{y1}) " +
                $"rgb=({r0},{g0},{b0}),({r1},{g1},{b1}) " +
                $"semi={(semi ? 1 : 0)} gouraud={(gouraud ? 1 : 0)} " +
                $"owner={GpuHle.DescribePacketOwner(_currentOtPacketAddress)}");

        var be = GpuHle.Backend!;
        be.SetDrawEnv(CurEnv());
        PrimFlags flags = PrimOf(false, semi, false, 0, gouraud);
        if (!GpuHle.GameplayActive || _currentOtDepth >= 0x800)
            flags.Material = HleMaterialKind.Ui;
        be.DrawLine(
            new HleVertex { X = x0, Y = y0, R = (byte)r0, G = (byte)g0, B = (byte)b0 },
            new HleVertex { X = x1, Y = y1, R = (byte)r1, G = (byte)g1, B = (byte)b1 },
            flags);
    }

    void HleFill(int x, int y, int w, int h, ushort color)
    {
        TraceVramTransfer($"fill color=0x{color:X4}", x, y, w, h);
        GpuHle.Backend!.FillRect(x, y, w, h, color);
    }

    void HleCopy(int sx, int sy, int dx, int dy, int w, int h)
    {
        if (TraceVram)
        {
            TraceVramTransfer($"copy-src to={dx},{dy}", sx, sy, w, h);
            TraceVramTransfer($"copy-dst from={sx},{sy}", dx, dy, w, h);
        }
        var backend = GpuHle.Backend!;
        if (!_checkMask && !_setMask)
        {
            backend.CopyVram(sx, sy, dx, dy, w, h);
            return;
        }

        int count = w * h;
        if (_hleCopySource.Length < count)
            _hleCopySource = new ushort[count];
        if (_hleCopyDest.Length < count)
            _hleCopyDest = new ushort[count];
        Span<ushort> source = _hleCopySource.AsSpan(0, count);
        Span<ushort> dest = _hleCopyDest.AsSpan(0, count);
        backend.ReadVram(sx, sy, w, h, source);
        backend.ReadVram(dx, dy, w, h, dest);
        for (int i = 0; i < count; i++)
        {
            if (_checkMask && (dest[i] & 0x8000) != 0) continue;
            ushort value = source[i];
            if (_setMask) value |= 0x8000;
            dest[i] = value;
        }
        backend.WriteVram(dx, dy, w, h, dest);
    }

    ushort[] _readBuf = Array.Empty<ushort>();
    ushort[] _hleCopySource = Array.Empty<ushort>();
    ushort[] _hleCopyDest = Array.Empty<ushort>();

    void HleReadback(int x, int y, int w, int h)
    {
        int n = w * h;
        if (_readBuf.Length < n) _readBuf = new ushort[n];
        GpuHle.Backend!.ReadVram(x, y, w, h, _readBuf);
    }

    //img load
    ushort[] _hleLoad = Array.Empty<ushort>();
    ushort[] _hleLoadDest = Array.Empty<ushort>();
    bool _hleLoadActive;
    int _hleLoadPos;

    void HleLoadBegin()
    {
        _hleLoadActive = HleOn;
        _traceVramLoadSequence++;
        _traceVramLoadHash = 2166136261u;
        _traceVramLoadOr = 0;
        if (TraceVram)
            TraceVramTransfer(
                $"load-begin #{_traceVramLoadSequence}",
                _loadX, _loadY, _loadW, _loadH);
        if (!_hleLoadActive) return;
        int n = _loadW * _loadH;
        if (_hleLoad.Length < n) _hleLoad = new ushort[n];
        _hleLoadPos = 0;
    }

    void HleLoadPut(ushort value)
    {
        if (TraceVram)
        {
            _traceVramLoadHash =
                (_traceVramLoadHash ^ (byte)value) * 16777619u;
            _traceVramLoadHash =
                (_traceVramLoadHash ^ (byte)(value >> 8)) * 16777619u;
            _traceVramLoadOr |= value;
        }
        if (_hleLoadActive && _hleLoadPos < _hleLoad.Length) _hleLoad[_hleLoadPos++] = value;
    }

    void HleLoadFlush()
    {
        if (TraceVram)
        {
            Console.Error.WriteLine(
                $"[V82VramTransfer] load-end #{_traceVramLoadSequence} " +
                $"xy={_loadX},{_loadY} size={_loadW}x{_loadH} " +
                $"or=0x{_traceVramLoadOr:X4} fnv=0x{_traceVramLoadHash:X8} " +
                $"hle={_hleLoadActive} set-mask={_setMask} " +
                $"check-mask={_checkMask}");
            if (_hleLoadActive &&
                _loadX == 32 && _loadY is >= 482 and <= 498 &&
                _loadW == 232 && _hleLoadPos >= 232)
            {
                int previewCount = _loadY == 497 ? 232 : 32;
                Console.Error.WriteLine(
                    $"[V82TerrainPalette] y={_loadY} " +
                    string.Join(' ', _hleLoad.AsSpan(0, previewCount).ToArray()
                        .Select(value => $"{value:X4}")));
            }
        }
        if (!_hleLoadActive) return;
        int count = _loadW * _loadH;
        ReadOnlySpan<ushort> upload = _hleLoad.AsSpan(0, count);
        ushort[]? replacement = Enhanced.VramImageReplacements.Resolve(
            _loadX, _loadY, _loadW, _loadH, upload);
        if (replacement != null) upload = replacement;
        if (_checkMask)
        {
            if (_hleLoadDest.Length < count)
                _hleLoadDest = new ushort[count];
            Span<ushort> dest = _hleLoadDest.AsSpan(0, count);
            GpuHle.Backend!.ReadVram(
                _loadX, _loadY, _loadW, _loadH, dest);
            for (int i = 0; i < count; i++)
                if ((dest[i] & 0x8000) == 0)
                    dest[i] = upload[i];
            upload = dest;
        }
        DumpVramLoad(upload);
        GpuHle.Backend!.WriteVram(
            _loadX, _loadY, _loadW, _loadH,
            upload);
        _hleLoadActive = false;
    }

    void DumpVramLoad(ReadOnlySpan<ushort> upload)
    {
        if (!DumpVramLoads || _loadW <= 0 || _loadH <= 0) return;
        if (DumpVramLoadSize is { } size &&
            (_loadW != size.Width || _loadH != size.Height)) return;
        if (_loadW * _loadH != upload.Length) return;

        Directory.CreateDirectory(DumpVramLoadDir);
        uint hash = 2166136261u;
        for (int i = 0; i < upload.Length; i++)
        {
            ushort value = upload[i];
            hash = (hash ^ (byte)value) * 16777619u;
            hash = (hash ^ (byte)(value >> 8)) * 16777619u;
        }
        int index = Interlocked.Increment(ref s_vramLoadDumpIndex) - 1;
        string path = Path.Combine(
            DumpVramLoadDir,
            $"vram_load_{index:0000}_{_loadX}_{_loadY}_{_loadW}x{_loadH}_{hash:X8}.ppm");
        using var fs = File.Create(path);
        byte[] header = System.Text.Encoding.ASCII.GetBytes(
            $"P6\n{_loadW} {_loadH}\n255\n");
        fs.Write(header);
        byte[] rgb = new byte[_loadW * _loadH * 3];
        for (int i = 0; i < upload.Length; i++)
            WriteRgb555(rgb, i * 3, upload[i]);
        fs.Write(rgb);
    }

    static (int Width, int Height)? ParseDumpSize(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;
        string[] parts = value.Split('x', 'X');
        if (parts.Length != 2 ||
            !int.TryParse(parts[0], out int width) ||
            !int.TryParse(parts[1], out int height) ||
            width <= 0 || height <= 0)
        {
            Console.Error.WriteLine(
                $"[GPU] ignored invalid RECOMPONE_VRAM_LOAD_DUMP_SIZE '{value}'");
            return null;
        }
        return (width, height);
    }

    static void WriteRgb555(byte[] rgb, int offset, ushort value)
    {
        int r = value & 0x1F;
        int g = (value >> 5) & 0x1F;
        int b = (value >> 10) & 0x1F;
        rgb[offset + 0] = (byte)((r << 3) | (r >> 2));
        rgb[offset + 1] = (byte)((g << 3) | (g >> 2));
        rgb[offset + 2] = (byte)((b << 3) | (b >> 2));
    }
}
