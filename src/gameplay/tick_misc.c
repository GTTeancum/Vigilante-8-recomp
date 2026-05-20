/* tick_misc.c -- small per-frame helpers and resetters.
 *
 * Source: SLUS_005.10
 *   FUN_80011a10 -- Buffer_StartOTagOther     (other parity)
 *   FUN_80015010 -- Tick_GetFrameCounter      (combined RCnt + frame epoch)
 *   FUN_800128bc -- Tick_WaitIrqAck           (spin on iRam00000168)
 *   FUN_80012980 -- Quest_Free                (release loaded Quest.bin)
 *
 * HIGH confidence: each is tiny and unambiguous.
 */
#include <stdint.h>

extern void Buffer_StartOTag(int parity);
extern int  GetRCnt(uint32_t spec);
extern void Heap_Free(void *p);

extern int32_t  iRam00000004;        /* current display parity */
extern uint16_t DAT_800102f2;        /* high-half frame counter (incremented on hi-roll) */
extern volatile int32_t iRam00000168; /* IRQ ack-flag */
extern uintptr_t iRam00000608;        /* loaded Quest.bin pointer */

void Buffer_StartOTagOther(void)
{
    Buffer_StartOTag(1 - iRam00000004);
}

/* Returns a 32-bit absolute frame counter: low 16 bits from the timer
 * counter, high 16 from an epoch advanced by the IRQ handler when the
 * lower counter wraps. */
uint32_t Tick_GetFrameCounter(void)
{
    uint32_t lo = (uint32_t)GetRCnt(0xf2000002);
    return lo | ((uint32_t)DAT_800102f2 << 16);
}

void Tick_WaitIrqAck(void)
{
    while (iRam00000168 == 0) { /* spin until IRQ posts */ }
}

void Quest_Free(void)
{
    if (iRam00000608 != 0) {
        Heap_Free((void *)iRam00000608);
        iRam00000608 = 0;
    }
}
