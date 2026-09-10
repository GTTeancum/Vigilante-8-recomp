namespace RecompOne.Runtime.Hle;

public static class GpuHle
{
    public static bool Active { get; set; }
    public static IGpuBackend? Backend { get; set; }

    public static float WideAspect { get; set; }
    public static float OutputAspect { get; set; } = 4f / 3f;
    public static bool NativeResolution { get; set; }
    public static bool GameplayActive { get; set; }
    // Explicit scope for a viewport HUD submitted outside the retail top layout.
    public static bool ViewportHudActive { get; set; }
    public static bool WidescreenMenuReturnPending { get; set; }

    // Set for the frame in which the retail PAUSED / QUEST OBJECTIVES /
    // ARE YOU SURE? overlay is drawn. Those are centred modals, not HUD, and
    // must not be pulled toward the screen edge by widescreen HUD anchoring:
    // their pieces straddle the anchor's top-of-screen test, so part of the
    // panel moves and part does not, which tears the layout apart.
    // Held for a few frames rather than a single flag: the game builds a
    // frame's packets during logic and the backend draws them after the next
    // frame boundary, so a flag set by the text hook and cleared at that
    // boundary is always false by the time the rectangles are emitted.
    public static int NativeModalHold { get; set; }
    public static bool NativeModalActive => NativeModalHold > 0;
    public static void SignalNativeModal() => NativeModalHold = 4;
    static readonly HashSet<uint> NativeModalPanels = [];
    public static void RegisterNativeModalPanel(uint address) =>
        NativeModalPanels.Add(NormalizePacketAddress(address));
    public static bool IsNativeModalPanel(uint address) =>
        NativeModalPanels.Contains(NormalizePacketAddress(address));
    public static float TargetAspect { get; set; } = 4f / 3f;
    public static string? DebugCaptureLabel { get; set; }
    public static int DebugGameplayTick { get; set; }
    public const float BaseAspect = 4f / 3f;

    readonly record struct PacketRange(uint Start, uint End);
    readonly record struct OwnedPacketRange(
        uint Start,
        uint End,
        string Owner);
    public readonly record struct DreamcastWaterBaseVertex(
        float X,
        float Y,
        float CameraDepth);
    public readonly record struct DreamcastWaterBaseQuad(
        DreamcastWaterBaseVertex V0,
        DreamcastWaterBaseVertex V1,
        DreamcastWaterBaseVertex V2,
        DreamcastWaterBaseVertex V3,
        byte R,
        byte G,
        byte B);
    public readonly record struct DreamcastWaterSurfaceVertex(
        float X,
        float Y,
        float CameraDepth,
        float U,
        float V,
        float ViewX,
        float ViewY,
        float ViewZ,
        float ProjectionCenterX,
        float ProjectionCenterY,
        float ProjectionScale);
    public sealed record DreamcastWaterTexture(
        int Width,
        int Height,
        byte[] Rgba,
        ulong Revision,
        string Source);
    public sealed record DreamcastWaterSurfaceMesh(
        DreamcastWaterSurfaceVertex[] Vertices,
        int Rows,
        int Columns,
        DreamcastWaterTexture? Texture = null);
    readonly record struct DreamcastWaterBaseRange(
        uint Start,
        uint End,
        DreamcastWaterBaseQuad Quad);
    readonly record struct DreamcastWaterSurfaceRange(
        uint Start,
        uint End,
        DreamcastWaterSurfaceMesh Mesh);
    static readonly List<PacketRange> VehiclePacketRanges = [];
    static readonly List<PacketRange> WorldObjectPacketRanges = [];
    static readonly List<PacketRange> SkyPacketRanges = [];
    static readonly List<PacketRange> WaterBasePacketRanges = [];
    static readonly List<PacketRange> WaterSurfacePacketRanges = [];
    static readonly List<DreamcastWaterBaseRange> DreamcastWaterBaseRanges = [];
    static readonly List<DreamcastWaterSurfaceRange>
        DreamcastWaterSurfaceRanges = [];
    static DreamcastWaterTexture? _dreamcastWaterTexture;
    static ulong _dreamcastWaterTextureRevision;
    static readonly List<OwnedPacketRange> OwnedPacketRanges = [];
    static readonly HashSet<uint> VehiclePackets = [];
    static readonly HashSet<uint> VehicleReflectionPackets = [];
    static readonly HashSet<uint> TerrainRoutePackets = [];
    static readonly Dictionary<uint, CoarseTerrainPacket>
        CoarseTerrainPackets = [];
    static readonly Dictionary<uint, TerrainTransitionPacket>
        TerrainTransitionPackets = [];
    static readonly Dictionary<uint, TriangleNclipPacket>
        TriangleNclipPackets = [];
    static readonly HashSet<uint> TriangleNclipHeaderPending = [];
    static readonly Dictionary<uint, string> PacketOwners = [];
    static long _vehiclePacketLookups;
    static long _vehiclePacketExplicitHits;
    static long _vehiclePacketRangeHits;
    static long _vehiclePacketRangeTests;
    static long _vehiclePacketRangeRegistrations;
    static long _vehiclePacketRangeDuplicates;
    static readonly bool TracePacketArenas =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_PACKET_ARENAS") == "1";
    static int _terrainRouteWriteScopeDepth;
    readonly record struct TerrainRouteColorRamp(
        byte LowR, byte LowG, byte LowB,
        byte HighR, byte HighG, byte HighB);
    static TerrainRouteColorRamp? _terrainRouteColorRamp;

