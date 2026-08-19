#include "m64p_min.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static ptr_CoreDoCommand g_command;
static V8N64Shared *g_shared;
static std::ofstream g_log;

template <typename T>
static T Load(HMODULE module, const char *name)
{
    T result = reinterpret_cast<T>(GetProcAddress(module, name));
    if (result == nullptr) {
        g_log << "missing export " << name << "\n";
        g_log.flush();
        ExitProcess(20);
    }
    return result;
}

static std::string Env(const char *name)
{
    const char *value = std::getenv(name);
    return value ? value : "";
}

static void DebugCallback(void *, int level, const char *message)
{
    g_log << "[m64p:" << level << "] " << (message ? message : "") << "\n";
    g_log.flush();
}

static void StateCallback(void *, m64p_core_param param, int value)
{
    g_log << "[state] " << (int)param << "=" << value << "\n";
    g_log.flush();
}

static void FrameCallback(unsigned int frame)
{
    if (g_shared == nullptr)
        return;
    InterlockedExchange(&g_shared->frame, (LONG)frame);
    const LONG stopFrame =
        InterlockedCompareExchange(&g_shared->stopFrame, 0, 0);
    if (stopFrame >= 0 && frame >= (unsigned int)stopFrame && g_command)
        g_command(M64CMD_STOP, 0, nullptr);
}

static std::vector<unsigned char> ReadFile(const fs::path &path)
{
    std::ifstream file(path, std::ios::binary);
    file.seekg(0, std::ios::end);
    const size_t size = (size_t)file.tellg();
    file.seekg(0);
    std::vector<unsigned char> result(size);
    file.read((char *)result.data(), (std::streamsize)size);
    return result;
}

static int Worker()
{
    const fs::path vendor = fs::path(Env("V8_N64_VENDOR"));
    const fs::path output = fs::path(Env("V8_N64_TRACE_OUTPUT"));
    const fs::path rom = fs::path(Env("V8_N64_ROM"));
    const fs::path config = fs::path(Env("V8_N64_CONFIG"));
    const fs::path data = vendor / "Data";
    const fs::path corePath = vendor / "Core" / "mupen64plus.dll";
    const fs::path gfxPath = fs::path(Env("V8_N64_GFX_PROXY"));
    const fs::path audioPath = fs::path(Env("V8_N64_AUDIO"));
    const fs::path inputPath = fs::path(Env("V8_N64_INPUT_PLUGIN"));
    const fs::path rspPath = fs::path(Env("V8_N64_RSP"));

    fs::create_directories(output);
    fs::create_directories(config);
    g_log.open(output / "headless_frontend.log", std::ios::app);
    SetDllDirectoryW(vendor.wstring().c_str());

    const std::string sharedName = Env("V8_N64_SHARED_NAME");
    HANDLE mapping = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS, FALSE, sharedName.c_str());
    if (mapping == nullptr)
        return 2;
    g_shared = (V8N64Shared *)MapViewOfFile(
        mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(V8N64Shared));
    if (g_shared == nullptr)
        return 3;

    HMODULE core = LoadLibraryW(corePath.wstring().c_str());
    HMODULE gfx = LoadLibraryW(gfxPath.wstring().c_str());
    HMODULE audio = LoadLibraryW(audioPath.wstring().c_str());
    HMODULE input = LoadLibraryW(inputPath.wstring().c_str());
    HMODULE rsp = LoadLibraryW(rspPath.wstring().c_str());
    if (!core || !gfx || !audio || !input || !rsp) {
        g_log << "LoadLibrary failure " << GetLastError() << "\n";
        return 4;
    }

    auto startup = Load<ptr_CoreStartup>(core, "CoreStartup");
    auto shutdown = Load<ptr_CoreShutdown>(core, "CoreShutdown");
    auto attach = Load<ptr_CoreAttachPlugin>(core, "CoreAttachPlugin");
    auto detach = Load<ptr_CoreDetachPlugin>(core, "CoreDetachPlugin");
    g_command = Load<ptr_CoreDoCommand>(core, "CoreDoCommand");
    auto errorText = Load<ptr_CoreErrorMessage>(core, "CoreErrorMessage");

    m64p_error error = startup(
        V8_M64P_FRONTEND_API,
        config.string().c_str(), data.string().c_str(),
        nullptr, DebugCallback, nullptr, StateCallback);
    if (error != M64ERR_SUCCESS) {
        g_log << "CoreStartup: " << errorText(error) << "\n";
        return 5;
    }

    std::vector<unsigned char> romBytes = ReadFile(rom);
    error = g_command(
        M64CMD_ROM_OPEN, (int)romBytes.size(), romBytes.data());
    if (error != M64ERR_SUCCESS) {
        g_log << "ROM_OPEN: " << errorText(error) << "\n";
        shutdown();
        return 8;
    }

    struct Plugin {
        HMODULE module;
        m64p_plugin_type type;
        const char *label;
    } plugins[] = {
        {gfx, M64PLUGIN_GFX, "gfx"},
        {audio, M64PLUGIN_AUDIO, "audio"},
        {input, M64PLUGIN_INPUT, "input"},
        {rsp, M64PLUGIN_RSP, "rsp"}
    };
    for (Plugin &plugin : plugins) {
        error = Load<ptr_PluginStartup>(
            plugin.module, "PluginStartup")(
                core, nullptr, DebugCallback);
        if (error != M64ERR_SUCCESS) {
            g_log << plugin.label << " PluginStartup failed " << error << "\n";
            g_command(M64CMD_ROM_CLOSE, 0, nullptr);
            shutdown();
            return 6;
        }
        error = attach(plugin.type, plugin.module);
        if (error != M64ERR_SUCCESS) {
            g_log << plugin.label << " attach: " << errorText(error) << "\n";
            Load<ptr_PluginShutdown>(
                plugin.module, "PluginShutdown")();
            g_command(M64CMD_ROM_CLOSE, 0, nullptr);
            shutdown();
            return 7;
        }
    }
    using MuteFn = void (*)();
    MuteFn mute = reinterpret_cast<MuteFn>(
        GetProcAddress(audio, "VolumeMute"));
    if (mute != nullptr)
        mute();

    g_command(
        M64CMD_SET_FRAME_CALLBACK, 0, (void *)FrameCallback);
    error = g_command(M64CMD_EXECUTE, 0, nullptr);
    g_log << "EXECUTE returned " << error << " frame=" << g_shared->frame << "\n";

    g_command(M64CMD_ROM_CLOSE, 0, nullptr);
    for (int index = 3; index >= 0; index--) {
        detach(plugins[index].type);
        Load<ptr_PluginShutdown>(
            plugins[index].module, "PluginShutdown")();
    }
    shutdown();
    FreeLibrary(rsp);
    FreeLibrary(input);
    FreeLibrary(audio);
    FreeLibrary(gfx);
    FreeLibrary(core);
    UnmapViewOfFile(g_shared);
    CloseHandle(mapping);
    return error == M64ERR_SUCCESS ? 0 : 9;
}

