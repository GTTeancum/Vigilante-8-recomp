using ImGuiNET;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Host.Window;

/// <summary>
/// Desktop adapter for the engine-facing append-only guest roster. Console
/// front ends can consume the same registry API without depending on ImGui.
/// </summary>
internal static class GuestVehicleMenu
{
    public static void Draw()
    {
        bool any = false;
        if (V82VehicleRegistry.HasPackage)
        {
            any = true;
            DrawV82();
        }
        if (V8VehicleRegistry.HasPackage)
        {
            if (any) ImGui.Separator();
            any = true;
            DrawV8();
        }
        if (!any)
            ImGui.TextDisabled("No independent vehicle package loaded");
    }

    static void DrawV82()
    {
        ImGui.TextDisabled("Vigilante 8: 2nd Offense");
        int selected = V82VehicleRegistry.SelectedType;
        if (ImGui.MenuItem("Built-in roster selection##v82", null, selected < 0))
            V82VehicleRegistry.SelectType(-1);
        foreach (VehicleRosterItem item in V82VehicleRegistry.Roster())
        {
            string label =
                $"{item.DisplayName}  [{item.StableId}]##v82_{item.Type}";
            if (ImGui.MenuItem(label, null, selected == item.Type))
                V82VehicleRegistry.SelectType(item.Type);
        }
    }

    static void DrawV8()
    {
        ImGui.TextDisabled("Vigilante 8");
        int selected = V8VehicleRegistry.SelectedType;
        if (ImGui.MenuItem("Built-in roster selection##v8", null, selected < 0))
            V8VehicleRegistry.SelectType(-1);
        foreach (VehicleRosterItem item in V8VehicleRegistry.Roster())
        {
            string label =
                $"{item.DisplayName}  [{item.StableId}]##v8_{item.Type}";
            if (ImGui.MenuItem(label, null, selected == item.Type))
                V8VehicleRegistry.SelectType(item.Type);
        }
    }
}
