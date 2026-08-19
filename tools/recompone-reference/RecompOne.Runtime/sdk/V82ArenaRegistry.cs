using System.Text;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Dispatch;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Append-only native location entry for the V8:2 Dreamland mod.</summary>
public static class V82ArenaRegistry
{
    public const string DreamlandStableId = "n64.super_dreamland_64";
    const uint AppendAddress = 0x80115D08u;
    const uint SelectedLocationAddress = 0x8006B418u;
    const int RecordSize = 0x10;
    static readonly byte[] SavedRecord = new byte[RecordSize];
    static uint _strings;
    static uint _path;
    static bool _installed;
    public static bool IsDreamlandSelected { get; private set; }
    public static bool IsDreamlandHighlighted { get; private set; }

    public static uint BeginNativeLocationSelector(
        CpuContext c, IMemory m, uint retailCount)
    {
        if (retailCount != 18u) return retailCount;
        if (_installed) Restore(m);
        EnsureStrings(c, m);
        for (int index = 0; index < RecordSize; index++)
            SavedRecord[index] = m.ReadU8(AppendAddress + (uint)index);
        m.WriteU32(AppendAddress, _strings);
        m.WriteU32(AppendAddress + 4u, _strings + 0x20u);
        m.WriteU16(AppendAddress + 8u, 84);
        m.WriteU16(AppendAddress + 0xAu, 222);
        m.WriteU32(AppendAddress + 0xCu, _path);
        _installed = true;
        IsDreamlandSelected = false;
        IsDreamlandHighlighted = false;
        Console.WriteLine(
            "[V82Arena] appended native location 18: Super Dreamland 64");
        return 19u;
    }

    public static void EndNativeLocationSelector(CpuContext c, IMemory m)
    {
        IsDreamlandSelected = c.V0 == 18u;
        IsDreamlandHighlighted = false;
        Restore(m);
        Console.WriteLine(
            $"[V82Arena] location={(IsDreamlandSelected ? DreamlandStableId : $"retail.{c.V0}")}");
    }

    public static uint ResolveLaunchPath(CpuContext c, IMemory m, uint retailPath)
    {
        if (m.ReadU8(SelectedLocationAddress) != 18u) return retailPath;
        EnsureStrings(c, m);
        IsDreamlandSelected = true;
        Console.WriteLine(
            "[V82Arena] launch LEVELS\\N64\\DREAMLND.EXP");
        return _path;
    }

    public static void TrackNativeLocationHighlight(uint selected) =>
        IsDreamlandHighlighted = selected == 18u;

    public static uint NativeLocationArtIndex(uint selected) =>
        selected == 18u ? 17u : selected;

    public static bool ResolveVirtualFile(CpuContext c, IMemory m)
    {
        if (!IsDreamlandSelected || Runtime.Cd == null)
            return true;

        m = Dispatcher.UnwrapMemory(m);
        string path = ReadAscii(m, c.A0, 96).Replace('/', '\\');
        if (!path.Contains("DREAMLND", StringComparison.OrdinalIgnoreCase))
            return true;
        if (Path.GetFileName(path).Equals(
                "DREAMLND", StringComparison.OrdinalIgnoreCase))
            path += ".EXP";
        if (!Runtime.Cd.Fs.Locate(path, out int lba, out uint size))
            return true;

        // func_80018210's callers consume +0x0C/+0x10 synchronously. Use the
        // otherwise-unused callee frame, matching the appended XA resolver.
        uint descriptor = c.SP - 0x20u;
        for (uint offset = 0; offset < 0x20u; offset += 4u)
            m.WriteU32(descriptor + offset, 0u);
        m.WriteU32(descriptor + 0x0Cu, checked((uint)lba));
        m.WriteU32(descriptor + 0x10u, size);
        c.V0 = descriptor;
        Console.WriteLine(
            $"[V82Arena] virtual file {path} lba={lba} size={size} " +
            $"descriptor=0x{descriptor:X8}");
        return false;
    }

    static void EnsureStrings(CpuContext c, IMemory m)
    {
        if (_strings != 0u && m.ReadU8(_strings) == (byte)'S') return;
        uint a0 = c.A0, a1 = c.A1, v0 = c.V0;
        c.A0 = 0u;
        c.A1 = 0x80u;
        V82Compat.PcRealloc(c, m);
        _strings = c.V0;
        c.A0 = a0; c.A1 = a1; c.V0 = v0;
        if (_strings == 0u)
            throw new OutOfMemoryException("could not allocate Dreamland selector strings");
        WriteAscii(m, _strings, "Super Dreamland 64");
        WriteAscii(m, _strings + 0x20u, "Super Dreamland 64");
        _path = _strings + 0x40u;
        WriteAscii(m, _path, "LEVELS\\N64\\DREAMLND.EXP");
    }

    static void WriteAscii(IMemory m, uint address, string value)
    {
        for (int index = 0; index < value.Length; index++)
            m.WriteU8(address + (uint)index, (byte)value[index]);
        m.WriteU8(address + (uint)value.Length, 0);
    }

    static string ReadAscii(IMemory m, uint address, int maximum)
    {
        var text = new StringBuilder();
        for (int index = 0; index < maximum; index++)
        {
            byte value = m.ReadU8(address + (uint)index);
            if (value == 0) break;
            if (value < 0x20 || value > 0x7E) return string.Empty;
            text.Append((char)value);
        }
        return text.ToString();
    }

    static void Restore(IMemory m)
    {
        if (!_installed) return;
        for (int index = 0; index < RecordSize; index++)
            m.WriteU8(AppendAddress + (uint)index, SavedRecord[index]);
        _installed = false;
    }
}
