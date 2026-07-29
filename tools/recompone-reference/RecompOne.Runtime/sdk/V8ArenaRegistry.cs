using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

public sealed record ArenaRosterItem(string StableId, string DisplayName);

/// <summary>
/// Append-only V8 arena selection. A selected arena supplies its own EXP and
/// DLL; retail location files remain addressable and unchanged.
/// </summary>
public static class V8ArenaRegistry
{
    private static readonly ArenaRosterItem[] Items =
    [
        new("n64.super_dreamland_64", "Super Dreamland 64"),
    ];

    private static string? _selectedStableId;
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

    public static bool ResolveVirtualFile(
        RecompOne.Runtime.Context.CpuContext c, IMemory m)
    {
        if (_selectedStableId == null || Runtime.Cd == null) return true;
        // FUN_80011A38 receives (descriptorTable, requestedPath).
        string path = ReadAscii(m, c.A1, 96).Replace('/', '\\');
        if (!path.Contains(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            return true;
        if (path.Equals("DREAMLND", StringComparison.OrdinalIgnoreCase))
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
        if (_selectedStableId == null || Runtime.Cd == null) return true;
        string path = ReadAscii(m, c.A0, 96).Replace('/', '\\');
        if (!path.Contains(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            return true;
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
        if (_selectedStableId != "n64.super_dreamland_64") return;
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
        WriteAscii(m, c.A0 + (uint)stem, "DREAMLND");
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
    }
}