    public readonly record struct TerrainTextureDescriptor(
        byte TextureId,
        ushort Uv00,
        ushort Uv01,
        ushort Uv10,
        ushort Uv11,
        ushort TPage,
        ushort Clut,
        byte Flags,
        byte AverageR,
        byte AverageG,
        byte AverageB);

    public readonly record struct TerrainDistanceColor(
        byte BaseR,
        byte BaseG,
        byte BaseB,
        byte FarR,
        byte FarG,
        byte FarB);

    public readonly record struct TerrainQuadDistanceColors(
        TerrainDistanceColor TopLeft,
        TerrainDistanceColor TopRight,
        TerrainDistanceColor BottomLeft,
        TerrainDistanceColor BottomRight,
        bool Alternate,
        bool Valid,
        TerrainPatchGeometry? Patch = null)
    {
        public TerrainDistanceColor Get(bool secondHalf, int vertex) =>
            (secondHalf, vertex) switch
            {
                (false, 0) => TopLeft,
                (false, 1) => TopRight,
                (false, 2) => BottomLeft,
                (true, 0) => TopRight,
                (true, 1) => BottomLeft,
                _ => BottomRight,
            };
    }

    public readonly record struct TerrainCellTextures(
        TerrainTextureDescriptor[]? Tiles,
        int GridSize,
        bool Valid,
        TerrainQuadDistanceColors DistanceColors)
    {
        public TerrainTextureDescriptor Get(int x, int z) =>
            Tiles![x * GridSize + z];
    }

    public sealed record TerrainPatchGeometry(
        DreamcastTerrainGeometry.Sample[] Samples,
        System.Numerics.Vector3 HeightAxis,
        float ProjectionCenterX, float ProjectionCenterY, float ProjectionScale)
    {
        // Shared only by patches from one native terrain traversal. Retain
        // rejected coarse cells with the packets that own that camera snapshot.
        public List<CoarseTerrainPacket>? CulledCells { get; init; }
    }

    public readonly record struct CoarseTerrainPacket(
        TerrainCellTextures Textures,
        bool SecondHalf,
        string Source,
        uint X,
        uint Z);

    public readonly record struct TerrainTransitionPacket(
        TerrainCellTextures Textures,
        byte Ax,
        byte Az,
        byte Bx,
        byte Bz,
        byte Cx,
        byte Cz,
        string Source,
        uint X,
        uint Z);

    public readonly record struct TriangleNclipPacket(
        long Sequence,
        long PackedArea,
        double PreciseArea,
        bool HasPreciseArea,
        bool HasPacketOrder,
        byte Gte0PacketIndex,
        byte Gte1PacketIndex,
        byte Gte2PacketIndex,
        int X0,
        int Y0,
        int X1,
        int Y1,
        int X2,
        int Y2);

    public readonly record struct VehiclePacketOwnershipMetrics(
        int RangeCount,
        int ExplicitPacketCount,
        int ReflectionPacketCount,
        int OwnedRangeCount,
        long TotalRangeBytes,
        uint MaximumRangeBytes,
        long Lookups,
        long ExplicitHits,
        long RangeHits,
        long RangeTests,
        long Registrations,
        long DuplicateRegistrations);

