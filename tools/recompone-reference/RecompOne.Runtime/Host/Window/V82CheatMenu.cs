using ImGuiNET;
using RecompOne.Runtime.Config;
using RecompOne.Runtime.Sdk;

namespace RecompOne.Runtime.Host.Window;

internal static class V82CheatMenu
{
    readonly record struct Cheat(int Bit, string Code, string Description);

    static readonly Cheat[] Cheats =
    [
        new(0,  "GO_MONSTER",  "Monster wheels"),
        new(1,  "NO_GRAVITY",  "Reduced gravity"),
        new(2,  "HOME_ALONE",  "No enemies in Arcade"),
        new(3,  "ELBICNIVNI",  "Invincibility"),
        new(4,  "LONG_MOVIE",  "All ending movies"),
        new(5,  "MIXED_CARS",  "Allow duplicate vehicles"),
        new(6,  "DRIVE_ONLY",  "Disable transformation pickups"),
        new(7,  "LLA_DORTOH",  "Maximum vehicle stats"),
        new(8,  "LLA_PUENUT",  "50 vehicle stat points"),
        new(9,  "LLA_KCOLNU",  "Unlock all vehicles"),
        new(10, "BLAST_FIRE",  "Deadlier missiles"),
        new(11, "RAPID_FIRE",  "Remove firing delay"),
        new(12, "UNDER_FIRE",  "More aggressive enemies"),
        new(13, "GO_SLOW_MO",  "Slow motion"),
        new(14, "GO_MAX_REZ",  "Legacy maximum-resolution flag"),
        new(15, "GO_RAMMING",  "Heavier player vehicles"),
        new(16, "MORE_SPEED",  "Faster vehicles"),
        new(17, "QUICK_PLAY",  "Quick-play Arcade setup"),
        new(18, "JACK_IT_UP",  "Higher suspension"),
        new(19, "HI_CEILING",  "Higher hover capability"),
        new(20, "OLD_LEVELS",  "Original Vigilante 8 arenas"),
    ];

    public static void Draw()
    {
        ImGui.TextDisabled("Vigilante 8: 2nd Offense");
        foreach (var cheat in Cheats)
        {
            uint mask = 1u << cheat.Bit;
            bool enabled = (ConfigManager.Game.V82CheatFlags & mask) != 0;
            string label = $"{cheat.Description}  [{cheat.Code}]##v82cheat{cheat.Bit}";
            if (!ImGui.MenuItem(label, null, enabled)) continue;

            uint flags = enabled
                ? ConfigManager.Game.V82CheatFlags & ~mask
                : ConfigManager.Game.V82CheatFlags | mask;
            V82Compat.SetConfiguredCheatFlags(flags);
        }

        ImGui.Separator();
        if (ImGui.MenuItem("Disable all cheats##v82cheatreset"))
            V82Compat.SetConfiguredCheatFlags(0u);
        if (ImGui.IsItemHovered())
            ImGui.SetTooltip(
                "Maximum/50-point stat changes match the retail password behavior " +
                "and cannot be undone for the current in-memory profile.");
    }
}
