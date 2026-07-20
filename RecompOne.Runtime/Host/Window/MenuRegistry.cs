using ImGuiNET;

namespace RecompOne.Runtime.Host.Window;

public static class MenuRegistry
{
    static readonly List<(string Label, Action Draw)> _menus = [];
    static readonly List<Action> _windows = [];

    public static void Register(string label, Action drawItems)
    {
        if (string.IsNullOrEmpty(label) || drawItems == null) return;
        _menus.Add((label, drawItems));
    }

    public static void RegisterWindow(Action draw)
    {
        if (draw == null) return;
        _windows.Add(draw);
    }

    internal static void DrawMenus()
    {
        foreach (var (label, draw) in _menus)
        {
            if (!ImGui.BeginMenu(label)) continue;
            draw();
            ImGui.EndMenu();
        }
    }

    internal static void DrawWindows()
    {
        foreach (var draw in _windows)
            draw();
    }
}
