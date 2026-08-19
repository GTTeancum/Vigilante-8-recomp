#include "m64p_min.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static HMODULE g_real;
static GFX_INFO g_gfx = {};
static HANDLE g_mapping;
static V8N64Shared *g_shared;
static fs::path g_output;
static std::set<long> g_captureFrames;
static std::set<long> g_dumpedMemory;
static std::set<long> g_dumpedScreen;
static long g_captureEvery;
static bool g_dumpRdram;
static bool g_dumpRdpCommands;
static unsigned long g_dlistSequence;
static std::ofstream g_callLog;
static unsigned long g_callSequence;

static void TraceCall(const char *name, const char *phase)
{
    if (!g_callLog.is_open() || g_callSequence >= 128)
        return;
    g_callLog << g_callSequence++ << " " << name << " " << phase << "\n";
    g_callLog.flush();
}

template <typename T>
static T Real(const char *name)
{
    return reinterpret_cast<T>(GetProcAddress(g_real, name));
}

template <typename T>
static bool CallOptionalVoid0(const char *name)
{
    TraceCall(name, "enter");
    T function = Real<T>(name);
    if (function == nullptr) {
        TraceCall(name, "missing");
        return false;
    }
    function();
    TraceCall(name, "return");
    return true;
}

static long CurrentFrame()
{
    if (g_shared == nullptr)
        return -1;
    return g_shared->frame;
}

static bool ShouldCapture(long frame)
{
    if (frame < 0)
        return false;
    if (g_captureFrames.find(frame) != g_captureFrames.end())
        return true;
    return g_captureEvery > 0 && frame % g_captureEvery == 0;
}

