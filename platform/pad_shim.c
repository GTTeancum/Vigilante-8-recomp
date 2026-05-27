/* pad_shim.c -- SDL keyboard -> engine pad bits + frame cap.
 *
 * Called once per engine game-tick. Reads SDL keys (or --auto-drive
 * synthetic input for headless smoke), writes the bitfield into
 * uRam0000062c (P1 pad) / uRam00000630 (P2 pad). The engine's per-
 * frame handlers consume those globals.
 *
 * Bit layout (see src/skipped/pad_input.c for the full Ghidra-source
 * description):
 *   0x10000000 = Up      (accel)
 *   0x40000000 = Down    (brake/reverse)
 *   0x80000000 = Left    (steer left)
 *   0x20000000 = Right   (steer right)
 *   0x00000100 = Start
 *   0x08000000 = X / machine guns
 *   0x04000000 = Circle / attached weapon fire
 *   0x00100000 = L1 / previous weapon
 *   0x00200000 = R1 / next weapon
 *   0x00400000 = Triangle / next target
 *   0x00000040 = Select / advance-load-screen
 *
 * Replay format (--replay <path>):
 *   Binary stream of uint32_t LE values, one per frame.
 *   Each value is directly written to uRam0000062c for that frame.
 *   When the file is exhausted, falls back to 0 (no input).
 *
 * NO host-side vehicle integration here. The engine's Physics_Step
 * is the source of truth. If the engine sees the pad bits and the
 * vehicle still doesn't move, the bug is downstream, not here.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(V8_HAVE_SDL)
#include <SDL.h>
#endif

extern int      g_v8_frame_count;
extern int      g_v8_frame_limit;
extern int      g_v8_auto_drive_frames;
extern int      g_v8_auto_fire_period;
extern uint32_t uRam0000062c;
extern uint32_t uRam00000630;
extern uint8_t  DAT_80065c28[];
extern void     Platform_PumpEventsOnly(void);
extern void     Platform_FrameTick(void);
extern void     Audio_PumpHeadless(int frames_per_tick);
extern int      Screenshot_Save(const char *path);

uint32_t Host_PadShimTick(void);
extern void FUN_800120d4(void);

uint32_t Host_PadToPhysicsFlags(uint32_t pad, uint32_t prevPad)
{
    uint32_t pressed = pad & ~prevPad;
    uint32_t physicsFlags = 0;

    if (pad & 0x10000000u) physicsFlags |= 0x00000100u;
    if (pad & 0x40000000u) physicsFlags |= 0x00000200u;
    if (pressed & 0x10000000u) physicsFlags |= 0x01000000u;
    if (pressed & 0x40000000u) physicsFlags |= 0x02000000u;
    if (pad & 0x80000000u) physicsFlags |= 0x00001400u;
    if (pad & 0x20000000u) physicsFlags |= 0x00000c00u;
    if (pad & 0x08000000u) physicsFlags |= 0x00040002u;
    if (pad & 0x04000000u) physicsFlags |= 0x00040004u;

    /* Source vehicle_event.c consumes these as one-frame events:
     * 0x80000/0x100000 cycle the active weapon slot, 0x200000 cycles
     * the lock-on target.  The rewritten controls layer only maps keys
     * to source event bits; the weapon/target behavior stays in gameplay. */
    if (pressed & 0x00200000u) physicsFlags |= 0x00080000u;
    if (pressed & 0x00100000u) physicsFlags |= 0x00100000u;
    if (pressed & 0x00400000u) physicsFlags |= 0x00200000u;

    return physicsFlags;
}

static void update_physics_input_lut(uint32_t pad)
{
    static uint32_t prevPad = 0;
    uint32_t physicsFlags = Host_PadToPhysicsFlags(pad, prevPad);
    prevPad = pad;

    *(int16_t  *)(DAT_80065c28 + 0x00) = 2;
    *(uint32_t *)(DAT_80065c28 + 0x08) = physicsFlags;
    *(uint32_t *)(DAT_80065c28 + 0x0c) = 0;
    DAT_80065c28[0x10] = (pad & 0x80000000u) ? 0x40 :
                         (pad & 0x20000000u) ? 0xc0 : 0x80;
    DAT_80065c28[0x11] = (pad & 0x10000000u) ? 0xff : 0x80;
    DAT_80065c28[0x12] = 0x80;
    DAT_80065c28[0x13] = (pad & 0x40000000u) ? 0xff : 0x80;
}

/* Screenshot series state (set by main.c via extern globals). */
int         g_v8_screenshot_interval = 0;  /* frames between screenshots; 0 = disabled */
const char *g_v8_screenshot_prefix   = NULL;

/* 22050 Hz / 60 fps = 367.5 samples per tick (truncated). */
#define AUDIO_FRAMES_PER_TICK 367

