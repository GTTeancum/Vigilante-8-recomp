using RecompOne.Runtime.Hardware;

namespace RecompOne.Runtime.Config;

// Physical device slots remain stable. Only the completed input images are
// reordered into the player slots selected by the join screen.
public static class LocalInputSession
{
    public const int KeyboardDevice = 4;
    public const int DeviceCount = 5;
    public static ushort KeyboardState { get; internal set; } = 0xFFFF;
    public static bool KeyboardConnected { get; internal set; }
    static int[] _devices = [];
    static int[] _unavailableTypes = [];
    public static bool IsTypeAvailable(int type) => !_unavailableTypes.Contains(type);
    public static int SelectorPlayer { get; private set; } = -1;
    public static int PlayerCount => _devices.Length;
    public static int DeviceForPlayer(int player) => _devices.Length == 0 ? player
        : SelectorPlayer >= 0 ? player == 0 ? _devices[SelectorPlayer] : -1
        : (uint)player < (uint)_devices.Length ? _devices[player] : -1;

    public static void Begin(IEnumerable<int> devices)
    {
        int[] joined = devices.ToArray();
        if (joined.Length is < 2 or > 4 || joined.Any(d => d is < 0 or > KeyboardDevice) ||
            joined.Distinct().Count() != joined.Length)
            throw new ArgumentException("Two to four distinct device slots required", nameof(devices));
        _devices = joined;
        SelectorPlayer = -1;
    }

    public static void Reset() { _devices = []; SelectorPlayer = -1; _unavailableTypes = []; }

    sealed class FocusScope(int previous, int[] unavailable) : IDisposable
    {
        public void Dispose() { SelectorPlayer = previous; _unavailableTypes = unavailable; }
    }

    public static IDisposable FocusSelector(int player, IEnumerable<int>? unavailable = null)
    {
        if ((uint)player >= (uint)_devices.Length) throw new ArgumentOutOfRangeException(nameof(player));
        int previous = SelectorPlayer;
        int[] priorTypes = _unavailableTypes;
        _unavailableTypes = unavailable?.ToArray() ?? [];
        SelectorPlayer = player;
        return new FocusScope(previous, priorTypes);
    }

    public static void RoutePolledDevices()
    {
        if (_devices.Length == 0) return;
        Span<ushort> states = stackalloc ushort[4];
        Span<uint> axes = stackalloc uint[4];
        Span<bool> connected = stackalloc bool[4];
        for (int device = 0; device < 4; device++)
        {
            states[device] = Controller.GetState(device);
            axes[device] = Controller.GetAxes(device);
            connected[device] = Controller.LocalConnected[device];
        }
        for (int player = 0; player < 4; player++)
        {
            int device = DeviceForPlayer(player);
            bool keyboard = device == KeyboardDevice;
            bool present = keyboard ? KeyboardConnected : device >= 0 && connected[device];
            uint sticks = present && !keyboard ? axes[device] : 0x80808080;
            Controller.SetState(player, present ? keyboard ? KeyboardState : states[device] : (ushort)0xFFFF);
            Controller.SetAxes(player, (byte)(sticks >> 16), (byte)(sticks >> 24),
                (byte)sticks, (byte)(sticks >> 8));
            Controller.LocalConnected[player] = present;
        }
        Controller.Connected2 = Controller.LocalConnected[1];
    }
}
