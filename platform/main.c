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
    int   max_frames;     /* 0 = unlimited */
    const char *replay_path;
    const char *screenshot_path;
    const char *audio_capture_path;
    int   report_heap;
} V8Opts;

/* Frame counter the engine can read & check against the cap. */
int g_v8_frame_count = 0;
int g_v8_frame_limit = 0;

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

    /* No args: print help and exit (avoid accidentally launching the
     * infinite V8_MainLoop when run with no flags). */
    if (opts.max_frames == 0 && !opts.want_headless && !opts.replay_path) {
        print_help();
        return 0;
    }

    g_screenshot_path = opts.screenshot_path;
    if (g_screenshot_path) atexit(on_exit_screenshot);

    printf("v8: phase 2 boot (max_frames=%d, headless=%d)\n",
           opts.max_frames, opts.want_headless);

    g_v8_frame_limit = opts.max_frames;

    printf("v8: Heap_Init(%p, 0x%x)\n", Host_HeapBase(), Host_HeapSize());
    Heap_Init((V8HeapBlock *)Host_HeapBase(), Host_HeapSize());

    if (!opts.want_headless) Platform_Init(640, 480, "v8 -- phase 2");

    printf("v8: entering V8_MainLoop\n");
    fflush(stdout);
    V8_MainLoop();
    printf("v8: V8_MainLoop returned (frame=%d)\n", g_v8_frame_count);

    if (!opts.want_headless) Platform_Shutdown();
    printf("v8: phase 2 boot complete\n");
    return 0;
}
