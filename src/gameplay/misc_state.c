/* misc_state.c -- small state resetters from the main loop's teardown path.
 *
 * Source: SLUS_005.10  FUN_8001265c, FUN_800126c8, FUN_800126f0.
 *
 * - IntroFlag_Reset (FUN_8001265c): zero out the intro / demo phase flag.
 *   The auxiliary write `*puRam00000610 = 0xffff` when the flag was 2
 *   appears to clear a UI sprite reference back to "blank".
 * - CharSelData_Free (FUN_800126c8): release the character-select data
 *   buffer if loaded.
 * - PadTick (FUN_800126f0): trivial wrapper around FUN_800120d4
 *   (pad poll / VSync tick). Pass 2 may inline this site.
 *
 * HIGH confidence (mechanical).
 */
#include <stdint.h>

extern void  Heap_Free(void *p);
extern void  Pad_Tick(void);          /* FUN_800120d4 -- still in auto/ */
extern int32_t   iRam00000618;
extern int32_t   iRam00000634;
extern uint16_t *puRam00000610;
extern void     *DAT_80012634_data;

void IntroFlag_Reset(void)
{
    if (iRam00000618 == 2) *puRam00000610 = 0xffff;
    iRam00000618 = 0;
}

void CharSelData_Free(void)
{
    if (iRam00000634 != 0) Heap_Free((void *)iRam00000634);
    iRam00000634 = 0;
}

void Tick_PadOnly(void)
{
    Pad_Tick();
}
