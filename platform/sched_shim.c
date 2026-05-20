/* sched_shim.c -- frame scheduler shims.
 *
 * V8 has two scheduler entry points used during boot:
 *   Sched_BeginFrame -- one-shot per outer-loop iteration (irrelevant here)
 *   Sched_WaitFrame  -- blocking wait for next vsync. Polled in shell
 *                       loops, the load-screen loop, the result-screen
 *                       hold, etc.
 *
 * On host we treat each Sched_WaitFrame as a "tick" for --frames N
 * counting. When the cap is reached we force the wait condition
 * (Start button) so the engine exits its hold loop and continues to
 * the next phase rather than spinning forever.
 *
 * Phase 2 just needs to NOT spin. Phase 4+ wires SDL key state into
 * the pad bits properly.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int g_v8_frame_count;
extern int g_v8_frame_limit;
extern uint32_t uRam0000062c;   /* pad bits for player 1 */
extern uint32_t uRam00000630;   /* pad bits for player 2 */

void Sched_WaitFrame(void)
{
    g_v8_frame_count++;
    if (g_v8_frame_limit > 0 && g_v8_frame_count >= g_v8_frame_limit) {
        /* Force the "Start pressed" bit so any wait-on-start loop
         * exits, and the "any button" bit so prompts dismiss. */
        uRam0000062c = 0x0000ffff;
        uRam00000630 = 0x0000ffff;
        if (g_v8_frame_count % 1000 == 0)
            fprintf(stderr, "v8: frame cap %d reached; forcing pad bits\n",
                    g_v8_frame_limit);
    }
    if (g_v8_frame_count > g_v8_frame_limit + 600 && g_v8_frame_limit > 0) {
        /* Engine refused to exit -- hard cap. */
        fprintf(stderr, "v8: HARD CAP at %d frames (limit was %d); exit\n",
                g_v8_frame_count, g_v8_frame_limit);
        exit(0);
    }
}

void Sched_BeginFrame(void)
{
    /* No-op on host. */
}
