/* frame.c -- VSync wait / frame counter helpers.
 *
 * Source: SLUS_005.10
 *   FUN_800156d4 -- V8_WaitVsync.  Spins until the IRQ-incremented counter
 *                   (g_vsyncCounter @ iRam000006a8) differs from the
 *                   last-latched value (g_vsyncLatched @ iRam000006a4),
 *                   then latches and returns the prior latched value.
 *   FUN_8001714c -- V8_SeedRng (see src/gameplay/rng.c)
 *
 * V8 ticks at the standard NTSC 60Hz. The IRQ handler at LAB_80014ff0
 * (installed by OpenEvent in main()) increments g_vsyncCounter every
 * field. Game code calls V8_WaitVsync at the end of each tick to
 * advance.
 *
 * Bit-exact note: a busy-wait spin is preserved verbatim. Replacing
 * it with a host sleep/condvar would shift timing observably in
 * code that polls g_vsyncCounter without going through this helper.
 */
#include <stdint.h>

extern volatile int32_t g_vsyncCounter;   /* iRam000006a8, updated by IRQ */
extern volatile int32_t g_vsyncLatched;   /* iRam000006a4 */

int32_t V8_WaitVsync(void)
{
    int32_t prev = g_vsyncLatched;
    while (g_vsyncCounter == g_vsyncLatched) {
        /* spin */
    }
    g_vsyncLatched = g_vsyncCounter;
    return prev;
}
