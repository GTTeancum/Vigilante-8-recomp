
namespace RecompOne.Runtime.Config;

public class KeyBindings
{
    public string Cross { get; set; } = "Z";
    public string Circle { get; set; } = "X";
    public string Square { get; set; } = "A";
    public string Triangle { get; set; } = "S";
    public string L1 { get; set; } = "Q";
    public string R1 { get; set; } = "W";
    public string L2 { get; set; } = "E";
    public string R2 { get; set; } = "R";
    public string L3 { get; set; } = "F";
    public string R3 { get; set; } = "G";
    public string Start { get; set; } = "Enter";
    public string Select { get; set; } = "ShiftRight";
    public string Up { get; set; } = "Up";
    public string Down { get; set; } = "Down";
    public string Left { get; set; } = "Left";
    public string Right { get; set; } = "Right";

    public static KeyBindings Empty() => new()
    {
        Cross = "", Circle = "", Square = "", Triangle = "",
        L1 = "", R1 = "", L2 = "", R2 = "",
        L3 = "", R3 = "", Start = "", Select = "",
        Up = "", Down = "", Left = "", Right = ""
    };
}

public class GamepadBindings
{
    public int[] Cross { get; set; } = [0];
    public int[] Circle { get; set; } = [1];
    public int[] Square { get; set; } = [2];
    public int[] Triangle { get; set; } = [3];
    public int[] L1 { get; set; } = [9];
    public int[] R1 { get; set; } = [10];
    public int[] L2 { get; set; } = [100];
    public int[] R2 { get; set; } = [101];
    public int[] L3 { get; set; } = [7];
    public int[] R3 { get; set; } = [8];
    public int[] Start { get; set; } = [6];
    public int[] Select { get; set; } = [4];
    public int[] Up { get; set; } = [11, 104];
    public int[] Down { get; set; } = [12, 105];
    public int[] Left { get; set; } = [13, 102];
    public int[] Right { get; set; } = [14, 103];

    public static GamepadBindings Empty() => new()
    {
        Cross = [], Circle = [], Square = [], Triangle = [],
        L1 = [], R1 = [], L2 = [], R2 = [],
        L3 = [], R3 = [], Start = [], Select = [],
        Up = [], Down = [], Left = [], Right = []
    };

    public bool HasAnyBinding() =>
        Cross.Length > 0 || Circle.Length > 0 || Square.Length > 0 || Triangle.Length > 0 ||
        L1.Length > 0 || R1.Length > 0 || L2.Length > 0 || R2.Length > 0 ||
        L3.Length > 0 || R3.Length > 0 || Start.Length > 0 || Select.Length > 0 ||
        Up.Length > 0 || Down.Length > 0 || Left.Length > 0 || Right.Length > 0;
}

public static class InputProfiles
{
    public const string Modern = "Modern";
    public const string TriggerDrive = "Trigger Drive";
    public const string Classic = "Classic";
    public const string Southpaw = "Southpaw";
    public const string Custom = "Custom";

    public static readonly string[] Names =
        [Modern, TriggerDrive, Classic, Southpaw];

    public static KeyBindings CreateKeys(string profile, bool playerTwo = false)
    {
        if (playerTwo) return KeyBindings.Empty();
        return new KeyBindings();
    }

    public static GamepadBindings CreatePad(string profile)
    {
        var pad = new GamepadBindings();
        switch (profile)
        {
            case TriggerDrive:
                // Xbox-style driving uses the original native commands:
                // RT is Gas (Cross), LT is Brake/Reverse (Down), A fires the
                // selected attachment (L2), and X fires the machine gun (R2).
                // B/Y retain Hand Brake/Select Target, while LB/RB use the
                // retail previous/next attached-weapon commands.
                pad.Cross = [101];
                pad.Square = [];
                pad.L2 = [0];
                pad.R2 = [2];
                pad.Down = [100, 12, 105];
                break;
            case Modern:
                // V8: 2nd Offense already implements the desired driving
                // behavior on the native Down command: brake until stopped,
                // then reverse. Physical Square joins D-pad/stick Down on
                // that command instead of creating a second physics path.
                pad.Square = [];
                pad.Down = [2, 12, 105];
                break;
            case Southpaw:
                pad.Square = [];
                pad.Down = [2, 12, 109];
                pad.Up = [11, 108];
                pad.Left = [13, 106];
                pad.Right = [14, 107];
                break;
            case Classic:
            default:
                break;
        }
        return pad;
    }

