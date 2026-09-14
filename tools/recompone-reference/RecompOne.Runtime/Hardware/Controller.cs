namespace RecompOne.Runtime.Hardware;

public static class Controller
{
    public const ushort Select = 1 << 0;
    public const ushort L3 = 1 << 1;
    public const ushort R3 = 1 << 2;
    public const ushort Start = 1 << 3;
    public const ushort Up = 1 << 4;
    public const ushort Right = 1 << 5;
    public const ushort Down = 1 << 6;
    public const ushort Left = 1 << 7;
    public const ushort L2 = 1 << 8;
    public const ushort R2 = 1 << 9;
    public const ushort L1 = 1 << 10;
    public const ushort R1 = 1 << 11;
    public const ushort Triangle = 1 << 12;
    public const ushort Circle = 1 << 13;
    public const ushort Cross = 1 << 14;
    public const ushort Square = 1 << 15;

    public static ushort State = 0xFFFF;
    public static byte   RightX = 0x80;
    public static byte   RightY = 0x80;
    public static byte   LeftX = 0x80;
    public static byte   LeftY = 0x80;

    public static ushort State2 = 0xFFFF;
    public static bool   Connected2;
    public static byte   RightX2 = 0x80;
    public static byte   RightY2 = 0x80;
    public static byte   LeftX2 = 0x80;
    public static byte   LeftY2 = 0x80;

    // Preserve the two retail pad images while exposing independent extension
    // ports to local multiplayer. No extra state occupies native PS1 globals.
    static readonly ushort[] _extraState = [0xFFFF, 0xFFFF];
    static readonly uint[] _extraAxes = [0x80808080, 0x80808080];
    public static readonly bool[] LocalConnected = new bool[4];

    public static ushort GetState(int player) => player switch
    {
        0 => State, 1 => State2, 2 or 3 => _extraState[player - 2],
        _ => throw new ArgumentOutOfRangeException(nameof(player)),
    };

    public static void SetState(int player, ushort state)
    {
        if (player == 0) State = state;
        else if (player == 1) State2 = state;
        else if (player is 2 or 3) _extraState[player - 2] = state;
        else throw new ArgumentOutOfRangeException(nameof(player));
    }

    public static uint GetAxes(int player) => player switch
    {
        0 => (uint)(RightX | RightY << 8 | LeftX << 16 | LeftY << 24),
        1 => (uint)(RightX2 | RightY2 << 8 | LeftX2 << 16 | LeftY2 << 24),
        2 or 3 => _extraAxes[player - 2],
        _ => throw new ArgumentOutOfRangeException(nameof(player)),
    };

    public static void SetAxes(int player, byte lx, byte ly, byte rx, byte ry)
    {
        if (player == 0) { LeftX = lx; LeftY = ly; RightX = rx; RightY = ry; }
        else if (player == 1) { LeftX2 = lx; LeftY2 = ly; RightX2 = rx; RightY2 = ry; }
        else if (player is 2 or 3) _extraAxes[player - 2] = (uint)(rx | ry << 8 | lx << 16 | ly << 24);
        else throw new ArgumentOutOfRangeException(nameof(player));
    }
}
