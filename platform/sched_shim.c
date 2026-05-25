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
#if defined(V8_HAVE_SDL)
#include <SDL.h>
#endif

extern uint32_t uRam0000062c;   /* pad bits for player 1 */
extern uint32_t uRam00000630;   /* pad bits for player 2 */

/* Separate counter: Sched_WaitFrame fires during shell wait loops
 * BEFORE the inner game-tick loop reaches Pad_Tick. We don't want
 * to burn the --frames budget here. After 240 wait spins we force
 * the start bit so the engine continues. */
static int g_wait_count = 0;

void Sched_WaitFrame(void)
{
    g_wait_count++;
    if (g_wait_count >= 240) {
        /* Force the "any button" bits so wait-on-input loops release. */
        uRam0000062c = 0x0000ffff;
        uRam00000630 = 0x0000ffff;
    }
    if (g_wait_count > 2000) {
        fprintf(stderr, "v8: WAIT LOOP HANG (%d spins); exiting\n", g_wait_count);
        exit(2);
    }
    /* Yield the CPU during wait-loops so we don't spin-lock. */
#if defined(V8_HAVE_SDL)
    SDL_Delay(1);
#endif
}

void Sched_BeginFrame(void)
{
    /* No-op on host. */
}
