/* timer_tick.c -- Frame tick counter read.
 *
 * Source: SLUS_005.10  FUN_80015010.
 *
 * Returns the 32-bit running game time as:
 *   (PSX RCnt2 low 16 bits) | (IRQ-maintained high 16 bits << 16)
 * The high byte is incremented by V8_TimerIRQ at 60 Hz.
 * On host GetRCnt always returns 0; the counter advances only via
 * the IRQ byte, giving a 16-bit frame counter in the high half.
 *
 * Used by the A* pathfinder (FUN_80024d54) to enforce a per-frame
 * time budget on the search.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "psyq/psyq_stubs.h"

/* High 16 bits of the tick counter maintained by V8_TimerIRQ.
 * Declared as byte at EXE address 0x800102f2 (PSY-Q RCnt overflow byte). */
extern uint8_t DAT_800102f2;

/* FUN_80015010 -- GetFrameTickCount */
uint32_t FUN_80015010(void)
{
    uint32_t uVar1 = (uint32_t)GetRCnt(0xf2000002);
    return uVar1 | ((uint32_t)DAT_800102f2 << 16);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80015010  (from analysis/SLUS_005.10/decomp/80015010.c) --- */
// addr: 0x80015010  name: FUN_80015010

uint FUN_80015010(void)

{
  uint uVar1;

  uVar1 = GetRCnt(0xf2000002);
  return uVar1 | (uint)DAT_800102f2 << 0x10;
}

#endif  /* GHIDRA REF */