static void ParseCaptureFrames()
{
    const char *list = std::getenv("V8_N64_CAPTURE_FRAMES");
    if (list != nullptr) {
        std::istringstream stream(list);
        std::string token;
        while (std::getline(stream, token, ','))
            g_captureFrames.insert(std::strtol(token.c_str(), nullptr, 0));
    }
    const char *every = std::getenv("V8_N64_CAPTURE_EVERY");
    g_captureEvery = every ? std::strtol(every, nullptr, 0) : 0;
    const char *dump = std::getenv("V8_N64_DUMP_RDRAM");
    g_dumpRdram = dump != nullptr && std::strtol(dump, nullptr, 0) != 0;
    const char *dumpRdp = std::getenv("V8_N64_DUMP_RDP_COMMANDS");
    g_dumpRdpCommands =
        dumpRdp != nullptr && std::strtol(dumpRdp, nullptr, 0) != 0;
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

static void WriteBinary(const fs::path &path, const void *data, size_t size)
{
    std::ofstream file(path, std::ios::binary);
    file.write((const char *)data, (std::streamsize)size);
}

static uint32_t RegisterValue(const unsigned int *value)
{
    return value ? *value : 0;
}

static void DumpTask(long frame)
{
    if (!ShouldCapture(frame) ||
        g_dumpedMemory.find(frame) != g_dumpedMemory.end())
        return;
    g_dumpedMemory.insert(frame);
    const std::string stem = "n64_frame_" +
        std::to_string(frame) + "_dlist_" + std::to_string(g_dlistSequence);
    WriteBinary(g_output / (stem + ".dmem.bin"), g_gfx.DMEM, 0x1000);
    WriteBinary(g_output / (stem + ".imem.bin"), g_gfx.IMEM, 0x1000);
    if (g_dumpRdram && g_gfx.RDRAM != nullptr) {
        const size_t size = g_gfx.RDRAM_SIZE ? *g_gfx.RDRAM_SIZE : 0x800000;
        WriteBinary(g_output / (stem + ".rdram.bin"), g_gfx.RDRAM, size);
    }
    std::ofstream manifest(g_output / "rsp_rdp_trace.jsonl", std::ios::app);
    manifest << "{\"frame\":" << frame
             << ",\"dlist_sequence\":" << g_dlistSequence
             << ",\"dpc_start\":" << RegisterValue(g_gfx.DPC_START_REG)
             << ",\"dpc_end\":" << RegisterValue(g_gfx.DPC_END_REG)
             << ",\"dpc_current\":" << RegisterValue(g_gfx.DPC_CURRENT_REG)
             << ",\"dpc_status\":" << RegisterValue(g_gfx.DPC_STATUS_REG)
             << ",\"vi_origin\":" << RegisterValue(g_gfx.VI_ORIGIN_REG)
             << ",\"vi_width\":" << RegisterValue(g_gfx.VI_WIDTH_REG)
             << ",\"rdram_size\":"
             << (g_gfx.RDRAM_SIZE ? *g_gfx.RDRAM_SIZE : 0x800000)
             << "}\n";
}

static void DumpRdpCommands(long frame)
{
    if (!g_dumpRdpCommands || !ShouldCapture(frame) ||
        g_gfx.RDRAM == nullptr || g_gfx.DPC_START_REG == nullptr ||
        g_gfx.DPC_END_REG == nullptr)
        return;
    const uint32_t mask =
        (g_gfx.RDRAM_SIZE ? *g_gfx.RDRAM_SIZE : 0x800000) - 1;
    const uint32_t start = *g_gfx.DPC_START_REG & mask;
    const uint32_t end = *g_gfx.DPC_END_REG & mask;
    if (end <= start || end - start > 0x100000)
        return;
    const std::string stem = "n64_frame_" + std::to_string(frame) +
        "_rdp_" + std::to_string(g_dlistSequence) + "_" +
        std::to_string(start) + "_" + std::to_string(end);
    WriteBinary(g_output / (stem + ".bin"), g_gfx.RDRAM + start, end - start);
    std::ofstream manifest(g_output / "rdp_command_trace.jsonl", std::ios::app);
    manifest << "{\"frame\":" << frame
             << ",\"dlist_sequence\":" << g_dlistSequence
             << ",\"start\":" << start
             << ",\"end\":" << end
             << ",\"bytes\":" << (end - start) << "}\n";
}

static void CaptureScreen(long frame)
{
    if (!ShouldCapture(frame) ||
        g_dumpedScreen.find(frame) != g_dumpedScreen.end())
        return;
    DumpTask(frame);
    using Fn = void (*)(void *, int *, int *, int);
    Fn readScreen = Real<Fn>("ReadScreen2");
    if (readScreen == nullptr)
        return;
    int width = 0;
    int height = 0;
    readScreen(nullptr, &width, &height, 1);
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192)
        return;
    std::vector<unsigned char> pixels((size_t)width * height * 3);
    readScreen(pixels.data(), &width, &height, 1);
    std::ofstream file(
        g_output / ("n64_frame_" + std::to_string(frame) + ".ppm"),
        std::ios::binary);
    file << "P6\n" << width << " " << height << "\n255\n";
    for (int y = height - 1; y >= 0; y--) {
        file.write(
            (const char *)(pixels.data() + (size_t)y * width * 3),
            (std::streamsize)width * 3);
    }
    g_dumpedScreen.insert(frame);
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
    const char *realPath = std::getenv("V8_N64_REAL_GFX");
    const char *output = std::getenv("V8_N64_TRACE_OUTPUT");
    if (realPath == nullptr || output == nullptr)
        return M64ERR_INPUT_INVALID;
    g_real = LoadLibraryA(realPath);
    if (g_real == nullptr)
        return M64ERR_FILES;
    g_output = fs::path(output);
    fs::create_directories(g_output);
    g_callLog.open(g_output / "gfx_proxy_calls.log", std::ios::app);
    ParseCaptureFrames();
    OpenSharedState();
    return Real<ptr_PluginStartup>("PluginStartup")(core, context, debug);
}

