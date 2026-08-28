using System.Buffers.Binary;
using System.Text;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Append-only, file-backed native V8:2 location registry.</summary>
public static class V82ArenaRegistry
{
    const uint SelectedLocationAddress = 0x8006B418u;
    const int RetailCount = 18;
    const int NativeRecordSize = 0x10;
    const int RegistryHeaderSize = 0x0C;
    const int RegistryRecordSize = 0x18;
    const ushort RegistryVersion = 1;
    const ushort RegistryGame = 2;
    const int NativeNameMax = 18;
    const int NativeSubtitleMax = 18;
    const int RetailSubtitleMax = 14;
    const uint NativeNameLayoutAddress = 0x80115DC0u;
    const uint NativeSubtitleLayoutAddress = 0x80115DC8u;

    static readonly List<ArenaEntry> Entries = [];
    static bool _loaded;
    static uint _nativeRecords;
    static ArenaEntry? _selected;
    static bool _resourceTableLogged;
    static int _selectorGeneration;
    static bool _subtitleLayoutExpanded;
    static ushort _subtitleLayoutX;
    static ushort _subtitleLayoutWidth;
    static readonly HashSet<uint> LoggedImportedRecordSlots = [];
    static readonly bool TraceSelector =
        Environment.GetEnvironmentVariable(
            "RECOMPONE_TRACE_V82_ARENA_SELECTOR") == "1";
    static int _lastTracedSelectedSlot = -1;
    static readonly int[] SmokeArenaSlotSequence =
        ParseSmokeArenaSlotSequence();
    static int _smokeArenaSlotSequenceIndex;
    static int _smokeRequestedSlot = -1;

    public static bool IsImportedArenaSelected => _selected != null;
    public static string? SelectedStableId => _selected?.StableId;
    public static string? SelectedOverlayName =>
        _selected == null
            ? null
            : Path.ChangeExtension(_selected.Path, null)
                .Replace('\\', '_')
                .Replace('/', '_')
                .ToUpperInvariant();

    public static uint BeginNativeLocationSelector(
        CpuContext c, IMemory m, uint retailCount)
    {
        if (retailCount != RetailCount)
            return retailCount;
        EnsureLoaded();
        EnsureNativeRecords(c, m);
        ExpandNativeSubtitleLayout(m);
        _selected = null;
        _resourceTableLogged = false;
        LoggedImportedRecordSlots.Clear();
        _lastTracedSelectedSlot = -1;
        int generation = ++_selectorGeneration;
        uint total = checked((uint)(RetailCount + Entries.Count));
        byte baselineSlot = m.ReadU8(SelectedLocationAddress);
        _smokeRequestedSlot = -1;
        if (_smokeArenaSlotSequenceIndex < SmokeArenaSlotSequence.Length)
        {
            int sequenceIndex = _smokeArenaSlotSequenceIndex++;
            int requestedSlot = SmokeArenaSlotSequence[sequenceIndex];
            if ((uint)requestedSlot >= total)
                throw new InvalidDataException(
                    $"smoke arena slot {requestedSlot} is outside " +
                    $"the native selector range 0..{total - 1}");
            _smokeRequestedSlot = requestedSlot;
            Console.Error.WriteLine(
                $"[V82TransitionSmoke] arena-slot-sequence " +
                $"generation={generation} index={sequenceIndex} " +
                $"slot={requestedSlot}");
        }
        InputManager.SignalScriptStage($"choose_location_{generation}");
        Console.WriteLine(
            $"[V82Arena] native locations generation={_selectorGeneration} " +
            $"retail={RetailCount} " +
            $"imported={Entries.Count} total={RetailCount + Entries.Count} " +
            $"baseline-slot={baselineSlot}");
        return total;
    }

