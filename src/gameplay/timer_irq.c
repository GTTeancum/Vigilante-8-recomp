/* timer_irq.c -- RCnt2 interrupt handler.
 *
 * Source: SLUS_005.10  FUN_80014ff0 (LAB_80014ff0).
 *
 * The 60Hz timer IRQ handler installed by V8_Main via
 * OpenEvent(0xf2000002, 2, ...). Increments the high-half of the
 * global frame counter (DAT_800102f2) each time the lower RCnt
 * wraps. The full 32-bit frame counter is reconstructed by
 * Tick_GetFrameCounter as (RCnt) | (DAT_800102f2 << 16).
 *
 * HIGH confidence (3-line IRQ stub).
 */
#include <stdint.h>

extern uint16_t DAT_800102f2;

void V8_TimerIRQ(void)
{
    DAT_800102f2++;
}
