namespace RecompOne.Runtime.Config;

public enum SoundtrackMode
{
    Vigilante8 = 0,
    SecondOffense = 1,
    Both = 2,
}

public static class SoundtrackSettings
{
    public static SoundtrackMode Normalize(SoundtrackMode mode) =>
        Enum.IsDefined(mode) ? mode : SoundtrackMode.Vigilante8;

    public static string Label(SoundtrackMode mode) => Normalize(mode) switch
    {
        SoundtrackMode.SecondOffense => "Second Offense",
        SoundtrackMode.Both => "Both",
        _ => "Vigilante 8",
    };

    public static SoundtrackMode Cycle(SoundtrackMode mode, int direction) =>
        (SoundtrackMode)(((int)Normalize(mode) + Math.Sign(direction) + 3) % 3);
}
