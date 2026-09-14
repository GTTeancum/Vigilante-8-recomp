using System.Security.Cryptography;
using System.Text.Json;
using RecompOne.Runtime.Context;
using RecompOne.Runtime.Memory;

namespace RecompOne.Runtime.Sdk;

/// <summary>Authored material masks shared by every native color-bank caller.</summary>
public static class V82VehiclePaint
{
    static readonly Dictionary<string, HashSet<int>> Masks = LoadMasks();
    static HashSet<int>? _paletteMask;

    static Dictionary<string, HashSet<int>> LoadMasks()
    {
        using Stream stream = typeof(V82VehiclePaint).Assembly.GetManifestResourceStream(
            "RecompOne.Runtime.sdk.VehicleLightPalettes.json")
            ?? throw new InvalidOperationException("Missing vehicle light material masks");
        using JsonDocument document = JsonDocument.Parse(stream);
        return document.RootElement.EnumerateObject().ToDictionary(
            entry => entry.Name,
            entry => entry.Value.EnumerateArray().Select(value => value.GetInt32()).ToHashSet());
    }

    public static void PreparePalette(CpuContext c, IMemory m)
    {
        _paletteMask = null;
        int count = m.ReadU16(c.SP + 0x34u);
        if (count is <= 0 or > 256) return;
        uint source = m.ReadU32(c.SP + 0x18u);
        Span<byte> bytes = stackalloc byte[count * 2];
        for (int i = 0; i < bytes.Length; i++) bytes[i] = m.ReadU8(source + (uint)i);
        Masks.TryGetValue(Convert.ToHexString(SHA256.HashData(bytes)), out _paletteMask);
    }

    public static void PreserveLightEntry(CpuContext c, IMemory m)
    {
        // S2 increments before the native conversion. Source-authored light
        // entries keep their RGB; identical body paint uses another index.
        int index = checked((int)c.S2 - 1);
        if (_paletteMask?.Contains(index) == true)
            c.V0 = m.ReadU16(m.ReadU32(c.SP + 0x18u) + (uint)index * 2u);
    }
}
