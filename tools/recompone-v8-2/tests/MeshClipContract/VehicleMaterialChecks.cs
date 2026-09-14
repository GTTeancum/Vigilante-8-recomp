using System.Reflection;
using RecompOne.Runtime;
using RecompOne.Runtime.Hle;

static class VehicleMaterialChecks
{
    public static int Run()
    {
        var gpu = new Gpu();
        const BindingFlags instance = BindingFlags.NonPublic | BindingFlags.Instance;
        var type = typeof(Gpu);
        type.GetField("_currentOtPacketVehicle", instance)!.SetValue(gpu, true);
        var blend = type.GetField("_blendMode", instance)!;
        var prim = type.GetMethod("PrimOf", instance)!;
        int checks = 0;
        // A captured native shadow uses CLUT 0x7F60 (row 509), which is also
        // used by panes. Palette location cannot override the packet blend.
        foreach (int clut in new[] { 0x7F60, 0x7800 })
        foreach (int mode in new[] { 0, 1, 2, 3 })
        {
            blend.SetValue(gpu, mode);
            var flags = (PrimFlags)prim.Invoke(gpu, [true, true, false, clut, false])!;
            var expected = mode switch {
                1 => HleMaterialKind.Additive,
                2 => HleMaterialKind.Subtractive,
                _ => HleMaterialKind.Glass,
            };
            if (flags.Material != expected)
                throw new Exception($"Vehicle CLUT {clut:X4} must retain blend {mode}: {flags.Material} != {expected}");
            var opaque = (PrimFlags)prim.Invoke(gpu, [true, false, false, clut, false])!;
            if (opaque.Material != HleMaterialKind.AlphaTest)
                throw new Exception("Opaque vehicle pixels must retain keyed/STP rendering");
            checks += 2;
        }
        return checks;
    }
}