    public static void EndNativeLocationSelector(CpuContext c, IMemory m)
    {
        EnsureLoaded();
        if (_smokeRequestedSlot >= 0)
        {
            c.V0 = checked((uint)_smokeRequestedSlot);
            m.WriteU8(
                SelectedLocationAddress, checked((byte)_smokeRequestedSlot));
            Console.Error.WriteLine(
                $"[V82TransitionSmoke] accepted arena-slot=" +
                $"{_smokeRequestedSlot} generation={_selectorGeneration}");
        }
        _selected = EntryForSlot(c.V0);
        RestoreNativeSubtitleLayout(m);
        Console.WriteLine(
            $"[V82Arena] location=" +
            $"{(_selected?.StableId ?? $"retail.{c.V0}")}");
    }

    public static uint ResolveLaunchPath(
        CpuContext c, IMemory m, uint retailPath)
    {
        EnsureLoaded();
        uint slot = m.ReadU8(SelectedLocationAddress);
        ArenaEntry? entry = EntryForSlot(slot);
        if (entry == null)
            return retailPath;
        EnsureNativeRecords(c, m);
        _selected = entry;
        Console.WriteLine(
            $"[V82Arena] launch stable-id={entry.StableId} " +
            $"path={entry.Path}");
        return entry.RuntimePath;
    }

    public static uint NativeLocationRecordAddress(
        IMemory m, uint slot, uint retailAddress)
    {
        _ = m;
        if (slot < RetailCount)
            return retailAddress;
        int index = checked((int)slot - RetailCount);
        if ((uint)index >= (uint)Entries.Count || _nativeRecords == 0u)
            throw new InvalidDataException(
                $"native location slot {slot} has no imported record");
        uint record = _nativeRecords + checked((uint)(index * NativeRecordSize));
        if (LoggedImportedRecordSlots.Add(slot))
        {
            ArenaEntry entry = Entries[index];
            Console.WriteLine(
                $"[V82ArenaSelectorRecord] slot={slot} record=0x{record:X8} " +
                $"name=0x{entry.RuntimeName:X8} subtitle=0x{entry.RuntimeSubtitle:X8} " +
                $"title=\"{entry.Name}\" subtitle-text=\"{entry.Subtitle}\"");
        }
        return record;
    }

    public static uint NativeSelectedLocationRecord(
        IMemory m, uint retailAddress)
    {
        uint slot = m.ReadU8(SelectedLocationAddress);
        if (TraceSelector && slot != _lastTracedSelectedSlot)
        {
            Console.Error.WriteLine(
                $"[V82ArenaSelector] generation={_selectorGeneration} " +
                $"selected-slot={slot}");
            _lastTracedSelectedSlot = checked((int)slot);
        }
        return NativeLocationRecordAddress(m, slot, retailAddress);
    }

    public static uint NativeLocationBackgroundOffset(IMemory m, uint table)
    {
        uint count = m.ReadU32(table);
        if (count == 0u || count > 4096u)
            throw new InvalidDataException(
                $"native location table has invalid resource count {count}");
        uint expected = checked((uint)(RetailCount + Entries.Count + 1));
        if (count != expected)
            throw new InvalidDataException(
                $"native location table has {count} resources; expected " +
                $"{RetailCount} retail previews + {Entries.Count} registry " +
                "previews + one background");

        uint backgroundIndex = count - 1u;
        uint backgroundOffset = m.ReadU32(
            table + 4u + backgroundIndex * 4u);
        if (!_resourceTableLogged)
        {
            _resourceTableLogged = true;
            uint tableEnd = m.ReadU32(table + 4u + count * 4u);
            Console.WriteLine(
                $"[V82ArenaTable] table=0x{table:X8} resources={count} " +
                $"previews={count - 1u} background-index={backgroundIndex} " +
                $"background-offset=0x{backgroundOffset:X} " +
                $"background-bytes={tableEnd - backgroundOffset}");
            foreach (ArenaEntry entry in Entries)
            {
                uint preview = entry.PreviewIndex;
                uint start = m.ReadU32(table + 4u + preview * 4u);
                uint end = m.ReadU32(table + 4u + (preview + 1u) * 4u);
                Console.WriteLine(
                    $"[V82ArenaTable] slot={preview} id={entry.StableId} " +
                    $"preview-index={preview} preview-offset=0x{start:X} " +
                    $"preview-bytes={end - start} marker=({entry.MarkerX}," +
                    $"{entry.MarkerY}) title=\"{entry.Name}\" " +
                    $"subtitle=\"{entry.Subtitle}\"");
            }
        }
        return backgroundOffset;
    }