    static uint NormalizePacketAddress(uint address)
    {
        uint ramSize = Runtime.Mode == RunMode.Devkit
            ? Memory.MemoryMap.DevkitRamSize
            : Memory.MemoryMap.RetailRamSize;
        return address & (ramSize - 1u);
    }

    public static void RegisterVehiclePacketRange(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;
        _vehiclePacketRangeRegistrations++;

        // Ownership is a union of packet intervals. Object and imported-group
        // scopes can be nested, so retaining every raw scope forces the hot OT
        // lookup through redundant ranges thousands of times per frame. Merge
        // overlapping/adjacent intervals at registration and keep the result
        // sorted; this preserves the exact address set while making lookup
        // logarithmic and independent of renderer nesting.
        uint mergedStart = start;
        uint mergedEnd = end;
        bool merged = false;
        for (int index = VehiclePacketRanges.Count - 1; index >= 0; index--)
        {
            PacketRange existing = VehiclePacketRanges[index];
            if (existing.End < mergedStart || existing.Start > mergedEnd)
                continue;
            mergedStart = Math.Min(mergedStart, existing.Start);
            mergedEnd = Math.Max(mergedEnd, existing.End);
            VehiclePacketRanges.RemoveAt(index);
            merged = true;
        }
        if (merged && mergedStart == start && mergedEnd == end)
            _vehiclePacketRangeDuplicates++;

        int insert = VehiclePacketRanges.BinarySearch(
            new PacketRange(mergedStart, mergedEnd),
            PacketRangeStartComparer.Instance);
        if (insert < 0)
            insert = ~insert;
        VehiclePacketRanges.Insert(
            insert,
            new PacketRange(mergedStart, mergedEnd));
    }

    public static void RegisterWorldObjectPacketRange(uint start, uint end) =>
        RegisterPacketRange(WorldObjectPacketRanges, start, end);

    public static bool IsWorldObjectPacket(uint address) =>
        IsPacketInRanges(WorldObjectPacketRanges, address);

    static void RegisterPacketRange(
        List<PacketRange> ranges,
        uint start,
        uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;

        uint mergedStart = start;
        uint mergedEnd = end;
        for (int index = ranges.Count - 1; index >= 0; index--)
        {
            PacketRange existing = ranges[index];
            if (existing.End < mergedStart || existing.Start > mergedEnd)
                continue;
            mergedStart = Math.Min(mergedStart, existing.Start);
            mergedEnd = Math.Max(mergedEnd, existing.End);
            ranges.RemoveAt(index);
        }

        int insert = ranges.BinarySearch(
            new PacketRange(mergedStart, mergedEnd),
            PacketRangeStartComparer.Instance);
        if (insert < 0)
            insert = ~insert;
        ranges.Insert(insert, new PacketRange(mergedStart, mergedEnd));
    }