extern "C" __declspec(dllexport) m64p_error PluginShutdown()
{
    const m64p_error result =
        Real<ptr_PluginShutdown>("PluginShutdown")();
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

extern "C" __declspec(dllexport) int InitiateGFX(GFX_INFO info)
{
    TraceCall("InitiateGFX", "enter");
    g_gfx = info;
    using Fn = int (*)(GFX_INFO);
    const int result = Real<Fn>("InitiateGFX")(info);
    TraceCall("InitiateGFX", "return");
    return result;
}

extern "C" __declspec(dllexport) void ProcessDList()
{
    TraceCall("ProcessDList", "enter");
    g_dlistSequence++;
    DumpTask(CurrentFrame());
    Real<void (*)()>("ProcessDList")();
    TraceCall("ProcessDList", "return");
}

extern "C" __declspec(dllexport) void ProcessRDPList()
{
    g_dlistSequence++;
    DumpRdpCommands(CurrentFrame());
    DumpTask(CurrentFrame());
    CallOptionalVoid0<void (*)()>("ProcessRDPList");
}

extern "C" __declspec(dllexport) void UpdateScreen()
{
    TraceCall("UpdateScreen", "enter");
    Real<void (*)()>("UpdateScreen")();
    CaptureScreen(CurrentFrame());
    TraceCall("UpdateScreen", "return");
}

#define V8_FORWARD_VOID0(name) \
    extern "C" __declspec(dllexport) void name() { \
        CallOptionalVoid0<void (*)()>(#name); \
    }
#define V8_FORWARD_VOID2(name, t1, t2) \
    extern "C" __declspec(dllexport) void name(t1 a, t2 b) { \
        auto function = Real<void (*)(t1, t2)>(#name); \
        if (function != nullptr) \
            function(a, b); \
    }

V8_FORWARD_VOID0(ChangeWindow)
V8_FORWARD_VOID0(RomClosed)
V8_FORWARD_VOID0(ShowCFB)
V8_FORWARD_VOID0(ViStatusChanged)
V8_FORWARD_VOID0(ViWidthChanged)

extern "C" __declspec(dllexport) int RomOpen()
{
    TraceCall("RomOpen", "enter");
    auto function = Real<int (*)()>("RomOpen");
    const int result = function != nullptr ? function() : 1;
    TraceCall("RomOpen", "return");
    return result;
}
extern "C" __declspec(dllexport) void MoveScreen(int x, int y)
{
    Real<void (*)(int, int)>("MoveScreen")(x, y);
}
extern "C" __declspec(dllexport) void ReadScreen2(
    void *dest, int *width, int *height, int front)
{
    TraceCall("ReadScreen2", "enter");
    auto function = Real<void (*)(void *, int *, int *, int)>("ReadScreen2");
    if (function != nullptr)
        function(dest, width, height, front);
    TraceCall("ReadScreen2", function != nullptr ? "return" : "missing");
}
extern "C" __declspec(dllexport) void ResizeVideoOutput(int width, int height)
{
    TraceCall("ResizeVideoOutput", "enter");
    auto function = Real<void (*)(int, int)>("ResizeVideoOutput");
    if (function != nullptr)
        function(width, height);
    TraceCall("ResizeVideoOutput", function != nullptr ? "return" : "missing");
}
extern "C" __declspec(dllexport) void SetRenderingCallback(void (*callback)(int))
{
    TraceCall("SetRenderingCallback", "enter");
    auto function = Real<void (*)(void (*)(int))>("SetRenderingCallback");
    if (function != nullptr)
        function(callback);
    TraceCall(
        "SetRenderingCallback", function != nullptr ? "return" : "missing");
}
extern "C" __declspec(dllexport) void FBRead(unsigned int address)
{
    TraceCall("FBRead", "enter");
    auto function = Real<void (*)(unsigned int)>("FBRead");
    if (function != nullptr)
        function(address);
    TraceCall("FBRead", function != nullptr ? "return" : "missing");
}
extern "C" __declspec(dllexport) void FBWrite(
    unsigned int address, unsigned int size)
{
    TraceCall("FBWrite", "enter");
    auto function = Real<void (*)(unsigned int, unsigned int)>("FBWrite");
    if (function != nullptr)
        function(address, size);
    TraceCall("FBWrite", function != nullptr ? "return" : "missing");
}
extern "C" __declspec(dllexport) void FBGetFrameBufferInfo(void *info)
{
    TraceCall("FBGetFrameBufferInfo", "enter");
    auto function = Real<void (*)(void *)>("FBGetFrameBufferInfo");
    if (function != nullptr)
        function(info);
    TraceCall(
        "FBGetFrameBufferInfo", function != nullptr ? "return" : "missing");
}
