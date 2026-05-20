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
 *   0x08000000 = X / fire
 *   0x00000040 = Select / advance-load-screen
 *
 * NO host-side vehicle integration here. The engine's Physics_Step
 * is the source of truth. If the engine sees the pad bits and the
 * vehicle still doesn't move, the bug is downstream, not here.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(V8_HAVE_SDL)
#include <SDL.h>
#endif

extern int      g_v8_frame_count;
extern int      g_v8_frame_limit;
extern int      g_v8_auto_drive_frames;
extern uint32_t uRam0000062c;
extern uint32_t uRam00000630;
extern void     Platform_FrameTick(void);

static uint32_t read_pad_input(void)
{
    uint32_t pad = 0;

#if defined(V8_HAVE_SDL)
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    if (k) {
        if (k[SDL_SCANCODE_UP]    || k[SDL_SCANCODE_W]) pad |= 0x10000000;
        if (k[SDL_SCANCODE_DOWN]  || k[SDL_SCANCODE_S]) pad |= 0x40000000;
        if (k[SDL_SCANCODE_LEFT]  || k[SDL_SCANCODE_A]) pad |= 0x80000000;
        if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D]) pad |= 0x20000000;
        if (k[SDL_SCANCODE_SPACE])  pad |= 0x08000000;
        if (k[SDL_SCANCODE_RETURN]) pad |= 0x00000100;
    }
#endif

    /* Auto-drive: synthetic accel for the first N frames (headless smoke). */
    if (g_v8_auto_drive_frames > 0 && g_v8_frame_count < g_v8_auto_drive_frames) {
        pad |= 0x10000000;
    }
    return pad;
}

void Pad_Tick(void)
{
    g_v8_frame_count++;

    uint32_t pad = read_pad_input();
    uRam0000062c = pad;
    uRam00000630 = 0;

    Platform_FrameTick();

    if (g_v8_frame_limit > 0 && g_v8_frame_count >= g_v8_frame_limit) {
        fprintf(stderr, "v8: frame cap %d reached; exit\n", g_v8_frame_limit);
        fflush(stderr);
        exit(0);
    }
}

void Pad_TickWithMode(int mode)
{
    (void)mode;
}
