using RecompOne.Runtime.Context;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Hardware;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public static class V82Compat
{
    readonly record struct GuestVramReservation(
        NativeVramAllocation Request,
        uint X,
        uint Y,
        uint Descriptor);

    const uint HeapHeadAddress = 0x8006B3F8u;
    const uint ShellLinkedBase = 0x80100000u;
    const uint ShellSectorAllocation = 0x0001B000u;
    const uint PcHeapBase = 0x80200000u;
    const uint PcHeapEnd = 0x80800000u;
    static bool _shellPinnedAtLinkedBase;
    static bool _extendedHeapInstalled;
    static readonly List<(uint Start, uint Size)> PcFreeBlocks = new();
    static readonly Dictionary<uint, (uint Header, uint Size)> PcAllocations = new();
    static bool _spuMallocRetrying;
    static readonly Stack<uint> SpuMallocRequests = new();
    static readonly Stack<uint[]> ShellDecodeCallers = new();
    static readonly Stack<(uint FrameSp, uint SourceRect)> ShellImageDecodeFrames = new();
    static readonly Stack<(uint Width, uint Height, uint AlignWidth, uint AlignHeight, uint LimitWidth, uint LimitHeight)> VramRequests = new();
    static readonly List<GuestVramReservation> GuestVramReservations = [];
    static readonly HashSet<int> ClaimedGuestVramReservations = [];
    static readonly HashSet<uint> SyntheticVramDescriptors = [];
    static bool _guestVramClaimActive;
    static int _guestVramClaimIndex;
    static int _guestVramClaimMisses;
    static bool _matchVramActive;
    static int _matchVramSuccesses;
    static int _matchVramFailures;
    static int _objectFactoryTraceCount;
    static readonly Stack<uint> ObjectFactorySources = new();
    static int _objectSchedulerPass;
    static int _gameplayFrameCount;
    static int _lastTracedOrderingTableFrame;
    static bool _matchGateRepairLogged;
    static bool _geometryContinuationActive;
    static bool _geometryContinuationDriving;
    static int _geometrySuppressedLeaves;
    static int _geometryContinuationIterations;
    static int _geometryClipCount;
    static uint _textureDecodeScratchTop;
    static uint _geometryNextTarget;
    static uint _playerVehicle;
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
    static bool _unlockRosterLogged;
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

    // Keep the retail heap intact for the executable's bookkeeping helpers,
    // while routing allocations to an independent PC-only arena. Mixing a
    // devkit block into the retail free list is unsafe when malloc is entered
    // through a relocated overlay: linked addresses can then be stored as
    // physical free-list links.
    public static void ExtendHeapPost(CpuContext c, IMemory m)
    {
        if (_extendedHeapInstalled || Runtime.Mode != RunMode.Devkit) return;

        // The PC loose build already carries the original game's ten arenas,
        // so expose the retail disc-swap flag for ordinary play as well as
        // automated coverage. This remains process-local and never edits a
        // memory-card image.
        m.WriteU16(0x8006A832u, (ushort)(m.ReadU16(0x8006A832u) | 0x10u));
        Console.Error.WriteLine(
            "[V82Compat] enabled all ten original Vigilante 8 arenas for this run");
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
            "[V82Compat] initialized isolated 6 MiB PC heap at 0x80200000-0x80800000");
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

        InsertFreeBlock(allocation.Header, allocation.Size);
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
        => RunVlcRegion(c, m, 0x80110D14u, 0x80111220u, "SHELL");

    static void RunVlcRegion(
        CpuContext c, IMemory m, uint start, uint end, string region)
    {
        uint pc = start;

        for (int steps = 0; steps < 50_000_000; steps++)
        {
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
        if (source != 0u && source == _playerVehicle && c.V0 != 0u)
        {
            _playerVehicle = c.V0;
            Console.Error.WriteLine(
                $"[V82Object] player runtime object=0x{_playerVehicle:X8}");
        }
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

    static void ApplyLevelOfDetail(uint objectAddress, IMemory m)
    {
        uint highMesh = m.ReadU32(objectAddress + 0x40u);
        uint lowMesh = m.ReadU32(objectAddress + 0x68u);
        uint threshold = m.ReadU32(objectAddress + 0x6Cu);
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
        int selectedPlayerType = _soakPlayerType >= 0
            ? _soakPlayerType
            : V82VehicleRegistry.SelectedType;
        if (selectedPlayerType >= 0)
        {
            if (selectedPlayerType >= V82VehicleRegistry.RetailVehicleCount &&
                !V82VehicleRegistry.IsCustomType((uint)selectedPlayerType))
                throw new InvalidOperationException(
                    $"requested vehicle type {selectedPlayerType} is not registered");
            m.WriteU8(c.GP + 0x1104u, (byte)selectedPlayerType);
            Console.Error.WriteLine(
                $"[V82Vehicles] player-type={selectedPlayerType}");
        }
        // func_800132CC builds the two resource masks in S3/S2 after several
        // split callbacks. Those callbacks can leak their callee-saved scratch
        // registers into the generated caller. Rebuild the masks from the same
        // six retail participant bytes and status table instead of preloading
        // every vehicle bank, which exhausts the 320-wide gameplay texture
        // page and leaves the local vehicle/HUD textures unavailable.
        uint commonMask = 0x001C0000u;
        uint alternateMask = 0u;
        int primaryParticipants = (sbyte)m.ReadU8(c.GP + 0x31u) < 9 ? 2 : 4;
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
            _playerVehicle = c.A0;
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
                    uint primitiveBuffer = m.ReadU32(c.GP + 0x20u);
                    uint primitiveBase = 0x80074A68u + (primitiveBuffer << 17);
                    if (primitiveAfter < primitiveBase ||
                        primitiveAfter >= primitiveBase + 0x20000u)
                    {
                        uint restored = primitiveBefore >= primitiveBase &&
                            primitiveBefore < primitiveBase + 0x20000u
                            ? primitiveBefore
                            : primitiveBase;
                        Console.Error.WriteLine(
                            $"[V82Render] rejected primitive cursor 0x{primitiveAfter:X8} " +
                            $"from callback=0x{callback:X8} object=0x{objectAddress:X8}; " +
                            $"restored 0x{restored:X8} range=0x{primitiveBase:X8}-" +
                            $"0x{primitiveBase + 0x20000u:X8}");
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
            if (frame == 1)
                InputManager.SignalScriptStage("gameplay", captureDelayPolls: 300);
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
        _soakAutomationInput = (ushort)(movement | action);
        _soakInputPhase = phase;

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
                m.WriteU16(_soakWeaponObject + 0x1Cu, 1000);
                _soakWeaponAmmo = 1000;
                Console.Error.WriteLine(
                    $"[V82Coverage] weapon-armed kind={kind} " +
                    $"object=0x{_soakWeaponObject:X8} " +
                    $"objectKind={unchecked((sbyte)m.ReadU8(_soakWeaponObject + 9u))} " +
                    $"callback=0x{m.ReadU32(_soakWeaponObject):X8} ammo=1000");
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
    public static bool EnterGeometry22164(CpuContext c, IMemory m) =>
        EnterGeometryContinuation(0x80022164u);

    public static bool EnterGeometry22910(CpuContext c, IMemory m) =>
        EnterGeometryContinuation(0x80022910u);

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
        return IsShapeAddress(texture, 0x0Cu) ||
               SkipMalformedGeometryPrimitive(c, m, texture);
    }

    static bool SkipMalformedGeometryPrimitive(
        CpuContext c, IMemory m, uint texture)
    {
        if (_geometryClipCount++ < 16)
            Console.Error.WriteLine(
                $"[V82Geometry] skipped malformed textured primitive " +
                $"texture=0x{texture:X8} s0=0x{c.S0:X8} " +
                $"s3=0x{c.S3:X8} a3=0x{c.A3:X8}");
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
        var snapshot = c.Snapshot();
        c.A0 = 1u;
        Dispatcher.Call(c, Dispatcher.UnwrapMemory(m), 0x8002091Cu);
        c.Restore(snapshot);
        _matchVramActive = true;
        _matchVramSuccesses = 0;
        _matchVramFailures = 0;
        Console.Error.WriteLine(
            "[V82Compat] reset VRAM allocator to 320-wide gameplay layout");
        ReserveGuestVramForMatch(c, m);
    }

    public static void MarkOriginalMatchVramReset(CpuContext c, IMemory m)
    {
        if (c.RA != 0x800137A0u)
            return;
        _matchVramActive = true;
        _matchVramSuccesses = 0;
        _matchVramFailures = 0;
        Console.Error.WriteLine(
            "[V82Compat] observed original 320-wide gameplay VRAM reset");
        ReserveGuestVramForMatch(c, m);
    }

    static void ReserveGuestVramForMatch(CpuContext c, IMemory m)
    {
        GuestVramReservations.Clear();
        if (V82VehicleRegistry.SelectedType < 0 &&
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
                Dispatcher.Call(c, m, 0x80020F5Cu);
                if (c.V0 == 0u)
                    throw new InvalidOperationException(
                        $"reserved V8:2 VRAM rectangle at " +
                        $"({GuestVramReservations[index].X}," +
                        $"{GuestVramReservations[index].Y}) disappeared");
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

    public static void BeginGuestVramClaim()
    {
        _guestVramClaimIndex = 0;
        _guestVramClaimMisses = 0;
        ClaimedGuestVramReservations.Clear();
        _guestVramClaimActive = GuestVramReservations.Count != 0;
    }

    public static void EndGuestVramClaim(CpuContext c, IMemory m)
    {
        var pending = new List<GuestVramReservation>();
        for (int index = 0; index < GuestVramReservations.Count; index++)
            if (!ClaimedGuestVramReservations.Contains(index))
                pending.Add(GuestVramReservations[index]);
        int claimed = ClaimedGuestVramReservations.Count;
        GuestVramReservations.Clear();
        GuestVramReservations.AddRange(pending);
        ClaimedGuestVramReservations.Clear();
        _guestVramClaimActive = pending.Count != 0;
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
            c.A0 = 0xD00u;
            PcMalloc(c, Dispatcher.UnwrapMemory(m));
            if (c.V0 == 0u)
                throw new OutOfMemoryException(
                    "V8:2 texture decoder scratch allocation failed");
            _textureDecodeScratchTop = c.V0 + 0xC70u;
            return _textureDecodeScratchTop;
        }
        finally
        {
            c.Restore(snapshot);
        }
    }

    public static void AbortGuestVramClaim()
    {
        _guestVramClaimActive = false;
        ClaimedGuestVramReservations.Clear();
    }

    public static bool TrackVramAllocationPre(CpuContext c, IMemory m)
    {
        var request = (
            c.A0, c.A1, c.A2, c.A3,
            m.ReadU32(c.SP + 0x10u), m.ReadU32(c.SP + 0x14u));
        VramRequests.Push(request);
        if (_guestVramClaimActive)
        {
            bool palette =
                request.Item3 == 16u && request.Item4 == 1u;
            bool image =
                request.Item3 == 64u && request.Item4 == 256u;
            if ((!palette && !image) ||
                !V82VehicleRegistry.OwnsCurrentTextureLoad(
                    c, m, palette))
                return true;

            for (int index = 0; index < GuestVramReservations.Count; index++)
            {
                if (ClaimedGuestVramReservations.Contains(index))
                    continue;

                GuestVramReservation reservation =
                    GuestVramReservations[index];
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
                if (ClaimedGuestVramReservations.Count ==
                    GuestVramReservations.Count)
                {
                    _guestVramClaimActive = false;
                    GuestVramReservations.Clear();
                    ClaimedGuestVramReservations.Clear();
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

    public static bool IgnoreSyntheticVramFree(CpuContext c, IMemory m) =>
        !SyntheticVramDescriptors.Contains(c.A0);

    public static void TrackVramAllocationPost(CpuContext c, IMemory m)
    {
        if (VramRequests.Count == 0) return;
        var request = VramRequests.Pop();
        if (!_matchVramActive) return;

        if (c.V0 != 0u)
        {
            int success = ++_matchVramSuccesses;
            if (success <= 24)
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
            if (failure <= 24 || failure % 100 == 0)
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
        if (!_matchVramActive || c.A0 != outOfVramMessage)
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
    public static void PreserveShellDecodeCallerPre(CpuContext c, IMemory m)
    {
        ShellDecodeCallers.Push(
        [
            c.SP, c.RA, c.S0, c.S1, c.S2, c.S3, c.S4,
            c.S5, c.S6, c.S7, c.FP,
        ]);
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
    public static void PreserveShellImageDecodePre(CpuContext c, IMemory m) =>
        ShellImageDecodeFrames.Push((c.SP - 0x50u, c.A2));

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
