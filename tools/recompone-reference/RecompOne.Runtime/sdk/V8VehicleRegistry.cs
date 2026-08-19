using System.Buffers.Binary;
using System.Text;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Strict V8 back-port of the V8:2-first independent vehicle registry.
///
/// V8 uses the shared body/stat subset. Each entry owns one complete native
/// XOBF bank and is created without occupying a retail common-object slot.
/// </summary>
public static class V8VehicleRegistry
{
    public const int RetailVehicleCount = 13;
    public const int FirstCustomType = 64;
    public const uint CustomDispatchAddress = 0x807FE000u;
    const uint BuildNativeBankAddress = 0x8001A640u;
    const uint CreateObjectAddress = 0x80021C6Cu;
    const uint GenericVehicleDispatchAddress = 0x8002A350u;

    const int HeaderSize = 20;
    const int LegacyEntrySize = 36;
    const int EntrySize = 40;
    const int StatsSize = 0x24;
    const ushort NoArchiveIndex = 0xFFFF;

    static readonly UTF8Encoding StrictUtf8 =
        new(encoderShouldEmitUTF8Identifier: false, throwOnInvalidBytes: true);
    static readonly List<VehicleEntry> Entries = [];
    static readonly List<NativeVehicleBankSource> Banks = [];
    static readonly Dictionary<uint, int> ObjectEntries = [];
    static readonly int TestPlayerType =
        int.TryParse(
            Environment.GetEnvironmentVariable("RECOMPONE_V8_PLAYER_TYPE"),
            out int testPlayerType)
            ? testPlayerType
            : -1;
    static bool _initialized;
    static bool _dispatchRegistered;
    static int _selectedType = -1;
    static string? _requestedStableId;
    static VehicleEntry? _constructingEntry;
    static VehicleEntry? _defaultReplacementEntry;
    static readonly string? DefaultReplacementStableId =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_V8_DEFAULT_REPLACEMENT");

    public static int Count => Entries.Count;
    public static int TotalVehicleCount => RetailVehicleCount + Entries.Count;
    public static bool HasPackage => Entries.Count != 0;
    public static bool HasDefaultReplacement =>
        _defaultReplacementEntry != null;
    public static int SelectedType => Volatile.Read(ref _selectedType);

    public static VehicleRosterItem[] Roster()
    {
        return Entries
            .OrderBy(entry => entry.SelectionOrder)
            .ThenBy(entry => entry.Type)
            .Select(entry => new VehicleRosterItem(
                entry.Type,
                entry.StableId,
                entry.DisplayName,
                entry.SelectionOrder))
            .ToArray();
    }

    public static void SelectType(int type)
    {
        if (type >= 0 && !IsCustomType((uint)type))
            throw new ArgumentOutOfRangeException(
                nameof(type), type, "vehicle type is not in the V8 guest roster");
        Volatile.Write(ref _selectedType, type);
        Console.Error.WriteLine(
            type < 0
                ? "[V8Vehicles] using the built-in roster selection"
                : $"[V8Vehicles] selected guest type={type} name={NameForType(type)}");
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
        if (!_dispatchRegistered)
        {
            Dispatcher.RegisterHostFunction(
                CustomDispatchAddress, DispatchCustomVehicle);
            _dispatchRegistered = true;
        }
        Console.Error.WriteLine(
            $"[V8Vehicles] validated {Entries.Count} independent vehicle" +
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
        _defaultReplacementEntry = null;
        Volatile.Write(ref _selectedType, -1);
        ParseArchive(archivePath);
        ParseRegistry(File.ReadAllBytes(registryPath));
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
                $"requested V8 default replacement " +
                $"'{DefaultReplacementStableId}' is not registered");
        Console.Error.WriteLine(
            $"[V8Vehicles] retail type 0 keeps its roster identity and uses " +
            $"independent model {_defaultReplacementEntry.StableId}");
    }

    static void ApplyRequestedSelection()
    {
        VehicleEntry? entry = Entries.FirstOrDefault(candidate =>
            candidate.StableId.Equals(
                _requestedStableId, StringComparison.Ordinal));
        if (entry == null)
            throw new InvalidDataException(
                $"requested V8 guest vehicle '{_requestedStableId}' is not registered");
        SelectType(entry.Type);
    }

    public static bool IsCustomType(uint type)
    {
        uint local = type - FirstCustomType;
        return local < (uint)Entries.Count;
    }

    public static void PrepareSelectedRuntime(CpuContext c, IMemory m)
    {
        int selected = SelectedType;
        VehicleEntry? entry = null;
        if (selected >= 0)
            TryEntryForType((uint)selected, out entry);
        entry ??= _defaultReplacementEntry;
        if (entry == null)
            return;
        EnsureRuntime(entry, c, Dispatcher.UnwrapMemory(m));
    }