    static ArenaEntry? EntryForSlot(uint slot)
    {
        if (slot < RetailCount)
            return null;
        int index = checked((int)slot - RetailCount);
        return (uint)index < (uint)Entries.Count ? Entries[index] : null;
    }

    static int[] ParseSmokeArenaSlotSequence()
    {
        string? text = Environment.GetEnvironmentVariable(
            "RECOMPONE_V82_ARENA_SLOT_SEQUENCE");
        if (string.IsNullOrWhiteSpace(text))
            return [];
        try
        {
            return text.Split(
                    ',', StringSplitOptions.RemoveEmptyEntries |
                         StringSplitOptions.TrimEntries)
                .Select(int.Parse)
                .ToArray();
        }
        catch (FormatException ex)
        {
            throw new InvalidDataException(
                "RECOMPONE_V82_ARENA_SLOT_SEQUENCE must be a " +
                "comma-separated integer list", ex);
        }
    }

    static void ExpandNativeSubtitleLayout(IMemory m)
    {
        // Retail reserves a narrower backing/erase rectangle for the lower
        // geographic subtitle than for the upper arena name.  An appended
        // registry entry can legally use the full 18-character name capacity
        // in both fields; drawing it into the 14-character subtitle rectangle
        // writes beyond the area the next selection clears, leaving the outer
        // glyphs behind.  Reuse the native name rectangle's width for the
        // selector session and keep the subtitle center fixed.  This extends
        // the registry field contract for every imported arena and does not
        // inspect a map identity or draw a replacement label.
        if (_subtitleLayoutExpanded ||
            !Entries.Any(entry => entry.Subtitle.Length > RetailSubtitleMax))
            return;

        ushort nameWidth = m.ReadU16(NativeNameLayoutAddress + 4u);
        ushort subtitleX = m.ReadU16(NativeSubtitleLayoutAddress);
        ushort subtitleWidth = m.ReadU16(NativeSubtitleLayoutAddress + 4u);
        if (nameWidth <= subtitleWidth)
            throw new InvalidDataException(
                $"native location title layouts are invalid: name-width=" +
                $"{nameWidth} subtitle-width={subtitleWidth}");
        int growth = nameWidth - subtitleWidth;
        if ((growth & 1) != 0 || subtitleX < growth / 2)
            throw new InvalidDataException(
                "native location subtitle layout cannot be expanded " +
                "while preserving its center");

        _subtitleLayoutX = subtitleX;
        _subtitleLayoutWidth = subtitleWidth;
        ushort expandedX = checked((ushort)(subtitleX - growth / 2));
        m.WriteU16(NativeSubtitleLayoutAddress, expandedX);
        m.WriteU16(NativeSubtitleLayoutAddress + 4u, nameWidth);
        _subtitleLayoutExpanded = true;
        Console.WriteLine(
            $"[V82ArenaTitleLayout] expanded native subtitle backing " +
            $"x={subtitleX}->{expandedX} width={subtitleWidth}->{nameWidth} " +
            $"center={subtitleX + subtitleWidth / 2}");
    }

