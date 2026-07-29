using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Host;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public sealed record ArenaRosterItem(string StableId, string DisplayName);

/// <summary>
/// Append-only V8 arena selection. A selected arena supplies its own EXP and
/// DLL; retail location files remain addressable and unchanged.
/// </summary>
public static class V8ArenaRegistry
{
    private const string DreamlandStableId = "n64.super_dreamland_64";
    private const uint LocationTableAppendAddress = 0x80111C68u;
    private const uint SelectedLocationAddress = 0x800658F8u;
    private const ushort DreamlandWheelModel = 12;
    private const ushort DreamlandMarkerX = 84;
    private const ushort DreamlandMarkerY = 222;
    private const int LocationRecordSize = 0x14;

    private static readonly ArenaRosterItem[] Items =
    [
        new(DreamlandStableId, "Super Dreamland 64"),
    ];

    private static string? _selectedStableId;
    private static readonly byte[] SavedAdjacentShellRecord =
        new byte[LocationRecordSize];
    private static bool _nativeSelectorRecordInstalled;
    private static uint _nativeStrings;
    private static uint _dreamlandPath;
    private static bool _loadingCardCaptureRequested;
    private static readonly Dictionary<string, uint> VirtualDescriptors =
        new(StringComparer.OrdinalIgnoreCase);

    public static string? SelectedStableId => _selectedStableId;
    public static bool HasArenas => Items.Length != 0;
    public static ArenaRosterItem[] Roster() => [.. Items];

    public static void Select(string? stableId)
    {
        if (string.IsNullOrWhiteSpace(stableId))
        {
            _selectedStableId = null;
            return;
        }
        ArenaRosterItem item = Items.FirstOrDefault(
            candidate => candidate.StableId.Equals(
                stableId, StringComparison.OrdinalIgnoreCase))
            ?? throw new KeyNotFoundException(
                $"requested V8 guest arena '{stableId}' is not registered");
        _selectedStableId = item.StableId;
        Console.WriteLine(
            $"[V8Arena] selected {item.DisplayName} [{item.StableId}]"
        );
    }

    public static void RequestSelection(string stableId) => Select(stableId);

    /// <summary>
    /// Appends Dreamland to the retail location table only for the lifetime of
    /// the native location selector. The 20 bytes after the retail table are
    /// another SHELL data structure, so they are saved byte-for-byte and
    /// restored before character selection resumes.
    /// </summary>
    public static void BeginNativeLocationSelector(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (_nativeSelectorRecordInstalled)
            RestoreAdjacentShellRecord(m);

        EnsureNativeStrings(c, m);
        for (int index = 0; index < LocationRecordSize; index++)
            SavedAdjacentShellRecord[index] =
                m.ReadU8(LocationTableAppendAddress + (uint)index);

        m.WriteU16(LocationTableAppendAddress + 0x00u, DreamlandWheelModel);
        m.WriteU16(LocationTableAppendAddress + 0x02u, 0);
        m.WriteU32(LocationTableAppendAddress + 0x04u, _nativeStrings);
        m.WriteU32(LocationTableAppendAddress + 0x08u, _nativeStrings + 0x20u);
        m.WriteU16(LocationTableAppendAddress + 0x0Cu, DreamlandMarkerX);
        m.WriteU16(LocationTableAppendAddress + 0x0Eu, DreamlandMarkerY);
        m.WriteU32(LocationTableAppendAddress + 0x10u, _dreamlandPath);
        _nativeSelectorRecordInstalled = true;
        _loadingCardCaptureRequested = false;
        Select(null);
        Console.WriteLine(
            "[V8Arena] appended native selector slot 10: " +
            "model=12 marker=(84,222) title=\"Super Dreamland 64\""
        );
    }

    public static void EndNativeLocationSelector(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        byte selected = unchecked((byte)c.V0);
        if (selected == 10)
            Select(DreamlandStableId);
        else
            Select(null);
        RestoreAdjacentShellRecord(m);
    }

