using RecompOne.Runtime.Hardware;

namespace RecompOne.Runtime.Config;

// Device slots and player numbers are distinct: the first device to join is
// P1, and only that device's Start button may confirm the roster.
public sealed class LocalJoinState
{
    readonly ushort[] _previous;
    readonly List<int> _devices = new(4);
    public IReadOnlyList<int> Devices => _devices;
    public bool Confirmed { get; private set; }
    public bool Cancelled { get; private set; }

    public LocalJoinState(ReadOnlySpan<ushort> held)
    {
        if (held.Length is < 4 or > LocalInputSession.DeviceCount) throw new ArgumentException("Four gamepads and optional keyboard required", nameof(held));
        _previous = new ushort[held.Length];
        held.CopyTo(_previous);
    }

    public void Update(ReadOnlySpan<ushort> held, ReadOnlySpan<bool> connected, bool keyboardConfirm = false)
    {
        if (held.Length != _previous.Length || connected.Length != held.Length) throw new ArgumentException("Device counts must match");
        if (Confirmed || Cancelled) return;
        Span<ushort> pressed = stackalloc ushort[held.Length];
        for (int device = 0; device < held.Length; device++)
        {
            pressed[device] = (ushort)(held[device] & ~_previous[device]);
            _previous[device] = held[device];
            if (!connected[device]) continue;
            if ((pressed[device] & Controller.Circle) != 0) _devices.Remove(device);
            else if ((pressed[device] & Controller.Cross) != 0 && _devices.Count < 4 && !_devices.Contains(device)) _devices.Add(device);
        }
        int host = _devices.Count > 0 ? _devices[0] : 0;
        if ((pressed[host] & Controller.Triangle) != 0) { Cancelled = true; return; }
        bool allConnected = true;
        foreach (int device in _devices) allConnected &= connected[device];
        if (_devices.Count >= 2 && allConnected &&
            ((pressed[host] & Controller.Start) != 0 || keyboardConfirm)) Confirmed = true;
    }
}
