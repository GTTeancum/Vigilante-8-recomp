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

        ImGui.SeparatorText("Experimental renderer");
        bool native = ConfigManager.View.NativeResolution;
        if (ImGui.Checkbox("Native resolution", ref native))
        {
            ConfigManager.View.NativeResolution = native;
            Hle.GpuHle.NativeResolution = native;
            ConfigManager.SaveView(PanelManager.Panels);
        }
        if (ConfigManager.View.NativeResolution != (Hle.GlVram.Scale == 1))
            ImGui.TextDisabled("A restart is required.");
    }
}
