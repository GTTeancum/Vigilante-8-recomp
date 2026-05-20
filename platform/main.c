/* main.c -- entry point for the v8 recomp binary. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "structs.h"

/* From platform/. */
int      Platform_Init(int w, int h, const char *title);
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
    const char *replay_path;
    const char *screenshot_path;
    const char *audio_capture_path;
    int   report_heap;
} V8Opts;

/* Frame counter the engine can read & check against the cap. */
int g_v8_frame_count = 0;
int g_v8_frame_limit = 0;
int g_v8_auto_drive_frames = 0;
int g_v8_auto_fire_period = 0;

/* Optional output paths from CLI. exit handlers consume these. */
const char *g_screenshot_path = NULL;
const char *g_audio_capture_path = NULL;

int Screenshot_Save(const char *path);

static void on_exit_screenshot(void) {
    if (g_screenshot_path) Screenshot_Save(g_screenshot_path);
}

static void print_help(void) {
    puts("v8 -- Vigilante 8 recomp\n"
         "Usage: v8 [flags]\n"
         "  --selftest           run deterministic unit checks, exit 0/1\n"
         "  --frames N           cap main-loop iterations to N then exit\n"
         "  --headless           do not open a window\n"
         "  --replay <path>      feed recorded pad-bytes (phase 4+)\n"
         "  --screenshot <path>  dump backbuffer to PNG on exit (phase 3+)\n"
         "  --audio-capture <p>  dump mixer to WAV (phase 8+)\n"
         "  --report-heap        alloc/free balance on exit\n"
         "  -h, --help           this message\n");
}

static int parse_args(int argc, char **argv, V8Opts *o) {
    memset(o, 0, sizeof(*o));
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "--selftest"))    o->want_selftest = 1;
        else if (!strcmp(a, "--headless"))    o->want_headless = 1;
        else if (!strcmp(a, "--report-heap")) o->report_heap = 1;
        else if (!strcmp(a, "--frames") && i+1 < argc)        o->max_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--auto-drive") && i+1 < argc)    o->auto_drive_frames = atoi(argv[++i]);
        else if (!strcmp(a, "--auto-fire") && i+1 < argc)     o->auto_fire_period = atoi(argv[++i]);
        else if (!strcmp(a, "--replay") && i+1 < argc)        o->replay_path = argv[++i];
        else if (!strcmp(a, "--screenshot") && i+1 < argc)    o->screenshot_path = argv[++i];
        else if (!strcmp(a, "--audio-capture") && i+1 < argc) o->audio_capture_path = argv[++i];
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

    V8Opts opts;
    int    pr = parse_args(argc, argv, &opts);
    if (pr == -1) return 0;
    if (pr <  0)  return 2;

    if (opts.want_selftest) return Smoke_RunSelfTest();

    /* Default: launch the game (windowed, no frame cap, no auto-drive).
     * The user closes the window to exit (SDL_QUIT -> exit(0) in
     * platform_init.c). --help shows the flag reference. */

    g_screenshot_path     = opts.screenshot_path;
    g_audio_capture_path  = opts.audio_capture_path;
    if (g_screenshot_path) atexit(on_exit_screenshot);
    g_v8_auto_drive_frames = opts.auto_drive_frames;
    g_v8_auto_fire_period  = opts.auto_fire_period;

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
        FILE *q = fopen("input/QUEST.BIN", "rb");
        if (!q) {
            fprintf(stderr, "v8: ERROR -- cannot find input/QUEST.BIN\n");
            fprintf(stderr, "v8:   run v8.exe from the project root so the\n");
            fprintf(stderr, "v8:   relative path 'input/' resolves correctly.\n");
            fprintf(stderr, "v8:   (current working dir matters!)\n");
            fprintf(stderr, "v8: press Enter to exit...\n");
            getchar();
            return 1;
        }
        fclose(q);
    }

    printf("v8: phase 4 boot (max_frames=%d, headless=%d)\n",
           opts.max_frames, opts.want_headless);

    g_v8_frame_limit = opts.max_frames;

    printf("v8: Heap_Init(%p, 0x%x)\n", Host_HeapBase(), Host_HeapSize());
    Heap_Init((V8HeapBlock *)Host_HeapBase(), Host_HeapSize());

    if (!opts.want_headless) Platform_Init(640, 480, "v8 -- engine vehicle");

    /* Parse OILFIELD.EXP and populate DAT_800911a0[] so the engine's
     * Terrain_HeightAt works against real game heightmap data. */
    extern int Host_TerrainLoad(const char *exp_path);
    Host_TerrainLoad("Terrain\\OilField.EXP");

    /* Allocate the player Vehicle in the engine heap and set
     * puRam000007d0. After this the engine's Physics_Step (in
     * physics_shim.c) can advance real vehicle state via the cleaned
     * Object_IntegrateAndOrient. */
    extern void Host_VehicleInit(void);
    Host_VehicleInit();

    printf("v8: entering V8_MainLoop\n");
    fflush(stdout);
    V8_MainLoop();
    printf("v8: V8_MainLoop returned (frame=%d)\n", g_v8_frame_count);

    if (!opts.want_headless) Platform_Shutdown();
    printf("v8: phase 2 boot complete\n");
    return 0;
}
