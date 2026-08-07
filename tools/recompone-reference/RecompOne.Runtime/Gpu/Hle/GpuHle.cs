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
    static readonly List<PacketRange> ImportedVehiclePacketRanges = [];
    static readonly List<PacketRange> DreamlandWaterPacketRanges = [];
    static readonly List<OwnedPacketRange> OwnedPacketRanges = [];
    static readonly HashSet<uint> VehiclePackets = [];
    static readonly HashSet<uint> DreamlandWaterPackets = [];
    static readonly HashSet<uint> TerrainRoutePackets = [];
    static readonly Dictionary<uint, string> PacketOwners = [];
    static int _terrainRouteWriteScopeDepth;

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

    public static void RegisterImportedVehiclePacketRange(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;
        var range = new PacketRange(start, end);
        if (!ImportedVehiclePacketRanges.Contains(range))
            ImportedVehiclePacketRanges.Add(range);
    }

    public static bool IsImportedVehiclePacket(uint address)
    {
        address = NormalizePacketAddress(address);
        foreach (PacketRange range in ImportedVehiclePacketRanges)
            if (address >= range.Start && address < range.End)
                return true;
        return false;
    }

    public static void ClearVehiclePacketRanges()
    {
        VehiclePacketRanges.Clear();
        ImportedVehiclePacketRanges.Clear();
        VehiclePackets.Clear();
    }

    public static void RegisterVehiclePacket(uint address) =>
        VehiclePackets.Add(NormalizePacketAddress(address));

    public static void RegisterDreamlandWaterPacket(uint address) =>
        DreamlandWaterPackets.Add(NormalizePacketAddress(address));

    public static void RegisterDreamlandWaterPacketRange(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;
        var range = new PacketRange(start, end);
        if (!DreamlandWaterPacketRanges.Contains(range))
            DreamlandWaterPacketRanges.Add(range);
    }

    public static bool IsDreamlandWaterPacket(uint address)
    {
        address = NormalizePacketAddress(address);
        if (DreamlandWaterPackets.Contains(address))
            return true;
        foreach (PacketRange range in DreamlandWaterPacketRanges)
            if (address >= range.Start && address < range.End)
                return true;
        return false;
    }

    // The original engine has a dedicated route-strip renderer at
    // FUN_80040E38/FUN_80040E5C. PSMemory calls this observer for native
    // 32-bit packet writes only while that renderer is on the call stack.
    // This is source provenance: no tpage, CLUT, UV, colour, or arena-specific
    // signature is used to infer whether a primitive is a road.
    public static void BeginTerrainRoutePacketWrites() =>
        _terrainRouteWriteScopeDepth++;

    public static void EndTerrainRoutePacketWrites()
    {
        if (_terrainRouteWriteScopeDepth > 0)
            _terrainRouteWriteScopeDepth--;
    }

    public static void ObserveTerrainRoutePacketWrite(uint physicalAddress)
    {
        if (_terrainRouteWriteScopeDepth <= 0 ||
            physicalAddress >= Memory.MemoryMap.RamWindow)
            return;
        TerrainRoutePackets.Add(NormalizePacketAddress(physicalAddress));
    }

    public static bool IsTerrainRoutePacket(uint address) =>
        TerrainRoutePackets.Contains(NormalizePacketAddress(address));

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

    public static void BeginPacketArena(uint start, uint end)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end <= start)
            return;

        // V8:2 builds one display list while the other remains queued for
        // DrawOTag.  Retire ownership only for the arena being reused; a
        // global per-present clear races the game's double buffering and
        // strips material identity from the list about to be consumed.
        VehiclePacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        ImportedVehiclePacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        DreamlandWaterPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        OwnedPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        VehiclePackets.RemoveWhere(address =>
            address >= start && address < end);
        DreamlandWaterPackets.RemoveWhere(address =>
            address >= start && address < end);
        TerrainRoutePackets.RemoveWhere(address =>
            address >= start && address < end);
        foreach (uint address in PacketOwners.Keys
                     .Where(address => address >= start && address < end)
                     .ToArray())
            PacketOwners.Remove(address);
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
