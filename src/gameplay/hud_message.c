/* hud_message.c -- Centred HUD text display helper.
 *
 * Source: SLUS_005.10
 *   FUN_800129e8  HudMsg_ShowCentred(slot, str)  (~15 instr)
 *
 * Computes the centred X coordinate for a text string in the current
 * view width, then positions and triggers the HUD text element at
 * DAT_8006eef0[slot * 0x84].
 *
 * param_1 (slot):
 *   0 -- centre-screen message (X = 0x140, Y = 0x78 or 0x98 in 4-player)
 *   n -- player-n sub-viewport (X = iRam000006d8/2, Y = iRam000006dc/2)
 *
 * FUN_800191e0(uRam00000628) -- Font_TextWidth: returns pixel width of the
 *   queued string (caller pre-queues via FUN_80053004 / sprintf).
 * FUN_80019cbc(&slot_base, str, x, y) -- Font_DrawQueued: renders text at (x,y).
 *
 * HIGH confidence: small function, control-flow straightforward.
 */
#include <stdint.h>

extern int32_t  FUN_800191e0(uint32_t font);   /* Font_TextWidth */
extern void     FUN_80019cbc(void *slot, uint32_t str, int x, int y);  /* Font_DrawQueued */

extern uint32_t uRam00000628;    /* font handle */
extern int32_t  iRam00000010;    /* game mode: 0=in-game, 2=4-player */
extern int32_t  iRam000006d8;    /* screen width  (pixels) */
extern int32_t  iRam000006dc;    /* screen height (pixels) */

extern uint32_t DAT_8006eef0;    /* first word of HUD element table, stride 0x84 bytes */

/* ================================================================
 * FUN_800129e8 -- HudMsg_ShowCentred
 * ================================================================ */
void FUN_800129e8(int param_1, uint32_t param_2)
{
    int iVar1, iVar2, iVar3;

    iVar1 = FUN_800191e0(uRam00000628);
    iVar2 = 0x140;
    if (param_1 != 0) {
        iVar2 = iRam000006d8;
    }
    if (param_1 == 0) {
        iVar3 = 0x78;
        if (iRam00000010 == 2) {
            iVar3 = 0x98;
        }
    } else {
        iVar3 = iRam000006dc / 2;
    }
    FUN_80019cbc((uint8_t *)&DAT_8006eef0 + param_1 * 0x84,
                 param_2,
                 (iVar2 - iVar1) / 2,
                 iVar3);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800129e8  (from analysis/SLUS_005.10/decomp/800129e8.c) --- */
// addr: 0x800129e8  name: FUN_800129e8

void FUN_800129e8(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;

  iVar1 = FUN_800191e0(uRam00000628);
  iVar2 = 0x140;
  if (param_1 != 0) {
    iVar2 = iRam000006d8;
  }
  if (param_1 == 0) {
    iVar3 = 0x78;
    if (iRam00000010 == 2) {
      iVar3 = 0x98;
    }
  }
  else {
    iVar3 = iRam000006dc / 2;
  }
  FUN_80019cbc(&DAT_8006eef0 + param_1 * 0x84,param_2,(iVar2 - iVar1) / 2,iVar3);
  return;
}

#endif  /* GHIDRA REF */
