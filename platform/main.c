/* main.c -- entry point for the v8 recomp binary. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#if defined(_MSC_VER)
#include <crtdbg.h>
#include <wchar.h>
#endif
#ifdef _WIN32
#define RECT WIN32_RECT
#include <windows.h>
#undef RECT
#endif
#include "structs.h"

#if defined(_MSC_VER)
static void V8_InvalidParameterHandler(const wchar_t *expr,
                                       const wchar_t *func,
                                       const wchar_t *file,
                                       unsigned int line,
                                       uintptr_t reserved)
{
    (void)reserved;
    fprintf(stderr,
            "v8: CRT invalid parameter expr=%ls func=%ls file=%ls line=%u\n",
            expr ? expr : L"(null)",
            func ? func : L"(null)",
            file ? file : L"(null)",
            line);
    fflush(stderr);
}
#endif

#ifdef _WIN32
static LONG WINAPI V8_UnhandledExceptionFilter(EXCEPTION_POINTERS *info)
{
    void *pc = info && info->ContextRecord
#if defined(_M_X64) || defined(__x86_64__)
             ? (void *)(uintptr_t)info->ContextRecord->Rip
#else
             ? (void *)(uintptr_t)info->ContextRecord->Eip
#endif
             : NULL;
    HMODULE mod = GetModuleHandleA(NULL);
    uintptr_t base = (uintptr_t)mod;
    uintptr_t rva = pc ? (uintptr_t)pc - base : 0;
    fprintf(stderr, "v8: FATAL exception=0x%08lx address=%p pc=%p base=%p rva=0x%Ix\n",
            info ? info->ExceptionRecord->ExceptionCode : 0,
            info ? info->ExceptionRecord->ExceptionAddress : NULL,
            pc, (void *)base, (size_t)rva);
#if defined(_M_X64) || defined(__x86_64__)
    if (info && info->ContextRecord) {
        CONTEXT *c = info->ContextRecord;
        fprintf(stderr,
                "v8: FATAL regs rax=%016llx rbx=%016llx rcx=%016llx rdx=%016llx rsi=%016llx rdi=%016llx r8=%016llx r9=%016llx r10=%016llx r11=%016llx r12=%016llx r13=%016llx r14=%016llx r15=%016llx rbp=%016llx rsp=%016llx\n",
                (unsigned long long)c->Rax, (unsigned long long)c->Rbx,
                (unsigned long long)c->Rcx, (unsigned long long)c->Rdx,
                (unsigned long long)c->Rsi, (unsigned long long)c->Rdi,
                (unsigned long long)c->R8, (unsigned long long)c->R9,
                (unsigned long long)c->R10, (unsigned long long)c->R11,
                (unsigned long long)c->R12, (unsigned long long)c->R13,
                (unsigned long long)c->R14, (unsigned long long)c->R15,
                (unsigned long long)c->Rbp, (unsigned long long)c->Rsp);
    }
#endif
    return EXCEPTION_EXECUTE_HANDLER;
}

static volatile LONG g_watchdog_stop = 0;

static DWORD WINAPI V8_WatchdogThread(LPVOID unused)
{
    (void)unused;
    extern int g_v8_frame_count;
    int last_frame = -1;
    int last_reported = -1;
    DWORD last_change = GetTickCount();

    while (InterlockedCompareExchange(&g_watchdog_stop, 0, 0) == 0) {
        int frame = g_v8_frame_count;
        DWORD now = GetTickCount();
        if (frame != last_frame) {
            last_frame = frame;
            last_change = now;
        } else if (frame > 0 && now - last_change > 2500 && frame != last_reported) {
            fprintf(stderr, "v8: WATCHDOG frame stalled at %d for %lu ms\n",
                    frame, (unsigned long)(now - last_change));
            fflush(stderr);
            last_reported = frame;
        }
        Sleep(250);
    }
    return 0;
}
#endif

/* From platform/. */
int      Platform_Init(int w, int h, const char *title);
void     Platform_SetHidden(int hidden);
void     Platform_Shutdown(void);
int      Smoke_RunSelfTest(void);
void    *Host_HeapBase(void);
uint32_t Host_HeapSize(void);

/* From v8core. */
void V8_MainLoop(void);
void Heap_Init(V8HeapBlock *base, uint32_t size);

