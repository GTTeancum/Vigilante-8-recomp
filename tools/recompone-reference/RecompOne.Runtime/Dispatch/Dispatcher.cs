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
    static readonly Dictionary<uint, Action<CpuContext, IMemory>> _hostFunctions = [];
    static readonly Dictionary<uint, (IOverlay Overlay, uint Delta)> _relocatedFunctions = [];
    static readonly Dictionary<uint, uint> _relocatedAliases = [];
    static readonly Dictionary<uint, (uint Base, uint Size, uint Delta)> _objectOwners = [];
    static readonly List<(IOverlay Overlay, uint Delta)> _relocatedImages = [];
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

    public static void RegisterHostFunction(
        uint address, Action<CpuContext, IMemory> function)
    {
        if (_hostFunctions.ContainsKey(address) || _funcMap.ContainsKey(address))
            throw new InvalidOperationException(
                $"function address 0x{address:X8} is already registered");
        _hostFunctions[address] = function;
    }

    public static IMemory UnwrapMemory(IMemory memory) =>
        memory is RelocatedMemory relocated ? relocated.Inner : memory;

    public static uint NormalizeLinkedAddress(IMemory memory, uint address)
    {
        if (memory is not RelocatedMemory relocated) return address;
        uint actualBase = relocated.LinkedBase + relocated.Delta;
        return address >= actualBase && address - actualBase < relocated.Size
            ? address - relocated.Delta
            : address;
    }

    public static uint ResolveLinkedAddress(IMemory memory, uint address)
    {
        if (memory is not RelocatedMemory relocated) return address;
        return address >= relocated.LinkedBase &&
               address - relocated.LinkedBase < relocated.Size
            ? address + relocated.Delta
            : address;
    }

    public static void AssociateObjectOwner(uint address, IMemory memory)
    {
        if (address == 0u || memory is not RelocatedMemory relocated) return;
        _objectOwners[address] = (relocated.LinkedBase, relocated.Size, relocated.Delta);
    }

    public static void ForgetObjectOwner(uint address) => _objectOwners.Remove(address);

    public static uint ResolveOwnedAddress(uint objectAddress, uint address)
    {
        if (_objectOwners.TryGetValue(objectAddress, out var owner) &&
            address >= owner.Base && address - owner.Base < owner.Size)
            return address + owner.Delta;

        // Static arena objects do not pass through the heap object factory and
        // therefore have no per-object owner record. They still retain linked
        // data pointers. Prefer the most recently registered matching image:
        // LOAD is registered first and the arena which supersedes its linked
        // address range is registered immediately afterward.
        for (int pass = 0; pass < 2; pass++)
        {
            for (int i = _relocatedImages.Count - 1; i >= 0; i--)
            {
                var image = _relocatedImages[i];
                bool arena = image.Overlay.Name.StartsWith(
                    "LEVELS_", StringComparison.OrdinalIgnoreCase);
                if ((pass == 0) != arena)
                    continue;
                if (address >= image.Overlay.Base &&
                    address - image.Overlay.Base < image.Overlay.Size)
                    return address + image.Delta;
            }
        }
        return address;
    }

    public static uint ResolveOwnedFunction(uint objectAddress, uint address)
    {
        if (_objectOwners.TryGetValue(objectAddress, out var owner))
        {
            uint owned = address >= owner.Base &&
                         address - owner.Base < owner.Size
                ? address + owner.Delta
                : address;
            if (_funcMap.ContainsKey(owned))
                return owned;
        }

        for (int i = _relocatedImages.Count - 1; i >= 0; i--)
        {
            var image = _relocatedImages[i];
            if (!image.Overlay.Functions.ContainsKey(address))
                continue;
            uint actual = address + image.Delta;
            if (_funcMap.ContainsKey(actual))
                return actual;
        }
        return address;
    }

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

        Sdk.LibCd.NotifyOverlayLoaded(name);
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

    // Attribution probe. Mesh emitters are reached through a function pointer
    // per model type, so no static search finds them. Bracketing every
    // indirect call with the packet-buffer pointer names whichever one wrote a
    // given packet. Gated: this is a hot path.
    public static readonly bool TraceIndirectPackets =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_INDIRECT") == "1";

    public static void Call(CpuContext c, IMemory m, uint addr)
    {
        if (TraceIndirectPackets && Sdk.V82Compat.IndirectPacketDepth < 8)
        {
            Sdk.V82Compat.BeginIndirectCall(c, m, addr);
            try
            {
                CallInner(c, m, addr);
            }
            finally
            {
                Sdk.V82Compat.EndIndirectCall(c, m, addr);
            }
            return;
        }
        CallInner(c, m, addr);
    }

    static void CallInner(CpuContext c, IMemory m, uint addr)
    {
        if (BiosKernel.TryDispatch(c, m, addr)) return;
        if (_hostFunctions.TryGetValue(addr, out var hostFunction))
        {
            hostFunction(c, m);
            return;
        }
        bool baseMemoryCall = m is not RelocatedMemory;
        (uint Base, uint Size, uint Delta) owner = default;
        bool hasOwner = _objectOwners.TryGetValue(c.A0, out owner);
        IMemory baseMemory = UnwrapMemory(m);
        uint objectPhysical = c.A0 & 0x1FFFFFFFu;
        uint ramSize = Runtime.Mode == RunMode.Devkit
            ? 0x00800000u
            : 0x00200000u;
        bool objectCallback =
            objectPhysical <= ramSize - 4u &&
            baseMemory.ReadU32(c.A0) == addr;
        if (objectCallback)
        {
            // A relocated arena callback can invoke a callback owned by a
            // vehicle/shared overlay. The caller's RelocatedMemory describes
            // the arena, not the target object; translating through it first
            // turns legitimate shared callbacks into arena data addresses.
            addr = ResolveOwnedFunction(c.A0, addr);
        }
        else if (m is RelocatedMemory linked &&
            addr >= linked.LinkedBase && addr - linked.LinkedBase < linked.Size)
            addr += linked.Delta;
        else if (baseMemoryCall && hasOwner &&
                 addr >= owner.Base && addr - owner.Base < owner.Size)
        {
            uint linkedAddr = addr;
            uint ownerAddr = linkedAddr + owner.Delta;
            if (_funcMap.ContainsKey(ownerAddr))
            {
                addr = ownerAddr;
            }
            else if (_relocatedAliases.TryGetValue(linkedAddr, out uint currentDelta) &&
                     _funcMap.ContainsKey(linkedAddr + currentDelta))
            {
                // LOAD constructs arena objects before the terrain DLL is in
                // memory. Those objects can retain LOAD ownership after their
                // linked callback slot is handed to the newly loaded arena.
                // Use the owner's relocation when it resolves; otherwise the
                // current alias is the only executable instance of that
                // original linked callback.
                addr = linkedAddr + currentDelta;
            }
            else
            {
                addr = ownerAddr;
            }
        }
        else if ((baseMemoryCall || !_funcMap.ContainsKey(addr)) &&
                 _relocatedAliases.TryGetValue(addr, out uint aliasDelta) &&
                 _funcMap.ContainsKey(addr + aliasDelta))
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
        fn(c, callMemory);
        RecompOne.Runtime.Sdk.V8DreamlandCompat
            .CaptureAnimationCallbackReturn(c);
    }

    static void TryLoadRelocatedOverlay(IMemory m, uint addr)
    {
        const uint RamBase = 0x80000000u;
        uint RamSize = Runtime.Mode == RunMode.Devkit
            ? 0x00800000u
            : 0x00200000u;
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
        _relocatedImages.RemoveAll(image =>
            ReferenceEquals(image.Overlay, overlay));
        _relocatedImages.Add((overlay, delta));

        lock (_active)
            if (!_active.Contains(overlay.Name)) _active.Add(overlay.Name);
        Rebuild();
        Runtime.OverlayLog.Record(overlay.Name, OverlayEventKind.Loaded,
            $"relocated by 0x{delta:X8}");
        Sdk.LibCd.NotifyOverlayLoaded(overlay.Name);
        Console.WriteLine($"[Dispatcher] loaded relocated overlay: {overlay.Name} base=0x{actualBase:X8} delta=0x{delta:X8}");
    }

    static string DescribeOverlayImages(IMemory memory, uint target)
    {
        const uint ramBase = 0x80000000u;
        uint ramEnd = Runtime.Mode == RunMode.Devkit
            ? 0x80800000u
            : 0x80200000u;
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
        uint ramSize = Runtime.Mode == RunMode.Devkit
            ? 0x00800000u
            : 0x00200000u;
        if (physical >= ramSize || physical > ramSize - 0xA0u)
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
        _relocatedFunctions.Clear();
        _relocatedAliases.Clear();
        lock (_active)
        {
            foreach (var name in _active)
                foreach (var (addr, fn) in _registry[name].Functions)
                    _funcMap[addr] = fn;

            // A relocated arena may unload another overlay after all of its
            // functions have already been registered. Rebuild used to retain
            // only linked addresses, leaving object-owner metadata and aliases
            // pointing at actual addresses absent from the dispatch table.
            // Reconstruct all three indexes from the same active image set so
            // an indirect callback cannot fall through to an older LOAD image.
            foreach (var (overlay, delta) in _relocatedImages)
            {
                if (!_active.Contains(overlay.Name))
                    continue;
                foreach (var (original, function) in overlay.Functions)
                {
                    uint actual = original + delta;
                    _funcMap[actual] = function;
                    _relocatedFunctions[actual] = (overlay, delta);
                    _relocatedAliases[original] = delta;
                }
            }
        }
    }
}