    public static void Apply(GameConfig game, string profile)
    {
        if (!Names.Contains(profile, StringComparer.Ordinal))
            throw new ArgumentOutOfRangeException(nameof(profile), profile, "Unknown input profile");

        game.Keys = CreateKeys(profile);
        game.Keys2 = CreateKeys(profile, playerTwo: true);
        game.Pad = CreatePad(profile);
        game.Pad2 = CreatePad(profile);
        game.InputProfile = profile;
    }

    public static bool IsClassicDefaults(GamepadBindings pad) =>
        Same(pad.Cross, [0]) && Same(pad.Circle, [1]) &&
        Same(pad.Square, [2]) && Same(pad.Triangle, [3]) &&
        Same(pad.L1, [9]) && Same(pad.R1, [10]) &&
        Same(pad.L2, [100]) && Same(pad.R2, [101]) &&
        Same(pad.L3, [7]) && Same(pad.R3, [8]) &&
        Same(pad.Start, [6]) && Same(pad.Select, [4]) &&
        Same(pad.Up, [11, 104]) && Same(pad.Down, [12, 105]) &&
        Same(pad.Left, [13, 102]) && Same(pad.Right, [14, 103]);

    public static bool ValidateModernBrakeReverse(GamepadBindings pad) =>
        pad.Square.Length == 0 &&
        pad.Down.Contains(2) &&
        pad.Down.Contains(105);

    public static bool ValidateTriggerDrive(GamepadBindings pad) =>
        Same(pad.Cross, [101]) &&
        Same(pad.Circle, [1]) &&
        pad.Square.Length == 0 &&
        Same(pad.Triangle, [3]) &&
        Same(pad.L1, [9]) && Same(pad.R1, [10]) &&
        Same(pad.L2, [0]) && Same(pad.R2, [2]) &&
        Same(pad.Down, [100, 12, 105]) &&
        Same(pad.Left, [13, 102]) && Same(pad.Right, [14, 103]);

    static bool Same(int[] actual, int[] expected) =>
        actual.AsSpan().SequenceEqual(expected);
}

public static class InputBindingResolver
{
    static readonly GamepadBindings MenuPad = new();

    // Trigger Drive's gameplay layout deliberately moves Cross and the two
    // weapon triggers. The retail shell and gameplay overlays still need the
    // familiar face-button navigation, so those contexts resolve through the
    // stock DualShock layout without modifying the saved preset.
    public static GamepadBindings ResolvePad(
        string profile,
        GamepadBindings configured,
        bool gameplayActive,
        bool nativeGameplayMenuActive)
    {
        if (!string.Equals(
                profile, InputProfiles.TriggerDrive,
                StringComparison.Ordinal))
            return configured;
        return gameplayActive && !nativeGameplayMenuActive
            ? configured
            : MenuPad;
    }
}

public class GameConfig
{
    // V8: 2nd Offense native cheat bits. DRIVE_ONLY (bit 6) and the
    // PlayStation original-arena flag (bit 20) are the PC defaults.
    public uint V82CheatFlags { get; set; } = (1u << 6) | (1u << 20);
    // Optional test package selected from the wrapper. An empty value keeps
    // loose-files mods discovery as the source of truth.
    public string V82VehiclePackagePath { get; set; } = "";
    public int InputBindingsVersion { get; set; }
    public string InputProfile { get; set; } = InputProfiles.Modern;
    public string CdPath { get; set; } = "";
    public float MasterVolume { get; set; } = 1.0f;
    public bool Muted { get; set; } = false;
    public KeyBindings Keys { get; set; } = new();
    public KeyBindings Keys2 { get; set; } = KeyBindings.Empty();
    public GamepadBindings Pad { get; set; } = new();
    public GamepadBindings Pad2 { get; set; } = new();
    public List<string> ActiveMods { get; set; } = [];
}
