using System.Buffers.Binary;
using System.Text;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Strict loader for independently authored V8:2 vehicle packages.
///
/// The package contains only named, decoded native records: a V8VR registry
/// and from-scratch XOBF/BIN/ANM banks. Unknown chunks and trailing payloads
/// are rejected; no retail entry or opaque byte range is retained.
/// </summary>
public static class V82VehicleRegistry
{
    public const int RetailVehicleCount = 18;
    public const int FirstCustomType = 64;
    public const uint CustomDispatchAddress = 0x807FF000u;
    const uint BuildNativeBankAddress = 0x8001E914u;
    const uint CreateObjectAddress = 0x80031DDCu;
    const uint GenericVehicleDispatchAddress = 0x800367A4u;

    const int HeaderSize = 20;
    const int LegacyEntrySize = 36;
    const int EntrySize = 40;
    const int PreviewEntrySize = 44;
    const int StatsSize = 0x30;
    const int TransformModeCount = 4;
    const int TransformWheelCount = 6;
    const int TransformTableSize = TransformModeCount * TransformWheelCount * 2;
    const int PowerupCount = 5;
    const int PowerupTableSize = PowerupCount * 4;
    const int PlayerSelectionCount = 2;
    const ushort NoArchiveIndex = 0xFFFF;
    const uint SelectorInputAddress = 0x8006B508u;
    const uint SelectorTextAddress = 0x807FE000u;
    const uint SelectorVehiclePreviewReturn = 0x80106C1Cu;
    const uint SelectorScaleReturn = 0x80106CD4u;
    const uint SelectorGuestZoom = 0x015Au;
    const uint SelectorDriverNameReturn = 0x80106A7Cu;
    const uint SelectorVehicleNameReturn = 0x80106B14u;

    static readonly UTF8Encoding StrictUtf8 =
        new(encoderShouldEmitUTF8Identifier: false, throwOnInvalidBytes: true);
    static readonly List<VehicleEntry> Entries = [];
    static readonly List<NativeVehicleBankSource> Banks = [];
    static readonly Dictionary<uint, int> ObjectEntries = [];
    static readonly Dictionary<uint, uint> ObjectUpgradeStatus = [];
    static bool _initialized;
    static bool _dispatchRegistered;
    static readonly int[] SelectedTypes = [-1, -1];
    static string? _requestedStableId;
    static string? _loadedPackageRoot;
    static VehicleEntry? _constructingEntry;
    static bool _constructingSelectorPreview;
    static VehicleEntry? _defaultReplacementEntry;
    static int _selectorGuestIndex = -1;
    static int _selectorFirstRetailSlot;
    static int _selectorLastRetailSlot;
    static int _selectorPreviousSlot = -1;
    static int _selectorPlayer;
    static int _selectorStableFrames;
    static int _selectorStableGuest = -1;
    static uint _selectorPreviewObject;
    static readonly bool[] SelectorProofCaptured = new bool[3];
    static readonly string[] SelectorVehicleNames =
    [
        "'67 Rattler",
        "'70 Clydesdale",
        "'74 Strider",
    ];
    static readonly string? DefaultReplacementStableId =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_DEFAULT_REPLACEMENT");
    static readonly bool CaptureNativeSelectorProof =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR") == "1";
    static readonly bool CaptureNativeSelectorSettle =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_CAPTURE_V82_SELECTOR_SETTLE") == "1";
    static readonly bool TraceNativeSelectorInput =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_V82_SELECTOR") == "1";
    static readonly bool TraceNativeSelectorPhysics =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_V82_SELECTOR_PHYSICS") == "1";

    public static int Count => Entries.Count;
    public static int TotalVehicleCount => RetailVehicleCount + Entries.Count;
    public static bool HasPackage => Entries.Count != 0;
    public static bool HasDefaultReplacement =>
        _defaultReplacementEntry != null;
    public static string? LoadedPackageRoot => _loadedPackageRoot;
    public static int SelectedType => SelectedTypeForPlayer(0);
    public static int NativeSelectorGuestIndex =>
        Volatile.Read(ref _selectorGuestIndex);
    public static string? NativeSelectorBannerPath
    {
        get
        {
            int index = NativeSelectorGuestIndex;
            if (index < 0 || index >= Math.Min(3, Entries.Count) ||
                string.IsNullOrEmpty(_loadedPackageRoot))
                return null;
            return Path.Combine(
                _loadedPackageRoot, $"SELECTOR_{index:00}.PPM");
        }
    }
    public static bool HasAnySelection =>
        Enumerable.Range(0, PlayerSelectionCount)
            .Any(player => SelectedTypeForPlayer(player) >= 0);

    public static VehicleRosterItem[] Roster()
    {
        return Entries
            .OrderBy(entry => entry.SelectionOrder)
            .ThenBy(entry => entry.Type)
            .Select(entry => new VehicleRosterItem(
                entry.Type,
                entry.StableId,
                entry.DisplayName,
                entry.SelectionOrder,
                FactionFor(entry.StableId),
                entry.Stats[0x2C],
                entry.Stats[0x2D],
                entry.Stats[0x2E],
                entry.Stats[0x2F],
                SupportsHotRod: false))
            .ToArray();
    }

    public static int SelectedTypeForPlayer(int player)
    {
        if ((uint)player >= PlayerSelectionCount)
            throw new ArgumentOutOfRangeException(nameof(player));
        return Volatile.Read(ref SelectedTypes[player]);
    }

    public static void SelectType(int type) => SelectTypeForPlayer(0, type);

    public static void SelectTypeForPlayer(int player, int type)
    {
        if ((uint)player >= PlayerSelectionCount)
            throw new ArgumentOutOfRangeException(nameof(player));
        if (type >= 0 && !IsCustomType((uint)type))
            throw new ArgumentOutOfRangeException(
                nameof(type), type, "vehicle type is not in the V8:2 guest roster");
        Volatile.Write(ref SelectedTypes[player], type);
        Console.Error.WriteLine(
            type < 0
                ? $"[V82Vehicles] player={player + 1} using the built-in roster selection"
                : $"[V82Vehicles] player={player + 1} selected guest " +
                  $"type={type} name={NameForType(type)}");
    }

    public static void ClearSelections()
    {
        for (int player = 0; player < PlayerSelectionCount; player++)
            Volatile.Write(ref SelectedTypes[player], -1);
    }

    public static void BeginNativeSelector(CpuContext c, IMemory m)
    {
        _selectorGuestIndex = -1;
        _selectorPreviousSlot = -1;
        _selectorFirstRetailSlot = 0;
        _selectorLastRetailSlot = RetailVehicleCount - 1;
        _selectorPlayer = c.A1 == 0u ? 0 : 1;
        _selectorStableFrames = 0;
        _selectorStableGuest = -1;
        _selectorPreviewObject = 0u;
        Array.Fill(SelectorProofCaptured, false);
        InputManager.SignalScriptStage("choose_player");
    }

