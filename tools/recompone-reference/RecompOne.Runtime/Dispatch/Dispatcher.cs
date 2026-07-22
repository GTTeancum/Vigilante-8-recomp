using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;
using BiosKernel = RecompOne.Runtime.Bios.Bios;

namespace RecompOne.Runtime.Dispatch;

public static class Dispatcher
{
    static readonly Dictionary<string, IOverlay> _registry = new(StringComparer.OrdinalIgnoreCase);
    static readonly Dictionary<int, string> _lbaToName = [];
    static readonly List<string> _active = [];
    static readonly Dictionary<uint, Action<CpuContext, IMemory>> _funcMap = [];
    static readonly Dictionary<uint, (IOverlay Overlay, uint Delta)> _relocatedFunctions = [];
    static readonly Dictionary<uint, uint> _relocatedAliases = [];
    static readonly Dictionary<uint, (uint Base, uint Size, uint Delta)> _objectOwners = [];
    private static IOverlay? _pending;
    public static void Register(string name, IOverlay overlay)
    {
        _registry[name] = overlay;
        if (overlay.LbaStart >= 0) _lbaToName[overlay.LbaStart] = name;
    }

    public static string[] ActiveNames
    {
        get { lock (_active) return _active.ToArray(); }
    }
    
    public static IReadOnlyDictionary<string, IOverlay> Overlays => _registry;

    public static IMemory UnwrapMemory(IMemory memory) =>
        memory is RelocatedMemory relocated ? relocated.Inner : memory;

    public static void AssociateObjectOwner(uint address, IMemory memory)
    {
        if (address == 0u || memory is not RelocatedMemory relocated) return;
        _objectOwners[address] = (relocated.LinkedBase, relocated.Size, relocated.Delta);
    }

    public static void ForgetObjectOwner(uint address) => _objectOwners.Remove(address);

    public static (uint Base, uint Size)[] LinkedAddressRanges() =>
        _registry.Values
            .Where(overlay => overlay.Base != 0u && overlay.Size != 0u)
            .Select(overlay => (overlay.Base, overlay.Size))
            .Distinct()
            .ToArray();

    public static void LoadByLba(int lba)
    {
        if (!_lbaToName.TryGetValue(lba, out var name)) return;
        var overlay = _registry[name];
        if(overlay.Base == 0) {
            Load(name);
            return;
        }

        _pending = overlay;
    }

    public static void NotifyWrite(uint phys)
    {
        var p = _pending;
        if (p == null) return;
        uint start = p.Base & 0x1FFFFFFFu;
        if (phys < start || phys >= start + 0x800u) return;
        _pending = null;
        Load(p.Name);
    }
    public static void ClearPending() => _pending = null;

    public static void Load(string name)
    {
        if (!_registry.TryGetValue(name, out var overlay))
            throw new KeyNotFoundException($"overlay not registered: {name}");

        bool already;
        lock (_active) already = _active.Remove(name);

        if (!already) HandleRegionOverwrites(overlay);

        lock (_active) _active.Add(name);
        foreach (var (addr, fn) in overlay.Functions)
            _funcMap[addr] = fn;

        if (already) return;
        Runtime.OverlayLog.Record(name, OverlayEventKind.Loaded);
        Console.WriteLine($"[Dispatcher] loaded overlay: {name}");
    }

