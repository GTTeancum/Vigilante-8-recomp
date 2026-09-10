using System.Reflection;
using RecompOne.Runtime.Enhanced;
using RecompOne.Runtime.Hle;

static class SceneDepthChecks
{
    public static int Run()
    {
        var method = typeof(EnhancedGlBackend).GetMethod("DepthOf", BindingFlags.Static | BindingFlags.NonPublic)!;
        float Depth(HleVertex v, bool coherent = true) =>
            (float)method.Invoke(null, new object[] { v, 197, coherent, false })!;
        var road = new HleVertex { HasViewSpace = true, ViewZ = 1376.1056f };
        if (Math.Abs(Depth(road) * 65535f - road.ViewZ) > .001f)
            throw new Exception("Exact road depth must not use the farther OT bucket");
        if (Depth(road) >= 1391.9288f / 65535f)
            throw new Exception("Utah road must remain in front of the sampled sand");
        float fallback = 197f / 8192f;
        if (Depth(road, false) != fallback) throw new Exception("Uncorrelated depth fallback");
        road.ReconstructedViewSpace = true;
        if (Depth(road) != fallback) throw new Exception("Reconstructed depth fallback");
        road.ReconstructedViewSpace = false;
        road.HasViewSpace = false;
        if (Depth(road) != fallback) throw new Exception("Absent source depth fallback");
        road.HasViewSpace = true;
        road.ViewZ = float.NaN;
        if (Depth(road) != fallback) throw new Exception("Invalid source depth fallback");
        return 6;
    }
}