static std::wstring Quote(const std::wstring &value)
{
    return L"\"" + value + L"\"";
}

static int LaunchHiddenDesktop()
{
    const DWORD pid = GetCurrentProcessId();
    const std::wstring desktopName =
        L"V8N64Trace_" + std::to_wstring(pid);
    HDESK desktop = CreateDesktopW(
        desktopName.c_str(), nullptr, nullptr, 0, GENERIC_ALL, nullptr);
    if (desktop == nullptr)
        return 40;

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring command = Quote(exePath) + L" --worker";
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(0);
    STARTUPINFOW startup = {};
    startup.cb = sizeof(startup);
    startup.lpDesktop = (wchar_t *)desktopName.c_str();
    PROCESS_INFORMATION process = {};
    const BOOL created = CreateProcessW(
        exePath, mutableCommand.data(), nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
        nullptr, nullptr, &startup, &process);
    if (!created) {
        CloseDesktop(desktop);
        return 41;
    }
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exitCode = 42;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    CloseDesktop(desktop);
    return (int)exitCode;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && std::string(argv[1]) == "--worker")
        return Worker();

    const std::string sharedName =
        "Local\\V8N64Trace_" + std::to_string(GetCurrentProcessId());
    SetEnvironmentVariableA("V8_N64_SHARED_NAME", sharedName.c_str());
    HANDLE mapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
        sizeof(V8N64Shared), sharedName.c_str());
    if (mapping == nullptr)
        return 30;
    V8N64Shared *shared = (V8N64Shared *)MapViewOfFile(
        mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(V8N64Shared));
    if (shared == nullptr)
        return 31;
    shared->magic = V8_N64_SHARED_MAGIC;
    shared->frame = -1;
    const std::string stop = Env("V8_N64_STOP_FRAME");
    shared->stopFrame = stop.empty() ? 1800 : std::strtol(
        stop.c_str(), nullptr, 0);

    const int result = LaunchHiddenDesktop();
    std::cout << "hidden_n64_exit=" << result
              << " final_frame=" << shared->frame << "\n";
    UnmapViewOfFile(shared);
    CloseHandle(mapping);
    return result;
}
