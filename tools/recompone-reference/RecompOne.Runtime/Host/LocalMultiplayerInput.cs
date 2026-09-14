using Silk.NET.Input;

namespace RecompOne.Runtime.Host;

public static class LocalMultiplayerInput
{
    public static ushort DeviceState(int device) => InputManager.JoinDeviceState(device);
    public static bool DeviceConnected(int device) => InputManager.JoinDeviceConnected(device);
    public static bool EnterHeld => InputManager.IsKeyDown(Key.Enter);
    public static void SignalStage(string stage) => InputManager.SignalScriptStage(stage);
    public static void MenuContext() => InputManager.SignalNativeGameplayMenu();
    public static bool UsesGamepad(int player) => InputManager.PromptUsesGamepad(player);
}