    static bool IsPacketInRanges(List<PacketRange> ranges, uint address)
    {
        address = NormalizePacketAddress(address);
        int low = 0;
        int high = ranges.Count - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) >> 1);
            PacketRange range = ranges[middle];
            if (address < range.Start)
                high = middle - 1;
            else if (address >= range.End)
                low = middle + 1;
            else
                return true;
        }
        return false;
    }

    public static void RegisterSkyPacketRange(uint start, uint end) =>
        RegisterPacketRange(SkyPacketRanges, start, end);

    public static bool IsSkyPacket(uint address) =>
        IsPacketInRanges(SkyPacketRanges, address);

    public static void RegisterWaterBasePacketRange(uint start, uint end) =>
        RegisterPacketRange(WaterBasePacketRanges, start, end);

    public static void RegisterDreamcastWaterBaseQuad(
        uint start,
        uint end,
        in DreamcastWaterBaseQuad quad)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;
        DreamcastWaterBaseRanges.Add(new(start, end, quad));
    }

    public static bool TryGetDreamcastWaterBaseQuad(
        uint address,
        out uint rangeStart,
        out DreamcastWaterBaseQuad quad)
    {
        address = NormalizePacketAddress(address);
        for (int index = DreamcastWaterBaseRanges.Count - 1;
             index >= 0;
             index--)
        {
            DreamcastWaterBaseRange range = DreamcastWaterBaseRanges[index];
            if (address < range.Start || address >= range.End)
                continue;
            rangeStart = range.Start;
            quad = range.Quad;
            return true;
        }
        rangeStart = 0;
        quad = default;
        return false;
    }

    public static void RegisterWaterSurfacePacketRange(uint start, uint end) =>
        RegisterPacketRange(WaterSurfacePacketRanges, start, end);

    public static DreamcastWaterTexture? CurrentDreamcastWaterTexture =>
        _dreamcastWaterTexture;

    /// <summary>
    /// Decodes the level's authored XWAT TIM directly from its EA-IFF bytes.
    /// Selection is structural: neither a level name nor a runtime texture
    /// address participates, so relocated PS1 VRAM cannot break the binding.
    /// </summary>
    public static bool TryRegisterDreamcastWaterTexture(
        string source,
        ReadOnlySpan<byte> data)
    {
        static uint U32Le(ReadOnlySpan<byte> bytes, int offset) =>
            (uint)(bytes[offset] |
                bytes[offset + 1] << 8 |
                bytes[offset + 2] << 16 |
                bytes[offset + 3] << 24);
        static ushort U16Le(ReadOnlySpan<byte> bytes, int offset) =>
            (ushort)(bytes[offset] | bytes[offset + 1] << 8);
        static uint U32Be(ReadOnlySpan<byte> bytes, int offset) =>
            (uint)(bytes[offset] << 24 |
                bytes[offset + 1] << 16 |
                bytes[offset + 2] << 8 |
                bytes[offset + 3]);
        static byte Expand5(int value) =>
            (byte)(((value & 31) << 3) | ((value & 31) >> 2));

        for (int chunk = 0; chunk + 8 <= data.Length; chunk++)
        {
            if (data[chunk] != (byte)'X' ||
                data[chunk + 1] != (byte)'W' ||
                data[chunk + 2] != (byte)'A' ||
                data[chunk + 3] != (byte)'T')
                continue;
            uint payloadSize = U32Be(data, chunk + 4);
            if (payloadSize > int.MaxValue ||
                chunk + 8L + payloadSize > data.Length)
                continue;
            ReadOnlySpan<byte> tim = data.Slice(
                chunk + 8, checked((int)payloadSize));
            if (tim.Length < 32 || U32Le(tim, 0) != 0x10u ||
                U32Le(tim, 4) != 0x08u)
                continue;

            int clutSize = checked((int)U32Le(tim, 8));
            int clutWidth = U16Le(tim, 16);
            int clutHeight = U16Le(tim, 18);
            int imageHeader = 8 + clutSize;
            if (clutSize < 12 || clutWidth <= 0 || clutWidth > 16 ||
                clutHeight != 1 || imageHeader + 12 > tim.Length ||
                20 + clutWidth * 2 > imageHeader)
                continue;
            int imageSize = checked((int)U32Le(tim, imageHeader));
            int widthWords = U16Le(tim, imageHeader + 8);
            int height = U16Le(tim, imageHeader + 10);
            int width = widthWords * 4;
            int packed = imageHeader + 12;
            int packedBytes = checked(widthWords * 2 * height);
            if (imageSize != 12 + packedBytes || width <= 0 || height <= 0 ||
                packed + packedBytes > tim.Length)
                continue;

            ushort[] palette = new ushort[clutWidth];
            for (int index = 0; index < palette.Length; index++)
                palette[index] = U16Le(tim, 20 + index * 2);
            byte[] rgba = new byte[checked(width * height * 4)];
            int output = 0;
            for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++, output += 4)
            {
                int packedOffset = packed + y * widthWords * 2 + (x >> 1);
                int index = (tim[packedOffset] >> ((x & 1) * 4)) & 15;
                ushort pixel = index < palette.Length ? palette[index] : (ushort)0;
                rgba[output] = Expand5(pixel);
                rgba[output + 1] = Expand5(pixel >> 5);
                rgba[output + 2] = Expand5(pixel >> 10);
                rgba[output + 3] = (byte)((pixel & 0x8000) != 0 ? 255 : 0);
            }
            _dreamcastWaterTexture = new(
                width,
                height,
                rgba,
                ++_dreamcastWaterTextureRevision,
                source);
            Console.WriteLine(
                $"[DreamcastWater] asset-direct XWAT {width}x{height} " +
                $"source={source} revision={_dreamcastWaterTextureRevision}");
            return true;
        }
        return false;
    }

    public static void RegisterDreamcastWaterSurfaceMesh(
        uint start,
        uint end,
        DreamcastWaterSurfaceMesh mesh)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start || mesh.Vertices.Length == 0)
            return;
        DreamcastWaterSurfaceRanges.Add(new(start, end, mesh));
    }

    public static bool TryGetDreamcastWaterSurfaceMesh(
        uint address,
        out uint rangeStart,
        out DreamcastWaterSurfaceMesh? mesh)
    {
        address = NormalizePacketAddress(address);
        for (int index = DreamcastWaterSurfaceRanges.Count - 1;
             index >= 0;
             index--)
        {
            DreamcastWaterSurfaceRange range =
                DreamcastWaterSurfaceRanges[index];
            if (address < range.Start || address >= range.End)
                continue;
            rangeStart = range.Start;
            mesh = range.Mesh;
            return true;
        }
        rangeStart = 0;
        mesh = null;
        return false;
    }

    public static bool IsWaterBasePacket(uint address) =>
        IsPacketInRanges(WaterBasePacketRanges, address);

    public static bool IsWaterSurfacePacket(uint address) =>
        IsPacketInRanges(WaterSurfacePacketRanges, address);

    public static bool IsVehiclePacket(uint address)
    {
        _vehiclePacketLookups++;
        address = NormalizePacketAddress(address);
        if (VehiclePackets.Contains(address))
        {
            _vehiclePacketExplicitHits++;
            return true;
        }
        int low = 0;
        int high = VehiclePacketRanges.Count - 1;
        int tests = 0;
        while (low <= high)
        {
            tests++;
            int middle = low + ((high - low) >> 1);
            PacketRange range = VehiclePacketRanges[middle];
            if (address < range.Start)
            {
                high = middle - 1;
                continue;
            }
            if (address >= range.End)
            {
                low = middle + 1;
                continue;
            }

            _vehiclePacketRangeTests += tests;
            if (address >= range.Start && address < range.End)
            {
                _vehiclePacketRangeHits++;
                return true;
            }
        }
        _vehiclePacketRangeTests += tests;
        return false;
    }

    sealed class PacketRangeStartComparer : IComparer<PacketRange>
    {
        public static readonly PacketRangeStartComparer Instance = new();

        public int Compare(PacketRange x, PacketRange y)
        {
            int start = x.Start.CompareTo(y.Start);
            return start != 0 ? start : x.End.CompareTo(y.End);
        }
    }

    public static VehiclePacketOwnershipMetrics
        ConsumeVehiclePacketOwnershipMetrics()
    {
        long totalRangeBytes = 0;
        uint maximumRangeBytes = 0;
        foreach (PacketRange range in VehiclePacketRanges)
        {
            uint width = range.End - range.Start;
            totalRangeBytes += width;
            maximumRangeBytes = Math.Max(maximumRangeBytes, width);
        }

        var metrics = new VehiclePacketOwnershipMetrics(
            VehiclePacketRanges.Count,
            VehiclePackets.Count,
            VehicleReflectionPackets.Count,
            OwnedPacketRanges.Count,
            totalRangeBytes,
            maximumRangeBytes,
            _vehiclePacketLookups,
            _vehiclePacketExplicitHits,
            _vehiclePacketRangeHits,
            _vehiclePacketRangeTests,
            _vehiclePacketRangeRegistrations,
            _vehiclePacketRangeDuplicates);
        _vehiclePacketLookups = 0;
        _vehiclePacketExplicitHits = 0;
        _vehiclePacketRangeHits = 0;
        _vehiclePacketRangeTests = 0;
        _vehiclePacketRangeRegistrations = 0;
        _vehiclePacketRangeDuplicates = 0;
        return metrics;
    }

    public static void ClearVehiclePacketRanges()
    {
        VehiclePacketRanges.Clear();
        VehiclePackets.Clear();
        VehicleReflectionPackets.Clear();
    }

    public static void RegisterVehiclePacket(uint address) =>
        VehiclePackets.Add(NormalizePacketAddress(address));

    public static bool IsVehicleReflectionPacket(uint address) =>
        VehicleReflectionPackets.Contains(NormalizePacketAddress(address));

    public static void RegisterVehicleReflectionPacket(uint address) =>
        VehicleReflectionPackets.Add(NormalizePacketAddress(address));

    public static void RegisterTriangleNclipPacket(
        uint address,
        in TriangleNclipPacket packet)
    {
        address = NormalizePacketAddress(address);
        TriangleNclipPackets[address] = packet;
        // The shared emitter registers after writing the payload but just
        // before linking the packet into the ordering table.  That final link
        // writes the packet header at +0 and is not packet reuse; let exactly
        // that write pass without retiring the winding metadata.  A later
        // non-triangle reuse still retires the old entry at its header write,
        // while a triangle reuse replaces it before linking.
        TriangleNclipHeaderPending.Add(address);
    }

    public static bool TryGetTriangleNclipPacket(
        uint address,
        out TriangleNclipPacket packet) =>
        TriangleNclipPackets.TryGetValue(
            NormalizePacketAddress(address), out packet);

    // The original engine has a dedicated route-strip renderer at
    // FUN_80040E38/FUN_80040E5C. Its call-stack scope provides source
    // provenance without inferring material identity from tpage, CLUT, UV,
    // colour, or packet address.
    public static void BeginTerrainRoutePacketWrites() =>
        _terrainRouteWriteScopeDepth++;

    public static void EndTerrainRoutePacketWrites()
    {
        if (_terrainRouteWriteScopeDepth > 0)
            _terrainRouteWriteScopeDepth--;
    }

    public static void ObservePacketWrite(uint physicalAddress)
    {
        if (physicalAddress >= Memory.MemoryMap.RamWindow)
            return;

        uint address = NormalizePacketAddress(physicalAddress);
        // Coarse reconstruction metadata is attached after a complete packet
        // is emitted. Any later write to that packet address starts new
        // ownership, even when the replacement also belongs to terrain.
        CoarseTerrainPackets.Remove(address);
        NativeModalPanels.Remove(address);
        TerrainTransitionPackets.Remove(address);
        if (!TriangleNclipHeaderPending.Remove(address))
            TriangleNclipPackets.Remove(address);
        if (_terrainRouteWriteScopeDepth > 0)
        {
            TerrainRoutePackets.Add(address);
            return;
        }

        // V8 recycles its GPU packet arena continuously. Provenance belongs
        // to the renderer that produced the current bytes, not permanently to
        // a RAM address. A write outside the source scope therefore retires an
        // old route tag before that address can be submitted again.
        TerrainRoutePackets.Remove(address);
    }

    public static bool IsTerrainRoutePacket(uint address) =>
        TerrainRoutePackets.Contains(NormalizePacketAddress(address));

    public static void RegisterCoarseTerrainPacket(
        uint address,
        in TerrainCellTextures textures,
        bool secondHalf,
        string source,
        uint x,
        uint z)
    {
        if (!textures.Valid)
            return;
        CoarseTerrainPackets[NormalizePacketAddress(address)] =
            new CoarseTerrainPacket(textures, secondHalf, source, x, z);
    }

    public static bool TryGetCoarseTerrainPacket(
        uint address,
        out CoarseTerrainPacket packet) =>
        CoarseTerrainPackets.TryGetValue(
            NormalizePacketAddress(address), out packet);

    public static void RegisterTerrainTransitionPacket(
        uint address,
        in TerrainTransitionPacket packet)
    {
        if (!packet.Textures.Valid)
            return;
        uint normalized = NormalizePacketAddress(address);
        TerrainTransitionPackets[normalized] = packet;
    }

    public static bool TryGetTerrainTransitionPacket(
        uint address,
        out TerrainTransitionPacket packet) =>
        TerrainTransitionPackets.TryGetValue(
            NormalizePacketAddress(address), out packet);

    /// <summary>
    /// Installs the two authored COLS endpoints used by an N64 arena's route
    /// builder. Native PS1 route vertices retain the terrain lighting index as
    /// grayscale <c>index &lt;&lt; 2</c>; the N64 renderer instead expands that
    /// index through this inclusive 32-entry colour ramp.
    /// </summary>
    public static bool SetTerrainRouteColorRamp(
        byte lowR, byte lowG, byte lowB,
        byte highR, byte highG, byte highB)
    {
        var value = new TerrainRouteColorRamp(
            lowR, lowG, lowB, highR, highG, highB);
        if (_terrainRouteColorRamp == value)
            return false;
        _terrainRouteColorRamp = value;
        return true;
    }

    public static void ClearTerrainRouteColorRamp() =>
        _terrainRouteColorRamp = null;

    public static bool TerrainRouteColorRampActive =>
        _terrainRouteColorRamp is not null;

    static int InterpolateRouteChannel(byte low, byte high, int shade) =>
        low + ((high - low) * shade) / 124;

    /// <summary>
    /// Decodes one native route shade into the N64 vertex RGB domain. Authored
    /// terrain indices are stored as <c>index &lt;&lt; 2</c> (0..124). The
    /// original near-plane path at 0x80040e5c also averages pairs of those
    /// bytes when it creates midpoint vertices, so valid generated shades can
    /// lie between the four-unit index samples. Using 124 as the denominator
    /// is algebraically identical to the source signed /31 ramp at every
    /// authored index and preserves those generated fractional samples.
    /// </summary>
    public static bool TryDecodeTerrainRouteColor(
        byte red,
        byte green,
        byte blue,
        out byte mappedRed,
        out byte mappedGreen,
        out byte mappedBlue,
        out int index)
    {
        mappedRed = red;
        mappedGreen = green;
        mappedBlue = blue;
        index = -1;
        if (_terrainRouteColorRamp is not { } ramp ||
            red != green || red != blue || red > 124)
            return false;

        index = red >> 2;
        mappedRed = (byte)InterpolateRouteChannel(
            ramp.LowR, ramp.HighR, red);
        mappedGreen = (byte)InterpolateRouteChannel(
            ramp.LowG, ramp.HighG, red);
        mappedBlue = (byte)InterpolateRouteChannel(
            ramp.LowB, ramp.HighB, red);
        return true;
    }

    public static void RegisterPacketOwner(uint address, string owner) =>
        PacketOwners[NormalizePacketAddress(address)] = owner;

    public static void RegisterPacketOwnerRange(
        uint start,
        uint end,
        string owner)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;
        var range = new OwnedPacketRange(start, end, owner);
        if (!OwnedPacketRanges.Contains(range))
            OwnedPacketRanges.Add(range);
    }

    public static string DescribePacketOwner(uint address)
    {
        address = NormalizePacketAddress(address);
        if (PacketOwners.TryGetValue(address, out string? owner))
            return owner;

        // Ownership scopes are deliberately nested: a whole vehicle render
        // contains individual authored model/render-group writes.  The outer
        // scope closes last, so "most recently registered" incorrectly hides
        // the useful source provenance.  Prefer the narrowest containing
        // range; break equal-width ties in favour of the newest registration.
        OwnedPacketRange? best = null;
        uint bestWidth = uint.MaxValue;
        for (int index = OwnedPacketRanges.Count - 1; index >= 0; index--)
        {
            OwnedPacketRange range = OwnedPacketRanges[index];
            if (address < range.Start || address >= range.End)
                continue;
            uint width = range.End - range.Start;
            if (best is null || width < bestWidth)
            {
                best = range;
                bestWidth = width;
            }
        }
        return best?.Owner ?? "unresolved";
    }

    public static void ClearPacketOwners()
    {
        PacketOwners.Clear();
        OwnedPacketRanges.Clear();
    }

    public static void ResetSceneTracking()
    {
        NativeModalPanels.Clear();
        VehiclePacketRanges.Clear();
        WorldObjectPacketRanges.Clear();
        SkyPacketRanges.Clear();
        WaterBasePacketRanges.Clear();
        WaterSurfacePacketRanges.Clear();
        DreamcastWaterBaseRanges.Clear();
        DreamcastWaterSurfaceRanges.Clear();
        OwnedPacketRanges.Clear();
        VehiclePackets.Clear();
        VehicleReflectionPackets.Clear();
        TerrainRoutePackets.Clear();
        CoarseTerrainPackets.Clear();
        TerrainTransitionPackets.Clear();
        TriangleNclipPackets.Clear();
        TriangleNclipHeaderPending.Clear();
        PacketOwners.Clear();
        _terrainRouteWriteScopeDepth = 0;
        _terrainRouteColorRamp = null;
        DebugGameplayTick = 0;
        NativeModalHold = 0;
        Backend?.ResetTransientState();
    }

    public static void ResetMatchAtmosphere()
    {
        Backend?.ResetAtmosphereState();
    }

    public static void SetDreamcastFogColor(
        byte red, byte green, byte blue)
    {
        Backend?.SetDreamcastFogColor(red, green, blue);
    }

    public static void BeginPacketArena(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;

        int vehicleRangesBefore = VehiclePacketRanges.Count;
        int worldObjectRangesBefore = WorldObjectPacketRanges.Count;
        int skyRangesBefore = SkyPacketRanges.Count;
        int waterBaseRangesBefore = WaterBasePacketRanges.Count;
        int waterSurfaceRangesBefore = WaterSurfacePacketRanges.Count;
        int ownedRangesBefore = OwnedPacketRanges.Count;
        int vehiclePacketsBefore = VehiclePackets.Count;
        int reflectionPacketsBefore = VehicleReflectionPackets.Count;

        // V8:2 builds one display list while the other remains queued for
        // DrawOTag.  Retire ownership only for the arena being reused; a
        // global per-present clear races the game's double buffering and
        // strips material identity from the list about to be consumed.
        VehiclePacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        WorldObjectPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        SkyPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        WaterBasePacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        WaterSurfacePacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        DreamcastWaterBaseRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        DreamcastWaterSurfaceRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        OwnedPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        VehiclePackets.RemoveWhere(address =>
            address >= start && address < end);
        VehicleReflectionPackets.RemoveWhere(address =>
            address >= start && address < end);
        TerrainRoutePackets.RemoveWhere(address =>
            address >= start && address < end);
        foreach (uint address in CoarseTerrainPackets.Keys
                     .Where(address => address >= start && address < end)
                     .ToArray())
            CoarseTerrainPackets.Remove(address);
        foreach (uint address in TerrainTransitionPackets.Keys
                     .Where(address => address >= start && address < end)
                     .ToArray())
            TerrainTransitionPackets.Remove(address);
        foreach (uint address in TriangleNclipPackets.Keys
                     .Where(address => address >= start && address < end)
                     .ToArray())
            TriangleNclipPackets.Remove(address);
        TriangleNclipHeaderPending.RemoveWhere(address =>
            address >= start && address < end);
        foreach (uint address in PacketOwners.Keys
                     .Where(address => address >= start && address < end)
                     .ToArray())
            PacketOwners.Remove(address);
        if (TracePacketArenas)
            Console.Error.WriteLine(
                $"[PacketArenaRetire] gameplay={(GameplayActive ? 1 : 0)} " +
                $"range=0x{start:X8}..0x{end:X8} " +
                $"vehicle-ranges={vehicleRangesBefore}->{VehiclePacketRanges.Count} " +
                $"world-object-ranges={worldObjectRangesBefore}->{WorldObjectPacketRanges.Count} " +
                $"sky-ranges={skyRangesBefore}->{SkyPacketRanges.Count} " +
                $"water-base-ranges={waterBaseRangesBefore}->{WaterBasePacketRanges.Count} " +
                $"water-surface-ranges={waterSurfaceRangesBefore}->{WaterSurfacePacketRanges.Count} " +
                $"owned-ranges={ownedRangesBefore}->{OwnedPacketRanges.Count} " +
                $"vehicle-packets={vehiclePacketsBefore}->{VehiclePackets.Count} " +
                $"reflection-packets={reflectionPacketsBefore}->{VehicleReflectionPackets.Count}");
    }

    public struct DispRect { public int X, Y, W, H; public long Stamp; public bool Valid; }

    static readonly DispRect[] _rects = new DispRect[2];
    static long _stamp;

    public static void NotifyDisplay(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0) return;
        int slot = -1;
        for (int i = 0; i < _rects.Length; i++)
            if (_rects[i].Valid && _rects[i].X == x && _rects[i].Y == y) { slot = i; break; }
        if (slot < 0)
        {
            slot = 0;
            for (int i = 1; i < _rects.Length; i++)
                if (!_rects[i].Valid || _rects[i].Stamp < _rects[slot].Stamp) slot = i;
        }
        _rects[slot] = new DispRect { X = x, Y = y, W = w, H = h, Stamp = ++_stamp, Valid = true };
    }

    public static int RectCount => _rects.Length;

    public static long DisplayStamp => _stamp;

    public static DispRect GetRect(int i) => _rects[i];

    public static int WideMargin(int w)
    {
        if (WideAspect <= 0f) return 0;
        int wide = (int)MathF.Ceiling(w * WideAspect / BaseAspect);
        return Math.Max(0, (wide - w + 1) / 2);
    }
}
