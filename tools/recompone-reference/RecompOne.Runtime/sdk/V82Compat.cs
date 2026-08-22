using RecompOne.Runtime.Context;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Hle;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Host.Window;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class V82Compat
{
    // Generated sources made during the rejected close-culling experiment may
    // still contain this instrumentation hook until the next full reference
    // regeneration. Keep it behavior-free; the experiment itself is gone.
    public static void RegisterTrianglePacketNclip(CpuContext c, IMemory m)
    {
    }

    static readonly bool TraceNativeOptions =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_NATIVE_OPTIONS") == "1";
    static readonly bool TraceImportedOverlayAbi =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_IMPORTED_OVERLAY_ABI") == "1";
    static int _nativeModelLifecycleTraceCount;
    static readonly HashSet<string> SeenNativeOptionText = [];
    static bool _nativeOptionsActive;
    public static int? GetFirstPressedNativeControlPadButton(int player) =>
        InputManager.GetFirstPressedPadButton(player);

    public static string? GetFirstPressedNativeControlKey()
    {
        foreach (Silk.NET.Input.Key key in Enum.GetValues<Silk.NET.Input.Key>())
        {
            if (key is Silk.NET.Input.Key.Unknown or Silk.NET.Input.Key.Menu)
                continue;
            if (InputManager.IsKeyDown(key))
                return key.ToString();
        }
        return null;
    }

    public static void SignalNativeControlsPage()
    {
        InputManager.SignalScriptStage(
            "v82_options_native_controls", captureDelayPolls: 12);
    }

    public static void SignalNativeVideoPage()
    {
        InputManager.SignalScriptStage(
            "v82_options_native_video", captureDelayPolls: 12);
    }

    public static void SetFullscreen(bool on) => HostWindow.SetFullscreen(on);

    public static void SetOutputResolution(string resolution) =>
        HostWindow.SetOutputResolution(resolution);

    public static void ApplyGraphicsConfiguration() =>
        HostWindow.ApplyGraphicsConfiguration();

    public static void TraceNativeOptionsText(CpuContext c, IMemory m)
    {
        string text = ReadNativeAscii(m, c.A1, 64);
        if (text.Length == 0)
            return;

        if (GpuHle.GameplayActive &&
            text is "PAUSED" or "QUEST OBJECTIVES" or "ARE YOU SURE?")
        {
            InputManager.SignalNativeGameplayMenu();
            GpuHle.SignalNativeModal();
        }

        if (TraceNativeOptions && SeenNativeOptionText.Add(text))
        {
            Span<byte> layout = stackalloc byte[16];
            for (int index = 0; index < layout.Length; index++)
                layout[index] = m.ReadU8(c.A2 + (uint)index);
            Console.Error.WriteLine(
                $"[V82NativeOptions] text='{text}' caller=0x{c.RA:X8} " +
                $"object=0x{c.A0:X8} layout=0x{c.A2:X8} flags=0x{c.A3:X8} " +
                $"layout16={Convert.ToHexString(layout)}");
        }

        string? stage = null;
        if (text == "OPTIONS")
        {
            _nativeOptionsActive = true;
            stage = "v82_options";
        }
        else if (_nativeOptionsActive)
        {
            stage = text switch
            {
                "GAME STATUS" => "v82_options_game_status",
                "MEMORY CARD" => "v82_options_memory_card",
                "DIFFICULTY" => "v82_options_difficulty",
                "CONTROLLER" => "v82_options_controller",
                "AUDIO" => "v82_options_audio",
                "BACK STORY" => "v82_options_back_story",
                "CREDITS" => "v82_options_credits",
                _ => null,
            };
        }

        if (stage != null)
            InputManager.SignalScriptStage(stage, captureDelayPolls: 12);
    }

    static string ReadNativeAscii(IMemory m, uint address, int maxLength)
    {
        if (address < 0x80010000u || address >= 0x80200000u)
            return string.Empty;
        Span<char> chars = stackalloc char[maxLength];
        int length = 0;
        while (length < chars.Length)
        {
            byte value = m.ReadU8(address + (uint)length);
            if (value == 0)
                break;
            chars[length++] = value is >= 0x20 and <= 0x7E
                ? (char)value
                : '?';
        }
        return new string(chars[..length]);
    }

    readonly record struct GuestVramReservation(
        NativeVramAllocation Request,
        uint X,
        uint Y,
        uint Descriptor);

    const uint HeapHeadAddress = 0x8006B3F8u;
    const uint ShellLinkedBase = 0x80100000u;
    const uint ShellSectorAllocation = 0x0001B000u;
    // Maximum LOD can exceed either retail 128 KiB GPU packet arena, most
    // visibly in Air Grave's arena callback. Reserve two host-only 512 KiB
    // arenas below the loose-file heap so high-detail packets cannot overwrite
    // gameplay allocations before the scheduler validates the cursor.
    //
    // Do not enlarge these without re-verifying gameplay frame by frame.
    // Growing them to 1 MiB moves PcHeapBase from 0x80300000 to 0x80400000 and
    // puts the second arena where the heap used to start; the visible result
    // was terrain dropping out on alternating frames once the car left spawn,
    // which single-frame captures do not show.
    const uint ExpandedPrimitiveBufferBase = 0x80200000u;
    const uint ExpandedPrimitiveBufferSize = 0x00080000u;
    const uint PcHeapBase =
        ExpandedPrimitiveBufferBase + ExpandedPrimitiveBufferSize * 2u;
    const uint PcHeapEnd = 0x80800000u;
    const ushort MaximumAttachedWeaponAmmo = 99;
    static bool _shellPinnedAtLinkedBase;
    static bool _extendedHeapInstalled;
    static readonly List<(uint Start, uint Size)> PcFreeBlocks = new();
    static readonly Dictionary<uint, (uint Header, uint Size)> PcAllocations = new();
    static bool _spuMallocRetrying;
    static readonly Stack<uint> SpuMallocRequests = new();
    static readonly Stack<uint[]> ShellDecodeCallers = new();
    static readonly Stack<(uint FrameSp, uint SourceRect)> ShellImageDecodeFrames = new();
    static readonly Stack<(uint Width, uint Height, uint AlignWidth, uint AlignHeight, uint LimitWidth, uint LimitHeight)> VramRequests = new();
    static readonly Stack<bool> SelectorOwnedVramRequests = new();
    static readonly List<GuestVramReservation> GuestVramReservations = [];
    static readonly List<GuestVramReservation> SelectorVramReservations = [];
    static readonly HashSet<int> ClaimedGuestVramReservations = [];
    static readonly HashSet<uint> SyntheticVramDescriptors = [];
    static List<GuestVramReservation>? _activeGuestVramReservations;
    static bool _guestVramClaimReusable;
    static bool _guestVramClaimActive;
    static int _guestVramClaimIndex;
    static int _guestVramClaimMisses;
    static int _relocatedTextureDecodeCount;
    static bool _matchVramActive;
    static int _matchVramSuccesses;
    static int _matchVramFailures;
    static readonly bool _traceVram =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VRAM") == "1";
    static readonly bool _traceSelector =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_V82_SELECTOR") == "1";
    static int _selectorDepth;
    static int _selectorTraceCount;
    static readonly HashSet<string> SelectorTraceLines = [];
    static int _objectFactoryTraceCount;
    static readonly Stack<uint> ObjectFactorySources = new();
    static readonly HashSet<uint> VehicleFactorySources = [];
    static readonly HashSet<uint> VehicleObjects = [];
    readonly record struct ObjectRenderScope(
        uint ObjectAddress,
        uint PacketStart,
        uint Caller,
        bool IsVehicle);
    readonly record struct ImportedRenderGroupScope(
        uint PacketStart,
        uint Descriptor,
        bool Resolved,
        V82VehicleRegistry.ImportedRenderGroupInfo Info);
    static readonly Stack<ObjectRenderScope> ObjectRenderScopes = [];
    static readonly bool TraceRendererOwnership =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_ENHANCED_RENDERER") == "1";
    static readonly uint TraceObjectAddress =
        uint.TryParse(
            Environment.GetEnvironmentVariable("RECOMPONE_TRACE_V82_OBJECT"),
            out uint traceObjectAddress)
            ? traceObjectAddress
            : 0u;
    static int _rendererOwnershipTraceCount;
    static int _traceObjectCount;
    static readonly HashSet<uint> TracedRenderObjects = [];
    static int _objectSchedulerPass;
    static int _gameplayFrameCount;
    static int _lastTracedOrderingTableFrame;
    static bool _matchGateRepairLogged;
    static bool _geometryContinuationActive;
    static bool _geometryContinuationDriving;
    static int _geometrySuppressedLeaves;
    static int _geometryContinuationIterations;
    static int _geometryClipCount;
    readonly record struct GeometryContinuationTrace(
        uint Target,
        uint S0,
        uint S2,
        uint S3,
        uint S5,
        uint A2,
        uint A3,
        byte PacketType);
    static readonly Queue<GeometryContinuationTrace> GeometryContinuationHistory = [];
    readonly record struct RenderGroupEntryTrace(
        uint Descriptor,
        uint Transform,
        uint A2,
        uint A3,
        uint Caller,
        uint Stack,
        uint PreviousS0,
        uint PreviousS1,
        uint PreviousS3,
        uint ObjectWord0,
        uint ObjectFlags,
        uint ObjectSibling,
        uint ObjectChild,
        uint ObjectIdTimer,
        uint ObjectModel,
        uint ObjectDatabase,
        uint ObjectDistanceModel,
        uint Word0,
        uint Word4,
        uint Word8,
        uint WordC,
        uint Word10,
        uint Word14,
        uint Word18);
    static readonly Queue<RenderGroupEntryTrace> RenderGroupEntryHistory = [];
    readonly record struct NativeModelResolveTrace(
        uint Database,
        uint Group,
        uint Caller,
        uint Descriptor,
        uint Word0,
        uint Word4,
        uint Word8,
        uint WordC,
        uint Word10,
        uint Word14,
        uint Word18);
    readonly record struct NativeModelReleaseTrace(
        uint Descriptor,
        uint Caller,
        bool HadActiveOwner,
        uint OwnerDatabase,
        uint OwnerGroup,
        uint OwnerCaller,
        uint Word0,
        uint Word4,
        uint Word8,
        uint WordC,
        uint Word10,
        uint Word14,
        uint Word18);
    static readonly Stack<(uint Database, uint Group, uint Caller)>
        NativeModelResolveScopes = [];
    static readonly Queue<NativeModelResolveTrace> NativeModelResolveHistory = [];
    static readonly Queue<NativeModelReleaseTrace> NativeModelReleaseHistory = [];
    static readonly Dictionary<uint, NativeModelResolveTrace> ActiveNativeModels = [];
    static readonly Dictionary<uint, NativeModelReleaseTrace> ReleasedNativeModels = [];
    readonly record struct NativeObjectModelAssignment(
        uint Object,
        uint Database,
        uint RecordIndex,
        uint Descriptor,
        uint Word8,
        uint WordC,
        uint Word10,
        uint Word18);
    static readonly Dictionary<uint, NativeObjectModelAssignment>
        AssignedNativeModels = [];
    static readonly Dictionary<uint, NativeObjectModelAssignment>
        AllAssignedNativeModels = [];
    static readonly Dictionary<uint, NativeObjectModelAssignment>
        AssignedNativeModelsByObject = [];
    static readonly HashSet<uint> ReportedNativeObjectModelMutations = [];
    readonly record struct NativeModelLifecycleScope(
        uint Object,
        uint Database,
        uint ModelIndex);
    static readonly Stack<NativeModelLifecycleScope> NativeModelLifecycleScopes = [];
    readonly record struct NativeObjectExtentTrace(
        uint Object,
        uint Caller,
        uint Callback,
        uint Flags,
        uint Sibling,
        uint Child,
        uint Parent,
        uint IdTimer,
        uint Model,
        uint Database);
    static readonly Queue<NativeObjectExtentTrace> NativeObjectExtentHistory = [];
    static uint _terrainFrustumWidthAddress;
    static uint _terrainFrustumNativeWidth;
    static bool _terrainFrustumAdjusted;
    static bool _terrainFrustumLogged;
    static readonly double TerrainFrustumScaleOverride =
        ReadTerrainFrustumScaleOverride();
    // Widening the widescreen terrain edge happens here rather than on the
    // traversal polygon. func_8001BE68 takes an inclusive/exclusive X-cell
    // span that it already clamps to the grid, so padding it cannot produce a
    // polygon the walker refuses to walk. Widening the polygon instead starved
    // func_8001BECC on roughly 6% of frames - it selected a single cell and
    // the ground vanished for that frame - while producing an identical edge
    // result. See tools/recompone-v8-2/analyze_terrain_flicker.py.
    //
    // 4 cells is where the outer-left hole count reaches zero; 2 leaves 536.
    const int DefaultTerrainRowCellPadding = 4;
    static readonly int TerrainRowCellPadding =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TERRAIN_ROW_CELL_PADDING"),
            out int terrainRowCellPadding)
            ? Math.Clamp(terrainRowCellPadding, 0, 16)
            : DefaultTerrainRowCellPadding;
    static bool _terrainRowCellPaddingLogged;
    static readonly int TerrainPolygonPaddingCells =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TERRAIN_POLYGON_PADDING_CELLS"),
            out int terrainPolygonPaddingCells)
            ? Math.Clamp(terrainPolygonPaddingCells, 0, 32)
            : 0;
    static bool _terrainPolygonPaddingLogged;
    static readonly int TerrainLateralPaddingCells =
        int.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TERRAIN_LATERAL_PADDING_CELLS"),
            out int terrainLateralPaddingCells)
            ? Math.Clamp(terrainLateralPaddingCells, 0, 64)
            : 0;
    static bool _terrainLateralPaddingLogged;
    static readonly bool TerrainOmnidirectional =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_OMNIDIRECTIONAL") == "1";
    static bool _terrainOmnidirectionalLogged;
    static readonly bool TerrainAspectPolygon =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_ASPECT_POLYGON") == "1";
    static bool _terrainAspectPolygonLogged;
    // Off by default. Extending the range never grounded the props it was
    // written for - 2.0, 3.0 and 4.0 all leave them floating by the same
    // amount - and it costs traversal and packet budget the arena cannot
    // absorb once the car leaves spawn. Kept only as an investigation lever.
    const double DefaultTerrainRangeScale = 1d;
    const double MaximumTerrainRangeScale = 2d;
    static readonly double TerrainRangeScaleOverride =
        double.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TERRAIN_RANGE_SCALE"),
            System.Globalization.NumberStyles.Float,
            System.Globalization.CultureInfo.InvariantCulture,
            out double terrainRangeScale) &&
        double.IsFinite(terrainRangeScale)
            // Deliberately clamped: higher values overflow the terrain packet
            // arena and corrupt the frame rather than degrading gracefully.
            ? Math.Clamp(terrainRangeScale, 1d, MaximumTerrainRangeScale)
            : 0d;
    static bool _terrainRangeLogged;
    // Off by default: it closes the widescreen edge exactly as well as the row
    // padding above, but it does so by rewriting the traversal polygon, and a
    // widened polygon intermittently starves the walker. Retained as an
    // investigation lever only.
    static readonly bool TerrainWideFit =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_WIDE_FIT") == "1";
    static readonly double TerrainWideFitMarginCells =
        double.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TERRAIN_WIDE_FIT_MARGIN_CELLS"),
            System.Globalization.NumberStyles.Float,
            System.Globalization.CultureInfo.InvariantCulture,
            out double terrainWideFitMarginCells) &&
        double.IsFinite(terrainWideFitMarginCells)
            ? Math.Clamp(terrainWideFitMarginCells, 0d, 8d)
            : 1d;
    // A widened traversal polygon still has to stay inside the terrain grid
    // and the packet budget; this bounds a pathological camera from demanding
    // an enormous one.
    const double MaximumTerrainWideFitFactor = 2.5d;
    // func_8001BE68 clamps its row spans to 0..2048 quarter-cell indices, so
    // the authored grid is 512 cells of 1024 world units on each axis.
    const double TerrainGridExtentCells = 512d;
    static bool _terrainWideFitLogged;
    static bool _objectVisibilityLogged;
    // func_8002E22C's three planes are built for the authored 4:3 view, so in
    // widescreen it rejects objects that are still on screen. Granting it the
    // lateral reach the widened frustum has at each object's own distance cuts
    // objects that stop drawing while still visible by 23% - 45 against 58.5
    // over paired deterministic runs, measured with
    // tools/recompone-v8-2/analyze_object_pops.py.
    //
    // It does not help the specific case of props vanishing at the extreme
    // left and right edges up close: those stay at 15 against 16. That remains
    // open and is pre-existing.
    static readonly bool WideObjectCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_WIDE_OBJECT_CULL") != "0";
    static readonly bool TraceObjectCull =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_OBJECT_CULL") == "1";
    const int MaximumFalseCullLogs = 400;
    const int MaximumFalseCullCaptures = 6;
    static int _falseCullsLogged;
    static uint _objectCullPosition;
    static uint _objectCullRadius;
    static readonly Dictionary<uint, (int Tick, bool Accepted, double Distance)>
        ObjectCullHistory = [];
    // Swept against the pop detector on repeated deterministic runs. Objects
    // that stop drawing while still on screen: 54 with no widening, 34 at
    // 1.05, 28 at 1.6, and back up to 46 at 2.5 - past a point the extra reach
    // only defers the transition rather than removing it.
    // Measured at the gate itself rather than through a downstream proxy:
    // of every object func_8002D9E0 renders, the share that emits anything is
    // 32.2% at stock, 51.4% at 1.6 and 61.5% at 2.5. At 4.0 the test rejects
    // nothing at all, which is no longer a frustum test and would draw objects
    // behind the camera, so 2.5 is the most reach that still culls.
    const double DefaultObjectCullSlack = 2.5d;
    static readonly double ObjectCullSlackScale =
        double.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_OBJECT_CULL_SLACK"),
            System.Globalization.NumberStyles.Float,
            System.Globalization.CultureInfo.InvariantCulture,
            out double objectCullSlack) &&
        double.IsFinite(objectCullSlack)
            ? Math.Clamp(objectCullSlack, 1d, 4d)
            : DefaultObjectCullSlack;
    static int _objectsTested;
    static int _objectsCulled;

    public static (int Tested, int Culled) ConsumeObjectCullCounts()
    {
        var counts = (_objectsTested, _objectsCulled);
        _objectsTested = 0;
        _objectsCulled = 0;
        return counts;
    }

    /// <summary>
    /// Records whether func_8002E22C accepted the object, and reports any
    /// object it rejected while that object was still overlapping the visible
    /// screen.
    ///
    /// This exists so the defect finds itself. Objects vanishing at the outer
    /// edges cannot be chased with the autodrive - it diverges after roughly
    /// 180 frames, so whichever object happens to be near an edge differs
    /// every run and every aggregate metric drowns in that. Instead the engine
    /// checks its own decision each time: project the object's bounding sphere
    /// with the real camera basis, and if a rejected object still covers part
    /// of the widened viewport, say so with everything needed to go back to
    /// that exact spot.
    ///
    /// Enable with RECOMPONE_V82_TRACE_OBJECT_CULL=1.
    /// </summary>
    public static void RecordObjectVisibility(CpuContext c, IMemory m)
    {
        if (!GpuHle.GameplayActive)
            return;
        _objectsTested++;
        bool culled = c.V0 == 0u;
        _lastVisibilityCulled = culled;
        if (culled)
            _objectsCulled++;
        if (!TraceObjectCull)
            return;

        uint position = _objectCullPosition;
        if (position < 0x80000000u || position > 0x807FFFF0u)
            return;

        m = Dispatcher.UnwrapMemory(m);
        double projection = unchecked((int)m.ReadU32(c.GP + 0xED8u));
        double nativeWidth = _terrainFrustumNativeWidth != 0u
            ? _terrainFrustumNativeWidth
            : unchecked((int)m.ReadU32(c.GP + 0xEDCu));
        double nativeHeight = unchecked((int)m.ReadU32(c.GP + 0xF20u));
        if (projection < 1d || nativeWidth < 1d || nativeHeight < 1d)
            return;

        double camX = unchecked((int)m.ReadU32(c.GP + 0xF3Cu)) / 256d;
        double camY = unchecked((int)m.ReadU32(c.GP + 0xF40u)) / 256d;
        double camZ = unchecked((int)m.ReadU32(c.GP + 0xF44u)) / 256d;
        double objX = unchecked((int)m.ReadU32(position)) / 256d;
        double objY = unchecked((int)m.ReadU32(position + 4u)) / 256d;
        double objZ = unchecked((int)m.ReadU32(position + 8u)) / 256d;
        double dx = objX - camX, dy = objY - camY, dz = objZ - camZ;
        double distance = Math.Sqrt(dx * dx + dy * dy + dz * dz);

        uint matrix = c.GP + 0xF28u;
        double R(int index) =>
            unchecked((short)m.ReadU16(matrix + (uint)(index * 2))) / 4096d;
        double viewX = R(0) * dx + R(1) * dy + R(2) * dz;
        double viewY = R(3) * dx + R(4) * dy + R(5) * dz;
        double viewZ = R(6) * dx + R(7) * dy + R(8) * dz;

        double screenX = viewZ >= 1d ? viewX * projection / viewZ : double.NaN;
        double screenY = viewZ >= 1d ? viewY * projection / viewZ : double.NaN;
        double halfWidth =
            nativeWidth * 0.5d +
            GpuHle.WideMargin((int)Math.Round(nativeWidth));
        double halfHeight = nativeHeight * 0.5d;

        int tick = GpuHle.DebugGameplayTick;
        ObjectCullHistory.TryGetValue(position, out var previous);
        ObjectCullHistory[position] = (tick, !culled, distance);

        // The defect is an object that was being drawn and stops while it is
        // still comfortably on screen and has not meaningfully moved away.
        // Ordinary distance culling shows up as a rejection at a much larger
        // distance, and objects leaving the view show up outside the frame.
        if (!culled ||
            !previous.Accepted ||
            previous.Tick != tick - 1 ||
            _falseCullsLogged >= MaximumFalseCullLogs ||
            double.IsNaN(screenX) ||
            distance > previous.Distance * 1.05d ||
            Math.Abs(screenX) > halfWidth * 0.94d ||
            Math.Abs(screenY) > halfHeight * 0.94d)
            return;

        // Which of the three planes rejected it, and by how much.
        int plane0 = unchecked((short)Gte.Read(9));
        int plane1 = unchecked((short)Gte.Read(10));
        int plane2 = unchecked((short)Gte.Read(11));
        int compared = (int)(_objectCullRadius >> 8);

        _falseCullsLogged++;
        Console.Error.WriteLine(
            $"[V82ObjectPop] tick={tick} object=0x{position:X8} " +
            $"planes=({plane0},{plane1},{plane2}) vs={compared} " +
            $"world=({objX:F0},{objY:F0},{objZ:F0}) " +
            $"camera=({camX:F0},{camY:F0},{camZ:F0}) " +
            $"distance={distance:F0} previous={previous.Distance:F0} " +
            $"screen=({screenX:F1},{screenY:F1}) " +
            $"halfWidth={halfWidth:F0} halfHeight={halfHeight:F0} " +
            $"radius={_objectCullRadius / 256d:F0}");
        if (_falseCullsLogged <= MaximumFalseCullCaptures)
            HostWindow.RequestDisplayCapture(
                $"object_pop_{_falseCullsLogged:00}");
    }

    /// <summary>
    /// Captures the arguments func_8002E22C was called with, so the post-hook
    /// can reason about the object it just rejected.
    /// </summary>
    public static void RecordObjectVisibilityInputs(CpuContext c, IMemory m)
    {
        RecordCameraPose(c, m);
        _objectCullPosition = c.A0;
        _objectCullRadius = c.A1;
        _lastVisibilityCulled = false;
        _visibilityTestRan = true;
    }

    // Why func_8002D9E0 produced nothing for an object. It has exactly three
    // ways out before it draws: a flag on the object, the frustum test, and a
    // distance limit. Counting which one fires says where a wall segment is
    // being dropped instead of testing candidate fixes one at a time.
    static bool _lastVisibilityCulled;
    // Which of func_8002D9E0's three exits fired, for the census.
    static string? _lastEmptyReason;
    static bool _visibilityTestRan;
    static uint _objectRenderPacketStart;
    static uint _objectRenderAddress;
    static int _renderedObjects, _emptyByFlag, _emptyByFrustum,
        _emptyByDistance, _emptyUnexplained, _emittedObjects;

    public static void TraceObjectRenderBegin(CpuContext c, IMemory m)
    {
        if ((!TraceObjectCull && !_censusOpen) || !GpuHle.GameplayActive)
            return;
        _objectRenderAddress = c.A0;
        _objectRenderPacketStart =
            Dispatcher.UnwrapMemory(m).ReadU32(c.GP + 0x610u);
        _visibilityTestRan = false;
        _lastVisibilityCulled = false;
        _lastEmptyReason = null;
        _renderedObjects++;
    }

    public static void TraceObjectRenderEnd(CpuContext c, IMemory m)
    {
        if (!GpuHle.GameplayActive || _objectRenderAddress == 0u)
            return;
        m = Dispatcher.UnwrapMemory(m);
        if (m.ReadU32(c.GP + 0x610u) != _objectRenderPacketStart)
        {
            _emittedObjects++;
            RecordCensus(m, _objectRenderAddress, "drawn");
            return;
        }
        uint flags = m.ReadU32(_objectRenderAddress + 4u);
        string outcome;
        if ((flags & 2u) != 0u) { _emptyByFlag++; outcome = "flag"; }
        else if (!_visibilityTestRan)
        { _emptyUnexplained++; outcome = "other"; }
        else if (_lastVisibilityCulled)
        { _emptyByFrustum++; outcome = "frustum"; }
        else { _emptyByDistance++; outcome = "distance"; }
        _lastEmptyReason = outcome;
        RecordCensus(m, _objectRenderAddress, outcome);
    }

    // Every object the engine considered this frame and what became of it.
    // The drawn-triangle dump can only show what survived; when a whole model
    // is missing the question is whether it was considered and rejected - and
    // by which gate - or never offered at all. Only a census answers that.
    public readonly record struct ObjectRecord(
        uint Address, int X, int Y, int Z, int Radius, string Outcome,
        long Frame);

    /// <summary>
    /// Frame counter stamped onto census records. Clearing the census per
    /// frame proved unreliable because the reset point sits elsewhere in the
    /// loop than object submission; stamping is order-independent.
    /// </summary>
    public static long CensusFrame;

    // Not cleared per frame: the per-frame clear runs at a different point in
    // the loop than object submission, so it emptied the list before any
    // capture could read it. A running map keyed by object address answers the
    // question that matters - was this object ever offered to the renderer -
    // and costs one entry per distinct object.
    static readonly Dictionary<uint, ObjectRecord> _objectCensus = [];
    static bool _censusOpen = true;
    static readonly bool CensusOwnership =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_CENSUS_OWNERSHIP") == "1";
    public static int CensusHookCalls;
    public static long ObjectRenderEntries;
    public static long ObjectRenderExits;
    public static int CensusRejected;

    // Kept running continuously rather than armed around a capture: the
    // arming point sits at a different place in the frame than object
    // submission, so an armed window recorded nothing. ~90 objects a frame is
    // cheap enough to just always collect and clear.
    // Camera pose, captured every frame. With it a capture is self-contained:
    // any object's world position can be projected to screen offline, so a
    // question about geometry that is missing does not need another play test
    // to answer.
    public static int CamX, CamY, CamZ;
    public static readonly short[] CamMatrix = new short[9];

    // func_8001D414 links a packet into the ordering table, but only while a
    // pool counter is under 64. Past that the packet is dropped silently -
    // a polygon-level loss with no flag, no cull and no trace. Count how often
    // that happens and how deep the offered geometry was.
    public static long PoolLinked, PoolDropped;
    public static long EmitterNearTotal, EmitterNearDropped;

    public readonly record struct RejectedTriangle(
        uint Emitter, int X0, int Y0, int Z0, int X1, int Y1, int Z1,
        int X2, int Y2, int Z2, uint Flags, int Mac0);

    public static readonly List<RejectedTriangle> RejectedTriangles = [];
    public static int PoolHighWater;

    public static void RecordPoolLink(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint bufferBase = 0x800C0000u - 0x2A90u;
        if (m.ReadU32(c.GP + 0x20u) != 0u)
            bufferBase += 0x404u;
        int count = unchecked((int)m.ReadU32(bufferBase));
        if (count > PoolHighWater)
            PoolHighWater = count;
        if (count < 64)
            PoolLinked++;
        else
            PoolDropped++;
    }

    // TO-DO #1 (no reverse). The measurement that matters is not pixels or
    // camera drift but the vehicle's own fields, per include/structs.h:
    //   +0x20 i16 inputLong  -- longitudinal-input scalar; negative == reverse
    //                           requested. This isolates input decode from
    //                           physics: if it never goes negative, the pad
    //                           path never asks for reverse at all.
    //   +0x80 i32 velX/Y/Z   -- world velocity
    //   +0x10 3x3 i16        -- rotation; third column is the local Z axis, so
    //                           dot(vel, thatAxis) is signed forward speed.
    public static readonly bool ReverseTrace =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_REVERSE_TRACE") == "1";
    const uint PadBuffer1 = 0x800B5298u;
    static int _reverseTraceFrame;

    public static void TraceReverse(IMemory m)
    {
        if (!ReverseTrace || !GpuHle.GameplayActive) return;
        _reverseTraceFrame++;
        if (_reverseTraceFrame % 5 != 0) return;
        m = Dispatcher.UnwrapMemory(m);
        ushort buttons = (ushort)(m.ReadU8(PadBuffer1 + 2) |
                                  (m.ReadU8(PadBuffer1 + 3) << 8));
        uint p = _playerVehicle;
        string state = " player=none";
        if (p >= PcHeapBase && p < PcHeapEnd - 0x200u)
        {
            short inputLong = unchecked((short)m.ReadU16(p + 0x20u));
            short inputLat = unchecked((short)m.ReadU16(p + 0x14u));
            int vx = unchecked((int)m.ReadU32(p + 0x80u));
            int vy = unchecked((int)m.ReadU32(p + 0x84u));
            int vz = unchecked((int)m.ReadU32(p + 0x88u));
            int speed = unchecked((int)m.ReadU32(p + 0x8Cu));
            short fx = unchecked((short)m.ReadU16(p + 0x10u + 4));
            short fy = unchecked((short)m.ReadU16(p + 0x10u + 10));
            short fz = unchecked((short)m.ReadU16(p + 0x10u + 16));
            long fwd = ((long)vx * fx + (long)vy * fy + (long)vz * fz) / 4096;
            state =
                $" inputLong={inputLong} inputLat={inputLat} " +
                $"speed={speed} fwd={fwd}";
        }
        Console.Error.WriteLine(
            $"[Reverse] f={_reverseTraceFrame} btn=0x{buttons:X4} " +
            $"id=0x{m.ReadU8(PadBuffer1 + 1):X2} mode={m.ReadU16(0x800B4A68u)} " +
            $"ry={m.ReadU8(PadBuffer1 + 5)} lx={m.ReadU8(PadBuffer1 + 6)} " +
            $"ly={m.ReadU8(PadBuffer1 + 7)}{state}");
    }

    public static void RecordCameraPose(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        CamX = unchecked((int)m.ReadU32(c.GP + 0xF3Cu));
        CamY = unchecked((int)m.ReadU32(c.GP + 0xF40u));
        CamZ = unchecked((int)m.ReadU32(c.GP + 0xF44u));
        for (int i = 0; i < 9; i++)
            CamMatrix[i] =
                unchecked((short)m.ReadU16(c.GP + 0xF28u + (uint)(i * 2)));
    }

    public static void BeginObjectCensus() => _censusOpen = true;

    public static IReadOnlyList<ObjectRecord> EndObjectCensus() =>
        _objectCensus.Values.ToList();

    /// <summary>Drops the frame just recorded, after any capture read it.</summary>
    public static void ResetObjectCensus()
    {
    }

    static void RecordCensus(IMemory m, uint address, string outcome)
    {
        // Addresses reach here both KSEG-mapped and physical, so judge the
        // offset rather than the window.
        CensusHookCalls++;
        if (!_censusOpen || _objectCensus.Count >= 8192 ||
            address == 0u || (address & 0x1FFFFFFFu) > 0x00FFFFF0u)
        {
            CensusRejected++;
            return;
        }
        _objectCensus[address] = new ObjectRecord(
            address,
            unchecked((int)m.ReadU32(address)),
            unchecked((int)m.ReadU32(address + 4u)),
            unchecked((int)m.ReadU32(address + 8u)),
            (int)_objectCullRadius,
            outcome,
            CensusFrame);
    }

    public static void ReportObjectRenderGates()
    {
        if (!TraceObjectCull || _renderedObjects == 0)
            return;
        Console.Error.WriteLine(
            $"[V82ObjectGates] rendered={_renderedObjects} " +
            $"emitted={_emittedObjects} " +
            $"emptyFlag={_emptyByFlag} emptyFrustum={_emptyByFrustum} " +
            $"emptyDistance={_emptyByDistance} " +
            $"emptyOther={_emptyUnexplained}");
        _renderedObjects = _emittedObjects = _emptyByFlag = _emptyByFrustum =
            _emptyByDistance = _emptyUnexplained = 0;
    }
    static readonly bool TraceTerrainTraversal =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_TERRAIN_TRAVERSAL") == "1";
    static int _terrainTraversalTraceCount;
    static readonly bool TraceTerrainCells =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_TERRAIN_CELLS") == "1";
    readonly record struct TerrainCellScope(
        int Frame,
        string Source,
        uint X,
        uint Z,
        uint PacketStart,
        GpuHle.TerrainCellTextures Textures);
    readonly record struct TerrainTransitionScope(
        uint PacketStart,
        uint VertexRecords,
        TerrainCellScope Terrain);
    sealed class TerrainCellFrameStats
    {
        public int Frame;
        public int Calls;
        public int Emitted;
        public long PacketBytes;
        public uint MinX = uint.MaxValue;
        public uint MaxX;
        public uint MinZ = uint.MaxValue;
        public uint MaxZ;
        public uint EmittedMinX = uint.MaxValue;
        public uint EmittedMaxX;
        public uint EmittedMinZ = uint.MaxValue;
        public uint EmittedMaxZ;
        public readonly List<string> Rejected = [];
    }
    // Cheap always-on counters so a per-frame terrain report can separate
    // "the walker selected nothing" from "the emit stage rejected everything".
    static int _terrainCellsSubmitted;
    static int _terrainCellsEmitted;
    static uint _terrainCellPacketCursor;

    public static (int Submitted, int Emitted) ConsumeTerrainCellCounts()
    {
        var counts = (_terrainCellsSubmitted, _terrainCellsEmitted);
        _terrainCellsSubmitted = 0;
        _terrainCellsEmitted = 0;
        return counts;
    }

    static readonly Stack<TerrainCellScope> TerrainCellScopes = [];
    static readonly Stack<TerrainTransitionScope> TerrainTransitionScopes = [];
    static TerrainCellFrameStats? _terrainCellFrame;
    static int _terrainCellFramesLogged;
    static int _geometryTextureTraceCount;
    static readonly bool TraceGeometryTextures =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TRACE_GEOMETRY_TEXTURES") == "1";
    static uint _textureDecodeScratchTop;
    static uint _textureDecodeScratchBase;
    static uint _geometryNextTarget;
    static uint _playerVehicle;
    static readonly int _testDefeatFrame =
        int.TryParse(Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TEST_DEFEAT_FRAME"),
            out int testDefeatFrame)
            ? Math.Max(1, testDefeatFrame)
            : 0;
    static bool _testDefeatInjected;
    // Generic, opt-in water lifecycle fixture.  Coordinates are supplied by
    // the test runner so this contains no arena identity or map-specific
    // behavior.  It teleports the native player object once, then records its
    // unmodified engine lifecycle in enough detail to compare any converted
    // arena directly with a stock V8:2 water arena.
    static readonly int _testWaterFrame =
        ReadOptionalInt("RECOMPONE_V82_TEST_WATER_FRAME") ?? 0;
    static readonly int? _testWaterX =
        ReadOptionalInt("RECOMPONE_V82_TEST_WATER_X");
    static readonly int? _testWaterY =
        ReadOptionalInt("RECOMPONE_V82_TEST_WATER_Y");
    static readonly int? _testWaterZ =
        ReadOptionalInt("RECOMPONE_V82_TEST_WATER_Z");
    static readonly int _testWaterTimeout = Math.Max(
        60, ReadOptionalInt("RECOMPONE_V82_TEST_WATER_TIMEOUT") ?? 900);
    static bool _testWaterInjected;
    static bool _testWaterConfigLogged;
    static bool _testWaterDestroyed;
    static bool _testWaterRespawned;
    static int _testWaterInjectedFrame;
    static uint _testWaterInitialPlayer;
    static uint _testWaterInitialCallback;
    static ushort _testWaterInitialHealth;
    const int ImportedWaterDrownFrames = 120;
    const int ImportedWaterDrownDepth = 0x10000;
    static readonly bool TraceImportedWater =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_NATIVE_WATER") == "1";
    static readonly Dictionary<uint, int> ImportedWaterDwell = [];
    static readonly Stack<uint> CommonObjectMasks = new();
    static readonly bool _soakEnabled =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK") == "1";
    static readonly bool _unlockRoster =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_UNLOCK_ROSTER") == "1";
    static readonly int _soakPlayerType =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_V82_PLAYER_TYPE"),
            out int soakPlayerType)
            ? Math.Clamp(soakPlayerType, 0, byte.MaxValue)
            : -1;
    static readonly bool _soakPowerUpsEnabled =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK_POWERUPS") != "0";
    static readonly bool _soakWeaponsEnabled =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK_WEAPONS") != "0";
    static readonly bool _soakTransformCaptures =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK_CAPTURE_TRANSFORMS") == "1";
    static readonly bool _graphicsShowcaseCaptures =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_GRAPHICS_SHOWCASE") == "1";
    static bool _unlockRosterLogged;
    static uint _lastLoggedCheatFlags = uint.MaxValue;
    static readonly int _soakHeartbeatFrames =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_SOAK_HEARTBEAT_FRAMES"),
            out int heartbeatFrames)
            ? Math.Max(1, heartbeatFrames)
            : 180;
    static readonly int _soakTeardownFrame =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_SOAK_TEARDOWN_FRAMES"),
            out int teardownFrame)
            ? Math.Max(0, teardownFrame)
            : 0;
    static bool _soakTeardownSignaled;
    // Probes that need to drive the car themselves cannot do it while the soak
    // automation is also holding the gas and steering.
    static readonly bool _soakNoAutoInput =
        Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK_NO_AUTOINPUT") == "1";
    static ushort _soakAutomationInput;
    static int _soakInputPhase;
    static int _soakWeaponKind = -1;
    static readonly int _soakWeaponStartKind =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_V82_SOAK_START_KIND"),
            out int soakWeaponStartKind)
            ? Math.Clamp(soakWeaponStartKind, 1, 7)
            : 1;
    static int _soakWeaponAttachFrame;
    static uint _soakWeaponObject;
    static ushort _soakWeaponAmmo;
    static readonly HashSet<int> SoakWeaponsFired = new();
    static readonly HashSet<string> SoakSpecialCommands = new();
    static readonly HashSet<uint> SoakCallbacks = new();
    static readonly HashSet<string> CollisionShapeWarnings = new();
    static readonly Dictionary<uint, (uint HighMesh, uint LowMesh, uint Threshold)> LodThresholds = new();
    static bool _maximumLodLogged;
    static bool _stockLodRestoreLogged;
    static bool _expandedPrimitiveBuffersLogged;
    static bool _expandedPrimitiveBuffersActive;
    static uint _previousPrimitiveHighWaterWords;
    static uint _previousPrimitiveUsedWords;
    static readonly uint[] SoakDamageZoneOffsets = [0xF8u, 0xFCu, 0x100u];
    static bool _soakRepairWrenchCovered;
    static string? _lastSoakPowerState;
    static int _soakDurabilityRepairs;
    static readonly int[][][] SoakWeaponCommands =
    [
        [[0, 0, 2], [0, 0, 0], [0, 0, 3]],
        [[0, 2, 2], [0, 2, 0], [0, 2, 3]],
        [[2, 2, 2], [2, 2, 0], [2, 2, 3]],
        [[2, 0, 2], [2, 0, 0], [2, 0, 3]],
        [[1, 3, 2], [1, 3, 0], [1, 3, 3]],
        [[3, 1, 0], [3, 1, 2], [3, 1, 3]],
    ];

    public static bool BeginNativeGuestSelector(CpuContext c, IMemory m)
    {
        _selectorDepth++;
        if (_selectorDepth == 1)
            V82VehicleRegistry.BeginNativeSelector(c, m);
        if (_traceSelector && _selectorDepth == 1)
        {
            _selectorTraceCount = 0;
            SelectorTraceLines.Clear();
            Console.Error.WriteLine(
                $"[V82Selector] enter caller=0x{c.RA:X8} " +
                $"allowed=0x{c.A0:X8} mode=0x{c.A1:X8}");
        }
        return true;
    }

    public static void EndNativeGuestSelector(CpuContext c, IMemory m)
    {
        if (_selectorDepth == 1)
            V82VehicleRegistry.EndNativeSelector(c, m);
        if (_traceSelector)
            Console.Error.WriteLine(
                $"[V82Selector] leave result={c.V0} traces={_selectorTraceCount}");
        _selectorDepth = Math.Max(0, _selectorDepth - 1);
    }

    public static bool TraceNativeSelectorCall(CpuContext c, IMemory m)
    {
        if (_selectorDepth != 0)
            V82VehicleRegistry.ObserveNativeSelectorCall(c, m);
        if (!_traceSelector || _selectorDepth == 0 || _selectorTraceCount >= 512)
            return true;

        m = Dispatcher.UnwrapMemory(m);
        string a0 = ReadSelectorText(m, c.A0);
        string a1 = ReadSelectorText(m, c.A1);
        string a2 = ReadSelectorText(m, c.A2);
        string line =
            $"ra=0x{c.RA:X8} " +
            $"a0=0x{c.A0:X8}{a0} a1=0x{c.A1:X8}{a1} " +
            $"a2=0x{c.A2:X8}{a2} a3=0x{c.A3:X8}";
        if (SelectorTraceLines.Add(line))
        {
            _selectorTraceCount++;
            Console.Error.WriteLine($"[V82Selector] {line}");
        }
        return true;
    }

    static string ReadSelectorText(IMemory m, uint address)
    {
        if (address < 0x80010000u || address >= 0x80800000u)
            return "";
        Span<byte> bytes = stackalloc byte[65];
        int length = 0;
        for (; length < 64; length++)
        {
            byte value = m.ReadU8(address + (uint)length);
            if (value == 0)
                break;
            if (value < 0x20 || value > 0x7E)
                return "";
            bytes[length] = value;
        }
        if (length < 3 || length == 64)
            return "";
        return $" \"{System.Text.Encoding.ASCII.GetString(bytes[..length])}\"";
    }

    // Keep the retail heap intact for the executable's bookkeeping helpers,
    // while routing allocations to an independent PC-only arena. Mixing a
    // devkit block into the retail free list is unsafe when malloc is entered
    // through a relocated overlay: linked addresses can then be stored as
    // physical free-list links.
    public static void ExtendHeapPost(CpuContext c, IMemory m)
    {
        if (_extendedHeapInstalled || Runtime.Mode != RunMode.Devkit) return;

        ApplyConfiguredCheats(m);
        if (_unlockRoster)
        {
            // The low nine bits are the retail completion flags for the nine
            // bonus drivers. Keep this process-local so automated coverage
            // does not modify either memory-card image.
            m.WriteU16(0x8006BAF8u, 0x01FF);
            LogRosterUnlock();
        }
        PcFreeBlocks.Clear();
        PcAllocations.Clear();
        PcFreeBlocks.Add((PcHeapBase, PcHeapEnd - PcHeapBase));
        _extendedHeapInstalled = true;
        V82VehicleRegistry.Initialize(c, m);
        Console.Error.WriteLine(
            $"[V82Compat] initialized isolated {(PcHeapEnd - PcHeapBase) >> 20} MiB " +
            $"PC heap at 0x{PcHeapBase:X8}-0x{PcHeapEnd:X8}; reserved " +
            $"0x{ExpandedPrimitiveBufferBase:X8}-0x{PcHeapBase:X8} for Maximum-LOD packets");
    }

    public static void PcMalloc(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        EnsurePcHeap();

        // SHELL.DLL is the first sector-rounded 0x1B000-byte file allocation.
        // Load it directly at its linked address and reserve that one physical
        // range. Loading it elsewhere and also reserving its alias consumes the
        // same 110 KiB twice, which leaves too little RAM for a complete match.
        if (!_shellPinnedAtLinkedBase &&
            c.RA == 0x800183B4u &&
            c.A0 == ShellSectorAllocation)
        {
            // The CD loader writes complete 2048-byte sectors before shrinking
            // the allocation to the file's 0x1AED8-byte logical size. Reserve
            // through 0x1B000 so the final sector cannot overwrite the free
            // block header immediately following the linked image.
            V8Compat.ReserveHeapRange(
                m, HeapHeadAddress, ShellLinkedBase,
                ShellLinkedBase + ShellSectorAllocation);
            _shellPinnedAtLinkedBase = true;
            c.V0 = ShellLinkedBase;
            return;
        }

        uint request = c.A0;
        if (request == 0u)
        {
            c.V0 = 0u;
            return;
        }

        uint total = Align8(request + 8u);
        for (int i = PcFreeBlocks.Count - 1; i >= 0; i--)
        {
            var free = PcFreeBlocks[i];
            if (free.Size < total) continue;

            uint header = free.Start + free.Size - total;
            uint payload = header + 8u;
            if (free.Size == total)
                PcFreeBlocks.RemoveAt(i);
            else
                PcFreeBlocks[i] = (free.Start, free.Size - total);

            PcAllocations[payload] = (header, total);
            m.WriteU32(header, 0u);
            m.WriteU32(header + 4u, total >> 3);
            c.V0 = payload;

            RestoreShellDecodeFrame(c, m);
            return;
        }

        c.V0 = 0u;
        Console.Error.WriteLine(
            $"[V82Compat] PC heap exhausted allocating {request} bytes from 0x{c.RA:X8}");
    }

    static void RestoreShellDecodeFrame(CpuContext c, IMemory m)
    {
        if (ShellImageDecodeFrames.Count != 0 &&
            c.RA is 0x801106D8u or 0x801106E4u)
        {
            var frame = ShellImageDecodeFrames.Peek();
            c.SP = frame.FrameSp;
            m.WriteU32(frame.FrameSp + 0x58u, frame.SourceRect);
        }
    }

    public static void PcFree(CpuContext c, IMemory m)
    {
        uint pointer = c.A0;
        if (pointer == 0u || (_shellPinnedAtLinkedBase && pointer == ShellLinkedBase))
            return;

        if (!PcAllocations.Remove(pointer, out var allocation))
        {
            Console.Error.WriteLine(
                $"[V82Compat] ignored non-PC free 0x{pointer:X8} from 0x{c.RA:X8}");
            return;
        }

        // Object event 2 can destroy a vehicle's visual/physics resources
        // while retaining the object itself for the result screen. Keep its
        // guest identity alive until the allocator actually releases that
        // object; the later global event-4 teardown still dispatches through
        // the vehicle callback.
        V82VehicleRegistry.ReleaseFreedObjectMapping(pointer, c, m);
        InsertFreeBlock(allocation.Header, allocation.Size);
    }

    public static int PcAllocationCount
    {
        get
        {
            lock (PcAllocations)
                return PcAllocations.Count;
        }
    }

    public static bool IsPcAllocationLive(uint pointer)
    {
        lock (PcAllocations)
            return PcAllocations.ContainsKey(pointer);
    }

    public static string ProbeGuestIdentityLifetime(
        CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        c.A0 = 0x100u;
        PcMalloc(c, m);
        uint vehicle = c.V0;
        if (vehicle == 0u)
            throw new InvalidOperationException(
                "guest identity probe could not allocate an object");

        V82VehicleRegistry.RegisterObjectMappingForProbe(vehicle);
        c.A0 = vehicle;
        V82VehicleRegistry.ReleaseVehicleMapping(c, m);
        if (!V82VehicleRegistry.HasObjectMappingForProbe(vehicle))
            throw new InvalidOperationException(
                "event-2 teardown retired the guest object too early");

        c.A0 = vehicle;
        PcFree(c, m);
        if (V82VehicleRegistry.HasObjectMappingForProbe(vehicle))
            throw new InvalidOperationException(
                "allocator free retained a stale guest object identity");

        return $"object=0x{vehicle:X8} event2=retained pcfree=retired";
    }

    public static void PcRealloc(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint pointer = c.A0;
        uint request = c.A1;
        if (pointer == 0u)
        {
            c.A0 = request;
            PcMalloc(c, m);
            return;
        }
        if (request == 0u)
        {
            PcFree(c, m);
            c.V0 = 0u;
            return;
        }

        if (pointer == ShellLinkedBase)
        {
            c.V0 = pointer;
            return;
        }

        if (!PcAllocations.TryGetValue(pointer, out var old))
        {
            c.V0 = pointer;
            Console.Error.WriteLine(
                $"[V82Compat] retained unknown realloc 0x{pointer:X8} ({request} bytes)");
            return;
        }

        uint oldCapacity = old.Size - 8u;
        if (request <= oldCapacity)
        {
            c.V0 = pointer;
            return;
        }

        c.A0 = request;
        PcMalloc(c, m);
        uint replacement = c.V0;
        if (replacement == 0u) return;

        CopyBytes(m, pointer, replacement, oldCapacity);
        c.A0 = pointer;
        PcFree(c, m);
        c.V0 = replacement;
    }

    static void EnsurePcHeap()
    {
        if (_extendedHeapInstalled) return;
        PcFreeBlocks.Add((PcHeapBase, PcHeapEnd - PcHeapBase));
        _extendedHeapInstalled = true;
    }

    static uint Align8(uint value) => (value + 7u) & ~7u;

    static void InsertFreeBlock(uint start, uint size)
    {
        int index = 0;
        while (index < PcFreeBlocks.Count && PcFreeBlocks[index].Start < start)
            index++;
        PcFreeBlocks.Insert(index, (start, size));

        if (index > 0)
        {
            var previous = PcFreeBlocks[index - 1];
            var current = PcFreeBlocks[index];
            if (previous.Start + previous.Size == current.Start)
            {
                PcFreeBlocks[index - 1] =
                    (previous.Start, previous.Size + current.Size);
                PcFreeBlocks.RemoveAt(index);
                index--;
            }
        }
        if (index + 1 < PcFreeBlocks.Count)
        {
            var current = PcFreeBlocks[index];
            var next = PcFreeBlocks[index + 1];
            if (current.Start + current.Size == next.Start)
            {
                PcFreeBlocks[index] = (current.Start, current.Size + next.Size);
                PcFreeBlocks.RemoveAt(index + 1);
            }
        }
    }

    static void CopyBytes(IMemory m, uint source, uint destination, uint length)
    {
        uint offset = 0u;
        while (offset + 4u <= length)
        {
            m.WriteU32(destination + offset, m.ReadU32(source + offset));
            offset += 4u;
        }
        while (offset < length)
        {
            m.WriteU8(destination + offset, m.ReadU8(source + offset));
            offset++;
        }
    }

    // LOAD.DLL's VLC decoder is hand-written assembly with several tail jumps
    // into the middle of the routine. Splitting those entries into C# methods
    // loses the native return-address semantics and corrupts decoded level
    // data. Execute this one bounded region directly so JAL/JR and delay slots
    // retain their original MIPS behavior.
    public static void RunLoadVlc(CpuContext c, IMemory m)
        => RunVlcRegion(c, m, 0x80109704u, 0x80109C0Cu, "LOAD");

    public static void RunShellVlc(CpuContext c, IMemory m)
    {
        if (_traceVram)
        {
            Console.Error.WriteLine(
                $"[ShellVlc] source=0x{c.A0:X8} target=0x{c.A1:X8} " +
                $"sp=0x{c.SP:X8} ra=0x{c.RA:X8}");
        }
        RunVlcRegion(c, m, 0x80110D14u, 0x80111220u, "SHELL");
    }

    // Current checked-in generated sources predate the SHELL replacement
    // hook. Keep their original body compile-reachable while routing every
    // live call through the exact interpreter; a fresh RecompOne generation
    // replaces that body outright via prepare_reference.py.
    public static bool RunShellVlcHandled(CpuContext c, IMemory m)
    {
        RunShellVlc(c, m);
        return true;
    }

    static void RunVlcRegion(
        CpuContext c, IMemory m, uint start, uint end, string region)
    {
        uint pc = start;

        for (int steps = 0; steps < 50_000_000; steps++)
        {
            if (pc < start || pc >= end)
            {
                throw new InvalidOperationException(
                    $"{region} VLC escaped its native region before fetch: " +
                    $"pc=0x{pc:X8} source=0x{c.A0:X8} target=0x{c.A1:X8} " +
                    $"sp=0x{c.SP:X8} ra=0x{c.RA:X8}");
            }
            uint instruction = m.ReadU32(pc);
            uint opcode = instruction >> 26;
            int rs = (int)((instruction >> 21) & 31u);
            int rt = (int)((instruction >> 16) & 31u);

            if (opcode == 0u && (instruction & 63u) == 8u)
            {
                uint target = c[rs];
                ExecuteLoadVlcDelaySlot(c, m, pc + 4u);
                if (target < start || target >= end)
                    return;
                pc = target;
                continue;
            }

            if (opcode is 2u or 3u)
            {
                uint target = ((pc + 4u) & 0xF0000000u) |
                    ((instruction & 0x03FFFFFFu) << 2);
                // V8 overlays are linked against 0x80000000 and the retail
                // loader patches absolute J/JAL instructions when placing
                // them at 0x80100000. A bounded probe may load the raw DLL
                // bytes directly, so recognize the equivalent unrelocated
                // target without weakening the decoder-region boundary.
                uint overlayTarget =
                    (start & 0xFFF00000u) | (target & 0x000FFFFFu);
                if (overlayTarget >= start && overlayTarget < end)
                    target = overlayTarget;
                if (m is RelocatedMemory relocated &&
                    target >= start + relocated.Delta &&
                    target < end + relocated.Delta)
                    target -= relocated.Delta;
                if (opcode == 3u)
                    c.RA = pc + 8u;
                ExecuteLoadVlcDelaySlot(c, m, pc + 4u);
                if (target < start || target >= end)
                    throw new InvalidOperationException(
                        $"{region} VLC jumped outside its native region: 0x{pc:X8} -> 0x{target:X8}");
                pc = target;
                continue;
            }

            bool branch = false;
            bool isBranch = true;
            switch (opcode)
            {
                case 1u:
                    branch = rt switch
                    {
                        0 => (int)c[rs] < 0,
                        1 => (int)c[rs] >= 0,
                        16 => (int)c[rs] < 0,
                        17 => (int)c[rs] >= 0,
                        _ => throw UnsupportedLoadVlcInstruction(pc, instruction),
                    };
                    if (branch && rt is 16 or 17)
                        c.RA = pc + 8u;
                    break;
                case 4u:
                    branch = c[rs] == c[rt];
                    break;
                case 5u:
                    branch = c[rs] != c[rt];
                    break;
                case 6u:
                    branch = (int)c[rs] <= 0;
                    break;
                case 7u:
                    branch = (int)c[rs] > 0;
                    break;
                default:
                    isBranch = false;
                    break;
            }

            if (isBranch)
            {
                ExecuteLoadVlcDelaySlot(c, m, pc + 4u);
                pc = branch
                    ? unchecked(pc + 4u + (uint)((int)(short)instruction << 2))
                    : pc + 8u;
                if (pc < start || pc >= end)
                    throw new InvalidOperationException(
                        $"{region} VLC branched outside its native region to 0x{pc:X8}");
                continue;
            }

            ExecuteLoadVlcInstruction(c, m, pc, instruction);
            pc += 4u;
        }

        throw new InvalidOperationException(
            $"{region} VLC exceeded its 50-million-instruction safety limit");
    }

    static void ExecuteLoadVlcDelaySlot(CpuContext c, IMemory m, uint pc)
    {
        uint instruction = m.ReadU32(pc);
        uint opcode = instruction >> 26;
        if (opcode is >= 1u and <= 7u ||
            opcode == 0u && (instruction & 63u) == 8u)
            throw new InvalidOperationException(
                $"LOAD VLC has an unsupported control transfer in delay slot 0x{pc:X8}");
        ExecuteLoadVlcInstruction(c, m, pc, instruction);
    }

    static void ExecuteLoadVlcInstruction(
        CpuContext c, IMemory m, uint pc, uint instruction)
    {
        uint opcode = instruction >> 26;
        int rs = (int)((instruction >> 21) & 31u);
        int rt = (int)((instruction >> 16) & 31u);
        int rd = (int)((instruction >> 11) & 31u);
        int shift = (int)((instruction >> 6) & 31u);
        ushort immediate = (ushort)instruction;
        uint address = unchecked(c[rs] + (uint)(int)(short)immediate);

        switch (opcode)
        {
            case 0u:
                c[rd] = (instruction & 63u) switch
                {
                    0x00u => c[rt] << shift,
                    0x02u => c[rt] >> shift,
                    0x04u => c[rt] << (int)(c[rs] & 31u),
                    0x06u => c[rt] >> (int)(c[rs] & 31u),
                    0x20u or 0x21u => unchecked(c[rs] + c[rt]),
                    0x22u or 0x23u => unchecked(c[rs] - c[rt]),
                    0x24u => c[rs] & c[rt],
                    0x25u => c[rs] | c[rt],
                    0x26u => c[rs] ^ c[rt],
                    0x2Au => (uint)((int)c[rs] < (int)c[rt] ? 1 : 0),
                    0x2Bu => c[rs] < c[rt] ? 1u : 0u,
                    _ => throw UnsupportedLoadVlcInstruction(pc, instruction),
                };
                return;
            case 8u:
            case 9u:
                c[rt] = address;
                return;
            case 12u:
                c[rt] = c[rs] & immediate;
                return;
            case 13u:
                c[rt] = c[rs] | immediate;
                return;
            case 14u:
                c[rt] = c[rs] ^ immediate;
                return;
            case 15u:
                c[rt] = (uint)immediate << 16;
                return;
            case 18u:
                uint cop2Operation = (instruction >> 21) & 31u;
                if (cop2Operation == 0u)
                {
                    c[rt] = Gte.Read(rd);
                    return;
                }
                if (cop2Operation == 4u)
                {
                    Gte.Write(rd, c[rt]);
                    return;
                }
                throw UnsupportedLoadVlcInstruction(pc, instruction);
            case 32u:
                c[rt] = (uint)(int)(sbyte)m.ReadU8(address);
                return;
            case 33u:
                c[rt] = (uint)(int)(short)m.ReadU16(address);
                return;
            case 35u:
                c[rt] = m.ReadU32(address);
                return;
            case 36u:
                c[rt] = m.ReadU8(address);
                return;
            case 37u:
                c[rt] = m.ReadU16(address);
                return;
            case 40u:
                m.WriteU8(address, (byte)c[rt]);
                return;
            case 41u:
                m.WriteU16(address, (ushort)c[rt]);
                return;
            case 43u:
                m.WriteU32(address, c[rt]);
                return;
            default:
                throw UnsupportedLoadVlcInstruction(pc, instruction);
        }
    }

    static InvalidOperationException UnsupportedLoadVlcInstruction(
        uint pc, uint instruction) =>
        new($"Unsupported LOAD VLC instruction 0x{instruction:X8} at 0x{pc:X8}");

    public static void TraceObjectFactorySource(CpuContext c, IMemory m)
    {
        ObjectFactorySources.Push(c.A0);
        Dispatcher.AssociateObjectOwner(c.A0, m);
        if (c.A0 < 0x80010000u || c.A0 >= PcHeapEnd)
            return;

        uint callback = m.ReadU32(c.A0);
        uint database = m.ReadU32(c.A0 + 0x5Cu);
        if (callback != 0x800368DCu &&
            database >= 0x80010000u && database < PcHeapEnd)
            return;
        if (_objectFactoryTraceCount++ >= 32)
            return;

        Console.Error.WriteLine(
            $"[V82Object] source=0x{c.A0:X8} callback=0x{callback:X8} " +
            $"database=0x{database:X8} flags=0x{m.ReadU32(c.A0 + 4u):X8} " +
            $"index={m.ReadU16(c.A0 + 0x1Au)}");
        if (database >= 0x80010000u && database < PcHeapEnd)
        {
            Console.Error.WriteLine(
                $"[V82Object] database words: " +
                $"0x{m.ReadU32(database):X8} 0x{m.ReadU32(database + 4u):X8} " +
                $"0x{m.ReadU32(database + 8u):X8} 0x{m.ReadU32(database + 12u):X8}");
        }
    }

    public static void RelocateLookupKey(CpuContext c, IMemory m)
    {
        c.A0 = Dispatcher.ResolveLinkedAddress(m, c.A0);
    }

    public static void TraceObjectFactoryResult(CpuContext c, IMemory m)
    {
        uint source = ObjectFactorySources.Count != 0
            ? ObjectFactorySources.Pop()
            : 0u;
        Dispatcher.AssociateObjectOwner(c.V0, m);
        if (source != 0u &&
            VehicleFactorySources.Remove(source) &&
            c.V0 != 0u)
        {
            VehicleObjects.Add(c.V0);
            if (TraceRendererOwnership)
                Console.Error.WriteLine(
                    $"[EnhancedOwner] vehicle source=0x{source:X8} " +
                    $"object=0x{c.V0:X8}");
        }
        if (source != 0u && source == _playerVehicle && c.V0 != 0u)
        {
            _playerVehicle = c.V0;
            Console.Error.WriteLine(
                $"[V82Object] player runtime object=0x{_playerVehicle:X8}");
        }
    }

    /// <summary>
    /// Marks the exact primitive-buffer interval produced while the native
    /// V8:2 object renderer traverses one vehicle hierarchy. Enhanced uses
    /// this upstream ownership instead of guessing glass from screen-space
    /// colors or texture contents.
    /// </summary>
    public static void BeginObjectRender(CpuContext c, IMemory m)
    {
        ObjectRenderEntries++;
        TraceObjectRenderBegin(c, m);
        m = Dispatcher.UnwrapMemory(m);
        uint objectAddress = c.A0;
        uint packetStart = m.ReadU32(c.GP + 0x610u);
        if (objectAddress == TraceObjectAddress && _traceObjectCount++ < 256)
        {
            Console.Error.WriteLine(
                $"[V82RenderObjectTrace] gameplay={_gameplayFrameCount} " +
                $"object=0x{objectAddress:X8} caller=0x{c.RA:X8} " +
                $"packets=0x{packetStart:X8} " +
                $"words00=0x{m.ReadU32(objectAddress):X8}," +
                $"0x{m.ReadU32(objectAddress + 4u):X8}," +
                $"0x{m.ReadU32(objectAddress + 8u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0xCu):X8} " +
                $"links=0x{m.ReadU32(objectAddress + 0x10u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x14u):X8} " +
                $"words18=0x{m.ReadU32(objectAddress + 0x18u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x1Cu):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x20u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x24u):X8} " +
                $"words40=0x{m.ReadU32(objectAddress + 0x40u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x44u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x48u):X8}," +
                $"0x{m.ReadU32(objectAddress + 0x4Cu):X8}");
        }
        bool isVehicle =
            VehicleObjects.Contains(objectAddress) ||
            V82VehicleRegistry.IsVehicleObject(objectAddress);
        ObjectRenderScopes.Push(
            new ObjectRenderScope(
                objectAddress,
                packetStart,
                c.RA,
                isVehicle));
    }

    public static void EndObjectRender(CpuContext c, IMemory m)
    {
        ObjectRenderExits++;
        TraceObjectRenderEnd(c, m);
        if (ObjectRenderScopes.Count == 0)
            return;

        m = Dispatcher.UnwrapMemory(m);
        ObjectRenderScope scope = ObjectRenderScopes.Pop();
        uint packetEnd = m.ReadU32(c.GP + 0x610u);
        if (TraceRendererOwnership &&
            scope.ObjectAddress != 0u &&
            TracedRenderObjects.Count < 4096 &&
            TracedRenderObjects.Add(scope.ObjectAddress))
        {
            Console.Error.WriteLine(
                $"[V82RenderObject] gameplay={_gameplayFrameCount} " +
                $"object=0x{scope.ObjectAddress:X8} caller=0x{scope.Caller:X8} " +
                $"packets=0x{scope.PacketStart:X8}..0x{packetEnd:X8} " +
                $"words00=0x{m.ReadU32(scope.ObjectAddress):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 4u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 8u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0xCu):X8} " +
                $"links=0x{m.ReadU32(scope.ObjectAddress + 0x10u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x14u):X8} " +
                $"words18=0x{m.ReadU32(scope.ObjectAddress + 0x18u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x1Cu):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x20u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x24u):X8} " +
                $"words40=0x{m.ReadU32(scope.ObjectAddress + 0x40u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x44u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x48u):X8}," +
                $"0x{m.ReadU32(scope.ObjectAddress + 0x4Cu):X8}");
        }
        // A0 is clobbered by the time the renderer returns, so the object
        // pointer has to come from the scope captured on entry. Diagnostic
        // only - gated so a shipping build does not pay a dictionary write and
        // three reads for every object every frame.
        // Always recorded: one dictionary write and three reads per object per
        // frame is nothing, and this is the only thing that distinguishes an
        // object that was offered and drew nothing from one that was never
        // offered. Gating it meant every capture from a normal play session
        // came back empty.
        if (_objectCensus.Count < 8192 &&
            scope.ObjectAddress != 0u &&
            (scope.ObjectAddress & 0x1FFFFFFFu) <= 0x00FFFFF0u)
        {
            // The object struct's first words are not its position; the
            // visibility test receives a separate position pointer, which is
            // the only place the world coordinates are known.
            uint pos = _objectCullPosition;
            bool havePos = pos != 0u &&
                (pos & 0x1FFFFFFFu) <= 0x00FFFFF0u;
            _objectCensus[scope.ObjectAddress] = new ObjectRecord(
                scope.ObjectAddress,
                havePos ? unchecked((int)m.ReadU32(pos)) : 0,
                havePos ? unchecked((int)m.ReadU32(pos + 4u)) : 0,
                havePos ? unchecked((int)m.ReadU32(pos + 8u)) : 0,
                (int)_objectCullRadius,
                packetEnd > scope.PacketStart
                    ? "drawn"
                    : _lastEmptyReason ?? "empty",
                CensusFrame);
        }
        if (TraceRendererOwnership && _rendererOwnershipTraceCount++ < 128)
            Console.Error.WriteLine(
                $"[EnhancedOwner] render object=0x{scope.ObjectAddress:X8} " +
                $"vehicle={(scope.IsVehicle ? 1 : 0)} " +
                $"packets=0x{scope.PacketStart:X8}..0x{packetEnd:X8}");
        // Ownership was only ever recorded for vehicles, which left every
        // other submitter reading as "unresolved" - including the arena walls
        // this investigation is chasing. Name them too when a capture is
        // running, so a missing model can be attributed to a subsystem.
        // Always named, for the same reason the census is always recorded: a
        // capture from a normal play session is the only place the reported
        // artifact appears, and an unattributed packet cannot be traced to the
        // object that failed to draw it.
        if (!scope.IsVehicle && packetEnd > scope.PacketStart)
            GpuHle.RegisterPacketOwnerRange(
                scope.PacketStart,
                packetEnd,
                $"v82-object=0x{scope.ObjectAddress:X8}");
        if (!scope.IsVehicle || packetEnd <= scope.PacketStart)
            return;

        GpuHle.RegisterVehiclePacketRange(scope.PacketStart, packetEnd);
        GpuHle.RegisterPacketOwnerRange(
            scope.PacketStart,
            packetEnd,
            $"v82-vehicle-object=0x{scope.ObjectAddress:X8}");
    }

    static readonly Stack<ImportedRenderGroupScope> ImportedRenderGroupScopes =
        new();
    static readonly bool TraceImportedRenderGroups =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_V82_RENDER_GROUPS") == "1";
    static int _importedRenderGroupTraceCount;

    /// <summary>
    /// Captures packet provenance at the native model renderer boundary.  This
    /// is the narrowest source-owned seam that distinguishes the imported
    /// body, its authored distance LOD, and independently mounted wheels.
    /// </summary>
    public static void BeginImportedRenderGroup(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        CaptureRenderGroupEntry(c, m);
        uint packetStart = m.ReadU32(c.GP + 0x610u);
        bool resolved = V82VehicleRegistry.TryDescribeImportedRenderGroup(
            m,
            c.A0,
            out V82VehicleRegistry.ImportedRenderGroupInfo info);
        ImportedRenderGroupScopes.Push(
            new ImportedRenderGroupScope(
                packetStart,
                c.A0,
                resolved,
                info));
        if (!resolved && TraceImportedRenderGroups)
        {
            string relationship =
                V82VehicleRegistry.DescribeImportedRenderGroupMiss(
                    m,
                    c.A0);
            // Retail shell/model draws occur before any imported bank exists.
            // They are outside this diagnostic's scope and must not consume
            // the bounded trace budget needed for an imported gameplay model.
            if (relationship != "none" &&
                _importedRenderGroupTraceCount++ < 128)
                Console.Error.WriteLine(
                    $"[V82RenderGroupMiss] descriptor=0x{c.A0:X8} " +
                    relationship);
        }
    }

    public static void EndImportedRenderGroup(CpuContext c, IMemory m)
    {
        if (ImportedRenderGroupScopes.Count == 0)
            return;
        m = Dispatcher.UnwrapMemory(m);
        ImportedRenderGroupScope scope = ImportedRenderGroupScopes.Pop();
        if (!scope.Resolved)
            return;
        uint packetEnd = m.ReadU32(c.GP + 0x610u);
        if (packetEnd <= scope.PacketStart)
            return;
        string owner =
            $"v82-imported={scope.Info.StableId} " +
            $"bank={scope.Info.BankKind} " +
            $"group={scope.Info.GroupIndex} " +
            $"distance-lod={(scope.Info.DistanceLod ? 1 : 0)}";
        // func_80021F70 is the native model renderer for each authored BIN
        // group.  Imported gameplay vehicles can reach it through a native
        // hierarchy child rather than the registry's top-level object
        // pointer, so the outer object scope alone is not authoritative.
        // Register this exact source-resolved packet interval with the same
        // vehicle material system used by native render scopes.
        GpuHle.RegisterVehiclePacketRange(
            scope.PacketStart,
            packetEnd);
        GpuHle.RegisterPacketOwnerRange(
            scope.PacketStart,
            packetEnd,
            owner);
        if (TraceImportedRenderGroups &&
            _importedRenderGroupTraceCount++ < 4096)
            Console.Error.WriteLine(
                $"[V82RenderGroup] descriptor=0x{scope.Descriptor:X8} " +
                $"packets=0x{scope.PacketStart:X8}..0x{packetEnd:X8} " +
                owner);
    }

    public static void ValidateConstructedObject(CpuContext c, IMemory m)
    {
        uint objectAddress = c.V0;
        if (!IsDevkitRamPointer(objectAddress))
            return;

        Dispatcher.AssociateObjectOwner(objectAddress, m);
        foreach (uint offset in new uint[] { 0x0Cu, 0x10u, 0x14u })
        {
            uint link = m.ReadU32(objectAddress + offset);
            if (link == 0u || IsDevkitRamPointer(link))
                continue;
            m.WriteU32(objectAddress + offset, 0u);
            Console.Error.WriteLine(
                $"[V82Object] rejected malformed hierarchy link " +
                $"object=0x{objectAddress:X8} offset=0x{offset:X2} " +
                $"value=0x{link:X8}");
        }
    }

    static bool IsDevkitRamPointer(uint address)
    {
        uint physical = address & 0x1FFFFFFFu;
        return (address & 0x80000000u) != 0u && physical < 0x00800000u;
    }

    public static void RepairTerrainQuery(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        const uint table = 0x800B93F0u;
        const uint blockMask = 0x003FFFFFu;

        uint xBlock = c.A0 >> 22;
        uint zBlock = c.A1 >> 22;
        if (xBlock >= 32u)
        {
            xBlock = 31u;
            c.A0 = (31u << 22) | (c.A0 & blockMask);
        }
        if (zBlock >= 32u)
        {
            zBlock = 31u;
            c.A1 = (31u << 22) | (c.A1 & blockMask);
        }

        uint xNextBlock = Math.Min(31u, ((c.A0 >> 16) + 1u) >> 6);
        uint zNextBlock = Math.Min(31u, ((c.A1 >> 16) + 1u) >> 6);
        foreach ((uint bx, uint bz) in new[]
        {
            (xBlock, zBlock),
            (xNextBlock, zBlock),
            (xBlock, zNextBlock),
            (xNextBlock, zNextBlock),
        })
        {
            RepairTerrainSlot(m, table, bx, bz);
        }
    }

    public static void RepairObjectTerrainQuery(CpuContext c, IMemory m)
    {
        uint objectAddress = c.A0;
        if (!IsDevkitRamRange(objectAddress, 0x70u))
            return;

        m = Dispatcher.UnwrapMemory(m);
        ApplyLevelOfDetail(objectAddress, m);

        uint savedA1 = c.A1;
        uint x = m.ReadU32(objectAddress + 0x34u);
        uint z = m.ReadU32(objectAddress + 0x3Cu);
        c.A0 = x;
        c.A1 = z;
        RepairTerrainQuery(c, m);
        if (c.A0 != x)
            m.WriteU32(objectAddress + 0x34u, c.A0);
        if (c.A1 != z)
            m.WriteU32(objectAddress + 0x3Cu, c.A1);
        c.A0 = objectAddress;
        c.A1 = savedA1;
    }

    /// <summary>
    /// Expands the native terrain traversal polygon to the same horizontal
    /// field of view as the Enhanced widescreen projection.  func_8001C158
    /// derives its X corner rays from gp+0xEDC and its Y corner rays from
    /// gp+0xF20.  Leaving the X extent at its authored 4:3 value means the
    /// native terrain walker never submits the sectors exposed at the sides
    /// of a 16:9 target, producing large sky-coloured holes even though the
    /// Enhanced renderer itself rejects no triangles.
    /// </summary>
    // func_8002DFF0 builds the frustum planes at gp+0xF00 (copied to gp+0xFD8)
    // from gp+0xEDC, the authored 320-wide clip width. Those planes are what
    // func_8002E22C tests every object and every scenery/backdrop quad
    // against, so in widescreen the leftmost and rightmost items are rejected
    // even though they are now on screen - the panorama then stops short of
    // the frame edge and the world beside it has nothing behind it.
    //
    // Widen the clip width across the plane build and put it straight back,
    // exactly as ExpandTerrainFrustum does for the terrain traversal. Only the
    // planes change; gp+0xEDC is restored before anything else reads it.
    static uint _objectFrustumWidthAddress;
    static uint _objectFrustumNativeWidth;
    static bool _objectFrustumAdjusted;
    static bool _objectFrustumLogged;
    static readonly double ObjectFrustumScale =
        double.TryParse(
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_OBJECT_FRUSTUM_SCALE"),
            System.Globalization.NumberStyles.Float,
            System.Globalization.CultureInfo.InvariantCulture,
            out double objectFrustumScale)
            ? objectFrustumScale
            : -1d;

    // Attribution probe: the panorama comes out "unresolved" because none of
    // the existing ownership brackets cover whatever draws it. Bracketing the
    // remaining per-frame renderers names the one that does.
    static readonly Stack<(string Name, uint Start)> NamedRenderScopes = new();

    static void BeginNamedRender(CpuContext c, IMemory m, string name)
    {
        if (!CensusOwnership)
            return;
        NamedRenderScopes.Push(
            (name, Dispatcher.UnwrapMemory(m).ReadU32(c.GP + 0x610u)));
    }

    static void EndNamedRender(CpuContext c, IMemory m)
    {
        if (!CensusOwnership || NamedRenderScopes.Count == 0)
            return;
        var (name, start) = NamedRenderScopes.Pop();
        uint end = Dispatcher.UnwrapMemory(m).ReadU32(c.GP + 0x610u);
        if (end > start)
            GpuHle.RegisterPacketOwnerRange(start, end, $"v82-pass={name}");
    }

    static readonly Stack<(uint Addr, uint Start)> IndirectCalls = new();
    public static int IndirectPacketDepth => IndirectCalls.Count;

    // Only the four mesh emitters. Bracketing every indirect call buried the
    // signal in unrelated traffic.
    static readonly HashSet<uint> MeshEmitters =
        [0x80022A4Cu, 0x80022C54u, 0x800229A0u, 0x80022870u];

    public static void BeginIndirectCall(CpuContext c, IMemory m, uint addr)
    {
        if (!MeshEmitters.Contains(addr))
            return;
        IndirectCalls.Push(
            (addr, Dispatcher.UnwrapMemory(m).ReadU32(c.GP + 0x610u)));
    }

    // Each mesh-emitter call handles one triangle, so a call that writes no
    // packet is a polygon the engine considered and dropped. Counting those
    // per emitter measures polygon-level loss directly, without having to
    // guess which test inside the emitter rejected it.
    public static readonly Dictionary<uint, (long Emitted, long Dropped)>
        IndirectPolygonCounts = [];

    public static void EndIndirectCall(CpuContext c, IMemory m, uint addr)
    {
        if (!MeshEmitters.Contains(addr) || IndirectCalls.Count == 0)
            return;
        // Depth of the triangle just projected, so drops can be attributed to
        // how close the polygon was rather than merely counted.
        int sz = Gte.LastProjectedDepth;
        if (sz > 0)
        {
            if (sz < 128) EmitterNearTotal++;
        }
        var (target, start) = IndirectCalls.Pop();
        uint end = Dispatcher.UnwrapMemory(m).ReadU32(c.GP + 0x610u);
        if (end > start)
            GpuHle.RegisterPacketOwnerRange(
                start, end, $"v82-indirect=0x{target:X8}");
        if (sz > 0 && sz < 128 && end <= start) EmitterNearDropped++;
        // Record the triangle itself when the emitter wrote no packet. This is
        // the geometry that goes missing, and nothing else in the pipeline can
        // observe it.
        if (end <= start && RejectedTriangles.Count < 4096)
        {
            Span<int> xs = stackalloc int[3];
            Span<int> ys = stackalloc int[3];
            Span<int> zs = stackalloc int[3];
            Gte.ReadProjectedTriangle(xs, ys, zs);
            RejectedTriangles.Add(new RejectedTriangle(
                target, xs[0], ys[0], zs[0], xs[1], ys[1], zs[1],
                xs[2], ys[2], zs[2], Gte.CurrentFlags, Gte.CurrentMac0));
        }
        IndirectPolygonCounts.TryGetValue(target, out var counts);
        IndirectPolygonCounts[target] = end > start
            ? (counts.Emitted + 1, counts.Dropped)
            : (counts.Emitted, counts.Dropped + 1);
    }

    public static string DescribePolygonDrops()
    {
        var parts = new List<string>();
        foreach (var kv in IndirectPolygonCounts)
        {
            if (kv.Value.Emitted + kv.Value.Dropped < 200)
                continue;
            parts.Add($"{kv.Key:X8}:{kv.Value.Emitted}/{kv.Value.Dropped}");
        }
        return string.Join(" ", parts);
    }

    public static void BeginSceneryPass(CpuContext c, IMemory m) =>
        BeginNamedRender(c, m, "scenery-80050B38");

    public static void EndSceneryPass(CpuContext c, IMemory m) =>
        EndNamedRender(c, m);

    public static void BeginSkyPass(CpuContext c, IMemory m) =>
        BeginNamedRender(c, m, "pass-8003150C");

    public static void EndSkyPass(CpuContext c, IMemory m) =>
        EndNamedRender(c, m);

    public static void BeginLatePass(CpuContext c, IMemory m) =>
        BeginNamedRender(c, m, "pass-8001C910");

    public static void EndLatePass(CpuContext c, IMemory m) =>
        EndNamedRender(c, m);

    public static void ExpandObjectFrustum(CpuContext c, IMemory m)
    {
        _objectFrustumAdjusted = false;
        if (ObjectFrustumScale == 0d ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            GpuHle.WideAspect <= GpuHle.BaseAspect + 0.001f)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint address = c.GP + 0xEDCu;
        uint nativeWidth = m.ReadU32(address);
        if (nativeWidth == 0u || nativeWidth > 0x00100000u)
            return;

        double scale = ObjectFrustumScale >= 1d
            ? ObjectFrustumScale
            : (double)GpuHle.WideAspect / GpuHle.BaseAspect;
        uint expandedWidth = checked((uint)Math.Clamp(
            Math.Round(nativeWidth * scale, MidpointRounding.AwayFromZero),
            1d,
            0x00100000d));
        if (expandedWidth <= nativeWidth)
            return;

        _objectFrustumWidthAddress = address;
        _objectFrustumNativeWidth = nativeWidth;
        _objectFrustumAdjusted = true;
        m.WriteU32(address, expandedWidth);

        if (!_objectFrustumLogged)
        {
            _objectFrustumLogged = true;
            Console.Error.WriteLine(
                $"[V82WideObjectFrustum] native={nativeWidth} " +
                $"expanded={expandedWidth} scale={scale:F6}");
        }
    }

    public static void RestoreObjectFrustum(CpuContext c, IMemory m)
    {
        if (!_objectFrustumAdjusted)
            return;
        _objectFrustumAdjusted = false;
        Dispatcher.UnwrapMemory(m)
            .WriteU32(_objectFrustumWidthAddress, _objectFrustumNativeWidth);
    }

    public static void ExpandTerrainFrustum(CpuContext c, IMemory m)
    {
        _terrainFrustumAdjusted = false;
        if (!ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            GpuHle.WideAspect <= GpuHle.BaseAspect + 0.001f)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint address = c.GP + 0xEDCu;
        uint nativeWidth = m.ReadU32(address);
        if (nativeWidth == 0u || nativeWidth > 0x00100000u)
            return;

        double aspectScale =
            (double)GpuHle.WideAspect / GpuHle.BaseAspect;
        double scale = TerrainFrustumScaleOverride >= 1d
            ? TerrainFrustumScaleOverride
            : aspectScale;
        uint expandedWidth = checked((uint)Math.Clamp(
            Math.Round(
                nativeWidth * scale,
                MidpointRounding.AwayFromZero),
            1d,
            0x00100000d));
        if (expandedWidth <= nativeWidth)
            return;

        _terrainFrustumWidthAddress = address;
        _terrainFrustumNativeWidth = nativeWidth;
        _terrainFrustumAdjusted = true;
        m.WriteU32(address, expandedWidth);

        if (!_terrainFrustumLogged)
        {
            _terrainFrustumLogged = true;
            Console.Error.WriteLine(
                $"[V82WideTerrainFrustum] native={nativeWidth} " +
                $"expanded={expandedWidth} " +
                $"scale={scale:F6} aspect={GpuHle.WideAspect:F6} " +
                $"projection={m.ReadU32(c.GP + 0xED8u)} " +
                $"focal={m.ReadU16(c.GP + 0xDB4u)} " +
                $"height={m.ReadU32(c.GP + 0xF20u)}");
        }
    }

    static double ReadTerrainFrustumScaleOverride()
    {
        string? value = Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_TERRAIN_FRUSTUM_SCALE");
        return double.TryParse(
            value,
            System.Globalization.NumberStyles.Float,
            System.Globalization.CultureInfo.InvariantCulture,
            out double scale) &&
            double.IsFinite(scale) &&
            scale >= 1d &&
            scale <= 4d
                ? scale
                : 0d;
    }

    public static void RestoreTerrainFrustum(CpuContext c, IMemory m)
    {
        if (!_terrainFrustumAdjusted)
            return;

        m = Dispatcher.UnwrapMemory(m);
        m.WriteU32(
            _terrainFrustumWidthAddress,
            _terrainFrustumNativeWidth);
        _terrainFrustumAdjusted = false;
    }

    public static void TraceTerrainTraversalPolygon(
        CpuContext c,
        IMemory m)
    {
        bool traceFrame = TraceTerrainCells && GpuHle.GameplayActive;
        if ((!TraceTerrainTraversal || _terrainTraversalTraceCount >= 8) &&
            !traceFrame)
            return;

        m = Dispatcher.UnwrapMemory(m);
        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)(count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count <= 0 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        int minX = int.MaxValue;
        int maxX = int.MinValue;
        int minZ = int.MaxValue;
        int maxZ = int.MinValue;
        long sumX = 0;
        long sumZ = 0;
        int[] polygonX = new int[count];
        int[] polygonZ = new int[count];
        var detail = new System.Text.StringBuilder();
        for (int index = 0; index < count; index++)
        {
            int x = unchecked((int)m.ReadU32(points + (uint)(index * 8)));
            int z = unchecked((int)m.ReadU32(
                points + (uint)(index * 8 + 4)));
            polygonX[index] = x;
            polygonZ[index] = z;
            sumX += x;
            sumZ += z;
            minX = Math.Min(minX, x);
            maxX = Math.Max(maxX, x);
            minZ = Math.Min(minZ, z);
            maxZ = Math.Max(maxZ, z);
            if (index != 0)
                detail.Append(';');
            detail.Append(x).Append(',').Append(z);
        }

        long area2 = 0;
        for (int index = 0; index < count; index++)
        {
            int next = (index + 1) % count;
            area2 +=
                (long)polygonX[index] * polygonZ[next] -
                (long)polygonX[next] * polygonZ[index];
        }

        uint matrix = c.GP + 0xF28u;
        short m00 = unchecked((short)m.ReadU16(matrix));
        short m01 = unchecked((short)m.ReadU16(matrix + 2u));
        short m02 = unchecked((short)m.ReadU16(matrix + 4u));
        short m10 = unchecked((short)m.ReadU16(matrix + 6u));
        short m11 = unchecked((short)m.ReadU16(matrix + 8u));
        short m12 = unchecked((short)m.ReadU16(matrix + 10u));
        short m20 = unchecked((short)m.ReadU16(matrix + 12u));
        short m21 = unchecked((short)m.ReadU16(matrix + 14u));
        short m22 = unchecked((short)m.ReadU16(matrix + 16u));
        uint translation = c.GP + 0xF3Cu;
        int cameraXFixed = unchecked((int)m.ReadU32(translation));
        int cameraYFixed = unchecked((int)m.ReadU32(translation + 4u));
        int cameraZFixed = unchecked((int)m.ReadU32(translation + 8u));
        int cameraX = cameraXFixed / 256;
        int cameraY = cameraYFixed / 256;
        int cameraZ = cameraZFixed / 256;
        if (traceFrame)
            Console.Error.WriteLine(
                $"[V82TerrainPolygonFrame] tick={GpuHle.DebugGameplayTick} " +
                $"count={count} area2={area2} " +
                $"bounds-x={minX}..{maxX} bounds-z={minZ}..{maxZ} " +
                $"camera={cameraX},{cameraY},{cameraZ} " +
                $"matrix=[{m00},{m01},{m02};{m10},{m11},{m12};" +
                $"{m20},{m21},{m22}] points={detail}");

        if (!TraceTerrainTraversal || _terrainTraversalTraceCount >= 8)
            return;
        double centerX = (double)sumX / count;
        double centerZ = (double)sumZ / count;
        double cameraToCenterX = centerX - cameraX;
        double cameraToCenterZ = centerZ - cameraZ;
        double cameraToCenterLength = Math.Sqrt(
            cameraToCenterX * cameraToCenterX +
            cameraToCenterZ * cameraToCenterZ);
        double inferredRightX = cameraToCenterLength >= 1d
            ? cameraToCenterZ / cameraToCenterLength
            : 0d;
        double inferredRightZ = cameraToCenterLength >= 1d
            ? -cameraToCenterX / cameraToCenterLength
            : 0d;

        _terrainTraversalTraceCount++;
        Console.Error.WriteLine(
            $"[V82TerrainTraversalPolygon] count={count} " +
            $"bounds-x={minX}..{maxX} bounds-z={minZ}..{maxZ} " +
            $"center={centerX:F1},{centerZ:F1} " +
            $"camera={cameraX},{cameraY},{cameraZ} " +
            $"inferred-right={inferredRightX:F6},{inferredRightZ:F6} " +
            $"matrix=[{m00},{m01},{m02};{m10},{m11},{m12};" +
            $"{m20},{m21},{m22}] points={detail}");
    }

    static void CaptureRenderGroupEntry(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi)
            return;

        uint descriptor = c.A0;
        bool mapped = IsShapeAddress(descriptor, 0x1Cu);
        uint objectAddress = c.S0;
        bool objectMapped = IsShapeAddress(objectAddress, 0x6Cu);
        RenderGroupEntryHistory.Enqueue(new(
            descriptor,
            c.A1,
            c.A2,
            c.A3,
            c.RA,
            c.SP,
            c.S0,
            c.S1,
            c.S3,
            objectMapped ? m.ReadU32(objectAddress) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 4u) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0xCu) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0x10u) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0x18u) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0x40u) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0x5Cu) : 0u,
            objectMapped ? m.ReadU32(objectAddress + 0x68u) : 0u,
            mapped ? m.ReadU32(descriptor) : 0u,
            mapped ? m.ReadU32(descriptor + 4u) : 0u,
            mapped ? m.ReadU32(descriptor + 8u) : 0u,
            mapped ? m.ReadU32(descriptor + 0xCu) : 0u,
            mapped ? m.ReadU32(descriptor + 0x10u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x14u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x18u) : 0u));
        while (RenderGroupEntryHistory.Count > 64)
            RenderGroupEntryHistory.Dequeue();
    }

    static void DumpRenderGroupEntryHistory()
    {
        if (!TraceImportedOverlayAbi)
            return;

        Console.Error.WriteLine("[V82RenderGroupHistory] begin");
        foreach (RenderGroupEntryTrace item in RenderGroupEntryHistory)
        {
            Console.Error.WriteLine(
                $"[V82RenderGroupHistory] descriptor=0x{item.Descriptor:X8} " +
                $"transform=0x{item.Transform:X8} caller=0x{item.Caller:X8} " +
                $"sp=0x{item.Stack:X8} a2=0x{item.A2:X8} a3=0x{item.A3:X8} " +
                $"previous-s0=0x{item.PreviousS0:X8} " +
                $"previous-s1=0x{item.PreviousS1:X8} " +
                $"previous-s3=0x{item.PreviousS3:X8} object=" +
                $"{item.ObjectWord0:X8},{item.ObjectFlags:X8}," +
                $"{item.ObjectSibling:X8},{item.ObjectChild:X8}," +
                $"{item.ObjectIdTimer:X8},{item.ObjectModel:X8}," +
                $"{item.ObjectDatabase:X8},{item.ObjectDistanceModel:X8} " +
                $"words=" +
                $"{item.Word0:X8},{item.Word4:X8},{item.Word8:X8}," +
                $"{item.WordC:X8},{item.Word10:X8},{item.Word14:X8}," +
                $"{item.Word18:X8}");
        }
        Console.Error.WriteLine("[V82RenderGroupHistory] end");
    }

    public static void TraceNativeModelResolveBegin(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi)
            return;
        _ = m;
        NativeModelResolveScopes.Push((c.A0, c.A1 & 0xFFFFu, c.RA));
    }

    public static void TraceNativeModelResolveEnd(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi || NativeModelResolveScopes.Count == 0)
            return;
        m = Dispatcher.UnwrapMemory(m);
        (uint database, uint group, uint caller) = NativeModelResolveScopes.Pop();
        uint descriptor = c.V0;
        bool mapped = IsShapeAddress(descriptor, 0x1Cu);
        NativeModelResolveTrace trace = new(
            database,
            group,
            caller,
            descriptor,
            mapped ? m.ReadU32(descriptor) : 0u,
            mapped ? m.ReadU32(descriptor + 4u) : 0u,
            mapped ? m.ReadU32(descriptor + 8u) : 0u,
            mapped ? m.ReadU32(descriptor + 0xCu) : 0u,
            mapped ? m.ReadU32(descriptor + 0x10u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x14u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x18u) : 0u);
        if (descriptor != 0u)
        {
            if (ActiveNativeModels.TryGetValue(descriptor, out NativeModelResolveTrace previous))
            {
                Console.Error.WriteLine(
                    $"[NativeModelOwnership] allocator reused active descriptor " +
                    $"descriptor=0x{descriptor:X8} old-db=0x{previous.Database:X8} " +
                    $"old-group={previous.Group} old-caller=0x{previous.Caller:X8} " +
                    $"new-db=0x{database:X8} new-group={group} " +
                    $"new-caller=0x{caller:X8}");
            }
            ActiveNativeModels[descriptor] = trace;
        }
        NativeModelResolveHistory.Enqueue(trace);
        while (NativeModelResolveHistory.Count > 512)
            NativeModelResolveHistory.Dequeue();
    }

    public static void TraceNativeModelRelease(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi)
            return;
        m = Dispatcher.UnwrapMemory(m);
        uint descriptor = c.A0;
        bool mapped = IsShapeAddress(descriptor, 0x1Cu);
        bool hadActiveOwner = ActiveNativeModels.Remove(
            descriptor, out NativeModelResolveTrace owner);
        AssignedNativeModels.Remove(descriptor);
        NativeModelReleaseTrace release = new(
            descriptor,
            c.RA,
            hadActiveOwner,
            owner.Database,
            owner.Group,
            owner.Caller,
            mapped ? m.ReadU32(descriptor) : 0u,
            mapped ? m.ReadU32(descriptor + 4u) : 0u,
            mapped ? m.ReadU32(descriptor + 8u) : 0u,
            mapped ? m.ReadU32(descriptor + 0xCu) : 0u,
            mapped ? m.ReadU32(descriptor + 0x10u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x14u) : 0u,
            mapped ? m.ReadU32(descriptor + 0x18u) : 0u);
        if (descriptor != 0u)
            ReleasedNativeModels[descriptor] = release;
        NativeModelReleaseHistory.Enqueue(release);
        while (NativeModelReleaseHistory.Count > 512)
            NativeModelReleaseHistory.Dequeue();
    }

    static void DumpNativeModelOwnershipHistory()
    {
        if (!TraceImportedOverlayAbi)
            return;
        Console.Error.WriteLine("[NativeModelResolveHistory] begin");
        foreach (NativeModelResolveTrace item in NativeModelResolveHistory)
            Console.Error.WriteLine(
                $"[NativeModelResolveHistory] database=0x{item.Database:X8} " +
                $"group={item.Group} caller=0x{item.Caller:X8} " +
                $"descriptor=0x{item.Descriptor:X8} words=" +
                $"{item.Word0:X8},{item.Word4:X8},{item.Word8:X8}," +
                $"{item.WordC:X8},{item.Word10:X8},{item.Word14:X8}," +
                $"{item.Word18:X8}");
        Console.Error.WriteLine("[NativeModelResolveHistory] end");
        Console.Error.WriteLine("[NativeModelReleaseHistory] begin");
        foreach (NativeModelReleaseTrace item in NativeModelReleaseHistory)
            Console.Error.WriteLine(
                $"[NativeModelReleaseHistory] descriptor=0x{item.Descriptor:X8} " +
                $"caller=0x{item.Caller:X8} active={Convert.ToInt32(item.HadActiveOwner)} " +
                $"owner-db=0x{item.OwnerDatabase:X8} owner-group={item.OwnerGroup} " +
                $"owner-caller=0x{item.OwnerCaller:X8} words=" +
                $"{item.Word0:X8},{item.Word4:X8},{item.Word8:X8}," +
                $"{item.WordC:X8},{item.Word10:X8},{item.Word14:X8}," +
                $"{item.Word18:X8}");
        Console.Error.WriteLine("[NativeModelReleaseHistory] end");
    }

    static void DumpNativeModelOwner(IMemory m, uint descriptor)
    {
        bool mapped = IsShapeAddress(descriptor, 0x1Cu);
        if (AllAssignedNativeModels.TryGetValue(
                descriptor, out NativeObjectModelAssignment assignment))
        {
            Console.Error.WriteLine(
                $"[NativeObjectModelAssignmentHistory] descriptor=0x{descriptor:X8} " +
                $"object=0x{assignment.Object:X8} database=0x{assignment.Database:X8} " +
                $"record={assignment.RecordIndex} initial=" +
                $"{assignment.Word8:X8},{assignment.WordC:X8}," +
                $"{assignment.Word10:X8},{assignment.Word18:X8}");
        }
        else
        {
            Console.Error.WriteLine(
                $"[NativeObjectModelAssignmentHistory] descriptor=0x{descriptor:X8} " +
                $"assigned=0");
        }
        if (!ActiveNativeModels.TryGetValue(
                descriptor, out NativeModelResolveTrace owner))
        {
            if (ReleasedNativeModels.TryGetValue(
                    descriptor, out NativeModelReleaseTrace release))
            {
                Console.Error.WriteLine(
                    $"[NativeModelOwnership] descriptor=0x{descriptor:X8} " +
                    $"active=0 released=1 release-caller=0x{release.Caller:X8} " +
                    $"had-owner={Convert.ToInt32(release.HadActiveOwner)} " +
                    $"owner-db=0x{release.OwnerDatabase:X8} " +
                    $"owner-group={release.OwnerGroup} " +
                    $"owner-caller=0x{release.OwnerCaller:X8} " +
                    $"release-words={release.Word0:X8},{release.Word4:X8}," +
                    $"{release.Word8:X8},{release.WordC:X8}," +
                    $"{release.Word10:X8},{release.Word14:X8}," +
                    $"{release.Word18:X8}");
            }
            else
            {
                Console.Error.WriteLine(
                    $"[NativeModelOwnership] descriptor=0x{descriptor:X8} " +
                    $"active=0 released=0");
            }
            return;
        }

        Console.Error.WriteLine(
            $"[NativeModelOwnership] descriptor=0x{descriptor:X8} active=1 " +
            $"database=0x{owner.Database:X8} group={owner.Group} " +
            $"caller=0x{owner.Caller:X8} initial=" +
            $"{owner.Word0:X8},{owner.Word4:X8},{owner.Word8:X8}," +
            $"{owner.WordC:X8},{owner.Word10:X8},{owner.Word14:X8}," +
            $"{owner.Word18:X8} current=" +
            $"{(mapped ? m.ReadU32(descriptor) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 4u) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 8u) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 0xCu) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 0x10u) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 0x14u) : 0u):X8}," +
            $"{(mapped ? m.ReadU32(descriptor + 0x18u) : 0u):X8}");
    }

    public static void TraceNativeObjectModelAssigned(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi)
            return;
        m = Dispatcher.UnwrapMemory(m);
        uint objectAddress = c.S0;
        uint recordAddress = c.S1;
        uint database = c.S2;
        uint descriptor = c.V0;
        if (!IsShapeAddress(descriptor, 0x1Cu) ||
            !IsShapeAddress(database, 8u))
            return;
        uint firstRecord = m.ReadU32(database + 4u) + 0x1Cu;
        uint recordIndex = recordAddress >= firstRecord
            ? (recordAddress - firstRecord) / 0x1Cu
            : uint.MaxValue;
        NativeObjectModelAssignment assignment = new(
            objectAddress,
            database,
            recordIndex,
            descriptor,
            m.ReadU32(descriptor + 8u),
            m.ReadU32(descriptor + 0xCu),
            m.ReadU32(descriptor + 0x10u),
            m.ReadU32(descriptor + 0x18u));
        AssignedNativeModels[descriptor] = assignment;
        AllAssignedNativeModels[descriptor] = assignment;
        AssignedNativeModelsByObject[objectAddress] = assignment;
        if (recordIndex is 342 or 343 or 344 or 345 or 405 or 406)
            Console.Error.WriteLine(
                $"[NativeObjectModelAssigned] object=0x{objectAddress:X8} " +
                $"database=0x{database:X8} record={recordIndex} " +
                $"descriptor=0x{descriptor:X8} words=" +
                $"{m.ReadU32(descriptor):X8},{m.ReadU32(descriptor + 4u):X8}," +
                $"{m.ReadU32(descriptor + 8u):X8}," +
                $"{m.ReadU32(descriptor + 0xCu):X8}," +
                $"{m.ReadU32(descriptor + 0x10u):X8}," +
                $"{m.ReadU32(descriptor + 0x14u):X8}," +
                $"{m.ReadU32(descriptor + 0x18u):X8}");
    }

    static void AuditAssignedNativeModels(IMemory m, string trigger)
    {
        foreach (NativeObjectModelAssignment item in AssignedNativeModels.Values)
        {
            if (!IsShapeAddress(item.Descriptor, 0x1Cu))
                continue;
            uint word8 = m.ReadU32(item.Descriptor + 8u);
            uint wordC = m.ReadU32(item.Descriptor + 0xCu);
            uint word10 = m.ReadU32(item.Descriptor + 0x10u);
            uint word18 = m.ReadU32(item.Descriptor + 0x18u);
            if (word8 == item.Word8 && wordC == item.WordC &&
                word10 == item.Word10 && word18 == item.Word18)
                continue;
            Console.Error.WriteLine(
                $"[NativeObjectModelMutation] trigger={trigger} " +
                $"object=0x{item.Object:X8} database=0x{item.Database:X8} " +
                $"record={item.RecordIndex} descriptor=0x{item.Descriptor:X8} " +
                $"initial={item.Word8:X8},{item.WordC:X8}," +
                $"{item.Word10:X8},{item.Word18:X8} current=" +
                $"{word8:X8},{wordC:X8},{word10:X8},{word18:X8}");
            AssignedNativeModels.Remove(item.Descriptor);
            break;
        }
    }

    static void AuditAssignedNativeObjectModels(IMemory m, string trigger)
    {
        foreach (NativeObjectModelAssignment item in
                 AssignedNativeModelsByObject.Values)
        {
            bool trackedRecord = item.RecordIndex is >= 342 and <= 345 or
                >= 405 and <= 406;
            if (!trackedRecord || !IsShapeAddress(item.Object, 0x60u))
                continue;
            uint currentDescriptor = m.ReadU32(item.Object + 0x40u);
            if (currentDescriptor == item.Descriptor ||
                !ReportedNativeObjectModelMutations.Add(item.Object))
                continue;
            Console.Error.WriteLine(
                $"[NativeObjectModelPointerMutation] trigger={trigger} " +
                $"object=0x{item.Object:X8} database=0x{item.Database:X8} " +
                $"record={item.RecordIndex} initial=0x{item.Descriptor:X8} " +
                $"current=0x{currentDescriptor:X8} object-words=" +
                $"{m.ReadU32(item.Object):X8},{m.ReadU32(item.Object + 4u):X8}," +
                $"{m.ReadU32(item.Object + 0xCu):X8}," +
                $"{m.ReadU32(item.Object + 0x10u):X8}," +
                $"{m.ReadU32(item.Object + 0x14u):X8}," +
                $"{m.ReadU32(item.Object + 0x18u):X8}," +
                $"{m.ReadU32(item.Object + 0x40u):X8}," +
                $"{m.ReadU32(item.Object + 0x5Cu):X8}");
        }
    }

    public static void TraceNativeObjectExtent(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi)
            return;
        m = Dispatcher.UnwrapMemory(m);
        uint objectAddress = c.A0;
        bool mapped = IsShapeAddress(objectAddress, 0x60u);
        NativeObjectExtentHistory.Enqueue(new(
            objectAddress,
            c.RA,
            mapped ? m.ReadU32(objectAddress) : 0u,
            mapped ? m.ReadU32(objectAddress + 4u) : 0u,
            mapped ? m.ReadU32(objectAddress + 0xCu) : 0u,
            mapped ? m.ReadU32(objectAddress + 0x10u) : 0u,
            mapped ? m.ReadU32(objectAddress + 0x14u) : 0u,
            mapped ? m.ReadU32(objectAddress + 0x18u) : 0u,
            mapped ? m.ReadU32(objectAddress + 0x40u) : 0u,
            mapped ? m.ReadU32(objectAddress + 0x5Cu) : 0u));
        while (NativeObjectExtentHistory.Count > 256)
            NativeObjectExtentHistory.Dequeue();
        if (mapped)
            return;

        Console.Error.WriteLine(
            $"[NativeObjectExtent] invalid object=0x{objectAddress:X8} " +
            $"caller=0x{c.RA:X8}");
        Console.Error.WriteLine("[NativeObjectExtentHistory] begin");
        foreach (NativeObjectExtentTrace item in NativeObjectExtentHistory)
            Console.Error.WriteLine(
                $"[NativeObjectExtentHistory] object=0x{item.Object:X8} " +
                $"caller=0x{item.Caller:X8} callback=0x{item.Callback:X8} " +
                $"flags=0x{item.Flags:X8} sibling=0x{item.Sibling:X8} " +
                $"child=0x{item.Child:X8} parent=0x{item.Parent:X8} " +
                $"id-timer=0x{item.IdTimer:X8} model=0x{item.Model:X8} " +
                $"database=0x{item.Database:X8}");
        Console.Error.WriteLine("[NativeObjectExtentHistory] end");
    }

    public static void TraceNativeModelLifecycle(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi || _nativeModelLifecycleTraceCount++ >= 256)
            return;

        m = Dispatcher.UnwrapMemory(m);
        NativeModelLifecycleScopes.Push(new(c.A0, c.A1, c.A2 & 0xFFFFu));
        AuditAssignedNativeModels(
            m, $"lifecycle:{c.A2 & 0xFFFFu}:object:{c.A0:X8}");
        AuditAssignedNativeObjectModels(
            m, $"before-lifecycle:{c.A2 & 0xFFFFu}:object:{c.A0:X8}");

        uint objectAddress = c.A0;
        uint database = c.A1;
        uint modelIndex = c.A2 & 0xFFFFu;
        uint parent = c.A3;
        string objectWords = "unmapped";
        string databaseWords = "unmapped";
        string modelRecord = "unmapped";

        if (objectAddress >= 0x80010000u && objectAddress + 0x80u < PcHeapEnd)
        {
            objectWords =
                $"cb=0x{m.ReadU32(objectAddress):X8} " +
                $"flags=0x{m.ReadU32(objectAddress + 4u):X8} " +
                $"id={m.ReadU16(objectAddress + 0x1Au)} " +
                $"timer={m.ReadU16(objectAddress + 0x1Cu)} " +
                $"reload={m.ReadU16(objectAddress + 0x1Eu)} " +
                $"db=0x{m.ReadU32(objectAddress + 0x5Cu):X8} " +
                $"model=0x{m.ReadU32(objectAddress + 0x40u):X8}";
        }

        if (database >= 0x80010000u && database + 0x10u < PcHeapEnd)
        {
            uint records = m.ReadU32(database + 4u);
            databaseWords =
                $"w0=0x{m.ReadU32(database):X8} " +
                $"records=0x{records:X8} " +
                $"w8=0x{m.ReadU32(database + 8u):X8} " +
                $"wC=0x{m.ReadU32(database + 0xCu):X8}";
            // database+4 is the relocated BIN header, whose model table starts
            // after its seven 32-bit header fields.  Include that 0x1c-byte
            // prefix so this diagnostic reports the requested record rather
            // than the preceding slot.
            uint record = records + 0x1Cu + modelIndex * 0x1Cu;
            if (records >= 0x80010000u && record + 0x1Cu < PcHeapEnd)
            {
                modelRecord =
                    $"addr=0x{record:X8} " +
                    $"words={m.ReadU32(record):X8}," +
                    $"{m.ReadU32(record + 4u):X8}," +
                    $"{m.ReadU32(record + 8u):X8}," +
                    $"{m.ReadU32(record + 0xCu):X8}," +
                    $"{m.ReadU32(record + 0x10u):X8}," +
                    $"{m.ReadU32(record + 0x14u):X8}," +
                    $"{m.ReadU32(record + 0x18u):X8}";
            }
        }

        Console.Error.WriteLine(
            $"[NativeModelLifecycle] caller=0x{c.RA:X8} " +
            $"object=0x{objectAddress:X8} database=0x{database:X8} " +
            $"modelIndex={modelIndex} parent=0x{parent:X8} " +
            $"object[{objectWords}] database[{databaseWords}] " +
            $"record[{modelRecord}]");
    }

    public static void TraceNativeModelLifecycleEnd(CpuContext c, IMemory m)
    {
        if (!TraceImportedOverlayAbi || NativeModelLifecycleScopes.Count == 0)
            return;
        _ = c;
        m = Dispatcher.UnwrapMemory(m);
        NativeModelLifecycleScope scope = NativeModelLifecycleScopes.Pop();
        AuditAssignedNativeObjectModels(
            m,
            $"after-lifecycle:{scope.ModelIndex}:object:{scope.Object:X8}");
    }

    public static void ExpandTerrainTraversalLateral(
        CpuContext c,
        IMemory m)
    {
        if (TerrainOmnidirectional ||
            TerrainAspectPolygon ||
            TerrainLateralPaddingCells <= 0 ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint matrix = c.GP + 0xF28u;
        double rightX = unchecked((short)m.ReadU16(matrix));
        double rightZ = unchecked((short)m.ReadU16(matrix + 12u));
        double rightLength = Math.Sqrt(
            rightX * rightX +
            rightZ * rightZ);
        if (rightLength < 1d)
            return;
        rightX /= rightLength;
        rightZ /= rightLength;

        uint translation = c.GP + 0xF3Cu;
        double cameraX =
            unchecked((int)m.ReadU32(translation)) / 256d;
        double cameraZ =
            unchecked((int)m.ReadU32(translation + 8u)) / 256d;
        double padding = TerrainLateralPaddingCells * 1024d;
        int movedLeft = 0;
        int movedRight = 0;
        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            double side =
                (x - cameraX) * rightX +
                (z - cameraZ) * rightZ;
            if (Math.Abs(side) < 0.5d)
                continue;

            double direction = side > 0d ? 1d : -1d;
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    x + rightX * padding * direction,
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    z + rightZ * padding * direction,
                    MidpointRounding.AwayFromZero)));
            if (direction < 0d)
                movedLeft++;
            else
                movedRight++;
        }

        if (!_terrainLateralPaddingLogged)
        {
            _terrainLateralPaddingLogged = true;
            Console.Error.WriteLine(
                $"[V82WideTerrainLateral] " +
                $"cells={TerrainLateralPaddingCells} " +
                $"worldPadding={padding:F0} " +
                $"camera={cameraX:F1},{cameraZ:F1} " +
                $"right={rightX:F6},{rightZ:F6} " +
                $"moved-left={movedLeft} moved-right={movedRight} " +
                $"vertices={count}");
        }
    }

    public static void ExpandTerrainTraversalPolygon(
        CpuContext c,
        IMemory m)
    {
        if (TerrainOmnidirectional ||
            TerrainAspectPolygon ||
            TerrainPolygonPaddingCells <= 0 ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);
        long sumX = 0;
        long sumZ = 0;
        for (int index = 0; index < count; index++)
        {
            sumX += unchecked((int)m.ReadU32(
                points + (uint)(index * 8)));
            sumZ += unchecked((int)m.ReadU32(
                points + (uint)(index * 8 + 4)));
        }

        double centerX = (double)sumX / count;
        double centerZ = (double)sumZ / count;
        double padding = TerrainPolygonPaddingCells * 1024d;
        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            double dx = x - centerX;
            double dz = z - centerZ;
            double length = Math.Sqrt(dx * dx + dz * dz);
            if (length < 1d)
                continue;
            double scale = (length + padding) / length;
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    centerX + dx * scale,
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    centerZ + dz * scale,
                    MidpointRounding.AwayFromZero)));
        }

        if (!_terrainPolygonPaddingLogged)
        {
            _terrainPolygonPaddingLogged = true;
            Console.Error.WriteLine(
                $"[V82WideTerrainPolygon] cells={TerrainPolygonPaddingCells} " +
                $"worldPadding={padding:F0} center={centerX:F1},{centerZ:F1} " +
                $"vertices={count}");
        }
    }

    public static void ExpandTerrainTraversalOmnidirectional(
        CpuContext c,
        IMemory m)
    {
        if (!TerrainOmnidirectional ||
            !ConfigManager.View.HighResolution3D ||
            !GpuHle.GameplayActive)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint translation = c.GP + 0xF3Cu;
        double cameraX =
            unchecked((int)m.ReadU32(translation)) / 256d;
        double cameraZ =
            unchecked((int)m.ReadU32(translation + 8u)) / 256d;
        var original = new (double X, double Z)[count];
        double radius = 0d;
        double signedArea = 0d;
        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            original[index] = (x, z);
            double dx = x - cameraX;
            double dz = z - cameraZ;
            radius = Math.Max(radius, Math.Sqrt(dx * dx + dz * dz));
        }
        for (int index = 0; index < count; index++)
        {
            (double x0, double z0) = original[index];
            (double x1, double z1) = original[(index + 1) % count];
            signedArea += x0 * z1 - x1 * z0;
        }
        if (radius < 1024d)
            return;

        double direction = signedArea < 0d ? -1d : 1d;
        double startAngle = Math.Atan2(
            original[0].Z - cameraZ,
            original[0].X - cameraX);
        for (int index = 0; index < count; index++)
        {
            double angle =
                startAngle +
                direction * Math.Tau * index / count;
            uint point = points + (uint)(index * 8);
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    cameraX + Math.Cos(angle) * radius,
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    cameraZ + Math.Sin(angle) * radius,
                    MidpointRounding.AwayFromZero)));
        }

        if (!_terrainOmnidirectionalLogged)
        {
            _terrainOmnidirectionalLogged = true;
            Console.Error.WriteLine(
                $"[V82TerrainOmnidirectional] vertices={count} " +
                $"camera={cameraX:F1},{cameraZ:F1} radius={radius:F1}");
        }
    }

    /// <summary>
    /// Pushes terrain out to the distance the objects standing on it are
    /// already drawn at.
    ///
    /// Extended draw distance moves only the object limit (see
    /// <see cref="ExtendObjectDrawDistance"/>), so roadside buildings on
    /// Route 66 are drawn past the last terrain row and visibly hang in the
    /// sky with nothing underneath them.
    ///
    /// Two limits have to move together, because either one alone does
    /// nothing:
    ///
    /// * the traversal polygon, or `func_8001BECC` never walks that far.
    ///   Every vertex is scaled away from the camera vertex, which preserves
    ///   the polygon's shape and angles and changes only how far it reaches.
    /// * scratchpad `+0x98`, which `func_8001C158` fills with
    ///   `gp+0xDB6 &lt;&lt; 8` (10240 by default) just before calling the
    ///   walker. `func_800288E0` tests each cell corner's GTE SZ3 against it
    ///   and switches to broad untextured shaded packets when all four are
    ///   beyond it.
    /// * scratchpad `+0x9A`, which carries the terrain texture/detail limit.
    ///   The native engine changes submitted distant cells to broad
    ///   vertex-shaded packets beyond it.
    ///
    /// This is not a widescreen problem and is deliberately not gated on it.
    /// </summary>
    static double EffectiveTerrainRangeScale() =>
        TerrainRangeScaleOverride >= 1d
            ? TerrainRangeScaleOverride
            : ConfigManager.View.ExtendedDrawDistance
                ? DefaultTerrainRangeScale
                : 1d;

    public static void ScaleTerrainTraversalRange(CpuContext c, IMemory m)
    {
        if (TerrainOmnidirectional ||
            !GpuHle.GameplayActive)
            return;

        double scale = EffectiveTerrainRangeScale();
        if (scale <= 1.0001d)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint translation = c.GP + 0xF3Cu;
        double cameraX = unchecked((int)m.ReadU32(translation)) / 256d;
        double cameraZ = unchecked((int)m.ReadU32(translation + 8u)) / 256d;

        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    cameraX + (x - cameraX) * scale,
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    cameraZ + (z - cameraZ) * scale,
                    MidpointRounding.AwayFromZero)));
        }

        // Move the emit-stage far plane with the polygon. Left alone it keeps
        // rejecting everything the widened polygon adds.
        const uint farPlaneAddress = 0x1F800000u + 0x98u;
        uint nativeFarPlane = m.ReadU16(farPlaneAddress);
        uint farPlane = nativeFarPlane;
        if (nativeFarPlane > 0u)
        {
            farPlane = (uint)Math.Clamp(
                Math.Round(nativeFarPlane * scale, MidpointRounding.AwayFromZero),
                nativeFarPlane,
                60000d);
            m.WriteU16(farPlaneAddress, (ushort)farPlane);
        }

        if (!_terrainRangeLogged)
        {
            _terrainRangeLogged = true;
            Console.Error.WriteLine(
                $"[V82TerrainRange] scale={scale:F3} vertices={count} " +
                $"camera={cameraX:F1},{cameraZ:F1} " +
                $"farPlane={nativeFarPlane}->{farPlane}");
        }
    }

    /// <summary>
    /// Fits the completed terrain traversal polygon to the widened Enhanced
    /// horizontal field of view.
    ///
    /// func_8001C158 builds the polygon from four camera-space corner rays and
    /// then projects the result onto the world XZ plane. That projection is not
    /// the frustum's angular sector: a corner ray's vertical component rotates
    /// partly into the horizontal plane whenever the camera is pitched, so each
    /// far corner gains forward distance while its lateral offset stays at the
    /// authored half-width. The footprint corner therefore sits at a narrower
    /// apparent angle than the frustum edge it came from, and simply scaling
    /// gp+0xEDC by the aspect ratio - which is the correct angular widening -
    /// still leaves a wedge of unvisited terrain against the outer 16:9 edges.
    ///
    /// Restore the invariant directly: measure every vertex in the camera's
    /// horizontal basis and push it out until its lateral offset covers the
    /// widened half-angle at its own forward distance. Vertices that already
    /// satisfy the ratio, and the camera-origin vertex, are left untouched, so
    /// this only ever adds terrain and never moves the polygon forward, back,
    /// or across the camera.
    /// </summary>
    public static void ExpandTerrainTraversalWideFit(
        CpuContext c,
        IMemory m)
    {
        if (!TerrainWideFit ||
            TerrainOmnidirectional ||
            TerrainAspectPolygon ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            GpuHle.WideAspect <= GpuHle.BaseAspect + 0.001f)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);

        // gp+0xED8 is the projection distance the native camera shares with the
        // GTE H register, and gp+0xEDC is the authored horizontal extent.
        // ExpandTerrainFrustum has already replaced the latter for this
        // traversal, so prefer the value it saved.
        double projection = unchecked((int)m.ReadU32(c.GP + 0xED8u));
        double nativeWidth = _terrainFrustumNativeWidth != 0u
            ? _terrainFrustumNativeWidth
            : unchecked((int)m.ReadU32(c.GP + 0xEDCu));
        if (projection < 1d || nativeWidth < 1d)
            return;

        double halfWidth = nativeWidth * 0.5d;
        double wideHalfWidth =
            halfWidth + GpuHle.WideMargin((int)Math.Round(nativeWidth));
        double tangent = wideHalfWidth / projection;
        if (!double.IsFinite(tangent) || tangent <= 0d)
            return;

        uint matrix = c.GP + 0xF28u;
        double rightX = unchecked((short)m.ReadU16(matrix));
        double rightZ = unchecked((short)m.ReadU16(matrix + 12u));
        double forwardX = unchecked((short)m.ReadU16(matrix + 4u));
        double forwardZ = unchecked((short)m.ReadU16(matrix + 16u));
        double rightLength = Math.Sqrt(rightX * rightX + rightZ * rightZ);
        double forwardLength =
            Math.Sqrt(forwardX * forwardX + forwardZ * forwardZ);
        if (rightLength < 1d || forwardLength < 1d)
            return;
        rightX /= rightLength;
        rightZ /= rightLength;
        forwardX /= forwardLength;
        forwardZ /= forwardLength;

        uint translation = c.GP + 0xF3Cu;
        double cameraX = unchecked((int)m.ReadU32(translation)) / 256d;
        double cameraZ = unchecked((int)m.ReadU32(translation + 8u)) / 256d;

        // One terrain cell of slack absorbs the walker's inward per-row cell
        // rounding, which otherwise discards the partially covered boundary
        // cell along the whole outer edge.
        double margin = TerrainWideFitMarginCells * 1024d;

        // Work out one factor for the whole polygon rather than moving each
        // vertex to its own target. Per-vertex fitting is not a linear map: it
        // can reorder neighbouring vertices and leave the polygon non-convex
        // or wound the wrong way, and the walker then fills nothing for that
        // frame. That showed up as terrain flickering out while driving, which
        // no still capture and no windowed average can see - see
        // tools/recompone-v8-2/analyze_terrain_flicker.py. Scaling the lateral
        // component uniformly is linear, so convexity and winding are
        // preserved by construction.
        double factor = 1d;
        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double dx = unchecked((int)m.ReadU32(point)) - cameraX;
            double dz = unchecked((int)m.ReadU32(point + 4u)) - cameraZ;
            double forward = dx * forwardX + dz * forwardZ;
            double lateral = Math.Abs(dx * rightX + dz * rightZ);
            // Vertices near the view axis carry no lateral information and
            // would demand an unbounded factor.
            if (forward <= 0d || lateral < 1024d)
                continue;
            factor = Math.Max(factor, (forward * tangent + margin) / lateral);
        }
        factor = Math.Min(factor, MaximumTerrainWideFitFactor);
        if (factor <= 1.0001d)
            return;

        // The walker turns these into cell indices with an arithmetic shift and
        // walks rows between them. A vertex outside the authored grid makes it
        // select almost nothing for that frame, which is a total terrain
        // dropout rather than a smaller one, so keep every widened vertex
        // inside the grid.
        const double gridLimit = TerrainGridExtentCells * 1024d - 1d;
        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            double dx = x - cameraX;
            double dz = z - cameraZ;
            double forward = dx * forwardX + dz * forwardZ;
            double lateral = dx * rightX + dz * rightZ;
            double widened = lateral * factor;
            double newX = cameraX + forwardX * forward + rightX * widened;
            double newZ = cameraZ + forwardZ * forward + rightZ * widened;
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    Math.Clamp(newX, 0d, gridLimit),
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    Math.Clamp(newZ, 0d, gridLimit),
                    MidpointRounding.AwayFromZero)));
        }

        if (!_terrainWideFitLogged)
        {
            _terrainWideFitLogged = true;
            Console.Error.WriteLine(
                $"[V82WideTerrainFit] vertices={count} factor={factor:F4} " +
                $"tangent={tangent:F6} halfWidth={wideHalfWidth:F1} " +
                $"projection={projection:F0} marginCells=" +
                $"{TerrainWideFitMarginCells:F2} " +
                $"camera={cameraX:F1},{cameraZ:F1} " +
                $"forward={forwardX:F6},{forwardZ:F6} " +
                $"right={rightX:F6},{rightZ:F6}");
        }
    }

    public static void ExpandTerrainTraversalAspect(
        CpuContext c,
        IMemory m)
    {
        if (!TerrainAspectPolygon ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            GpuHle.WideAspect <= GpuHle.BaseAspect + 0.001f)
            return;

        int count = unchecked((int)c.A1);
        uint points = c.A0;
        uint byteCount = (uint)Math.Max(0, count * 8);
        bool retailRam =
            points >= 0x80000000u &&
            points <= 0x801FFFFFu - byteCount;
        bool scratchpad =
            points >= 0x1F800000u &&
            points <= 0x1F800400u - byteCount;
        if (count < 3 ||
            count > 32 ||
            (!retailRam && !scratchpad))
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint translation = c.GP + 0xF3Cu;
        double cameraX =
            unchecked((int)m.ReadU32(translation)) / 256d;
        double cameraZ =
            unchecked((int)m.ReadU32(translation + 8u)) / 256d;
        uint matrix = c.GP + 0xF28u;
        double rightX = unchecked((short)m.ReadU16(matrix));
        double rightZ = unchecked((short)m.ReadU16(matrix + 12u));
        double rightLength = Math.Sqrt(
            rightX * rightX + rightZ * rightZ);
        if (rightLength < 1d)
            return;
        rightX /= rightLength;
        rightZ /= rightLength;
        double aspectScale =
            (double)GpuHle.WideAspect / GpuHle.BaseAspect;

        for (int index = 0; index < count; index++)
        {
            uint point = points + (uint)(index * 8);
            double x = unchecked((int)m.ReadU32(point));
            double z = unchecked((int)m.ReadU32(point + 4u));
            double side =
                (x - cameraX) * rightX +
                (z - cameraZ) * rightZ;
            double extraSide = side * (aspectScale - 1d);
            m.WriteU32(
                point,
                unchecked((uint)(int)Math.Round(
                    x + rightX * extraSide,
                    MidpointRounding.AwayFromZero)));
            m.WriteU32(
                point + 4u,
                unchecked((uint)(int)Math.Round(
                    z + rightZ * extraSide,
                    MidpointRounding.AwayFromZero)));
        }

        if (!_terrainAspectPolygonLogged)
        {
            _terrainAspectPolygonLogged = true;
            Console.Error.WriteLine(
                $"[V82TerrainAspectPolygon] vertices={count} " +
                $"camera={cameraX:F1},{cameraZ:F1} " +
                $"right={rightX:F6},{rightZ:F6} " +
                $"scale={aspectScale:F6}");
        }
    }

    public static void ExpandTerrainRowSpan(CpuContext c, IMemory m)
    {
        if (TraceTerrainCells && GpuHle.GameplayActive)
            Console.Error.WriteLine(
                $"[V82TerrainRow] tick={GpuHle.DebugGameplayTick} " +
                $"start={c.A0} end={c.A1} row={c.A2} " +
                $"valid={(c.A1 > c.A0 ? 1 : 0)}");
        if (TerrainRowCellPadding <= 0 ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            c.A1 <= c.A0)
            return;

        const uint terrainGridExtent = 2048u;
        uint padding = (uint)TerrainRowCellPadding * 4u;
        uint originalStart = c.A0;
        uint originalEnd = c.A1;
        c.A0 = originalStart > padding
            ? originalStart - padding
            : 0u;
        c.A1 = Math.Min(terrainGridExtent, originalEnd + padding);

        if (!_terrainRowCellPaddingLogged)
        {
            _terrainRowCellPaddingLogged = true;
            Console.Error.WriteLine(
                $"[V82WideTerrainRows] cells={TerrainRowCellPadding} " +
                $"first={originalStart}..{originalEnd} " +
                $"expanded={c.A0}..{c.A1} row={c.A2}");
        }
    }

    public static void BeginTerrainRoutePacketWrites(
        CpuContext c,
        IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        GpuHle.ClearTerrainRouteColorRamp();
        GpuHle.BeginTerrainRoutePacketWrites();
        Gte.BeginTerrainProjection();
        _terrainCellsSubmitted++;
        // A2 is the packet cursor handed to func_800288E0; it returns the new
        // cursor in V0, which only advances when the cell emitted something.
        _terrainCellPacketCursor = c.A2;
        int frame = GpuHle.DebugGameplayTick;
        GpuHle.TerrainCellTextures textures = ReadTerrainCellTextures(c, m);
        TerrainCellScopes.Push(new TerrainCellScope(
            frame,
            "outer",
            c.A0,
            c.A1,
            c.A2,
            textures));
        if (!TraceTerrainCells)
            return;

        if (_terrainCellFrame is null || _terrainCellFrame.Frame != frame)
        {
            FlushTerrainCellFrame();
            _terrainCellFrame = new TerrainCellFrameStats { Frame = frame };
        }
    }

    public static void EndTerrainRoutePacketWrites(
        CpuContext c,
        IMemory m)
    {
        GpuHle.EndTerrainRoutePacketWrites();
        Gte.EndTerrainProjection();
        if (c.V0 > _terrainCellPacketCursor)
            _terrainCellsEmitted++;
        if (TerrainCellScopes.Count == 0)
            return;

        TerrainCellScope scope = TerrainCellScopes.Pop();
        if (!TraceTerrainCells)
            return;

        TerrainCellFrameStats? stats = _terrainCellFrame;
        if (stats is null || stats.Frame != scope.Frame)
            return;

        uint packetEnd = c.V0;
        bool emitted = packetEnd > scope.PacketStart;
        stats.Calls++;
        stats.MinX = Math.Min(stats.MinX, scope.X);
        stats.MaxX = Math.Max(stats.MaxX, scope.X);
        stats.MinZ = Math.Min(stats.MinZ, scope.Z);
        stats.MaxZ = Math.Max(stats.MaxZ, scope.Z);
        if (!emitted)
        {
            if (stats.Rejected.Count < 24)
                stats.Rejected.Add($"{scope.X},{scope.Z}");
            return;
        }

        stats.Emitted++;
        stats.PacketBytes += packetEnd - scope.PacketStart;
        stats.EmittedMinX = Math.Min(stats.EmittedMinX, scope.X);
        stats.EmittedMaxX = Math.Max(stats.EmittedMaxX, scope.X);
        stats.EmittedMinZ = Math.Min(stats.EmittedMinZ, scope.Z);
        stats.EmittedMaxZ = Math.Max(stats.EmittedMaxZ, scope.Z);
        GpuHle.RegisterPacketOwnerRange(
            scope.PacketStart,
            packetEnd,
            $"terrain-cell={scope.X},{scope.Z},frame={scope.Frame}");
    }

    static GpuHle.TerrainCellTextures ReadTerrainCellTextures(
        CpuContext c,
        IMemory m)
    {
        const uint terrainPageTable = 0x800B93F0u;
        uint x = c.A0;
        uint z = c.A1;
        if (x >= 2048u || z >= 2048u)
            return default;

        uint block = terrainPageTable +
            ((((x >> 6) << 5) + (z >> 6)) << 2);
        uint page = m.ReadU32(block + 0x80u);
        if (!IsShapeAddress(page, 0x4042u))
            return default;

        uint local = ((x & 0x3Fu) << 6) + (z & 0x3Fu);
        uint textureGrid = page + 0x2000u + local;
        ushort clut = m.ReadU16(c.GP + 0xDA8u);
        return ReadTerrainTextureGrid(m, textureGrid, clut, 4);
    }

    public static void BeginTerrainDetailPacketWrites(
        CpuContext c,
        IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint localBytes = c.S0 - c.S4;
        GpuHle.TerrainCellTextures textures = default;
        uint local = 0;
        if (localBytes < 0x2000u && (localBytes & 1u) == 0u)
        {
            local = localBytes >> 1;
            uint textureGrid = c.S4 + 0x2000u + local;
            ushort clut = m.ReadU16(c.GP + 0xDA8u);
            textures = ReadTerrainTextureGrid(m, textureGrid, clut, 2);
        }

        string source = TerrainCellScopes.TryPeek(out TerrainCellScope parent)
            ? $"detail@{parent.X},{parent.Z}"
            : "detail";
        TerrainCellScopes.Push(new TerrainCellScope(
            GpuHle.DebugGameplayTick,
            source,
            local >> 6,
            local & 0x3Fu,
            c.A2,
            textures));
    }

    public static void EndTerrainDetailPacketWrites(
        CpuContext c,
        IMemory m)
    {
        if (TerrainCellScopes.Count == 0)
            return;

        TerrainCellScopes.Pop();
    }

    public static void BeginTerrainTransitionPacketWrites(
        CpuContext c,
        IMemory m)
    {
        TerrainCellScope terrain = TerrainCellScopes.TryPeek(
            out TerrainCellScope active)
            ? active
            : default;
        TerrainTransitionScopes.Push(new TerrainTransitionScope(
            c.A2,
            c.A3,
            terrain));
    }

    public static void EndTerrainTransitionPacketWrites(
        CpuContext c,
        IMemory m)
    {
        if (TerrainTransitionScopes.Count == 0)
            return;

        TerrainTransitionScope scope = TerrainTransitionScopes.Pop();
        if (!scope.Terrain.Textures.Valid || c.A2 <= scope.PacketStart)
            return;

        m = Dispatcher.UnwrapMemory(m);
        for (uint packet = scope.PacketStart;
             packet + 0x1Cu <= c.A2;
             packet += 0x1Cu)
        {
            uint header = m.ReadU32(packet);
            byte command = (byte)(m.ReadU32(packet + 4u) >> 24);
            if ((header >> 24) != 6u || command is < 0x30 or > 0x33)
                continue;
            bool foundA = TryFindTerrainTransitionVertex(
                m, scope, packet + 4u, packet + 8u,
                out byte ax, out byte az);
            bool foundB = TryFindTerrainTransitionVertex(
                m, scope, packet + 0xCu, packet + 0x10u,
                out byte bx, out byte bz);
            bool foundC = TryFindTerrainTransitionVertex(
                m, scope, packet + 0x14u, packet + 0x18u,
                out byte cx, out byte cz);
            if (!foundA || !foundB || !foundC)
                continue;

            GpuHle.RegisterTerrainTransitionPacket(
                packet,
                new GpuHle.TerrainTransitionPacket(
                    scope.Terrain.Textures,
                    ax, az,
                    bx, bz,
                    cx, cz,
                    scope.Terrain.Source,
                    scope.Terrain.X,
                    scope.Terrain.Z));
        }
    }

    static bool TryFindTerrainTransitionVertex(
        IMemory m,
        in TerrainTransitionScope scope,
        uint packetColor,
        uint packetPosition,
        out byte x,
        out byte z)
    {
        uint position = m.ReadU32(packetPosition);
        uint color = m.ReadU32(packetColor) & 0x00FFFFFFu;
        int gridSize = scope.Terrain.Textures.GridSize;
        int positionMatch = -1;
        for (int index = 0; index < 25; index++)
        {
            int localX = index % 5;
            int localZ = index / 5;
            if (localX > gridSize || localZ > gridSize)
                continue;
            uint record = scope.VertexRecords + (uint)(index * 0x10);
            if (m.ReadU32(record + 8u) != position)
                continue;
            positionMatch = index;
            if ((m.ReadU32(record + 0xCu) & 0x00FFFFFFu) == color)
                break;
        }

        if (positionMatch < 0)
        {
            x = z = 0;
            return false;
        }

        x = (byte)(positionMatch % 5);
        z = (byte)(positionMatch / 5);
        return true;
    }

    public static void TagFirstCoarseTerrainPacket(
        CpuContext c,
        IMemory m) =>
        TagCoarseTerrainPacket(c.A2, m, secondHalf: false);

    public static void TagSecondCoarseTerrainPacket(
        CpuContext c,
        IMemory m) =>
        TagCoarseTerrainPacket(c.A2, m, secondHalf: true);

    static void TagCoarseTerrainPacket(
        uint packet,
        IMemory m,
        bool secondHalf)
    {
        if (!TerrainCellScopes.TryPeek(out TerrainCellScope scope) ||
            !scope.Textures.Valid)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint header = m.ReadU32(packet);
        byte command = (byte)(m.ReadU32(packet + 4u) >> 24);
        if ((header >> 24) != 6u || command is < 0x30 or > 0x33)
            return;

        GpuHle.RegisterCoarseTerrainPacket(
            packet,
            scope.Textures,
            secondHalf,
            scope.Source,
            scope.X,
            scope.Z);
    }

    static GpuHle.TerrainCellTextures ReadTerrainTextureGrid(
        IMemory m,
        uint textureGrid,
        ushort clut,
        int gridSize)
    {
        const uint terrainTextureTable = 0x800B7270u;
        var tiles = new GpuHle.TerrainTextureDescriptor[gridSize * gridSize];
        for (int x = 0; x < gridSize; x++)
        {
            for (int z = 0; z < gridSize; z++)
            {
                byte textureId = m.ReadU8(
                    textureGrid + (uint)(x * 0x40 + z));
                uint descriptor =
                    terrainTextureTable + (uint)textureId * 0x20u;
                tiles[x * gridSize + z] =
                    new GpuHle.TerrainTextureDescriptor(
                        textureId,
                        m.ReadU16(descriptor),
                        m.ReadU16(descriptor + 4u),
                        m.ReadU16(descriptor + 8u),
                        m.ReadU16(descriptor + 0xCu),
                        m.ReadU16(descriptor + 6u),
                        clut,
                        m.ReadU8(descriptor + 0x1Fu),
                        m.ReadU8(descriptor + 0x1Cu),
                        m.ReadU8(descriptor + 0x1Du),
                        m.ReadU8(descriptor + 0x1Eu));
            }
        }
        return new GpuHle.TerrainCellTextures(tiles, gridSize, true);
    }

    static void FlushTerrainCellFrame()
    {
        TerrainCellFrameStats? stats = _terrainCellFrame;
        if (stats is null || stats.Calls == 0 || _terrainCellFramesLogged >= 120)
            return;

        _terrainCellFramesLogged++;
        string submittedBounds = stats.MinX == uint.MaxValue
            ? "none"
            : $"{stats.MinX}..{stats.MaxX},{stats.MinZ}..{stats.MaxZ}";
        string emittedBounds = stats.EmittedMinX == uint.MaxValue
            ? "none"
            : $"{stats.EmittedMinX}..{stats.EmittedMaxX}," +
              $"{stats.EmittedMinZ}..{stats.EmittedMaxZ}";
        Console.Error.WriteLine(
            $"[V82TerrainCells] frame={stats.Frame} " +
            $"submitted={stats.Calls} emitted={stats.Emitted} " +
            $"packetBytes={stats.PacketBytes} " +
            $"submittedBounds={submittedBounds} " +
            $"emittedBounds={emittedBounds} " +
            $"rejectedSample={string.Join(';', stats.Rejected)}");
    }

    /// <summary>
    /// Widens the per-object visibility test to the widescreen field of view.
    ///
    /// func_8002E22C tests an object against three planes built for the
    /// authored 4:3 frustum: each plane distance must be under the object's
    /// bounding radius. Widescreen shows a third more view to each side, so an
    /// object whose centre has left the 4:3 frustum is still on screen, and
    /// the engine culls it - it vanishes at the left or right edge. The effect
    /// is worst up close, where an object covers a lot of screen while its
    /// centre is already outside.
    ///
    /// The planes themselves are rebuilt every frame by func_8002DFF0, so this
    /// grants the test the equivalent slack instead: the extra lateral reach a
    /// widened frustum has at the object's own distance. Objects are only ever
    /// kept, never dropped, and the far cull is unaffected.
    /// </summary>
    public static void WidenObjectVisibilityTest(CpuContext c, IMemory m)
    {
        RecordObjectVisibilityInputs(c, m);
        if (!WideObjectCull ||
            !ConfigManager.View.HighResolution3D ||
            !ConfigManager.View.Widescreen ||
            !GpuHle.GameplayActive ||
            GpuHle.WideAspect <= GpuHle.BaseAspect + 0.001f)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint position = c.A0;
        if (position < 0x80000000u || position > 0x807FFFF0u)
            return;

        double dx = unchecked((int)m.ReadU32(position)) -
            unchecked((int)m.ReadU32(c.GP + 0xF3Cu));
        double dy = unchecked((int)m.ReadU32(position + 4u)) -
            unchecked((int)m.ReadU32(c.GP + 0xF40u));
        double dz = unchecked((int)m.ReadU32(position + 8u)) -
            unchecked((int)m.ReadU32(c.GP + 0xF44u));
        double distance = Math.Sqrt(dx * dx + dy * dy + dz * dz);
        if (!double.IsFinite(distance) || distance <= 0d)
            return;

        // Lateral reach of the widened frustum minus the authored one, per
        // unit of distance, taken from the same half-width and projection the
        // terrain traversal uses.
        double projection = unchecked((int)m.ReadU32(c.GP + 0xED8u));
        double nativeWidth = _terrainFrustumNativeWidth != 0u
            ? _terrainFrustumNativeWidth
            : unchecked((int)m.ReadU32(c.GP + 0xEDCu));
        if (projection < 1d || nativeWidth < 1d)
            return;

        double halfWidth = nativeWidth * 0.5d;
        double wideHalfWidth =
            halfWidth + GpuHle.WideMargin((int)Math.Round(nativeWidth));
        // The planes measure perpendicular distance, not lateral offset, so
        // the extra reach has to be divided by the cosine of the frustum's
        // half angle. Measured against the objects that were still being
        // rejected: the required slack per unit of distance is 0.221 to 0.253,
        // where the uncorrected figure gives 0.211 and this gives 0.249.
        // The 1.05 keeps the marginal cases - every one that still failed was
        // short by only a few percent.
        double planeScale =
            Math.Sqrt(halfWidth * halfWidth + projection * projection) /
            projection;
        double slack =
            (wideHalfWidth - halfWidth) / projection * distance *
            planeScale * ObjectCullSlackScale;
        if (slack <= 0d)
            return;

        long widened = c.A1 + (long)Math.Round(
            slack, MidpointRounding.AwayFromZero);
        // The callee shifts this right by 8 and compares it against saturated
        // GTE output, so keep it inside that range.
        c.A1 = (uint)Math.Clamp(widened, 0L, 0x007FFFFFL);

        if (!_objectVisibilityLogged)
        {
            _objectVisibilityLogged = true;
            Console.Error.WriteLine(
                $"[V82WideObjectCull] halfWidth={halfWidth:F1}->" +
                $"{wideHalfWidth:F1} projection={projection:F0} " +
                $"slackPerUnit={(wideHalfWidth - halfWidth) / projection:F4}");
        }
    }

    /// <summary>
    /// Extends how far arena objects remain visible - but only as far as the
    /// ground beneath them is drawn.
    ///
    /// The retail object cull distance is paired with the terrain walker's
    /// reach. Extending objects on their own makes arena props stand past the
    /// last terrain row with nothing under them: on Route 66 the roadside
    /// diner and Super Donuts stand visibly hang in the sky. Terrain cannot be
    /// pushed out to meet them - the walker stops selecting new cells beyond
    /// roughly twice its stock range whatever the traversal polygon says - so
    /// the two are kept locked together instead.
    /// </summary>
    public static void ExtendObjectDrawDistance(CpuContext c, IMemory m)
    {
        if (!ConfigManager.View.ExtendedDrawDistance ||
            EffectiveTerrainRangeScale() <= 1.0001d ||
            c.RA != 0x8002DA34u)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint distance = m.ReadU32(c.SP + 0x2Cu);
        if (distance > 0x003FFFFFu && distance <= 0x007FFFFFu)
            m.WriteU32(c.SP + 0x2Cu, 0x003FFFFFu);
    }

    static void ApplyLevelOfDetail(uint objectAddress, IMemory m)
    {
        uint highMesh = m.ReadU32(objectAddress + 0x40u);
        uint lowMesh = m.ReadU32(objectAddress + 0x68u);
        uint threshold = m.ReadU32(objectAddress + 0x6Cu);
        bool maximum = IsMaximumLevelOfDetail();

        if (maximum)
        {
            if (threshold != 0u)
            {
                LodThresholds[objectAddress] = (highMesh, lowMesh, threshold);
                if (!_maximumLodLogged)
                {
                    _maximumLodLogged = true;
                    Console.Error.WriteLine(
                        $"[V82LOD] Maximum active; forcing high-detail geometry " +
                        $"object=0x{objectAddress:X8} threshold=0x{threshold:X8}");
                }
            }
            m.WriteU32(objectAddress + 0x6Cu, 0u);
            return;
        }

        if (threshold == 0u &&
            LodThresholds.TryGetValue(objectAddress, out var stock) &&
            stock.HighMesh == highMesh &&
            stock.LowMesh == lowMesh)
        {
            m.WriteU32(objectAddress + 0x6Cu, stock.Threshold);
            if (!_stockLodRestoreLogged)
            {
                _stockLodRestoreLogged = true;
                Console.Error.WriteLine(
                    "[V82LOD] Stock active; restored distance-based geometry thresholds");
            }
        }
    }

    static bool IsMaximumLevelOfDetail()
    {
        bool maximum = ConfigManager.View.LevelOfDetail.Equals(
            "Maximum", StringComparison.OrdinalIgnoreCase);
        string? environmentMode =
            Environment.GetEnvironmentVariable("RECOMPONE_V82_LOD") ??
            Environment.GetEnvironmentVariable("RECOMPONE_LOD_MODE");
        if (!string.IsNullOrWhiteSpace(environmentMode))
        {
            maximum = environmentMode.Equals("maximum", StringComparison.OrdinalIgnoreCase) ||
                      environmentMode.Equals("max", StringComparison.OrdinalIgnoreCase) ||
                      environmentMode.Equals("1", StringComparison.OrdinalIgnoreCase) ||
                      environmentMode.Equals("true", StringComparison.OrdinalIgnoreCase);
        }
        return maximum;
    }

    static uint ExpandedPrimitiveBase(uint buffer) =>
        ExpandedPrimitiveBufferBase +
        (buffer & 1u) * ExpandedPrimitiveBufferSize;

    // func_80014B3C derives a packet high-water mark from the retail arena
    // before flipping buffers. Preserve the equivalent host-arena count so its
    // bookkeeping remains meaningful when the post-hook redirects the cursor.
    public static void PrepareExpandedPrimitiveBuffer(CpuContext c, IMemory m)
    {
        if (Runtime.Mode != RunMode.Devkit ||
            !GpuHle.GameplayActive ||
            !IsMaximumLevelOfDetail())
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint buffer = m.ReadU32(c.GP + 0x20u) & 1u;
        uint cursor = m.ReadU32(c.GP + 0x610u);
        uint expandedBase = ExpandedPrimitiveBase(buffer);
        uint nativeBase = 0x80074A68u + (buffer << 17);
        _previousPrimitiveHighWaterWords = m.ReadU32(c.GP + 0xCE4u);
        _previousPrimitiveUsedWords =
            cursor >= expandedBase &&
            cursor <= expandedBase + ExpandedPrimitiveBufferSize
                ? (cursor - expandedBase) >> 2
                : cursor >= nativeBase && cursor <= nativeBase + 0x20000u
                    ? (cursor - nativeBase) >> 2
                    : 0u;
    }

    // Redirect both the packet cursor and its end pointer after the retail
    // buffer-flip function clears the ordering table and retires objects tied
    // to the buffer being reused.
    public static void ActivateExpandedPrimitiveBuffer(CpuContext c, IMemory m)
    {
        if (Runtime.Mode != RunMode.Devkit)
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint buffer = m.ReadU32(c.GP + 0x20u) & 1u;
        if (!GpuHle.GameplayActive || !IsMaximumLevelOfDetail())
        {
            uint nativeBase = 0x80074A68u + (buffer << 17);
            GpuHle.BeginPacketArena(nativeBase, nativeBase + 0x20000u);
            return;
        }

        uint expandedBase = ExpandedPrimitiveBase(buffer);
        GpuHle.BeginPacketArena(
            expandedBase,
            expandedBase + ExpandedPrimitiveBufferSize);
        m.WriteU32(c.GP + 0x610u, expandedBase);
        m.WriteU32(c.GP + 0xCDCu, expandedBase + ExpandedPrimitiveBufferSize);
        m.WriteU32(
            c.GP + 0xCE4u,
            Math.Max(_previousPrimitiveHighWaterWords, _previousPrimitiveUsedWords));
        _expandedPrimitiveBuffersActive = true;

        if (!_expandedPrimitiveBuffersLogged)
        {
            _expandedPrimitiveBuffersLogged = true;
            Console.Error.WriteLine(
                $"[V82LOD] Maximum packet arenas active: " +
                $"0x{ExpandedPrimitiveBufferBase:X8}-0x{PcHeapBase:X8} " +
                $"({ExpandedPrimitiveBufferSize >> 10} KiB each)");
        }
    }

    static void RepairTerrainSlot(
        IMemory m, uint table, uint xBlock, uint zBlock)
    {
        uint slot = table + xBlock * 0x80u + zBlock * 4u;
        uint tile = m.ReadU32(slot);
        if (IsDevkitRamRange(tile, 0x3000u))
            return;

        uint replacement = 0u;
        uint replacementX = 0u;
        uint replacementZ = 0u;
        for (int radius = 1; radius < 32 && replacement == 0u; radius++)
        {
            for (int dx = -radius; dx <= radius && replacement == 0u; dx++)
            {
                int dzAbs = radius - Math.Abs(dx);
                foreach (int dz in dzAbs == 0 ? new[] { 0 } : new[] { -dzAbs, dzAbs })
                {
                    int bx = (int)xBlock + dx;
                    int bz = (int)zBlock + dz;
                    if (bx is < 0 or >= 32 || bz is < 0 or >= 32)
                        continue;
                    uint candidate = m.ReadU32(
                        table + (uint)bx * 0x80u + (uint)bz * 4u);
                    if (!IsDevkitRamRange(candidate, 0x3000u))
                        continue;
                    replacement = candidate;
                    replacementX = (uint)bx;
                    replacementZ = (uint)bz;
                    break;
                }
            }
        }

        if (replacement == 0u)
            throw new InvalidOperationException(
                $"Vigilante 8: 2nd Offense has no valid terrain chunks: " +
                $"query={xBlock},{zBlock} slot=0x{slot:X8} tile=0x{tile:X8}");

        m.WriteU32(slot, replacement);
        Console.Error.WriteLine(
            $"[V82Physics] repaired terrain chunk {xBlock},{zBlock} " +
            $"slot=0x{slot:X8} invalid=0x{tile:X8} " +
            $"from={replacementX},{replacementZ} tile=0x{replacement:X8}");
    }

    static bool IsDevkitRamRange(uint address, uint length)
    {
        if ((address & 0x80000000u) == 0u)
            return false;
        uint physical = address & 0x1FFFFFFFu;
        return length <= 0x00800000u &&
               physical <= 0x00800000u - length;
    }

    public static void TraceCommonObjectLoadPre(CpuContext c, IMemory m)
    {
        uint requestedMask = c.A0;
        for (int player = 0; player < 2; player++)
        {
            int selectedPlayerType = player == 0 && _soakPlayerType >= 0
                ? _soakPlayerType
                : V82VehicleRegistry.SelectedTypeForPlayer(player);
            if (selectedPlayerType < 0)
                continue;
            if (selectedPlayerType >= V82VehicleRegistry.RetailVehicleCount &&
                !V82VehicleRegistry.IsCustomType((uint)selectedPlayerType))
                throw new InvalidOperationException(
                    $"requested vehicle type {selectedPlayerType} is not registered");
            m.WriteU8(
                c.GP + 0x1104u + (uint)player,
                (byte)selectedPlayerType);
            Console.Error.WriteLine(
                $"[V82Vehicles] player={player + 1} type={selectedPlayerType}");
        }
        int primaryParticipants = (sbyte)m.ReadU8(c.GP + 0x31u) < 9 ? 2 : 4;
        V82VehicleRegistry.ApplySelectedNpcTypes(
            m, c.GP + 0x1104u, primaryParticipants);
        // func_800132CC builds the two resource masks in S3/S2 after several
        // split callbacks. Those callbacks can leak their callee-saved scratch
        // registers into the generated caller. Rebuild the masks from the same
        // six retail participant bytes and status table instead of preloading
        // every vehicle bank, which exhausts the 320-wide gameplay texture
        // page and leaves the local vehicle/HUD textures unavailable.
        uint commonMask = 0x001C0000u;
        uint alternateMask = 0u;
        var participantTypes = new List<int>(6);
        for (int index = 0; index < 6; index++)
        {
            int type = (sbyte)m.ReadU8(c.GP + 0x1104u + (uint)index);
            participantTypes.Add(type);
            if (type < 0)
                continue;
            // A registered default replacement owns type 0 end-to-end.
            // Loading COMMON's retail type-0 bank as well only duplicates its
            // model and textures, consuming the VRAM required by transformed
            // forms. The normal roster shell remains available independently.
            if (type == 0 && V82VehicleRegistry.HasDefaultReplacement)
                continue;
            if (type >= 21)
            {
                int alternateType = type - 21;
                if (alternateType < 32)
                    alternateMask |= 1u << alternateType;
                continue;
            }

            if (index < primaryParticipants)
            {
                uint status = 0x8006B9E8u + (uint)(index * 188 + type * 10);
                bool useAlternate =
                    m.ReadU8(status + 6u) >= 100 &&
                    m.ReadU8(status + 7u) >= 100 &&
                    m.ReadU8(status + 8u) >= 100 &&
                    m.ReadU8(status + 9u) >= 100;
                if (useAlternate)
                    alternateMask |= 1u << type;
                else
                    commonMask |= 1u << type;
            }
            else if (m.ReadU8(0x8006B802u + (uint)index) != 0u ||
                     (sbyte)m.ReadU8(c.GP + 0x31u) == 3)
            {
                commonMask |= 1u << type;
            }
        }

        c.A0 = commonMask;
        c.S3 = commonMask;
        c.S2 = alternateMask;
        CommonObjectMasks.Push(c.A0);
        Console.Error.WriteLine(
            $"[V82Object] rebuilt participant masks common=0x{commonMask:X8} " +
            $"alternate=0x{alternateMask:X8} leaked=0x{requestedMask:X8} " +
            $"types={string.Join(',', participantTypes)}");
    }

    public static void TraceCommonObjectLoadPost(CpuContext c, IMemory m)
    {
        uint mask = CommonObjectMasks.Count != 0 ? CommonObjectMasks.Pop() : 0u;
        var loaded = new List<string>();
        for (int index = 0; index < 32; index++)
        {
            uint pointer = m.ReadU32(0x800C6178u + (uint)index * 4u);
            if (pointer != 0u || (mask & (1u << index)) != 0u)
                loaded.Add($"{index}:0x{pointer:X8}");
        }
        Console.Error.WriteLine(
            $"[V82Object] COMMON slots after mask 0x{mask:X8}: {string.Join(' ', loaded)}");
    }

    public static bool TraceVehicleCreateRequest(CpuContext c, IMemory m)
    {
        // Variant -1 identifies the local player's pre-runtime vehicle for
        // every roster type. Restricting this to type 0 loses automation and
        // diagnostics as soon as the player selects another vehicle.
        if (c.A2 == 0xFFFFFFFFu)
        {
            _playerVehicle = c.A0;
        }
        VehicleFactorySources.Add(c.A0);
        uint requestedType = c.A1;
        if (c.A2 != 0xFFFFFFFFu &&
            requestedType < 21u &&
            m.ReadU32(0x800C6178u + requestedType * 4u) == 0u)
        {
            for (uint fallback = 1u; fallback < 21u; fallback++)
            {
                if (m.ReadU32(0x800C6178u + fallback * 4u) == 0u)
                    continue;
                c.A1 = fallback;
                Console.Error.WriteLine(
                    $"[V82Object] remapped unavailable enemy type " +
                    $"{requestedType} to loaded type {fallback}");
                break;
            }
        }
        Console.Error.WriteLine(
            $"[V82Object] vehicle request source=0x{c.A0:X8} " +
            $"type={(int)c.A1} variant={(int)c.A2}");
        return V82VehicleRegistry.CreateVehicle(c, m);
    }

    static void ResetRendererObjectTracking()
    {
        VehicleFactorySources.Clear();
        VehicleObjects.Clear();
        ObjectRenderScopes.Clear();
        TracedRenderObjects.Clear();
        _rendererOwnershipTraceCount = 0;
    }

    // The pre-game gate polls the retail pad callback without VSync while it
    // waits for Start. Advance one host frame for that call site so live and
    // scripted input can change, while leaving ordinary pad polling untouched.
    public static void ServiceMatchStartPadWait(CpuContext c, IMemory m)
    {
        if (c.RA == 0x800139ECu)
        {
            // The static callback path used immediately before this gate
            // returns through split continuations and leaks callee-saved
            // scratch values. Re-establish the arena loop's explicit state
            // before it stores those registers into persistent globals.
            byte mode = m.ReadU8(c.GP + 0x31u);
            if (!_matchGateRepairLogged)
            {
                _matchGateRepairLogged = true;
                Console.Error.WriteLine(
                    $"[V82Gameplay] repairing pre-match callee state mode={mode} " +
                    $"s3=0x{c.S3:X8} s4=0x{c.S4:X8} " +
                    $"s6=0x{c.S6:X8} s7=0x{c.S7:X8}");
            }
            c.S3 = mode == 4 ? 0x708u : 0x78u;
            c.S4 = mode is 0 or 7 ? 0x201u : 0u;
            c.S6 = 0u;
            c.S7 = 1u;
            Runtime.PresentFrame();
        }
    }

    // The retail object list is null-terminated. A malformed callback can
    // accidentally splice an already-visited node back into the active list;
    // on hardware this freezes the match before its first rendered frame.
    // Preserve the scheduler's callback ABI while checking that invariant.
    public static void RunObjectScheduler(CpuContext c, IMemory m)
    {
        ClampAttachedWeaponAmmo(m);

        uint schedulerArgument = c.A0;
        uint entrySp = c.SP;
        uint savedRa = c.RA;
        uint savedS0 = c.S0;
        uint savedS1 = c.S1;
        if (_objectSchedulerPass == 0 && c.RA == 0x80013B64u)
        {
            uint catchUpTicks = m.ReadU32(c.SP + 0x18u);
            Console.Error.WriteLine(
                $"[V82Scheduler] first state gp=0x{c.GP:X8} " +
                $"buffer={m.ReadU32(c.GP + 0x24u)} " +
                $"renderBase=0x{m.ReadU32(c.GP + 0xCE0u):X8} " +
                $"primitive=0x{m.ReadU32(c.GP + 0x610u):X8}");
            if (catchUpTicks > 2u)
            {
                // VSync runs during the long static boot/load path, so the
                // retail first-frame delta includes thousands of menu ticks.
                // Do not replay that stale time through vehicle physics.
                m.WriteU32(c.SP + 0x18u, 2u);
                Console.Error.WriteLine(
                    $"[V82Scheduler] capped first-frame catch-up " +
                    $"from {catchUpTicks} to 2 ticks");
            }
            if (c.S7 != 1u)
            {
                Console.Error.WriteLine(
                    $"[V82Scheduler] repaired arena viewport index " +
                    $"from 0x{c.S7:X8} to 1");
                c.S7 = 1u;
            }
        }
        uint savedS2 = c.S2;
        uint savedS3 = c.S3;
        uint savedS4 = c.S4;
        uint savedS5 = c.S5;
        uint savedS6 = c.S6;
        uint savedS7 = c.S7;
        uint savedFp = c.FP;
        uint savedGp = c.GP;

        // Match the original function's stack frame. Object callbacks use the
        // caller-owned ABI area above SP; omitting this frame lets them trample
        // func_800132CC's live locals.
        c.SP -= 0x10u;
        m.WriteU32(c.SP + 0x18u, savedRa);
        m.WriteU32(c.SP + 0x14u, savedS1);
        m.WriteU32(c.SP + 0x10u, savedS0);

        uint head = m.ReadU32(c.GP + 0x1088u);
        uint node = m.ReadU32(head);
        var visited = new HashSet<uint>();
        int callbacks = 0;
        while (node != 0u)
        {
            if (!visited.Add(node))
            {
                Console.Error.WriteLine(
                    $"[V82Scheduler] broke cycle pass={_objectSchedulerPass + 1} " +
                    $"node=0x{node:X8} callbacks={callbacks}");
                break;
            }
            if (++callbacks > 100_000)
                throw new InvalidOperationException(
                    "Vigilante 8: 2nd Offense object scheduler exceeded 100000 callbacks");

            uint objectAddress = m.ReadU32(node + 8u);
            if (objectAddress != 0u)
            {
                uint callback = m.ReadU32(objectAddress);
                if (callback != 0u)
                {
                    uint resolvedCallback =
                        Dispatcher.ResolveOwnedFunction(objectAddress, callback);
                    bool traceObject =
                        objectAddress == TraceObjectAddress &&
                        _traceObjectCount++ < 256;
                    if (traceObject)
                    {
                        Console.Error.WriteLine(
                            $"[V82ObjectTrace] pass={_objectSchedulerPass + 1} " +
                            $"gameplay={_gameplayFrameCount} node=0x{node:X8} " +
                            $"object=0x{objectAddress:X8} callback=0x{callback:X8} " +
                            $"resolved=0x{resolvedCallback:X8} " +
                            $"kind={m.ReadU8(objectAddress + 8u)} " +
                            $"subkind={unchecked((sbyte)m.ReadU8(objectAddress + 9u))} " +
                            $"id={(short)m.ReadU16(objectAddress + 0xAu)} " +
                            $"flags=0x{m.ReadU32(objectAddress + 4u):X8} " +
                            $"timer=0x{m.ReadU16(objectAddress + 0x1Cu):X4} " +
                            $"model=0x{m.ReadU32(objectAddress + 0x40u):X8} " +
                            $"database=0x{m.ReadU32(objectAddress + 0x5Cu):X8} " +
                            $"aux64=0x{m.ReadU32(objectAddress + 0x64u):X8} " +
                            $"aux68=0x{m.ReadU32(objectAddress + 0x68u):X8} " +
                            $"aux6C=0x{m.ReadU32(objectAddress + 0x6Cu):X8} " +
                            $"pos=({unchecked((int)m.ReadU32(objectAddress + 0x34u))}," +
                            $"{unchecked((int)m.ReadU32(objectAddress + 0x38u))}," +
                            $"{unchecked((int)m.ReadU32(objectAddress + 0x3Cu))})");
                    }
                    if (_soakEnabled && SoakCallbacks.Count < 256 &&
                        SoakCallbacks.Add(callback))
                    {
                        Console.Error.WriteLine(
                            $"[V82Coverage] callback=0x{callback:X8} " +
                            $"object=0x{objectAddress:X8} kind={m.ReadU8(objectAddress + 8u)} " +
                            $"id={(short)m.ReadU16(objectAddress + 0xAu)} " +
                            $"unique={SoakCallbacks.Count}");
                    }
                    uint callbackSp = c.SP;
                    uint callbackFp = c.FP;
                    uint callbackGp = c.GP;
                    uint callbackS0 = c.S0;
                    uint callbackS1 = c.S1;
                    uint callbackS2 = c.S2;
                    uint callbackS3 = c.S3;
                    uint callbackS4 = c.S4;
                    uint callbackS5 = c.S5;
                    uint callbackS6 = c.S6;
                    uint callbackS7 = c.S7;
                    uint primitiveBefore = m.ReadU32(c.GP + 0x610u);
                    c.A0 = objectAddress;
                    c.A1 = 0u;
                    c.A2 = schedulerArgument;
                    c.RA = 0x80031418u;
                    Dispatcher.Call(c, m, resolvedCallback);
                    c.SP = callbackSp;
                    c.FP = callbackFp;
                    c.GP = callbackGp;
                    c.S0 = callbackS0;
                    c.S1 = callbackS1;
                    c.S2 = callbackS2;
                    c.S3 = callbackS3;
                    c.S4 = callbackS4;
                    c.S5 = callbackS5;
                    c.S6 = callbackS6;
                    c.S7 = callbackS7;
                    uint primitiveAfter = m.ReadU32(c.GP + 0x610u);
                    if (traceObject)
                    {
                        Console.Error.WriteLine(
                            $"[V82ObjectTrace] pass={_objectSchedulerPass + 1} " +
                            $"object=0x{objectAddress:X8} " +
                            $"packets=0x{primitiveBefore:X8}..0x{primitiveAfter:X8} " +
                            $"callbackAfter=0x{m.ReadU32(objectAddress):X8} " +
                            $"timerAfter=0x{m.ReadU16(objectAddress + 0x1Cu):X4}");
                    }
                    uint primitiveBuffer = m.ReadU32(c.GP + 0x20u) & 1u;
                    bool expandedPrimitives =
                        Runtime.Mode == RunMode.Devkit &&
                        GpuHle.GameplayActive &&
                        _expandedPrimitiveBuffersActive &&
                        IsMaximumLevelOfDetail();
                    uint primitiveBase = expandedPrimitives
                        ? ExpandedPrimitiveBase(primitiveBuffer)
                        : 0x80074A68u + (primitiveBuffer << 17);
                    uint primitiveSize = expandedPrimitives
                        ? ExpandedPrimitiveBufferSize
                        : 0x20000u;
                    if (primitiveAfter < primitiveBase ||
                        primitiveAfter >= primitiveBase + primitiveSize)
                    {
                        uint restored = primitiveBefore >= primitiveBase &&
                            primitiveBefore < primitiveBase + primitiveSize
                            ? primitiveBefore
                            : primitiveBase;
                        Console.Error.WriteLine(
                            $"[V82Render] rejected primitive cursor 0x{primitiveAfter:X8} " +
                            $"from callback=0x{callback:X8} object=0x{objectAddress:X8}; " +
                            $"restored 0x{restored:X8} range=0x{primitiveBase:X8}-" +
                            $"0x{primitiveBase + primitiveSize:X8}");
                        m.WriteU32(c.GP + 0x610u, restored);
                    }
                }
            }
            node = m.ReadU32(node);
        }
        c.RA = savedRa;
        c.S1 = savedS1;
        c.S0 = savedS0;
        c.S2 = savedS2;
        c.S3 = savedS3;
        c.S4 = savedS4;
        c.S5 = savedS5;
        c.S6 = savedS6;
        c.S7 = savedS7;
        c.FP = savedFp;
        c.GP = savedGp;
        c.SP = entrySp;

        int pass = ++_objectSchedulerPass;
        if (pass <= 3 || pass % 600 == 0)
            Console.Error.WriteLine(
                $"[V82Scheduler] pass={pass} callbacks={callbacks}");

        // The last simulation pass in an arena frame carries the nonzero tick
        // count. Retail VSync continues asynchronously after this pass; the
        // static host must advance it explicitly so input, pacing and GPU
        // presentation do not stall while simulation runs unbounded.
        if (schedulerArgument != 0u)
        {
            int frame = ++_gameplayFrameCount;
            Hle.GpuHle.DebugGameplayTick = frame;
            if (frame == 1)
                InputManager.SignalScriptStage("gameplay", captureDelayPolls: 300);
            UpdateDefeatRegression(c, m, frame);
            UpdateImportedWaterDrowning(c, m, frame);
            UpdateWaterLifecycle(c, m, frame);
            UpdateSoak(c, m, frame);
            if (frame <= 3 || frame % 60 == 0)
            {
                uint player = _playerVehicle;
                string motion = player >= PcHeapBase && player < PcHeapEnd - 0x98u
                    ? $" pos=({(int)m.ReadU32(player + 0x34u)}," +
                      $"{(int)m.ReadU32(player + 0x38u)}," +
                      $"{(int)m.ReadU32(player + 0x3Cu)})" +
                      $" motion=({(int)m.ReadU32(player + 0x74u)}," +
                      $"{(int)m.ReadU32(player + 0x78u)}," +
                      $"{(int)m.ReadU32(player + 0x7Cu)})"
                    : string.Empty;
                Console.Error.WriteLine(
                    $"[V82Gameplay] frame={frame} ticks={schedulerArgument} " +
                    $"clock={m.ReadU32(c.GP + 0x28u)} player=0x{player:X8}{motion}");
            }
            Runtime.PresentFrame();
        }
    }

    static void UpdateDefeatRegression(CpuContext c, IMemory m, int frame)
    {
        if (_testDefeatFrame == 0 || _testDefeatInjected ||
            frame < _testDefeatFrame)
            return;

        uint player = _playerVehicle != 0u
            ? _playerVehicle
            : m.ReadU32(0x8006BB58u);
        if (player < 0x80010000u || player >= PcHeapEnd - 0x200u)
            throw new InvalidOperationException(
                $"Defeat regression could not resolve player object " +
                $"at frame {frame}: 0x{player:X8}");

        byte kind = m.ReadU8(player + 8u);
        if (m.ReadU32(player) == 0u || kind != 2)
            throw new InvalidOperationException(
                $"Defeat regression found unexpected player object " +
                $"0x{player:X8}: callback=0x{m.ReadU32(player):X8} " +
                $"kind={kind}");

        // Route the fixture through the same lethal-damage function used by
        // retail collisions and weapons.  Directly zeroing the 0x1C fields is
        // insufficient: it bypasses func_80038C40/func_80038870, so the object
        // never becomes a wreck and the match never enters its defeat flow.
        //
        // func_80039DCC(vehicle, signedDamage, hitPosition, fullDestroy) owns
        // zone depletion, the explosion, callback transition, match counters,
        // and result scheduling.  A large negative hit is intentionally used
        // only by this opt-in regression fixture.
        _testDefeatInjected = true;
        uint callbackBefore = m.ReadU32(player);
        ushort healthBefore = m.ReadU16(player + 0x1Cu);
        uint result = CallGameFunction(
            c, m, 0x80039DCCu,
            player,
            unchecked((uint)-0x7FFF),
            player + 0x34u,
            1u);
        uint callbackAfter = m.ReadU32(player);
        ushort healthAfter = m.ReadU16(player + 0x1Cu);
        const uint destroyedVehicleCallback = 0x800384A4u;
        if (callbackAfter != destroyedVehicleCallback || healthAfter != 0)
            throw new InvalidOperationException(
                $"Native lethal-damage path did not destroy player " +
                $"0x{player:X8} at frame {frame}: " +
                $"callback=0x{callbackBefore:X8}->0x{callbackAfter:X8} " +
                $"health={healthBefore}->{healthAfter} result={result}");
        Console.Error.WriteLine(
            $"[V82DefeatRegression] native lethal damage completed " +
            $"frame={frame} player=0x{player:X8} " +
            $"callback=0x{callbackBefore:X8}->0x{callbackAfter:X8} " +
            $"health={healthBefore}->{healthAfter} result={result}; " +
            $"awaiting retail defeat flow");
        InputManager.SignalScriptStage("defeated", captureDelayPolls: 180);
    }

    static int? ReadOptionalInt(string name)
    {
        return int.TryParse(
            Environment.GetEnvironmentVariable(name),
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int value)
            ? value
            : null;
    }

    static void UpdateImportedWaterDrowning(CpuContext c, IMemory m, int frame)
    {
        // Original-V8's 0x0043/-1 RECT is a drowning volume. V8:2 has the
        // XWAT renderer and buoyancy physics but no region-driven drowning
        // equivalent (its stock Bayou 0x8043/1 region leaves the vehicle
        // floating). Converted arenas retain 0x0043/-1 as a data-driven
        // extension marker, so this applies to every imported arena without
        // consulting a map identity, path, slot, or object name.
        uint node = m.ReadU32(c.GP + 0x10D8u);
        var regions = new List<(int X, int Z, int Width, int Height)>();
        for (int count = 0; count < 256; count++)
        {
            if (node < 0x80010000u || node >= PcHeapEnd - 0x20u)
                break;
            uint next = m.ReadU32(node);
            if (next == 0u)
                break;
            ushort attribute = m.ReadU16(node + 8u);
            short selector = unchecked((short)m.ReadU16(node + 0xAu));
            if (attribute == 0x0043u && selector == -1)
            {
                regions.Add((
                    unchecked((short)m.ReadU16(node + 0xCu)),
                    unchecked((short)m.ReadU16(node + 0xEu)),
                    m.ReadU16(node + 0x10u),
                    m.ReadU16(node + 0x12u)));
            }
            node = next;
        }
        if (regions.Count == 0)
        {
            ImportedWaterDwell.Clear();
            return;
        }

        int plane = unchecked((int)m.ReadU32(c.GP + 0xDB0u));
        var candidates = new HashSet<uint>(VehicleObjects);
        if (_playerVehicle != 0u)
            candidates.Add(_playerVehicle);
        var active = new HashSet<uint>();
        foreach (uint vehicle in candidates)
        {
            uint callback = vehicle >= 0x80010000u && vehicle < PcHeapEnd - 0x200u
                ? m.ReadU32(vehicle)
                : 0u;
            if (vehicle < 0x80010000u || vehicle >= PcHeapEnd - 0x200u ||
                m.ReadU8(vehicle + 8u) != 2 || m.ReadU16(vehicle + 0x1Cu) == 0 ||
                callback == 0x800384A4u)
                continue;
            int x = unchecked((int)m.ReadU32(vehicle + 0x34u)) >> 16;
            int y = unchecked((int)m.ReadU32(vehicle + 0x38u));
            int z = unchecked((int)m.ReadU32(vehicle + 0x3Cu)) >> 16;
            bool inside = regions.Any(region =>
                x >= region.X && x <= region.X + region.Width &&
                z >= region.Z && z <= region.Z + region.Height);
            // Y grows downward. Start the source-authored drowning timer once
            // the vehicle reaches the water surface, rather than while it is
            // merely driving through the rectangle above the water.
            if (!inside || y < plane - 0x2000)
            {
                ImportedWaterDwell.Remove(vehicle);
                continue;
            }

            active.Add(vehicle);
            int dwell = ImportedWaterDwell.TryGetValue(vehicle, out int prior)
                ? prior + 1
                : 1;
            ImportedWaterDwell[vehicle] = dwell;
            int depth = y - plane;
            if (TraceImportedWater && (dwell <= 10 || dwell % 30 == 0 ||
                depth >= ImportedWaterDrownDepth))
                Console.Error.WriteLine(
                    $"[V82ConvertedWater] frame={frame} vehicle=0x{vehicle:X8} " +
                    $"region=({x},{z}) y={y} plane={plane} " +
                    $"depth={depth}/{ImportedWaterDrownDepth} " +
                    $"dwell={dwell}/{ImportedWaterDrownFrames}");
            if (dwell < ImportedWaterDrownFrames &&
                depth < ImportedWaterDrownDepth)
                continue;

            uint callbackBefore = m.ReadU32(vehicle);
            ushort healthBefore = m.ReadU16(vehicle + 0x1Cu);
            uint result = CallGameFunction(
                c, m, 0x80039DCCu,
                vehicle,
                unchecked((uint)-0x7FFF),
                vehicle + 0x34u,
                1u);
            Console.Error.WriteLine(
                $"[V82ConvertedWater] drowned frame={frame} " +
                $"vehicle=0x{vehicle:X8} dwell={dwell} " +
                $"depth={depth} " +
                $"callback=0x{callbackBefore:X8}->0x{m.ReadU32(vehicle):X8} " +
                $"health={healthBefore}->{m.ReadU16(vehicle + 0x1Cu)} result={result}");
            ImportedWaterDwell.Remove(vehicle);
        }
        foreach (uint vehicle in ImportedWaterDwell.Keys.ToArray())
        {
            if (!active.Contains(vehicle))
                ImportedWaterDwell.Remove(vehicle);
        }
    }

    static void UpdateWaterLifecycle(CpuContext c, IMemory m, int frame)
    {
        bool configured = _testWaterFrame > 0 &&
            _testWaterX.HasValue && _testWaterY.HasValue &&
            _testWaterZ.HasValue;
        if (!_testWaterConfigLogged &&
            Environment.GetEnvironmentVariable(
                "RECOMPONE_V82_TEST_WATER_FRAME") is not null)
        {
            _testWaterConfigLogged = true;
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] config frame={_testWaterFrame} " +
                $"x={_testWaterX?.ToString() ?? "missing"} " +
                $"y={_testWaterY?.ToString() ?? "missing"} " +
                $"z={_testWaterZ?.ToString() ?? "missing"} " +
                $"timeout={_testWaterTimeout} configured={(configured ? 1 : 0)}");
        }
        if (!configured || _testWaterRespawned)
            return;

        uint player = _playerVehicle != 0u
            ? _playerVehicle
            : m.ReadU32(0x8006BB58u);
        bool validPlayer = player >= 0x80010000u &&
            player < PcHeapEnd - 0x200u &&
            m.ReadU8(player + 8u) == 2;
        if (!_testWaterInjected)
        {
            if (frame < _testWaterFrame)
                return;
            if (!validPlayer)
            {
                Console.Error.WriteLine(
                    $"[V82WaterLifecycle] waiting frame={frame} " +
                    $"player=0x{player:X8}");
                return;
            }

            _testWaterInjected = true;
            _testWaterInjectedFrame = frame;
            _testWaterInitialPlayer = player;
            _testWaterInitialCallback = m.ReadU32(player);
            _testWaterInitialHealth = m.ReadU16(player + 0x1Cu);
            m.WriteU32(player + 0x34u, unchecked((uint)_testWaterX!.Value));
            m.WriteU32(player + 0x38u, unchecked((uint)_testWaterY!.Value));
            m.WriteU32(player + 0x3Cu, unchecked((uint)_testWaterZ!.Value));
            m.WriteU32(player + 0x74u, 0u);
            m.WriteU32(player + 0x78u, 0u);
            m.WriteU32(player + 0x7Cu, 0u);
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] injected frame={frame} " +
                $"player=0x{player:X8} callback=0x{_testWaterInitialCallback:X8} " +
                $"health={_testWaterInitialHealth} " +
                $"pos=({_testWaterX.Value},{_testWaterY.Value},{_testWaterZ.Value}) " +
                $"plane={unchecked((int)m.ReadU32(c.GP + 0xDB0u))}");
        }

        player = _playerVehicle != 0u ? _playerVehicle : player;
        validPlayer = player >= 0x80010000u &&
            player < PcHeapEnd - 0x200u;
        if (!validPlayer)
        {
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] frame={frame} player=0x{player:X8} invalid=1");
            return;
        }

        uint callback = m.ReadU32(player);
        ushort health = m.ReadU16(player + 0x1Cu);
        int x = unchecked((int)m.ReadU32(player + 0x34u));
        int y = unchecked((int)m.ReadU32(player + 0x38u));
        int z = unchecked((int)m.ReadU32(player + 0x3Cu));
        int plane = unchecked((int)m.ReadU32(c.GP + 0xDB0u));
        Console.Error.WriteLine(
            $"[V82WaterLifecycle] frame={frame} player=0x{player:X8} " +
            $"callback=0x{callback:X8} kind={m.ReadU8(player + 8u)} " +
            $"health={health} flags=0x{m.ReadU32(player + 4u):X8} " +
            $"timer=0x{m.ReadU16(player + 0x1Eu):X4} " +
            $"pos=({x},{y},{z}) plane={plane} deltaY={y - plane} " +
            $"motion=({unchecked((int)m.ReadU32(player + 0x74u))}," +
            $"{unchecked((int)m.ReadU32(player + 0x78u))}," +
            $"{unchecked((int)m.ReadU32(player + 0x7Cu))})");

        if (!_testWaterDestroyed &&
            (callback == 0x800384A4u || health == 0))
        {
            _testWaterDestroyed = true;
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] destroyed frame={frame} " +
                $"elapsed={frame - _testWaterInjectedFrame} " +
                $"player=0x{player:X8} callback=0x{callback:X8} health={health}");
        }
        if (_testWaterDestroyed &&
            health > 0 && callback == _testWaterInitialCallback &&
            (player != _testWaterInitialPlayer ||
             Math.Abs(y - _testWaterY!.Value) > 0x10000))
        {
            _testWaterRespawned = true;
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] respawned frame={frame} " +
                $"elapsed={frame - _testWaterInjectedFrame} " +
                $"player=0x{player:X8} callback=0x{callback:X8} health={health}");
        }
        if (!_testWaterDestroyed &&
            frame - _testWaterInjectedFrame == _testWaterTimeout)
        {
            Console.Error.WriteLine(
                $"[V82WaterLifecycle] timeout frame={frame} " +
                $"elapsed={_testWaterTimeout} destroyed=0 respawned=0");
        }
    }

    static void UpdateSoak(CpuContext c, IMemory m, int frame)
    {
        if (!_soakEnabled) return;

        // The original-level switch is normally set by swapping to the first
        // game's disc from the pause menu. The crash harness owns both discs,
        // so expose the same retail flag without mutating memory-card data.
        m.WriteU16(0x8006A832u, (ushort)(m.ReadU16(0x8006A832u) | 0x10u));

        uint player = _playerVehicle != 0u
            ? _playerVehicle
            : m.ReadU32(0x8006BB58u);
        if (frame == 1)
        {
            Console.Error.WriteLine(
                $"[V82Soak] begin player=0x{player:X8} old-levels=0x{m.ReadU16(0x8006A832u):X4}");
            if (player >= PcHeapBase && player < PcHeapEnd - 0x200u)
            {
                var words = new List<string>();
                for (uint offset = 0u; offset < 0x200u; offset += 4u)
                    words.Add($"{offset:X3}:{m.ReadU32(player + offset):X8}");
                Console.Error.WriteLine(
                    $"[V82Soak] player-words {string.Join(' ', words)}");
            }
        }

        // Cycle ordinary controls rather than teleporting or altering vehicle
        // dynamics. The isolated power-up profile remains parked so its three
        // transformation captures are visually comparable and cannot overlap
        // incidental weapon/pickup events.
        MaintainSoakVehicle(m, player, frame);
        if (_soakPowerUpsEnabled)
            UpdateSoakPowerUps(c, m, player, frame);
        if (_soakWeaponsEnabled)
            UpdateSoakWeapon(c, m, player, frame);
        TraceSoakPowerState(m, player, frame);
        if (_soakPowerUpsEnabled && _soakTransformCaptures)
        {
            int captureMode = frame switch
            {
                980 => 1,
                1580 => 2,
                2180 => 3,
                _ => 0,
            };
            if (captureMode != 0)
                HostWindow.RequestDisplayCapture($"transform_{captureMode}");
        }
        int phase = (frame / 180) % 8;
        ushort movement = _soakWeaponsEnabled
            ? (ushort)(Controller.Cross |
                (phase is 0 or 1 or 6 ? Controller.Left :
                 phase is 3 or 4 or 7 ? Controller.Right : 0))
            : (ushort)0;
        ushort action = _soakWeaponsEnabled
            ? SoakWeaponCommandInput(frame)
            : (ushort)0;
        if (_soakWeaponsEnabled && action == 0)
        {
            action = (ushort)(((frame / 30) % 12) switch
            {
                0 => Controller.L2,
                2 => Controller.R2,
                4 => Controller.L1,
                6 => Controller.R1,
                8 => Controller.Circle,
                10 => Controller.Triangle,
                _ => 0,
            });
        }
        _soakAutomationInput =
            _soakNoAutoInput ? (ushort)0 : (ushort)(movement | action);
        _soakInputPhase = phase;

        if (_graphicsShowcaseCaptures && frame is 64 or 68 or 72 or 90 or 96)
            HostWindow.RequestDisplayCapture($"particle_{frame:0000}");

        if (frame == 1 || frame % _soakHeartbeatFrames == 0)
        {
            string state = player >= PcHeapBase && player < PcHeapEnd - 0x100u
                ? $" pos=({(int)m.ReadU32(player + 0x34u)}," +
                  $"{(int)m.ReadU32(player + 0x38u)}," +
                  $"{(int)m.ReadU32(player + 0x3Cu)})" +
                  $" motion=({(int)m.ReadU32(player + 0x74u)}," +
                  $"{(int)m.ReadU32(player + 0x78u)}," +
                  $"{(int)m.ReadU32(player + 0x7Cu)})"
                : string.Empty;
            Console.Error.WriteLine(
                $"[V82Soak] gameplay frame={frame} phase={_soakInputPhase} " +
                $"input=0x{_soakAutomationInput:X4} player=0x{player:X8}{state}");
        }

        if (!_soakTeardownSignaled &&
            _soakTeardownFrame > 0 &&
            frame >= _soakTeardownFrame)
        {
            _soakTeardownSignaled = true;
            _soakAutomationInput = 0;
            InputManager.SignalScriptStage("soak_teardown");
        }
    }

    static void UpdateSoakPowerUps(CpuContext c, IMemory m, uint player, int frame)
    {
        if (player < PcHeapBase || player >= PcHeapEnd - 0x134u ||
            m.ReadU8(player + 8u) != 2)
            return;

        // Exercise each retail pickup effect without teleporting the vehicle or
        // bypassing its simulation. These are the exact action paths used by
        // pickup callback 0x80049D54 for item kinds 0, 2, 3, 4, 7, 8 and 9.
        if (!_soakRepairWrenchCovered && frame >= 30 && frame <= 180 &&
            frame % 30 == 0)
        {
            ushort before = m.ReadU16(player + 0x1Cu);
            ushort damaged = (ushort)Math.Max(0, before - 64);
            m.WriteU16(player + 0x1Cu, damaged);
            CallGameFunction(c, m, 0x8003A0C0u, player, 500u);
            _soakRepairWrenchCovered = true;
            Console.Error.WriteLine(
                $"[V82Coverage] powerup=repair-wrench frame={frame} " +
                $"health={damaged}->{m.ReadU16(player + 0x1Cu)}");
        }
        if (frame == 120)
        {
            m.WriteU16(player + 0x132u, 900);
            Console.Error.WriteLine(
                $"[V82Coverage] powerup=radar-jammer frame={frame} timer=900");
        }
        if (frame == 420)
        {
            m.WriteU16(player + 0x12Eu, 900);
            Console.Error.WriteLine(
                $"[V82Coverage] powerup=weapon-upgrade frame={frame} timer=900");
        }
        if (frame == 720)
        {
            m.WriteU16(player + 0x130u, 900);
            Console.Error.WriteLine(
                $"[V82Coverage] powerup=shield frame={frame} timer=900");
        }

        int transformMode = frame switch
        {
            950 => 1,
            1550 => 2,
            2150 => 3,
            _ => 0,
        };
        if (transformMode != 0)
        {
            CallGameFunction(c, m, 0x8003E32Cu,
                player, (uint)transformMode, 500u);
            Console.Error.WriteLine(
                $"[V82Coverage] powerup=transform-{transformMode} " +
                $"frame={frame} active={m.ReadU8(player + 0xACu)} " +
                $"timer={m.ReadU16(player + 0x12Cu)}");
        }
    }

    static void MaintainSoakVehicle(IMemory m, uint player, int frame)
    {
        if (player < PcHeapBase || player >= PcHeapEnd - 0x104u ||
            m.ReadU8(player + 8u) != 2)
            return;

        // Weapon/effect coverage deliberately fires at close range for several
        // minutes. Preserve retail durability values between scheduler passes
        // so the test does not end early when splash damage destroys the
        // harness vehicle. Dynamics, collisions and damage callbacks still run.
        ushort maxHealth = m.ReadU16(player + 0x1Eu);
        if (maxHealth == 0)
            return;

        bool repaired = false;
        if (m.ReadU16(player + 0x1Cu) < maxHealth)
        {
            m.WriteU16(player + 0x1Cu, maxHealth);
            repaired = true;
        }
        foreach (uint offset in SoakDamageZoneOffsets)
        {
            uint zone = m.ReadU32(player + offset);
            if (zone < PcHeapBase || zone >= PcHeapEnd - 0x20u)
                continue;
            if (m.ReadU16(zone + 0x1Cu) < maxHealth)
            {
                m.WriteU16(zone + 0x1Cu, maxHealth);
                repaired = true;
            }
        }

        if (repaired && (++_soakDurabilityRepairs <= 8 ||
            _soakDurabilityRepairs % 120 == 0))
        {
            Console.Error.WriteLine(
                $"[V82Coverage] durability-restored frame={frame} " +
                $"max={maxHealth} repairs={_soakDurabilityRepairs}");
        }
    }

    static void TraceSoakPowerState(IMemory m, uint player, int frame)
    {
        if (player < PcHeapBase || player >= PcHeapEnd - 0x130u)
            return;
        if (frame != 1 && frame % _soakHeartbeatFrames != 0)
            return;
        string state =
            $"transform={m.ReadU8(player + 0xACu)} " +
            $"transformTimer={m.ReadU16(player + 0x12Cu)} " +
            $"weaponUpgrade={m.ReadU16(player + 0x12Eu)} " +
            $"shield={m.ReadU16(player + 0x130u)} " +
            $"radarJammer={m.ReadU16(player + 0x132u)}";
        if (state == _lastSoakPowerState)
            return;
        _lastSoakPowerState = state;
        Console.Error.WriteLine(
            $"[V82Coverage] power-state frame={frame} {state}");
    }

    static void UpdateSoakWeapon(CpuContext c, IMemory m, uint player, int frame)
    {
        if (player < PcHeapBase || player >= PcHeapEnd - 0x130u)
            return;

        const int firstAttachFrame = 30;
        const int weaponWindowFrames = 360;
        if (frame >= firstAttachFrame &&
            (frame - firstAttachFrame) % weaponWindowFrames == 0)
        {
            int window = (frame - firstAttachFrame) / weaponWindowFrames;
            int kind = ((_soakWeaponStartKind - 1 + window) % 7) + 1;
            for (int guard = 0;
                 guard < 3 && m.ReadU32(player + 0x120u) != 0u;
                 guard++)
            {
                CallGameFunction(c, m, 0x8003A280u, player, 0u);
            }
            CallGameFunction(c, m, 0x8003A500u, player, 1u << (24 + kind));
            _soakWeaponKind = kind;
            _soakWeaponAttachFrame = frame;
            _soakWeaponObject = m.ReadU32(player + 0x120u);
            m.WriteU8(player + 0xAEu, 0);
            if (_soakWeaponObject >= PcHeapBase &&
                _soakWeaponObject < PcHeapEnd - 0x80u)
            {
                m.WriteU16(
                    _soakWeaponObject + 0x1Cu,
                    MaximumAttachedWeaponAmmo);
                _soakWeaponAmmo = MaximumAttachedWeaponAmmo;
                Console.Error.WriteLine(
                    $"[V82Coverage] weapon-armed kind={kind} " +
                    $"object=0x{_soakWeaponObject:X8} " +
                    $"objectKind={unchecked((sbyte)m.ReadU8(_soakWeaponObject + 9u))} " +
                    $"callback=0x{m.ReadU32(_soakWeaponObject):X8} " +
                    $"ammo={MaximumAttachedWeaponAmmo}");
            }
            else
            {
                _soakWeaponAmmo = 0;
                Console.Error.WriteLine(
                    $"[V82Coverage] weapon-attach-failed kind={kind} " +
                    $"slot0=0x{_soakWeaponObject:X8}");
            }
        }

        if (_soakWeaponObject >= PcHeapBase &&
            _soakWeaponObject < PcHeapEnd - 0x20u &&
            m.ReadU32(player + 0x120u) == _soakWeaponObject)
        {
            ushort ammo = m.ReadU16(_soakWeaponObject + 0x1Cu);
            if (ammo < _soakWeaponAmmo && SoakWeaponsFired.Add(_soakWeaponKind))
            {
                Console.Error.WriteLine(
                    $"[V82Coverage] weapon-fired kind={_soakWeaponKind} " +
                    $"ammo={_soakWeaponAmmo}->{ammo} fired=" +
                    $"{string.Join(',', SoakWeaponsFired.Order())}");
            }
            _soakWeaponAmmo = ammo;
        }
    }

    static void ClampAttachedWeaponAmmo(IMemory m)
    {
        uint player = _playerVehicle;
        if (player < PcHeapBase || player >= PcHeapEnd - 0x124u)
            return;

        uint weapon = m.ReadU32(player + 0x120u);
        if (weapon < PcHeapBase || weapon >= PcHeapEnd - 0x20u)
            return;

        ushort ammo = m.ReadU16(weapon + 0x1Cu);
        if (ammo > MaximumAttachedWeaponAmmo)
            m.WriteU16(weapon + 0x1Cu, MaximumAttachedWeaponAmmo);
    }

    static ushort SoakWeaponCommandInput(int frame)
    {
        if (_soakWeaponKind is < 1 or > 6)
            return 0;
        int age = frame - _soakWeaponAttachFrame;
        if (age < 20)
            return 0;

        int commandIndex = ((age - 20) / 100) % 3;
        int commandFrame = (age - 20) % 100;
        int[] command = SoakWeaponCommands[_soakWeaponKind - 1][commandIndex];
        ushort input = commandFrame switch
        {
            0 or 1 => DirectionMask(command[0]),
            4 or 5 => DirectionMask(command[1]),
            8 or 9 => DirectionMask(command[2]),
            12 or 13 => Controller.R2,
            40 or 41 => Controller.L2,
            _ => (ushort)0,
        };
        if (commandFrame == 12 &&
            SoakSpecialCommands.Add($"{_soakWeaponKind}:{commandIndex}"))
        {
            Console.Error.WriteLine(
                $"[V82Coverage] special-command weapon={_soakWeaponKind} " +
                $"variant={commandIndex + 1} sequence=" +
                $"{command[0]},{command[1]},{command[2]}");
        }
        return input;
    }

    static ushort DirectionMask(int direction) => direction switch
    {
        0 => Controller.Up,
        1 => Controller.Left,
        2 => Controller.Down,
        3 => Controller.Right,
        _ => (ushort)0,
    };

    static uint CallGameFunction(
        CpuContext c, IMemory m, uint address,
        uint a0 = 0u, uint a1 = 0u, uint a2 = 0u, uint a3 = 0u)
    {
        var snapshot = c.Snapshot();
        c.A0 = a0;
        c.A1 = a1;
        c.A2 = a2;
        c.A3 = a3;
        Dispatcher.Call(c, m, address);
        uint result = c.V0;
        c.Restore(snapshot);
        return result;
    }

    public static void SafeShapeCollision(CpuContext c, IMemory m)
    {
        uint objectA = c.A0;
        uint objectB = c.A1;
        uint matrixA = c.A2;
        uint matrixB = c.A3;
        uint rawShapeA = m.ReadU32(objectA + 0x60u);
        uint rawShapeB = m.ReadU32(objectB + 0x60u);
        uint shapeA = Dispatcher.ResolveOwnedAddress(objectA, rawShapeA);
        uint shapeB = Dispatcher.ResolveOwnedAddress(objectB, rawShapeB);

        if (shapeA == 0u || shapeB == 0u ||
            !IsShapeAddress(shapeA, 4u) || !IsShapeAddress(shapeB, 4u))
        {
            c.V0 = 0u;
            return;
        }

        uint a = shapeA;
        for (int outerGuard = 0; outerGuard < 4096; outerGuard++)
        {
            if (!TryReadShapeRecord(m, a, out ushort typeA,
                out ushort countA, out uint nextA))
            {
                WarnShapeStream(objectA, objectB, a, shapeB, typeA, countA);
                c.V0 = 0u;
                return;
            }
            if (typeA == 0)
            {
                c.V0 = 0u;
                return;
            }

            bool testA = typeA == 2 ||
                (typeA == 1 && (countA & 0x8000) == 0);
            if (testA)
            {
                uint b = shapeB;
                for (int innerGuard = 0; innerGuard < 4096; innerGuard++)
                {
                    if (!TryReadShapeRecord(m, b, out ushort typeB,
                        out ushort countB, out uint nextB))
                    {
                        WarnShapeStream(objectA, objectB, a, b, typeB, countB);
                        c.V0 = 0u;
                        return;
                    }
                    if (typeB == 0)
                        break;

                    bool hit = false;
                    if (typeA == 1 && typeB == 1 &&
                        (countB & 0x8000) == 0)
                    {
                        hit =
                            CallGameFunction(c, m, 0x800281FCu,
                                a + 4u, matrixA, b + 4u, matrixB) != 0u &&
                            CallGameFunction(c, m, 0x800281FCu,
                                b + 4u, matrixB, a + 4u, matrixA) != 0u;
                    }
                    else if (typeA == 1 && typeB == 2)
                    {
                        hit = countB == 0;
                        for (uint i = 0; i < countB && !hit; i++)
                        {
                            if (CallGameFunction(c, m, 0x8002E2E8u,
                                a + 4u, matrixA, b + 4u + i * 12u, matrixB) == 0u)
                                break;
                            hit = i + 1u == countB;
                        }
                    }
                    else if (typeA == 2 && typeB == 1 &&
                        (countB & 0x8000) == 0)
                    {
                        hit = countA == 0;
                        for (uint i = 0; i < countA && !hit; i++)
                        {
                            if (CallGameFunction(c, m, 0x8002E2E8u,
                                b + 4u, matrixB, a + 4u + i * 12u, matrixA) == 0u)
                                break;
                            hit = i + 1u == countA;
                        }
                    }

                    if (hit)
                    {
                        m.WriteU32(0x1F800004u, a);
                        m.WriteU32(0x1F800008u, b);
                        m.WriteU32(0x1F80000Cu, objectA);
                        m.WriteU32(0x1F800010u, objectB);
                        c.V0 = 0x1F800000u;
                        return;
                    }
                    b = nextB;
                }
            }
            a = nextA;
        }

        WarnShapeStream(objectA, objectB, a, shapeB, 0xFFFF, 0);
        c.V0 = 0u;
    }

    static bool TryReadShapeRecord(
        IMemory m, uint address, out ushort type, out ushort count, out uint next)
    {
        type = 0xFFFF;
        count = 0;
        next = address;
        if (!IsShapeAddress(address, 4u))
            return false;
        type = m.ReadU16(address);
        count = m.ReadU16(address + 2u);
        if (type == 0)
            return true;
        if (type == 1)
        {
            next = address + 0x1Cu;
            return IsShapeAddress(address, 0x1Cu);
        }
        if (type != 2 || count > 4096)
            return false;
        uint size = 4u + (uint)count * 12u;
        next = address + size;
        return IsShapeAddress(address, size);
    }

    static bool IsShapeAddress(uint address, uint size) =>
        address >= 0x80000000u && address < PcHeapEnd &&
        size <= PcHeapEnd - address;

    static void WarnShapeStream(
        uint objectA, uint objectB, uint recordA, uint recordB,
        ushort type, ushort count)
    {
        string key = $"{recordA:X8}:{recordB:X8}:{type:X4}:{count:X4}";
        if (CollisionShapeWarnings.Add(key))
        {
            Console.Error.WriteLine(
                $"[V82Collision] rejected malformed shape stream " +
                $"objects=0x{objectA:X8}/0x{objectB:X8} " +
                $"records=0x{recordA:X8}/0x{recordB:X8} " +
                $"type=0x{type:X4} count={count}");
        }
    }

    public static ushort GetAutomationInputMask() =>
        _soakEnabled ? _soakAutomationInput : (ushort)0;

    public static void TraceGameplayOrderingTable(CpuContext c, IMemory m)
    {
        int frame = _gameplayFrameCount;
        if (frame <= 0 || frame > 3 || frame == _lastTracedOrderingTableFrame)
            return;
        _lastTracedOrderingTableFrame = frame;

        uint ramAddressMask =
            (Runtime.Mode == RunMode.Devkit
                ? MemoryMap.DevkitRamSize
                : MemoryMap.RetailRamSize) - 4u;
        uint address = c.A2 & ramAddressMask;
        int packets = 0;
        int words = 0;
        var opcodes = new Dictionary<byte, int>();
        var tpages = new List<string>();
        var tags = new List<string>();
        for (int guard = 0; guard < 0x100000; guard++)
        {
            uint header = m.ReadU32(address);
            int count = (int)(header >> 24);
            if (tags.Count < 12)
                tags.Add($"0x{address:X6}:0x{header:X8}");
            if (count != 0)
            {
                packets++;
                words += count;
                uint command = m.ReadU32(address + 4u);
                byte opcode = (byte)(command >> 24);
                opcodes[opcode] = opcodes.GetValueOrDefault(opcode) + 1;
                bool polygon = opcode is >= 0x20 and <= 0x3F;
                bool textured = (command & (1u << 26)) != 0u;
                if (polygon && textured && tpages.Count < 24)
                {
                    bool gouraud = (command & (1u << 28)) != 0u;
                    int tpageWord = gouraud ? 6 : 5;
                    if (count >= tpageWord)
                    {
                        uint uv = m.ReadU32(address + 4u + (uint)(tpageWord - 1) * 4u);
                        tpages.Add($"0x{(uv >> 16):X4}");
                    }
                }
            }

            uint next = header & 0xFFFFFFu;
            if (next == 0xFFFFFFu || (next & 0x800000u) != 0u)
                break;
            address = next & ramAddressMask;
        }

        Console.Error.WriteLine(
            $"[V82Render] frame={frame} ot=0x{c.A2:X8} packets={packets} " +
            $"words={words} ops={string.Join(',', opcodes.OrderBy(p => p.Key).Select(p => $"{p.Key:X2}:{p.Value}"))} " +
            $"tpages={string.Join(',', tpages)} tags={string.Join(',', tags)}");
    }

    // The retail geometry microcode uses JR dispatch and tail-jumps back to
    // 0x80022164. Recompiled continuation methods turn those jumps into managed
    // calls. Queue the next continuation and let the outermost invocation drive
    // an iterative trampoline so large primitive streams cannot overflow the
    // host stack.
    public static bool EnterGeometry22164(CpuContext c, IMemory m)
    {
        CaptureGeometryContinuation(c, m, 0x80022164u);
        return EnterGeometryContinuation(0x80022164u);
    }

    public static bool EnterGeometry22910(CpuContext c, IMemory m)
    {
        CaptureGeometryContinuation(c, m, 0x80022910u);
        return EnterGeometryContinuation(0x80022910u);
    }

    static void CaptureGeometryContinuation(CpuContext c, IMemory m, uint target)
    {
        if (!TraceImportedOverlayAbi)
            return;
        byte packetType = IsShapeAddress(c.S3, 4u)
            ? m.ReadU8(c.S3 + 3u)
            : (byte)0xFF;
        GeometryContinuationHistory.Enqueue(new(
            target, c.S0, c.S2, c.S3, c.S5, c.A2, c.A3, packetType));
        while (GeometryContinuationHistory.Count > 48)
            GeometryContinuationHistory.Dequeue();
    }

    static void DumpGeometryContinuationHistory()
    {
        if (!TraceImportedOverlayAbi)
            return;
        Console.Error.WriteLine("[V82GeometryHistory] begin");
        foreach (GeometryContinuationTrace item in GeometryContinuationHistory)
        {
            Console.Error.WriteLine(
                $"[V82GeometryHistory] target=0x{item.Target:X8} " +
                $"s0=0x{item.S0:X8} s2=0x{item.S2:X8} " +
                $"s3=0x{item.S3:X8} s5={item.S5} " +
                $"a2=0x{item.A2:X8} a3=0x{item.A3:X8} " +
                $"packet=0x{item.PacketType:X2}");
        }
        Console.Error.WriteLine("[V82GeometryHistory] end");
    }

    public static bool GuardGeometry22E78(CpuContext c, IMemory m)
    {
        uint descriptor = c.A3;
        if (!IsShapeAddress(descriptor, 0x18u))
            return SkipMalformedGeometryPrimitive(c, m, 0u);

        uint textureIndex = (uint)(m.ReadU16(descriptor + 0x16u) & 0x3FFF);
        uint tableEntry = c.S0 + textureIndex * 4u + 0x1Cu;
        if (!IsShapeAddress(tableEntry, 4u))
            return SkipMalformedGeometryPrimitive(c, m, tableEntry);

        uint texture = m.ReadU32(tableEntry);
        if (TraceGeometryTextures && _geometryTextureTraceCount++ < 32)
        {
            Console.Error.WriteLine(
                $"[V82GeometryTexture] packet=0x{m.ReadU8(descriptor + 3u):X2} " +
                $"textureWord=0x{m.ReadU16(descriptor + 0x16u):X4} " +
                $"index={textureIndex} table=0x{tableEntry:X8} " +
                $"texture=0x{texture:X8} s0=0x{c.S0:X8} s0Words=" +
                $"{m.ReadU32(c.S0):X8},{m.ReadU32(c.S0 + 4u):X8}," +
                $"{m.ReadU32(c.S0 + 8u):X8},{m.ReadU32(c.S0 + 12u):X8}");
        }
        return IsShapeAddress(texture, 0x0Cu) ||
               SkipMalformedGeometryPrimitive(c, m, texture);
    }

    static bool SkipMalformedGeometryPrimitive(
        CpuContext c, IMemory m, uint texture)
    {
        if (_geometryClipCount++ < 16)
        {
            uint descriptor = c.A3;
            ushort textureWord = IsShapeAddress(descriptor, 0x18u)
                ? m.ReadU16(descriptor + 0x16u)
                : (ushort)0;
            uint textureIndex = (uint)(textureWord & 0x3FFF);
            uint tableEntry = c.S0 + textureIndex * 4u + 0x1Cu;
            Console.Error.WriteLine(
                $"[V82Geometry] skipped malformed textured primitive " +
                $"packet=0x{(IsShapeAddress(descriptor, 4u) ? m.ReadU8(descriptor + 3u) : 0):X2} " +
                $"textureWord=0x{textureWord:X4} index={textureIndex} " +
                $"table=0x{tableEntry:X8} texture=0x{texture:X8} " +
                $"s0=0x{c.S0:X8} s0Words=" +
                $"{m.ReadU32(c.S0):X8},{m.ReadU32(c.S0 + 4u):X8}," +
                $"{m.ReadU32(c.S0 + 8u):X8},{m.ReadU32(c.S0 + 12u):X8} " +
                $"s3=0x{c.S3:X8} a3=0x{descriptor:X8}");
            DumpGeometryContinuationHistory();
            DumpRenderGroupEntryHistory();
            DumpNativeModelOwner(m, c.S0);
            DumpNativeModelOwnershipHistory();
        }
        c.S3 += 0x18u;
        Dispatcher.Call(c, m, 0x80022164u);
        return false;
    }

    static bool EnterGeometryContinuation(uint target)
    {
        if (_geometryContinuationActive)
        {
            _geometryNextTarget = target;
            _geometrySuppressedLeaves++;
            return false;
        }

        _geometryContinuationActive = true;
        _geometryContinuationIterations = 0;
        return true;
    }

    /// <summary>
    /// Records the packet produced by V8:2's native kind-12 geometry handler.
    /// Kind 12 is the authored environment/reflection packet in both retail
    /// V8:2 banks and converted V8 banks, so classification comes from the
    /// engine dispatch path rather than vehicle identity, palette, or colour.
    /// </summary>
    public static void ObserveVehicleReflectionPacket(CpuContext c, IMemory m)
    {
        bool vehicleScope =
            ObjectRenderScopes.TryPeek(out ObjectRenderScope objectScope) &&
            objectScope.IsVehicle;
        bool registeredVehicleGroup =
            ImportedRenderGroupScopes.TryPeek(
                out ImportedRenderGroupScope groupScope) &&
            groupScope.Resolved;
        if (vehicleScope || registeredVehicleGroup)
            GpuHle.RegisterVehicleReflectionPacket(c.A2);
    }

    public static void LeaveGeometryContinuation(CpuContext c, IMemory m)
    {
        if (_geometrySuppressedLeaves > 0)
        {
            _geometrySuppressedLeaves--;
            return;
        }

        _geometryContinuationActive = false;
        if (_geometryContinuationDriving)
            return;

        _geometryContinuationDriving = true;
        try
        {
            while (_geometryNextTarget != 0u)
            {
                if (++_geometryContinuationIterations > 8192)
                {
                    if (_geometryClipCount++ < 16)
                        Console.Error.WriteLine(
                            $"[V82Geometry] terminated runaway stream " +
                            $"target=0x{_geometryNextTarget:X8} s3=0x{c.S3:X8} " +
                            $"s5={c.S5} s2=0x{c.S2:X8} sp=0x{c.SP:X8}");
                    CompleteGeometryFrame(c, m);
                    _geometryNextTarget = 0u;
                    break;
                }

                uint target = _geometryNextTarget;
                _geometryNextTarget = 0u;
                Dispatcher.Call(c, m, target);
            }
        }
        catch (InvalidOperationException ex) when (
            c.A2 >= 0x1F8003E0u &&
            c.A2 < 0x1F801000u &&
            ex.Message.StartsWith("unmapped address: 0x1F800", StringComparison.Ordinal))
        {
            if (_geometryClipCount++ < 16)
                Console.Error.WriteLine(
                    $"[V82Geometry] terminated scratchpad-overflow stream " +
                    $"output=0x{c.A2:X8} s3=0x{c.S3:X8} " +
                    $"target=0x{_geometryNextTarget:X8}");
            CompleteGeometryFrame(c, m);
            _geometryNextTarget = 0u;
        }
        finally
        {
            _geometryContinuationActive = false;
            _geometryContinuationDriving = false;
            _geometrySuppressedLeaves = 0;
            _geometryContinuationIterations = 0;
            _geometryNextTarget = 0u;
        }
    }

    static void CompleteGeometryFrame(CpuContext c, IMemory m)
    {
        // Exact func_80021F70/func_80022164 epilogue. Continuation targets share
        // the original 0x38-byte frame; merely returning from the managed
        // trampoline leaves SP and every saved S register displaced.
        m.WriteU32(c.GP + 0x610u, c.S2);
        c.RA = m.ReadU32(c.SP + 0x34u);
        c.S7 = m.ReadU32(c.SP + 0x30u);
        c.S6 = m.ReadU32(c.SP + 0x2Cu);
        c.S5 = m.ReadU32(c.SP + 0x28u);
        c.S4 = m.ReadU32(c.SP + 0x24u);
        c.S3 = m.ReadU32(c.SP + 0x20u);
        c.S2 = m.ReadU32(c.SP + 0x1Cu);
        c.S1 = m.ReadU32(c.SP + 0x18u);
        c.S0 = m.ReadU32(c.SP + 0x14u);
        c.SP += 0x38u;
    }

    // SHELL uses a 640-wide front-end VRAM layout. LOAD.DLL immediately begins
    // uploading arena textures at 320-wide gameplay resolution, but the retail
    // asynchronous teardown has not reset the allocator yet in the static
    // host. Re-run the original 320x480 allocator initialization before LOAD
    // consumes any texture banks.
    public static void ResetMatchVram(CpuContext c, IMemory m)
    {
        ResetRendererObjectTracking();
        GpuHle.GameplayActive = true;
        var snapshot = c.Snapshot();
        c.A0 = 1u;
        Dispatcher.Call(c, Dispatcher.UnwrapMemory(m), 0x8002091Cu);
        c.Restore(snapshot);
        _matchVramActive = true;
        _matchVramSuccesses = 0;
        _matchVramFailures = 0;
        _testDefeatInjected = false;
        _testWaterInjected = false;
        _testWaterConfigLogged = false;
        _testWaterDestroyed = false;
        _testWaterRespawned = false;
        _testWaterInjectedFrame = 0;
        ImportedWaterDwell.Clear();
        _gameplayFrameCount = 0;
        Console.Error.WriteLine(
            "[V82Compat] reset VRAM allocator to 320-wide gameplay layout");
        ReserveGuestVramForMatch(c, m);
    }

    public static void MarkOriginalMatchVramReset(CpuContext c, IMemory m)
    {
        if (c.RA != 0x800137A0u)
            return;
        ResetRendererObjectTracking();
        _matchVramActive = true;
        GpuHle.GameplayActive = true;
        _matchVramSuccesses = 0;
        _matchVramFailures = 0;
        _testDefeatInjected = false;
        _testWaterInjected = false;
        _testWaterConfigLogged = false;
        _testWaterDestroyed = false;
        _testWaterRespawned = false;
        _testWaterInjectedFrame = 0;
        ImportedWaterDwell.Clear();
        _gameplayFrameCount = 0;
        Console.Error.WriteLine(
            "[V82Compat] observed original 320-wide gameplay VRAM reset");
        ReserveGuestVramForMatch(c, m);
    }

    static void ReserveGuestVramForMatch(CpuContext c, IMemory m)
    {
        GuestVramReservations.Clear();
        if (!V82VehicleRegistry.HasAnySelection &&
            !V82VehicleRegistry.HasDefaultReplacement)
            return;

        V82VehicleRegistry.ResetRuntimeForMatch();
        IReadOnlyList<NativeVramAllocation> requests =
            V82VehicleRegistry.SelectedVramAllocations();
        if (requests.Count == 0)
            return;

        m = Dispatcher.UnwrapMemory(m);
        var snapshot = c.Snapshot();
        uint stack10 = m.ReadU32(c.SP + 0x10u);
        uint stack14 = m.ReadU32(c.SP + 0x14u);
        try
        {
            foreach (NativeVramAllocation request in requests)
            {
                c.A0 = request.Width;
                c.A1 = request.Height;
                c.A2 = request.AlignWidth;
                c.A3 = request.AlignHeight;
                m.WriteU32(c.SP + 0x10u, request.LimitWidth);
                m.WriteU32(c.SP + 0x14u, request.LimitHeight);
                Dispatcher.Call(c, m, 0x80020A80u);
                if (c.V0 == 0u)
                    throw new OutOfMemoryException(
                        $"V8:2 guest VRAM reservation failed for " +
                        $"{request.Width}x{request.Height}");
                uint x = (uint)(short)m.ReadU16(c.V0);
                uint y = (uint)(short)m.ReadU16(c.V0 + 2u);
                c.A0 = 0x18u;
                PcMalloc(c, m);
                uint descriptor = c.V0;
                if (descriptor == 0u)
                    throw new OutOfMemoryException(
                        "V8:2 synthetic VRAM descriptor allocation failed");
                m.WriteU16(descriptor, checked((ushort)x));
                m.WriteU16(descriptor + 2u, checked((ushort)y));
                m.WriteU16(
                    descriptor + 4u, checked((ushort)request.Width));
                m.WriteU16(
                    descriptor + 6u, checked((ushort)request.Height));
                m.WriteU32(descriptor + 8u, 1u);
                m.WriteU32(descriptor + 0xCu, 0u);
                m.WriteU32(descriptor + 0x10u, 0u);
                m.WriteU32(descriptor + 0x14u, 0u);
                SyntheticVramDescriptors.Add(descriptor);
                GuestVramReservations.Add(new GuestVramReservation(
                    request, x, y, descriptor));
            }
            Console.Error.WriteLine(
                $"[V82Vehicles] reserved {GuestVramReservations.Count} " +
                "native VRAM rectangles for selected guests");
        }
        catch
        {
            ReleaseGuestVramReservation(c, m);
            throw;
        }
        finally
        {
            m.WriteU32(c.SP + 0x10u, stack10);
            m.WriteU32(c.SP + 0x14u, stack14);
            c.Restore(snapshot);
        }
    }

    public static void ReserveSelectorVram(
        CpuContext c, IMemory m, int guest)
    {
        ReleaseSelectorVramReservation(c, m);
        Console.Error.WriteLine(
            $"[V82Vehicles] streaming native selector VRAM for guest={guest}");
    }

    public static void ReleaseSelectorVramReservation(
        CpuContext c, IMemory m)
    {
        if (SelectorVramReservations.Count == 0)
            return;

        m = Dispatcher.UnwrapMemory(m);
        var snapshot = c.Snapshot();
        int retiredDescriptors = 0;
        int releasedBacking = 0;
        int alreadyReleasedBacking = 0;
        try
        {
            for (int index = SelectorVramReservations.Count - 1;
                 index >= 0;
                 index--)
            {
                GuestVramReservation reservation =
                    SelectorVramReservations[index];
                if (SyntheticVramDescriptors.Remove(reservation.Descriptor))
                {
                    c.A0 = reservation.Descriptor;
                    PcFree(c, m);
                    retiredDescriptors++;
                }
                // The allocator can restructure/coalesce native nodes while a
                // preview is alive, so a saved descriptor pointer is not
                // stable. Use retail's coordinate-release wrapper; it searches
                // the current tree and dispatches descriptor teardown itself.
                c.A0 = reservation.X;
                c.A1 = reservation.Y;
                Dispatcher.Call(c, m, 0x80020F5Cu);
                if (c.V0 == 0u)
                    alreadyReleasedBacking++;
                else
                    releasedBacking++;
            }
            Console.Error.WriteLine(
                $"[V82Vehicles] selector teardown retired " +
                $"{retiredDescriptors} host descriptors, released " +
                $"{releasedBacking} independent native VRAM rectangles, " +
                $"observed {alreadyReleasedBacking} retail-owned releases");
            SelectorVramReservations.Clear();
        }
        finally
        {
            c.Restore(snapshot);
        }
    }

    public static int SelectorVramReservationCount =>
        SelectorVramReservations.Count;

    /// <summary>
    /// Retires a stale host-side view after an external allocator reset. Normal
    /// selector preview teardown must call ReleaseSelectorVramReservation so
    /// the native backing rectangles and synthetic descriptors are both freed.
    /// </summary>
    public static void ForgetSelectorVramReservations()
    {
        if (SelectorVramReservations.Count != 0)
            Console.Error.WriteLine(
                $"[V82Vehicles] native teardown released " +
                $"{SelectorVramReservations.Count} selector VRAM backing " +
                "rectangles");
        SelectorVramReservations.Clear();
        ClaimedGuestVramReservations.Clear();
        _activeGuestVramReservations = null;
        _guestVramClaimReusable = false;
        _guestVramClaimActive = false;
        _guestVramClaimIndex = 0;
        _guestVramClaimMisses = 0;
    }

    public static void TraceResultStringLength(
        CpuContext c, IMemory m)
    {
        if (c.RA < 0x80012800u || c.RA >= 0x80013300u)
            return;

        Console.Error.WriteLine(
            $"[V82ResultString] strlen caller=0x{c.RA:X8} " +
            $"source=0x{c.A0:X8} sp=0x{c.SP:X8} gp=0x{c.GP:X8} " +
            $"s0=0x{c.S0:X8} s1=0x{c.S1:X8} s2=0x{c.S2:X8} " +
            $"s3=0x{c.S3:X8}");
    }

    public static bool TraceResultFormat(
        CpuContext c, IMemory m)
    {
        if (c.RA < 0x80012800u || c.RA >= 0x80013300u)
            return true;

        // func_80012930 builds "Shared\\%s.xa" at this call site. Its native
        // table has only the 18 retail types; imported stable types (64+) map
        // to the original V8 outcome bank and roster-ordered XA channel.
        if (c.RA == 0x80013268u)
        {
            int player = checked((int)c.S2);
            if ((uint)player < 2u)
            {
                int type = (sbyte)m.ReadU8(c.GP + 0x1104u + (uint)player);
                // The retail result player adds this byte to its XA-channel
                // choice. Zero is the defeated branch; one is the winner
                // branch. Preserve that native outcome bit while replacing
                // only the imported driver's file bank and channel.
                bool defeated = m.ReadU8(c.GP + 0x33u) == 0;
                if (V82VehicleRegistry.WriteResultVoicePath(
                    m,
                    type,
                    defeated,
                    c.A0,
                    out string path))
                {
                    Console.Error.WriteLine(
                        $"[V82ResultString] imported type={type} " +
                        $"wrote original V8 XA path='{path}'");
                    c.V0 = checked((uint)path.Length);
                    // The host has produced exactly the string the retail
                    // sprintf call would have written. Skipping that call
                    // avoids borrowing permanent PS1 memory for a host-only
                    // stem while preserving the caller's native buffer and
                    // subsequent CD/audio lifecycle.
                    return false;
                }
            }
        }

        Console.Error.WriteLine(
            $"[V82ResultString] sprintf caller=0x{c.RA:X8} " +
            $"destination=0x{c.A0:X8} format=0x{c.A1:X8} " +
            $"a2=0x{c.A2:X8} a3=0x{c.A3:X8} sp=0x{c.SP:X8} " +
            $"s2=0x{c.S2:X8}");
        return true;
    }

    public static bool OverrideResultVoiceChannel(
        CpuContext c,
        IMemory m) =>
        V82VehicleRegistry.OverrideResultVoiceChannel(c, m);

    public static bool ResolveVirtualLooseFile(
        CpuContext c,
        IMemory m)
    {
        if (Runtime.Cd != null)
        {
            IMemory raw = Dispatcher.UnwrapMemory(m);
            string path = ReadNativeAscii(raw, c.A0, 256)
                .Replace('/', '\\');
            if (path.Length != 0 &&
                Runtime.Cd.Fs.Locate(path, out int lba, out uint size) &&
                lba >= Runtime.Cd.LeadOutLba)
            {
                // Expanded overrides and append-only mod files live in a
                // private extent after the retail lead-out. Return the same
                // descriptor layout as V8:2's native file-table search so
                // every existing loader keeps its allocation/read lifecycle.
                uint descriptor = c.SP - 0x20u;
                for (uint offset = 0; offset < 0x20u; offset += 4u)
                    raw.WriteU32(descriptor + offset, 0u);
                raw.WriteU32(descriptor + 0x0Cu, checked((uint)lba));
                raw.WriteU32(descriptor + 0x10u, size);
                c.V0 = descriptor;
                Console.WriteLine(
                    $"[V82LooseFile] virtual descriptor path={path} " +
                    $"lba={lba} size={size} descriptor=0x{descriptor:X8}");
                return false;
            }
        }
        return V82VehicleRegistry.ResolveOriginalResultVoiceFile(c, m);
    }

    public static void ReleaseGuestVramReservation(CpuContext c, IMemory m)
    {
        if (GuestVramReservations.Count == 0)
            return;

        m = Dispatcher.UnwrapMemory(m);
        var snapshot = c.Snapshot();
        try
        {
            for (int index = GuestVramReservations.Count - 1;
                 index >= 0;
                 index--)
            {
                GuestVramReservation reservation =
                    GuestVramReservations[index];
                c.A0 = reservation.X;
                c.A1 = reservation.Y;
                Dispatcher.Call(c, m, 0x80020F5Cu);
                if (c.V0 == 0u)
                    Console.Error.WriteLine(
                        $"[V82Vehicles] match VRAM rectangle at " +
                        $"({reservation.X},{reservation.Y}) was already released");
            }
            Console.Error.WriteLine(
                $"[V82Vehicles] released {GuestVramReservations.Count} " +
                "reserved native VRAM rectangles");
            GuestVramReservations.Clear();
        }
        finally
        {
            c.Restore(snapshot);
        }
    }

    public static void BeginGuestVramClaim(bool reusable = false)
    {
        _guestVramClaimIndex = 0;
        _guestVramClaimMisses = 0;
        ClaimedGuestVramReservations.Clear();
        _guestVramClaimReusable = reusable;
        _activeGuestVramReservations =
            reusable ? SelectorVramReservations : GuestVramReservations;
        _guestVramClaimActive =
            reusable || _activeGuestVramReservations.Count != 0;
    }

    public static void EndGuestVramClaim(CpuContext c, IMemory m)
    {
        List<GuestVramReservation> reservations =
            _activeGuestVramReservations ?? GuestVramReservations;
        if (_guestVramClaimReusable)
        {
            int reusableClaimed = ClaimedGuestVramReservations.Count;
            ClaimedGuestVramReservations.Clear();
            _guestVramClaimActive = false;
            _activeGuestVramReservations = null;
            Console.Error.WriteLine(
                $"[V82Vehicles] claimed {reusableClaimed} reusable " +
                "selector VRAM rectangles");
            return;
        }

        var pending = new List<GuestVramReservation>();
        for (int index = 0; index < reservations.Count; index++)
            if (!ClaimedGuestVramReservations.Contains(index))
                pending.Add(reservations[index]);
        int claimed = ClaimedGuestVramReservations.Count;
        reservations.Clear();
        reservations.AddRange(pending);
        ClaimedGuestVramReservations.Clear();
        _guestVramClaimActive = pending.Count != 0;
        _activeGuestVramReservations = null;
        Console.Error.WriteLine(
            $"[V82Vehicles] claimed {claimed} body-time VRAM rectangles; " +
            $"retained {pending.Count} independent late-load reservations");
    }

    public static uint TextureDecodeScratchTop(
        CpuContext c, IMemory m)
    {
        if (_textureDecodeScratchTop != 0u)
            return _textureDecodeScratchTop;

        var snapshot = c.Snapshot();
        try
        {
            c.A0 = 0x2000u;
            PcMalloc(c, Dispatcher.UnwrapMemory(m));
            if (c.V0 == 0u)
                throw new OutOfMemoryException(
                    "V8:2 texture decoder scratch allocation failed");
            _textureDecodeScratchBase = c.V0;
            // func_80021064 owns a 0xC70-byte frame but calls through the GPU
            // callback table, so the complete nested depth is not represented
            // by that one frame.  It also writes the normal 8-byte caller ABI
            // area above its incoming SP.  The original placement put all
            // spare bytes above the frame, allowing nested helpers to
            // overwrite the adjacent game object.  Give the nested call chain
            // 0x1380 bytes below the main frame and retain 0x10 above it.
            _textureDecodeScratchTop = c.V0 + 0x1FF0u;
            return _textureDecodeScratchTop;
        }
        finally
        {
            c.Restore(snapshot);
        }
    }

    public static uint BeginTextureDecode(CpuContext c, IMemory m)
    {
        uint callerSp = c.SP;
        uint physicalSp = callerSp & 0x1FFFFFFFu;
        if (physicalSp < MemoryMap.ScratchpadBase ||
            physicalSp >= MemoryMap.ScratchpadBase + MemoryMap.ScratchpadSize)
            return 0u;

        uint relocatedTop = TextureDecodeScratchTop(c, m);
        for (uint offset = 0u; offset < 0x20u; offset++)
            m.WriteU8(_textureDecodeScratchBase + offset, 0xA5);
        c.SP = relocatedTop;
        int count = ++_relocatedTextureDecodeCount;
        if (count <= 16)
        {
            short x = (short)m.ReadU16(c.A0);
            short y = (short)m.ReadU16(c.A0 + 2u);
            short width = (short)m.ReadU16(c.A0 + 4u);
            short height = (short)m.ReadU16(c.A0 + 6u);
            Console.Error.WriteLine(
                $"[V82Texture] relocated late decode #{count} " +
                $"sp=0x{callerSp:X8}->0x{relocatedTop:X8} " +
                $"source=0x{c.A1:X8} target=0x{c.A0:X8} " +
                $"rect={x},{y} {width}x{height}");
        }
        return callerSp;
    }

    public static void EndTextureDecode(
        CpuContext c, IMemory m, uint callerSp)
    {
        if (callerSp == 0u)
            return;
        for (uint offset = 0u; offset < 0x20u; offset++)
            if (m.ReadU8(_textureDecodeScratchBase + offset) != 0xA5)
                throw new InvalidOperationException(
                    "V8:2 texture decoder exceeded its relocated lower guard");
        c.SP = callerSp;
    }

    public static void AbortGuestVramClaim()
    {
        _guestVramClaimActive = false;
        ClaimedGuestVramReservations.Clear();
        _activeGuestVramReservations = null;
    }

    public static bool TrackVramAllocationPre(CpuContext c, IMemory m)
    {
        var request = (
            c.A0, c.A1, c.A2, c.A3,
            m.ReadU32(c.SP + 0x10u), m.ReadU32(c.SP + 0x14u));
        VramRequests.Push(request);
        bool palette = request.Item3 == 16u && request.Item4 == 1u;
        bool image = request.Item3 == 64u && request.Item4 == 256u;
        bool selectorOwned =
            _guestVramClaimReusable &&
            (palette || image) &&
            V82VehicleRegistry.OwnsCurrentTextureLoad(c, m, palette);
        SelectorOwnedVramRequests.Push(selectorOwned);
        if (_traceVram)
            Console.Error.WriteLine(
                $"[VRAM] allocate begin {request.Item1}x{request.Item2} " +
                $"align={request.Item3}x{request.Item4} " +
                $"limit={request.Item5}x{request.Item6} " +
                $"caller=0x{c.RA:X8}");
        if (_guestVramClaimActive)
        {
            List<GuestVramReservation> reservations =
                _activeGuestVramReservations ?? GuestVramReservations;
            if ((!palette && !image) ||
                !V82VehicleRegistry.OwnsCurrentTextureLoad(
                    c, m, palette))
                return true;

            for (int index = 0; index < reservations.Count; index++)
            {
                if (ClaimedGuestVramReservations.Contains(index))
                    continue;

                GuestVramReservation reservation =
                    reservations[index];
                NativeVramAllocation expected = reservation.Request;
                bool reusableFit =
                    _guestVramClaimReusable &&
                    request.Item1 <= expected.Width &&
                    request.Item2 <= expected.Height &&
                    request.Item3 == expected.AlignWidth &&
                    request.Item4 == expected.AlignHeight &&
                    request.Item5 == expected.LimitWidth &&
                    request.Item6 == expected.LimitHeight;
                bool exactFit = request == (
                    expected.Width,
                    expected.Height,
                    expected.AlignWidth,
                    expected.AlignHeight,
                    expected.LimitWidth,
                    expected.LimitHeight);
                if (!exactFit && !reusableFit)
                    continue;

                if (_guestVramClaimReusable)
                {
                    m.WriteU16(
                        reservation.Descriptor,
                        checked((ushort)reservation.X));
                    m.WriteU16(
                        reservation.Descriptor + 2u,
                        checked((ushort)reservation.Y));
                    m.WriteU16(
                        reservation.Descriptor + 4u,
                        checked((ushort)request.Item1));
                    m.WriteU16(
                        reservation.Descriptor + 6u,
                        checked((ushort)request.Item2));
                }
                c.V0 = reservation.Descriptor;
                ClaimedGuestVramReservations.Add(index);
                _guestVramClaimIndex++;
                if (ClaimedGuestVramReservations.Count ==
                    reservations.Count)
                {
                    _guestVramClaimActive = false;
                    if (!_guestVramClaimReusable)
                    {
                        reservations.Clear();
                        ClaimedGuestVramReservations.Clear();
                    }
                    Console.Error.WriteLine(
                        $"[V82Vehicles] all {_guestVramClaimIndex} native " +
                        "VRAM reservations claimed");
                }
                return false;
            }
            int miss = ++_guestVramClaimMisses;
            if (miss <= 24)
            {
                Console.Error.WriteLine(
                    $"[V82Vehicles] reservation miss #{miss}: " +
                    $"{request.Item1}x{request.Item2} " +
                    $"align={request.Item3}x{request.Item4} " +
                    $"limit={request.Item5}x{request.Item6}");
            }
        }
        return true;
    }

    public static bool IgnoreSyntheticVramFree(CpuContext c, IMemory m)
    {
        uint descriptor = c.A0;
        if (!SyntheticVramDescriptors.Remove(descriptor))
            return true;

        // The selector constructor owns the synthetic descriptor while the
        // retail allocator owns the backing rectangle at the same coordinates.
        // Free both at the constructor's normal texture teardown boundary.
        // Suppressing only the descriptor walk leaked the backing allocator
        // node once per texture and exhausted VRAM after eleven guest previews.
        m = Dispatcher.UnwrapMemory(m);
        uint x = (uint)(short)m.ReadU16(descriptor);
        uint y = (uint)(short)m.ReadU16(descriptor + 2u);
        var snapshot = c.Snapshot();
        try
        {
            c.A0 = descriptor;
            PcFree(c, m);
            c.A0 = x;
            c.A1 = y;
            Dispatcher.Call(c, m, 0x80020F5Cu);
            if (c.V0 == 0u)
                Console.Error.WriteLine(
                    $"[V82Vehicles] selector backing VRAM rectangle at " +
                    $"({x},{y}) was already released");
        }
        finally
        {
            c.Restore(snapshot);
        }
        return false;
    }

    public static void TrackVramAllocationPost(CpuContext c, IMemory m)
    {
        if (VramRequests.Count == 0) return;
        var request = VramRequests.Pop();
        if (_traceVram)
            Console.Error.WriteLine(
                $"[VRAM] allocate finish {request.Width}x{request.Height} " +
                $"descriptor=0x{c.V0:X8}");
        bool selectorOwned =
            SelectorOwnedVramRequests.Count != 0 &&
            SelectorOwnedVramRequests.Pop();
        if (_guestVramClaimReusable &&
            selectorOwned &&
            c.V0 != 0u &&
            !SyntheticVramDescriptors.Contains(c.V0))
        {
            m = Dispatcher.UnwrapMemory(m);
            uint backingDescriptor = c.V0;
            uint x = (uint)(short)m.ReadU16(backingDescriptor);
            uint y = (uint)(short)m.ReadU16(backingDescriptor + 2u);
            var snapshot = c.Snapshot();
            c.A0 = 0x18u;
            PcMalloc(c, m);
            uint descriptor = c.V0;
            c.Restore(snapshot);
            if (descriptor == 0u)
                throw new OutOfMemoryException(
                    "V8:2 selector VRAM descriptor allocation failed");
            m.WriteU16(descriptor, checked((ushort)x));
            m.WriteU16(descriptor + 2u, checked((ushort)y));
            m.WriteU16(
                descriptor + 4u, checked((ushort)request.Width));
            m.WriteU16(
                descriptor + 6u, checked((ushort)request.Height));
            m.WriteU32(descriptor + 8u, 1u);
            m.WriteU32(descriptor + 0xCu, 0u);
            m.WriteU32(descriptor + 0x10u, 0u);
            m.WriteU32(descriptor + 0x14u, 0u);
            SyntheticVramDescriptors.Add(descriptor);
            SelectorVramReservations.Add(new GuestVramReservation(
                new NativeVramAllocation(
                    request.Width,
                    request.Height,
                    request.AlignWidth,
                    request.AlignHeight,
                    request.LimitWidth,
                    request.LimitHeight),
                x,
                y,
                descriptor));
            ClaimedGuestVramReservations.Add(
                SelectorVramReservations.Count - 1);
            c.V0 = descriptor;
        }
        if (!_matchVramActive) return;

        if (c.V0 != 0u)
        {
            int success = ++_matchVramSuccesses;
            if (_traceVram || success <= 24)
            {
                Console.Error.WriteLine(
                    $"[V82VRAM] #{success} {request.Width}x{request.Height} " +
                    $"align={request.AlignWidth}x{request.AlignHeight} " +
                    $"limit={request.LimitWidth}x{request.LimitHeight} -> " +
                    $"({(short)m.ReadU16(c.V0)},{(short)m.ReadU16(c.V0 + 2u)}) " +
                    $"{m.ReadU16(c.V0 + 4u)}x{m.ReadU16(c.V0 + 6u)}");
            }
        }
        else
        {
            int failure = ++_matchVramFailures;
            if (_traceVram || failure <= 24 || failure % 100 == 0)
            {
                Console.Error.WriteLine(
                    $"[V82VRAM] failure #{failure}: {request.Width}x{request.Height}, " +
                    $"align={request.AlignWidth}x{request.AlignHeight}, " +
                    $"limit={request.LimitWidth}x{request.LimitHeight}");
            }
        }
    }

    public static bool RecoverMatchVramFailure(CpuContext c, IMemory m)
    {
        const uint outOfVramMessage = 0x8006B054u;
        if (!_matchVramActive)
            return true;

        string message;
        try
        {
            message = RecompOne.Runtime.Bios.Bios.ReadString(m, c.A0);
        }
        catch
        {
            message = "<unreadable>";
        }
        Console.Error.WriteLine(
            $"[V82Fatal] caller=0x{c.RA:X8} message=0x{c.A0:X8} " +
            $"'{message}' saved-ra=0x{m.ReadU32(c.SP + 0x18u):X8}");
        if (c.A0 != outOfVramMessage)
            return true;

        // Texture users in the match loader already test a null rectangle and
        // omit the affected texture. Let that recovery path run instead of
        // entering the retail fatal-screen infinite loop.
        return false;
    }

    public static void SpuMallocPre(CpuContext c, IMemory m) =>
        SpuMallocRequests.Push(c.A0);

    // The front end retains its preview bank until the match bank is opened.
    // Retail replaces those samples during the transition, but the static host
    // reaches SpuMalloc before the asynchronous teardown. Retry once with the
    // original 512 KiB allocator freshly initialized; audio transfer and
    // playback still run through the retail routines.
    public static void SpuMallocPost(CpuContext c, IMemory m)
    {
        uint requestedBytes = SpuMallocRequests.Count != 0
            ? SpuMallocRequests.Pop()
            : 0u;
        if (c.V0 != 0xFFFFFFFFu || _spuMallocRetrying) return;

        const uint table = 0x800BDD78u;
        uint shift = m.ReadU32(0x800641A8u);
        m.WriteU32(table, 0x40001010u);
        m.WriteU32(table + 4u, (0x00010000u << (int)(shift & 31u)) - 0x1010u);
        m.WriteU32(0x800641E8u, table);
        m.WriteU32(0x800641E4u, 0u);
        m.WriteU32(0x800641E0u, 16u);

        Console.Error.WriteLine(
            $"[V82Compat] reclaimed front-end SPU bank for {requestedBytes} byte match bank");
        c.A0 = requestedBytes;
        _spuMallocRetrying = true;
        try
        {
            Dispatcher.Call(c, m, 0x80052F9Cu);
        }
        finally
        {
            _spuMallocRetrying = false;
        }
    }

    // SHELL's hand-written VLC decoder crosses several internal tail-entry
    // points. Preserve the MIPS callee-saved contract around its high-level
    // image conversion caller even when those continuations are dispatched as
    // separate recompiled methods.
    public static bool PreserveShellDecodeCallerPre(CpuContext c, IMemory m)
    {
        if (!V82VehicleRegistry.BeginNativeSelectorPortrait(c, m))
            return false;
        ShellDecodeCallers.Push(
        [
            c.SP, c.RA, c.S0, c.S1, c.S2, c.S3, c.S4,
            c.S5, c.S6, c.S7, c.FP,
        ]);
        return true;
    }

    public static void PreserveShellDecodeCallerPost(CpuContext c, IMemory m)
    {
        if (ShellDecodeCallers.Count == 0) return;
        uint[] saved = ShellDecodeCallers.Pop();
        c.SP = saved[0];
        c.RA = saved[1];
        c.S0 = saved[2];
        c.S1 = saved[3];
        c.S2 = saved[4];
        c.S3 = saved[5];
        c.S4 = saved[6];
        c.S5 = saved[7];
        c.S6 = saved[8];
        c.S7 = saved[9];
        c.FP = saved[10];
    }

    // The image blitter stores its third argument in the caller-owned ABI area
    // above its frame, then makes several hand-written helper calls. Separate
    // recompilation of those continuations can reuse that slot. Preserve it
    // across the two heap allocations that precede the second read.
    public static void PreserveShellImageDecodePre(CpuContext c, IMemory m)
    {
        GpuHle.GameplayActive = false;
        if (_traceVram)
        {
            Console.Error.WriteLine(
                $"[ShellImageDecode] source=0x{c.A0:X8} " +
                $"target=0x{c.A1:X8} sourceRect=0x{c.A2:X8} " +
                $"flags=0x{c.A3:X8} parser=0x{c.GP + 0xE68u:X8} " +
                $"parserNext=0x{m.ReadU32(c.GP + 0xE7Cu):X8}");
        }
        ShellImageDecodeFrames.Push((c.SP - 0x50u, c.A2));
    }

    public static void PreserveShellImageDecodePost(CpuContext c, IMemory m)
    {
        if (ShellImageDecodeFrames.Count != 0)
            ShellImageDecodeFrames.Pop();
    }

    // SLUS_008.68 0x80018110 waits for the retail CD-ready interrupt to move
    // the producer pointer. Static recompilation has no asynchronous R3000
    // execution while this loop is active, so service one synchronous host
    // sector and retain the original producer/consumer return semantics.
    public static void WaitForSector(CpuContext c, IMemory m)
    {
        uint previous = m.ReadU32(c.GP + 0xD5Cu);
        uint producer = m.ReadU32(c.GP + 0xD60u);
        for (int attempt = 0; producer == previous && attempt < 8; attempt++)
        {
            LibCd.ServiceReadOnce();
            producer = m.ReadU32(c.GP + 0xD60u);
        }

        m.WriteU32(c.GP + 0xD5Cu, producer);
        c.V0 = previous;
    }

    // DrawOTag completes asynchronously on the PS1. RecompOne consumes the
    // ordering table synchronously, so deliver the sequel's registered
    // DrawSync callback before its front-end transition wait can spin.
    public static void ServiceDrawSyncWait(CpuContext c, IMemory m)
    {
        // These two callers are the alternate single-player draw paths in the
        // arena loop. Treat either as one gameplay heartbeat so deterministic
        // input and captures can start only after the live match is running.
        if (_gameplayFrameCount == 0 &&
            c.RA is 0x80014360u or 0x800146B4u)
        {
            int frame = ++_gameplayFrameCount;
            if (frame == 1)
                InputManager.SignalScriptStage("gameplay", captureDelayPolls: 60);
            if (frame == 1 || frame % 60 == 0)
            {
                uint player = m.ReadU32(0x8006BB58u);
                Console.Error.WriteLine(
                    $"[V82Gameplay] frame={frame} caller=0x{c.RA:X8} " +
                    $"clock={m.ReadU32(c.GP + 0x28u)} player=0x{player:X8}");
            }
        }

        if (m.ReadU32(c.GP + 0x614u) != 0u) return;

        const uint drawSyncCallbackSlot = 0x8006A4FCu;
        uint callback = m.ReadU32(drawSyncCallbackSlot);
        if (callback == 0u)
        {
            // Mid-match texture uploads can use this barrier without installing
            // the front-end DrawOTag callback. The host GPU operations are
            // synchronous, so their completion state is already satisfied.
            m.WriteU32(c.GP + 0x614u, 1u);
            return;
        }

        var snapshot = c.Snapshot();
        Dispatcher.Call(c, m, callback);
        c.Restore(snapshot);

        int frames = 0;
        while (m.ReadU32(c.GP + 0x614u) == 0u && frames < 16)
        {
            Runtime.PresentFrame();
            frames++;
        }

        if (m.ReadU32(c.GP + 0x614u) == 0u)
            throw new InvalidOperationException(
                $"Vigilante 8: 2nd Offense DrawSync wait did not complete after {frames} VSync frames");
    }

    // The display-fade callback advances gp+E98 from active to complete during
    // VSync. Static execution cannot receive that callback while 0x80021C24 is
    // spinning, so advance host frames until the original state reaches 2.
    public static void ServiceDisplayTransitionWait(CpuContext c, IMemory m)
    {
        ApplyConfiguredCheats(m);
        if (_soakEnabled)
            m.WriteU16(0x8006A832u, (ushort)(m.ReadU16(0x8006A832u) | 0x10u));
        if (_unlockRoster)
        {
            m.WriteU16(0x8006BAF8u, 0x01FF);
            LogRosterUnlock();
        }
        if (c.A0 != 0u) return;

        uint state = m.ReadU32(c.GP + 0xE98u);
        if (state == 0u || state == 2u) return;
        int frames = 0;
        while (m.ReadU32(c.GP + 0xE98u) != 2u && frames < 256)
        {
            Runtime.PresentFrame();
            frames++;
        }

        if (m.ReadU32(c.GP + 0xE98u) != 2u)
            throw new InvalidOperationException(
                $"Vigilante 8: 2nd Offense display transition did not complete after {frames} VSync frames");
    }

    static void LogRosterUnlock()
    {
        if (_unlockRosterLogged) return;
        _unlockRosterLogged = true;
        Console.Error.WriteLine(
            "[V82Compat] enabled all nine bonus drivers for this run");
    }

    const uint CheatFlagsAddress = 0x8006A830u;
    const uint ReducedGravityAddress = 0x8006A82Cu;
    const uint UpgradeTableAddress = 0x8006B9E8u;

    public static void SetConfiguredCheatFlags(uint flags)
    {
        ConfigManager.Game.V82CheatFlags = flags & 0x001FFFFFu;
        ConfigManager.SaveGame();
        if (Runtime.Mem is { } memory)
            ApplyConfiguredCheats(memory);
    }

    static void ApplyConfiguredCheats(IMemory memory)
    {
        IMemory m = Dispatcher.UnwrapMemory(memory);
        uint flags = ConfigManager.Game.V82CheatFlags & 0x001FFFFFu;
        m.WriteU32(CheatFlagsAddress, flags);
        if (_lastLoggedCheatFlags != flags)
        {
            _lastLoggedCheatFlags = flags;
            Console.Error.WriteLine(
                $"[V82Cheats] native-flags=0x{flags:X6} " +
                $"drive-only={((flags & (1u << 6)) != 0)} " +
                $"old-levels={((flags & (1u << 20)) != 0)}");
        }

        // These are the two extra writes performed by the retail password
        // handler in addition to setting its corresponding flag bit.
        if ((flags & (1u << 1)) != 0)
            m.WriteU32(ReducedGravityAddress, 0x1680u);
        if ((flags & (1u << 7)) != 0)
            SetAllVehicleUpgradeStats(m, 100);
        else if ((flags & (1u << 8)) != 0)
            SetAllVehicleUpgradeStats(m, 50);
    }

    static void SetAllVehicleUpgradeStats(IMemory m, byte value)
    {
        for (uint vehicle = 0; vehicle < 18; vehicle++)
        {
            uint record = UpgradeTableAddress + vehicle * 10u;
            m.WriteU8(record + 6u, value);
            m.WriteU8(record + 7u, value);
            m.WriteU8(record + 8u, value);
            m.WriteU8(record + 9u, value);
        }
    }

    // The sequel links a newer card library at different SHELL addresses, but
    // retains the original asynchronous state layout. Service its callback
    // synchronously so a blocking card query cannot spin forever.
    public static void WaitCardOperation(CpuContext c, IMemory m)
    {
        const uint state = 0x80116870u;
        uint nonblocking = c.A0;
        uint result1 = c.A1;
        uint result2 = c.A2;
        if (m.ReadU32(state) == 0u && m.ReadU32(state + 8u) == 0u)
        {
            c.V0 = 0xFFFFFFFFu;
            return;
        }

        if (m.ReadU32(state + 8u) == 0u)
            V8Compat.DispatchLinked(c, m, 0x80114418u);

        uint complete = m.ReadU32(state + 8u);
        if (nonblocking == 0u && complete == 0u)
        {
            for (int i = 0; i < 8 && m.ReadU32(state + 8u) == 0u; i++)
                V8Compat.DispatchLinked(c, m, 0x80114418u);
            complete = m.ReadU32(state + 8u);
        }

        if (complete != 0u)
        {
            if (result2 != 0u) m.WriteU32(result2, m.ReadU32(state - 0xCu));
            if (result1 != 0u) m.WriteU32(result1, m.ReadU32(state - 0x10u));
            m.WriteU32(state + 8u, 0u);
            c.V0 = 1u;
            return;
        }

        if (result2 != 0u) m.WriteU32(result2, m.ReadU32(state + 4u));
        if (result1 != 0u) m.WriteU32(result1, m.ReadU32(state));
        c.V0 = nonblocking == 0u ? 1u : 0u;
    }
}
