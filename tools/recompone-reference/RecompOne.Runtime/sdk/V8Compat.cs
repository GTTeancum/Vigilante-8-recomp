using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class V8Compat
{
    readonly record struct HeapAllocation(uint Units, uint RequestedBytes, uint Caller, int Operation);

    static bool _heapCycleLogged;
    static bool _heapExhaustedLogged;
    static int _vramAllocCount;
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
    static readonly bool _victoryAutopilot =
        Environment.GetEnvironmentVariable("RECOMPONE_V8_VICTORY_AUTOPILOT") == "1";
    static readonly string? _stateTracePath =
        Environment.GetEnvironmentVariable("RECOMPONE_STATE_TRACE_PATH");
    static readonly int _soakHeartbeatTicks =
        int.TryParse(Environment.GetEnvironmentVariable("RECOMPONE_SOAK_HEARTBEAT_TICKS"),
            out int heartbeatTicks)
            ? Math.Max(1, heartbeatTicks)
            : 0;
    static readonly string? _automationTargetLocation =
        Environment.GetEnvironmentVariable("RECOMPONE_TARGET_LOCATION");
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
    static readonly HashSet<uint> _playerProjectiles = new();
    static readonly HashSet<string> _seenMenuText = new(StringComparer.Ordinal);
    static readonly Dictionary<uint, uint> _lastAnimationPointers = new();
    static Timer? _animationWatchdog;
    static CpuContext? _animationWatchContext;
    static IMemory? _animationWatchMemory;
    static uint _casinoScatterObject;
    static uint _casinoScatterMode;
    static bool _casinoScatterEndFixLogged;
    static StreamWriter? _stateTraceWriter;
    static bool _stateTraceUnavailable;

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

    public static void TraceCasinoScatterPre(CpuContext c, IMemory m)
    {
        _casinoScatterObject = c.A0;
        _casinoScatterMode = c.A1;
    }

    public static void FixCasinoScatterAnimationEnd(CpuContext c, IMemory m)
    {
        if (_casinoScatterMode != 5u || _casinoScatterObject == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        if (m.ReadU32(_casinoScatterObject + 0x60u) != 0u) return;

        // The Casino City scatter object clears its animation cursor on event
        // 5 but returns zero. Object_PreTick then follows address zero as if it
        // were another keyframe. Return the animation walker's documented
        // negative abort value when this exact end-of-sequence state occurs.
        c.V0 = 0xFFFFFFFFu;
        if (!_casinoScatterEndFixLogged)
        {
            _casinoScatterEndFixLogged = true;
            Console.Error.WriteLine(
                $"[V8Compat] Casino City scatter animation ended cleanly " +
                $"object=0x{_casinoScatterObject:X8}");
        }
    }

    public static void Alloc(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint requestedBytes = c.A0;
        if (requestedBytes == 0u)
        {
            c.V0 = 0u;
            return;
        }

        uint units = (requestedBytes + 15u) >> 3;
        const uint headAddress = 0x8005ED4Cu;
        ReserveLinkedOverlayRanges(m, headAddress);
        int operation = ++_heapOperation;
        ValidateFreeList(m, $"before alloc #{operation} request={requestedBytes} caller=0x{c.RA:X8}");
        ValidateLiveAllocations(m, $"before alloc #{operation} request={requestedBytes} caller=0x{c.RA:X8}");
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
                _liveHeapAllocations[c.V0] = new HeapAllocation(units, requestedBytes, c.RA, operation);
                _lastHeapOperation = $"alloc #{operation} ptr=0x{c.V0:X8} bytes={requestedBytes} units={units} caller=0x{c.RA:X8}";
                ValidateFreeList(m, $"after {_lastHeapOperation}");
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
        m = Dispatcher.UnwrapMemory(m);
        if (c.A0 == 0u) return;

        Dispatcher.ForgetObjectOwner(c.A0);
        if (_objectHistory.ContainsKey(c.A0))
            RecordObjectEvent(c.A0, $"heap-free caller=0x{c.RA:X8}");

        const uint headAddress = 0x8005ED4Cu;
        int operation = ++_heapOperation;
        ValidateFreeList(m, $"before free #{operation} ptr=0x{c.A0:X8} caller=0x{c.RA:X8}");
        ReconcileFreedAllocation(m, c.A0, operation, c.RA);
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
                _lastHeapOperation = $"free #{operation} ptr=0x{c.A0:X8} caller=0x{c.RA:X8}";
                ValidateFreeList(m, $"after {_lastHeapOperation}");
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
        RecordObjectEvent(c.A0,
            $"retire flags=0x{m.ReadU32(c.A0):X8} cb=0x{m.ReadU32(c.A0 + 0x64u):X8} caller=0x{c.RA:X8}");
        if (_traceWeapons && _playerProjectiles.Remove(c.A0))
        {
            Console.Error.WriteLine(
                $"[V8Weapon] tick={_vehiclePhysicsTick} event=retire projectile=0x{c.A0:X8} " +
                $"flags=0x{m.ReadU32(c.A0):X8} state8={m.ReadU8(c.A0 + 8u)} " +
                $"callback=0x{m.ReadU32(c.A0 + 0x64u):X8} pos={ReadVec3(m, c.A0 + 0x24u)}");
        }
    }

    public static void TracePlayerProjectileRegister(CpuContext c, IMemory m)
    {
        if (!_traceWeapons || !_playerProjectiles.Contains(c.A0)) return;
        m = Dispatcher.UnwrapMemory(m);
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

    public static void TraceVehiclePhysicsTick(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (player == 0u || c.A0 != player) return;

        int tick = ++_vehiclePhysicsTick;
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
                $"[Soak] gameplay tick={tick} match_mode={m.ReadU8(c.GP + 0x15u)} " +
                $"player=0x{player:X8} " +
                $"pos={ReadVec3(m, player + 0x24u)} vel={ReadVec3(m, player + 0x80u)}" +
                player2State);
        }
        TracePlayerWeaponState(m, player, tick);
        if (!_traceVehicle || tick > 900) return;
        if (tick is 1 or 15 or 30 or 60 or 120 or 180 or 300 or 420 or 510 or 600 or 720 or 780 or 840 or 900)
            HostWindow.RequestDisplayCapture($"physics_{tick:000}");
        Console.Error.WriteLine(
            $"[V8Physics] tick={tick} phase=begin obj=0x{player:X8} " +
            $"flags=0x{m.ReadU32(player):X8} upY={unchecked((short)m.ReadU16(player + 0x18u))} " +
            $"pos={ReadVec3(m, player + 0x24u)} vel={ReadVec3(m, player + 0x80u)} " +
            $"ang={ReadVec3(m, player + 0x90u)} matrix={ReadMatrix(m, player + 0x10u)}");
    }

    public static ushort GetAutomationInputMask() =>
        _victoryAutopilot ? _victoryAutopilotInput : (ushort)0;

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
        if (!_victoryAutopilot) return;

        ushort fire = (ushort)(Controller.R2 | Controller.L2);
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
        if (!_traceVehicle || c.A1 == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint player = m.ReadU32(c.GP + 0x7D0u);
        if (player == 0u || c.A0 != player) return;

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
        if (!_traceVehicle || _collidingVehicle == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
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
        if (!_traceWeapons) return;
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
        if (!_traceWeapons || !_pendingPlayerChildSpawn) return;
        m = Dispatcher.UnwrapMemory(m);
        _pendingPlayerChildSpawn = false;
        uint projectile = c.V0;
        if (projectile == 0u) return;
        _playerProjectiles.Add(projectile);
        Console.Error.WriteLine(
            $"[V8Weapon] tick={_vehiclePhysicsTick} event=spawn-post projectile=0x{projectile:X8} " +
            $"weapon=0x{_pendingPlayerSpawnWeapon:X8} kind={_pendingPlayerSpawnKind} " +
            $"caller=0x{_pendingPlayerSpawnCaller:X8} pos={ReadVec3(m, projectile + 0x24u)}");
    }

    public static void TracePlayerWeaponHitPre(CpuContext c, IMemory m)
    {
        if (!_traceWeapons || !_playerProjectiles.Contains(c.A0) || c.A1 == 0u) return;
        m = Dispatcher.UnwrapMemory(m);
        uint target = m.ReadU32(c.A1);
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
            $"player=0x{player:X8} state={ReadVehicleDamageState(m, player)}");
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

    static void DispatchLinked(CpuContext c, IMemory m, uint address)
    {
        uint runtimeAddress = m is RelocatedMemory relocated ? address + relocated.Delta : address;
        Dispatcher.Call(c, m, runtimeAddress);
    }
}