    static void HandleRegionOverwrites(IOverlay overlay)
    {
        uint newStart = overlay.Base & 0x1FFFFFFFu;
        uint newEnd = newStart + overlay.Size;
        bool hasRegion = overlay.Base != 0 && overlay.Size != 0;

        List<string>? overwritten = null;
        List<(string Name, int Funcs)>? vramCollisions = null;

        lock (_active)
        {
            foreach (var activeName in _active)
            {
                var other = _registry[activeName];
                bool otherHasRegion = other.Base != 0 && other.Size != 0;

                if (hasRegion && otherHasRegion)
                {
                    uint s = other.Base & 0x1FFFFFFFu;
                    uint e = s + other.Size;

                    if (s < newEnd && e > newStart)
                    {
                        if (s >= newStart && e <= newEnd)
                        {
                            overwritten ??= [];
                            overwritten.Add(activeName);
                        }
                        continue;
                    }
                }

                int shared = CountSharedFunctions(overlay, other);
                if (shared > 0)
                {
                    vramCollisions ??= [];
                    vramCollisions.Add((activeName, shared));
                }
            }

            if (overwritten != null)
                foreach (var d in overwritten) _active.Remove(d);
        }

        if (overwritten != null)
        {
            Rebuild();
            foreach (var d in overwritten)
            {
                Runtime.OverlayLog.Record(d, OverlayEventKind.Overwritten, overlay.Name);
                Console.WriteLine($"[Dispatcher] overlay {d} overwritten by {overlay.Name}");
            }
        }

        if (vramCollisions != null)
        {
            foreach (var (otherName, n) in vramCollisions)
            {
                Runtime.OverlayLog.Record(overlay.Name, OverlayEventKind.VramCollision, $"{otherName} ({n} funcs)");
                Console.WriteLine($"[Dispatcher] overlay {overlay.Name} vvram colision with {otherName}: {n} functions");
            }
        }
    }

    static int CountSharedFunctions(IOverlay a, IOverlay b)
    {
        var smaller = a.Functions.Count <= b.Functions.Count ? a : b;
        var larger = ReferenceEquals(smaller, a) ? b : a;

        int n = 0;
        foreach (var addr in smaller.Functions.Keys)
            if (larger.Functions.ContainsKey(addr)) n++;
        return n;
    }

    public static void TryLoad(string name)
    {
        if (_registry.ContainsKey(name))
            Load(name);
    }

    public static void Unload(string name)
    {
        bool removed;
        lock (_active) removed = _active.Remove(name);
        if (!removed) return;
        Rebuild();
        Runtime.OverlayLog.Record(name, OverlayEventKind.Unloaded);
    }

    public static void Call(CpuContext c, IMemory m, uint addr)
    {
        if (BiosKernel.TryDispatch(c, m, addr)) return;
        bool baseMemoryCall = m is not RelocatedMemory;
        (uint Base, uint Size, uint Delta) owner = default;
        bool hasOwner = baseMemoryCall && _objectOwners.TryGetValue(c.A0, out owner);
        if (m is RelocatedMemory linked &&
            addr >= linked.LinkedBase && addr - linked.LinkedBase < linked.Size)
            addr += linked.Delta;
        else if (hasOwner && addr >= owner.Base && addr - owner.Base < owner.Size)
            addr += owner.Delta;
        else if (!_funcMap.ContainsKey(addr) && _relocatedAliases.TryGetValue(addr, out uint aliasDelta))
            addr += aliasDelta;
        if (!_funcMap.ContainsKey(addr))
            TryLoadRelocatedOverlay(m, addr);
        if (!_funcMap.TryGetValue(addr, out var fn))
            throw new InvalidOperationException(
                $"unmapped call: 0x{addr:X8}; {DescribeCallContext(c, m)}; " +
                $"overlay images: {DescribeOverlayImages(m, addr)}");
        IMemory callMemory = m;
        if (_relocatedFunctions.TryGetValue(addr, out var relocation) && relocation.Delta != 0)
        {
            if (m is not RelocatedMemory current ||
                !current.Matches(relocation.Overlay.Base, relocation.Overlay.Size, relocation.Delta))
            {
                callMemory = new RelocatedMemory(m, relocation.Overlay.Base,
                    relocation.Overlay.Size, relocation.Delta);
            }
        }
        else if (hasOwner)
        {
            callMemory = new RelocatedMemory(m, owner.Base, owner.Size, owner.Delta);
        }
        fn(c, callMemory);
    }

