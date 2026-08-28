using ImGuiNET;
using RecompOne.Runtime.Config;

namespace RecompOne.Runtime.Host.Window;

internal sealed class DisplaySettingsSection : ISettingsSection
{
    static readonly string[] OutputResolutions = ["1280x720", "1920x1080", "2560x1440", "3840x2160"];
    static readonly string[] AntiAliasingModes = ["Off", "FXAA"];
    static readonly string[] LevelOfDetailModes = ["Stock", "Maximum"];
    static readonly string[] GraphicsPresets = ["Enhanced", "Custom"];
    static readonly int[] MsaaModes = [0, 2, 4, 8];
    static readonly int[] AnisotropicModes = [1, 2, 4, 8, 16];
    static readonly int[] InternalResolutionScales = [1, 2, 3, 4];

    public string Id => "display";
    public string Title => "Display";
    public int Order => 5;

    public void Draw()
    {
        string preset = ConfigManager.View.ResolveGraphicsPreset();
        ConfigManager.View.GraphicsPreset = preset;
        if (ImGui.BeginCombo("Graphics preset", preset))
        {
            foreach (string candidate in GraphicsPresets)
            {
                bool selected = candidate.Equals(preset, StringComparison.OrdinalIgnoreCase);
                if (ImGui.Selectable(candidate, selected) && candidate != "Custom")
                {
                    ConfigManager.View.ApplyGraphicsPreset(candidate);
                    HostWindow.ApplyGraphicsConfiguration();
                    ConfigManager.SaveView(PanelManager.Panels);
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("Enhanced GL is the shipping renderer; Custom changes its individual options.");

        string resolution = ConfigManager.View.OutputResolution;
        if (!OutputResolutions.Contains(resolution, StringComparer.OrdinalIgnoreCase))
            resolution = OutputResolutions[0];
        if (ImGui.BeginCombo("Output resolution", resolution))
        {
            foreach (string candidate in OutputResolutions)
            {
                bool selected = candidate.Equals(resolution, StringComparison.OrdinalIgnoreCase);
                if (ImGui.Selectable(candidate, selected))
                {
                    ConfigManager.View.OutputResolution = candidate;
                    HostWindow.SetOutputResolution(candidate);
                    ConfigManager.SaveView(PanelManager.Panels);
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("Fullscreen uses the desktop resolution.");

        string antiAliasing = ConfigManager.View.AntiAliasing;
        if (!AntiAliasingModes.Contains(antiAliasing, StringComparer.OrdinalIgnoreCase))
            antiAliasing = AntiAliasingModes[0];
        if (ImGui.BeginCombo("Anti-aliasing", antiAliasing))
        {
            foreach (string candidate in AntiAliasingModes)
            {
                bool selected = candidate.Equals(antiAliasing, StringComparison.OrdinalIgnoreCase);
                if (ImGui.Selectable(candidate, selected))
                {
                    ConfigManager.View.AntiAliasing = candidate;
                    SaveCustom();
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("FXAA is a final presentation pass.");

        int msaa = ConfigManager.View.MsaaSamples;
        string msaaLabel = msaa <= 1 ? "Off" : $"{msaa}x";
        if (ImGui.BeginCombo("Multisample anti-aliasing", msaaLabel))
        {
            foreach (int candidate in MsaaModes)
            {
                bool selected = candidate == msaa;
                string label = candidate == 0 ? "Off" : $"{candidate}x";
                if (ImGui.Selectable(label, selected))
                {
                    ConfigManager.View.MsaaSamples = candidate;
                    SaveCustom();
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("Applies to high-resolution 3D render targets; restart required.");

        bool fullscreen = ConfigManager.View.Fullscreen;
        if (ImGui.Checkbox("Fullscreen", ref fullscreen))
        {
            ConfigManager.View.Fullscreen = fullscreen;
            HostWindow.SetFullscreen(fullscreen);
            ConfigManager.SaveView(PanelManager.Panels);
        }

        ImGui.SeparatorText("3D rendering");
        bool highResolution3D = ConfigManager.View.HighResolution3D;
        if (ImGui.Checkbox("High-resolution 3D rendering", ref highResolution3D))
        {
            ConfigManager.View.HighResolution3D = highResolution3D;
            HostWindow.ApplyGraphicsConfiguration();
            SaveCustom();
        }
        int internalScale = ConfigManager.View.InternalResolutionScale;
        if (ImGui.BeginCombo("Internal 3D resolution", $"{internalScale}x"))
        {
            foreach (int candidate in InternalResolutionScales)
            {
                bool selected = candidate == internalScale;
                if (ImGui.Selectable($"{candidate}x", selected))
                {
                    ConfigManager.View.InternalResolutionScale = candidate;
                    ConfigManager.View.HighResolution3D = candidate > 1;
                    HostWindow.ApplyGraphicsConfiguration();
                    SaveCustom();
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("Enhanced defaults to 3x native; 4x is available for faster GPUs.");
        ImGui.TextDisabled(
            Hle.GpuHle.Active
                ? $"Active renderer: Enhanced GL at {Enhanced.GlVram.Scale}x"
                : "Active renderer: PS1 software at 1x");

        bool textureSmoothing = ConfigManager.View.TextureSmoothing;
        if (ImGui.Checkbox("Upscale/smooth in-game textures", ref textureSmoothing))
        {
            ConfigManager.View.TextureSmoothing = textureSmoothing;
            SaveCustom();
        }
        ImGui.TextDisabled("Reconstructs in-game PS1 textures without allocating assets over 512x512.");

        bool highResolutionTextures = ConfigManager.View.HighResolutionTextures;
        if (ImGui.Checkbox("Use high-resolution texture pack", ref highResolutionTextures))
        {
            ConfigManager.View.HighResolutionTextures = highResolutionTextures;
            SaveCustom();
        }
        ImGui.TextDisabled("Loads content-matched replacements from the loose textures folder.");

        bool mipmaps = ConfigManager.View.TextureMipmaps;
        if (ImGui.Checkbox("3D texture mipmaps", ref mipmaps))
        {
            ConfigManager.View.TextureMipmaps = mipmaps;
            SaveCustom();
        }

        int anisotropic = ConfigManager.View.AnisotropicFiltering;
        if (ImGui.BeginCombo("Anisotropic filtering", $"{anisotropic}x"))
        {
            foreach (int candidate in AnisotropicModes)
            {
                bool selected = candidate == anisotropic;
                if (ImGui.Selectable($"{candidate}x", selected))
                {
                    ConfigManager.View.AnisotropicFiltering = candidate;
                    SaveCustom();
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }

        bool perspectiveCorrectTextures = ConfigManager.View.PerspectiveCorrectTextures;
        if (ImGui.Checkbox("Fix PS1 texture projection", ref perspectiveCorrectTextures))
        {
            ConfigManager.View.PerspectiveCorrectTextures = perspectiveCorrectTextures;
            SaveCustom();
        }
        ImGui.TextDisabled("Uses GTE depth when available and projective quad homography otherwise.");

        bool geometryCorrection = ConfigManager.View.GeometryCorrection;
        if (ImGui.Checkbox("Sub-pixel geometry correction", ref geometryCorrection))
        {
            ConfigManager.View.GeometryCorrection = geometryCorrection;
            SaveCustom();
        }
        ImGui.TextDisabled("Preserves fractional GTE screen coordinates to remove polygon wobble.");

        bool preciseCulling = ConfigManager.View.PreciseCulling;
        if (ImGui.Checkbox("Precise polygon culling", ref preciseCulling))
        {
            ConfigManager.View.PreciseCulling = preciseCulling;
            SaveCustom();
        }
        ImGui.TextDisabled("Uses corrected coordinates for NCLIP to reduce cracks and geometry holes.");

        bool perspectiveColors = ConfigManager.View.PerspectiveCorrectColors;
        if (ImGui.Checkbox("Perspective-correct vertex colors", ref perspectiveColors))
        {
            ConfigManager.View.PerspectiveCorrectColors = perspectiveColors;
            SaveCustom();
        }

        bool enhancedDepth = ConfigManager.View.EnhancedDepthBuffer;
        if (ImGui.Checkbox("Enhanced depth buffer", ref enhancedDepth))
        {
            ConfigManager.View.EnhancedDepthBuffer = enhancedDepth;
            SaveCustom();
        }
        ImGui.TextDisabled("Uses coherent GTE depth and ordering-table fallback to reduce Z fighting.");

        bool trueColor = ConfigManager.View.TrueColor;
        if (ImGui.Checkbox("True-color output", ref trueColor))
        {
            ConfigManager.View.TrueColor = trueColor;
            SaveCustom();
        }
        ImGui.TextDisabled("Disables the PS1's final 5-bit color quantization in enhanced mode.");

        string levelOfDetail = ConfigManager.View.LevelOfDetail;
        if (!LevelOfDetailModes.Contains(levelOfDetail, StringComparer.OrdinalIgnoreCase))
            levelOfDetail = LevelOfDetailModes[0];
        string levelOfDetailLabel = levelOfDetail.Equals("Maximum", StringComparison.OrdinalIgnoreCase)
            ? "Maximum (terrain + models)"
            : "Stock (distance-based)";
        if (ImGui.BeginCombo("Level of detail", levelOfDetailLabel))
        {
            foreach (string candidate in LevelOfDetailModes)
            {
                bool selected = candidate.Equals(levelOfDetail, StringComparison.OrdinalIgnoreCase);
                string candidateLabel = candidate == "Maximum"
                    ? "Maximum (terrain + models)"
                    : "Stock (distance-based)";
                if (ImGui.Selectable(candidateLabel, selected))
                {
                    ConfigManager.View.LevelOfDetail = candidate;
                    SaveCustom();
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("Maximum keeps the highest-detail geometry at every distance.");

        bool widescreen = ConfigManager.View.Widescreen;
        if (ImGui.Checkbox("Proper widescreen 3D", ref widescreen))
        {
            ConfigManager.View.Widescreen = widescreen;
            SaveCustom();
        }
        bool hudAnchoring = ConfigManager.View.HudAnchoring;
        if (ImGui.Checkbox("Anchor HUD to widescreen edges", ref hudAnchoring))
        {
            ConfigManager.View.HudAnchoring = hudAnchoring;
            SaveCustom();
        }

        bool shadows = ConfigManager.View.EnhancedShadows;
        if (ImGui.Checkbox("Soft projected shadows", ref shadows))
        {
            ConfigManager.View.EnhancedShadows = shadows;
            SaveCustom();
        }
        bool particles = ConfigManager.View.EnhancedParticles;
        if (ImGui.Checkbox("High-resolution filtered particles", ref particles))
        {
            ConfigManager.View.EnhancedParticles = particles;
            SaveCustom();
        }

        bool fonts = ConfigManager.View.VectorFonts;
        if (ImGui.Checkbox("Vector font contours", ref fonts))
        {
            ConfigManager.View.VectorFonts = fonts;
            SaveCustom();
        }
        bool icons = ConfigManager.View.VectorIcons;
        if (ImGui.Checkbox("Vector HUD, logo, and icon contours", ref icons))
        {
            ConfigManager.View.VectorIcons = icons;
            SaveCustom();
        }

        bool drawDistance = ConfigManager.View.ExtendedDrawDistance;
        if (ImGui.Checkbox("Extended draw distance", ref drawDistance))
        {
            ConfigManager.View.ExtendedDrawDistance = drawDistance;
            SaveCustom();
        }
        bool fog = ConfigManager.View.EnhancedFog;
        if (ImGui.Checkbox("Improved distance fog/haze", ref fog))
        {
            ConfigManager.View.EnhancedFog = fog;
            SaveCustom();
        }

        bool ps1Dithering = ConfigManager.View.Ps1Dithering;
        if (ImGui.Checkbox("PS1 color dithering (fidelity)", ref ps1Dithering))
        {
            ConfigManager.View.Ps1Dithering = ps1Dithering;
            SaveCustom();
        }
        ImGui.TextDisabled("Off by default; changes take effect immediately.");
    }

    static void SaveCustom()
    {
        ConfigManager.View.MarkGraphicsCustom();
        ConfigManager.SaveView(PanelManager.Panels);
    }
}
