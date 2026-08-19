#include "m64p_min.h"

static int g_volume = 100;
static bool g_muted;

extern "C" __declspec(dllexport) m64p_error PluginGetVersion(
    m64p_plugin_type *type, int *version, int *api,
    const char **name, int *capabilities)
{
    if (type)
        *type = M64PLUGIN_AUDIO;
    if (version)
        *version = V8_M64P_PLUGIN_VERSION;
    if (api)
        *api = 0x020000;
    if (name)
        *name = "V8 deterministic null audio";
    if (capabilities)
        *capabilities = 0;
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) m64p_error PluginStartup(
    HMODULE, void *, void (*)(void *, int, const char *))
{
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) m64p_error PluginShutdown()
{
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) int InitiateAudio(AUDIO_INFO)
{
    return 1;
}

extern "C" __declspec(dllexport) void AiDacrateChanged(int) {}
extern "C" __declspec(dllexport) void AiLenChanged() {}
extern "C" __declspec(dllexport) void ProcessAList() {}
extern "C" __declspec(dllexport) int RomOpen() { return 1; }
extern "C" __declspec(dllexport) void RomClosed() {}
extern "C" __declspec(dllexport) void SetSpeedFactor(int) {}
extern "C" __declspec(dllexport) void VolumeDown()
{
    if (g_volume > 0)
        g_volume -= 10;
}
extern "C" __declspec(dllexport) int VolumeGetLevel() { return g_volume; }
extern "C" __declspec(dllexport) const char *VolumeGetString()
{
    return g_muted ? "Mute" : "100%";
}
extern "C" __declspec(dllexport) void VolumeMute() { g_muted = !g_muted; }
extern "C" __declspec(dllexport) void VolumeSetLevel(int level)
{
    g_volume = level < 0 ? 0 : (level > 100 ? 100 : level);
}
extern "C" __declspec(dllexport) void VolumeUp()
{
    if (g_volume < 100)
        g_volume += 10;
}
