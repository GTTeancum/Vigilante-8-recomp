using System.Text;
using RecompOne.Runtime.Hardware;

namespace RecompOne.Runtime.Config;

public static class InputPromptLabels
{
    public static string PadButton(int value) => value switch
    {
        0 => "A", 1 => "B", 2 => "X", 3 => "Y", 4 => "View", 6 => "Menu",
        7 => "LS", 8 => "RS", 9 => "LB", 10 => "RB", 11 => "D-Pad Up",
        12 => "D-Pad Down", 13 => "D-Pad Left", 14 => "D-Pad Right",
        100 => "LT", 101 => "RT", 102 => "L-Stick Left", 103 => "L-Stick Right",
        104 => "L-Stick Up", 105 => "L-Stick Down", 106 => "R-Stick Left",
        107 => "R-Stick Right", 108 => "R-Stick Up", 109 => "R-Stick Down",
        _ => $"Button {value}",
    };

    public static bool IsReadyPrompt(string text) =>
        text.StartsWith("Press ", StringComparison.OrdinalIgnoreCase) &&
        (text.Contains("to start", StringComparison.OrdinalIgnoreCase) ||
         text.Contains("to play", StringComparison.OrdinalIgnoreCase));

    public static string Button(GameConfig game, int player, ushort action,
        bool gamepad, bool gameplay, bool menu)
    {
        if (!gamepad)
        {
            var keys = InputBindingResolver.ResolveKeys(player == 0 ? game.Keys : game.Keys2,
                player, gameplay, menu);
            string key = action switch {
                Controller.Cross => keys.Cross, Controller.Circle => keys.Circle,
                Controller.Square => keys.Square, Controller.Triangle => keys.Triangle,
                Controller.Start => keys.Start, _ => "",
            };
            return key.Length == 0 ? "Unbound" : key;
        }
        var pad = InputBindingResolver.ResolvePad(InputProfiles.ForPlayer(game, player),
            player == 0 ? game.Pad : game.Pad2, gameplay, menu);
        var binding = action switch {
            Controller.Cross => pad.Cross, Controller.Circle => pad.Circle,
            Controller.Square => pad.Square, Controller.Triangle => pad.Triangle,
            Controller.Start => pad.Start, _ => [],
        };
        return binding.Length == 0 ? "Unbound" : PadButton(binding[0]);
    }

    public static string Format(string text, GameConfig game, int player,
        bool gamepad, bool gameplay, bool menu)
    {
        bool ready = IsReadyPrompt(text);
        string Label(ushort action) => Button(game, player, action, gamepad,
            gameplay || ready, menu && !ready);
        if (text == "PRESS START") return "PRESS " + Label(Controller.Start);
        if (text == "Press X to configure") return "Press " + Label(Controller.Cross) + " to configure";
        var result = new StringBuilder(text.Length);
        for (int i = 0; i < text.Length; i++)
        {
            // Native text embeds RGB as 01 rr gg bb. Color bytes can have
            // the same values as button glyphs, and must remain untouched.
            if (text[i] == '\x01' && i + 3 < text.Length)
            {
                result.Append(text, i, 4);
                i += 3;
                continue;
            }
            ushort action = text[i] switch {
                '\x80' => Controller.Circle, '\x81' => Controller.Square,
                '\x82' => Controller.Cross, '\x83' => Controller.Triangle, _ => 0,
            };
            if (action == 0) result.Append(text[i]);
            else
            {
                result.Append(Label(action));
                if (i + 1 < text.Length && char.IsLetter(text[i + 1])) result.Append(' ');
            }
        }
        return ready ? result.ToString().Replace("to start...", "to play") : result.ToString();
    }
}
