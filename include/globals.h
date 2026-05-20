#ifndef V8_GLOBALS_H
#define V8_GLOBALS_H

#include <stdint.h>
#include "fixed.h"
#include "structs.h"

/* Recovered globals for Vigilante 8 (PSX SLUS_005.10).
 * Pass 1: addresses from Ghidra-exported data symbols, types and names
 * inferred from cross-function usage. Confidence tags per DECOMP_RULES.md.
 *
 * Address comments give the original RAM VA in the binary; the port should
 * keep these as plain C globals -- physical placement no longer matters.
 */

/* ---- Match / shell state ------------------------------------------------- */

/* @ 0x80000015 (bRam00000015) -- V8MatchMode. */
extern uint8_t   g_matchMode;          /* MED */

/* @ 0x80000010 (iRam00000010) -- split-screen / single-player flag, 0=1P, 2=2P */
extern int32_t   g_splitScreenMode;    /* LOW */

/* @ 0x80000008 (iRam00000008) -- current draw buffer index (0 or 1) */
extern int32_t   g_drawBufIndex;       /* HIGH: standard PSX double-buffer toggle */

/* @ 0x80000004 (iRam00000004) -- current display buffer index */
extern int32_t   g_dispBufIndex;       /* HIGH */

/* @ 0x80000018 / 0x80000019 (cRam00000018/19) -- display offsets X,Y */
extern int8_t    g_dispOffsetX;        /* LOW */
extern int8_t    g_dispOffsetY;        /* LOW */

/* @ 0x80065674 (DAT_80065674[8]) -- player slot indices (8 entries, 8 bytes apart) */
extern uint8_t   g_playerSlotIdx[8];   /* LOW */

/* @ 0x800007d0 / 0x800007d4 -- player 1 / player 2 vehicle pointer.
 * NULL means "no player in slot". */
extern Vehicle  *g_player1Vehicle;     /* MED @ 0x800007d0 */
extern Vehicle  *g_player2Vehicle;     /* MED @ 0x800007d4 */

/* @ 0x800006a4 (iRam000006a4) / 0x800006a8 (iRam000006a8) -- VSync IRQ counter
 * (a4 = last latched, a8 = IRQ-incremented) */
extern volatile int32_t g_vsyncLatched;  /* HIGH @ 0x800006a4 */
extern volatile int32_t g_vsyncCounter;  /* HIGH @ 0x800006a8 -- written by IRQ handler */

/* @ 0x80000000 (iRam00000000) -- end of EXE BSS section, used as heap base */
extern uint32_t  g_heapBaseTop;        /* HIGH: == &_end of EXE */

/* @ 0x8005ed4c / 0x8005ed50 -- heap free-list head / tail pointer */
extern V8HeapBlock *g_heapFreeList;    /* HIGH */
extern V8HeapBlock *g_heapFreeBase;    /* HIGH */

/* @ 0x80065c30 / 0x80065c28 / 0x80065c40 -- match config flags / option bits */
extern uint32_t  g_matchConfigFlags;   /* LOW @ 0x80065c30 */
extern int32_t   g_optionFlag_c28;     /* LOW */
extern int32_t   g_optionFlag_c40;     /* LOW */

/* RNG seed -- the function FUN_8001714c(seed) sets a single global seed.
 * Address inside the function (LW from a fixed VA) reveals the seed location.
 * Pass 2 must confirm. Provisional name: */
extern int32_t   g_rngSeed;            /* LOW */

#endif /* V8_GLOBALS_H */
