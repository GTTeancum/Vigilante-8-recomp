using RecompOne.Runtime.Config;

namespace RecompOne.Runtime.Sdk;

/// <summary>
/// Shared request authority. Neither player identity nor arena content changes
/// eligibility. Automatic water requests are scoped to one native call, not a
/// global bypass that can leak into later pickups or another vehicle.
/// </summary>
public static class V82TransformationPolicy
{
    // Native mode 2 uses the floating/water movement handler 80040598.
    // Mode 3 is the distinct snow/ski handler 80040F10.
    public const uint WaterMode = 2;
    [ThreadStatic] static uint _automaticVehicle;

    public static bool AllowsActivation(uint vehicle, uint nativeMode)
    {
        if (nativeMode == 0) return true; // Cleanup is always legal.
        var policy = V82TransformationSettings.Normalize(ConfigManager.Game.V82Transformations);
        return policy == V82TransformationMode.All ||
            (policy == V82TransformationMode.AutoWaterski &&
             vehicle != 0 && vehicle == _automaticVehicle && nativeMode == WaterMode);
    }

    internal static IDisposable AuthorizeWaterTransition(uint vehicle) => new RequestScope(vehicle);

    sealed class RequestScope : IDisposable
    {
        readonly uint _previous = _automaticVehicle;
        bool _disposed;
        public RequestScope(uint vehicle) => _automaticVehicle = vehicle;
        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;
            _automaticVehicle = _previous;
        }
    }
}