    /// <summary>
    /// SHELL normally returns the selected table record's +0x10 path. The
    /// temporary appended record has already been restored by this point, so
    /// supply Dreamland's independently allocated path for native slot 10.
    /// </summary>
    public static void FinishNativeLocationLaunch(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (m.ReadU8(SelectedLocationAddress) != 10)
            return;
        Select(DreamlandStableId);
        c.V0 = _dreamlandPath;
        Console.WriteLine(
            $"[V8Arena] native selector launch path=0x{_dreamlandPath:X8} " +
            "\"Terrain\\\\DreamLnd.exp\""
        );
    }

    /// <summary>
    /// LOAD's loading-card caption table has ten retail pointers. Keep that
    /// immutable and resolve the appended selector slot through the engine's
    /// arena registry instead of indexing into unrelated adjacent data.
    /// </summary>
    public static uint ResolveNativeLoadingTitle(
        RecompOne.Runtime.Context.CpuContext c,
        IMemory m,
        uint retailTitle)
    {
        if (m.ReadU8(SelectedLocationAddress) != 10 ||
            _selectedStableId != DreamlandStableId)
            return retailTitle;
        if (_nativeStrings == 0u)
            EnsureNativeStrings(c, m);
        return _nativeStrings;
    }

    /// <summary>
    /// Proof-only synchronous presentation at LOAD's native end-of-card seam.
    /// Normal gameplay never pumps an extra frame here.
    /// </summary>
    public static void CaptureNativeLoadingCard(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (_loadingCardCaptureRequested ||
            m.ReadU8(SelectedLocationAddress) != 10 ||
            _selectedStableId != DreamlandStableId ||
            Environment.GetEnvironmentVariable(
                "RECOMPONE_CAPTURE_LOADING_CARD") != "1")
            return;
        _loadingCardCaptureRequested = true;
        HostWindow.RequestDisplayCapture("dreamland_loading_card");
        HostWindow.Pump();
    }