    internal static IReadOnlyList<NativeVramAllocation>
        SelectedVramAllocations()
    {
        int selected = SelectedType;
        VehicleEntry? entry = null;
        if (selected >= 0)
            TryEntryForType((uint)selected, out entry);
        entry ??= _defaultReplacementEntry;
        if (entry == null)
            return Array.Empty<NativeVramAllocation>();
        return Banks[entry.BodyArchiveIndex].ReadVramAllocations(
            palettesFirst: false);
    }

    public static void ResetRuntimeForMatch()
    {
        ObjectEntries.Clear();
        _constructingEntry = null;
        foreach (VehicleEntry entry in Entries)
        {
            entry.BodyRuntime = 0u;
            entry.StatsRuntime = 0u;
        }
    }

    public static string? NameForType(int type)
    {
        int local = type - FirstCustomType;
        return local >= 0 && local < Entries.Count
            ? Entries[local].DisplayName
            : null;
    }

    /// <summary>
    /// Pre-hook for LAB_80021E5C. Returning false skips its fixed retail
    /// callback/model tables after independently creating the custom entry.
    /// </summary>
    public static bool CreateVehicle(CpuContext c, IMemory m)
    {
        m = Dispatcher.UnwrapMemory(m);
        uint source = c.A0;
        int selectedType = TestPlayerType >= 0
            ? TestPlayerType
            : SelectedType;
        if (selectedType >= 0 && source != 0u &&
            (short)m.ReadU16(source + 0x06u) <= 0)
        {
            if (!IsCustomType((uint)selectedType))
                throw new InvalidOperationException(
                    $"requested vehicle type {selectedType} is not registered");
            c.A1 = checked((uint)selectedType);
        }
        uint requestedType = c.A1;
        bool replacingDefault =
            requestedType == 0u && _defaultReplacementEntry != null;
        VehicleEntry? entry = replacingDefault
            ? _defaultReplacementEntry
            : null;
        if (entry == null &&
            !TryEntryForType(requestedType, out entry))
            return true;

        if (source == 0u)
        {
            c.V0 = 0u;
            return false;
        }

        EnsureRuntime(entry, c, m);
        m.WriteU32(source + 0x64u, CustomDispatchAddress);
        m.WriteU16(source + 0x0Au, checked((ushort)entry.BodyKind));
        m.WriteU32(source + 0x58u, entry.BodyRuntime);

        uint callerRa = c.RA;
        uint vehicle;
        _constructingEntry = entry;
        V8Compat.BeginGuestVramClaim();
        try
        {
            c.A0 = source;
            c.RA = CustomDispatchAddress;
            Dispatcher.Call(c, m, CreateObjectAddress);
            vehicle = c.V0;
            V8Compat.EndGuestVramClaim();
        }
        catch
        {
            V8Compat.AbortGuestVramClaim();
            throw;
        }
        finally
        {
            _constructingEntry = null;
            c.RA = callerRa;
        }
        if (vehicle == 0u)
            return false;

        int velocitySource = (short)m.ReadU16(source + 0x06u);
        if (velocitySource > 0)
        {
            int x = (short)m.ReadU16(vehicle + 0x14u);
            int z = (short)m.ReadU16(vehicle + 0x20u);
            m.WriteU32(vehicle + 0x80u, unchecked((uint)((x * 4577) >> 5)));
            m.WriteU32(vehicle + 0x84u, 0u);
            m.WriteU32(vehicle + 0x88u, unchecked((uint)((z * 4577) >> 5)));
        }

        ObjectEntries[vehicle] = entry.Type - FirstCustomType;
        c.V0 = vehicle;
        Console.Error.WriteLine(
            $"[V8Vehicles] created {entry.StableId} " +
            $"identity={(replacingDefault ? 0 : entry.Type)} " +
            $"object=0x{vehicle:X8}");
        return false;
    }

    public static uint StatsPointerForObject(
        uint vehicle, uint retailPointer)
    {
        return TryEntryForObject(null, vehicle, out VehicleEntry? entry) &&
               entry.StatsRuntime != 0u
            ? entry.StatsRuntime
            : retailPointer;
    }

    public static uint WheelBankForObject(
        IMemory m, uint vehicle, uint retailPointer)
    {
        VehicleEntry? entry = _constructingEntry;
        if (entry == null)
        {
            if (!TryEntryForObject(m, vehicle, out entry))
                return retailPointer;
        }
        if (entry.BodyRuntime == 0u)
            throw new InvalidOperationException(
                $"custom V8 vehicle 0x{vehicle:X8} has no owned wheel bank");
        return entry.BodyRuntime;
    }

    static void DispatchCustomVehicle(CpuContext c, IMemory m)
    {
        VehicleEntry? entry = _constructingEntry;
        if (entry == null &&
            !TryEntryForObject(m, c.A0, out entry))
            throw new InvalidOperationException(
                $"custom V8 callback has no independent entry for 0x{c.A0:X8}");
        EnsureRuntime(entry, c, Dispatcher.UnwrapMemory(m));
        c.A3 = entry.StatsRuntime;
        Dispatcher.Call(c, m, GenericVehicleDispatchAddress);
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
        IMemory? m, uint vehicle, out VehicleEntry? entry)
    {
        if (ObjectEntries.TryGetValue(vehicle, out int local) &&
            (uint)local < (uint)Entries.Count)
        {
            entry = Entries[local];
            return true;
        }

        if (m != null)
        {
            uint body = m.ReadU32(vehicle + 0x58u);
            entry = Entries.FirstOrDefault(
                candidate =>
                    candidate.BodyRuntime != 0u &&
                    candidate.BodyRuntime == body);
            if (entry != null)
                return true;
        }

        entry = null;
        return false;
    }

