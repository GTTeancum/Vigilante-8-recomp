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
        // Source-matched Canyonlands approach: a support at Z3890.9504 was
        // painting over foreground sand at Z1875.962. Scenery must use the
        // same coherent depth domain, while vehicle/effect ordering stays out.
        var support = new HleVertex { HasViewSpace = true, ViewZ = 3890.9504f };
        var sand = new HleVertex { HasViewSpace = true, ViewZ = 1875.962f };
        if (Depth(support) <= Depth(sand)) throw new Exception("Buried support must be behind foreground terrain");
        var scenery = new PrimFlags { WorldObject = true, Material = HleMaterialKind.AlphaTest };
        if (!EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Textured scenery must compare coherent depth");
        scenery.Material = HleMaterialKind.Opaque;
        if (!EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Untextured scenery must compare coherent depth");
        if (EnhancedGlBackend.UsesSceneryDepthCompare(scenery, false)) throw new Exception("Uncorrelated fallback must preserve native ordering");
        scenery.Vehicle = true;
        if (EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Vehicle layers must preserve native ordering");
        scenery.Vehicle = false;
        scenery.SemiTrans = true;
        if (EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Translucent scenery must preserve its pass contract");
        scenery.SemiTrans = false;
        scenery.WorldObject = false;
        if (EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Unowned packets must not acquire scenery semantics");
        scenery.WorldObject = true;
        scenery.Material = HleMaterialKind.WaterSurface;
        if (EnhancedGlBackend.UsesSceneryDepthCompare(scenery, true)) throw new Exception("Water must retain its recovered depth contract");
        return 14;
    }
}