    public static bool ResolveVirtualFile(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (Runtime.Cd == null) return true;
        // FUN_80011A38 receives (descriptorTable, requestedPath).
        string path = ReadAscii(m, c.A1, 96).Replace('/', '\\');
        if (Runtime.Cd.Fs.IsVirtualLoosePath(path))
            return ResolveVirtualPath(c, m, path);
        if (_selectedStableId == null) return true;
        if (!path.Contains(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            return true;
        if (Path.GetFileName(path).Equals(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
        {
            // LOAD first resolves the extensionless arena name to the generic
            // level factory, then performs separate DLL/EXP file lookups.
            c.V0 = 0x800222A8u;
            return false;
        }
        return ResolveVirtualPath(c, m, path);
    }

    public static bool ResolveVirtualStreamFile(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (Runtime.Cd == null) return true;
        string path = ReadAscii(m, c.A0, 96).Replace('/', '\\');
        if (Runtime.Cd.Fs.IsVirtualLoosePath(path))
            return ResolveVirtualPath(c, m, path);
        if (_selectedStableId == null) return true;
        if (!path.Contains(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            return true;
        if (Path.GetFileName(path).Equals(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            path += ".EXP";
        return ResolveVirtualPath(c, m, path);
    }

    private static bool ResolveVirtualPath(
        RecompOne.Runtime.Context.CpuContext c, IMemory m, string path)
    {
        if (VirtualDescriptors.TryGetValue(path, out uint cached))
        {
            c.V0 = cached;
            return false;
        }
        if (!Runtime.Cd.Fs.Locate(path, out int lba, out uint size))
            return true;

        uint savedA0 = c.A0;
        c.A0 = 0x40u;
        V8Compat.AllocFromHead(c, m, 0x8005ED4Cu);
        uint descriptor = c.V0;
        c.A0 = savedA0;
        if (descriptor == 0u)
            throw new OutOfMemoryException(
                $"could not allocate virtual file descriptor for {path}");
        for (uint offset = 0; offset < 0x40u; offset += 4u)
            m.WriteU32(descriptor + offset, 0u);
        m.WriteU32(descriptor + 0x0Cu, checked((uint)lba));
        m.WriteU32(descriptor + 0x10u, size);
        string fileName = Path.GetFileName(path);
        for (int index = 0; index < Math.Min(12, fileName.Length); index++)
            m.WriteU8(
                descriptor + 0x14u + (uint)index, (byte)fileName[index]
            );
        VirtualDescriptors[path] = descriptor;
        c.V0 = descriptor;
        Console.WriteLine(
            $"[V8Arena] virtual file {path} lba={lba} size={size} " +
            $"descriptor=0x{descriptor:X8}"
        );
        return false;
    }

    public static void RewriteRequestedPath(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (_selectedStableId != DreamlandStableId) return;
        string path = ReadAscii(m, c.A0, 96);
        int extension = path.LastIndexOf('.');
        if (extension < 8 ||
            !(path.EndsWith(".DLL", StringComparison.OrdinalIgnoreCase) ||
              path.EndsWith(".EXP", StringComparison.OrdinalIgnoreCase)))
            return;
        int separator = Math.Max(
            path.LastIndexOf('/'), path.LastIndexOf('\\')
        );
        int stem = separator + 1;
        if (extension - stem != 8) return;
        // The replacement stem is exactly eight bytes, matching every retail
        // arena stem. Preserve the existing ".DLL"/".EXP" suffix in place;
        // WriteAscii would add a terminator over the period.
        for (int index = 0; index < 8; index++)
            m.WriteU8(
                c.A0 + (uint)stem + (uint)index,
                (byte)"DREAMLND"[index]
            );
        Console.WriteLine(
            $"[V8Arena] request {path} -> " +
            $"{path[..stem]}DREAMLND{path[extension..]}"
        );
    }

    private static string ReadAscii(IMemory m, uint address, int limit)
    {
        var bytes = new List<byte>(limit);
        for (int index = 0; index < limit; index++)
        {
            byte value = m.ReadU8(address + (uint)index);
            if (value == 0) break;
            bytes.Add(value);
        }
        return System.Text.Encoding.ASCII.GetString(bytes.ToArray());
    }

    private static void WriteAscii(IMemory m, uint address, string value)
    {
        for (int index = 0; index < value.Length; index++)
            m.WriteU8(address + (uint)index, (byte)value[index]);
        m.WriteU8(address + (uint)value.Length, 0);
    }

    private static void EnsureNativeStrings(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (_nativeStrings != 0u &&
            ReadAscii(m, _nativeStrings, 32) == "Super Dreamland 64")
            return;

        uint savedA0 = c.A0;
        uint savedV0 = c.V0;
        c.A0 = 0x80u;
        V8Compat.AllocFromHead(c, m, 0x8005ED4Cu);
        _nativeStrings = c.V0;
        c.A0 = savedA0;
        c.V0 = savedV0;
        if (_nativeStrings == 0u)
            throw new OutOfMemoryException(
                "could not allocate native Dreamland selector strings");

        WriteAscii(m, _nativeStrings, "Super Dreamland 64");
        WriteAscii(m, _nativeStrings + 0x20u, "California");
        _dreamlandPath = _nativeStrings + 0x40u;
        WriteAscii(m, _dreamlandPath, "Terrain\\DreamLnd.exp");
    }

    private static void RestoreAdjacentShellRecord(IMemory m)
    {
        if (!_nativeSelectorRecordInstalled) return;
        for (int index = 0; index < LocationRecordSize; index++)
            m.WriteU8(
                LocationTableAppendAddress + (uint)index,
                SavedAdjacentShellRecord[index]
            );
        _nativeSelectorRecordInstalled = false;
        Console.WriteLine(
            "[V8Arena] restored adjacent SHELL character data after selector"
        );
    }
}
