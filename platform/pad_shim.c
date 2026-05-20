/* pad_shim.c -- pad input stubs + frame-cap enforcement.
 *
 * Pad_Tick is called once per game-tick in the main loop. We use it
 * as a single point to:
 *   1. Count frames against the --frames N cap
 *   2. Read SDL keyboard into uRam0000062c / uRam00000630 (Phase 4)
 *
 * For Phase 2 we just count + force-exit when the cap is hit. The
 * pad bits stay 0 unless Sched_WaitFrame (which fires in inner wait
 * loops, before this inner game-tick loop) needed to set them.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int      g_v8_frame_count;
extern int      g_v8_frame_limit;
extern uint32_t uRam0000062c;
extern uint32_t uRam00000630;

void Pad_Tick(void)
{
    g_v8_frame_count++;
    if (g_v8_frame_limit > 0 && g_v8_frame_count >= g_v8_frame_limit) {
        fprintf(stderr, "v8: frame cap %d reached in game-tick loop; exit\n",
                g_v8_frame_limit);
        fflush(stderr);
        exit(0);
    }
    /* Phase 2: pad bits stay 0 (no input) so the AI/level just sits. */
}

void Pad_TickWithMode(int mode)
{
    (void)mode;
    /* Sub-tick variant of Pad_Tick used by the main loop's per-tick
     * second pass. Don't double-count frames. */
}