/* Replay state. */
static uint32_t *g_replay_buf  = NULL;
static int       g_replay_len  = 0;   /* number of frames in file */
static int       g_replay_done = 0;

void Replay_Open(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "v8: replay -- cannot open '%s'\n", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || (sz & 3)) {
        fprintf(stderr, "v8: replay -- bad file size %ld (must be multiple of 4)\n", sz);
        fclose(f);
        return;
    }
    g_replay_buf = (uint32_t *)malloc((size_t)sz);
    if (!g_replay_buf) {
        fprintf(stderr, "v8: replay -- OOM for %ld bytes\n", sz);
        fclose(f);
        return;
    }
    if (fread(g_replay_buf, 1, (size_t)sz, f) != (size_t)sz) {
        fprintf(stderr, "v8: replay -- short read\n");
        free(g_replay_buf); g_replay_buf = NULL;
        fclose(f);
        return;
    }
    fclose(f);
    g_replay_len = (int)(sz / 4);
    fprintf(stderr, "v8: replay loaded '%s' -- %d frames\n", path, g_replay_len);
}

static uint32_t read_pad_input(void)
{
    uint32_t pad = 0;

    /* Replay takes precedence over keyboard and auto-drive. */
    if (g_replay_buf && !g_replay_done) {
        int idx = g_v8_frame_count;  /* frame count before increment */
        if (idx < g_replay_len) {
            pad = g_replay_buf[idx];
        } else {
            if (!g_replay_done) {
                fprintf(stderr, "v8: replay exhausted at frame %d\n", idx);
                g_replay_done = 1;
            }
        }
        return pad;
    }

#if defined(V8_HAVE_SDL)
    Platform_PumpEventsOnly();
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    if (k) {
        if (k[SDL_SCANCODE_UP]    || k[SDL_SCANCODE_W]) pad |= 0x10000000;
        if (k[SDL_SCANCODE_DOWN]  || k[SDL_SCANCODE_S]) pad |= 0x40000000;
        if (k[SDL_SCANCODE_LEFT]  || k[SDL_SCANCODE_A]) pad |= 0x80000000;
        if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D]) pad |= 0x20000000;
        if (k[SDL_SCANCODE_SPACE])  pad |= 0x08000000;
        if (k[SDL_SCANCODE_LCTRL] || k[SDL_SCANCODE_RCTRL] ||
            k[SDL_SCANCODE_LSHIFT] || k[SDL_SCANCODE_RSHIFT]) pad |= 0x04000000;
        if (k[SDL_SCANCODE_Q])      pad |= 0x00100000;
        if (k[SDL_SCANCODE_E])      pad |= 0x00200000;
        if (k[SDL_SCANCODE_TAB])    pad |= 0x00400000;
        if (k[SDL_SCANCODE_RETURN]) pad |= 0x00000100;
    }
#endif

    /* Auto-drive: synthetic accel for the first N frames (headless smoke). */
    if (g_v8_auto_drive_frames > 0 && g_v8_frame_count < g_v8_auto_drive_frames) {
        pad |= 0x10000000;
    }
    if (g_v8_auto_fire_period > 0 &&
        (g_v8_frame_count % g_v8_auto_fire_period) == 0) {
        pad |= 0x08000000 | 0x04000000;
    }
    return pad;
}

void Pad_Tick(void)
{
    FUN_800120d4();
}

uint32_t Host_PadShimTick(void)
{
    static uint32_t prev_logged_pad = 0;
    static int transition_logs = 0;

    g_v8_frame_count++;

    uint32_t pad = read_pad_input();
    if (pad != prev_logged_pad && transition_logs < 64) {
        fprintf(stderr, "v8: pad transition @%d pad=0x%08x\n",
                g_v8_frame_count, pad);
        prev_logged_pad = pad;
        transition_logs++;
    }
    uRam0000062c = pad;
    uRam00000630 = 0;
    update_physics_input_lut(pad);

    Platform_FrameTick();

    /* Screenshot series: save every N frames to prefix_NNN.png */
    if (g_v8_screenshot_interval > 0 && g_v8_screenshot_prefix &&
        (g_v8_frame_count % g_v8_screenshot_interval) == 0) {
        char sspath[512];
        snprintf(sspath, sizeof(sspath), "%s_%04d.png",
                 g_v8_screenshot_prefix, g_v8_frame_count);
        Screenshot_Save(sspath);
    }

    /* Headless audio: pump mixer output into WAV capture if active.
     * In windowed mode, miniaudio's device callback handles this. */
    Audio_PumpHeadless(AUDIO_FRAMES_PER_TICK);

    if (g_v8_frame_limit > 0 && g_v8_frame_count >= g_v8_frame_limit) {
        fprintf(stderr, "v8: frame cap %d reached; exit\n", g_v8_frame_limit);
        fflush(stderr);
        exit(0);
    }
    return pad;
}

void Pad_TickWithMode(int mode)
{
    (void)mode;
}