    public static void EndNativeSelector(CpuContext c, IMemory m)
    {
        int guest = NativeSelectorGuestIndex;
        if (guest >= 0 && guest < Math.Min(3, Entries.Count) &&
            c.V0 == (uint)guest)
        {
            VehicleEntry entry = Entries[guest];
            c.V0 = checked((uint)entry.Type);
            SelectTypeForPlayer(_selectorPlayer, entry.Type);
        }
        _selectorGuestIndex = -1;
        _selectorPreviousSlot = -1;
        _selectorPreviewObject = 0u;
    }

    /// <summary>
    /// Extends the native 18-entry carousel without replacing it. The retail
    /// selector still performs all input repeat, transition, and availability
    /// handling; this seam inserts the three V8 entries only at the wrap.
    /// </summary>
    public static uint ResolveNativeSelectorSlot(uint slot, IMemory m)
    {
        if (Entries.Count == 0)
            return slot;

        int current = checked((int)slot);
        uint input = m.ReadU32(SelectorInputAddress);
        bool left = (input & 0x80000000u) != 0u;
        bool right = (input & 0x20000000u) != 0u;
        int guest = NativeSelectorGuestIndex;

        if (_selectorPreviousSlot < 0)
        {
            _selectorFirstRetailSlot = current;
            _selectorLastRetailSlot = current;
            _selectorPreviousSlot = current;
            return slot;
        }

        if (guest < 0)
        {
            if (right && current < _selectorPreviousSlot)
            {
                _selectorLastRetailSlot = _selectorPreviousSlot;
                guest = 0;
                current = 0;
            }
            else if (left && current > _selectorPreviousSlot)
            {
                _selectorFirstRetailSlot = _selectorPreviousSlot;
                _selectorLastRetailSlot = current;
                guest = Math.Min(3, Entries.Count) - 1;
                current = guest;
            }
            else
            {
                _selectorPreviousSlot = current;
                return slot;
            }
        }
        else if (right && current != guest)
        {
            if (guest + 1 < Math.Min(3, Entries.Count))
            {
                guest++;
                current = guest;
            }
            else
            {
                _selectorGuestIndex = -1;
                _selectorPreviousSlot = _selectorFirstRetailSlot;
                return checked((uint)_selectorFirstRetailSlot);
            }
        }
        else if (left && current != guest)
        {
            if (guest > 0)
            {
                guest--;
                current = guest;
            }
            else
            {
                _selectorGuestIndex = -1;
                _selectorPreviousSlot = _selectorLastRetailSlot;
                return checked((uint)_selectorLastRetailSlot);
            }
        }
        else
        {
            current = guest;
        }

        _selectorGuestIndex = guest;
        _selectorPreviousSlot = current;
        if (_selectorStableGuest != guest)
        {
            _selectorStableGuest = guest;
            _selectorStableFrames = 0;
        }
        // Circle is the native Hot Rod/custom modifier. Guest entries own one
        // canonical bank, so consume it before the selector can enter the
        // retail variant editor.
        m.WriteU32(SelectorInputAddress, input & ~0x00200000u);
        return checked((uint)current);
    }

    public static bool TickNativeSelector(CpuContext c, IMemory m)
    {
        int guest = NativeSelectorGuestIndex;
        if (TraceNativeSelectorInput && guest >= 0)
        {
            uint input = m.ReadU32(SelectorInputAddress);
            if (input != 0u)
                Console.Error.WriteLine(
                    $"[V82Selector] guest={guest} input=0x{input:X8}");
        }
        if (!CaptureNativeSelectorProof ||
            guest < 0 || guest >= Math.Min(3, Entries.Count) ||
            SelectorProofCaptured[guest])
            return true;

        int frame = ++_selectorStableFrames;
        if (TraceNativeSelectorPhysics && _selectorPreviewObject != 0u &&
            frame <= 220)
            TraceSelectorPhysics(m, guest, frame, _selectorPreviewObject);
        if (CaptureNativeSelectorSettle && frame <= 64 && frame % 2 == 0)
            HostWindow.RequestDisplayCapture(
                $"native_guest_{guest:00}_settle_{frame:000}");
        if (frame == 80)
            HostWindow.RequestDisplayCapture($"native_guest_{guest:00}");
        else if (frame > 80 && frame <= 200 && (frame - 80) % 4 == 0)
            HostWindow.RequestDisplayCapture(
                $"native_guest_{guest:00}_turn_{(frame - 80) / 4:000}");
        if (frame == 200)
            SelectorProofCaptured[guest] = true;
        return true;
    }

    public static uint NativeSelectorVariant(uint retailVariant) =>
        NativeSelectorGuestIndex >= 0 ? 0u : retailVariant;

