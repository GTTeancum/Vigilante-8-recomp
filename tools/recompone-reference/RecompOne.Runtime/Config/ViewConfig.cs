using System.Globalization;

namespace RecompOne.Runtime.Config;

public class PanelState
{
    public bool Open { get; set; }
}

public class ViewConfig
{
    static bool IsV82 =>
        Runtime.GameTitle.Contains("2nd Offense", StringComparison.Ordinal);

    public Dictionary<string, string> Values { get; set; } = new(StringComparer.OrdinalIgnoreCase);
    public Dictionary<string, PanelState> Panels { get; set; } = [];

    public bool GetBool(string key, bool fallback = false)
        => Values.TryGetValue(key, out var v) && bool.TryParse(v, out var b) ? b : fallback;

    public void SetBool(string key, bool value) => Values[key] = value.ToString();

    public int GetInt(string key, int fallback = 0)
        => Values.TryGetValue(key, out var v) && int.TryParse(v, NumberStyles.Integer, CultureInfo.InvariantCulture, out var i) ? i : fallback;

    public void SetInt(string key, int value) => Values[key] = value.ToString(CultureInfo.InvariantCulture);

    public float GetFloat(string key, float fallback = 0f)
        => Values.TryGetValue(key, out var v) && float.TryParse(v, NumberStyles.Float, CultureInfo.InvariantCulture, out var f) ? f : fallback;

    public void SetFloat(string key, float value) => Values[key] = value.ToString(CultureInfo.InvariantCulture);

    public string GetString(string key, string fallback = "")
        => Values.TryGetValue(key, out var v) ? v : fallback;

    public void SetString(string key, string value) => Values[key] = value;

    public bool HideTopBar
    {
        get => GetBool("HideTopBar");
        set => SetBool("HideTopBar", value);
    }

    public bool Fullscreen
    {
        get => GetBool("Fullscreen");
        set => SetBool("Fullscreen", value);
    }

    public bool HighResolution3D
    {
        get => GetBool("HighResolution3D", true);
        set => SetBool("HighResolution3D", value);
    }

    public bool Ps1Dithering
    {
        get => GetBool("Ps1Dithering", false);
        set => SetBool("Ps1Dithering", value);
    }

    public bool TextureSmoothing
    {
        get => GetBool("TextureSmoothing", true);
        set => SetBool("TextureSmoothing", value);
    }

    public bool PerspectiveCorrectTextures
    {
        get => GetBool("PerspectiveCorrectTextures", true);
        set => SetBool("PerspectiveCorrectTextures", value);
    }

    public string OutputResolution
    {
        get => GetString("OutputResolution", "1920x1080");
        set => SetString("OutputResolution", value);
    }

    public string AntiAliasing
    {
        get => GetString("AntiAliasing", "FXAA");
        set => SetString("AntiAliasing", value);
    }

    public string LevelOfDetail
    {
        // V8's "Maximum" terrain mode deliberately traverses every terrain
        // leaf and is an opt-in diagnostic/view-distance mode.  It is not a
        // suitable default for the original game's much denser terrain tree.
        get => GetString("LevelOfDetail", IsV82 ? "Maximum" : "Stock");
        set => SetString("LevelOfDetail", value);
    }

    public string GraphicsPreset
    {
        get => GetString("GraphicsPreset", "Enhanced");
        set => SetString("GraphicsPreset", value);
    }

    public int MsaaSamples
    {
        get => GetInt("MsaaSamples", 2);
        set => SetInt("MsaaSamples", value);
    }

    public int AnisotropicFiltering
    {
        get => GetInt("AnisotropicFiltering", 4);
        set => SetInt("AnisotropicFiltering", value);
    }

    public bool TextureMipmaps
    {
        get => GetBool("TextureMipmaps", true);
        set => SetBool("TextureMipmaps", value);
    }

    public bool Widescreen
    {
        get => GetBool("Widescreen", true);
        set => SetBool("Widescreen", value);
    }

    public bool HudAnchoring
    {
        get => GetBool("HudAnchoring", true);
        set => SetBool("HudAnchoring", value);
    }

    public bool EnhancedShadows
    {
        get => GetBool("EnhancedShadows", true);
        set => SetBool("EnhancedShadows", value);
    }

    public bool EnhancedParticles
    {
        get => GetBool("EnhancedParticles", true);
        set => SetBool("EnhancedParticles", value);
    }

    public bool VectorFonts
    {
        get => GetBool("VectorFonts", true);
        set => SetBool("VectorFonts", value);
    }

    public bool VectorIcons
    {
        get => GetBool("VectorIcons", true);
        set => SetBool("VectorIcons", value);
    }

    public bool ExtendedDrawDistance
    {
        get => GetBool("ExtendedDrawDistance", true);
        set => SetBool("ExtendedDrawDistance", value);
    }

    public bool EnhancedFog
    {
        get => GetBool("EnhancedFog", true);
        set => SetBool("EnhancedFog", value);
    }

    public void MarkGraphicsCustom() => GraphicsPreset = "Custom";

    public void ApplyGraphicsPreset(string preset)
    {
        bool original = preset.Equals("Original", StringComparison.OrdinalIgnoreCase);
        GraphicsPreset = original ? "Original" : "Enhanced";
        HighResolution3D = !original;
        Ps1Dithering = original;
        TextureSmoothing = !original;
        PerspectiveCorrectTextures = !original;
        AntiAliasing = original ? "Off" : "FXAA";
        // 2x MSAA at the enhanced 2x internal resolution resolves geometry
        // cleanly without the fill-rate spike of the former 4x default.
        MsaaSamples = original ? 0 : 2;
        AnisotropicFiltering = original ? 1 : 4;
        TextureMipmaps = !original;
        Widescreen = !original;
        HudAnchoring = !original;
        EnhancedShadows = !original;
        EnhancedParticles = !original;
        VectorFonts = !original;
        VectorIcons = !original;
        ExtendedDrawDistance = !original;
        EnhancedFog = !original;
        LevelOfDetail = original || !IsV82 ? "Stock" : "Maximum";
    }
}
