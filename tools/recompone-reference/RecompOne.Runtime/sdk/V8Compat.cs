using RecompOne.Runtime.Context;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class V8Compat
{
    readonly record struct HeapAllocation(uint Units, uint RequestedBytes, uint Caller, int Operation);
    readonly record struct GuestVramReservation(
        NativeVramAllocation Request,
        uint X,
        uint Y,
        uint Descriptor);

    static bool _heapCycleLogged;
    static bool _heapExhaustedLogged;
    static int _vramAllocCount;
    static readonly List<GuestVramReservation> GuestVramReservations = [];
    static readonly HashSet<int> ClaimedGuestVramReservations = [];
    static readonly HashSet<uint> SyntheticVramDescriptors = [];
    static bool _guestVramClaimActive;
    static int _guestVramClaimIndex;
    static int _binParseCount;
    static (uint Destination, uint Size, uint Offset, uint Source)? _largeStreamCopy;
    static int _heapOperation;
    static int _objectEventSerial;
    static bool _linkedOverlayRangesReserved;
    static readonly bool _traceMenuText =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_MENU_TEXT") == "1";
    static readonly bool _zeroGameVolume =
        Environment.GetEnvironmentVariable("RECOMPONE_V8_GAME_VOLUME") == "0";
    static readonly bool _traceVehicle =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_VEHICLE") == "1";
    static readonly bool _traceWeapons =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_WEAPONS") == "1";
    static readonly bool _traceResults =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_RESULTS") == "1";
    static readonly bool _traceOptions =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_OPTIONS") == "1";
    static readonly bool _traceAnimation =
        Environment.GetEnvironmentVariable("RECOMPONE_TRACE_ANIMATION") == "1";
    static readonly bool _validateHeap =
        Environment.GetEnvironmentVariable("RECOMPONE_VALIDATE_HEAP") == "1";
    static readonly bool _victoryAutopilot =
        Environment.GetEnvironmentVariable("RECOMPONE_V8_VICTORY_AUTOPILOT") == "1";
    static readonly bool _whammyMatrix =
        Environment.GetEnvironmentVariable("RECOMPONE_V8_WHAMMY_MATRIX") == "1";
    static readonly string? _stateTracePath =
        Environment.GetEnvironmentVariable("RECOMPONE_STATE_TRACE_PATH");
    static readonly int _soakHeartbeatTicks =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_SOAK_HEARTBEAT_TICKS"),
            out int heartbeatTicks)
            ? Math.Max(1, heartbeatTicks)
            : 0;
    static readonly string? _automationTargetLocation =
        Environment.GetEnvironmentVariable("RECOMPONE_TARGET_LOCATION");
    static readonly string? _automationTargetCharacter =
        Environment.GetEnvironmentVariable("RECOMPONE_TARGET_CHARACTER");
    static readonly string? _automationTargetTwoPlayerMode =
        Environment.GetEnvironmentVariable("RECOMPONE_TARGET_TWO_PLAYER_MODE");
    static readonly int _automationGameplayCaptureDelay =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_GAMEPLAY_CAPTURE_DELAY_POLLS"),
            out int gameplayCaptureDelay)
            ? Math.Max(0, gameplayCaptureDelay)
            : 0;
    static readonly int _soakTeardownTick =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_SOAK_TEARDOWN_TICKS"),
            out int soakTeardownTick)
            ? Math.Max(0, soakTeardownTick)
            : 0;
    static bool _soakTeardownSignaled;
    static int _vehiclePhysicsTick;
    static int _gameplayHeartbeatTick;
    static uint _lastGameplayFrameCounter = uint.MaxValue;
    static uint _integratingVehicle;
    static int _integratingVehicleTick;
    static uint _collidingVehicle;
    static int _collidingVehicleTick;
    static int _playerCollisionCount;
    static byte _collidingOtherKind;
    static bool _capturedVehicleCollision;
    static string? _lastWeaponState;
    static int _weaponStateChangeCount;
    static bool _pendingPlayerChildSpawn;
    static uint _pendingPlayerSpawnWeapon;
    static uint _pendingPlayerSpawnKind;
    static uint _pendingPlayerSpawnCaller;
    static uint _pendingPlayerHitProjectile;
    static uint _pendingPlayerHitTarget;
    static string? _pendingPlayerHitState;
    static uint _pendingVehicleDamageTarget;
    static uint _pendingVehicleDamageCaller;
    static int _pendingVehicleDamageImpulse;
    static string? _pendingVehicleDamageState;
    static int _vehicleDamageChangeCount;
    static uint _victoryAutopilotTarget;
    static ushort _victoryAutopilotInput;
    static readonly int[] _whammyMatrixKinds = BuildWhammyMatrixKinds();
    static int _whammyMatrixIndex;
    static int _whammyMatrixCurrentKind;
    static uint _whammyMatrixCurrentWeapon;
    static int _whammyMatrixCurrentSlot;
    static int _whammyMatrixPartnerKind;
    static uint _whammyMatrixPartnerWeapon;
    static int _whammyMatrixPartnerSlot;
    static ushort _whammyMatrixArmedAmmo;
    static int _whammyMatrixCurrentAttachTick;
    static int _whammyMatrixNextAttachTick;
    static bool _whammyMatrixMutating;
    static uint _pendingWhammyOwner;
    static byte _pendingWhammyCount;
    static int _pendingWhammyKind;
    static uint _pendingWhammyProjectile;
    static uint _pendingWhammyTarget;
    static int _pendingDirectWhammyKind;
    static uint _pendingDirectWhammyProjectile;
    static uint _pendingDirectWhammyTarget;
    static int _pendingDirectWhammyTick;
    static byte _lastPlayerWhammyCount;
    static uint _pendingSpecialWeapon;
    static int _pendingSpecialKind;
    static uint _pendingSpecialCode;
    static ushort _pendingSpecialAmmo;
    static int _activeMatrixWeaponKind;
    static uint _lastSpecialCommandCode = uint.MaxValue;
    static uint _lastSpecialCommandFlags = uint.MaxValue;
    static int _lastInjectedSpecialAttachTick = -1;
    static int _lastInjectedSpecialOpportunity = -1;
    static readonly HashSet<int> _whammyMatrixFiredKinds = new();
    static readonly HashSet<int> _whammyMatrixConfirmedKinds = new();
    static readonly HashSet<string> _specialAttackCoverage = new();
    static readonly Dictionary<int, uint[]> _specialAttackCodes = new()
    {
        [1] = [0x422u, 0x424u],
        [2] = [0x442u, 0x444u],
        [3] = [0x242u, 0x244u],
        [4] = [0x222u, 0x224u],
        [5] = [0x132u, 0x134u],
    };
    static bool _capturedPlayerProjectile;
    static bool _capturedPlayerWeaponHit;
    static bool _capturedVehicleDestruction;
    static bool _resultScreenReached;
    static bool _gameplayStage;
    static bool _optionsMenuActive;
    static bool _passcodeEditorVisited;
    static int _passcodeEntryCount;
    static bool _pauseResumePending;
    static int _pauseScreenCount;
    static int _pauseConfirmCount;
    static int _pauseResumeCount;
    static int _optionsAudioSetCount;
    static ushort _lastPauseRawPad = 0xFFFF;
    static uint _lastPauseMappedPad;
    static int _lastScreenX = int.MinValue;
    static int _lastScreenY = int.MinValue;
    static bool _zeroGameVolumeLogged;
    static string _lastHeapOperation = "heap initialization";
    static string? _lastMenuStage;
    static readonly Dictionary<uint, HeapAllocation> _liveHeapAllocations = new();
    static readonly Dictionary<uint, List<string>> _objectHistory = new();
    static readonly Dictionary<uint, (uint HighMesh, uint LowMesh, uint Threshold)> _lodThresholds = new();
    static readonly HashSet<uint> _playerProjectiles = new();
    static readonly Dictionary<uint, int> _matrixWeaponKinds = new();
    static readonly Dictionary<uint, int> _playerProjectileMatrixKinds = new();
    static readonly Dictionary<byte, int> _matrixProjectileTypeKinds = new();
    static readonly HashSet<string> _seenMenuText = new(StringComparer.Ordinal);
    static readonly Dictionary<uint, uint> _lastAnimationPointers = new();
    static Timer? _animationWatchdog;
    static CpuContext? _animationWatchContext;
    static IMemory? _animationWatchMemory;
    static uint _cursorClearingAnimationObject;
    static uint _cursorClearingAnimationMode;
    static bool _cursorClearingAnimationFixLogged;
    static uint _childCursorClearingAnimationObject;
    static bool _childCursorClearingAnimationFixLogged;
    static CpuContext? _collisionNeighborContext;
    static uint _collisionNeighborSubject;
    static bool _collisionFaultTraceActive;
    static bool _missingDiagonalNeighborFixLogged;
    static StreamWriter? _stateTraceWriter;
    static bool _stateTraceUnavailable;

    static int[] BuildWhammyMatrixKinds()
    {
        int start = int.TryParse(
            Environment.GetEnvironmentVariable("RECOMPONE_V8_WHAMMY_START_KIND"),
            out int configured)
            ? Math.Clamp(configured, 1, 6)
            : 1;
        return Enumerable.Range(0, 6)
            .Select(offset => ((start - 1 + offset) % 6) + 1)
            .ToArray();
    }

    public static void TraceAnimationObject(CpuContext c, IMemory m)
    {
        if (!_traceAnimation) return;
        m = Dispatcher.UnwrapMemory(m);
        _animationWatchContext = c;
        _animationWatchMemory = m;
        _animationWatchdog ??= new Timer(
            _ => TraceAnimationStall(), null, Timeout.Infinite, Timeout.Infinite);
        _animationWatchdog.Change(5000, Timeout.Infinite);

        uint obj = c.A0;
        uint pointer = m.ReadU32(obj + 0x60u);
        _lastAnimationPointers.TryGetValue(obj, out uint previous);
        _lastAnimationPointers[obj] = pointer;
        if (pointer == 0u || pointer >= 0x80000000u) return;

        string history = _objectHistory.TryGetValue(obj, out var events)
            ? string.Join(" | ", events.TakeLast(8))
            : "none";
        Console.Error.WriteLine(
            $"[V8Animation] object=0x{obj:X8} pointer=0x{pointer:X8} previous=0x{previous:X8} " +
            $"flags=0x{m.ReadU32(obj):X8} id={m.ReadU16(obj + 0x0Au)} " +
            $"parent=0x{m.ReadU32(obj + 0x3Cu):X8} child=0x{m.ReadU32(obj + 0x38u):X8} " +
            $"next=0x{m.ReadU32(obj + 0x34u):X8} callback=0x{m.ReadU32(obj + 0x64u):X8} " +
            $"history={history}");
    }

    public static void TraceAnimationObjectPost(CpuContext c, IMemory m)
    {
        if (!_traceAnimation) return;
        _animationWatchdog?.Change(Timeout.Infinite, Timeout.Infinite);
        _animationWatchContext = null;
        _animationWatchMemory = null;
    }

    static void TraceAnimationStall()
    {
        CpuContext? c = _animationWatchContext;
        IMemory? m = _animationWatchMemory;
        if (c == null || m == null) return;
        uint obj = c.S2;
        uint cursor = c.S1;
        uint pointer = m.ReadU32(obj + 0x60u);
        string history = _objectHistory.TryGetValue(obj, out var events)
            ? string.Join(" | ", events.TakeLast(8))
            : "none";
        Console.Error.WriteLine(
            $"[V8AnimationStall] object=0x{obj:X8} target={c.S6 & 0xFFFFu} " +
            $"frame={m.ReadU16(obj + 0x46u)} pointer=0x{pointer:X8} cursor=0x{cursor:X8} " +
            $"flags=0x{c.S3:X8} s0=0x{c.S0:X8} v0=0x{c.V0:X8} v1=0x{c.V1:X8} " +
            $"entry=({m.ReadU16(pointer)},{m.ReadU16(pointer + 2u)}) " +
            $"cursorWords=({m.ReadU16(cursor)},{m.ReadU16(cursor + 2u)}) " +
            $"objectFlags=0x{m.ReadU32(obj):X8} id={m.ReadU16(obj + 0x0Au)} " +
            $"callback=0x{m.ReadU32(obj + 0x64u):X8} history={history}");
    }

    public static void TraceCursorClearingAnimationPre(CpuContext c, IMemory m)
    {
        _cursorClearingAnimationObject = c.A0;
        _cursorClearingAnimationMode = c.A1;
    }

    public static void FixCursorClearingAnimationEnd(CpuContext c, IMemory m)
    {
        if (_cursorClearingAnimationMode != 5u || _cursorClearingAnimationObject == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        if (m.ReadU32(_cursorClearingAnimationObject + 0x60u) != 0u) return;

        // Some arena callbacks clear their animation cursor on event 5 but
        // return zero. Object_PreTick then follows address zero as if it were
        // another keyframe. Return the animation walker's documented negative
        // abort value only for this exact end-of-sequence state.
        c.V0 = 0xFFFFFFFFu;
        if (!_cursorClearingAnimationFixLogged)
        {
            _cursorClearingAnimationFixLogged = true;
            Console.Error.WriteLine(
                $"[V8Compat] cursor-clearing animation ended cleanly " +
                $"object=0x{_cursorClearingAnimationObject:X8} " +
                $"callback=0x{m.ReadU32(_cursorClearingAnimationObject + 0x64u):X8}");
        }
    }

    public static void TraceChildCursorClearingAnimationPre(CpuContext c, IMemory m)
    {
        _childCursorClearingAnimationObject = 0u;
        if (c.A1 != 5u || c.A0 == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        _childCursorClearingAnimationObject = m.ReadU32(c.A0 + 0x38u);
    }

    public static void FixChildCursorClearingAnimationEnd(CpuContext c, IMemory m)
    {
        uint obj = _childCursorClearingAnimationObject;
        if (obj == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        if (m.ReadU32(obj + 0x60u) != 0u) return;

        // Main callback 0x800378D0 event 5 dispatches through 0x80037ADC,
        // which clears the child object's animation cursor and returns zero.
        // Abort the outer animation walk for that exact cleared-child state.
        c.V0 = 0xFFFFFFFFu;
        if (!_childCursorClearingAnimationFixLogged)
        {
            _childCursorClearingAnimationFixLogged = true;
            Console.Error.WriteLine(
                $"[V8Compat] child cursor-clearing animation ended cleanly " +
                $"object=0x{obj:X8} callback=0x800378D0");
        }
    }

    public static void TraceCollisionNeighborScanPre(CpuContext c, IMemory m)
    {
        _collisionNeighborContext = c;
        _collisionNeighborSubject = c.A0;
    }

    public static void TraceCollisionNeighborScanPost(CpuContext c, IMemory m)
    {
        _collisionNeighborContext = null;
        _collisionNeighborSubject = 0u;
    }

    public static void TraceUnmappedMemoryAddress(uint address, int size)
    {
        CpuContext? c = _collisionNeighborContext;
        if (c == null || _collisionFaultTraceActive) return;
        _collisionFaultTraceActive = true;
        try
        {
            Console.Error.WriteLine(
                $"[V8CollisionFault] address=0x{address:X8} size={size} " +
                $"subject=0x{_collisionNeighborSubject:X8} " +
                $"a0=0x{c.A0:X8} a1=0x{c.A1:X8} a2=0x{c.A2:X8} a3=0x{c.A3:X8} " +
                $"v0=0x{c.V0:X8} v1=0x{c.V1:X8} " +
                $"s0=0x{c.S0:X8} s1=0x{c.S1:X8} s2=0x{c.S2:X8} " +
                $"s3=0x{c.S3:X8} s4=0x{c.S4:X8} " +
                $"t0=0x{c.T0:X8} t1=0x{c.T1:X8} t2=0x{c.T2:X8} " +
                $"t3=0x{c.T3:X8} sp=0x{c.SP:X8} ra=0x{c.RA:X8}");
        }
        finally
        {
            _collisionFaultTraceActive = false;
        }
    }

    public static void FixMissingDiagonalCollisionNeighbor(CpuContext c, IMemory m)
    {
        if (c.RA != 0x80024C70u || c.V0 != 0u) return;
        m = Dispatcher.UnwrapMemory(m);

        // FUN_80024998's diagonal-neighbor case omits the null check used by
        // its four axial cases. Supply any live pool cell whose terrain flag
        // is zero, which makes the original caller take its existing
        // "no neighbor to link" branch without mutating the cell.
        const uint poolBase = 0x800738A0u;
        const uint poolStride = 0x1Cu;
        for (uint i = 0; i < 0x400u; i++)
        {
            uint node = poolBase + i * poolStride;
            uint data = m.ReadU32(node + 0x08u);
            uint phys = data & 0x1FFFFFFFu;
            if (data == 0u || phys >= 0x00200000u) continue;
            uint slot = m.ReadU8(node + 0x10u);
            if (m.ReadU16(data + slot * 2u + 2u) != 0u) continue;

            c.V0 = node;
            if (!_missingDiagonalNeighborFixLogged)
            {
                _missingDiagonalNeighborFixLogged = true;
                Console.Error.WriteLine(
                    $"[V8Compat] missing diagonal collision neighbor treated as empty " +
                    $"subject=0x{c.S1:X8} sentinel=0x{node:X8}");
            }
            return;
        }

        Console.Error.WriteLine(
            $"[V8CollisionFault] no empty pool cell for missing diagonal neighbor " +
            $"subject=0x{c.S1:X8}");
    }

    public static void Alloc(CpuContext c, IMemory m)
    {
        V8VehicleRegistry.Initialize(c, m);
        AllocFromHead(c, m, 0x8005ED4Cu);
    }

    internal static void AllocFromHead(
        CpuContext c, IMemory m, uint headAddress, bool reserveLinkedOverlayRanges = true)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint requestedBytes = c.A0;
        if (requestedBytes == 0u)
        {
            c.V0 = 0u;
            return;
        }

        uint units = (requestedBytes + 15u) >> 3;
        if (reserveLinkedOverlayRanges)
            ReserveLinkedOverlayRanges(m, headAddress);
        int operation = ++_heapOperation;
        bool validateNow = _validateHeap &&
            (operation == 1 || (operation & 0xFF) == 0);
        uint predecessor = m.ReadU32(headAddress);
        uint head = predecessor;
        var visited = new HashSet<uint>();

        while (visited.Add(predecessor))
        {
            uint block = m.ReadU32(predecessor);
            uint available = m.ReadU32(block + 4u);
            int remaining = unchecked((int)(available - units));
            if (remaining >= 0)
            {
                if (remaining == 0)
                {
                    m.WriteU32(headAddress, predecessor);
                    m.WriteU32(predecessor, m.ReadU32(block));
                }
                else
                {
                    m.WriteU32(block + 4u, (uint)remaining);
                    block += (uint)remaining << 3;
                    m.WriteU32(block + 4u, units);
                    m.WriteU32(headAddress, predecessor);
                }
                c.V0 = block + 8u;
                if (_validateHeap)
                {
                    _liveHeapAllocations[c.V0] = new HeapAllocation(units, requestedBytes, c.RA, operation);
                    _lastHeapOperation = $"alloc #{operation} ptr=0x{c.V0:X8} bytes={requestedBytes} units={units} caller=0x{c.RA:X8}";
                    if (validateNow)
                    {
                        ValidateFreeList(m, $"after {_lastHeapOperation}");
                        ValidateLiveAllocations(m, $"after {_lastHeapOperation}");
                    }
                }
                return;
            }

            if (block == head)
            {
                LogHeapExhausted(m, requestedBytes, headAddress);
                c.V0 = 0u;
                return;
            }
            predecessor = block;
        }

        if (!_heapCycleLogged)
        {
            _heapCycleLogged = true;
            Console.Error.WriteLine($"[V8Compat] corrupt heap free-list cycle: request={requestedBytes} head=0x{head:X8} repeated=0x{predecessor:X8}");
        }
        c.V0 = 0u;
    }

    public static void Free(CpuContext c, IMemory m)
    {
        FreeFromHead(c, m, 0x8005ED4Cu);
    }

    internal static void FreeFromHead(CpuContext c, IMemory m, uint headAddress)
    {
        m = Dispatcher.UnwrapMemory(m);
        if (c.A0 == 0u) return;

        Dispatcher.ForgetObjectOwner(c.A0);
        if (_objectHistory.ContainsKey(c.A0))
            RecordObjectEvent(c.A0, $"heap-free caller=0x{c.RA:X8}");

        int operation = ++_heapOperation;
        if (_validateHeap)
            ReconcileFreedAllocation(m, c.A0, operation, c.RA);
        bool validateNow = _validateHeap &&
            (operation == 1 || (operation & 0xFF) == 0);
        uint block = c.A0 - 8u;
        uint predecessor = m.ReadU32(headAddress);
        var visited = new HashSet<uint>();

        while (visited.Add(predecessor))
        {
            uint successor = m.ReadU32(predecessor);
            bool between = predecessor < block && block < successor;
            bool wrapped = predecessor >= successor && (block > predecessor || block < successor);
            if (between || wrapped)
            {
                uint units = m.ReadU32(block + 4u);
                if (block + (units << 3) == successor && m.ReadU32(successor + 4u) != 0u)
                {
                    units += m.ReadU32(successor + 4u);
                    successor = m.ReadU32(successor);
                    m.WriteU32(block + 4u, units);
                }

                uint predecessorUnits = m.ReadU32(predecessor + 4u);
                m.WriteU32(block, successor);
                if (predecessor + (predecessorUnits << 3) == block)
                {
                    m.WriteU32(predecessor + 4u, predecessorUnits + units);
                    block = successor;
                }

                m.WriteU32(headAddress, predecessor);
                m.WriteU32(predecessor, block);
                if (_validateHeap)
                {
                    _lastHeapOperation = $"free #{operation} ptr=0x{c.A0:X8} caller=0x{c.RA:X8}";
                    if (validateNow)
                    {
                        ValidateFreeList(m, $"after {_lastHeapOperation}");
                        ValidateLiveAllocations(m, $"after {_lastHeapOperation}");
                    }
                }
                return;
            }
            predecessor = successor;
        }

        Console.Error.WriteLine($"[V8Compat] corrupt heap while freeing 0x{c.A0:X8}: repeated=0x{predecessor:X8}");
    }

    public static void TraceObjectSchedule(CpuContext c, IMemory m)
    {
        RecordObjectEvent(c.A0,
            $"schedule tick={m.ReadU32(c.GP + 0x0Cu)} delay={c.A1} " +
            $"flags=0x{m.ReadU32(c.A0):X8} cb=0x{m.ReadU32(c.A0 + 0x64u):X8} caller=0x{c.RA:X8}");
    }

    public static void TraceObjectRetire(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        _matrixWeaponKinds.Remove(c.A0);
        RecordObjectEvent(c.A0,
            $"retire flags=0x{m.ReadU32(c.A0):X8} cb=0x{m.ReadU32(c.A0 + 0x64u):X8} caller=0x{c.RA:X8}");
        if ((_traceWeapons || _whammyMatrix) && _playerProjectiles.Remove(c.A0))
        {
            _playerProjectileMatrixKinds.Remove(c.A0);
            if (_traceWeapons)
            {
                Console.Error.WriteLine(
                    $"[V8Weapon] tick={_vehiclePhysicsTick} event=retire projectile=0x{c.A0:X8} " +
                    $"flags=0x{m.ReadU32(c.A0):X8} state8={m.ReadU8(c.A0 + 8u)} " +
                    $"callback=0x{m.ReadU32(c.A0 + 0x64u):X8} pos={ReadVec3(m, c.A0 + 0x24u)}");
            }
        }
    }

    public static void TracePlayerProjectileRegister(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        if (_whammyMatrix &&
            _activeMatrixWeaponKind != 0 &&
            IsRetailRamRange(c.A0, 0x84u))
        {
            uint player = m.ReadU32(c.GP + 0x7D0u);
            if (m.ReadU32(c.A0 + 0x80u) == player &&
                (m.ReadU32(c.A0) & 0x800000u) != 0u)
            {
                int kind = _activeMatrixWeaponKind;
                _playerProjectiles.Add(c.A0);
                _playerProjectileMatrixKinds[c.A0] = kind;
                _matrixProjectileTypeKinds[m.ReadU8(c.A0 + 0x0Au)] = kind;
                if (_whammyMatrixFiredKinds.Add(kind))
                {
                    Console.Error.WriteLine(
                        $"[V8Whammy] tick={_gameplayHeartbeatTick} " +
                        $"event=fired weaponKind={kind} projectile=0x{c.A0:X8}");
                }
            }
        }
        if (!_traceWeapons || !_playerProjectiles.Contains(c.A0)) return;
        Console.Error.WriteLine(
            $"[V8Weapon] tick={_vehiclePhysicsTick} event=register projectile=0x{c.A0:X8} " +
            $"flags=0x{m.ReadU32(c.A0):X8} state8={m.ReadU8(c.A0 + 8u)} " +
            $"callback=0x{m.ReadU32(c.A0 + 0x64u):X8} life={m.ReadU16(c.A0 + 0x94u)} " +
            $"pos={ReadVec3(m, c.A0 + 0x24u)} vel={ReadVec3(m, c.A0 + 0x88u)}");
        if (!_capturedPlayerProjectile)
        {
            _capturedPlayerProjectile = true;
            HostWindow.RequestDisplayCapture("weapon_projectile_first");
        }
    }

    public static void TrackObjectOwner(CpuContext c, IMemory m) =>
        Dispatcher.AssociateObjectOwner(c.A0, m);

    public static void TrackReturnedObjectOwner(CpuContext c, IMemory m) =>
        Dispatcher.AssociateObjectOwner(c.V0, m);

    static void RecordObjectEvent(uint address, string detail)
    {
        if (!_objectHistory.TryGetValue(address, out var history))
        {
            history = new List<string>(8);
            _objectHistory[address] = history;
        }
        history.Add($"#{++_objectEventSerial} {detail}");
        if (history.Count > 12) history.RemoveAt(0);
    }

    public static string DescribeObjectHistory(uint address) =>
        _objectHistory.TryGetValue(address, out var history)
            ? string.Join(" | ", history)
            : "no recorded schedule/retire/free history";

    static void ReserveLinkedOverlayRanges(IMemory m, uint headAddress)
    {
        if (_linkedOverlayRangesReserved) return;

        foreach (var range in Dispatcher.LinkedAddressRanges()
                     .OrderByDescending(range => range.Size))
        {
            uint start = range.Base & ~7u;
            uint end = (range.Base + range.Size + 7u) & ~7u;
            if (end <= start) continue;
            CarveFreeRange(m, headAddress, start, end);
        }

        _linkedOverlayRangesReserved = true;
    }

    internal static void ReserveLinkedOverlayRangesForHeap(
        IMemory m, uint headAddress) =>
        ReserveLinkedOverlayRanges(Dispatcher.UnwrapMemory(m), headAddress);

    internal static void ReserveHeapRange(
        IMemory m, uint headAddress, uint reservedStart, uint reservedEnd)
    {
        if (_linkedOverlayRangesReserved) return;
        CarveFreeRange(
            Dispatcher.UnwrapMemory(m), headAddress, reservedStart, reservedEnd);
        _linkedOverlayRangesReserved = true;
    }

    static void CarveFreeRange(IMemory m, uint headAddress, uint reservedStart, uint reservedEnd)
    {
        uint head = m.ReadU32(headAddress);
        uint predecessor = head;
        var visited = new HashSet<uint>();

        while (visited.Add(predecessor))
        {
            uint block = m.ReadU32(predecessor);
            uint units = m.ReadU32(block + 4u);
            ulong blockStart = block;
            ulong blockEnd = blockStart + ((ulong)units << 3);
            ulong cutStart = Math.Max(blockStart, reservedStart);
            ulong cutEnd = Math.Min(blockEnd, reservedEnd);

            if (units != 0u && cutStart < cutEnd)
            {
                uint next = m.ReadU32(block);
                uint beforeUnits = (uint)((cutStart - blockStart) >> 3);
                uint afterUnits = (uint)((blockEnd - cutEnd) >> 3);
                uint after = (uint)cutEnd;

                if (beforeUnits != 0u && afterUnits != 0u)
                {
                    m.WriteU32(block + 4u, beforeUnits);
                    m.WriteU32(after, next);
                    m.WriteU32(after + 4u, afterUnits);
                    m.WriteU32(block, after);
                }
                else if (beforeUnits != 0u)
                {
                    m.WriteU32(block + 4u, beforeUnits);
                }
                else if (afterUnits != 0u)
                {
                    m.WriteU32(after, next);
                    m.WriteU32(after + 4u, afterUnits);
                    m.WriteU32(predecessor, after);
                    if (m.ReadU32(headAddress) == block) m.WriteU32(headAddress, after);
                }
                else
                {
                    m.WriteU32(predecessor, next);
                    if (m.ReadU32(headAddress) == block) m.WriteU32(headAddress, predecessor);
                }

                Console.Error.WriteLine(
                    $"[V8Compat] reserved overlay link range 0x{reservedStart:X8}-0x{reservedEnd:X8} from heap");
                return;
            }

            predecessor = block;
            if (predecessor == head) return;
        }
    }

    static void ReconcileFreedAllocation(IMemory m, uint pointer, int operation, uint caller)
    {
        if (_liveHeapAllocations.Remove(pointer)) return;

        // realloc() shrinks in place by writing a smaller size into the original
        // header, fabricating a tail block immediately after it, and passing that
        // tail to free(). Account for that original-library behavior here.
        foreach (var pair in _liveHeapAllocations.ToArray())
        {
            uint originalPointer = pair.Key;
            HeapAllocation original = pair.Value;
            uint currentUnits = m.ReadU32(originalPointer - 4u);
            if (currentUnits < original.Units && pointer == originalPointer + (currentUnits << 3))
            {
                uint tailUnits = m.ReadU32(pointer - 4u);
                uint expectedTailUnits = original.Units - currentUnits;
                if (tailUnits != expectedTailUnits)
                    ThrowHeapCorruption($"realloc tail 0x{pointer:X8} has {tailUnits} units, expected {expectedTailUnits}; free #{operation} caller=0x{caller:X8}");

                _liveHeapAllocations[originalPointer] = original with { Units = currentUnits };
                return;
            }
        }

        ThrowHeapCorruption($"untracked or double free 0x{pointer:X8}; free #{operation} caller=0x{caller:X8}");
    }

    static void ValidateLiveAllocations(IMemory m, string context)
    {
        foreach (var pair in _liveHeapAllocations)
        {
            uint pointer = pair.Key;
            HeapAllocation allocation = pair.Value;
            if (!IsRetailRamRange(pointer - 8u, 8u))
                ThrowHeapCorruption($"tracked allocation pointer 0x{pointer:X8} is outside retail RAM; {context}");

            uint actualUnits = m.ReadU32(pointer - 4u);
            if (actualUnits != allocation.Units)
            {
                ThrowHeapCorruption(
                    $"allocation header changed at 0x{pointer - 8u:X8}: actualUnits={actualUnits} expectedUnits={allocation.Units} " +
                    $"allocatedBy=#{allocation.Operation}/0x{allocation.Caller:X8}/bytes={allocation.RequestedBytes}; {context}");
            }
        }
    }

    static void ValidateFreeList(IMemory m, string context)
    {
        const uint headAddress = 0x8005ED4Cu;
        uint head = m.ReadU32(headAddress);
        if (!IsRetailRamRange(head, 8u))
            ThrowHeapCorruption($"free-list head 0x{head:X8} is outside retail RAM; {context}");

        uint cursor = head;
        var visited = new HashSet<uint>();
        while (visited.Add(cursor))
        {
            if (!IsRetailRamRange(cursor, 8u))
                ThrowHeapCorruption($"free-list node 0x{cursor:X8} is outside retail RAM; {context}");

            uint next = m.ReadU32(cursor);
            if (!IsRetailRamRange(next, 8u))
                ThrowHeapCorruption($"free-list node 0x{cursor:X8} points outside retail RAM to 0x{next:X8}; {context}");

            uint units = m.ReadU32(next + 4u);
            uint physical = MemoryMap.ToPhysical(next);
            ulong end = (ulong)physical + ((ulong)units << 3);
            if (units != 0u && end > MemoryMap.RetailRamSize)
            {
                ThrowHeapCorruption(
                    $"free-list block 0x{next:X8} has invalid units={units} end=0x{end:X}; predecessor=0x{cursor:X8}; {context}");
            }

            if (units != 0u)
            {
                foreach (var live in _liveHeapAllocations)
                {
                    uint liveStart = MemoryMap.ToPhysical(live.Key - 8u);
                    ulong liveEnd = (ulong)liveStart + ((ulong)live.Value.Units << 3);
                    if ((ulong)physical < liveEnd && (ulong)liveStart < end)
                    {
                        ThrowHeapCorruption(
                            $"free-list block 0x{next:X8} units={units} overlaps live allocation 0x{live.Key:X8} " +
                            $"units={live.Value.Units} allocatedBy=#{live.Value.Operation}/0x{live.Value.Caller:X8}; {context}");
                    }
                }
            }

            cursor = next;
            if (cursor == head) return;
        }

        ThrowHeapCorruption($"free-list cycle repeats 0x{cursor:X8} without returning to head 0x{head:X8}; {context}");
    }

    static bool IsRetailRamRange(uint address, uint size)
    {
        if (address == 0u) return false;
        uint physical = MemoryMap.ToPhysical(address);
        return physical < MemoryMap.RetailRamSize && size <= MemoryMap.RetailRamSize - physical;
    }

    static void ThrowHeapCorruption(string detail)
    {
        throw new InvalidOperationException($"Vigilante 8 heap corruption: {detail}; last={_lastHeapOperation}; live={_liveHeapAllocations.Count}");
    }

    public static void Fatal(CpuContext c, IMemory m)
    {
        uint address = c.A0;
        var chars = new List<byte>(128);
        for (int i = 0; i < 256; i++)
        {
            byte value = m.ReadU8(address + (uint)i);
            if (value == 0) break;
            chars.Add(value);
        }
        string message = System.Text.Encoding.ASCII.GetString(chars.ToArray());
        throw new InvalidOperationException($"Vigilante 8 fatal error: {message} (0x{address:X8})");
    }

    public static void TraceLevelLoadEntry(CpuContext c, IMemory m)
    {
        if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_LEVEL_LOAD") != "1") return;
        Console.WriteLine($"[LevelLoad] entry path=0x{c.A0:X8} '{ReadAscii(m, c.A0)}' " +
            $"label=0x{c.A1:X8} kind={c.A2}");
    }

    public static void TraceStreamOpen(CpuContext c, IMemory m)
    {
        if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_LEVEL_LOAD") != "1") return;
        Console.WriteLine($"[LevelLoad] stream-open path=0x{c.A0:X8} '{ReadAscii(m, c.A0)}' " +
            $"sp=0x{c.SP:X8} s3=0x{c.S3:X8}");
    }

    public static void TraceLevelFactoryPre(CpuContext c, IMemory m)
    {
        if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_LEVEL_LOAD") != "1") return;
        Console.WriteLine($"[LevelLoad] factory-enter callback=0x{c.A0:X8} object=0x{c.A1:X8} " +
            $"kind={c.A2} caller-s3=0x{c.S3:X8} sp=0x{c.SP:X8}");
    }

    public static void TraceLevelFactoryPost(CpuContext c, IMemory m)
    {
        if (Environment.GetEnvironmentVariable("RECOMPONE_TRACE_LEVEL_LOAD") != "1") return;
        Console.WriteLine($"[LevelLoad] factory-exit result=0x{c.V0:X8} caller-s3=0x{c.S3:X8} " +
            $"sp=0x{c.SP:X8}");
    }

    static string ReadAscii(IMemory m, uint address)
    {
        var chars = new List<byte>(96);
        for (int i = 0; i < 95; i++)
        {
            byte value = m.ReadU8(address + (uint)i);
            if (value == 0) break;
            chars.Add(value is >= 0x20 and < 0x7F ? value : (byte)'?');
        }
        return System.Text.Encoding.ASCII.GetString(chars.ToArray());
    }

    // The PS1 continues delivering VSync interrupts while the game spins in
    // FUN_800165cc waiting for its transition callback. Recompiled code runs on
    // one host thread, so explicitly advance frames until that callback sets the
    // original completion flag. The original cleanup body then runs unchanged.
    public static void ServiceDisplayTransitionWait(CpuContext c, IMemory m)
    {
        if (c.A0 != 0u) return;

        uint transition = m.ReadU32(c.GP + 0x6BCu);
        if (transition == 0u || m.ReadU32(transition + 0x5DCCu) != 0u) return;

        int frames = 0;
        while (m.ReadU32(transition + 0x5DCCu) == 0u && frames < 256)
        {
            Runtime.PresentFrame();
            frames++;
        }

        if (m.ReadU32(transition + 0x5DCCu) == 0u)
            throw new InvalidOperationException($"Vigilante 8 display transition did not complete after {frames} VSync frames");

        Console.Error.WriteLine($"[V8Compat] serviced display transition wait in {frames} VSync frames");
    }

    // DrawOTag completes asynchronously on PS1 hardware. The HLE GPU consumes
    // the ordering table immediately, so deliver the registered DrawSync
    // callback and then advance VSync until the game's completion callback
    // raises gp+0x168. Without this bridge FUN_800128BC spins forever.
    public static void ServiceDrawSyncWait(CpuContext c, IMemory m)
    {
        TraceGameplayHeartbeat(c, m);
        if (m.ReadU32(c.GP + 0x168u) != 0u) return;

        const uint drawSyncCallbackSlot = 0x80065030u;
        uint callback = m.ReadU32(drawSyncCallbackSlot);
        if (callback == 0u)
            throw new InvalidOperationException("Vigilante 8 entered DrawSync wait without a registered callback");

        var snapshot = c.Snapshot();
        Dispatcher.Call(c, m, callback);
        c.Restore(snapshot);

        int frames = 0;
        while (m.ReadU32(c.GP + 0x168u) == 0u && frames < 16)
        {
            Runtime.PresentFrame();
            frames++;
        }

        if (m.ReadU32(c.GP + 0x168u) == 0u)
            throw new InvalidOperationException($"Vigilante 8 DrawSync wait did not complete after {frames} VSync frames");

    }

    public static void ValidateTerrainQuery(CpuContext c, IMemory m)
    {
        uint xBlock = c.A0 >> 22;
        uint zBlock = c.A1 >> 22;
        uint xNextBlock = ((c.A0 >> 16) + 1u) >> 6;
        uint zNextBlock = ((c.A1 >> 16) + 1u) >> 6;
        foreach ((uint bx, uint bz) in new[] {
            (xBlock, zBlock), (xNextBlock, zBlock),
            (xBlock, zNextBlock), (xNextBlock, zNextBlock) })
        {
            if (bx >= 32u || bz >= 32u)
            {
                string detail = c.RA == 0x800324F8u
                    ? $" mine=0x{c.S2:X8} flags=0x{m.ReadU32(c.S2):X8} " +
                      $"callback=0x{m.ReadU32(c.S2 + 0x64u):X8} " +
                      $"pos={ReadVec3(m, c.S2 + 0x48u)} vel={ReadVec3(m, c.S2 + 0x88u)} " +
                      $"owner=0x{m.ReadU32(c.S2 + 0x80u):X8} target=0x{m.ReadU32(c.S2 + 0x84u):X8} " +
                      $"life={m.ReadU16(c.S2 + 0x94u)}"
                    : string.Empty;
                throw new InvalidOperationException(
                    $"Vigilante 8 terrain query outside the 32x32 tile map: " +
                    $"x=0x{c.A0:X8} z=0x{c.A1:X8} block={bx},{bz} caller=0x{c.RA:X8}{detail}");
            }

            uint tileSlot = 0x800911A0u + bx * 0x80u + bz * 4u;
            uint tile = m.ReadU32(tileSlot);
            if (!IsRetailRamRange(tile, 0x2000u))
                throw new InvalidOperationException(
                    $"Vigilante 8 terrain tile pointer is invalid: x=0x{c.A0:X8} z=0x{c.A1:X8} " +
                    $"block={bx},{bz} slot=0x{tileSlot:X8} tile=0x{tile:X8} caller=0x{c.RA:X8}");
        }
    }

    public static bool DrawMaxTerrainLod(CpuContext c, IMemory m)
    {
        if (!MaxLevelOfDetailEnabled())
            return false;

        m = Dispatcher.UnwrapMemory(m);
        uint root = m.ReadU32(c.GP + 0x6FCu);
        if (IsRetailRamRange(root, 16u))
        {
            var saved = c.Snapshot();
            DrawAllTerrainLeaves(c, m, root, new HashSet<uint>());
            c.Restore(saved);
        }
        return true;
    }

    static void DrawAllTerrainLeaves(
        CpuContext c, IMemory m, uint node, HashSet<uint> visited)
    {
        if (!IsRetailRamRange(node, 16u) || !visited.Add(node))
            return;

        uint kind = m.ReadU32(node);
        if (kind == 0u)
        {
            var saved = c.Snapshot();
            c.A0 = node + 4u;
            Dispatcher.Call(c, m, 0x800206F0u);
            c.Restore(saved);
            return;
        }

        if (kind is 1u or 2u)
        {
            DrawAllTerrainLeaves(c, m, m.ReadU32(node + 8u), visited);
            DrawAllTerrainLeaves(c, m, m.ReadU32(node + 12u), visited);
        }
    }

    public static void ApplyModelLevelOfDetail(CpuContext c, IMemory m) =>
        ApplyModelLevelOfDetail(c.A0, m);

    public static void ApplyModelLevelOfDetail(uint objectAddress, IMemory m)
    {
        if (!IsRetailRamRange(objectAddress, 0x70u))
            return;

        m = Dispatcher.UnwrapMemory(m);
        uint highMesh = m.ReadU32(objectAddress + 0x30u);
        uint lowMesh = m.ReadU32(objectAddress + 0x68u);
        uint threshold = m.ReadU32(objectAddress + 0x6Cu);
        if (MaxLevelOfDetailEnabled())
        {
            if (threshold != 0u)
                _lodThresholds[objectAddress] = (highMesh, lowMesh, threshold);
            m.WriteU32(objectAddress + 0x6Cu, 0u);
            return;
        }

        if (threshold == 0u &&
            _lodThresholds.TryGetValue(objectAddress, out var stock) &&
            stock.HighMesh == highMesh &&
            stock.LowMesh == lowMesh)
        {
            m.WriteU32(objectAddress + 0x6Cu, stock.Threshold);
        }
    }

    static bool MaxLevelOfDetailEnabled()
    {
        string? environmentMode =
            Environment.GetEnvironmentVariable("RECOMPONE_V8_LOD") ??
            Environment.GetEnvironmentVariable("RECOMPONE_LOD_MODE");
        if (!string.IsNullOrWhiteSpace(environmentMode))
        {
            return environmentMode.Equals("maximum", StringComparison.OrdinalIgnoreCase) ||
                   environmentMode.Equals("max", StringComparison.OrdinalIgnoreCase) ||
                   environmentMode.Equals("1", StringComparison.OrdinalIgnoreCase) ||
                   environmentMode.Equals("true", StringComparison.OrdinalIgnoreCase);
        }

        return ConfigManager.View.LevelOfDetail.Equals(
            "Maximum", StringComparison.OrdinalIgnoreCase);
    }

    public static void TraceVehiclePhysicsTick(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (player == 0u || c.A0 != player) return;

        int tick = _gameplayHeartbeatTick;
        if (!_traceVehicle || tick > 900) return;
        if (tick is 1 or 15 or 30 or 60 or 120 or 180 or 300 or 420 or 510 or 600 or 720 or 780 or 840 or 900)
            HostWindow.RequestDisplayCapture($"physics_{tick:000}");
        Console.Error.WriteLine(
            $"[V8Physics] tick={tick} phase=begin obj=0x{player:X8} " +
            $"flags=0x{m.ReadU32(player):X8} upY={unchecked((short)m.ReadU16(player + 0x18u))} " +
            $"pos={ReadVec3(m, player + 0x24u)} vel={ReadVec3(m, player + 0x80u)} " +
            $"ang={ReadVec3(m, player + 0x90u)} matrix={ReadMatrix(m, player + 0x10u)}");
    }

    public static void TraceGameplayHeartbeat(CpuContext c, IMemory m)
    {
        if (_whammyMatrixMutating) return;
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (!IsRetailRamRange(player, 0xF8u) ||
            m.ReadU8(player + 4u) != 2u)
            return;

        byte matchMode = m.ReadU8(c.GP + 0x15u);
        bool arenaTransition = _lastMenuStage == "choose_enemies";
        if (matchMode is not (1 or 3 or 4) && !arenaTransition)
            return;

        uint frameCounter = m.ReadU32(c.GP + 0x0Cu);
        if (frameCounter == _lastGameplayFrameCounter)
            return;
        _lastGameplayFrameCounter = frameCounter;

        int tick = ++_gameplayHeartbeatTick;
        _vehiclePhysicsTick = tick;
        if (_whammyMatrix)
        {
            uint lockedTarget = m.ReadU32(player + 0xE4u);
            _victoryAutopilotTarget = IsLiveVehicle(m, lockedTarget, player)
                ? lockedTarget
                : FindNearestLiveVehicle(m, player);
            TraceWhammyCounter(m, player, tick);
            if (IsLivePlayerVehicle(m, player))
                UpdateWhammyMatrix(c, m, player, tick);
        }
        UpdateVictoryAutopilot(m, player, tick);
        WriteStateTrace(m, c.GP, player, tick);
        if (!_soakTeardownSignaled && _soakTeardownTick > 0 && tick >= _soakTeardownTick)
        {
            _soakTeardownSignaled = true;
            InputManager.SignalScriptStage("soak_teardown");
        }
        if (tick == 1)
        {
            _gameplayStage = true;
            _lastMenuStage = null;
            InputManager.SignalScriptStage("gameplay", _automationGameplayCaptureDelay);
        }
        else if (_pauseResumePending)
        {
            _pauseResumePending = false;
            _lastMenuStage = null;
            InputManager.SignalScriptStage($"gameplay_resumed_{++_pauseResumeCount}");
        }
        if (_soakHeartbeatTicks > 0 &&
            (tick == 1 || tick % _soakHeartbeatTicks == 0))
        {
            uint player2 = m.ReadU32(c.GP + 0x7D4u);
            string player2State = player2 == 0u
                ? " player2=0x00000000"
                : $" player2=0x{player2:X8} pos2={ReadVec3(m, player2 + 0x24u)} " +
                  $"vel2={ReadVec3(m, player2 + 0x80u)}";
            Console.Error.WriteLine(
                $"[Soak] gameplay tick={tick} frame={frameCounter} match_mode={matchMode} " +
                $"player=0x{player:X8} callback=0x{m.ReadU32(player + 0x64u):X8} " +
                $"pos={ReadVec3(m, player + 0x24u)} vel={ReadVec3(m, player + 0x80u)}" +
                player2State);
        }
        TracePlayerWeaponState(m, player, tick);
    }

    static void TraceWhammyCounter(IMemory m, uint player, int tick)
    {
        byte count = m.ReadU8(player + 0xB9u);
        if (count > _lastPlayerWhammyCount)
        {
            byte projectileType = m.ReadU8(player + 0xB8u);
            _matrixProjectileTypeKinds.TryGetValue(projectileType, out int kind);
            bool directHitAttribution = false;
            if (kind == 0 &&
                _pendingDirectWhammyKind != 0 &&
                tick - _pendingDirectWhammyTick is >= 0 and <= 2)
            {
                kind = _pendingDirectWhammyKind;
                directHitAttribution = true;
                _matrixProjectileTypeKinds[projectileType] = kind;
            }
            if (kind != 0 && _whammyMatrixFiredKinds.Contains(kind))
            {
                _whammyMatrixConfirmedKinds.Add(kind);
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={tick} event=confirmed-counter " +
                    $"weaponKind={kind} projectileType={projectileType} " +
                    $"attribution={(directHitAttribution ? "direct-hit" : "projectile-type")} " +
                    $"counter={_lastPlayerWhammyCount}->{count} " +
                    $"confirmedKinds=" +
                    $"{string.Join(',', _whammyMatrixConfirmedKinds.Order())}");
            }
            else
            {
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={tick} event=counter-unmapped " +
                    $"projectileType={projectileType} " +
                    $"counter={_lastPlayerWhammyCount}->{count}");
            }
            _pendingDirectWhammyKind = 0;
            _pendingDirectWhammyProjectile = 0u;
            _pendingDirectWhammyTarget = 0u;
        }
        else if (_pendingDirectWhammyKind != 0 &&
                 tick - _pendingDirectWhammyTick > 2)
        {
            _pendingDirectWhammyKind = 0;
            _pendingDirectWhammyProjectile = 0u;
            _pendingDirectWhammyTarget = 0u;
        }
        _lastPlayerWhammyCount = count;
    }

    public static ushort GetAutomationInputMask()
    {
        if (!_victoryAutopilot && !_whammyMatrix)
            return 0;

        if (_victoryAutopilotInput != 0)
            return _victoryAutopilotInput;

        // Input polling can begin after the final enemy-selection confirmation
        // but before the first gameplay heartbeat while the arena transition
        // is still being serviced. Arm only in that narrow transition; doing
        // so globally would corrupt passcode and front-end navigation.
        ushort fire = _whammyMatrix
            ? Controller.L2
            : (ushort)(Controller.R2 | Controller.L2);
        return _lastMenuStage == "choose_enemies" || _gameplayStage
            ? (ushort)(fire | Controller.Cross)
            : (ushort)0;
    }

    static uint FindNearestLiveVehicle(IMemory m, uint player)
    {
        uint best = 0u;
        ulong bestDistance = ulong.MaxValue;
        foreach (uint listAddress in new[] {
                     0x80065A18u, 0x80065A60u, 0x80065A80u, 0x80065AC0u })
        {
            uint node = m.ReadU32(listAddress);
            var visited = new HashSet<uint>();
            for (int count = 0;
                 count < 4096 && IsRetailRamRange(node, 12u) && visited.Add(node);
                 count++)
            {
                uint next = m.ReadU32(node);
                if (next == 0u) break;
                uint candidate = m.ReadU32(node + 8u);
                if (IsLiveVehicle(m, candidate, player))
                {
                    long dx = unchecked((int)m.ReadU32(candidate + 0x24u)) -
                              (long)unchecked((int)m.ReadU32(player + 0x24u));
                    long dz = unchecked((int)m.ReadU32(candidate + 0x2Cu)) -
                              (long)unchecked((int)m.ReadU32(player + 0x2Cu));
                    ulong distance = (ulong)(dx * dx + dz * dz);
                    if (distance < bestDistance)
                    {
                        best = candidate;
                        bestDistance = distance;
                    }
                }
                node = next;
            }
        }
        return best;
    }

    static bool IsLiveVehicle(IMemory m, uint candidate, uint player) =>
        candidate != player &&
        IsRetailRamRange(candidate, 0xF8u) &&
        m.ReadU8(candidate + 4u) == 2u &&
        m.ReadU16(candidate + 0x0Cu) > 0u;

    static bool IsLivePlayerVehicle(IMemory m, uint player) =>
        IsRetailRamRange(player, 0xF8u) &&
        m.ReadU8(player + 4u) == 2u &&
        m.ReadU16(player + 0x0Cu) > 0u;

    static void UpdateWhammyMatrix(CpuContext c, IMemory m, uint player, int tick)
    {
        if (!_whammyMatrix) return;
        if (tick == 1)
        {
            Console.Error.WriteLine(
                $"[V8Whammy] tick=1 event=matrix-start " +
                $"order={string.Join(',', _whammyMatrixKinds)}");
            uint baseOne = EnsureMatrixWeapon(c, m, player, 1, 0);
            uint baseTwo = EnsureMatrixWeapon(c, m, player, 2, 1);
            Console.Error.WriteLine(
                $"[V8Whammy] tick=1 event=pair-base " +
                $"kind1=0x{baseOne:X8} kind2=0x{baseTwo:X8}");
        }

        if (_whammyMatrixCurrentWeapon != 0u &&
            IsRetailRamRange(_whammyMatrixCurrentWeapon, 0x0Eu))
        {
            ApplyMatrixPairSelection(m, player, tick);
            ushort ammo = m.ReadU16(_whammyMatrixCurrentWeapon + 0x0Cu);
            if (ammo < _whammyMatrixArmedAmmo &&
                _whammyMatrixFiredKinds.Add(_whammyMatrixCurrentKind))
            {
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={tick} event=fired " +
                    $"weaponKind={_whammyMatrixCurrentKind} ammo={ammo}");
            }
        }

        const int attributionSettleTicks = 180;
        const int maximumWeaponWindowTicks = 1500;
        if (_whammyMatrixCurrentWeapon != 0u)
        {
            int age = tick - _whammyMatrixCurrentAttachTick;
            bool fired = _whammyMatrixFiredKinds.Contains(_whammyMatrixCurrentKind);
            bool confirmed = _whammyMatrixConfirmedKinds.Contains(_whammyMatrixCurrentKind);
            bool specialsComplete =
                _whammyMatrixCurrentKind == 6 ||
                SpecialAttacksComplete(_whammyMatrixCurrentKind);
            if (age < attributionSettleTicks ||
                ((!confirmed || !specialsComplete) &&
                 age < maximumWeaponWindowTicks))
                return;

            Console.Error.WriteLine(
                $"[V8Whammy] tick={tick} event=window-" +
                $"{(fired && confirmed ? "complete" : "timeout")} " +
                $"weaponKind={_whammyMatrixCurrentKind} age={age} " +
                $"fired={fired} confirmed={confirmed} " +
                $"specialsComplete={specialsComplete}");
            _whammyMatrixCurrentWeapon = 0u;
            _whammyMatrixPartnerWeapon = 0u;
        }

        if (_whammyMatrixIndex >= _whammyMatrixKinds.Length)
            return;
        if (tick < _whammyMatrixNextAttachTick)
            return;
        _whammyMatrixNextAttachTick = tick + 120;

        int kind = _whammyMatrixKinds[_whammyMatrixIndex];
        int partnerKind = kind == 1 ? 2 : 1;
        uint partnerWeapon = EnsureMatrixWeapon(
            c, m, player, partnerKind, partnerKind - 1);
        uint weapon = EnsureMatrixWeapon(
            c, m, player, kind, kind <= 2 ? kind - 1 : 2);
        if (weapon == 0u)
        {
            Console.Error.WriteLine(
                $"[V8Whammy] tick={tick} event=attach-failed weaponKind={kind}");
            return;
        }
        if (partnerWeapon == 0u)
        {
            Console.Error.WriteLine(
                $"[V8Whammy] tick={tick} event=partner-attach-failed " +
                $"weaponKind={kind} partnerKind={partnerKind}");
            return;
        }

        int slot = FindMatrixWeaponSlot(m, player, weapon);
        int partnerSlot = FindMatrixWeaponSlot(m, player, partnerWeapon);
        if (slot < 0 || partnerSlot < 0 || slot == partnerSlot)
        {
            Console.Error.WriteLine(
                $"[V8Whammy] tick={tick} event=pair-slot-missing " +
                $"weaponKind={kind} weapon=0x{weapon:X8} slot={slot} " +
                $"partnerKind={partnerKind} partner=0x{partnerWeapon:X8} " +
                $"partnerSlot={partnerSlot}");
            return;
        }

        m.WriteU16(weapon + 0x0Cu, 100);
        m.WriteU16(partnerWeapon + 0x0Cu, 100);
        _whammyMatrixCurrentKind = kind;
        _whammyMatrixCurrentWeapon = weapon;
        _whammyMatrixCurrentSlot = slot;
        _whammyMatrixPartnerKind = partnerKind;
        _whammyMatrixPartnerWeapon = partnerWeapon;
        _whammyMatrixPartnerSlot = partnerSlot;
        _whammyMatrixArmedAmmo = 100;
        _whammyMatrixCurrentAttachTick = tick;
        _whammyMatrixNextAttachTick = tick;
        _whammyMatrixIndex++;
        ApplyMatrixPairSelection(m, player, tick);
        Console.Error.WriteLine(
            $"[V8Whammy] tick={tick} event=armed weaponKind={kind} " +
            $"weapon=0x{weapon:X8} callback=0x{m.ReadU32(weapon + 0x64u):X8} " +
            $"slot={slot} partnerKind={partnerKind} " +
            $"partner=0x{partnerWeapon:X8} partnerSlot={partnerSlot} " +
            $"target=0x{_victoryAutopilotTarget:X8}");
    }

    static void ApplyMatrixPairSelection(IMemory m, uint player, int tick)
    {
        if (_whammyMatrixCurrentWeapon == 0u ||
            _whammyMatrixPartnerWeapon == 0u)
            return;
        int slot = (tick & 1) == 0
            ? _whammyMatrixCurrentSlot
            : _whammyMatrixPartnerSlot;
        m.WriteU8(player + 0xB3u, (byte)slot);
    }

    static int FindMatrixWeaponSlot(IMemory m, uint player, uint weapon)
    {
        for (int index = 0; index < 3; index++)
        {
            if (m.ReadU32(player + 0x110u + (uint)index * 4u) == weapon)
                return index;
        }
        return -1;
    }

    static uint EnsureMatrixWeapon(
        CpuContext c, IMemory m, uint player, int kind,
        int preferredReplacementSlot)
    {
        foreach (var entry in _matrixWeaponKinds)
        {
            if (entry.Value == kind &&
                FindMatrixWeaponSlot(m, player, entry.Key) >= 0)
                return entry.Key;
        }

        uint expectedCallback = MatrixWeaponCallback(c, m, player, kind);
        if (expectedCallback != 0u)
        {
            for (int index = 0; index < 3; index++)
            {
                uint existing = m.ReadU32(
                    player + 0x110u + (uint)index * 4u);
                if (!IsRetailRamRange(existing, 0x68u) ||
                    m.ReadU32(existing + 0x64u) != expectedCallback)
                    continue;
                _matrixWeaponKinds[existing] = kind;
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={_gameplayHeartbeatTick} " +
                    $"event=adopt-existing weaponKind={kind} " +
                    $"weapon=0x{existing:X8} slot={index}");
                return existing;
            }
        }

        m.WriteU8(player + 0xB3u, (byte)preferredReplacementSlot);
        uint weapon = AttachMatrixWeapon(c, m, player, kind);
        if (weapon != 0u)
            _matrixWeaponKinds[weapon] = kind;
        return weapon;
    }

    static uint MatrixWeaponCallback(
        CpuContext c, IMemory m, uint player, int kind) =>
        kind switch
        {
            1 => 0x80031FA0u,
            2 => 0x8003302Cu,
            3 => 0x800336FCu,
            4 => 0x80034920u,
            5 => 0x8003565Cu,
            6 => CallGameFunction(
                c, m, 0x8003D1E8u, m.ReadU8(player + 0xD0u)),
            _ => 0u,
        };

    static uint AttachMatrixWeapon(CpuContext c, IMemory m, uint player, int kind)
    {
        uint callback = MatrixWeaponCallback(c, m, player, kind);
        if (callback == 0u || kind is < 1 or > 6)
            return 0u;

        _whammyMatrixMutating = true;
        try
        {
            bool hasEmptySlot = false;
            for (int index = 0; index < 3; index++)
            {
                if (m.ReadU32(player + 0x110u + (uint)index * 4u) == 0u)
                {
                    hasEmptySlot = true;
                    break;
                }
            }
            if (!hasEmptySlot)
            {
                int replacement = Math.Clamp(
                    kind <= 2 ? kind - 1 : 2, 0, 2);
                CallGameFunction(c, m, 0x8002CA94u, player, (uint)replacement);
            }

            // FUN_8002CCE8 is the retail starting-weapon attachment path. It
            // allocates the exact 0x80-byte object, initializes its callback,
            // resolves the vehicle bone, links the transform, and owns the
            // slot update. Reusing it avoids the overlapping heap ownership
            // caused by manually assembling a 0x84-byte approximation.
            CallGameFunction(c, m, 0x8002CCE8u, player, 1u << kind);
            for (int index = 0; index < 3; index++)
            {
                uint weapon = m.ReadU32(
                    player + 0x110u + (uint)index * 4u);
                if (IsRetailRamRange(weapon, 0x68u) &&
                    m.ReadU32(weapon + 0x64u) == callback)
                    return weapon;
            }
            return 0u;
        }
        finally
        {
            _whammyMatrixMutating = false;
        }
    }

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

    public static void TraceSprintf(CpuContext c, IMemory m)
    {
        if (!_whammyMatrix || c.A1 != 0x80065738u) return;
        Console.Error.WriteLine(
            $"[V8Whammy] tick={_gameplayHeartbeatTick} event=display " +
            $"multiplier={unchecked((int)c.A2)} " +
            $"confirmedKinds={string.Join(',', _whammyMatrixConfirmedKinds.Order())}");
    }

    public static void TraceSpecialAttackPre(CpuContext c, IMemory m)
    {
        if (!_whammyMatrix) return;
        m = Dispatcher.UnwrapMemory(m);
        if (!_matrixWeaponKinds.TryGetValue(c.A0, out int kind))
            return;
        if (c.A1 is 9u or 0xBu)
            _activeMatrixWeaponKind = kind;
        if (c.A1 != 9u || kind is < 1 or > 5 ||
            !IsRetailRamRange(c.A0, 0x0Eu))
            return;

        _pendingSpecialWeapon = c.A0;
        _pendingSpecialKind = kind;
        _pendingSpecialCode = c.A2 & 0xFFFu;
        _pendingSpecialAmmo = m.ReadU16(c.A0 + 0x0Cu);
    }

    public static void TraceWeaponCommandState(CpuContext c, IMemory m)
    {
        if (!_whammyMatrix) return;
        m = Dispatcher.UnwrapMemory(m);
        if (!IsRetailRamRange(c.A1, 12u)) return;
        const int initialDelayTicks = 30;
        const int commandPeriodTicks = 300;
        const int opportunitiesPerWindow = 5;
        int tick = _gameplayHeartbeatTick;
        int age = tick - _whammyMatrixCurrentAttachTick;
        if (_gameplayStage &&
            _whammyMatrixCurrentWeapon != 0u &&
            age >= initialDelayTicks)
        {
            if (_lastInjectedSpecialAttachTick !=
                _whammyMatrixCurrentAttachTick)
            {
                _lastInjectedSpecialAttachTick =
                    _whammyMatrixCurrentAttachTick;
                _lastInjectedSpecialOpportunity = -1;
            }

            int opportunity =
                (age - initialDelayTicks) / commandPeriodTicks;
            if (opportunity < opportunitiesPerWindow &&
                opportunity != _lastInjectedSpecialOpportunity)
            {
                _lastInjectedSpecialOpportunity = opportunity;
                uint injected =
                    MissingSpecialAttackCode(_whammyMatrixCurrentKind);
                int commandKind = _whammyMatrixCurrentKind;
                if (injected == 0u)
                {
                    commandKind = _whammyMatrixPartnerKind;
                    injected = MissingSpecialAttackCode(commandKind);
                }
                if (injected != 0u)
                {
                    uint commandWeapon =
                        commandKind == _whammyMatrixCurrentKind
                            ? _whammyMatrixCurrentWeapon
                            : _whammyMatrixPartnerWeapon;
                    if (IsRetailRamRange(commandWeapon, 0x0Eu))
                        m.WriteU16(commandWeapon + 0x0Cu, 100);
                    m.WriteU32(c.A1 + 4u, injected);
                    m.WriteU32(
                        c.A1 + 8u, m.ReadU32(c.A1 + 8u) | 0x20000u);
                    Console.Error.WriteLine(
                        $"[V8SpecialInput] tick={tick} " +
                        $"event=command-injected weaponKind={commandKind} " +
                        $"code=0x{injected:X3}");
                }
            }
        }
        uint code = m.ReadU32(c.A1 + 4u);
        uint flags = m.ReadU32(c.A1 + 8u);
        if (code == _lastSpecialCommandCode &&
            flags == _lastSpecialCommandFlags)
            return;
        _lastSpecialCommandCode = code;
        _lastSpecialCommandFlags = flags;
        if (code != 0u || (flags & 0x20000u) != 0u)
        {
            Console.Error.WriteLine(
                $"[V8SpecialInput] tick={_gameplayHeartbeatTick} " +
                $"code=0x{code:X8} flags=0x{flags:X8}");
        }
    }

    static uint MissingSpecialAttackCode(int kind)
    {
        if (!_specialAttackCodes.TryGetValue(kind, out uint[]? codes))
            return 0u;
        foreach (uint code in codes)
        {
            if (!_specialAttackCoverage.Contains($"{kind}:{code:X3}"))
                return code;
        }
        return 0u;
    }

    static bool SpecialAttacksComplete(int kind) =>
        _specialAttackCodes.TryGetValue(kind, out uint[]? codes) &&
        codes.All(code =>
            _specialAttackCoverage.Contains($"{kind}:{code:X3}"));

    public static void TraceSpecialAttackPost(CpuContext c, IMemory m)
    {
        _activeMatrixWeaponKind = 0;
        if (_pendingSpecialWeapon == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint weapon = _pendingSpecialWeapon;
        int kind = _pendingSpecialKind;
        uint code = _pendingSpecialCode;
        ushort before = _pendingSpecialAmmo;
        _pendingSpecialWeapon = 0u;

        if (!IsRetailRamRange(weapon, 0x0Eu))
            return;
        ushort after = m.ReadU16(weapon + 0x0Cu);
        if (after >= before)
            return;

        string coverage = $"{kind}:{code:X3}";
        if (_specialAttackCoverage.Add(coverage))
        {
            Console.Error.WriteLine(
                $"[V8Special] tick={_gameplayHeartbeatTick} " +
                $"event=fired weaponKind={kind} code=0x{code:X3} " +
                $"ammo={before}->{after} coverage=" +
                $"{string.Join(',', _specialAttackCoverage.Order())}");
        }
    }

    static void WriteStateTrace(IMemory m, uint gp, uint player, int tick)
    {
        if (string.IsNullOrWhiteSpace(_stateTracePath) || _stateTraceUnavailable)
            return;

        try
        {
            if (_stateTraceWriter == null)
            {
                string path = Path.GetFullPath(_stateTracePath);
                string? directory = Path.GetDirectoryName(path);
                if (!string.IsNullOrEmpty(directory))
                    Directory.CreateDirectory(directory);
                _stateTraceWriter = new StreamWriter(path, false);
                _stateTraceWriter.WriteLine(
                    "{\"record\":\"schema\",\"schema\":\"v8-reference-state-v1\"," +
                    "\"source\":\"SLUS_005.10\",\"tickHook\":\"FUN_8002f9bc:pre\"," +
                    "\"inputEncoding\":\"PS1 active-low pad bits\"}");
                AppDomain.CurrentDomain.ProcessExit += (_, _) =>
                {
                    _stateTraceWriter?.Flush();
                    _stateTraceWriter?.Dispose();
                };
                Console.Error.WriteLine($"[V8StateTrace] writing {path}");
            }

            uint player2 = m.ReadU32(gp + 0x7D4u);
            _stateTraceWriter.WriteLine(
                $"{{\"record\":\"state\",\"tick\":{tick}," +
                $"\"rngSeed\":{unchecked((int)m.ReadU32(0x800568D4u))}," +
                $"\"rngCarry\":{m.ReadU8(0x800568D8u)}," +
                $"\"matchMode\":{m.ReadU8(gp + 0x15u)}," +
                $"\"splitScreen\":{m.ReadU32(gp + 0x10u)}," +
                $"\"pad1\":{Controller.State},\"pad2\":{Controller.State2}," +
                $"\"p1\":{ReadStateTraceVehicle(m, player)}," +
                $"\"p2\":{ReadStateTraceVehicle(m, player2)}}}");
            if (tick % 60 == 0)
                _stateTraceWriter.Flush();
        }
        catch (Exception ex)
        {
            _stateTraceUnavailable = true;
            _stateTraceWriter?.Dispose();
            _stateTraceWriter = null;
            Console.Error.WriteLine($"[V8StateTrace] disabled after write failure: {ex.Message}");
        }
    }

    static string ReadStateTraceVehicle(IMemory m, uint address)
    {
        if (!IsRetailRamRange(address, 0x11Cu) || m.ReadU8(address + 4u) != 2u)
            return "null";

        uint slot0 = m.ReadU32(address + 0x110u);
        uint slot1 = m.ReadU32(address + 0x114u);
        uint slot2 = m.ReadU32(address + 0x118u);
        return
            $"{{\"flags\":{m.ReadU32(address)},\"kind\":{m.ReadU8(address + 4u)}," +
            $"\"health\":{m.ReadU16(address + 0x0Cu)}," +
            $"\"maxHealth\":{m.ReadU16(address + 0x0Eu)}," +
            $"\"matrix\":{ReadStateTraceMatrix(m, address + 0x10u)}," +
            $"\"position\":{ReadStateTraceVec3(m, address + 0x24u)}," +
            $"\"velocity\":{ReadStateTraceVec3(m, address + 0x80u)}," +
            $"\"angularVelocity\":{ReadStateTraceVec3(m, address + 0x90u)}," +
            $"\"damageZones\":[{ReadNodeHealthValue(m, m.ReadU32(address + 0xECu))}," +
            $"{ReadNodeHealthValue(m, m.ReadU32(address + 0xF0u))}," +
            $"{ReadNodeHealthValue(m, m.ReadU32(address + 0xF4u))}]," +
            $"\"selectedWeapon\":{m.ReadU8(address + 0xB3u)}," +
            $"\"weaponSlots\":[{ReadStateTraceWeapon(m, slot0)}," +
            $"{ReadStateTraceWeapon(m, slot1)},{ReadStateTraceWeapon(m, slot2)}]}}";
    }

    static string ReadStateTraceVec3(IMemory m, uint address) =>
        $"[{unchecked((int)m.ReadU32(address))}," +
        $"{unchecked((int)m.ReadU32(address + 4u))}," +
        $"{unchecked((int)m.ReadU32(address + 8u))}]";

    static string ReadStateTraceMatrix(IMemory m, uint address) =>
        $"[{unchecked((short)m.ReadU16(address))}," +
        $"{unchecked((short)m.ReadU16(address + 2u))}," +
        $"{unchecked((short)m.ReadU16(address + 4u))}," +
        $"{unchecked((short)m.ReadU16(address + 6u))}," +
        $"{unchecked((short)m.ReadU16(address + 8u))}," +
        $"{unchecked((short)m.ReadU16(address + 10u))}," +
        $"{unchecked((short)m.ReadU16(address + 12u))}," +
        $"{unchecked((short)m.ReadU16(address + 14u))}," +
        $"{unchecked((short)m.ReadU16(address + 16u))}]";

    static string ReadStateTraceWeapon(IMemory m, uint address) =>
        IsRetailRamRange(address, 0x0Eu)
            ? $"{{\"kind\":{m.ReadU8(address + 8u)},\"ammo\":{m.ReadU16(address + 0x0Cu)}}}"
            : "null";

    static void UpdateVictoryAutopilot(IMemory m, uint player, int tick)
    {
        if (!_victoryAutopilot && !_whammyMatrix) return;

        ushort fire = _whammyMatrix
            ? Controller.L2
            : (ushort)(Controller.R2 | Controller.L2);
        uint target = _victoryAutopilotTarget;
        if (!IsRetailRamRange(target, 0xF8u) || m.ReadU8(target + 4u) != 2u)
        {
            _victoryAutopilotInput = fire;
            return;
        }

        long dx = unchecked((int)m.ReadU32(target + 0x24u)) -
                  (long)unchecked((int)m.ReadU32(player + 0x24u));
        long dz = unchecked((int)m.ReadU32(target + 0x2Cu)) -
                  (long)unchecked((int)m.ReadU32(player + 0x2Cu));
        int forwardX = unchecked((short)m.ReadU16(player + 0x14u));
        int forwardZ = unchecked((short)m.ReadU16(player + 0x20u));
        long cross = (long)forwardX * dz - (long)forwardZ * dx;
        long dot = (long)forwardX * dx + (long)forwardZ * dz;
        long absoluteCross = Math.Abs(cross);
        long absoluteDot = Math.Abs(dot);
        ulong distanceSquared = (ulong)(dx * dx + dz * dz);
        ushort steer = 0;
        if (absoluteCross > absoluteDot / 8)
            steer = cross < 0 ? Controller.Right : Controller.Left;
        else if (dot < 0)
            steer = Controller.Right;

        // Brake inside a short engagement radius instead of continually
        // overshooting the target. All movement remains ordinary pad input;
        // the automation never changes gameplay state directly.
        ushort movement = distanceSquared > 20_000_000_000UL
            ? Controller.Cross
            : Controller.Square;
        _victoryAutopilotInput = (ushort)(fire | movement | steer);

        if (tick % 600 == 0)
        {
            Console.Error.WriteLine(
                $"[V8Autopilot] tick={tick} target=0x{target:X8} " +
                $"distance2={distanceSquared} " +
                $"movement={(movement == Controller.Cross ? "throttle" : "brake")} " +
                $"steer={(steer == Controller.Right ? "right" : steer == Controller.Left ? "left" : "center")} " +
                $"state={ReadVehicleDamageState(m, target)}");
        }
    }

    public static void TraceVehicleIntegratePre(CpuContext c, IMemory m)
    {
        if (!_traceVehicle) return;
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (player == 0u || c.A0 != player || _vehiclePhysicsTick > 900) return;

        _integratingVehicle = player;
        _integratingVehicleTick = _vehiclePhysicsTick;
        Console.Error.WriteLine(
            $"[V8Physics] tick={_integratingVehicleTick} phase=force " +
            $"force={ReadVec3(m, c.A1)} torque={ReadVec3(m, c.A2)}");
    }

    public static void TraceVehicleIntegratePost(CpuContext c, IMemory m)
    {
        if (!_traceVehicle || _integratingVehicle == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint player = _integratingVehicle;
        int tick = _integratingVehicleTick;
        _integratingVehicle = 0u;
        Console.Error.WriteLine(
            $"[V8Physics] tick={tick} phase=integrated " +
            $"pos={ReadVec3(m, player + 0x24u)} vel={ReadVec3(m, player + 0x80u)} " +
            $"ang={ReadVec3(m, player + 0x90u)} matrix={ReadMatrix(m, player + 0x10u)}");
    }

    public static void TracePlayerCollisionPre(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (_whammyMatrix &&
            IsRetailRamRange(player, 0xBAu) &&
            IsRetailRamRange(c.A1, 4u))
        {
            uint projectile = m.ReadU32(c.A1);
            if (IsRetailRamRange(projectile, 0x84u) &&
                m.ReadU32(projectile + 0x80u) == player)
            {
                int kind = 0;
                if (!_playerProjectileMatrixKinds.TryGetValue(
                        projectile, out kind))
                {
                    _matrixProjectileTypeKinds.TryGetValue(
                        m.ReadU8(projectile + 0x0Au), out kind);
                }
                if (kind != 0 && _whammyMatrixFiredKinds.Contains(kind))
                {
                    _pendingWhammyOwner = player;
                    _pendingWhammyCount = m.ReadU8(player + 0xB9u);
                    _pendingWhammyKind = kind;
                    _pendingWhammyProjectile = projectile;
                    _pendingWhammyTarget = c.A0;
                }
            }
        }

        if (!_traceVehicle || c.A1 == 0u || player == 0u || c.A0 != player)
            return;

        uint node = c.A1;
        uint other = m.ReadU32(node);
        uint subObject = m.ReadU32(node + 0x10u);
        int collision = ++_playerCollisionCount;
        byte otherKind = other == 0u ? (byte)0 : m.ReadU8(other + 4u);
        if (collision > 32) return;
        _collidingVehicle = player;
        _collidingVehicleTick = _vehiclePhysicsTick;
        _collidingOtherKind = otherKind;
        Console.Error.WriteLine(
            $"[V8Collision] count={collision} tick={_collidingVehicleTick} phase=pre " +
            $"other=0x{other:X8} kind={otherKind} " +
            $"sub=0x{subObject:X8} subKind={(subObject == 0u ? 0 : m.ReadU8(subObject + 4u))} " +
            $"contact={ReadVec3(m, node + 0x14u)} normal={ReadShortVec3(m, node + 0x20u)} " +
            $"penetration={unchecked((int)m.ReadU32(node + 0x30u))} " +
            $"vel={ReadVec3(m, player + 0x80u)} ang={ReadVec3(m, player + 0x90u)}");
    }

    public static void TracePlayerCollisionPost(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        if (_pendingWhammyOwner != 0u)
        {
            byte count = m.ReadU8(_pendingWhammyOwner + 0xB9u);
            if (count != _pendingWhammyCount)
            {
                _whammyMatrixConfirmedKinds.Add(_pendingWhammyKind);
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={_gameplayHeartbeatTick} " +
                    $"event=confirmed-hit weaponKind={_pendingWhammyKind} " +
                    $"projectile=0x{_pendingWhammyProjectile:X8} " +
                    $"target=0x{_pendingWhammyTarget:X8} " +
                    $"counter={_pendingWhammyCount}->{count} " +
                    $"confirmedKinds={string.Join(',', _whammyMatrixConfirmedKinds.Order())}");
            }
            _pendingWhammyOwner = 0u;
            _pendingWhammyProjectile = 0u;
            _pendingWhammyTarget = 0u;
        }

        if (!_traceVehicle || _collidingVehicle == 0u)
            return;
        uint player = _collidingVehicle;
        int tick = _collidingVehicleTick;
        _collidingVehicle = 0u;
        Console.Error.WriteLine(
            $"[V8Collision] count={_playerCollisionCount} tick={tick} phase=post " +
            $"vel={ReadVec3(m, player + 0x80u)} ang={ReadVec3(m, player + 0x90u)} " +
            $"matrix={ReadMatrix(m, player + 0x10u)}");
        if (_collidingOtherKind == 2 && !_capturedVehicleCollision)
        {
            _capturedVehicleCollision = true;
            HostWindow.RequestDisplayCapture("collision_vehicle_first");
        }
    }

    static void TracePlayerWeaponState(IMemory m, uint player, int tick)
    {
        if (!_traceWeapons) return;
        uint primary = m.ReadU32(player + 0x10Cu);
        string state =
            $"selected={m.ReadU8(player + 0xB3u)} " +
            $"primary=0x{primary:X8}/cb=0x{ReadObjectCallback(m, primary):X8} " +
            $"slot0={ReadWeaponSlotState(m, m.ReadU32(player + 0x110u))} " +
            $"slot1={ReadWeaponSlotState(m, m.ReadU32(player + 0x114u))} " +
            $"slot2={ReadWeaponSlotState(m, m.ReadU32(player + 0x118u))}";
        if (state == _lastWeaponState) return;
        _lastWeaponState = state;
        if (++_weaponStateChangeCount <= 256)
            Console.Error.WriteLine($"[V8Weapon] tick={tick} event=state {state}");
    }

    public static void TracePlayerChildSpawnPre(CpuContext c, IMemory m)
    {
        if (!_traceWeapons && !_whammyMatrix) return;
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (player == 0u || c.A0 != player) return;

        _pendingPlayerChildSpawn = true;
        _pendingPlayerSpawnWeapon = c.A1;
        _pendingPlayerSpawnKind = c.A2;
        _pendingPlayerSpawnCaller = c.RA;
        Console.Error.WriteLine(
            $"[V8Weapon] tick={_vehiclePhysicsTick} event=spawn-pre owner=0x{player:X8} " +
            $"weapon={ReadWeaponSlot(m, c.A1)} kind={c.A2} caller=0x{c.RA:X8}");
    }

    public static void TracePlayerChildSpawnPost(CpuContext c, IMemory m)
    {
        if ((!_traceWeapons && !_whammyMatrix) || !_pendingPlayerChildSpawn) return;
        m = Dispatcher.UnwrapMemory(m);
        _pendingPlayerChildSpawn = false;
        uint projectile = c.V0;
        if (projectile == 0u) return;
        _playerProjectiles.Add(projectile);
        if (_whammyMatrix &&
            _matrixWeaponKinds.TryGetValue(
                _pendingPlayerSpawnWeapon, out int matrixKind))
        {
            _playerProjectileMatrixKinds[projectile] = matrixKind;
            _matrixProjectileTypeKinds[m.ReadU8(projectile + 0x0Au)] =
                matrixKind;
            if (_whammyMatrixFiredKinds.Add(matrixKind))
            {
                Console.Error.WriteLine(
                    $"[V8Whammy] tick={_gameplayHeartbeatTick} event=fired " +
                    $"weaponKind={matrixKind} projectile=0x{projectile:X8}");
            }
        }
        if (_traceWeapons)
        {
            Console.Error.WriteLine(
                $"[V8Weapon] tick={_vehiclePhysicsTick} event=spawn-post projectile=0x{projectile:X8} " +
                $"weapon=0x{_pendingPlayerSpawnWeapon:X8} kind={_pendingPlayerSpawnKind} " +
                $"caller=0x{_pendingPlayerSpawnCaller:X8} pos={ReadVec3(m, projectile + 0x24u)}");
        }
    }

    public static void TracePlayerWeaponHitPre(CpuContext c, IMemory m)
    {
        if ((!_traceWeapons && !_whammyMatrix) || c.A1 == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint target = m.ReadU32(c.A1);
        if (_whammyMatrix &&
            _playerProjectileMatrixKinds.TryGetValue(c.A0, out int matrixKind))
        {
            uint player = m.ReadU32(c.GP + 0x7D0u);
            if (IsLiveVehicle(m, target, player))
            {
                _pendingDirectWhammyKind = matrixKind;
                _pendingDirectWhammyProjectile = c.A0;
                _pendingDirectWhammyTarget = target;
                _pendingDirectWhammyTick = _gameplayHeartbeatTick;
            }
        }
        if (!_traceWeapons || !_playerProjectiles.Contains(c.A0)) return;
        _pendingPlayerHitProjectile = c.A0;
        _pendingPlayerHitTarget = target;
        _pendingPlayerHitState = ReadVehicleDamageState(m, target);
        Console.Error.WriteLine(
            $"[V8Weapon] tick={_vehiclePhysicsTick} event=hit-pre projectile=0x{c.A0:X8} " +
            $"target=0x{target:X8} kind={(IsRetailRamRange(target, 5u) ? m.ReadU8(target + 4u) : 0)} " +
            $"damage={_pendingPlayerHitState} effect={c.A2} sfx={unchecked((int)c.A3)}");
    }

    public static void TracePlayerWeaponHitPost(CpuContext c, IMemory m)
    {
        if (!_traceWeapons || _pendingPlayerHitProjectile == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        string damageState = ReadVehicleDamageState(m, _pendingPlayerHitTarget);
        Console.Error.WriteLine(
            $"[V8Weapon] tick={_vehiclePhysicsTick} event=hit-post projectile=0x{_pendingPlayerHitProjectile:X8} " +
            $"target=0x{_pendingPlayerHitTarget:X8} damage={_pendingPlayerHitState}->{damageState} " +
            $"result={unchecked((int)c.V0)}");
        if (!_capturedPlayerWeaponHit)
        {
            _capturedPlayerWeaponHit = true;
            HostWindow.RequestDisplayCapture("weapon_hit_first");
        }
        _pendingPlayerHitProjectile = 0u;
        _pendingPlayerHitTarget = 0u;
        _pendingPlayerHitState = null;
    }

    public static void TraceVehicleDamagePre(CpuContext c, IMemory m)
    {
        if ((!_traceWeapons && !_victoryAutopilot) || unchecked((int)c.A1) >= 0) return;
        m = Dispatcher.UnwrapMemory(m);
        if (!IsRetailRamRange(c.A0, 0xF8u) || m.ReadU8(c.A0 + 4u) != 2u) return;

        _pendingVehicleDamageTarget = c.A0;
        _pendingVehicleDamageCaller = c.RA;
        _pendingVehicleDamageImpulse = unchecked((int)c.A1);
        _pendingVehicleDamageState = ReadVehicleDamageState(m, c.A0);
    }

    public static void TraceVehicleDamagePost(CpuContext c, IMemory m)
    {
        if ((!_traceWeapons && !_victoryAutopilot) || _pendingVehicleDamageTarget == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint target = _pendingVehicleDamageTarget;
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (_victoryAutopilot && target != player)
            _victoryAutopilotTarget = target;
        string before = _pendingVehicleDamageState ?? "unavailable";
        string after = ReadVehicleDamageState(m, target);
        if (_traceWeapons && (before != after || c.V0 != 0u) &&
            ++_vehicleDamageChangeCount <= 128)
        {
            Console.Error.WriteLine(
                $"[V8Damage] tick={_vehiclePhysicsTick} target=0x{target:X8} " +
                $"role={(target == player ? "player" : "other")} impulse={_pendingVehicleDamageImpulse} " +
                $"caller=0x{_pendingVehicleDamageCaller:X8} state={before}->{after} " +
                $"result={unchecked((int)c.V0)}");
        }
        _pendingVehicleDamageTarget = 0u;
        _pendingVehicleDamageState = null;
    }

    public static void TraceVehicleFullDestroy(CpuContext c, IMemory m) =>
        TraceVehicleDestruction(c, m, "full");

    public static void TraceVehicleSoftKill(CpuContext c, IMemory m) =>
        TraceVehicleDestruction(c, m, "soft");

    static void TraceVehicleDestruction(CpuContext c, IMemory m, string mode)
    {
        if (!_traceWeapons) return;
        m = Dispatcher.UnwrapMemory(m);
        if (!IsRetailRamRange(c.A0, 0xF8u) || m.ReadU8(c.A0 + 4u) != 2u) return;
        uint player = m.ReadU32(c.GP + 0x7D0u);
        Console.Error.WriteLine(
            $"[V8Destroy] tick={_vehiclePhysicsTick} target=0x{c.A0:X8} " +
            $"role={(c.A0 == player ? "player" : "other")} mode={mode} " +
            $"caller=0x{c.RA:X8} state={ReadVehicleDamageState(m, c.A0)}");
        if (!_capturedVehicleDestruction)
        {
            _capturedVehicleDestruction = true;
            HostWindow.RequestDisplayCapture("vehicle_destroy_first");
        }
    }

    public static void TraceResultScreen(CpuContext c, IMemory m)
    {
        if ((!_traceWeapons && !_traceResults) || _resultScreenReached) return;
        m = Dispatcher.UnwrapMemory(m);
        _resultScreenReached = true;
        uint player = m.ReadU32(c.GP + 0x7D0u);
        Console.Error.WriteLine(
            $"[V8Result] tick={_vehiclePhysicsTick} mode={m.ReadU8(c.GP + 0x15u)} " +
            $"aliveFlag={unchecked((int)m.ReadU32(c.GP + 0x24u))} " +
            $"timer={unchecked((int)m.ReadU32(c.GP + 0x624u))} " +
            $"player=0x{player:X8} callback=0x{ReadObjectCallback(m, player):X8} " +
            $"state={ReadVehicleDamageState(m, player)}");
        // ResultScreen_Build runs before the result layout has reached the
        // displayed ordering table. Capture after several result-stage polls
        // so the proof contains the rendered overlay rather than the final
        // destroyed-vehicle frame.
        InputManager.SignalScriptStage("result_screen", captureDelayPolls: 60);
    }

    static string ReadWeaponSlot(IMemory m, uint address)
    {
        if (!IsRetailRamRange(address, 0x68u)) return $"0x{address:X8}";
        return $"0x{address:X8}/kind={m.ReadU8(address + 8u)}/status={unchecked((short)m.ReadU16(address + 6u))}/" +
               $"ammo={m.ReadU16(address + 0x0Cu)}/cb=0x{m.ReadU32(address + 0x64u):X8}";
    }

    static string ReadWeaponSlotState(IMemory m, uint address)
    {
        if (!IsRetailRamRange(address, 0x68u)) return $"0x{address:X8}";
        return $"0x{address:X8}/kind={m.ReadU8(address + 8u)}/" +
               $"ammo={m.ReadU16(address + 0x0Cu)}/cb=0x{m.ReadU32(address + 0x64u):X8}";
    }

    static uint ReadObjectCallback(IMemory m, uint address) =>
        IsRetailRamRange(address, 0x68u) ? m.ReadU32(address + 0x64u) : 0u;

    static string ReadVehicleDamageState(IMemory m, uint address)
    {
        if (!IsRetailRamRange(address, 0xF8u) || m.ReadU8(address + 4u) != 2u)
            return "not-vehicle";
        return $"hp={m.ReadU16(address + 0x0Cu)}/{m.ReadU16(address + 0x0Eu)}," +
               $"zones=({ReadNodeHealth(m, m.ReadU32(address + 0xECu))}," +
               $"{ReadNodeHealth(m, m.ReadU32(address + 0xF0u))}," +
               $"{ReadNodeHealth(m, m.ReadU32(address + 0xF4u))})";
    }

    static string ReadNodeHealth(IMemory m, uint address) =>
        IsRetailRamRange(address, 0x0Eu) ? m.ReadU16(address + 0x0Cu).ToString() : "-";

    static int ReadNodeHealthValue(IMemory m, uint address) =>
        IsRetailRamRange(address, 0x0Eu) ? m.ReadU16(address + 0x0Cu) : -1;

    static string ReadVec3(IMemory m, uint address) =>
        $"({unchecked((int)m.ReadU32(address))},{unchecked((int)m.ReadU32(address + 4u))}," +
        $"{unchecked((int)m.ReadU32(address + 8u))})";

    static string ReadShortVec3(IMemory m, uint address) =>
        $"({unchecked((short)m.ReadU16(address))},{unchecked((short)m.ReadU16(address + 2u))}," +
        $"{unchecked((short)m.ReadU16(address + 4u))})";

    static string ReadMatrix(IMemory m, uint address) =>
        $"[{unchecked((short)m.ReadU16(address))},{unchecked((short)m.ReadU16(address + 2u))}," +
        $"{unchecked((short)m.ReadU16(address + 4u))};" +
        $"{unchecked((short)m.ReadU16(address + 6u))},{unchecked((short)m.ReadU16(address + 8u))}," +
        $"{unchecked((short)m.ReadU16(address + 10u))};" +
        $"{unchecked((short)m.ReadU16(address + 12u))},{unchecked((short)m.ReadU16(address + 14u))}," +
        $"{unchecked((short)m.ReadU16(address + 16u))}]";

    public static void TraceMenuText(CpuContext c, IMemory m)
    {
        string text = ReadAscii(m, c.A1, 48);
        if (_traceMenuText && text.Length != 0 && _seenMenuText.Add(text))
            Console.Error.WriteLine($"[V8Compat] menu text: {text}");

        string? stage;
        if (text == "PAUSED")
        {
            stage = $"pause_{++_pauseScreenCount}";
        }
        else if (text == "ARE YOU SURE?")
        {
            stage = $"pause_confirm_{++_pauseConfirmCount}";
        }
        else if (text == "GAME STATUS")
        {
            _optionsMenuActive = true;
            stage = _passcodeEditorVisited
                ? _passcodeEntryCount == 1
                    ? "game_status_return"
                    : $"game_status_return_{_passcodeEntryCount}"
                : "game_status";
            _passcodeEditorVisited = false;
        }
        else if (text.StartsWith("passcode", StringComparison.OrdinalIgnoreCase) ||
                 (_optionsMenuActive && text.Length == 1 &&
                  (text[0] == '_' || text[0] is >= 'A' and <= 'Z')))
        {
            if (!_passcodeEditorVisited)
                _passcodeEntryCount++;
            _passcodeEditorVisited = true;
            stage = _passcodeEntryCount == 1
                ? "passcode"
                : $"passcode_{_passcodeEntryCount}";
        }
        else if (text == "MEMORY CARD SLOT 1" && _optionsMenuActive)
        {
            stage = "memory_card";
        }
        else if (text == "DIFFICULTY SETTING" && _optionsMenuActive)
        {
            stage = "difficulty_setting";
        }
        else if (text == "CONTROLLER" && _optionsMenuActive)
        {
            stage = "controller_1";
        }
        else if (text == "UNPLUGGED" && _optionsMenuActive)
        {
            stage = "controller_2";
        }
        else if (text == "AUDIO SETTINGS")
        {
            stage = "audio_settings";
            if (_traceOptions)
                Console.Error.WriteLine($"[V8Options] event=enter {ReadAudioOptions(m)}");
        }
        else if (text == "SCREEN ADJUSTMENT" && _optionsMenuActive)
        {
            stage = "screen_adjustment";
            if (_traceOptions)
                Console.Error.WriteLine($"[V8Options] event=screen-enter {ReadScreenOptions(m)}");
        }
        else if (text == "CREDITS" && _optionsMenuActive)
        {
            stage = "credits";
        }
        else if (text == "1 PLAYER" && _optionsMenuActive)
        {
            _optionsMenuActive = false;
            stage = "options_return";
            if (_traceOptions)
                Console.Error.WriteLine(
                    $"[V8Options] event=return {ReadAudioOptions(m)} {ReadScreenOptions(m)}");
        }
        else stage = text switch
        {
            "PRESS START" => "press_start",
            "ARCADE" => "main_menu",
            "VERSUS" or "COOPERATIVE" =>
                _automationTargetTwoPlayerMode == null ||
                text.Equals(_automationTargetTwoPlayerMode, StringComparison.OrdinalIgnoreCase)
                    ? "two_player_mode"
                    : null,
            "1 PLAYER" => "player_count",
            "SELECT LOCATION" => "select_location",
            "QUEST ROUTE" => "quest_route",
            "CHOOSE PLAYER" => "choose_player",
            "CHOOSE PLAYERS" => "choose_players",
            "PLAYERS" => "choose_players",
            "CHOOSE ENEMIES" => "choose_enemies",
            _ => null,
        };
        if (stage == null && TryGetLocationStage(text, out string? locationStage) &&
            ((_automationTargetLocation == null && text == "Oil Fields") ||
             text.Equals(_automationTargetLocation, StringComparison.OrdinalIgnoreCase)))
        {
            stage = locationStage;
        }
        if (stage == null && TryGetCharacterStage(text, out string? characterStage) &&
            text.Equals(_automationTargetCharacter, StringComparison.OrdinalIgnoreCase))
        {
            stage = characterStage;
        }
        if (stage == "press_start")
        {
            _gameplayStage = false;
            _vehiclePhysicsTick = 0;
        }
        if (stage != null &&
            (!_gameplayStage || stage.StartsWith("pause_", StringComparison.Ordinal)))
            _lastMenuStage = stage;
    }

    static bool TryGetLocationStage(string text, out string? stage)
    {
        stage = text switch
        {
            "Ski Resort" => "location_ski_resort",
            "Canyonlands" => "location_canyonlands",
            "Casino City" => "location_casino_city",
            "Valley Farms" => "location_valley_farms",
            "Hoover Dam" => "location_hoover_dam",
            "Ghost Town" => "location_ghost_town",
            "Aircraft Graveyard" => "location_aircraft_graveyard",
            "Oil Fields" => "location_oilfield",
            "Sand Factory" => "location_sand_factory",
            "Secret Base" => "location_secret_base",
            _ => null,
        };
        return stage != null;
    }

    static bool TryGetCharacterStage(string text, out string? stage)
    {
        stage = text switch
        {
            "\"Y\" the Alien" => "character_y_the_alien",
            _ => null,
        };
        return stage != null;
    }

    public static void TraceMenuPad(CpuContext c, IMemory m)
    {
        if (_lastMenuStage != null &&
            (!_gameplayStage || _lastMenuStage.StartsWith("pause_", StringComparison.Ordinal)))
        {
            int captureDelay = _lastMenuStage switch
            {
                "memory_card" => 20,
                "passcode" or "passcode_2" => 40,
                "choose_enemies" => 100,
                "difficulty_setting" or "controller_1" or "controller_2" or
                "audio_settings" or "screen_adjustment" or "quest_route" or
                "choose_players" or "two_player_mode" => 12,
                _ => 0,
            };
            InputManager.SignalScriptStage(_lastMenuStage, captureDelay);
            if (_traceOptions && _lastMenuStage == "screen_adjustment")
            {
                int screenX = unchecked((sbyte)m.ReadU8(0x8006531Cu));
                int screenY = unchecked((sbyte)m.ReadU8(0x8006531Du));
                if (screenX != _lastScreenX || screenY != _lastScreenY)
                {
                    _lastScreenX = screenX;
                    _lastScreenY = screenY;
                    Console.Error.WriteLine(
                        $"[V8Options] event=screen-change screenX={screenX} screenY={screenY}");
                }
            }
            if (_lastMenuStage.StartsWith("pause_confirm_", StringComparison.Ordinal))
            {
                // The retail confirmation loop polls the asynchronous pad
                // hardware without waiting for VSync. The host must advance a
                // frame here so live and scripted controller state can change.
                Runtime.PresentFrame();
            }
        }
    }

    public static void TracePauseMenuPost(CpuContext c, IMemory m)
    {
        bool quit = c.V0 != 0u;
        Console.Error.WriteLine(
            $"[V8Pause] event=return result={(quit ? "quit" : "resume")} tick={_vehiclePhysicsTick}");
        _lastMenuStage = null;
        if (quit)
        {
            _gameplayStage = false;
            InputManager.SignalScriptStage("pause_quit");
        }
        else
        {
            _pauseResumePending = true;
        }
    }

    public static void TraceMenuPadPost(CpuContext c, IMemory m)
    {
        if (!_traceMenuText || _lastMenuStage == null ||
            !_lastMenuStage.StartsWith("pause_confirm_", StringComparison.Ordinal)) return;
        ushort raw = RecompOne.Runtime.Hardware.Controller.State;
        uint mapped = m.ReadU32(0x80065930u);
        if (raw == _lastPauseRawPad && mapped == _lastPauseMappedPad) return;
        _lastPauseRawPad = raw;
        _lastPauseMappedPad = mapped;
        Console.Error.WriteLine(
            $"[V8PauseInput] stage={_lastMenuStage} raw=0x{raw:X4} mapped=0x{mapped:X8}");
    }

    public static void ApplyUserGameVolume(CpuContext c, IMemory m)
    {
        if (_traceOptions && ++_optionsAudioSetCount <= 32)
        {
            Console.Error.WriteLine(
                $"[V8Options] event=apply stereo={c.A0} music={c.A1} sfx={c.A2}");
        }
        if (!_zeroGameVolume) return;

        // These are Vigilante 8's own Music and Sound Effects slider values.
        // Keep the host mixer untouched so this behaves like an in-game option.
        m.WriteU16(0x80065C04u, 0);
        m.WriteU16(0x80065BE8u, 0);
        c.A1 = 0u;
        c.A2 = 0u;
        if (!_zeroGameVolumeLogged)
        {
            _zeroGameVolumeLogged = true;
            Console.Error.WriteLine(
                "[V8Compat] in-game audio controls: Music=0 Sound Effects=0");
        }
    }

    static string ReadAudioOptions(IMemory m) =>
        $"music={m.ReadU16(0x80065C04u)} sfx={m.ReadU16(0x80065BE8u)} " +
        $"stereo={m.ReadU8(0x800658ACu)}";

    static string ReadScreenOptions(IMemory m) =>
        $"screenX={unchecked((sbyte)m.ReadU8(0x8006531Cu))} " +
        $"screenY={unchecked((sbyte)m.ReadU8(0x8006531Du))}";

    public static void TranslateOverlayDmaSource(CpuContext c, IMemory m)
    {
        if (m is not RelocatedMemory relocated) return;
        if (c.A0 < relocated.LinkedBase || c.A0 - relocated.LinkedBase >= relocated.Size) return;

        // The retail loader relocates pointers embedded in an overlay before
        // they cross into DMA hardware. Generated code retains linked virtual
        // addresses, so materialize that relocation in the source register.
        c.A0 += relocated.Delta;
    }

    static string ReadAscii(IMemory m, uint address, int maxLength)
    {
        if (!IsRetailRamRange(address, 1u)) return string.Empty;
        Span<char> chars = stackalloc char[maxLength];
        int length = 0;
        while (length < chars.Length)
        {
            byte value = m.ReadU8(address + (uint)length);
            if (value == 0) break;
            if (value is < 0x20 or > 0x7E) return string.Empty;
            chars[length++] = (char)value;
        }
        return new string(chars[..length]);
    }

    public static void TraceBinParse(CpuContext c, IMemory m)
    {
        uint data = c.A0;
        uint companion = c.A1;
        int parse = ++_binParseCount;
        var bytes = new byte[32];
        for (int i = 0; i < bytes.Length; i++) bytes[i] = m.ReadU8(data + (uint)i);
        Console.Error.WriteLine(
            $"[V8Compat] BIN parse #{parse}: data=0x{data:X8} companion=0x{companion:X8} " +
            $"words={m.ReadU32(data):X8},{m.ReadU32(data + 4u):X8},{m.ReadU32(data + 8u):X8}," +
            $"{m.ReadU32(data + 12u):X8},{m.ReadU32(data + 16u):X8},{m.ReadU32(data + 20u):X8} " +
            $"bytes={Convert.ToHexString(bytes)}");
    }

    public static void TraceStreamCopyPre(CpuContext c, IMemory m)
    {
        if (c.A1 < 0x3000u) return;

        uint offset = m.ReadU32(c.GP + 0x6ACu);
        uint source = m.ReadU32(c.GP + 0x69Cu);
        uint sourceAtOffset = source + (offset & 0x7FFu);
        _largeStreamCopy = (c.A0, c.A1, offset, sourceAtOffset);
        Console.Error.WriteLine(
            $"[V8Compat] stream copy pre: dst=0x{c.A0:X8} size=0x{c.A1:X8} offset=0x{offset:X8} " +
            $"source=0x{sourceAtOffset:X8} bytes={ReadHex(m, sourceAtOffset, 16)}");
    }

    public static void TraceStreamCopyPost(CpuContext c, IMemory m)
    {
        if (_largeStreamCopy is not { } copy) return;
        _largeStreamCopy = null;
        Console.Error.WriteLine(
            $"[V8Compat] stream copy post: dst=0x{copy.Destination:X8} size=0x{copy.Size:X8} " +
            $"offset=0x{copy.Offset:X8} bytes={ReadHex(m, copy.Destination, 16)}");
    }

    static string ReadHex(IMemory m, uint address, int count)
    {
        var bytes = new byte[count];
        for (int i = 0; i < count; i++) bytes[i] = m.ReadU8(address + (uint)i);
        return Convert.ToHexString(bytes);
    }

    /// <summary>
    /// Removes the retail preview vehicle selected on the built-in character
    /// screen from the match bank mask when an independent guest is selected.
    /// The guest is built from its own archive later; retaining the unused
    /// retail bank wastes the only native gameplay texture page.
    /// </summary>
    public static void PrepareGuestCommonObjectMask(CpuContext c, IMemory m)
    {
        if (V8VehicleRegistry.SelectedType < 0 &&
            !V8VehicleRegistry.HasDefaultReplacement)
            return;

        ReserveGuestVramForMatch(c, m);
        int retailPreviewType = (sbyte)m.ReadU8(c.GP + 0x18u);
        if (retailPreviewType < 0 || retailPreviewType >= 13)
            return;

        bool everyTypeZeroUsesIndependentReplacement =
            V8VehicleRegistry.HasDefaultReplacement &&
            retailPreviewType == 0;
        for (uint participant = 1u;
             !everyTypeZeroUsesIndependentReplacement && participant < 8u;
             participant++)
        {
            bool active = participant < 2u ||
                m.ReadU8(c.GP + 0x1Eu + participant) != 0u;
            if (active &&
                (sbyte)m.ReadU8(c.GP + 0x18u + participant) ==
                retailPreviewType)
            {
                Console.Error.WriteLine(
                    $"[V8Vehicles] retained shared retail bank {retailPreviewType} " +
                    "for another participant");
                return;
            }
        }

        uint originalMask = c.A0;
        c.A0 &= ~(1u << retailPreviewType);
        Console.Error.WriteLine(
            $"[V8Vehicles] released preview bank {retailPreviewType} from " +
            $"COMMON mask 0x{originalMask:X4}->0x{c.A0:X4}");
    }

    static void ReserveGuestVramForMatch(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        var resetState = c.Snapshot();
        c.A0 = 1u;
        Dispatcher.Call(c, m, 0x80018080u);
        c.Restore(resetState);
        Console.Error.WriteLine(
            "[V8Vehicles] reset native VRAM allocator for guest match banks");

        GuestVramReservations.Clear();
        V8VehicleRegistry.ResetRuntimeForMatch();
        IReadOnlyList<NativeVramAllocation> requests =
            V8VehicleRegistry.SelectedVramAllocations();
        if (requests.Count == 0)
            return;

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
                Dispatcher.Call(c, m, 0x80018124u);
                if (c.V0 == 0u)
                    throw new OutOfMemoryException(
                        $"V8 guest VRAM reservation failed for " +
                        $"{request.Width}x{request.Height}");
                uint x = (uint)(short)m.ReadU16(c.V0);
                uint y = (uint)(short)m.ReadU16(c.V0 + 2u);
                c.A0 = 0x18u;
                Alloc(c, m);
                uint descriptor = c.V0;
                if (descriptor == 0u)
                    throw new OutOfMemoryException(
                        "V8 synthetic VRAM descriptor allocation failed");
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
                $"[V8Vehicles] reserved {GuestVramReservations.Count} " +
                "native VRAM rectangles for selected guest");
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
                c.A0 = GuestVramReservations[index].X;
                c.A1 = GuestVramReservations[index].Y;
                Dispatcher.Call(c, m, 0x80018530u);
                if (c.V0 == 0u)
                    throw new InvalidOperationException(
                        $"reserved V8 VRAM rectangle at " +
                        $"({GuestVramReservations[index].X}," +
                        $"{GuestVramReservations[index].Y}) disappeared");
            }
            Console.Error.WriteLine(
                $"[V8Vehicles] released {GuestVramReservations.Count} " +
                "reserved native VRAM rectangles");
            GuestVramReservations.Clear();
        }
        finally
        {
            c.Restore(snapshot);
        }
    }

    public static void BeginGuestVramClaim()
    {
        _guestVramClaimIndex = 0;
        ClaimedGuestVramReservations.Clear();
        _guestVramClaimActive = GuestVramReservations.Count != 0;
    }

    public static void EndGuestVramClaim()
    {
        _guestVramClaimActive = false;
        Console.Error.WriteLine(
            $"[V8Vehicles] claimed {_guestVramClaimIndex} of " +
            $"{GuestVramReservations.Count} reserved native VRAM rectangles; " +
            "unused authored textures remain reserved for this match");
        GuestVramReservations.Clear();
        ClaimedGuestVramReservations.Clear();
    }

    public static void AbortGuestVramClaim()
    {
        _guestVramClaimActive = false;
        ClaimedGuestVramReservations.Clear();
    }

    public static bool ClaimGuestVramAllocation(CpuContext c, IMemory m)
    {
        if (!_guestVramClaimActive)
            return true;

        var request = (
            c.A0, c.A1, c.A2, c.A3,
            m.ReadU32(c.SP + 0x10u), m.ReadU32(c.SP + 0x14u));
        for (int index = 0; index < GuestVramReservations.Count; index++)
        {
            if (ClaimedGuestVramReservations.Contains(index))
                continue;

            GuestVramReservation reservation = GuestVramReservations[index];
            NativeVramAllocation expected = reservation.Request;
            if (request != (
                    expected.Width,
                    expected.Height,
                    expected.AlignWidth,
                    expected.AlignHeight,
                    expected.LimitWidth,
                    expected.LimitHeight))
                continue;

            c.V0 = reservation.Descriptor;
            ClaimedGuestVramReservations.Add(index);
            _guestVramClaimIndex++;
            return false;
        }

        Console.Error.WriteLine(
            $"[V8Vehicles] no reserved VRAM rectangle matches requested " +
            $"{request.Item1}x{request.Item2} align=" +
            $"{request.Item3}x{request.Item4} bounds=" +
            $"{request.Item5}x{request.Item6}");
        return true;
    }

    public static bool IgnoreSyntheticVramFree(CpuContext c, IMemory m) =>
        !SyntheticVramDescriptors.Contains(c.A0);

    public static void TraceVramAlloc(CpuContext c, IMemory m)
    {
        uint root = m.ReadU32(c.GP + 0x6C4u);
        int nodes = 0, free = 0, used = 0, largestArea = 0;
        var pending = new Stack<uint>();
        var visited = new HashSet<uint>();
        if (root != 0u) pending.Push(root);
        while (pending.Count != 0 && nodes < 4096)
        {
            uint node = pending.Pop();
            if (node == 0u || !visited.Add(node)) continue;
            nodes++;
            uint type = m.ReadU32(node + 8u);
            if (type == 0u)
            {
                free++;
                int width = (short)m.ReadU16(node + 4u);
                int height = (short)m.ReadU16(node + 6u);
                largestArea = Math.Max(largestArea, Math.Max(0, width * height));
            }
            else if (type == 1u) used++;
            else
            {
                pending.Push(m.ReadU32(node + 0x10u));
                pending.Push(m.ReadU32(node + 0x14u));
            }
        }

        _vramAllocCount++;
        Console.Error.WriteLine(
            $"[V8Compat] VRAM alloc #{_vramAllocCount}: {c.A0}x{c.A1} align={c.A2}x{c.A3} " +
            $"bounds={m.ReadU32(c.SP + 0x10u)}x{m.ReadU32(c.SP + 0x14u)} root=0x{root:X8} " +
            $"nodes={nodes} free={free} used={used} largestArea={largestArea}");
    }

    public static void TraceRectUpload(CpuContext c, IMemory m)
    {
        uint rect = c.A0;
        Console.Error.WriteLine(
            $"[V8Compat] rect upload: sp=0x{c.SP:X8} ra=0x{c.RA:X8} rect=0x{rect:X8} " +
            $"xywh={(short)m.ReadU16(rect)},{(short)m.ReadU16(rect + 2u)}," +
            $"{(short)m.ReadU16(rect + 4u)}x{(short)m.ReadU16(rect + 6u)}");
    }

    static void LogHeapExhausted(IMemory m, uint requestedBytes, uint headAddress)
    {
        if (_heapExhaustedLogged) return;
        _heapExhaustedLogged = true;

        uint head = m.ReadU32(headAddress);
        uint cursor = head;
        uint totalUnits = 0u;
        uint largestUnits = 0u;
        int blocks = 0;
        var visited = new HashSet<uint>();
        while (visited.Add(cursor) && blocks < 4096)
        {
            uint next = m.ReadU32(cursor);
            uint units = m.ReadU32(next + 4u);
            totalUnits += units;
            largestUnits = Math.Max(largestUnits, units);
            blocks++;
            cursor = next;
            if (cursor == head) break;
        }
        Console.Error.WriteLine($"[V8Compat] heap exhausted: request={requestedBytes} freeBlocks={blocks} freeBytes={totalUnits << 3} largestBytes={largestUnits << 3}");
    }

    // The original card library blocks here until its interrupt callback updates
    // an event counter. Recompiled code has no instruction-level scheduler, and
    // RecompOne's memory-card operations have already completed synchronously.
    public static void WaitCardEvent(CpuContext c, IMemory m)
    {
        c.V0 = Runtime.CardA.Enabled ? 0u : 1u;
    }

    public static void WaitCardOperation(CpuContext c, IMemory m)
    {
        const uint state = 0x80113438u;
        uint blocking = c.A0;
        uint result1 = c.A1;
        uint result2 = c.A2;
        if (m.ReadU32(state) == 0u && m.ReadU32(state + 8u) == 0u)
        {
            c.V0 = 0xFFFFFFFFu;
            return;
        }

        if (m.ReadU32(state + 8u) == 0u)
            DispatchLinked(c, m, 0x80110768u);

        uint complete = m.ReadU32(state + 8u);
        if (blocking == 0u && complete == 0u)
        {
            // A queued synchronous card operation may require more than one
            // library service pass, but should never need a host frame.
            for (int i = 0; i < 8 && m.ReadU32(state + 8u) == 0u; i++)
                DispatchLinked(c, m, 0x80110768u);
            complete = m.ReadU32(state + 8u);
        }

        if (complete != 0u)
        {
            if (result2 != 0u) m.WriteU32(result2, m.ReadU32(0x80113484u));
            if (result1 != 0u) m.WriteU32(result1, m.ReadU32(0x80113480u));
            m.WriteU32(state + 8u, 0u);
            c.V0 = 1u;
            return;
        }

        if (result2 != 0u) m.WriteU32(result2, m.ReadU32(state + 4u));
        if (result1 != 0u) m.WriteU32(result1, m.ReadU32(state));
        c.V0 = blocking == 0u ? 1u : 0u;
    }

    internal static void DispatchLinked(CpuContext c, IMemory m, uint address)
    {
        uint runtimeAddress = m is RelocatedMemory relocated ? address + relocated.Delta : address;
        Dispatcher.Call(c, m, runtimeAddress);
    }
}
