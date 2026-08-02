namespace RecompOne.Runtime.Hle;

public static class GpuHle
{
    public static bool Active { get; set; }
    public static IGpuBackend? Backend { get; set; }

    public static float WideAspect { get; set; }
    public static float OutputAspect { get; set; } = 4f / 3f;
    public static bool NativeResolution { get; set; }
    public static bool GameplayActive { get; set; }
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
    static readonly Dictionary<uint, string> PacketOwners = [];

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
        if (end > start)
            VehiclePacketRanges.Add(new PacketRange(start, end));
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
    }

    public static void RegisterVehiclePacket(uint address) =>
        VehiclePackets.Add(NormalizePacketAddress(address));

    public static void RegisterPacketOwner(uint address, string owner) =>
        PacketOwners[NormalizePacketAddress(address)] = owner;

    public static void RegisterPacketOwnerRange(
        uint start,
        uint end,
        string owner)
    {
        start = NormalizePacketAddress(start);
        end = NormalizePacketAddress(end);
        if (end > start)
            OwnedPacketRanges.Add(new OwnedPacketRange(start, end, owner));
    }

    public static string DescribePacketOwner(uint address)
    {
        address = NormalizePacketAddress(address);
        if (PacketOwners.TryGetValue(address, out string? owner))
            return owner;
        for (int index = OwnedPacketRanges.Count - 1; index >= 0; index--)
        {
            OwnedPacketRange range = OwnedPacketRanges[index];
            if (address >= range.Start && address < range.End)
                return range.Owner;
        }
        return "unresolved";
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
        OwnedPacketRanges.RemoveAll(range =>
            range.Start < end && range.End > start);
        VehiclePackets.RemoveWhere(address =>
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