/* CLI options. */
typedef struct {
    int   want_selftest;
    int   want_headless;
    int   max_frames;            /* 0 = unlimited */
    int   auto_drive_frames;     /* 0 = no auto-drive; N = synth forward-input for N frames */
    int   auto_fire_period;      /* 0 = no auto-fire; N = fire every N frames */
    int   versus;                /* Phase 9: enable versus match with match timer */
    int   match_timer;           /* override match timer length in frames (default 1800) */
    const char *replay_path;
    const char *screenshot_path;
    const char *screenshot_series_prefix;  /* save screenshot every N frames */
    int   screenshot_interval;             /* frames between series shots (default 150) */
    int   hidden_render;                    /* create hidden GL context for render-path tests */
    const char *audio_capture_path;
    const char *level_name;
    int   report_heap;
    int   terrain_probe;
    int   vehicle_template_probe;
    int   vehicle_construct_probe;
    int   vehicle_spawn_probe;
    int   vehicle_construct_kind;
} V8Opts;

/* Frame counter the engine can read & check against the cap. */
int g_v8_frame_count = 0;
int g_v8_frame_limit = 0;
int g_v8_auto_drive_frames = 0;
int g_v8_auto_fire_period = 0;
int g_v8_match_timer = 0;       /* if >0, set iRam00000624=1 at this frame count */
int g_v8_vehicle_construct_probe_kind = 5;

/* Optional output paths from CLI. exit handlers consume these. */
const char *g_screenshot_path = NULL;
const char *g_audio_capture_path = NULL;
char g_v8_level_exp_path[128] = "Terrain\\SKIRESRT.EXP";

int Screenshot_Save(const char *path);

static void on_exit_screenshot(void) {
    if (g_screenshot_path) Screenshot_Save(g_screenshot_path);
}

static void archive_previous_run_log(void)
{
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA("v8_latest.log");
    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
        return;

    SYSTEMTIME st;
    char archived[128];
    GetLocalTime(&st);
    snprintf(archived, sizeof(archived),
             "v8_previous_%04u%02u%02u_%02u%02u%02u.log",
             (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
             (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond);
    MoveFileExA("v8_latest.log", archived, MOVEFILE_REPLACE_EXISTING);
#endif
}

static void print_help(void) {
    puts("v8 -- Vigilante 8 recomp\n"
         "Usage: v8 [flags]\n"
         "  --selftest           run deterministic unit checks, exit 0/1\n"
         "  --frames N           cap main-loop iterations to N then exit\n"
         "  --headless           do not open a window\n"
         "  --versus             enable versus match (AI opponent, match timer)\n"
         "  --match-timer N      override match timer length in frames (default 1800)\n"
         "  --replay <path>      feed recorded pad-bytes (phase 4+)\n"
         "  --screenshot <path>  dump backbuffer to PNG on exit (phase 3+)\n"
         "  --screenshot-series <pfx>  save screenshots every N frames as pfx_NNNN.png\n"
         "  --screenshot-interval N    frames between series shots (default 150)\n"
         "  --hidden-render      create a hidden GL context while headless\n"
         "  --audio-capture <p>  dump mixer to WAV (phase 8+)\n"
         "  --level <name>       load TERRAIN/<name>.EXP (default SKIRESRT)\n"
         "  --terrain-probe      headless terrain height scan, then exit\n"
         "  --vehicle-template-probe  headless Vehicles.exp object-template audit, then exit\n"
         "  --vehicle-construct-probe  headless Common.exp Vehicle_Construct audit, then exit\n"
         "  --vehicle-spawn-probe  headless LOAD spawn-record to vehicle audit, then exit\n"
         "  --vehicle-construct-kind N  kind index for construct probe (default 5)\n"
         "  --report-heap        alloc/free balance on exit\n"
         "  -h, --help           this message\n");
}

static int parse_args(int argc, char **argv, V8Opts *o) {
    memset(o, 0, sizeof(*o));
    o->vehicle_construct_kind = -1;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "--selftest"))    o->want_selftest = 1;
        else if (!strcmp(a, "--headless"))    o->want_headless = 1;
        else if (!strcmp(a, "--report-heap")) o->report_heap = 1;
        else if (!strcmp(a, "--frames") && i+1 < argc)        o->max_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--auto-drive") && i+1 < argc)    o->auto_drive_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--auto-fire") && i+1 < argc)     o->auto_fire_period = atoi(argv[++i]);
        else if (!strcmp(a, "--versus"))                             o->versus = 1;
        else if (!strcmp(a, "--match-timer") && i+1 < argc)         o->match_timer = atoi(argv[++i]);
        else if (!strcmp(a, "--screenshot-series") && i+1 < argc)   o->screenshot_series_prefix = argv[++i];
        else if (!strcmp(a, "--screenshot-interval") && i+1 < argc) o->screenshot_interval = atoi(argv[++i]);
        else if (!strcmp(a, "--hidden-render"))                     o->hidden_render = 1;
        else if (!strcmp(a, "--replay") && i+1 < argc)              o->replay_path = argv[++i];
        else if (!strcmp(a, "--screenshot") && i+1 < argc)    o->screenshot_path = argv[++i];
        else if (!strcmp(a, "--audio-capture") && i+1 < argc) o->audio_capture_path = argv[++i];
        else if (!strcmp(a, "--level") && i+1 < argc)          o->level_name = argv[++i];
        else if (!strcmp(a, "--terrain-probe"))                o->terrain_probe = 1;
        else if (!strcmp(a, "--vehicle-template-probe"))       o->vehicle_template_probe = 1;
        else if (!strcmp(a, "--vehicle-construct-probe"))      o->vehicle_construct_probe = 1;
        else if (!strcmp(a, "--vehicle-spawn-probe"))          o->vehicle_spawn_probe = 1;
        else if (!strcmp(a, "--vehicle-construct-kind") && i+1 < argc) o->vehicle_construct_kind = atoi(argv[++i]);
        else if (!strcmp(a, "-h") || !strcmp(a, "--help"))    { print_help(); return -1; }
        else { fprintf(stderr, "unknown arg: %s\n", a); print_help(); return -2; }
    }
    return 0;
}

