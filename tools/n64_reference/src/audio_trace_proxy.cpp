#include "m64p_min.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static HMODULE g_real;
static AUDIO_INFO g_audio = {};
static HANDLE g_mapping;
static V8N64Shared *g_shared;
static std::ofstream g_trace;
static unsigned long g_sequence;

template <typename T>
static T Real(const char *name)
{
    return reinterpret_cast<T>(GetProcAddress(g_real, name));
}

static long CurrentFrame()
{
    return g_shared ? g_shared->frame : -1;
}

static uint32_t RegisterValue(const unsigned int *value)
{
    return value ? *value : 0;
}

static uint32_t TaskWord(size_t index)
{
    if (g_audio.DMEM == nullptr || index >= 16)
        return 0;
    return reinterpret_cast<const uint32_t *>(g_audio.DMEM + 0xFC0)[index];
}

static void Trace(const char *event)
{
    if (!g_trace.is_open())
        return;
    g_trace << "{\"sequence\":" << g_sequence++
            << ",\"frame\":" << CurrentFrame()
            << ",\"event\":\"" << event << "\""
            << ",\"ai_dram_addr\":"
            << (RegisterValue(g_audio.AI_DRAM_ADDR_REG) & 0x00FFFFFFu)
            << ",\"ai_len\":" << RegisterValue(g_audio.AI_LEN_REG)
            << ",\"task_type\":" << TaskWord(0)
            << ",\"task_ucode_data\":" << (TaskWord(6) & 0x00FFFFFFu)
            << ",\"task_ucode_data_size\":" << TaskWord(7)
            << ",\"task_output_buffer\":" << (TaskWord(10) & 0x00FFFFFFu)
            << ",\"task_output_size_ptr\":" << (TaskWord(11) & 0x00FFFFFFu)
            << ",\"task_data_ptr\":" << (TaskWord(12) & 0x00FFFFFFu)
            << ",\"task_data_size\":" << TaskWord(13)
            << "}\n";
    g_trace.flush();
}

static void OpenSharedState()
{
    const char *name = std::getenv("V8_N64_SHARED_NAME");
    if (name == nullptr || *name == 0)
        return;
    g_mapping = OpenFileMappingA(FILE_MAP_READ, FALSE, name);
    if (g_mapping == nullptr)
        return;
    g_shared = (V8N64Shared *)MapViewOfFile(
        g_mapping, FILE_MAP_READ, 0, 0, sizeof(V8N64Shared));
    if (g_shared != nullptr && g_shared->magic != V8_N64_SHARED_MAGIC) {
        UnmapViewOfFile(g_shared);
        g_shared = nullptr;
    }
}

extern "C" __declspec(dllexport) m64p_error PluginGetVersion(
    m64p_plugin_type *type, int *version, int *api,
    const char **name, int *capabilities)
{
    return Real<ptr_PluginGetVersion>("PluginGetVersion")(
        type, version, api, name, capabilities);
}

extern "C" __declspec(dllexport) m64p_error PluginStartup(
    HMODULE core, void *context, void (*debug)(void *, int, const char *))
{
    const char *realPath = std::getenv("V8_N64_REAL_AUDIO");
    const char *output = std::getenv("V8_N64_TRACE_OUTPUT");
    if (realPath == nullptr || output == nullptr)
        return M64ERR_INPUT_INVALID;
    g_real = LoadLibraryA(realPath);
    if (g_real == nullptr)
        return M64ERR_FILES;
    fs::create_directories(output);
    g_trace.open(fs::path(output) / "audio_task_trace.jsonl", std::ios::app);
    OpenSharedState();
    return Real<ptr_PluginStartup>("PluginStartup")(core, context, debug);
}

extern "C" __declspec(dllexport) m64p_error PluginShutdown()
{
    const m64p_error result = Real<ptr_PluginShutdown>("PluginShutdown")();
    if (g_shared)
        UnmapViewOfFile(g_shared);
    if (g_mapping)
        CloseHandle(g_mapping);
    g_shared = nullptr;
    g_mapping = nullptr;
    if (g_real)
        FreeLibrary(g_real);
    g_real = nullptr;
    return result;
}

extern "C" __declspec(dllexport) int InitiateAudio(AUDIO_INFO info)
{
    g_audio = info;
    return Real<int (*)(AUDIO_INFO)>("InitiateAudio")(info);
}

extern "C" __declspec(dllexport) void AiDacrateChanged(int systemType)
{
    Trace("AiDacrateChanged");
    Real<void (*)(int)>("AiDacrateChanged")(systemType);
}

extern "C" __declspec(dllexport) void AiLenChanged()
{
    Trace("AiLenChanged");
    Real<void (*)()>("AiLenChanged")();
}

extern "C" __declspec(dllexport) void ProcessAList()
{
    Trace("ProcessAList");
    Real<void (*)()>("ProcessAList")();
}

extern "C" __declspec(dllexport) int RomOpen()
{
    auto function = Real<int (*)()>("RomOpen");
    return function ? function() : 1;
}

extern "C" __declspec(dllexport) void RomClosed()
{
    Real<void (*)()>("RomClosed")();
}

extern "C" __declspec(dllexport) void SetSpeedFactor(int percent)
{
    Real<void (*)(int)>("SetSpeedFactor")(percent);
}

extern "C" __declspec(dllexport) void VolumeDown()
{
    Real<void (*)()>("VolumeDown")();
}

extern "C" __declspec(dllexport) int VolumeGetLevel()
{
    return Real<int (*)()>("VolumeGetLevel")();
}

extern "C" __declspec(dllexport) const char *VolumeGetString()
{
    return Real<const char *(*)()>("VolumeGetString")();
}

extern "C" __declspec(dllexport) void VolumeMute()
{
    Real<void (*)()>("VolumeMute")();
}

extern "C" __declspec(dllexport) void VolumeSetLevel(int level)
{
    Real<void (*)(int)>("VolumeSetLevel")(level);
}

extern "C" __declspec(dllexport) void VolumeUp()
{
    Real<void (*)()>("VolumeUp")();
}
