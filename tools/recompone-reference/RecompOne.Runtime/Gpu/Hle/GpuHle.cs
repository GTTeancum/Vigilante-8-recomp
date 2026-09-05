namespace RecompOne.Runtime.Hle;

public static class GpuHle
{
    public static bool Active { get; set; }
    public static IGpuBackend? Backend { get; set; }

    public static float WideAspect { get; set; }
    public static float OutputAspect { get; set; } = 4f / 3f;
    public static bool NativeResolution { get; set; }
    public static bool GameplayActive { get; set; }
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
    static readonly List<PacketRange> VehiclePacketRanges = [];
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

    public readonly record struct TerrainCellTextures(
        TerrainTextureDescriptor[]? Tiles,
        int GridSize,
        bool Valid)
    {
        public TerrainTextureDescriptor Get(int x, int z) =>
            Tiles![x * GridSize + z];
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
        var range = new PacketRange(start, end);
        if (!VehiclePacketRanges.Contains(range))
            VehiclePacketRanges.Add(range);
    }

    public static bool IsVehiclePacket(uint address)
    {
        address = NormalizePacketAddress(address);
        if (VehiclePackets.Contains(address))
            return true;
        foreach (PacketRange range in VehiclePacketRanges)
            if (address >= range.Start && address < range.End)
                return true;
        return false;
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

    public static void BeginPacketArena(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;

        int vehicleRangesBefore = VehiclePacketRanges.Count;
        int ownedRangesBefore = OwnedPacketRanges.Count;
        int vehiclePacketsBefore = VehiclePackets.Count;
        int reflectionPacketsBefore = VehicleReflectionPackets.Count;

        // V8:2 builds one display list while the other remains queued for
        // DrawOTag.  Retire ownership only for the arena being reused; a
        // global per-present clear races the game's double buffering and
        // strips material identity from the list about to be consumed.
        VehiclePacketRanges.RemoveAll(range =>
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

    public static DispRect GetRect(int i) => _rects[i];

    public static int WideMargin(int w)
    {
        if (WideAspect <= 0f) return 0;
        int wide = (int)MathF.Ceiling(w * WideAspect / BaseAspect);
        return Math.Max(0, (wide - w + 1) / 2);
    }
}