int main(int argc, char **argv)
{
    /* Force unbuffered stdout/stderr so crash logs don't drop lines. */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    archive_previous_run_log();
    FILE *run_log = fopen("v8_latest.log", "w");
    if (run_log) {
        fprintf(run_log, "v8: run log started\n");
        fflush(run_log);
        freopen("v8_latest.log", "a", stdout);
        freopen("v8_latest.log", "a", stderr);
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
#if defined(_MSC_VER)
    _set_invalid_parameter_handler(V8_InvalidParameterHandler);
    _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDERR);
    _CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
#ifdef _WIN32
    SetUnhandledExceptionFilter(V8_UnhandledExceptionFilter);
#endif
    fprintf(stderr, "v8: logging to v8_latest.log\n");

    V8Opts opts;
    int    pr = parse_args(argc, argv, &opts);
    if (pr == -1) return 0;
    if (pr <  0)  return 2;

    extern int Host_LoadExeDataTables(void);
    Host_LoadExeDataTables();

    if (opts.want_selftest) return Smoke_RunSelfTest();

    if (opts.level_name) {
        const char *name = opts.level_name;
        const char *slash = strrchr(name, '\\');
        const char *slash2 = strrchr(name, '/');
        if (slash2 && (!slash || slash2 > slash)) slash = slash2;
        if (slash) name = slash + 1;

        char bare[64];
        snprintf(bare, sizeof(bare), "%s", name);
        char *dot = strrchr(bare, '.');
        if (dot) *dot = 0;
        snprintf(g_v8_level_exp_path, sizeof(g_v8_level_exp_path),
                 "Terrain\\%s.EXP", bare);
    }

    /* Default: launch the game (windowed, no frame cap, no auto-drive).
     * The user closes the window to exit (SDL_QUIT -> exit(0) in
     * platform_init.c). --help shows the flag reference. */

    g_screenshot_path     = opts.screenshot_path;
    g_audio_capture_path  = opts.audio_capture_path;
    if (g_screenshot_path) atexit(on_exit_screenshot);
    g_v8_auto_drive_frames = opts.auto_drive_frames;
    g_v8_auto_fire_period  = opts.auto_fire_period;
    if (opts.vehicle_construct_kind >= 0) {
        g_v8_vehicle_construct_probe_kind = opts.vehicle_construct_kind;
    }
    if (opts.versus) {
        g_v8_match_timer = opts.match_timer > 0 ? opts.match_timer : 1800;
        fprintf(stderr, "v8: --versus mode, match timer = %d frames\n", g_v8_match_timer);
    }

    /* Screenshot series: expose to pad_shim.c via extern globals. */
    if (opts.screenshot_series_prefix) {
        extern int g_v8_screenshot_interval;
        extern const char *g_v8_screenshot_prefix;
        g_v8_screenshot_prefix   = opts.screenshot_series_prefix;
        g_v8_screenshot_interval = opts.screenshot_interval > 0
                                   ? opts.screenshot_interval : 150;
        fprintf(stderr, "v8: screenshot-series: prefix='%s' interval=%d\n",
                g_v8_screenshot_prefix, g_v8_screenshot_interval);
    }

    /* Audio init -- real device when not headless; WAV capture if asked. */
    extern void Audio_Init(void);
    extern int  Audio_CaptureStart(const char *path);
    extern void Audio_CaptureStop(void);
    if (g_audio_capture_path) {
        Audio_CaptureStart(g_audio_capture_path);
        atexit(Audio_CaptureStop);
    }
    if (!opts.want_headless) Audio_Init();

    /* Sanity check: are the extracted game assets where we expect? */
    {
        FILE *q = fopen("QUEST.BIN", "rb");
        if (!q) {
            fprintf(stderr, "v8: ERROR -- cannot find QUEST.BIN\n");
            fprintf(stderr, "v8:   run v8.exe from the directory containing\n");
            fprintf(stderr, "v8:   QUEST.BIN, TERRAIN/, SHELL/, etc.\n");
            fprintf(stderr, "v8: press Enter to exit...\n");
            getchar();
            return 1;
        }
        fclose(q);
    }

    printf("v8: phase 4 boot (max_frames=%d, headless=%d, level=%s)\n",
           opts.max_frames, opts.want_headless, g_v8_level_exp_path);

    g_v8_frame_limit = opts.max_frames;

    printf("v8: Heap_Init(%p, 0x%x)\n", Host_HeapBase(), Host_HeapSize());
    Heap_Init((V8HeapBlock *)Host_HeapBase(), Host_HeapSize());

    if (opts.vehicle_template_probe) {
        extern int Host_VehicleTemplateProbe(void);
        return Host_VehicleTemplateProbe();
    }
    if (opts.vehicle_construct_probe) {
        extern int Host_VehicleConstructProbe(void);
        return Host_VehicleConstructProbe();
    }

    int platform_started = 0;
    if (!opts.want_headless || opts.screenshot_path || opts.screenshot_series_prefix || opts.hidden_render) {
        char title[192];
        snprintf(title, sizeof(title), "v8 -- engine vehicle -- %s", g_v8_level_exp_path);
        Platform_SetHidden(opts.want_headless || opts.hidden_render);
        if (Platform_Init(1280, 960, title) == 0) {
            platform_started = 1;
        }
    }

    /* Load replay file if requested (replaces SDL keyboard input per-frame). */
    if (opts.replay_path) {
        extern void Replay_Open(const char *path);
        Replay_Open(opts.replay_path);
    }

    /* Verify the generated sin/cos LUT matches the user's EXE byte-
     * for-byte. Diagnoses ROM-revision drift early; quiet success. */
    extern int Host_VerifySinCosTable(void);
    Host_VerifySinCosTable();

    /* Parse the selected terrain EXP and populate DAT_800911a0[] so the engine's
     * Terrain_HeightAt works against real game heightmap data. */
    extern int Host_TerrainLoad(const char *exp_path);
    Host_TerrainLoad(g_v8_level_exp_path);

    {
        extern uint8_t g_terrain_tile_x_min, g_terrain_tile_x_max;
        extern uint8_t g_terrain_tile_z_min, g_terrain_tile_z_max;
        extern void TerrainMesh_LoadCpuOnly(const char *exp_path,
                                            float world_x_centre,
                                            float world_z_centre);
        float mesh_origin_x = ((float)g_terrain_tile_x_min * 64.0f
                             + ((float)g_terrain_tile_x_max + 1.0f) * 64.0f) * 0.5f;
        float mesh_origin_z = ((float)g_terrain_tile_z_min * 64.0f
                             + ((float)g_terrain_tile_z_max + 1.0f) * 64.0f) * 0.5f;
        TerrainMesh_LoadCpuOnly(g_v8_level_exp_path, mesh_origin_x, mesh_origin_z);
    }

    if (opts.terrain_probe) {
        extern uint8_t g_terrain_tile_x_min, g_terrain_tile_x_max;
        extern uint8_t g_terrain_tile_z_min, g_terrain_tile_z_max;
        extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
        extern int TerrainMesh_HeightAt(float wx, float wz, float *out_gl_y);
        extern int TerrainMesh_Bounds(float *out_x0, float *out_x1,
                                      float *out_z0, float *out_z1);
        extern int TerrainMesh_ObstacleHeightAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                                int32_t terrain_y, int32_t *out_y);

        int x0 = ((int)g_terrain_tile_x_min * 64) - 96;
        int x1 = (((int)g_terrain_tile_x_max + 1) * 64) + 96;
        int z0 = ((int)g_terrain_tile_z_min * 64) - 96;
        int z1 = (((int)g_terrain_tile_z_max + 1) * 64) + 96;
        float bx0, bx1, bz0, bz1;
        if (TerrainMesh_Bounds(&bx0, &bx1, &bz0, &bz1)) {
            int mx0 = (int)floorf(bx0) - 16;
            int mx1 = (int)ceilf(bx1) + 16;
            int mz0 = (int)floorf(bz0) - 16;
            int mz1 = (int)ceilf(bz1) + 16;
            if (mx0 < x0) x0 = mx0;
            if (mx1 > x1) x1 = mx1;
            if (mz0 < z0) z0 = mz0;
            if (mz1 > z1) z1 = mz1;
        }
        int n_mesh = 0, n_zone = 0, n_obstacle = 0;
        float mesh_min = 1e30f, mesh_max = -1e30f;
        int32_t zone_min = INT32_MAX, zone_max = INT32_MIN;
        int32_t obstacle_min = INT32_MAX, obstacle_max = INT32_MIN;
        int scan_step = ((x1 - x0) * (z1 - z0) > 300000) ? 16 : 8;
        for (int z = z0; z <= z1; z += scan_step) {
            for (int x = x0; x <= x1; x += scan_step) {
                float gy;
                if (TerrainMesh_HeightAt((float)x, (float)z, &gy)) {
                    if (gy < mesh_min) mesh_min = gy;
                    if (gy > mesh_max) mesh_max = gy;
                    n_mesh++;
                }
                int32_t zy = Terrain_HeightAt((uint32_t)(x << 16),
                                              (uint32_t)(z << 16));
                for (int dy = -0x200000; dy <= 0x200000; dy += 0x10000) {
                    int32_t oy;
                    if (TerrainMesh_ObstacleHeightAt((int32_t)(x << 16),
                                                     zy + dy,
                                                     (int32_t)(z << 16),
                                                     zy, &oy)) {
                        if (oy < obstacle_min) obstacle_min = oy;
                        if (oy > obstacle_max) obstacle_max = oy;
                        n_obstacle++;
                        break;
                    }
                }
                if (zy < zone_min) zone_min = zy;
                if (zy > zone_max) zone_max = zy;
                n_zone++;
            }
        }
        fprintf(stderr,
                "v8: terrain probe %s x=[%d..%d] z=[%d..%d] step=%d\n",
                g_v8_level_exp_path, x0, x1, z0, z1, scan_step);
        if (n_mesh > 0) {
            fprintf(stderr,
                    "v8:   XOBF patch hits=%d gl_y=[%.2f..%.2f] span=%.2fm\n",
                    n_mesh, mesh_min, mesh_max, mesh_max - mesh_min);
        } else {
            fprintf(stderr, "v8:   XOBF patch hits=0\n");
        }
        if (n_obstacle > 0) {
            fprintf(stderr,
                    "v8:   XOBF obstacle hits=%d psx_y=[0x%x..0x%x] gl_y=[%.2f..%.2f]\n",
                    n_obstacle, (unsigned)obstacle_min, (unsigned)obstacle_max,
                    -(float)obstacle_max / 65536.0f,
                    -(float)obstacle_min / 65536.0f);
        } else {
            fprintf(stderr, "v8:   XOBF obstacle hits=0\n");
        }
        fprintf(stderr,
                "v8:   ZONE/table samples=%d psx_y=[0x%x..0x%x] gl_y=[%.2f..%.2f]\n",
                n_zone, (unsigned)zone_min, (unsigned)zone_max,
                -(float)zone_max / 65536.0f, -(float)zone_min / 65536.0f);
        return 0;
    }

    if (opts.vehicle_spawn_probe) {
        extern int Host_VehicleSpawnProbe(void);
        return Host_VehicleSpawnProbe();
    }

#ifdef _WIN32
    HANDLE watchdog = NULL;
    InterlockedExchange(&g_watchdog_stop, 0);
    watchdog = CreateThread(NULL, 0, V8_WatchdogThread, NULL, 0, NULL);
    if (watchdog)
        fprintf(stderr, "v8: watchdog active\n");
#endif

    printf("v8: entering V8_MainLoop\n");
    fflush(stdout);
    V8_MainLoop();
    printf("v8: V8_MainLoop returned (frame=%d)\n", g_v8_frame_count);
#ifdef _WIN32
    if (watchdog) {
        InterlockedExchange(&g_watchdog_stop, 1);
        WaitForSingleObject(watchdog, 1000);
        CloseHandle(watchdog);
    }
#endif

    if (platform_started) Platform_Shutdown();
    printf("v8: phase 2 boot complete\n");
    return 0;
}
