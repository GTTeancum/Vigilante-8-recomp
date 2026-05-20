/* stub_misc.c -- the smallest one-liner glue functions.
 *
 * Source: SLUS_005.10
 *   FUN_800116ec  -- C runtime placeholder (empty).
 *   FUN_80040e18  -- Heap_Free wrapper.
 *   FUN_8003d898  -- ScreenHalf_Configure(obj): toggle obj+0x8c / +0x90
 *                   based on g_splitScreenMode (1P vs 2P horizontal).
 *
 * HIGH confidence (each is trivial).
 */
#include <stdint.h>

extern void Heap_Free(void *p);
extern int32_t iRam00000010;   /* g_splitScreenMode */

void Bootstrap_C_Init(void) { /* deliberately empty -- matches binary */ }

void FreeHelper_80040e18(void *p) { Heap_Free(p); }

void ScreenHalf_Configure(uint8_t *obj)
{
    /* +0x8c i16: aspect-correct screen half y origin (0xff00 = -256, 0xff60 = -160).
     * +0x90 i16: half-height (0xa0=160 for 1P, 0x50=80 for 2P horizontal). */
    int twoP = (iRam00000010 == 1);
    *(int16_t *)(obj + 0x8c) = twoP ? (int16_t)0xff60 : (int16_t)0xff00;
    *(int16_t *)(obj + 0x90) = twoP ? (int16_t)0x50   : (int16_t)0xa0;
}
