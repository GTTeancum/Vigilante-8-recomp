#include "m64p_min.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static HMODULE g_real;
static RSP_INFO g_rsp = {};
static HANDLE g_mapping;
static V8N64Shared *g_shared;
static std::ofstream g_trace;
static unsigned long g_sequence;

template <typename T>
static T Real(const char *name)
{
    return reinterpret_cast<T>(GetProcAddress(g_real, name));
}

static uint32_t TaskWord(size_t index)
{
    if (g_rsp.DMEM == nullptr || index >= 16)
        return 0;
    return reinterpret_cast<const uint32_t *>(g_rsp.DMEM + 0xFC0)[index];
}

static void TraceTask(unsigned int cycles)
{
    if (!g_trace.is_open())
        return;
    const uint32_t type = TaskWord(0);
    if (type != 1 && type != 2)
        return;
    g_trace << "{\"sequence\":" << g_sequence++
            << ",\"frame\":" << (g_shared ? g_shared->frame : -1)
            << ",\"cycles\":" << cycles
            << ",\"task_type\":" << type
            << ",\"task_flags\":" << TaskWord(1)
            << ",\"ucode_boot\":" << (TaskWord(2) & 0x00FFFFFFu)
            << ",\"ucode_boot_size\":" << TaskWord(3)
            << ",\"ucode\":" << (TaskWord(4) & 0x00FFFFFFu)
            << ",\"ucode_size\":" << TaskWord(5)
            << ",\"ucode_data\":" << (TaskWord(6) & 0x00FFFFFFu)
            << ",\"ucode_data_size\":" << TaskWord(7)
            << ",\"dram_stack\":" << (TaskWord(8) & 0x00FFFFFFu)
            << ",\"dram_stack_size\":" << TaskWord(9)
            << ",\"output_buffer\":" << (TaskWord(10) & 0x00FFFFFFu)
            << ",\"output_size_ptr\":" << (TaskWord(11) & 0x00FFFFFFu)
            << ",\"data_ptr\":" << (TaskWord(12) & 0x00FFFFFFu)
            << ",\"data_size\":" << TaskWord(13)
            << ",\"yield_data_ptr\":" << (TaskWord(14) & 0x00FFFFFFu)
            << ",\"yield_data_size\":" << TaskWord(15)
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
    const char *realPath = std::getenv("V8_N64_REAL_RSP");
    const char *output = std::getenv("V8_N64_TRACE_OUTPUT");
    if (realPath == nullptr || output == nullptr)
        return M64ERR_INPUT_INVALID;
    g_real = LoadLibraryA(realPath);
    if (g_real == nullptr)
        return M64ERR_FILES;
    fs::create_directories(output);
    g_trace.open(fs::path(output) / "rsp_task_trace.jsonl", std::ios::app);
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

extern "C" __declspec(dllexport) void InitiateRSP(
    RSP_INFO info, unsigned int *cycleCount)
{
    g_rsp = info;
    Real<void (*)(RSP_INFO, unsigned int *)>("InitiateRSP")(info, cycleCount);
}

extern "C" __declspec(dllexport) unsigned int DoRspCycles(unsigned int cycles)
{
    TraceTask(cycles);
    return Real<unsigned int (*)(unsigned int)>("DoRspCycles")(cycles);
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

extern "C" __declspec(dllexport) void GetDllInfo(void *info)
{
    auto function = Real<void (*)(void *)>("GetDllInfo");
    if (function)
        function(info);
}
