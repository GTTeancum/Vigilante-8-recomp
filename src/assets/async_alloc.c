/* async_alloc.c -- stop an active asynchronous-allocation pipeline.
 *
 * Source: SLUS_005.10  FUN_800165cc.
 *
 * The game sometimes runs a VSync-callback-driven allocator: a single
 * 24 KiB-ish state block lives at iRam000006bc, with:
 *   +0x5dcc  -- completion flag (set by the callback when done)
 *   +0x5dd0  -- the VSync callback function pointer
 *
 * Async_StopAllocCallback(wait):
 *   - if wait == 0, spins until +0x5dcc is non-zero (waits for the
 *     in-flight allocation to complete naturally);
 *   - re-arms the original VSync callback (`VSyncCallback(*ptr)`),
 *     swaps the now-idle parity's draw env back into place, frees
 *     the state block, clears the global pointer and the OT
 *     interlock fields.
 *
 * HIGH confidence: lock-and-tear-down sequence with clear field offsets.
 */
#include <stdint.h>

extern void  VSyncCallback(void (*cb)(void));
extern void  PutDrawEnv(void *drawEnv);
extern void  Heap_Free(void *p);
extern uintptr_t iRam000006bc;       /* state block base, or 0 */
extern int32_t  iRam00000004;        /* current display parity */
extern uint8_t  DAT_8006f208[];       /* draw env table (0x5c per parity) */
extern uint32_t DAT_8006f220, DAT_8006f27c;

void Async_StopAllocCallback(int wait)
{
    if (iRam000006bc == 0) return;

    if (wait == 0) {
        volatile int32_t *done = (volatile int32_t *)(iRam000006bc + 0x5dcc);
        while (*done == 0) { /* spin */ }
    }

    VSyncCallback(*(void (**)(void))(iRam000006bc + 0x5dd0));
    PutDrawEnv(DAT_8006f208 + (1 - iRam00000004) * 0x5c);
    Heap_Free((void *)iRam000006bc);
    iRam000006bc = 0;
    DAT_8006f27c  = 0;
    DAT_8006f220  = 0;
}
