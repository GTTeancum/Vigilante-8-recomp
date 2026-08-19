#include "m64p_min.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct InputSpan {
    long first;
    long last;
    uint32_t buttons;
    int8_t x;
    int8_t y;
};

static std::vector<InputSpan> g_spans;
static HANDLE g_mapping;
static V8N64Shared *g_shared;
static void *g_debugContext;
static void (*g_debug)(void *, int, const char *);

static std::string Trim(std::string value)
{
    while (!value.empty() && std::isspace((unsigned char)value.front()))
        value.erase(value.begin());
    while (!value.empty() && std::isspace((unsigned char)value.back()))
        value.pop_back();
    return value;
}

static uint32_t ParseButtons(const std::string &text)
{
    static const struct {
        const char *name;
        uint32_t mask;
    } names[] = {
        {"DR", 0x0001}, {"DL", 0x0002}, {"DD", 0x0004}, {"DU", 0x0008},
        {"START", 0x0010}, {"Z", 0x0020}, {"B", 0x0040}, {"A", 0x0080},
        {"CR", 0x0100}, {"CL", 0x0200}, {"CD", 0x0400}, {"CU", 0x0800},
        {"R", 0x1000}, {"L", 0x2000}
    };
    uint32_t result = 0;
    std::istringstream stream(text);
    std::string token;
    while (std::getline(stream, token, '+')) {
        token = Trim(token);
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c) { return (char)std::toupper(c); });
        if (token.empty() || token == "NONE")
            continue;
        for (const auto &entry : names) {
            if (token == entry.name) {
                result |= entry.mask;
                break;
            }
        }
    }
    return result;
}

static void LoadTimeline()
{
    g_spans.clear();
    const char *path = std::getenv("V8_N64_INPUT_SCRIPT");
    if (path == nullptr || *path == 0)
        return;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream stream(line);
        std::string fields[5];
        if (!std::getline(stream, fields[0], ',') ||
            !std::getline(stream, fields[1], ',') ||
            !std::getline(stream, fields[2], ',') ||
            !std::getline(stream, fields[3], ',') ||
            !std::getline(stream, fields[4]))
            continue;
        InputSpan span = {};
        span.first = std::strtol(Trim(fields[0]).c_str(), nullptr, 0);
        span.last = std::strtol(Trim(fields[1]).c_str(), nullptr, 0);
        const std::string buttons = Trim(fields[2]);
        std::string upperButtons = buttons;
        std::transform(
            upperButtons.begin(), upperButtons.end(), upperButtons.begin(),
            [](unsigned char c) { return (char)std::toupper(c); });
        if (upperButtons.rfind("PASSCODE:", 0) == 0) {
            const std::string code = upperButtons.substr(9);
            long cursor = span.first;
            auto pulse = [&](uint32_t mask) {
                g_spans.push_back({cursor, cursor + 1, mask, 0, 0});
                cursor += 4;
            };
            for (size_t index = 0; index < code.size(); index++) {
                const char character = code[index];
                int position = 0;
                if (character >= 'A' && character <= 'Z')
                    position = character - 'A' + 1;
                else if (character >= '0' && character <= '9')
                    position = character - '0' + 27;
                else if (character == '@')
                    position = 37;
                const int downSteps = position;
                const int upSteps = (38 - position) % 38;
                const bool useDown = downSteps <= upSteps;
                const int steps = useDown ? downSteps : upSteps;
                for (int step = 0; step < steps; step++)
                    pulse(useDown ? 0x0004u : 0x0008u);
                if (index + 1 < code.size())
                    pulse(0x0001u);
            }
            cursor += 16;
            pulse(0x0080u);
            continue;
        }
        span.buttons = ParseButtons(buttons);
        span.x = (int8_t)std::clamp(
            std::strtol(Trim(fields[3]).c_str(), nullptr, 0), -128L, 127L);
        span.y = (int8_t)std::clamp(
            std::strtol(Trim(fields[4]).c_str(), nullptr, 0), -128L, 127L);
        g_spans.push_back(span);
    }
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
    if (type)
        *type = M64PLUGIN_INPUT;
    if (version)
        *version = V8_M64P_PLUGIN_VERSION;
    if (api)
        *api = V8_M64P_INPUT_API;
    if (name)
        *name = "V8 deterministic reference input";
    if (capabilities)
        *capabilities = 0;
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) m64p_error PluginStartup(
    HMODULE, void *context, void (*debug)(void *, int, const char *))
{
    g_debugContext = context;
    g_debug = debug;
    LoadTimeline();
    OpenSharedState();
    if (g_debug)
    {
        std::string message = "V8 deterministic input initialized; spans=" +
            std::to_string(g_spans.size());
        g_debug(g_debugContext, 3, message.c_str());
    }
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) m64p_error PluginShutdown()
{
    if (g_shared)
        UnmapViewOfFile(g_shared);
    if (g_mapping)
        CloseHandle(g_mapping);
    g_shared = nullptr;
    g_mapping = nullptr;
    g_spans.clear();
    return M64ERR_SUCCESS;
}

extern "C" __declspec(dllexport) void InitiateControllers(CONTROL_INFO info)
{
    for (int index = 0; index < 4; index++) {
        info.Controls[index].Present = index == 0;
        info.Controls[index].RawData = 0;
        info.Controls[index].Plugin = V8_N64_PLUGIN_NONE;
        info.Controls[index].Type = 0;
    }
}

extern "C" __declspec(dllexport) void GetKeys(int control, BUTTONS *keys)
{
    keys->Value = 0;
    if (control != 0 || g_shared == nullptr)
        return;
    const long frame = g_shared->frame;
    for (const InputSpan &span : g_spans) {
        if (frame >= span.first && frame <= span.last) {
            keys->Value = span.buttons |
                ((uint32_t)(uint8_t)span.x << 16) |
                ((uint32_t)(uint8_t)span.y << 24);
        }
    }
}

extern "C" __declspec(dllexport) void ControllerCommand(int, unsigned char *) {}
extern "C" __declspec(dllexport) void ReadController(int, unsigned char *) {}
extern "C" __declspec(dllexport) void SDL_KeyDown(int, int) {}
extern "C" __declspec(dllexport) void SDL_KeyUp(int, int) {}
extern "C" __declspec(dllexport) void RenderCallback() {}
extern "C" __declspec(dllexport) int RomOpen() { return 1; }
extern "C" __declspec(dllexport) void RomClosed() {}
