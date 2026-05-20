/* main_entry.c -- the program entry point.
 *
 * Source: SLUS_005.10  FUN_80015098.
 *
 * Called from the binary's `start` symbol after the BSS zero-fill.
 * Cold-boots the whole game:
 *   1. Bootstrap_C_Init  -- (empty stub; reserved for crt0)
 *   2. Heap_Init         -- arena from end-of-BSS to 0x80115b0e8,
 *                           rounded down to 8 bytes.
 *   3. Boot_Init         -- CdInit + read PVD + cache + walk root dir.
 *   4. ResetGraph(0), SetGraphDebug(0), InitGeom() -- PSY-Q libgs.
 *   5. Latch the current root-counter value as the seed for the
 *      timer-IRQ-driven RNG counter (V8_SeedRng(GetRCnt(...))).
 *   6. SetRCnt(0xf2000002, 0xffff, 0x1000) -- enable RCnt mode 2 IRQ
 *      with the maximum wrap counter and IRQ-on-overflow.
 *   7. Audio_Init -- SPU bring-up (out of scope).
 *   8. OpenEvent / EnableEvent on RCnt2 with the timer-IRQ handler
 *      (LAB_80014ff0 -- the per-field 60Hz tick).
 *   9. CD_FetchTOC (now-named Audio_RefreshTOC).
 *  10. V8_MainLoop (FUN_80013cac, still in auto/) -- never returns.
 *
 * Returns 0 but the call never falls through to the return; the main
 * loop is an infinite do-while.
 */
#include <stdint.h>

extern void  Bootstrap_C_Init(void);            /* FUN_800116ec */
extern void  Heap_Init(void *base, uint32_t size);
extern void  Boot_Init(void);                   /* FUN_80015e8c */
extern void  ResetGraph(int mode);
extern void  SetGraphDebug(int level);
extern void  InitGeom(void);
extern int   GetRCnt(uint32_t spec);
extern void  V8_SeedRng(int32_t seed);
extern void  SetRCnt(uint32_t spec, uint16_t wrap, uint16_t mode);
extern void  Audio_Init(void);                  /* FUN_80043ef0 -- skipped */
extern int   OpenEvent(uint32_t cls, int spec, int mode, void (*cb)(void));
extern void  EnableEvent(int eid);
extern void  Audio_RefreshTOC(void);            /* FUN_80053a24 / 80053a34 -- maps to FUN_80043a74 */
extern void  V8_MainLoop(void);                 /* FUN_80013cac */

extern uint32_t iRam00000000;                   /* end-of-BSS marker */
extern void     V8_TimerIRQ(void);              /* LAB_80014ff0 */
extern uint8_t  DAT_800a4f18[];                 /* heap arena base */

#define HEAP_TOP   0x115b0e8u                   /* hard-coded top in the binary */

int V8_Main(void)
{
    Bootstrap_C_Init();
    Heap_Init(DAT_800a4f18, (HEAP_TOP - iRam00000000) & 0xfffffff8u);
    Boot_Init();
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();

    int32_t rcnt = GetRCnt(0xf2000002);
    V8_SeedRng(rcnt);
    SetRCnt(0xf2000002, 0xffff, 0x1000);

    Audio_Init();
    int eid = OpenEvent(0xf2000002, 2, 0x1000, V8_TimerIRQ);
    EnableEvent(eid);
    Audio_RefreshTOC();

    V8_MainLoop();        /* never returns */
    return 0;
}