    static void EnsureRuntime(VehicleEntry entry, CpuContext c, IMemory m)
    {
        if (entry.BodyRuntime != 0u)
            return;

        var state = c.Snapshot();
        try
        {
            entry.StatsRuntime = AllocateBytes(c, m, entry.Stats);
            entry.BodyRuntime = BuildNativeBank(c, m, Banks[entry.BodyArchiveIndex]);
            Console.Error.WriteLine(
                $"[V8Vehicles] built {entry.StableId} " +
                $"body=0x{entry.BodyRuntime:X8}");
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
            throw new OutOfMemoryException("native V8 object-bank build failed");
        return c.V0;
    }

    static uint AllocateBytes(CpuContext c, IMemory m, byte[] data)
    {
        c.A0 = checked((uint)data.Length);
        V8Compat.Alloc(c, m);
        uint pointer = c.V0;
        if (pointer == 0u)
            throw new OutOfMemoryException(
                $"V8 custom vehicle allocation failed for {data.Length} bytes");
        for (int index = 0; index < data.Length; index++)
            m.WriteU8(pointer + (uint)index, data[index]);
        return pointer;
    }

    static bool TryFindPackage(out string registryPath, out string archivePath)
    {
        var roots = new List<string>();
        string? package =
            Environment.GetEnvironmentVariable("RECOMPONE_V8_VEHICLE_PACKAGE");
        if (!string.IsNullOrWhiteSpace(package))
            roots.Add(Path.GetFullPath(package));
        string? loose = Environment.GetEnvironmentVariable("RECOMPONE_LOOSE_DIR");
        if (!string.IsNullOrWhiteSpace(loose) && loose != "0")
            roots.Add(Path.GetFullPath(loose));
        roots.Add(Runtime.ExecutableDirectory);
        roots.Add(Environment.CurrentDirectory);

        foreach (string root in roots.Distinct(StringComparer.OrdinalIgnoreCase))
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
        if ((!legacy && !current) || game != 1 || reserved != 0)
            throw new InvalidDataException(
                "VEHICLES.V8R is not a supported V8 registry");
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
            ushort rearSuspensionDamping = current
                ? U16(data, record + 36)
                : (ushort)0;
            ushort extensionReserved = current
                ? U16(data, record + 38)
                : (ushort)0;
            if (flags != 0 || statSize != StatsSize ||
                transformArchiveIndex != NoArchiveIndex ||
                transformOffset != 0 || powerupOffset != 0 ||
                entryReserved != 0 || rearSuspensionDamping != 0 ||
                extensionReserved != 0)
                throw new InvalidDataException(
                    $"custom V8 entry {index} has sequel-only or unsupported data");

            RequireRange(data, statOffset, StatsSize, $"entry {index} stats");
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
            Entries.Add(new VehicleEntry(
                FirstCustomType + index,
                stableId,
                displayName,
                bodyArchiveIndex,
                bodyKind,
                selectionOrder,
                stats));
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
                !referenced.Add(entry.BodyArchiveIndex))
                throw new InvalidDataException(
                    $"vehicle {entry.StableId} has invalid or shared archive ownership");
            byte[] body = Banks[entry.BodyArchiveIndex].ReadBin();
            int bodySlotCount = BinSlotCount(body);
            if (entry.BodyKind >= bodySlotCount ||
                !BinSlotIsTopLevel(body, entry.BodyKind))
                throw new InvalidDataException(
                    $"vehicle {entry.StableId} body kind does not own a top-level object");
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
        if (count > int.MaxValue || 0x1Cu + count * 0x1Cu > bin.Length)
            throw new InvalidDataException("native BIN slot table is truncated");
        return (int)count;
    }

    static bool BinSlotIsTopLevel(byte[] bin, int target)
    {
        int count = BinSlotCount(bin);
        for (int index = 0; index < count; index++)
        {
            int child = U16(bin, 0x1C + index * 0x1C + 0x1A);
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
        int bodyKind,
        int selectionOrder,
        byte[] stats)
    {
        public int Type { get; } = type;
        public string StableId { get; } = stableId;
        public string DisplayName { get; } = displayName;
        public int BodyArchiveIndex { get; } = bodyArchiveIndex;
        public int BodyKind { get; } = bodyKind;
        public int SelectionOrder { get; } = selectionOrder;
        public byte[] Stats { get; } = stats;
        public uint BodyRuntime { get; set; }
        public uint StatsRuntime { get; set; }
    }
}