    public static uint NativeSelectorStatsPointer(
        CpuContext c, IMemory m, uint retailPointer, uint field)
    {
        if (!TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return retailPointer;
        EnsureRuntime(entry, c, Dispatcher.UnwrapMemory(m));
        return entry.StatsRuntime + field;
    }

    public static bool OverrideNativeSelectorText(CpuContext c, IMemory m)
    {
        V82Compat.TraceNativeSelectorCall(c, m);
        if (!TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return true;

        string? text = c.RA switch
        {
            SelectorDriverNameReturn => entry.DisplayName,
            SelectorVehicleNameReturn => SelectorVehicleNames[
                NativeSelectorGuestIndex],
            _ => null,
        };
        if (text == null)
            return true;

        m = Dispatcher.UnwrapMemory(m);
        byte[] bytes = Encoding.ASCII.GetBytes(text);
        for (int index = 0; index < bytes.Length; index++)
            m.WriteU8(SelectorTextAddress + (uint)index, bytes[index]);
        m.WriteU8(SelectorTextAddress + (uint)bytes.Length, 0);
        c.A1 = SelectorTextAddress;
        return true;
    }

    /// <summary>
    /// Substitutes the guest's dedicated selector bank and native stats, then
    /// allows the original V8:2 selector vehicle constructor to run unchanged.
    /// That preserves the retail wheel creation, suspension initialization,
    /// ground placement, camera, continuous rotation, lighting, and framing.
    /// </summary>
    public static bool BuildNativeSelectorPreview(CpuContext c, IMemory m)
    {
        V82Compat.TraceNativeSelectorCall(c, m);
        if (c.RA != SelectorVehiclePreviewReturn ||
            !TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return true;

        m = Dispatcher.UnwrapMemory(m);
        EnsureRuntime(entry, c, m);
        c.A0 = entry.SelectorPreviewRuntime;
        c.A1 = checked((uint)entry.SelectorPreviewBodyKind);
        c.A2 = entry.StatsRuntime;
        _constructingSelectorPreview = true;
        _constructingEntry = entry;
        V82Compat.BeginGuestVramClaim();
        return true;
    }

    /// <summary>
    /// Records the vehicle produced by the unchanged retail selector
    /// constructor. No preview transform, suspension, or camera state is
    /// synthesized here.
    /// </summary>
    public static void FinalizeNativeSelectorPreview(CpuContext c, IMemory m)
    {
        if (!_constructingSelectorPreview)
            return;

        m = Dispatcher.UnwrapMemory(m);
        VehicleEntry? entry = _constructingEntry;
        try
        {
            V82Compat.EndGuestVramClaim(c, m);
            uint vehicle = c.V0;
            if (vehicle != 0u && entry != null)
            {
                ObjectEntries[vehicle] = entry.Type - FirstCustomType;
                m.WriteU8(vehicle + 0xDCu, checked((byte)entry.Type));
                _selectorPreviewObject = vehicle;
                Console.Error.WriteLine(
                    $"[V82Vehicles] created {entry.StableId} " +
                    $"native-selector object=0x{vehicle:X8}");
            }
        }
        finally
        {
            _constructingEntry = null;
            _constructingSelectorPreview = false;
        }
    }

    /// <summary>
    /// Uses V8:2's native selector projection-scale input as a modest,
    /// uniform camera zoom-out for the imported roster. Asset coordinates,
    /// model scale, projection code, and gameplay transforms are unchanged.
    /// </summary>
    public static bool ZoomNativeSelectorPreview(CpuContext c, IMemory m)
    {
        if (c.RA == SelectorScaleReturn &&
            NativeSelectorGuestIndex >= 0)
            c.A1 = SelectorGuestZoom;
        return true;
    }

    /// <summary>
    /// Object storage is recycled immediately by the retail allocator. Drop
    /// host-side custom identity before teardown so a later retail object at
    /// the same address cannot inherit guest banks or stats.
    /// </summary>
    public static bool ReleaseVehicleMapping(CpuContext c, IMemory m)
    {
        if (_selectorPreviewObject == c.A0)
            _selectorPreviewObject = 0u;
        ObjectEntries.Remove(c.A0);
        ObjectUpgradeStatus.Remove(c.A0);
        return true;
    }

    static void TraceSelectorPhysics(
        IMemory m, int guest, int frame, uint vehicle)
    {
        static int S32(uint value) => unchecked((int)value);

        var line = new StringBuilder()
            .Append("[V82SelectorPhysics] guest=").Append(guest)
            .Append(" frame=").Append(frame)
            .Append(" vehicle=0x").Append(vehicle.ToString("X8"))
            .Append(" pos=(")
            .Append(S32(m.ReadU32(vehicle + 0x20u))).Append(',')
            .Append(S32(m.ReadU32(vehicle + 0x24u))).Append(',')
            .Append(S32(m.ReadU32(vehicle + 0x28u))).Append(')');
        for (int index = 0; index < TransformWheelCount; index++)
        {
            uint wheel = m.ReadU32(
                vehicle + 0x104u + checked((uint)(index * 4)));
            if (wheel == 0u)
                continue;
            line.Append(" w").Append(index).Append("=0x")
                .Append(wheel.ToString("X8")).Append("{xyz=(")
                .Append(S32(m.ReadU32(wheel + 0x4Cu))).Append(',')
                .Append(S32(m.ReadU32(wheel + 0x50u))).Append(',')
                .Append(S32(m.ReadU32(wheel + 0x54u))).Append("),limit=")
                .Append(S32(m.ReadU32(wheel + 0x80u))).Append('/')
                .Append(S32(m.ReadU32(wheel + 0x84u))).Append('/')
                .Append(S32(m.ReadU32(wheel + 0x88u))).Append(",rest=")
                .Append(S32(m.ReadU32(wheel + 0x90u))).Append(",vel=")
                .Append(S32(m.ReadU32(wheel + 0x98u))).Append(",sd=")
                .Append((short)m.ReadU16(wheel + 0x8Cu)).Append('/')
                .Append((short)m.ReadU16(wheel + 0x8Eu)).Append('}');
        }
        Console.Error.WriteLine(line);
    }

    static bool TrySelectorEntry(out VehicleEntry? entry)
    {
        int guest = NativeSelectorGuestIndex;
        if (guest >= 0 && guest < Math.Min(3, Entries.Count))
        {
            entry = Entries[guest];
            return true;
        }
        entry = null;
        return false;
    }

    public static void RequestSelection(string stableId)
    {
        if (string.IsNullOrWhiteSpace(stableId))
            throw new ArgumentException(
                "guest vehicle stable ID cannot be empty", nameof(stableId));
        _requestedStableId = stableId;
        if (Entries.Count != 0)
            ApplyRequestedSelection();
    }

    public static void Initialize(CpuContext c, IMemory m)
    {
        if (_initialized) return;
        _initialized = true;

        if (!TryFindPackage(out string registryPath, out string archivePath))
            return;

        LoadAndValidate(registryPath, archivePath);
        _loadedPackageRoot = Path.GetDirectoryName(registryPath);
        if (!_dispatchRegistered)
        {
            Dispatcher.RegisterHostFunction(
                CustomDispatchAddress, DispatchCustomVehicle);
            _dispatchRegistered = true;
        }

        Console.Error.WriteLine(
            $"[V82Vehicles] validated {Entries.Count} independent vehicle" +
            $"{(Entries.Count == 1 ? "" : "s")} and {Banks.Count} native bank" +
            $"{(Banks.Count == 1 ? "" : "s")}");
    }

    public static string ValidatePackage(string directory)
    {
        string root = Path.GetFullPath(directory);
        string registryPath = Path.Combine(root, "VEHICLES.V8R");
        string archivePath = Path.Combine(root, "CUSTOM.EXP");
        if (!File.Exists(registryPath) || !File.Exists(archivePath))
            throw new FileNotFoundException(
                $"Vehicle package is incomplete in {root}");
        LoadAndValidate(registryPath, archivePath);
        return $"vehicles={Entries.Count} banks={Banks.Count}";
    }

    static void LoadAndValidate(string registryPath, string archivePath)
    {
        Entries.Clear();
        Banks.Clear();
        ObjectEntries.Clear();
        ObjectUpgradeStatus.Clear();
        _defaultReplacementEntry = null;
        ClearSelections();
        byte[] registry = File.ReadAllBytes(registryPath);
        ParseArchive(archivePath);
        ParseRegistry(registry);
        ValidateCrossReferences();
        ResolveDefaultReplacement();
        if (_requestedStableId != null)
            ApplyRequestedSelection();
    }

    static void ResolveDefaultReplacement()
    {
        if (string.IsNullOrWhiteSpace(DefaultReplacementStableId))
            return;
        _defaultReplacementEntry = Entries.FirstOrDefault(entry =>
            entry.StableId.Equals(
                DefaultReplacementStableId, StringComparison.Ordinal));
        if (_defaultReplacementEntry == null)
            throw new InvalidDataException(
                $"requested V8:2 default replacement " +
                $"'{DefaultReplacementStableId}' is not registered");
        Console.Error.WriteLine(
            $"[V82Vehicles] retail type 0 keeps its roster identity and uses " +
            $"independent model {_defaultReplacementEntry.StableId}");
    }

    static void ApplyRequestedSelection()
    {
        VehicleEntry? entry = Entries.FirstOrDefault(candidate =>
            candidate.StableId.Equals(
                _requestedStableId, StringComparison.Ordinal));
        if (entry == null)
            throw new InvalidDataException(
                $"requested V8:2 guest vehicle '{_requestedStableId}' is not registered");
        SelectType(entry.Type);
    }

    public static string? NameForType(int type)
    {
        int local = type - FirstCustomType;
        return local >= 0 && local < Entries.Count
            ? Entries[local].DisplayName
            : null;
    }

    public static bool IsCustomType(uint type)
    {
        uint local = type - FirstCustomType;
        return local < (uint)Entries.Count;
    }

    /// <summary>
    /// Guest packages currently own one canonical body bank per entry. They
    /// never inherit an unrelated retail Hot Rod bank.
    /// </summary>
    public static bool SupportsHotRod(int type) =>
        !IsCustomType(unchecked((uint)type));

    public static void PrepareSelectedRuntime(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        foreach (VehicleEntry entry in ActiveEntries())
            EnsureRuntime(entry, c, m);
    }

    internal static IReadOnlyList<NativeVramAllocation>
        SelectedVramAllocations()
    {
        var result = new List<NativeVramAllocation>();
        foreach (VehicleEntry entry in ActiveEntries())
        {
            int bodyArchiveIndex =
                _constructingSelectorPreview &&
                entry.SelectorPreviewArchiveIndex != NoArchiveIndex
                    ? entry.SelectorPreviewArchiveIndex
                    : entry.BodyArchiveIndex;
            result.AddRange(
                Banks[bodyArchiveIndex].ReadVramAllocations(
                    palettesFirst: true));
            result.AddRange(
                Banks[entry.TransformArchiveIndex].ReadVramAllocations(
                    palettesFirst: true));
        }
        return result;
    }

    internal static bool OwnsCurrentTextureLoad(
        CpuContext c,
        IMemory m,
        bool palette)
    {
        uint source = m.ReadU32(c.GP + (palette ? 0xE6Cu : 0xE74u));
        foreach (VehicleEntry entry in ActiveEntries())
        {
            if (PointerInBank(
                    m,
                    entry.BodyRuntime,
                    Banks[entry.BodyArchiveIndex].BinLength,
                    source) ||
                PointerInBank(
                    m,
                    entry.TransformRuntime,
                    Banks[entry.TransformArchiveIndex].BinLength,
                    source) ||
                (
                    entry.SelectorPreviewArchiveIndex != NoArchiveIndex &&
                    PointerInBank(
                        m,
                        entry.SelectorPreviewRuntime,
                        Banks[entry.SelectorPreviewArchiveIndex].BinLength,
                        source)
                ))
                return true;
        }
        return false;
    }

    static VehicleEntry[] ActiveEntries()
    {
        var active = new List<VehicleEntry>(PlayerSelectionCount + 1);
        for (int player = 0; player < PlayerSelectionCount; player++)
        {
            int selected = SelectedTypeForPlayer(player);
            if (selected >= 0 &&
                TryEntryForType((uint)selected, out VehicleEntry? entry) &&
                entry != null &&
                !active.Contains(entry))
                active.Add(entry);
        }
        if (_defaultReplacementEntry != null &&
            !active.Contains(_defaultReplacementEntry))
            active.Add(_defaultReplacementEntry);
        if (_constructingSelectorPreview &&
            TrySelectorEntry(out VehicleEntry? selectorEntry) &&
            selectorEntry != null &&
            !active.Contains(selectorEntry))
            active.Add(selectorEntry);
        return active.ToArray();
    }

    static string FactionFor(string stableId)
    {
        if (stableId.StartsWith("guest.v8.", StringComparison.Ordinal))
            return "V8 LEGACY";
        int separator = stableId.IndexOf('.');
        return separator > 0
            ? stableId[..separator].Replace('_', ' ').ToUpperInvariant()
            : "GUEST";
    }

    static bool PointerInBank(
        IMemory m,
        uint runtime,
        int binLength,
        uint pointer)
    {
        if (runtime == 0u || pointer == 0u)
            return false;
        uint bin = m.ReadU32(runtime + 4u);
        uint length = checked((uint)binLength);
        return pointer >= bin && pointer - bin < length;
    }

    public static void ResetRuntimeForMatch()
    {
        ObjectEntries.Clear();
        ObjectUpgradeStatus.Clear();
        _constructingEntry = null;
        _constructingSelectorPreview = false;
        foreach (VehicleEntry entry in Entries)
        {
            entry.BodyRuntime = 0u;
            entry.TransformRuntime = 0u;
            entry.SelectorPreviewRuntime = 0u;
            entry.StatsRuntime = 0u;
            entry.TransformTableRuntime = 0u;
            entry.PowerupTableRuntime = 0u;
        }
    }

    /// <summary>
    /// Pre-hook for the retail callback resolver. Custom identities resolve to
    /// one host callback which dispatches through their independently allocated
    /// stats record. Returning false skips the fixed 21-entry retail resolver.
    /// </summary>
    public static bool ResolveVehicleCallback(CpuContext c, IMemory m)
    {
        if (!(c.A0 == 0u && _defaultReplacementEntry != null) &&
            !TryEntryForType(c.A0, out _))
            return true;
        c.V0 = CustomDispatchAddress;
        return false;
    }

    /// <summary>
    /// Creates a custom vehicle without occupying or replacing a retail common
    /// object slot. Returning false tells the pre-hook to skip func_80036C2C.
    /// </summary>
    public static bool CreateVehicle(CpuContext c, IMemory m)
    {
        uint requestedType = c.A1;
        bool replacingDefault =
            requestedType == 0u && _defaultReplacementEntry != null;
        VehicleEntry? entry = replacingDefault
            ? _defaultReplacementEntry
            : null;
        if (entry == null &&
            !TryEntryForType(requestedType, out entry))
            return true;

        m = Dispatcher.UnwrapMemory(m);
        uint source = c.A0;
        int variant = unchecked((int)c.A2);
        if (source == 0u)
        {
            c.V0 = 0u;
            return false;
        }

        EnsureRuntime(entry, c, m);
        bool selectorPreview =
            _constructingSelectorPreview &&
            entry.SelectorPreviewRuntime != 0u;
        m.WriteU32(source, CustomDispatchAddress);
        m.WriteU16(
            source + 0x1Au,
            checked((ushort)(
                selectorPreview
                    ? entry.SelectorPreviewBodyKind
                    : entry.BodyKind)));
        m.WriteU32(
            source + 0x5Cu,
            selectorPreview
                ? entry.SelectorPreviewRuntime
                : entry.BodyRuntime);

        uint callerRa = c.RA;
        uint vehicle;
        _constructingEntry = entry;
        V82Compat.BeginGuestVramClaim();
        try
        {
            c.A0 = source;
            c.RA = CustomDispatchAddress;
            Dispatcher.Call(c, m, CreateObjectAddress);
            vehicle = c.V0;
            V82Compat.EndGuestVramClaim(c, m);
        }
        catch
        {
            V82Compat.AbortGuestVramClaim();
            throw;
        }
        finally
        {
            _constructingEntry = null;
            c.RA = callerRa;
        }
        if (vehicle == 0u)
            return false;

        ApplyAuthoredSuspension(m, vehicle, entry);

        for (uint offset = 0xF8u; offset <= 0xFCu; offset += 4u)
        {
            uint child = m.ReadU32(vehicle + offset);
            if (child != 0u)
                m.WriteU16(child + 0xAu, m.ReadU16(vehicle + 0xAu));
        }

        if (variant > 0)
        {
            int x = (short)m.ReadU16(vehicle + 0x24u);
            int z = (short)m.ReadU16(vehicle + 0x30u);
            m.WriteU32(vehicle + 0x80u, unchecked((uint)((x * 4577) >> 5)));
            m.WriteU32(vehicle + 0x84u, 0u);
            m.WriteU32(vehicle + 0x88u, unchecked((uint)((z * 4577) >> 5)));
        }

        ObjectEntries[vehicle] = entry.Type - FirstCustomType;
        m.WriteU8(
            vehicle + 0xDCu,
            replacingDefault ? (byte)0 : checked((byte)entry.Type));
        c.V0 = vehicle;
        Console.Error.WriteLine(
            $"[V82Vehicles] created {entry.StableId} " +
            $"{(selectorPreview ? "selector-preview " : "")}" +
            $"identity={(replacingDefault ? 0 : entry.Type)} " +
            $"object=0x{vehicle:X8}");
        return false;
    }

    static void ApplyAuthoredSuspension(
        IMemory m, uint vehicle, VehicleEntry entry)
    {
        short frontSpring = (short)U16(entry.Stats, 0x04);
        short middleSpring = (short)U16(entry.Stats, 0x06);
        short rearSpring = (short)U16(entry.Stats, 0x08);
        short frontDamping = (short)U16(entry.Stats, 0x08);
        short middleDamping = (short)U16(entry.Stats, 0x0A);
        short rearDamping = checked((short)entry.RearSuspensionDamping);
        short[] springs =
        [
            frontSpring, frontSpring,
            middleSpring, middleSpring,
            rearSpring, rearSpring,
        ];
        short[] damping =
        [
            frontDamping, frontDamping,
            middleDamping, middleDamping,
            rearDamping, rearDamping,
        ];
        int appliedWheels = 0;
        for (int wheelIndex = 0; wheelIndex < TransformWheelCount; wheelIndex++)
        {
            uint wheel = m.ReadU32(vehicle + 0x104u + (uint)(wheelIndex * 4));
            if (wheel == 0u)
                continue;
            m.WriteU16(wheel + 0x8Cu, unchecked((ushort)springs[wheelIndex]));
            m.WriteU16(wheel + 0x8Eu, unchecked((ushort)damping[wheelIndex]));
            appliedWheels++;
        }
        if (appliedWheels == 0)
            throw new InvalidOperationException(
                $"custom vehicle {entry.StableId} created no suspension objects");
        Console.Error.WriteLine(
            $"[V82Vehicles] suspension {entry.StableId} " +
            $"axle1={frontSpring}/{frontDamping} " +
            $"axle2={middleSpring}/{middleDamping} " +
            $"axle3={rearSpring}/{rearDamping} " +
            $"active-wheels={appliedWheels} (spring/damping)");
    }

    public static uint StatsPointerForObject(
        IMemory m, uint vehicle, uint retailPointer)
    {
        return TryEntryForObject(m, vehicle, out VehicleEntry? entry) &&
               entry.StatsRuntime != 0u
            ? entry.StatsRuntime
            : retailPointer;
    }

    public static uint WheelBankForObject(
        IMemory m, uint vehicle, uint retailPointer)
    {
        VehicleEntry? entry = _constructingEntry;
        if (entry == null &&
            !TryEntryForObject(m, vehicle, out entry))
            return retailPointer;
        if (entry.TransformRuntime == 0u)
            throw new InvalidOperationException(
                $"custom vehicle 0x{vehicle:X8} has no owned wheel bank");
        return entry.TransformRuntime;
    }

    public static uint TransformBankForObject(
        IMemory m, uint vehicle, uint retailPointer)
    {
        return TryEntryForObject(m, vehicle, out VehicleEntry? entry) &&
               entry.TransformRuntime != 0u
            ? entry.TransformRuntime
            : retailPointer;
    }

    public static uint TransformTableForObject(
        IMemory m, uint vehicle, uint retailPointer)
    {
        return TryEntryForObject(m, vehicle, out VehicleEntry? entry) &&
               entry.TransformTableRuntime != 0u
            ? entry.TransformTableRuntime
            : retailPointer;
    }

    public static uint UpgradeStatusForObject(
        CpuContext c, IMemory m, uint vehicle, uint retailPointer)
    {
        if (!TryEntryForObject(m, vehicle, out _))
            return retailPointer;
        if (ObjectUpgradeStatus.TryGetValue(vehicle, out uint pointer))
            return pointer;

        m = Dispatcher.UnwrapMemory(m);
        var state = c.Snapshot();
        try
        {
            pointer = AllocateBytes(c, m, new byte[10]);
            ObjectUpgradeStatus[vehicle] = pointer;
            return pointer;
        }
        finally
        {
            c.Restore(state);
        }
    }

    static void DispatchCustomVehicle(CpuContext c, IMemory m)
    {
        VehicleEntry? entry = _constructingEntry;
        if (entry == null &&
            !TryEntryForObject(m, c.A0, out entry))
            throw new InvalidOperationException(
                $"custom vehicle callback has no independent entry for 0x{c.A0:X8}");
        EnsureRuntime(entry, c, Dispatcher.UnwrapMemory(m));
        c.A3 = entry.StatsRuntime;
        try
        {
            Dispatcher.Call(c, m, GenericVehicleDispatchAddress);
        }
        catch
        {
            Console.Error.WriteLine(
                $"[V82Vehicles] dispatch failure {entry.StableId} " +
                $"message={c.A1} object=0x{c.A0:X8} " +
                $"stats=0x{entry.StatsRuntime:X8} " +
                $"a2=0x{c.A2:X8} a3=0x{c.A3:X8} " +
                $"s4=0x{c.S4:X8} s5=0x{c.S5:X8} s6=0x{c.S6:X8}");
            throw;
        }
    }

    static bool TryEntryForType(uint type, out VehicleEntry? entry)
    {
        uint local = type - FirstCustomType;
        if (local < (uint)Entries.Count)
        {
            entry = Entries[(int)local];
            return true;
        }
        entry = null;
        return false;
    }

    static bool TryEntryForObject(
        IMemory m, uint vehicle, out VehicleEntry? entry)
    {
        if (ObjectEntries.TryGetValue(vehicle, out int local) &&
            (uint)local < (uint)Entries.Count)
        {
            entry = Entries[local];
            return true;
        }
        return TryEntryForType(m.ReadU8(vehicle + 0xDCu), out entry);
    }

    static void EnsureRuntime(VehicleEntry entry, CpuContext c, IMemory m)
    {
        if (entry.BodyRuntime != 0u)
            return;

        var state = c.Snapshot();
        try
        {
            entry.StatsRuntime = AllocateBytes(c, m, entry.Stats);
            entry.TransformTableRuntime =
                AllocateTransformTable(c, m, entry.TransformModes);
            entry.PowerupTableRuntime =
                AllocatePowerupTable(c, m, entry.Powerups);
            entry.BodyRuntime =
                BuildNativeBank(c, m, Banks[entry.BodyArchiveIndex]);
            entry.TransformRuntime =
                BuildNativeBank(c, m, Banks[entry.TransformArchiveIndex]);
            if (entry.SelectorPreviewArchiveIndex != NoArchiveIndex)
            {
                entry.SelectorPreviewRuntime = BuildNativeBank(
                    c, m, Banks[entry.SelectorPreviewArchiveIndex]);
            }
            Console.Error.WriteLine(
                $"[V82Vehicles] built {entry.StableId} body=0x{entry.BodyRuntime:X8} " +
                $"transform=0x{entry.TransformRuntime:X8}" +
                (
                    entry.SelectorPreviewRuntime == 0u
                        ? ""
                        : $" selector=0x{entry.SelectorPreviewRuntime:X8}"
                ));
        }
        finally
        {
            c.Restore(state);
        }
    }

    static uint BuildNativeBank(
        CpuContext c, IMemory m, NativeVehicleBankSource bank)
    {
        byte[] binSource = bank.ReadBin();
        byte[]? animationSource = bank.ReadAnimation();
        uint bin = AllocateBytes(c, m, binSource);
        uint animation = animationSource == null
            ? 0u
            : AllocateBytes(c, m, animationSource);
        c.A0 = bin;
        c.A1 = animation;
        c.RA = CustomDispatchAddress;
        Dispatcher.Call(c, m, BuildNativeBankAddress);
        if (c.V0 == 0u)
            throw new OutOfMemoryException("native V8:2 object-bank build failed");
        return c.V0;
    }

    static uint AllocateTransformTable(
        CpuContext c, IMemory m, ushort[,] modes)
    {
        var data = new byte[TransformTableSize];
        for (int mode = 0; mode < TransformModeCount; mode++)
            for (int wheel = 0; wheel < TransformWheelCount; wheel++)
                BinaryPrimitives.WriteUInt16LittleEndian(
                    data.AsSpan((mode * TransformWheelCount + wheel) * 2, 2),
                    modes[mode, wheel]);
        return AllocateBytes(c, m, data);
    }

    static uint AllocatePowerupTable(
        CpuContext c, IMemory m, uint[] powerups)
    {
        var data = new byte[PowerupTableSize];
        for (int index = 0; index < powerups.Length; index++)
            BinaryPrimitives.WriteUInt32LittleEndian(
                data.AsSpan(index * 4, 4), powerups[index]);
        return AllocateBytes(c, m, data);
    }

    static uint AllocateBytes(CpuContext c, IMemory m, byte[] data)
    {
        c.A0 = checked((uint)data.Length);
        V82Compat.PcMalloc(c, m);
        uint pointer = c.V0;
        if (pointer == 0u)
            throw new OutOfMemoryException(
                $"V8:2 custom vehicle allocation failed for {data.Length} bytes");
        for (int index = 0; index < data.Length; index++)
            m.WriteU8(pointer + (uint)index, data[index]);
        return pointer;
    }

    static bool TryFindPackage(out string registryPath, out string archivePath)
    {
        var roots = new List<string>();
        string? package =
            Environment.GetEnvironmentVariable("RECOMPONE_V82_VEHICLE_PACKAGE");
        if (!string.IsNullOrWhiteSpace(package))
            roots.Add(Path.GetFullPath(package));
        if (!string.IsNullOrWhiteSpace(
                ConfigManager.Game.V82VehiclePackagePath))
            roots.Add(Path.GetFullPath(
                ConfigManager.Game.V82VehiclePackagePath));
        string? loose = Runtime.ResolveLoosePath();
        if (!string.IsNullOrWhiteSpace(loose))
            roots.Add(Path.GetFullPath(loose));
        roots.Add(AppContext.BaseDirectory);
        roots.Add(Environment.CurrentDirectory);

        var candidates = new List<string>();
        foreach (string root in roots.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            candidates.Add(root);
            string mods = Path.Combine(root, "mods");
            if (!Directory.Exists(mods))
                continue;
            candidates.AddRange(
                Directory.EnumerateDirectories(mods)
                    .OrderBy(path => path, StringComparer.OrdinalIgnoreCase));
        }

        foreach (string root in candidates.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            string candidateRegistry = Path.Combine(root, "VEHICLES.V8R");
            string candidateArchive = Path.Combine(root, "CUSTOM.EXP");
            bool hasRegistry = File.Exists(candidateRegistry);
            bool hasArchive = File.Exists(candidateArchive);
            if (hasRegistry != hasArchive)
                throw new FileNotFoundException(
                    "A custom vehicle package requires both VEHICLES.V8R and " +
                    $"CUSTOM.EXP in {root}");
            if (!hasRegistry) continue;
            registryPath = candidateRegistry;
            archivePath = candidateArchive;
            return true;
        }

        registryPath = "";
        archivePath = "";
        return false;
    }

    static void ParseRegistry(byte[] data)
    {
        if (data.Length < HeaderSize ||
            !data.AsSpan(0, 4).SequenceEqual("V8VR"u8))
            throw new InvalidDataException("VEHICLES.V8R header is invalid");

        ushort version = U16(data, 4);
        byte game = data[6];
        byte reserved = data[7];
        ushort count = U16(data, 8);
        ushort entrySize = U16(data, 10);
        uint tableOffset = U32(data, 12);
        uint stringOffset = U32(data, 16);
        bool legacy = version == 2 && entrySize == LegacyEntrySize;
        bool current = version == 3 && entrySize == EntrySize;
        bool previewCurrent =
            version == 4 && entrySize == PreviewEntrySize;
        if ((!legacy && !current && !previewCurrent) ||
            game != 2 || reserved != 0)
            throw new InvalidDataException(
                "VEHICLES.V8R is not a supported V8:2 registry");
        RequireRange(data, tableOffset, checked((uint)count * entrySize),
            "registry entry table");
        if (stringOffset > data.Length)
            throw new InvalidDataException("registry string table is invalid");
        if (count > byte.MaxValue - FirstCustomType + 1)
            throw new InvalidDataException(
                "custom vehicle types exceed the native one-byte identity field");

        var stableIds = new HashSet<string>(StringComparer.Ordinal);
        for (int index = 0; index < count; index++)
        {
            uint record = tableOffset + (uint)(index * entrySize);
            uint stableOffset = U32(data, record);
            uint displayOffset = U32(data, record + 4);
            ushort bodyArchiveIndex = U16(data, record + 8);
            ushort transformArchiveIndex = U16(data, record + 10);
            uint flags = U32(data, record + 12);
            ushort statSize = U16(data, record + 16);
            ushort bodyKind = U16(data, record + 18);
            ushort selectionOrder = U16(data, record + 20);
            ushort entryReserved = U16(data, record + 22);
            uint statOffset = U32(data, record + 24);
            uint transformOffset = U32(data, record + 28);
            uint powerupOffset = U32(data, record + 32);
            ushort rearSuspensionDamping = current || previewCurrent
                ? U16(data, record + 36)
                : (ushort)0;
            ushort extensionReserved = current || previewCurrent
                ? U16(data, record + 38)
                : (ushort)0;
            ushort selectorPreviewArchiveIndex = previewCurrent
                ? U16(data, record + 40)
                : NoArchiveIndex;
            ushort selectorPreviewBodyKind = previewCurrent
                ? U16(data, record + 42)
                : (ushort)0;
            if (flags != 0 || statSize != StatsSize ||
                transformArchiveIndex == NoArchiveIndex ||
                entryReserved != 0 || extensionReserved != 0)
                throw new InvalidDataException(
                    $"custom vehicle entry {index} has unsupported flags or sizes");

            RequireRange(data, statOffset, StatsSize, $"entry {index} stats");
            if (legacy)
                rearSuspensionDamping = U16(data, statOffset + 0x0C);
            if (rearSuspensionDamping > short.MaxValue)
                throw new InvalidDataException(
                    $"custom vehicle entry {index} rear suspension damping " +
                    "is outside the signed native coefficient range");
            RequireRange(data, transformOffset, TransformTableSize,
                $"entry {index} transformation table");
            RequireRange(data, powerupOffset, PowerupTableSize,
                $"entry {index} powerup table");
            if (data[statOffset + 0x0D] != index)
                throw new InvalidDataException(
                    $"custom vehicle entry {index} has a mismatched local type");

            string stableId = ReadString(data, stableOffset, stringOffset);
            string displayName = ReadString(data, displayOffset, stringOffset);
            if (!IsStableId(stableId) || !stableIds.Add(stableId))
                throw new InvalidDataException(
                    $"custom vehicle entry {index} has an invalid or duplicate stable ID");
            if (string.IsNullOrWhiteSpace(displayName))
                throw new InvalidDataException(
                    $"custom vehicle entry {index} has an empty display name");

            byte[] stats = data.AsSpan((int)statOffset, StatsSize).ToArray();
            stats[0x0D] = checked((byte)(FirstCustomType + index));
            var transformModes = new ushort[TransformModeCount, TransformWheelCount];
            for (int mode = 0; mode < TransformModeCount; mode++)
            {
                for (int wheel = 0; wheel < TransformWheelCount; wheel++)
                {
                    int item = mode * TransformWheelCount + wheel;
                    transformModes[mode, wheel] =
                        U16(data, transformOffset + (uint)(item * 2));
                }
            }
            for (int wheel = 0; wheel < TransformWheelCount; wheel++)
                if (transformModes[0, wheel] != 0)
                    throw new InvalidDataException(
                        $"custom vehicle entry {index} normal transform mode is not zero");

            var powerups = new uint[PowerupCount];
            for (int item = 0; item < powerups.Length; item++)
                powerups[item] = U32(data, powerupOffset + (uint)(item * 4));

            Entries.Add(new VehicleEntry(
                FirstCustomType + index,
                stableId,
                displayName,
                bodyArchiveIndex,
                transformArchiveIndex,
                selectorPreviewArchiveIndex,
                bodyKind,
                selectorPreviewBodyKind,
                selectionOrder,
                rearSuspensionDamping,
                stats,
                transformModes,
                powerups));
        }
    }

    static void ParseArchive(string path) =>
        Banks.AddRange(NativeVehicleBankSource.Open(path));

    static void ValidateCrossReferences()
    {
        var referenced = new HashSet<int>();
        foreach (VehicleEntry entry in Entries)
        {
            if (entry.BodyArchiveIndex >= Banks.Count ||
                entry.TransformArchiveIndex >= Banks.Count ||
                entry.BodyArchiveIndex == entry.TransformArchiveIndex ||
                !referenced.Add(entry.BodyArchiveIndex) ||
                !referenced.Add(entry.TransformArchiveIndex))
                throw new InvalidDataException(
                    $"vehicle {entry.StableId} has invalid or shared archive ownership");
            if (entry.SelectorPreviewArchiveIndex != NoArchiveIndex &&
                (
                    entry.SelectorPreviewArchiveIndex >= Banks.Count ||
                    !referenced.Add(entry.SelectorPreviewArchiveIndex)
                ))
                throw new InvalidDataException(
                    $"vehicle {entry.StableId} has invalid or shared selector preview ownership");

            byte[] transform = Banks[entry.TransformArchiveIndex].ReadBin();
            byte[] body = Banks[entry.BodyArchiveIndex].ReadBin();
            int bodySlotCount = BinSlotCount(body);
            if (entry.BodyKind >= bodySlotCount ||
                !BinSlotIsTopLevel(body, entry.BodyKind))
                throw new InvalidDataException(
                    $"vehicle {entry.StableId} body kind does not own a top-level object");
            if (entry.SelectorPreviewArchiveIndex != NoArchiveIndex)
            {
                byte[] selector =
                    Banks[entry.SelectorPreviewArchiveIndex].ReadBin();
                int selectorSlotCount = BinSlotCount(selector);
                if (entry.SelectorPreviewBodyKind >= selectorSlotCount ||
                    !BinSlotIsTopLevel(
                        selector, entry.SelectorPreviewBodyKind))
                    throw new InvalidDataException(
                        $"vehicle {entry.StableId} selector preview kind " +
                        "does not own a top-level object");
            }
            int slotCount = BinSlotCount(transform);
            for (int mode = 1; mode < TransformModeCount; mode++)
            {
                for (int wheel = 0; wheel < TransformWheelCount; wheel++)
                {
                    int kind = entry.TransformModes[mode, wheel];
                    if (kind >= slotCount ||
                        !BinSlotIsTopLevel(transform, kind) ||
                        !BinTransformRootHasNativeState(transform, kind) ||
                        !BinSlotHasInstanceMarker(transform, kind))
                        throw new InvalidDataException(
                            $"vehicle {entry.StableId} transform mode {mode} " +
                            $"wheel {wheel} lacks an independent native wheel root");
                }
            }
        }
        if (referenced.Count != Banks.Count)
            throw new InvalidDataException(
                "CUSTOM.EXP contains unreferenced banks; implicit donor/support data is forbidden");
    }

    static int BinSlotCount(byte[] bin)
    {
        if (bin.Length < 0x1C)
            throw new InvalidDataException("native BIN header is truncated");
        uint count = U32(bin, 0x18);
        if (count > int.MaxValue ||
            0x1Cu + count * 0x1Cu > bin.Length)
            throw new InvalidDataException("native BIN slot table is truncated");
        return (int)count;
    }

    static bool BinSlotIsTopLevel(byte[] bin, int target)
    {
        int count = BinSlotCount(bin);
        for (int index = 0; index < count; index++)
        {
            int offset = 0x1C + index * 0x1C;
            int child = U16(bin, offset + 0x1A);
            int guard = 0;
            while (child != 0xFFFF && guard++ < count)
            {
                if (child == target) return false;
                if (child < 0 || child >= count)
                    throw new InvalidDataException(
                        "native BIN slot hierarchy is invalid");
                child = U16(bin, 0x1C + child * 0x1C + 0x18);
            }
            if (guard > count)
                throw new InvalidDataException("native BIN slot hierarchy cycles");
        }
        return true;
    }

    static bool BinSlotHasInstanceMarker(byte[] bin, int root)
    {
        int count = BinSlotCount(bin);
        uint groupCount = U32(bin, 0);
        int child = U16(bin, 0x1C + root * 0x1C + 0x1A);
        int guard = 0;
        while (child != 0xFFFF && guard++ < count)
        {
            if (child < 0 || child >= count)
                throw new InvalidDataException(
                    "native BIN transformation hierarchy is invalid");
            int offset = 0x1C + child * 0x1C;
            ushort renderKey = U16(bin, offset);
            if ((renderKey & 0xC000) == 0xC000 &&
                (renderKey & 0x07FF) < groupCount)
                return true;
            child = U16(bin, offset + 0x18);
        }
        if (guard > count)
            throw new InvalidDataException(
                "native BIN transformation hierarchy cycles");
        return false;
    }

    static bool BinTransformRootHasNativeState(byte[] bin, int root)
    {
        int offset = 0x1C + root * 0x1C;
        return U16(bin, offset + 0x02) != 0xFFFF &&
               U16(bin, offset + 0x16) == 0xAAAA;
    }

    static string ReadString(byte[] data, uint offset, uint stringTable)
    {
        if (offset < stringTable || offset >= data.Length)
            throw new InvalidDataException("registry string offset is invalid");
        int end = Array.IndexOf(data, (byte)0, (int)offset);
        if (end < 0)
            throw new InvalidDataException("registry string is unterminated");
        return StrictUtf8.GetString(data, (int)offset, end - (int)offset);
    }

    static bool IsStableId(string value) =>
        value.Length != 0 && value.All(character =>
            character is >= 'a' and <= 'z' ||
            character is >= '0' and <= '9' ||
            character is '_' or '-' or '.');

    static ushort U16(byte[] data, int offset) =>
        BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(offset, 2));

    static ushort U16(byte[] data, uint offset) => U16(data, checked((int)offset));

    static uint U32(byte[] data, int offset) =>
        BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset, 4));

