using ImGuiNET;
using RecompOne.Runtime.Config;

namespace RecompOne.Runtime.Host.Window;

internal sealed class DisplaySettingsSection : ISettingsSection
{
    static readonly string[] OutputResolutions = ["1280x720", "1920x1080", "2560x1440", "3840x2160"];
    static readonly string[] AntiAliasingModes = ["Off", "FXAA"];

    public string Id => "display";
    public string Title => "Display";
    public int Order => 5;

    public void Draw()
    {
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
                    ConfigManager.SaveView(PanelManager.Panels);
                }
                if (selected) ImGui.SetItemDefaultFocus();
            }
            ImGui.EndCombo();
        }
        ImGui.TextDisabled("FXAA affects presentation only; PS1 rendering stays native.");

        bool fullscreen = ConfigManager.View.Fullscreen;
        if (ImGui.Checkbox("Fullscreen", ref fullscreen))
        {
            ConfigManager.View.Fullscreen = fullscreen;
            HostWindow.SetFullscreen(fullscreen);
            ConfigManager.SaveView(PanelManager.Panels);
        }

        ImGui.SeparatorText("3D rendering");
        bool highResolution3D = ConfigManager.View.HighResolution3D;
        if (ImGui.Checkbox("High-resolution 3D (4x)", ref highResolution3D))
        {
            ConfigManager.View.HighResolution3D = highResolution3D;
            ConfigManager.SaveView(PanelManager.Panels);
        }
        ImGui.TextDisabled("Rasterizes PS1 polygons at 4x internal resolution.");
        if (ConfigManager.View.HighResolution3D != Hle.GpuHle.Active)
            ImGui.TextDisabled("A restart is required.");

        bool ps1Dithering = ConfigManager.View.Ps1Dithering;
        if (ImGui.Checkbox("PS1 color dithering (fidelity)", ref ps1Dithering))
        {
            ConfigManager.View.Ps1Dithering = ps1Dithering;
            ConfigManager.SaveView(PanelManager.Panels);
        }
        ImGui.TextDisabled("Off by default; changes take effect immediately.");
    }
}
