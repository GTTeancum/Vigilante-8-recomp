using ImGuiNET;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Host.Window;

internal static class GuestArenaMenu
{
    public static void Draw()
    {
        string? selected = V8ArenaRegistry.SelectedStableId;
        if (ImGui.MenuItem(
                "Built-in arena selection", null, selected == null))
            V8ArenaRegistry.Select(null);
        foreach (ArenaRosterItem item in V8ArenaRegistry.Roster())
        {
            string label =
                $"{item.DisplayName}  [{item.StableId}]##{item.StableId}";
            if (ImGui.MenuItem(label, null, selected == item.StableId))
                V8ArenaRegistry.Select(item.StableId);
        }
        if (selected != null)
        {
            ImGui.Separator();
            ImGui.TextDisabled(
                "The next location launch uses the selected independent arena."
            );
        }
    }
}