    static uint U32(byte[] data, uint offset) => U32(data, checked((int)offset));

    static uint Be32(byte[] data, uint offset) =>
        BinaryPrimitives.ReadUInt32BigEndian(
            data.AsSpan(checked((int)offset), 4));

    static void RequireRange(
        byte[] data, uint offset, uint size, string context)
    {
        uint length = checked((uint)data.Length);
        if (offset > length || size > length - offset)
            throw new InvalidDataException($"{context} is truncated");
    }

    sealed class VehicleEntry(
        int type,
        string stableId,
        string displayName,
        int bodyArchiveIndex,
        int transformArchiveIndex,
        int selectorPreviewArchiveIndex,
        int bodyKind,
        int selectorPreviewBodyKind,
        int selectionOrder,
        ushort rearSuspensionDamping,
        byte[] stats,
        ushort[,] transformModes,
        uint[] powerups)
    {
        public int Type { get; } = type;
        public string StableId { get; } = stableId;
        public string DisplayName { get; } = displayName;
        public int BodyArchiveIndex { get; } = bodyArchiveIndex;
        public int TransformArchiveIndex { get; } = transformArchiveIndex;
        public int SelectorPreviewArchiveIndex { get; } =
            selectorPreviewArchiveIndex;
        public int BodyKind { get; } = bodyKind;
        public int SelectorPreviewBodyKind { get; } =
            selectorPreviewBodyKind;
        public int SelectionOrder { get; } = selectionOrder;
        public ushort RearSuspensionDamping { get; } = rearSuspensionDamping;
        public byte[] Stats { get; } = stats;
        public ushort[,] TransformModes { get; } = transformModes;
        public uint[] Powerups { get; } = powerups;
        public uint BodyRuntime { get; set; }
        public uint TransformRuntime { get; set; }
        public uint SelectorPreviewRuntime { get; set; }
        public uint StatsRuntime { get; set; }
        public uint TransformTableRuntime { get; set; }
        public uint PowerupTableRuntime { get; set; }
    }
}
