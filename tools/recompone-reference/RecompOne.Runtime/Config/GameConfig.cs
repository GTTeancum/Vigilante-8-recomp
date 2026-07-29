
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

public class GameConfig
{
    // V8: 2nd Offense native cheat bits. DRIVE_ONLY (bit 6) and the
    // PlayStation original-arena flag (bit 20) are the PC defaults.
    public uint V82CheatFlags { get; set; } = (1u << 6) | (1u << 20);
    // Optional test package selected from the wrapper. An empty value keeps
    // loose-files mods discovery as the source of truth.
    public string V82VehiclePackagePath { get; set; } = "";
    public int InputBindingsVersion { get; set; }
    public string CdPath { get; set; } = "";
    public float MasterVolume { get; set; } = 1.0f;
    public bool Muted { get; set; } = false;
    public KeyBindings Keys { get; set; } = new();
    public KeyBindings Keys2 { get; set; } = KeyBindings.Empty();
    public GamepadBindings Pad { get; set; } = new();
    public GamepadBindings Pad2 { get; set; } = new();
    public List<string> ActiveMods { get; set; } = [];
}
