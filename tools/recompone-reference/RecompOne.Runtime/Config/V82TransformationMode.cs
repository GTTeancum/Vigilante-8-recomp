namespace RecompOne.Runtime.Config;

/// <summary>Saved gameplay policy, independent of the native vehicle mode byte.</summary>
public enum V82TransformationMode
{
    All = 0,
    None = 1,
    AutoWaterski = 2,
}

public static class V82TransformationSettings
{
    public static V82TransformationMode Normalize(V82TransformationMode mode) =>
        mode is V82TransformationMode.All or V82TransformationMode.None or
            V82TransformationMode.AutoWaterski ? mode : V82TransformationMode.All;

    public static string Label(V82TransformationMode mode) => Normalize(mode) switch
    {
        V82TransformationMode.None => "No transformations",
        V82TransformationMode.AutoWaterski => "Auto Waterski",
        _ => "All transformations",
    };

    public static V82TransformationMode Cycle(V82TransformationMode mode, int direction)
    {
        int index = (int)Normalize(mode);
        return (V82TransformationMode)((index + Math.Sign(direction) + 3) % 3);
    }

    public static bool AllowsPowerups(V82TransformationMode mode) =>
        Normalize(mode) == V82TransformationMode.All;
}