    static void RestoreNativeSubtitleLayout(IMemory m)
    {
        if (!_subtitleLayoutExpanded)
            return;
        m.WriteU16(NativeSubtitleLayoutAddress, _subtitleLayoutX);
        m.WriteU16(
            NativeSubtitleLayoutAddress + 4u, _subtitleLayoutWidth);
        Console.WriteLine(
            $"[V82ArenaTitleLayout] restored native subtitle backing " +
            $"x={_subtitleLayoutX} width={_subtitleLayoutWidth}");
        _subtitleLayoutExpanded = false;
    }

    static void EnsureLoaded()
    {
        if (_loaded)
            return;
        _loaded = true;

        var paths = new List<string>();
        string rootRegistry = Path.Combine(
            Runtime.ExecutableDirectory, "ARENAS.V8R");
        if (File.Exists(rootRegistry))
            paths.Add(rootRegistry);
        string mods = Runtime.ModsDirectory;
        if (Directory.Exists(mods))
        {
            paths.AddRange(Directory.EnumerateDirectories(mods)
                .Select(directory => Path.Combine(directory, "ARENAS.V8R"))
                .Where(File.Exists)
                .OrderBy(path => path, StringComparer.OrdinalIgnoreCase));
        }

        var stableIds = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (string path in paths)
        {
            foreach (ArenaEntry entry in ParseRegistry(path))
            {
                if (!stableIds.Add(entry.StableId))
                    throw new InvalidDataException(
                        $"duplicate imported arena stable ID '{entry.StableId}'");
                Entries.Add(entry);
            }
        }
        if (Entries.Count > byte.MaxValue - RetailCount + 1)
            throw new InvalidDataException(
                "imported arena registry exceeds the native selector byte index");
        if (Entries.Count != 0)
            Console.WriteLine(
                $"[V82Arena] validated {Entries.Count} native imported " +
                $"arena{(Entries.Count == 1 ? "" : "s")}");
    }