    static void TryLoadRelocatedOverlay(IMemory m, uint addr)
    {
        const uint RamBase = 0x80000000u;
        const uint RamSize = 0x00200000u;
        if (addr < RamBase || addr >= RamBase + RamSize) return;

        IMemory baseMemory = UnwrapMemory(m);
        foreach (var overlay in _registry.Values)
        {
            if (overlay.Base == 0u || overlay.ImageSize == 0u) continue;
            foreach (uint original in overlay.Functions.Keys)
            {
                uint offset = original - overlay.Base;
                if (addr < RamBase + offset) continue;
                uint candidateBase = addr - offset;
                if (candidateBase < RamBase || candidateBase >= RamBase + RamSize) continue;
                if (baseMemory.ReadU32(candidateBase) != overlay.ImageSize) continue;

                RegisterRelocatedOverlay(overlay, candidateBase);
                return;
            }
        }

        uint lower = addr > RamBase + RamSize ? addr - RamSize : RamBase;
        uint candidate = (addr & ~3u) - 4u;
        while (candidate >= lower)
        {
            if (m.ReadU32(candidate + 4u) == addr)
            {
                uint offset = addr - candidate;
                foreach (var overlay in _registry.Values)
                {
                    if (overlay.Base == 0 || !overlay.Functions.ContainsKey(overlay.Base + offset))
                        continue;

                    RegisterRelocatedOverlay(overlay, candidate);
                    return;
                }
            }

            if (candidate < lower + 4u) break;
            candidate -= 4u;
        }
    }

    static void RegisterRelocatedOverlay(IOverlay overlay, uint actualBase)
    {
        uint delta = actualBase - overlay.Base;
        foreach (var (original, function) in overlay.Functions)
        {
            _funcMap[original + delta] = function;
            _relocatedFunctions[original + delta] = (overlay, delta);
            _relocatedAliases[original] = delta;
        }

        lock (_active)
            if (!_active.Contains(overlay.Name)) _active.Add(overlay.Name);
        Runtime.OverlayLog.Record(overlay.Name, OverlayEventKind.Loaded,
            $"relocated by 0x{delta:X8}");
        Console.WriteLine($"[Dispatcher] loaded relocated overlay: {overlay.Name} base=0x{actualBase:X8} delta=0x{delta:X8}");
    }

    static string DescribeOverlayImages(IMemory memory, uint target)
    {
        const uint ramBase = 0x80000000u;
        const uint ramEnd = 0x80200000u;
        IMemory baseMemory = UnwrapMemory(memory);
        var sizes = _registry.Values
            .Where(overlay => overlay.ImageSize != 0u)
            .GroupBy(overlay => overlay.ImageSize)
            .ToDictionary(group => group.Key, group => group.ToArray());
        var found = new List<string>();

        for (uint address = ramBase; address < ramEnd; address += 4u)
        {
            uint value = baseMemory.ReadU32(address);
            if (!sizes.TryGetValue(value, out var overlays)) continue;
            foreach (var overlay in overlays)
            {
                int offset = unchecked((int)(target - address));
                found.Add($"{overlay.Name}@0x{address:X8}/targetOffset=0x{offset:X}");
            }
        }

        return found.Count == 0 ? "none" : string.Join(", ", found);
    }

    static string DescribeCallContext(CpuContext c, IMemory memory)
    {
        IMemory baseMemory = UnwrapMemory(memory);
        uint physical = c.A0 & 0x1FFFFFFFu;
        if (physical >= 0x00200000u || physical > 0x00200000u - 0xA0u)
            return $"a0=0x{c.A0:X8} a1=0x{c.A1:X8} a2=0x{c.A2:X8} ra=0x{c.RA:X8}";

        uint[] offsets = [0x00u, 0x04u, 0x08u, 0x0Cu, 0x34u, 0x38u,
            0x58u, 0x64u, 0x74u, 0x78u, 0x80u, 0x84u, 0x88u, 0x8Cu, 0x94u];
        string words = string.Join(",", offsets.Select(offset =>
            $"+{offset:X2}={baseMemory.ReadU32(c.A0 + offset):X8}"));
        return $"a0=0x{c.A0:X8} a1=0x{c.A1:X8} a2=0x{c.A2:X8} " +
            $"ra=0x{c.RA:X8} object[{words}] history=[" +
            $"{RecompOne.Runtime.Sdk.V8Compat.DescribeObjectHistory(c.A0)}]";
    }

    static void Rebuild()
    {
        _funcMap.Clear();
        lock (_active)
        {
            foreach (var name in _active)
                foreach (var (addr, fn) in _registry[name].Functions)
                    _funcMap[addr] = fn;
        }
    }
}
