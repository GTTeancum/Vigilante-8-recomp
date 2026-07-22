using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
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
    static string _lastHeapOperation = "heap initialization";
    static readonly Dictionary<uint, HeapAllocation> _liveHeapAllocations = new();
    static readonly Dictionary<uint, List<string>> _objectHistory = new();

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
        RecordObjectEvent(c.A0,
            $"retire flags=0x{m.ReadU32(c.A0):X8} cb=0x{m.ReadU32(c.A0 + 0x64u):X8} caller=0x{c.RA:X8}");
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
                throw new InvalidOperationException(
                    $"Vigilante 8 terrain query outside the 32x32 tile map: " +
                    $"x=0x{c.A0:X8} z=0x{c.A1:X8} block={bx},{bz} caller=0x{c.RA:X8}");

            uint tileSlot = 0x800911A0u + bx * 0x80u + bz * 4u;
            uint tile = m.ReadU32(tileSlot);
            if (!IsRetailRamRange(tile, 0x2000u))
                throw new InvalidOperationException(
                    $"Vigilante 8 terrain tile pointer is invalid: x=0x{c.A0:X8} z=0x{c.A1:X8} " +
                    $"block={bx},{bz} slot=0x{tileSlot:X8} tile=0x{tile:X8} caller=0x{c.RA:X8}");
        }
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
