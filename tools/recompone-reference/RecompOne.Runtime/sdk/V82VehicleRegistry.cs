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
    const uint SelectorBankPrepareReturn = 0x80106BA8u;
    const uint SelectorBankFinalizeReturn = 0x80106C58u;
    const uint SelectorScaleReturn = 0x80106CD4u;
    const uint SelectorGuestZoom = 0x015Au;
    const uint SelectorDriverNameReturn = 0x80106A7Cu;
    const uint SelectorVehicleNameReturn = 0x80106B14u;
    const uint SelectorPortraitBaseReturn = 0x801069ECu;
    const uint SelectorPortraitPoseReturn = 0x80106A00u;
    const uint SelectorEnemyHeaderReturn = 0x80107A1Cu;
    const uint SelectorEnemyHeaderText = 0x801008E8u;
    const int SelectorPortraitWidth = 260;
    const int SelectorPortraitHeight = 422;
    const uint ShellDisplayXAddress = 0x8006B7C0u;
    const uint ShellDisplayYAddress = 0x8006B7C4u;

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
    static uint _initializingGuestCamera;
    static VehicleEntry? _defaultReplacementEntry;
    static int _selectorGuestIndex = -1;
    static int _selectorFirstRetailSlot;
    static int _selectorLastRetailSlot;
    static int _selectorProxySlot;
    static int _selectorPreviousSlot = -1;
    static int _selectorPlayer;
    static uint _selectorContext;
    static int _selectorStableFrames;
    static int _selectorStableGuest = -1;
    static bool _selectorEnemyPhase;
    static int _selectorEnemyFrames;
    static int _selectorAcceptedGuest = -1;
    static int _selectorAcceptedProxySlot = -1;
    static uint _selectorPreviewObject;
    static readonly Dictionary<string, ushort[]> SelectorPortraitPixels =
        new(StringComparer.OrdinalIgnoreCase);
    static readonly HashSet<int> SelectorProofCaptured = [];
    static readonly string[] SelectorVehicleNames =
    [
        "'67 Rattler",
        "'70 Clydesdale",
        "'74 Strider",
        "'69 Jefferson",
        "'70 Van",
        "'72 Moth Truck",
        "'73 Glenn 4x4",
        "'75 Palamino",
        "'76 Leprechaun",
        "'70 Stag Pickup",
        "'66 School Bus",
        "'69 Manta",
    ];
    // The retail result screen indexes an 18-entry table of native V8:2
    // character/vehicle/audio stems. Imported types intentionally live outside
    // that range, so feeding their stable type IDs into the retail table reads
    // unrelated memory. Each legacy V8 driver uses the closest existing V8:2
    // character record for result-screen XA playback until a package supplies
    // an independently authored result-audio record. Exact returning
    // characters reuse their own native sequel record.
    static readonly IReadOnlyDictionary<string, int> ResultVoiceProxySlots =
        new Dictionary<string, int>(StringComparer.Ordinal)
        {
            ["guest.v8.chassey_blue"] = 15,
            ["guest.v8.slick_clyde"] = 9,
            ["guest.v8.sheila"] = 0,
            ["guest.v8.john_torque"] = 1,
            ["guest.v8.dave"] = 5,
            ["guest.v8.convoy"] = 4,
            ["guest.v8.loki"] = 12,
            ["guest.v8.houston_3"] = 3,
            ["guest.v8.boogie"] = 11,
            ["guest.v8.beezwax"] = 13,
            ["guest.v8.molo"] = 8,
            ["guest.v8.sid_burn"] = 17,
        };
    const uint ResultVoiceTableAddress = 0x8006383Cu;
    static readonly string? DefaultReplacementStableId =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_DEFAULT_REPLACEMENT");
    static readonly bool CaptureNativeSelectorProof =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR") == "1";
    static readonly bool CaptureNativeSelectorSettle =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_CAPTURE_V82_SELECTOR_SETTLE") == "1";
    static readonly bool CaptureNativeSelectorTurns =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS") != "0";
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
    public static bool HasAnySelection =>
        Enumerable.Range(0, PlayerSelectionCount)
            .Any(player => SelectedTypeForPlayer(player) >= 0);

    public static bool IsVehicleObject(uint objectAddress) =>
        ObjectEntries.ContainsKey(objectAddress);

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
        V82Compat.ReleaseSelectorVramReservation(c, m);
        _selectorGuestIndex = -1;
        _selectorPreviousSlot = -1;
        _selectorFirstRetailSlot = 0;
        _selectorLastRetailSlot = RetailVehicleCount - 1;
        _selectorProxySlot = 0;
        // Native selector context 1 is player one and context 2 is player
        // two. Context 0 is the AI/enemy pass and must not displace player
        // one's accepted guest.
        _selectorContext = c.A1;
        _selectorPlayer = c.A1 == 2u ? 1 : 0;
        _selectorStableFrames = 0;
        _selectorStableGuest = -1;
        _selectorEnemyPhase = false;
        _selectorEnemyFrames = 0;
        _selectorAcceptedGuest = -1;
        _selectorAcceptedProxySlot = -1;
        _selectorPreviewObject = 0u;
        SelectorProofCaptured.Clear();
        InputManager.SignalScriptStage(
            _selectorContext == 0u ? "choose_enemies" : "choose_player");
    }

    public static void EndNativeSelector(CpuContext c, IMemory m)
    {
        int guest = _selectorAcceptedGuest >= 0
            ? _selectorAcceptedGuest
            : NativeSelectorGuestIndex;
        int proxySlot = _selectorAcceptedGuest >= 0
            ? _selectorAcceptedProxySlot
            : _selectorProxySlot;
        if (guest >= 0 && guest < Entries.Count &&
            c.V0 == (uint)proxySlot)
        {
            VehicleEntry entry = Entries[guest];
            c.V0 = checked((uint)entry.Type);
            SelectTypeForPlayer(_selectorPlayer, entry.Type);
        }
        _selectorGuestIndex = -1;
        _selectorEnemyPhase = false;
        _selectorAcceptedGuest = -1;
        _selectorAcceptedProxySlot = -1;
        _selectorPreviousSlot = -1;
        _selectorPreviewObject = 0u;
        V82Compat.ReleaseSelectorVramReservation(c, m);
        _selectorContext = uint.MaxValue;
    }

    /// <summary>
    /// Extends the native 18-entry carousel without replacing it. The retail
    /// selector still performs all input repeat, transition, and availability
    /// handling; this seam inserts the packaged V8 entries only at the wrap.
    /// </summary>
    public static uint ResolveNativeSelectorSlot(CpuContext c, IMemory m)
    {
        uint slot = c.FP;
        if (Entries.Count == 0 || _selectorEnemyPhase)
            return slot;

        int current = checked((int)slot);
        uint input = m.ReadU32(SelectorInputAddress);
        bool left = (input & 0x80000000u) != 0u;
        bool right = (input & 0x20000000u) != 0u;
        int guest = NativeSelectorGuestIndex;
        int previousGuest = guest;
        if (TraceNativeSelectorInput && (left || right))
            Console.Error.WriteLine(
                $"[V82SelectorSlot] input=0x{input:X8} current={current} " +
                $"previous={_selectorPreviousSlot} guest={guest} " +
                $"proxy={_selectorProxySlot}");

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
                _selectorFirstRetailSlot = current;
                _selectorLastRetailSlot = _selectorPreviousSlot;
                guest = 0;
                _selectorProxySlot = _selectorFirstRetailSlot;
                current = _selectorProxySlot;
            }
            else if (left && current > _selectorPreviousSlot)
            {
                _selectorFirstRetailSlot = _selectorPreviousSlot;
                _selectorLastRetailSlot = current;
                guest = Entries.Count - 1;
                _selectorProxySlot = _selectorFirstRetailSlot;
                current = _selectorProxySlot;
            }
            else
            {
                _selectorPreviousSlot = current;
                return slot;
            }
        }
        else if (right && current != _selectorPreviousSlot)
        {
            if (guest + 1 < Entries.Count)
            {
                guest++;
                current = _selectorProxySlot;
            }
            else
            {
                _selectorGuestIndex = -1;
                _selectorPreviousSlot = _selectorFirstRetailSlot;
                V82Compat.ReleaseSelectorVramReservation(c, m);
                return checked((uint)_selectorFirstRetailSlot);
            }
        }
        else if (left && current != _selectorPreviousSlot)
        {
            if (guest > 0)
            {
                guest--;
                current = _selectorProxySlot;
            }
            else
            {
                _selectorGuestIndex = -1;
                _selectorPreviousSlot = _selectorLastRetailSlot;
                V82Compat.ReleaseSelectorVramReservation(c, m);
                return checked((uint)_selectorLastRetailSlot);
            }
        }
        else
        {
            current = _selectorProxySlot;
        }

        if (guest != previousGuest)
            V82Compat.ReleaseSelectorVramReservation(c, m);
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
        if (_selectorEnemyPhase)
        {
            int enemyFrame = ++_selectorEnemyFrames;
            if (CaptureNativeSelectorProof && enemyFrame == 80)
                HostWindow.RequestDisplayCapture("native_enemy_selector");
            return true;
        }

        int guest = NativeSelectorGuestIndex;
        if (TraceNativeSelectorInput && guest >= 0)
        {
            uint input = m.ReadU32(SelectorInputAddress);
            if (input != 0u)
                Console.Error.WriteLine(
                    $"[V82Selector] guest={guest} input=0x{input:X8}");
        }
        if (guest < 0 || guest >= Entries.Count ||
            SelectorProofCaptured.Contains(guest))
            return true;

        int frame = ++_selectorStableFrames;
        if (TraceNativeSelectorPhysics && _selectorPreviewObject != 0u &&
            frame <= 220)
            TraceSelectorPhysics(m, guest, frame, _selectorPreviewObject);
        if (!CaptureNativeSelectorProof)
            return true;
        if (CaptureNativeSelectorSettle && frame <= 64 && frame % 2 == 0)
            HostWindow.RequestDisplayCapture(
                $"native_guest_{guest:00}_settle_{frame:000}");
        if (frame == 80)
            HostWindow.RequestDisplayCapture($"native_guest_{guest:00}");
        else if (CaptureNativeSelectorTurns &&
                 frame > 80 && frame <= 200 && (frame - 80) % 4 == 0)
            HostWindow.RequestDisplayCapture(
                $"native_guest_{guest:00}_turn_{(frame - 80) / 4:000}");
        if (frame == 200)
            SelectorProofCaptured.Add(guest);
        return true;
    }

    /// <summary>
    /// V8:2 keeps player and enemy selection inside one native selector call.
    /// Observe its exact stock "SELECT ENEMIES" header draw so imported player
    /// substitutions stop before the enemy portraits and previews are built.
    /// The accepted guest is retained separately until the outer selector
    /// returns its player slot.
    /// </summary>
    public static void ObserveNativeSelectorCall(CpuContext c, IMemory m)
    {
        if (_selectorEnemyPhase ||
            c.RA != SelectorEnemyHeaderReturn ||
            c.A1 != SelectorEnemyHeaderText ||
            _selectorContext is not (1u or 2u))
            return;

        int guest = NativeSelectorGuestIndex;
        if (guest < 0 || guest >= Entries.Count)
            return;

        _selectorAcceptedGuest = guest;
        _selectorAcceptedProxySlot = _selectorProxySlot;
        _selectorEnemyPhase = true;
        _selectorEnemyFrames = 0;
        _selectorGuestIndex = -1;
        _selectorPreviousSlot = -1;
        ReleaseSelectorPreviewForEnemyPhase(c, m);
        InputManager.SignalScriptStage("choose_enemies");
        Console.Error.WriteLine(
            $"[V82Vehicles] entered native enemy selector after accepting " +
            $"guest={guest}; stock portraits and previews restored");
    }

    static void ReleaseSelectorPreviewForEnemyPhase(CpuContext c, IMemory m)
    {
        V82Compat.ReleaseSelectorVramReservation(c, m);
        _selectorPreviewObject = 0u;
    }

    public static uint NativeSelectorVariant(uint retailVariant) =>
        NativeSelectorGuestIndex >= 0 ? 0u : retailVariant;

    public static uint NativeSelectorStatsPointer(
        CpuContext c, IMemory m, uint retailPointer, uint field)
    {
        if (!TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return retailPointer;
        EnsureSelectorRuntime(entry, c, Dispatcher.UnwrapMemory(m));
        return entry.StatsRuntime + field;
    }

    public static bool OverrideNativeSelectorText(CpuContext c, IMemory m)
    {
        V82Compat.TraceNativeSelectorCall(c, m);
        if (!TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return true;

        // The imported 260x422 V8 banner already contains the original
        // driver-name treatment. Drawing V8:2's separate caption over it
        // produces two offset copies at the lower edge.
        if (c.RA == SelectorDriverNameReturn)
            return false;

        string? text = c.RA switch
        {
            SelectorVehicleNameReturn => NativeSelectorGuestIndex <
                SelectorVehicleNames.Length
                ? SelectorVehicleNames[NativeSelectorGuestIndex]
                : entry.DisplayName,
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
    /// Replaces the stock V8:2 player portrait with the exact settled V8
    /// 260x422 left banner. V8 composes its faction and character VLC layers
    /// asynchronously; the durable package stores that completed composition.
    /// Uploading it with the ordinary PS1 GP0 LoadImage command keeps the
    /// result in native VRAM for both the software and Enhanced renderers.
    ///
    /// The second V8:2 pose-layer call must be skipped for imported entries:
    /// the complete V8 banner already contains both layers. Enemy selection
    /// (context 0) is never intercepted.
    /// </summary>
    public static bool BeginNativeSelectorPortrait(CpuContext c, IMemory m)
    {
        if ((c.RA != SelectorPortraitBaseReturn &&
             c.RA != SelectorPortraitPoseReturn) ||
            _selectorContext is not (1u or 2u) ||
            !TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return true;

        if (c.RA == SelectorPortraitPoseReturn)
            return false;

        int guest = NativeSelectorGuestIndex;
        m = Dispatcher.UnwrapMemory(m);
        ushort[] pixels = LoadSelectorPortraitPixels(guest);
        int x = checked((int)m.ReadU16(ShellDisplayXAddress) + (short)c.A1);
        int y = checked((int)m.ReadU16(ShellDisplayYAddress) + (short)c.A2);
        UploadSelectorPortrait(x, y, pixels);
        Console.Error.WriteLine(
            $"[V82SelectorPortrait] guest={guest} format=RGB555 " +
            $"size={SelectorPortraitWidth}x{SelectorPortraitHeight} " +
            $"destination=({x},{y})");
        return false;
    }

    /// <summary>
    /// The selector surrounds its constructor with two relocation calls for
    /// raw retail disc records. Packaged preview banks have already passed
    /// through V8:2's native bank builder, so replaying raw relocation would
    /// treat relocated pointers as byte counts. Skip only those two calls;
    /// the unchanged constructor consumes the prepared native bank below.
    /// </summary>
    public static bool PrepareNativeSelectorBank(CpuContext c, IMemory m)
    {
        V82Compat.TraceNativeSelectorCall(c, m);
        if ((c.RA != SelectorBankPrepareReturn &&
             c.RA != SelectorBankFinalizeReturn) ||
            !TrySelectorEntry(out VehicleEntry? entry) || entry == null)
            return true;

        m = Dispatcher.UnwrapMemory(m);
        EnsureSelectorRuntime(entry, c, m);
        if (TraceNativeSelectorInput)
            Console.Error.WriteLine(
                $"[V82SelectorBank] guest={NativeSelectorGuestIndex} " +
                $"caller=0x{c.RA:X8} prepared-runtime=" +
                $"0x{entry.SelectorPreviewRuntime:X8}");
        return false;
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
        EnsureSelectorRuntime(entry, c, m);
        c.A0 = entry.SelectorPreviewRuntime;
        c.A1 = checked((uint)entry.SelectorPreviewBodyKind);
        c.A2 = entry.StatsRuntime;
        _constructingSelectorPreview = true;
        _constructingEntry = entry;
        V82Compat.BeginGuestVramClaim(reusable: true);
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
    /// Compatibility no-op for generated projects produced before object
    /// identity retirement moved into PcFree. Vehicle event 2 can call
    /// func_8002CC08 while retaining the destroyed vehicle for the result
    /// screen, so that routine is not an object-lifetime boundary.
    /// </summary>
    public static bool ReleaseVehicleMapping(CpuContext c, IMemory m)
        => true;

    /// <summary>
    /// Retires guest identity at the actual object-storage lifetime boundary.
    /// PcFree invokes this only after it has proved that the pointer owns a
    /// live PC-heap allocation, preventing a destroy/model teardown from
    /// invalidating the later event-4 result-screen cleanup callback.
    /// </summary>
    internal static void ReleaseFreedObjectMapping(
        uint pointer, CpuContext c, IMemory m)
    {
        if (_selectorPreviewObject == pointer)
        {
            _selectorPreviewObject = 0u;
            V82Compat.ReleaseSelectorVramReservation(c, m);
        }
        ObjectEntries.Remove(pointer);
        ObjectUpgradeStatus.Remove(pointer);
    }

    internal static void RegisterObjectMappingForProbe(uint pointer)
    {
        if (Entries.Count == 0)
            throw new InvalidOperationException(
                "guest identity probe requires a loaded vehicle package");
        ObjectEntries[pointer] = 0;
    }

    internal static bool HasObjectMappingForProbe(uint pointer) =>
        ObjectEntries.ContainsKey(pointer);

    /// <summary>
    /// Record only follow cameras owned by imported V8 vehicles. The sequel
    /// continues to create and initialize the camera through its normal
    /// object lifecycle.
    /// </summary>
    public static bool BeginOriginalV8CameraValues(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint camera = c.A0;
        uint vehicle = camera == 0u ? 0u : m.ReadU32(camera + 0x80u);
        _initializingGuestCamera =
            vehicle != 0u && ObjectEntries.ContainsKey(vehicle)
                ? camera
                : 0u;
        return true;
    }

    /// <summary>
    /// Preserve V8:2's camera code and viewport handling, then substitute the
    /// exact first-game follow distance for imported cars. V8 FUN_8003d8c4
    /// stores vehicle Z + 0x1E000; the equivalent V8:2 fields are owner +0x58
    /// and camera +0x9C.
    /// </summary>
    public static void FinishOriginalV8CameraValues(CpuContext c, IMemory m)
    {
        uint camera = _initializingGuestCamera;
        _initializingGuestCamera = 0u;
        if (camera == 0u)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint vehicle = m.ReadU32(camera + 0x80u);
        if (!ObjectEntries.ContainsKey(vehicle))
            return;

        int vehicleZ = unchecked((int)m.ReadU32(vehicle + 0x58u));
        int cameraZ = unchecked(vehicleZ + 0x1E000);
        m.WriteU32(camera + 0x9Cu, unchecked((uint)cameraZ));
        Console.Error.WriteLine(
            $"[V82Vehicles] applied original V8 camera distance " +
            $"vehicle=0x{vehicle:X8} camera=0x{camera:X8} z={cameraZ}");
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
        if (guest >= 0 && guest < Entries.Count)
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
        ValidateSelectorPortraits(_loadedPackageRoot);
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
        ValidateSelectorPortraits(root);
        return $"vehicles={Entries.Count} banks={Banks.Count} " +
            $"portraits={Entries.Count}";
    }

    static void ValidateSelectorPortraits(string root)
    {
        for (int index = 0; index < Entries.Count; index++)
        {
            string path = Path.Combine(
                root, "SHELL", $"SELECTOR_{index:00}.PPM");
            _ = BuildSelectorPortraitPixels(path);
        }
    }

    static void LoadAndValidate(string registryPath, string archivePath)
    {
        Entries.Clear();
        Banks.Clear();
        ObjectEntries.Clear();
        ObjectUpgradeStatus.Clear();
        SelectorPortraitPixels.Clear();
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

    /// <summary>
    /// Resolves the native result-screen XA stem for an imported vehicle
    /// without allowing its out-of-range stable type to index the retail
    /// 18-entry table. Unknown third-party packages deliberately fall back to
    /// slot zero, which is a valid native record and keeps the unchanged result
    /// flow alive while providing deterministic audio.
    /// </summary>
    public static uint ResultVoicePointer(IMemory m, int type)
    {
        if (!TryEntryForType(unchecked((uint)type), out VehicleEntry? entry) ||
            entry == null)
            return 0u;

        int slot = ResultVoiceProxySlots.TryGetValue(
            entry.StableId, out int mappedSlot)
                ? mappedSlot
                : 0;
        if ((uint)slot >= RetailVehicleCount)
            throw new InvalidOperationException(
                $"result voice proxy slot {slot} for {entry.StableId} is invalid");

        m = Dispatcher.UnwrapMemory(m);
        uint pointer = m.ReadU32(
            ResultVoiceTableAddress + checked((uint)(slot * 16)));
        if (pointer < 0x80010000u || pointer >= 0x80200000u)
            throw new InvalidOperationException(
                $"result voice proxy for {entry.StableId} resolved invalid " +
                $"pointer 0x{pointer:X8}");
        return pointer;
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

        // func_80036C2C assigns the resolved vehicle callback to the source
        // object before func_80031DDC clones it. Both objects subsequently
        // participate in the native object lifecycle: the clone drives the
        // match, while the source remains in the object lists and receives
        // the global event-4 teardown callback. Record the same independent
        // identity on the source at the exact point where it acquires our
        // callback. Registering only the clone leaves a valid custom callback
        // with no registry entry when func_800333D0 tears the match down.
        ObjectEntries[source] = entry.Type - FirstCustomType;
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
            if (entry.StatsRuntime == 0u)
                entry.StatsRuntime = AllocateBytes(c, m, entry.Stats);
            if (entry.TransformTableRuntime == 0u)
                entry.TransformTableRuntime =
                    AllocateTransformTable(c, m, entry.TransformModes);
            if (entry.PowerupTableRuntime == 0u)
                entry.PowerupTableRuntime =
                    AllocatePowerupTable(c, m, entry.Powerups);
            entry.BodyRuntime =
                BuildNativeBank(c, m, Banks[entry.BodyArchiveIndex]);
            if (entry.TransformRuntime == 0u)
                entry.TransformRuntime =
                    BuildNativeBank(c, m, Banks[entry.TransformArchiveIndex]);
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

    static void EnsureSelectorRuntime(
        VehicleEntry entry, CpuContext c, IMemory m)
    {
        if (entry.SelectorPreviewRuntime != 0u)
            return;
        if (entry.SelectorPreviewArchiveIndex == NoArchiveIndex)
            throw new InvalidDataException(
                $"custom vehicle {entry.StableId} has no selector preview bank");

        var state = c.Snapshot();
        try
        {
            if (entry.StatsRuntime == 0u)
                entry.StatsRuntime = AllocateBytes(c, m, entry.Stats);
            if (entry.TransformRuntime == 0u)
                entry.TransformRuntime =
                    BuildNativeBank(c, m, Banks[entry.TransformArchiveIndex]);
            entry.SelectorPreviewRuntime = BuildNativeBank(
                c, m, Banks[entry.SelectorPreviewArchiveIndex]);
            Console.Error.WriteLine(
                $"[V82Vehicles] built {entry.StableId} selector=" +
                $"0x{entry.SelectorPreviewRuntime:X8} wheels=" +
                $"0x{entry.TransformRuntime:X8}");
        }
        finally
        {
            c.Restore(state);
        }
    }

    static ushort[] LoadSelectorPortraitPixels(int guestIndex)
    {
        if (string.IsNullOrEmpty(_loadedPackageRoot))
            throw new InvalidOperationException(
                "native selector portrait has no loaded package root");

        string fileName = $"SELECTOR_{guestIndex:00}.PPM";
        string path = Path.Combine(
            _loadedPackageRoot, "SHELL", fileName);
        return BuildSelectorPortraitPixels(path);
    }

    static ushort[] BuildSelectorPortraitPixels(string path)
    {
        path = Path.GetFullPath(path);
        if (SelectorPortraitPixels.TryGetValue(path, out ushort[]? cached))
            return cached;

        byte[] source = File.ReadAllBytes(path);
        int cursor = 0;
        string magic = ReadPpmToken(source, ref cursor);
        int width = int.Parse(ReadPpmToken(source, ref cursor));
        int height = int.Parse(ReadPpmToken(source, ref cursor));
        int maximum = int.Parse(ReadPpmToken(source, ref cursor));
        if (cursor >= source.Length ||
            !char.IsWhiteSpace((char)source[cursor]))
            throw new InvalidDataException(
                $"selector portrait has no PPM header terminator: {path}");
        if (source[cursor] == '\r' &&
            cursor + 1 < source.Length && source[cursor + 1] == '\n')
            cursor += 2;
        else
            cursor++;
        int pixelLength = checked(width * height * 3);
        if (magic != "P6" ||
            width != SelectorPortraitWidth ||
            height != SelectorPortraitHeight ||
            maximum != 255 ||
            cursor + pixelLength != source.Length)
            throw new InvalidDataException(
                $"selector portrait must be an exact " +
                $"{SelectorPortraitWidth}x{SelectorPortraitHeight} " +
                $"binary PPM: {path}");

        ushort[] pixels = new ushort[checked(width * height)];
        for (int input = cursor, output = 0;
             input < source.Length;
             input += 3, output++)
        {
            pixels[output] = checked((ushort)(
                source[input] >> 3 |
                (source[input + 1] >> 3) << 5 |
                (source[input + 2] >> 3) << 10));
        }
        SelectorPortraitPixels[path] = pixels;
        return pixels;
    }

    static string ReadPpmToken(byte[] data, ref int cursor)
    {
        while (cursor < data.Length)
        {
            while (cursor < data.Length &&
                   char.IsWhiteSpace((char)data[cursor]))
                cursor++;
            if (cursor >= data.Length || data[cursor] != '#')
                break;
            while (cursor < data.Length && data[cursor] != '\n')
                cursor++;
        }
        int start = cursor;
        while (cursor < data.Length &&
               !char.IsWhiteSpace((char)data[cursor]))
            cursor++;
        if (start == cursor)
            throw new InvalidDataException(
                "native selector portrait PPM is truncated");
        return Encoding.ASCII.GetString(data, start, cursor - start);
    }

    /// <summary>
    /// Returns the exact RGB555 pixels used by the live selector.
    /// </summary>
    public static ushort[] BuildSelectorPortraitPixelsForProbe(string path)
        => BuildSelectorPortraitPixels(path);

    public static void UploadSelectorPortraitForProbe(
        int x, int y, ushort[] pixels) =>
        UploadSelectorPortrait(x, y, pixels);

    static void UploadSelectorPortrait(int x, int y, ushort[] pixels)
    {
        if (pixels.Length != SelectorPortraitWidth * SelectorPortraitHeight)
            throw new InvalidDataException(
                $"selector portrait has {pixels.Length} RGB555 pixels, " +
                $"expected {SelectorPortraitWidth * SelectorPortraitHeight}");
        if (x < 0 || y < 0 ||
            x + SelectorPortraitWidth > Gpu.VramWidth ||
            y + SelectorPortraitHeight > Gpu.VramHeight)
            throw new InvalidOperationException(
                $"selector portrait destination ({x},{y}) is outside VRAM");
        Gpu gpu = Runtime.Gpu ??
            throw new InvalidOperationException(
                "selector portrait upload requires an initialized GPU");
        gpu.WriteGp0(0xA0000000u);
        gpu.WriteGp0(
            checked((uint)(ushort)x) |
            checked((uint)(ushort)y) << 16);
        gpu.WriteGp0(
            checked((uint)SelectorPortraitWidth) |
            checked((uint)SelectorPortraitHeight) << 16);
        for (int index = 0; index < pixels.Length; index += 2)
        {
            uint word = pixels[index];
            if (index + 1 < pixels.Length)
                word |= (uint)pixels[index + 1] << 16;
            gpu.WriteGp0(word);
        }
    }

#if false
    // Superseded 2026-07-30: generated TIM/PPM conversion is intentionally
    // excluded. The imported roster now consumes the exact original V8
    // CHARSEL1 records through V8:2's compatible native VLC path.
    static byte[] BuildGeneratedSelectorPortraitRecord(string path)
    {
        path = Path.GetFullPath(path);
        if (SelectorPortraitRecords.TryGetValue(path, out byte[]? cached))
            return cached;

        byte[] file = File.ReadAllBytes(path);
        int cursor = 0;
        string magic = ReadPpmToken(file, ref cursor);
        int width = int.Parse(ReadPpmToken(file, ref cursor));
        int height = int.Parse(ReadPpmToken(file, ref cursor));
        int maximum = int.Parse(ReadPpmToken(file, ref cursor));
        if (cursor >= file.Length ||
            !char.IsWhiteSpace((char)file[cursor]))
            throw new InvalidDataException(
                $"selector portrait has no PPM header terminator: {path}");
        if (file[cursor] == '\r' &&
            cursor + 1 < file.Length && file[cursor + 1] == '\n')
            cursor += 2;
        else
            cursor++;
        int pixelLength = checked(width * height * 3);
        if (magic != "P6" || width != SelectorPortraitSourceWidth ||
            height != SelectorPortraitSourceHeight || maximum != 255 ||
            cursor + pixelLength != file.Length)
            throw new InvalidDataException(
                $"selector portrait has invalid PPM layout: {path}");

        byte[] resized = ResizeSelectorPortrait(
            file.AsSpan(cursor, pixelLength));
        var (palette, indices) = QuantizeSelectorPortrait(resized);
        int rawLength = checked(
            SelectorPortraitWordsPerRow * SelectorPortraitHeight * 2);
        byte[] record =
            new byte[SelectorPortraitTimHeaderSize + rawLength];
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(0, 2), checked((ushort)SelectorPortraitWidth));
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(2, 2), checked((ushort)SelectorPortraitHeight));
        // Standard PS1 TIM: magic, 16-bpp flags, image-block byte count,
        // authored TIM origin, dimensions in 16-bit words, then RGB555 data.
        // func_801109FC already has a dedicated path for this format, avoiding
        // the fixed VLC scratch area that a full-size raw image would overrun.
        BinaryPrimitives.WriteUInt32LittleEndian(
            record.AsSpan(4, 4), SelectorPortraitTimMagic);
        BinaryPrimitives.WriteUInt32LittleEndian(
            record.AsSpan(8, 4), SelectorPortraitTim4BppWithClut);
        BinaryPrimitives.WriteUInt32LittleEndian(
            record.AsSpan(12, 4), SelectorPortraitClutBlockSize);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(16, 2), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(18, 2), 480);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(20, 2), SelectorPortraitPaletteEntries);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(22, 2), 1);
        for (int index = 0; index < palette.Length; index++)
            BinaryPrimitives.WriteUInt16LittleEndian(
                record.AsSpan(24 + index * 2, 2),
                checked((ushort)(0x8000u | palette[index])));

        BinaryPrimitives.WriteUInt32LittleEndian(
            record.AsSpan(SelectorPortraitImageBlockOffset, 4),
            checked((uint)(12 + rawLength)));
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(SelectorPortraitImageBlockOffset + 4, 2), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(SelectorPortraitImageBlockOffset + 6, 2), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(SelectorPortraitImageBlockOffset + 8, 2),
            SelectorPortraitWordsPerRow);
        BinaryPrimitives.WriteUInt16LittleEndian(
            record.AsSpan(SelectorPortraitImageBlockOffset + 10, 2),
            SelectorPortraitHeight);

        int output = SelectorPortraitTimHeaderSize;
        for (int pixelIndex = 0; pixelIndex < indices.Length; pixelIndex += 4)
        {
            ushort pixel = checked((ushort)(
                indices[pixelIndex] |
                indices[pixelIndex + 1] << 4 |
                indices[pixelIndex + 2] << 8 |
                indices[pixelIndex + 3] << 12));
            BinaryPrimitives.WriteUInt16LittleEndian(
                record.AsSpan(output, 2), pixel);
            output += 2;
        }
        SelectorPortraitRecords[path] = record;
        return record;
    }

    /// <summary>
    /// Produces the exact runtime record so the generated host can pass it
    /// through the recompiled TIM parser and image loader without opening a
    /// window. This is intentionally the same cached builder used in-game.
    /// </summary>
    public static byte[] BuildGeneratedSelectorPortraitRecordForProbe(
        string path) => BuildGeneratedSelectorPortraitRecord(path);

    static byte[] ResizeSelectorPortrait(ReadOnlySpan<byte> source)
    {
        byte[] resized = new byte[
            SelectorPortraitWidth * SelectorPortraitHeight * 3];
        for (int y = 0; y < SelectorPortraitHeight; y++)
        {
            int sourceY = SelectorPortraitHeight == 1
                ? 0
                : checked((int)(
                    (long)y * (SelectorPortraitSourceHeight - 1) * 65536 /
                    (SelectorPortraitHeight - 1)));
            int y0 = sourceY >> 16;
            int y1 = Math.Min(y0 + 1, SelectorPortraitSourceHeight - 1);
            int fy = sourceY & 0xFFFF;
            for (int x = 0; x < SelectorPortraitWidth; x++)
            {
                int sourceX = SelectorPortraitWidth == 1
                    ? 0
                    : checked((int)(
                        (long)x * (SelectorPortraitSourceWidth - 1) * 65536 /
                        (SelectorPortraitWidth - 1)));
                int x0 = sourceX >> 16;
                int x1 = Math.Min(x0 + 1, SelectorPortraitSourceWidth - 1);
                int fx = sourceX & 0xFFFF;
                int destination =
                    (y * SelectorPortraitWidth + x) * 3;
                for (int channel = 0; channel < 3; channel++)
                {
                    int topLeft =
                        source[(y0 * SelectorPortraitSourceWidth + x0) * 3 +
                            channel];
                    int topRight =
                        source[(y0 * SelectorPortraitSourceWidth + x1) * 3 +
                            channel];
                    int bottomLeft =
                        source[(y1 * SelectorPortraitSourceWidth + x0) * 3 +
                            channel];
                    int bottomRight =
                        source[(y1 * SelectorPortraitSourceWidth + x1) * 3 +
                            channel];
                    int top = topLeft +
                        ((topRight - topLeft) * fx + 32768 >> 16);
                    int bottom = bottomLeft +
                        ((bottomRight - bottomLeft) * fx + 32768 >> 16);
                    resized[destination + channel] = checked((byte)(
                        top + ((bottom - top) * fy + 32768 >> 16)));
                }
            }
        }
        return resized;
    }

    static (ushort[] Palette, byte[] Indices) QuantizeSelectorPortrait(
        ReadOnlySpan<byte> pixels)
    {
        var frequencies = new Dictionary<ushort, int>();
        ushort[] colors = new ushort[pixels.Length / 3];
        for (int source = 0, pixel = 0; source < pixels.Length;
             source += 3, pixel++)
        {
            ushort color = checked((ushort)(
                pixels[source] >> 3 |
                (pixels[source + 1] >> 3) << 5 |
                (pixels[source + 2] >> 3) << 10));
            colors[pixel] = color;
            frequencies[color] =
                frequencies.TryGetValue(color, out int count)
                    ? count + 1
                    : 1;
        }

        var boxes = new List<List<(ushort Color, int Count)>>
        {
            frequencies
                .OrderBy(pair => pair.Key)
                .Select(pair => (pair.Key, pair.Value))
                .ToList(),
        };
        while (boxes.Count < SelectorPortraitPaletteEntries)
        {
            int splitIndex = -1;
            long splitScore = long.MinValue;
            int splitChannel = 0;
            for (int index = 0; index < boxes.Count; index++)
            {
                List<(ushort Color, int Count)> box = boxes[index];
                if (box.Count < 2) continue;
                int minR = 31, minG = 31, minB = 31;
                int maxR = 0, maxG = 0, maxB = 0;
                int population = 0;
                foreach (var item in box)
                {
                    int red = item.Color & 31;
                    int green = item.Color >> 5 & 31;
                    int blue = item.Color >> 10 & 31;
                    minR = Math.Min(minR, red);
                    minG = Math.Min(minG, green);
                    minB = Math.Min(minB, blue);
                    maxR = Math.Max(maxR, red);
                    maxG = Math.Max(maxG, green);
                    maxB = Math.Max(maxB, blue);
                    population += item.Count;
                }
                int rangeR = maxR - minR;
                int rangeG = maxG - minG;
                int rangeB = maxB - minB;
                int channel = rangeG >= rangeR && rangeG >= rangeB
                    ? 1
                    : rangeB >= rangeR ? 2 : 0;
                int range = channel == 0 ? rangeR :
                    channel == 1 ? rangeG : rangeB;
                long score = (long)(range + 1) * (range + 1) * population;
                if (score <= splitScore) continue;
                splitScore = score;
                splitIndex = index;
                splitChannel = channel;
            }
            if (splitIndex < 0) break;

            List<(ushort Color, int Count)> selected = boxes[splitIndex];
            selected.Sort((left, right) =>
            {
                int leftValue = splitChannel == 0
                    ? left.Color & 31
                    : splitChannel == 1
                        ? left.Color >> 5 & 31
                        : left.Color >> 10 & 31;
                int rightValue = splitChannel == 0
                    ? right.Color & 31
                    : splitChannel == 1
                        ? right.Color >> 5 & 31
                        : right.Color >> 10 & 31;
                int comparison = leftValue.CompareTo(rightValue);
                return comparison != 0
                    ? comparison
                    : left.Color.CompareTo(right.Color);
            });
            int total = selected.Sum(item => item.Count);
            int running = 0;
            int cut = 1;
            for (; cut < selected.Count; cut++)
            {
                running += selected[cut - 1].Count;
                if (running * 2 >= total) break;
            }
            boxes[splitIndex] = selected.GetRange(0, cut);
            boxes.Add(selected.GetRange(cut, selected.Count - cut));
        }

        ushort[] palette = new ushort[SelectorPortraitPaletteEntries];
        for (int index = 0; index < boxes.Count; index++)
        {
            long red = 0, green = 0, blue = 0, total = 0;
            foreach (var item in boxes[index])
            {
                red += (item.Color & 31) * (long)item.Count;
                green += (item.Color >> 5 & 31) * (long)item.Count;
                blue += (item.Color >> 10 & 31) * (long)item.Count;
                total += item.Count;
            }
            int averageRed = checked((int)((red + total / 2) / total));
            int averageGreen = checked((int)((green + total / 2) / total));
            int averageBlue = checked((int)((blue + total / 2) / total));
            palette[index] = checked((ushort)(
                averageRed | averageGreen << 5 | averageBlue << 10));
        }
        for (int index = boxes.Count;
             index < SelectorPortraitPaletteEntries;
             index++)
            palette[index] = palette[Math.Max(0, boxes.Count - 1)];

        byte[] indices = new byte[colors.Length];
        for (int pixel = 0; pixel < colors.Length; pixel++)
        {
            int red = colors[pixel] & 31;
            int green = colors[pixel] >> 5 & 31;
            int blue = colors[pixel] >> 10 & 31;
            int best = 0;
            int bestDistance = int.MaxValue;
            for (int index = 0; index < palette.Length; index++)
            {
                int deltaRed = red - (palette[index] & 31);
                int deltaGreen = green - (palette[index] >> 5 & 31);
                int deltaBlue = blue - (palette[index] >> 10 & 31);
                int distance = deltaRed * deltaRed * 3 +
                    deltaGreen * deltaGreen * 6 +
                    deltaBlue * deltaBlue;
                if (distance >= bestDistance) continue;
                bestDistance = distance;
                best = index;
            }
            indices[pixel] = checked((byte)best);
        }
        return (palette, indices);
    }

    static string ReadPpmToken(byte[] data, ref int cursor)
    {
        while (cursor < data.Length)
        {
            while (cursor < data.Length &&
                   char.IsWhiteSpace((char)data[cursor]))
                cursor++;
            if (cursor >= data.Length || data[cursor] != '#')
                break;
            while (cursor < data.Length && data[cursor] != '\n')
                cursor++;
        }
        int start = cursor;
        while (cursor < data.Length &&
               !char.IsWhiteSpace((char)data[cursor]))
            cursor++;
        if (start == cursor)
            throw new InvalidDataException(
                "native selector portrait PPM is truncated");
        return Encoding.ASCII.GetString(data, start, cursor - start);
    }

#endif

    static uint BuildNativeBank(
        CpuContext c, IMemory m, NativeVehicleBankSource bank)
    {
        byte[] binSource = bank.ReadBin();
        byte[]? animationSource = bank.ReadAnimation();
        uint bin = AllocateBytes(c, m, binSource);
        uint animation = animationSource == null
            ? 0u
            : AllocateBytes(c, m, animationSource);
        Console.Error.WriteLine(
            $"[V82Bank] build bin=0x{bin:X8}+0x{binSource.Length:X} " +
            $"anm=0x{animation:X8}+0x{animationSource?.Length ?? 0:X}");
        c.A0 = bin;
        c.A1 = animation;
        c.RA = CustomDispatchAddress;
        try
        {
            Dispatcher.Call(c, m, BuildNativeBankAddress);
        }
        catch (Exception error)
        {
            Console.Error.WriteLine(
                $"[V82Bank] build failed bin=0x{bin:X8}-" +
                $"0x{bin + checked((uint)binSource.Length):X8} " +
                $"packet=0x{c.A0:X8} group=0x{c.A2:X8} " +
                $"source=0x{c.S1:X8} packet-index={c.T1} " +
                $"group-index={c.A3}: {error.Message}");
            throw;
        }
        if (c.V0 == 0u)
            throw new OutOfMemoryException("native V8:2 object-bank build failed");
        Console.Error.WriteLine(
            $"[V82Bank] built runtime=0x{c.V0:X8}");
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