    static IReadOnlyList<ArenaEntry> ParseRegistry(string path)
    {
        byte[] data = File.ReadAllBytes(path);
        if (data.Length < RegistryHeaderSize ||
            !data.AsSpan(0, 4).SequenceEqual("V8AR"u8))
            throw new InvalidDataException(
                $"ARENAS.V8R header is invalid: {path}");
        ushort version = BinaryPrimitives.ReadUInt16LittleEndian(
            data.AsSpan(4, 2));
        ushort game = BinaryPrimitives.ReadUInt16LittleEndian(
            data.AsSpan(6, 2));
        ushort count = BinaryPrimitives.ReadUInt16LittleEndian(
            data.AsSpan(8, 2));
        ushort recordSize = BinaryPrimitives.ReadUInt16LittleEndian(
            data.AsSpan(10, 2));
        if (version != RegistryVersion || game != RegistryGame ||
            recordSize != RegistryRecordSize)
            throw new InvalidDataException(
                $"ARENAS.V8R is not a supported V8:2 registry: {path}");
        int tableEnd = checked(RegistryHeaderSize + count * recordSize);
        if (tableEnd > data.Length)
            throw new InvalidDataException(
                $"ARENAS.V8R record table is truncated: {path}");

        var result = new List<ArenaEntry>(count);
        for (int index = 0; index < count; index++)
        {
            int offset = RegistryHeaderSize + index * recordSize;
            string stableId = ReadString(data,
                BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset, 4)));
            string name = ReadString(data,
                BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset + 4, 4)));
            string subtitle = ReadString(data,
                BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset + 8, 4)));
            string arenaPath = ReadString(data,
                BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset + 12, 4)));
            ushort markerX = BinaryPrimitives.ReadUInt16LittleEndian(
                data.AsSpan(offset + 16, 2));
            ushort markerY = BinaryPrimitives.ReadUInt16LittleEndian(
                data.AsSpan(offset + 18, 2));
            ushort previewIndex = BinaryPrimitives.ReadUInt16LittleEndian(
                data.AsSpan(offset + 20, 2));
            if (string.IsNullOrWhiteSpace(stableId) ||
                string.IsNullOrWhiteSpace(name) ||
                string.IsNullOrWhiteSpace(arenaPath) ||
                Path.IsPathRooted(arenaPath) ||
                arenaPath.Contains("..", StringComparison.Ordinal))
                throw new InvalidDataException(
                    $"ARENAS.V8R entry {index} is invalid: {path}");
            subtitle = string.IsNullOrWhiteSpace(subtitle) ? name : subtitle;
            ValidateNativeAsciiField(
                name, NativeNameMax, "display name", index, path);
            ValidateNativeAsciiField(
                subtitle, NativeSubtitleMax, "subtitle", index, path);
            ValidateNativeAsciiField(
                arenaPath, int.MaxValue, "path", index, path);
            ValidatePrimaryArenaExport(path, arenaPath, stableId);
            int expectedPreview = RetailCount + Entries.Count + result.Count;
            if (previewIndex != expectedPreview)
                throw new InvalidDataException(
                    $"ARENAS.V8R entry {stableId} preview {previewIndex} " +
                    $"does not match native slot {expectedPreview}");
            result.Add(new(
                stableId, name, subtitle,
                arenaPath.Replace('/', '\\'), markerX, markerY,
                previewIndex));
        }
        return result;
    }

    static void ValidatePrimaryArenaExport(
        string registryPath, string arenaPath, string stableId)
    {
        string registryRoot = Path.GetDirectoryName(registryPath) ?? ".";
        string dllRelative = Path.ChangeExtension(
            arenaPath.Replace('\\', Path.DirectorySeparatorChar), ".DLL");
        string modCandidate = Path.Combine(registryRoot, "files", dllRelative);
        string rootCandidate = Path.Combine(registryRoot, dllRelative);
        string? dllPath = File.Exists(modCandidate)
            ? modCandidate
            : File.Exists(rootCandidate) ? rootCandidate : null;
        if (dllPath == null)
            return;

        byte[] dll = File.ReadAllBytes(dllPath);
        if (dll.Length < 24)
            throw new InvalidDataException(
                $"arena {stableId} DLL header is truncated: {dllPath}");
        uint imageSize = BinaryPrimitives.ReadUInt32LittleEndian(
            dll.AsSpan(0, 4));
        uint exportTable = BinaryPrimitives.ReadUInt32LittleEndian(
            dll.AsSpan(4, 4));
        if (imageSize == 0 || imageSize > dll.Length ||
            exportTable < 8 || exportTable > imageSize - 8)
            throw new InvalidDataException(
                $"arena {stableId} DLL export table is invalid: {dllPath}");
        int table = checked((int)exportTable);
        uint nameOffset = BinaryPrimitives.ReadUInt32LittleEndian(
            dll.AsSpan(table, 4));
        uint callbackOffset = BinaryPrimitives.ReadUInt32LittleEndian(
            dll.AsSpan(table + 4, 4));
        if (nameOffset >= imageSize || callbackOffset >= imageSize)
            throw new InvalidDataException(
                $"arena {stableId} primary DLL export is invalid: {dllPath}");
        int nameStart = checked((int)nameOffset);
        int nameEnd = Array.IndexOf(dll, (byte)0, nameStart,
            checked((int)imageSize) - nameStart);
        if (nameEnd < 0)
            throw new InvalidDataException(
                $"arena {stableId} primary DLL export is unterminated: {dllPath}");
        string export = Encoding.ASCII.GetString(
            dll, nameStart, nameEnd - nameStart);
        string stem = Path.GetFileNameWithoutExtension(arenaPath);
        if (!string.Equals(stem, export, StringComparison.Ordinal))
            throw new InvalidDataException(
                $"arena {stableId} path stem '{stem}' does not exactly match " +
                $"primary DLL export '{export}'; V8:2 lookup is case-sensitive");
    }

    static void ValidateNativeAsciiField(
        string value, int maximum, string label, int index, string path)
    {
        if (value.Any(character => character > 0x7F))
            throw new InvalidDataException(
                $"ARENAS.V8R entry {index} {label} is not ASCII: {path}");
        if (value.Length > maximum)
            throw new InvalidDataException(
                $"ARENAS.V8R entry {index} {label} exceeds the native " +
                $"{maximum}-character selector field: {path}");
    }

    static string ReadString(byte[] data, uint offset)
    {
        if (offset >= data.Length)
            throw new InvalidDataException(
                $"ARENAS.V8R string offset 0x{offset:X} is invalid");
        int start = checked((int)offset);
        int end = Array.IndexOf(data, (byte)0, start);
        if (end < 0)
            throw new InvalidDataException("ARENAS.V8R string is unterminated");
        return Encoding.UTF8.GetString(data, start, end - start);
    }

    static void EnsureNativeRecords(CpuContext c, IMemory m)
    {
        if (Entries.Count == 0)
            return;
        if (_nativeRecords != 0u &&
            m.ReadU32(_nativeRecords) == Entries[0].RuntimeName)
            return;

        int stringBytes = Entries.Sum(entry =>
            Encoding.ASCII.GetByteCount(entry.Name) + 1 +
            Encoding.ASCII.GetByteCount(entry.Subtitle) + 1 +
            Encoding.ASCII.GetByteCount(entry.Path) + 1);
        uint total = checked((uint)(
            Entries.Count * NativeRecordSize + stringBytes + 16));
        uint savedA0 = c.A0, savedA1 = c.A1, savedV0 = c.V0;
        c.A0 = 0u;
        c.A1 = total;
        V82Compat.PcRealloc(c, m);
        _nativeRecords = c.V0;
        c.A0 = savedA0; c.A1 = savedA1; c.V0 = savedV0;
        if (_nativeRecords == 0u)
            throw new OutOfMemoryException(
                "could not allocate native imported-arena records");

        uint cursor = _nativeRecords +
            checked((uint)(Entries.Count * NativeRecordSize));
        for (int index = 0; index < Entries.Count; index++)
        {
            ArenaEntry entry = Entries[index];
            entry.RuntimeName = cursor;
            cursor = WriteAscii(m, cursor, entry.Name);
            entry.RuntimeSubtitle = cursor;
            cursor = WriteAscii(m, cursor, entry.Subtitle);
            entry.RuntimePath = cursor;
            cursor = WriteAscii(m, cursor, entry.Path);
            uint record = _nativeRecords + checked((uint)(index * NativeRecordSize));
            m.WriteU32(record, entry.RuntimeName);
            m.WriteU32(record + 4u, entry.RuntimeSubtitle);
            m.WriteU16(record + 8u, entry.MarkerX);
            m.WriteU16(record + 0xAu, entry.MarkerY);
            m.WriteU32(record + 0xCu, entry.RuntimePath);
        }
    }

    static uint WriteAscii(IMemory m, uint address, string value)
    {
        byte[] bytes = Encoding.ASCII.GetBytes(value);
        for (int index = 0; index < bytes.Length; index++)
            m.WriteU8(address + (uint)index, bytes[index]);
        m.WriteU8(address + (uint)bytes.Length, 0);
        return address + checked((uint)bytes.Length) + 1u;
    }

    sealed class ArenaEntry(
        string stableId,
        string name,
        string subtitle,
        string path,
        ushort markerX,
        ushort markerY,
        ushort previewIndex)
    {
        public string StableId { get; } = stableId;
        public string Name { get; } = name;
        public string Subtitle { get; } = subtitle;
        public string Path { get; } = path;
        public ushort MarkerX { get; } = markerX;
        public ushort MarkerY { get; } = markerY;
        public ushort PreviewIndex { get; } = previewIndex;
        public uint RuntimeName { get; set; }
        public uint RuntimeSubtitle { get; set; }
        public uint RuntimePath { get; set; }
    }
}
