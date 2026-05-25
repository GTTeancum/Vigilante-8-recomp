/* hud_reticle.c -- Vehicle targeting reticle and HUD slot state machine.
 *
 * Source: SLUS_005.10
 *   FUN_80011f8c  VehicleAnim_GetInputState(slot)       (~17 instr)
 *   FUN_80012050  HudSlot_SetTypeByte(idx, val)          (~7 instr)
 *   FUN_80012088  HudReticle_PackBits(mask,_,p3,p4)      (~11 instr, loop)
 *   FUN_800120d4  HudReticle_Tick()                      (~100 instr)
 *   FUN_8001265c  HudReticle_Reset()                     (~6 instr)
 *   FUN_800126c8  HudLock_Free()                         (~6 instr)
 *   FUN_800126f0  HudReticle_Update()                    (~2 instr, wrapper)
 *
 * FUN_80011f8c:
 *   Looks up the vehicle animation config table (DAT_80066458, stride 0x22)
 *   for slot param_1, returns a state index 0..5 based on the type byte.
 *
 * FUN_80012088:
 *   Iterates the low 16 bits of param_1 (right-shifting by 1 each step).
 *   For each set bit, ORs (param_1 & 0x10001) << (param3 & 0xf) into result.
 *   param3/param4 form a 64-bit value shifted right by 4 each iteration.
 *   param2 is passed but unused (PSX register artifact).
 *
 * FUN_800120d4:
 *   Main per-frame reticle update.  Advances bRam00000614 (animation slot
 *   counter, 0..7), reads the current vehicle animation config entry, builds
 *   a 32-bit button state mask (uVar9), optionally advances the replay/record
 *   sequence, then updates both HUD reticle slots (uVar5 = 0, 1) writing the
 *   animation state, packed bit result, and colour look-up-table entries into
 *   the DAT_80065c28 state block.
 *
 * FUN_8001265c:
 *   If mode is 2 (record), terminates the sequence by writing 0xffff at the
 *   current sequence pointer.  Resets iRam00000618 to 0.
 *
 * FUN_800126c8:
 *   Frees the lock-on data buffer (iRam00000634) via FUN_80045088.
 *
 * FUN_800126f0:
 *   Thin wrapper -- just calls FUN_800120d4.
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>
#include "gte.h"

/* Ghidra CONCAT11(a,b): a -> high byte, b -> low byte of a uint16_t */
#define CONCAT11(a,b) ((uint16_t)(((uint8_t)(a) << 8) | (uint8_t)(b)))

/* ---- data tables --------------------------------------------------------- */

/* DAT_80066458: vehicle animation config table. stride 0x22 per slot.
 * byte 0: type (0=none, 'A'=0x41, '#'=0x23, 'S'=0x53, 's'=0x73).
 * At least 8 entries (indexed by param_1 0..7). */
extern uint8_t DAT_80066458[];    /* 8 * 0x22 = 272 bytes; zero-filled stub */

/* DAT_8006ecb8: targeting reticle animation entry table. stride 0x44 per slot.
 * slot[0x00] : flag byte (0xff = inactive)
 * slot[0x01] : type byte ('#' or other)
 * slot[0x02..03]: u16 lo-16 of state mask (CONCAT11 big-endian)
 * slot[0x05..07]: animation colour params
 * slot[0x22] : flag byte for hi-16 ('#' or other)
 * slot[0x24..25]: u16 hi-16 of state mask
 * ... additional per-frame colour bytes
 * 8 entries x 0x44 bytes = 544 bytes; zero-filled stub */
extern uint8_t DAT_8006ecb8[];

/* DAT_80065930: per-slot animation base data (uint32_t[2]).
 * Element 0 = slot 0 entry; element 1 = slot 1 entry. */
extern uint32_t DAT_80065930[];

/* DAT_80056774 / DAT_80056778: animation data tables (byte-addressed).
 * Accessed as: *(uint32_t *)(&table + *psVar11 * 8 + iVar12)
 * where *psVar11 = 0..5, iVar12 = 0 or 0x30. Max offset ~88 bytes.
 * DAT_80056774 is 4 bytes before DAT_80056778 in PSX memory. */
extern uint8_t DAT_80056774[];   /* >=92 bytes; zero-filled stub */
extern uint8_t DAT_80056778[];   /* >=92 bytes; zero-filled stub */

/* DAT_800567d4: stride look-up table (uint16_t[]), byte-addressed.
 * Accessed as *(uint16_t *)(&DAT_800567d4 + *psVar11 * 2 + iVar14 * 4)
 * Max offset: 5*2 + 3*4 = 22 bytes. */
extern uint8_t DAT_800567d4[];   /* >=24 bytes; zero-filled stub */

/* DAT_80065940: HUD damage-flash table (already defined in hud_damage_flash.c).
 * Bytes [3 + uVar5*8] are written by FUN_800120d4. */
extern uint8_t DAT_80065940[];

/* DAT_80065c28: HUD reticle slot state block (uint8_t[0x40]).
 * Layout per 0x18-byte slot:
 *   +0x00 (int16_t): animation state index
 *   +0x02 (int16_t): countdown timer
 *   +0x04 (uint32_t): (reserved)
 *   +0x08 (uint32_t): packed-bits result
 *   +0x0c (uint32_t): animation table entry copy
 *   +0x10 (uint32_t): current RGBA colour
 *   +0x14 (uint32_t): previous RGBA colour
 * 2 slots × 0x18 = 0x30 bytes, plus 0x10 bytes of colour overrides. */
extern uint8_t DAT_80065c28[];

/* DAT_80065c58..DAT_80065f58: colour LUT tables A..D.
 * Indexed by (pcVar10[4..7] + iVar8) where iVar8 = 0 or 0x400, pcVar10[n] 0..255.
 * Maximum index: 0x400 + 255 = 1279. 4 tables × 1280 bytes = 5120 bytes. */
extern uint8_t DAT_80065c58[];   /* colour LUT A; zero-filled stub */
extern uint8_t DAT_80065d58[];   /* colour LUT B; zero-filled stub */
extern uint8_t DAT_80065e58[];   /* colour LUT C; zero-filled stub */
extern uint8_t DAT_80065f58[];   /* colour LUT D; zero-filled stub */

/* ---- scalar globals ------------------------------------------------------ */

/* psRam00000610: pointer to current replay/record sequence entry (short *).
 * Points into a sequence buffer; advanced per-frame in replay/record modes. */
extern int16_t *psRam00000610;

/* bRam00000601: target animation slot index.
 * bRam00000614: current animation slot index (chases bRam00000601, 0..7). */
extern int8_t   bRam00000601;
extern int8_t   bRam00000614;

/* sRam0000061e: per-frame countdown for replay mode. */
extern int16_t  sRam0000061e;

/* iRam00000618: reticle mode (0=free, 1=replay, 2=record). */
extern int32_t  iRam00000618;

/* iRam00000634: lock-on buffer handle (0 = not allocated). */
extern int32_t  iRam00000634;

/* uRam000005d4: previous frame state mask (for edge-detect). */
extern uint32_t uRam000005d4;

/* uRam0000062c / uRam00000630: button-down / button-up edge masks. */
extern uint32_t uRam0000062c;
extern uint32_t uRam00000630;
extern uint32_t Host_PadShimTick(void);

/* FUN_80045088: Heap_Free (frees a buffer by handle). */
extern void FUN_80045088(int handle);

/* ================================================================
 * FUN_80011f8c -- VehicleAnim_GetInputState
 *
 * Returns an animation state index (0..5) for vehicle slot param_1
 * based on the type byte at DAT_80066458[slot * 0x22 + 1]:
 *   '#' (0x23) -> 3
 *   'A' (0x41) -> 2
 *   'S' (0x53) -> 4
 *   's' (0x73) -> 5
 *   other      -> 1
 * Returns 0 if the first byte of the entry is non-zero (slot inactive).
 * ================================================================ */
int16_t FUN_80011f8c(int param_1)
{
    uint8_t bVar1;

    if (DAT_80066458[param_1 * 0x22] != '\0') {
        return 0;
    }
    bVar1 = DAT_80066458[param_1 * 0x22 + 1];
    if (bVar1 == 0x53) {
        return 4;
    }
    if (bVar1 < 0x54) {
        if (bVar1 == 0x23) {
            return 3;
        }
        if (bVar1 == 0x41) {
            return 2;
        }
    } else if (bVar1 == 0x73) {
        return 5;
    }
    return 1;
}

/* ================================================================
 * FUN_80012050 -- HudSlot_SetTypeByte
 *
 * Writes param_2 into byte [0] of DAT_80065944 entry idx,
 * where DAT_80065944 = DAT_80065940 + 4 (byte 4 of each 8-byte slot).
 * ================================================================ */
void FUN_80012050(int param_1, uint8_t param_2)
{
    /* DAT_80065944 = &DAT_80065940[4]; element stride = 8 bytes */
    DAT_80065940[4 + (unsigned)param_1 * 8] = param_2;
}

/* ================================================================
 * FUN_80012088 -- HudReticle_PackBits
 *
 * Iterates the low 16 bits of param_1 (right-shifting 1 per step).
 * On each step ORs (param_1 & 0x10001) << (param_3 & 0xf) into result.
 * param_3/param_4 form a 64-bit value, right-shifted by 4 each step.
 * param_2 is unused (PSX register artifact -- passed in but ignored).
 * ================================================================ */
uint32_t FUN_80012088(uint32_t param_1, uint32_t param_2,
                      uint32_t param_3, uint32_t param_4)
{
    uint32_t uVar1;
    (void)param_2;   /* unused -- PSX calling-convention artefact */

    uVar1 = 0;
    for (; (param_1 & 0xffffu) != 0; param_1 = param_1 >> 1) {
        uVar1 = uVar1 | (param_1 & 0x10001u) << (param_3 & 0xfu);
        param_3 = param_3 >> 4 | param_4 << 0x1c;
        param_4 = param_4 >> 4;
    }
    return uVar1;
}

/* ================================================================
 * FUN_800120d4 -- HudReticle_Tick
 *
 * Per-frame reticle update.  Advances bRam00000614 toward bRam00000601
 * (mod 8), reads the animation config entry, builds a 32-bit state
 * mask, optionally processes replay/record sequence, then writes the
 * results into both HUD reticle slots (0 and 1) in DAT_80065c28.
 * ================================================================ */
void FUN_800120d4(void)
{
    int16_t *psVar1;
    int      bVar2;
    uint16_t uVar3;
    int16_t  sVar4;
    uint32_t uVar5;
    int      iVar6;
    uint32_t uVar7;
    int      iVar8;
    uint32_t uVar9;
    uint8_t *pcVar10;
    int16_t *psVar11;
    int      iVar12;
    int      iVar13;
    int      iVar14;

    psVar11 = psRam00000610;
    if (bRam00000601 != bRam00000614) {
        bRam00000614 = (int8_t)(((uint8_t)bRam00000614 + 1u) & 7u);
    }
    iVar8 = (uint32_t)(uint8_t)bRam00000614 * 0x44;
    pcVar10 = &DAT_8006ecb8[iVar8];

    /* Build lo-16 of state mask from entry bytes 0..3 */
    uVar9 = 0xffffu;
    if (pcVar10[0] != (uint8_t)(-1)) {
        uVar9 = (uint32_t)CONCAT11(DAT_8006ecb8[iVar8 + 2],
                                    DAT_8006ecb8[iVar8 + 3]);
    }
    /* Build hi-16 of state mask from entry bytes 0x22..0x25 */
    if (DAT_8006ecb8[iVar8 + 0x22] == (uint8_t)(-1)) {
        uVar5 = 0xffff0000u;
    } else {
        uVar5 = (uint32_t)CONCAT11(DAT_8006ecb8[iVar8 + 0x24],
                                    DAT_8006ecb8[iVar8 + 0x25]) << 0x10;
    }
    uVar9 = ~(uVar5 | uVar9);

    /* Augment mask from lo-group extra bits */
    if (DAT_8006ecb8[iVar8 + 1] == '#') {
        uVar5 = (uint32_t)(DAT_8006ecb8[iVar8 + 5] == (uint8_t)(-1)) << 6;
        if (DAT_8006ecb8[iVar8 + 7] == (uint8_t)(-1)) {
            uVar5 = uVar5 | 4u;
        }
        if (DAT_8006ecb8[iVar8 + 6] == (uint8_t)(-1)) {
            uVar9 = uVar9 | 0x80u | uVar5;
        } else {
            uVar9 = uVar9 | uVar5;
        }
    }
    /* Augment mask from hi-group extra bits */
    if (DAT_8006ecb8[iVar8 + 0x23] == '#') {
        uVar5 = (uint32_t)(DAT_8006ecb8[iVar8 + 0x27] == (uint8_t)(-1)) << 0x16;
        if (DAT_8006ecb8[iVar8 + 0x29] == (uint8_t)(-1)) {
            uVar5 = uVar5 | 0x40000u;
        }
        if (DAT_8006ecb8[iVar8 + 0x28] == (uint8_t)(-1)) {
            uVar9 = uVar9 | 0x800000u | uVar5;
        } else {
            uVar9 = uVar9 | uVar5;
        }
    }

    /* Replay mode: advance sequence, substitute mask from sequence entry */
    if (iRam00000618 == 1) {
        sRam0000061e = sRam0000061e - 1;
        if (sRam0000061e == -1) {
            sRam0000061e = psRam00000610[2];
            psRam00000610 = psRam00000610 + 2;
        }
        bVar2 = uVar9 != 0;
        uVar9 = (uint32_t)(uint16_t)psRam00000610[1];
        if (bVar2) {
            uVar9 = (uint16_t)psRam00000610[1] | 0x800u;
        }
    }
    /* Record mode: append new entry if state changed or at end of buffer */
    else if (iRam00000618 == 2) {
        if ((psRam00000610 == (int16_t *)(uintptr_t)(uint32_t)(iRam00000634 + 8)) ||
            ((uint32_t)(uint16_t)psRam00000610[1] != (uVar9 & 0xffffu))) {
            psVar1 = psRam00000610 + 3;
            psRam00000610 = psRam00000610 + 2;
            *psVar1 = (int16_t)uVar9;
            psVar11[2] = 0;
        } else {
            *psRam00000610 = *psRam00000610 + 1;
        }
    }

    /* Controls are a rewrite seam: sample host input at the same point the
     * original polls the pad, then let source gameplay consume the original
     * globals/state block below. */
    uint32_t hostPad = Host_PadShimTick();

    /* Compute edge masks and latch current state */
    uVar5  = 0;
    iVar8  = 0;
    iVar14 = 0;
    iVar12 = 0;
    iVar13 = 0;
    uRam0000062c = (uVar9 & 0xffffu) | ((uVar9 & ~uRam000005d4) << 0x10);
    uRam00000630 = (uVar9 >> 0x10) | (uVar9 & ~uRam000005d4 & 0xffff0000u);
    uRam000005d4 = uVar9;

    /* Update both HUD reticle slots (uVar5 = 0, 1) */
    do {
        psVar11 = (int16_t *)(DAT_80065c28 + iVar13);

        /* Determine animation state index */
        if (iRam00000618 == 0) {
            sVar4 = FUN_80011f8c((int)uVar5);
        } else {
            sVar4 = 2;
        }
        *psVar11 = sVar4;

        /* Copy animation entry and write derived flag byte */
        *(uint32_t *)(DAT_80065c28 + 0x0c + iVar13) = DAT_80065930[uVar5];

        /* Write leading-zero flag byte into HUD flash table slot */
        DAT_80065940[3 + uVar5 * 8] =
            (uint8_t)((*(uint32_t *)(&DAT_80056778[(uint32_t)(int16_t)*psVar11 * 8 + iVar12]) >> 0xc) & 1u);

        /* Pack bits from animation data */
        iVar6 = (int16_t)*psVar11 * 8 + iVar12;
        uVar7 = FUN_80012088(
            DAT_80065930[uVar5] & 0xf7fff7ffu,
            (uint32_t)(uintptr_t)(&DAT_80065940[uVar5 * 8]),
            *(uint32_t *)(&DAT_80056774[iVar6]),
            *(uint32_t *)(&DAT_80056778[iVar6]));
        *(uint32_t *)(DAT_80065c28 + 0x08 + iVar13) = uVar7;

        /* Update the animation-frame countdown / LZC */
        uVar9 = *(uint32_t *)(DAT_80065c28 + 0x0c + iVar13) & 0xf0000000u;
        if (uVar9 == 0) {
            sVar4 = *(int16_t *)(DAT_80065c28 + 0x02 + iVar13);
            if ((*(int16_t *)(DAT_80065c28 + 0x02 + iVar13) != 0) &&
                (*(int16_t *)(DAT_80065c28 + 0x02 + iVar13) = sVar4 - 1, sVar4 == 1)) {
                *(uint32_t *)(DAT_80065c28 + 0x04 + iVar13) = 0;
            }
        } else {
            gte_ldLZCS((int32_t)(uVar9 >> 1));
            uVar9 = (uint32_t)gte_stLZCR();
            *(uint32_t *)(DAT_80065c28 + 0x04 + iVar13) =
                (*(uint32_t *)(DAT_80065c28 + 0x04 + iVar13) << 4) | uVar9;
            *(uint16_t *)(DAT_80065c28 + 0x02 + iVar13) = 0x14u;
        }

        /* Write colour LUT entries if animation state > 2 */
        if (2 < (int16_t)*psVar11) {
            uVar3 = *(uint16_t *)(&DAT_800567d4[(int16_t)*psVar11 * 2 + iVar14 * 4]);
            uVar7 = *(uint32_t *)(DAT_80065c28 + 0x10 + iVar13);
            *(uint32_t *)(DAT_80065c28 + 0x10 + iVar13) = 0x80808080u;
            *(uint32_t *)(DAT_80065c28 + 0x14 + iVar13) = uVar7;
            if ((uVar3 & 0xfu) != 0) {
                ((uint8_t *)psVar11)[(uVar3 & 0xfu) + 0xf] =
                    DAT_80065c58[(uint32_t)(uint8_t)pcVar10[4] + iVar8];
            }
            if ((uVar3 >> 4 & 0xfu) != 0) {
                ((uint8_t *)psVar11)[(uVar3 >> 4 & 0xfu) + 0xf] =
                    DAT_80065d58[(uint32_t)(uint8_t)pcVar10[5] + iVar8];
            }
            if ((uVar3 >> 8 & 0xfu) != 0) {
                ((uint8_t *)psVar11)[(uVar3 >> 8 & 0xfu) + 0xf] =
                    DAT_80065e58[(uint32_t)(uint8_t)pcVar10[6] + iVar8];
            }
            if ((uVar3 >> 0xc) != 0) {
                ((uint8_t *)psVar11)[(uVar3 >> 0xc) + 0xf] =
                    DAT_80065f58[(uint32_t)(uint8_t)pcVar10[7] + iVar8];
            }
        }

        iVar8  += 0x400;
        pcVar10 += 0x22;
        iVar14 += 3;
        iVar12 += 0x30;
        uVar5  += 1;
        iVar13 += 0x18;
    } while (uVar5 < 2);

    uRam0000062c = hostPad;
    uRam00000630 = 0;

    static uint32_t prevHostPad = 0;
    uint32_t pressedHostPad = hostPad & ~prevHostPad;
    uint32_t physicsFlags = 0;
    if (hostPad & 0x10000000u) physicsFlags |= 0x00000100u;
    if (hostPad & 0x40000000u) physicsFlags |= 0x00000200u;
    if (pressedHostPad & 0x10000000u) physicsFlags |= 0x01000000u;
    if (pressedHostPad & 0x40000000u) physicsFlags |= 0x02000000u;
    if (hostPad & 0x80000000u) physicsFlags |= 0x00001400u;
    if (hostPad & 0x20000000u) physicsFlags |= 0x00000c00u;
    if (hostPad & 0x08000000u) physicsFlags |= 0x00040000u;
    prevHostPad = hostPad;

    *(int16_t  *)(DAT_80065c28 + 0x00) = 2;
    *(uint32_t *)(DAT_80065c28 + 0x08) = physicsFlags;
    *(uint32_t *)(DAT_80065c28 + 0x0c) = 0;
    DAT_80065c28[0x10] = (hostPad & 0x80000000u) ? 0x40 :
                         (hostPad & 0x20000000u) ? 0xc0 : 0x80;
    DAT_80065c28[0x11] = (hostPad & 0x10000000u) ? 0xff : 0x80;
    DAT_80065c28[0x12] = 0x80;
    DAT_80065c28[0x13] = (hostPad & 0x40000000u) ? 0xff : 0x80;
}

/* ================================================================
 * FUN_8001265c -- HudReticle_Reset
 *
 * If mode is record (2), terminates the sequence buffer by writing
 * 0xffff at the current sequence pointer.  Resets mode to 0.
 * ================================================================ */
void FUN_8001265c(void)
{
    if (iRam00000618 == 2) {
        *(uint16_t *)psRam00000610 = 0xffffu;
    }
    iRam00000618 = 0;
}

/* ================================================================
 * FUN_800126c8 -- HudLock_Free
 *
 * If iRam00000634 (lock-on buffer) is non-zero, frees it via
 * FUN_80045088 and clears the handle.
 * ================================================================ */
void FUN_800126c8(void)
{
    if (iRam00000634 != 0) {
        FUN_80045088(iRam00000634);
    }
    iRam00000634 = 0;
}

/* ================================================================
 * FUN_800126f0 -- HudReticle_Update
 *
 * Thin wrapper -- calls FUN_800120d4.
 * ================================================================ */
void FUN_800126f0(void)
{
    FUN_800120d4();
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80011f8c  (from analysis/SLUS_005.10/decomp/80011f8c.c) --- */
// addr: 0x80011f8c  name: FUN_80011f8c

undefined4 FUN_80011f8c(int param_1)

{
  byte bVar1;

  if ((&DAT_80066458)[param_1 * 0x22] != '\0') {
    return 0;
  }
  bVar1 = (&DAT_80066459)[param_1 * 0x22];
  if (bVar1 == 0x53) {
    return 4;
  }
  if (bVar1 < 0x54) {
    if (bVar1 == 0x23) {
      return 3;
    }
    if (bVar1 == 0x41) {
      return 2;
    }
  }
  else if (bVar1 == 0x73) {
    return 5;
  }
  return 1;
}

/* --- SLUS_005.10 FUN_80012050  (from analysis/SLUS_005.10/decomp/80012050.c) --- */
// addr: 0x80012050  name: FUN_80012050

void FUN_80012050(int param_1,undefined1 param_2)

{
  (&DAT_80065944)[param_1 * 8] = param_2;
  return;
}

/* --- SLUS_005.10 FUN_80012088  (from analysis/SLUS_005.10/decomp/80012088.c) --- */
// addr: 0x80012088  name: FUN_80012088

uint FUN_80012088(uint param_1,undefined4 param_2,uint param_3,uint param_4)

{
  uint uVar1;

  uVar1 = 0;
  for (; (param_1 & 0xffff) != 0; param_1 = param_1 >> 1) {
    uVar1 = uVar1 | (param_1 & 0x10001) << (param_3 & 0xf);
    param_3 = param_3 >> 4 | param_4 << 0x1c;
    param_4 = param_4 >> 4;
  }
  return uVar1;
}

/* --- SLUS_005.10 FUN_800120d4  (from analysis/SLUS_005.10/decomp/800120d4.c) --- */
// addr: 0x800120d4  name: FUN_800120d4

void FUN_800120d4(void)

{
  short *psVar1;
  bool bVar2;
  ushort uVar3;
  short sVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  uint uVar9;
  char *pcVar10;
  short *psVar11;
  int iVar12;
  int iVar13;
  int iVar14;

  psVar11 = psRam00000610;
  if (bRam00000601 != bRam00000614) {
    bRam00000614 = bRam00000614 + 1 & 7;
  }
  iVar8 = (uint)bRam00000614 * 0x44;
  pcVar10 = &DAT_8006ecb8 + iVar8;
  uVar9 = 0xffff;
  if (*pcVar10 != -1) {
    uVar9 = (uint)CONCAT11((&DAT_8006ecba)[iVar8],(&DAT_8006ecbb)[iVar8]);
  }
  if ((&DAT_8006ecda)[iVar8] == -1) {
    uVar5 = 0xffff0000;
  }
  else {
    uVar5 = (uint)CONCAT11((&DAT_8006ecdc)[iVar8],(&DAT_8006ecdd)[iVar8]) << 0x10;
  }
  uVar9 = ~(uVar5 | uVar9);
  if ((&DAT_8006ecb9)[iVar8] == '#') {
    uVar5 = (uint)((&DAT_8006ecbd)[iVar8] == -1) << 6;
    if ((&DAT_8006ecbf)[iVar8] == -1) {
      uVar5 = uVar5 | 4;
    }
    if ((&DAT_8006ecbe)[iVar8] == -1) {
      uVar9 = uVar9 | 0x80 | uVar5;
    }
    else {
      uVar9 = uVar9 | uVar5;
    }
  }
  if ((&DAT_8006ecdb)[iVar8] == '#') {
    uVar5 = (uint)((&DAT_8006ecdf)[iVar8] == -1) << 0x16;
    if ((&DAT_8006ece1)[iVar8] == -1) {
      uVar5 = uVar5 | 0x40000;
    }
    if ((&DAT_8006ece0)[iVar8] == -1) {
      uVar9 = uVar9 | 0x800000 | uVar5;
    }
    else {
      uVar9 = uVar9 | uVar5;
    }
  }
  if (iRam00000618 == 1) {
    sRam0000061e = sRam0000061e + -1;
    if (sRam0000061e == -1) {
      sRam0000061e = psRam00000610[2];
      psRam00000610 = psRam00000610 + 2;
    }
    bVar2 = uVar9 != 0;
    uVar9 = (uint)(ushort)psRam00000610[1];
    if (bVar2) {
      uVar9 = (ushort)psRam00000610[1] | 0x800;
    }
  }
  else if (iRam00000618 == 2) {
    if ((psRam00000610 == (short *)(iRam00000634 + 8)) ||
       ((uint)(ushort)psRam00000610[1] != (uVar9 & 0xffff))) {
      psVar1 = psRam00000610 + 3;
      psRam00000610 = psRam00000610 + 2;
      *psVar1 = (short)uVar9;
      psVar11[2] = 0;
    }
    else {
      *psRam00000610 = *psRam00000610 + 1;
    }
  }
  uVar5 = 0;
  iVar8 = 0;
  iVar14 = 0;
  iVar12 = 0;
  iVar13 = 0;
  uRam0000062c = uVar9 & 0xffff | (uVar9 & ~uRam000005d4) << 0x10;
  uRam00000630 = uVar9 >> 0x10 | uVar9 & ~uRam000005d4 & 0xffff0000;
  uRam000005d4 = uVar9;
  do {
    psVar11 = (short *)((int)&DAT_80065c28 + iVar13);
    if (iRam00000618 == 0) {
      sVar4 = FUN_80011f8c(uVar5);
    }
    else {
      sVar4 = 2;
    }
    *psVar11 = sVar4;
    *(undefined4 *)((int)&DAT_80065c34 + iVar13) = (&DAT_80065930)[uVar5];
    (&DAT_80065943)[uVar5 * 8] = (byte)(*(uint *)(&DAT_80056778 + *psVar11 * 8 + iVar12) >> 0xc) & 1
    ;
    iVar6 = *psVar11 * 8 + iVar12;
    uVar7 = FUN_80012088((&DAT_80065930)[uVar5] & 0xf7fff7ff,&DAT_80065940 + uVar5 * 8,
                         *(undefined4 *)(&DAT_80056774 + iVar6),
                         *(undefined4 *)(&DAT_80056778 + iVar6));
    *(undefined4 *)((int)&DAT_80065c30 + iVar13) = uVar7;
    uVar9 = *(uint *)((int)&DAT_80065c34 + iVar13) & 0xf0000000;
    if (uVar9 == 0) {
      sVar4 = *(short *)((int)&DAT_80065c2a + iVar13);
      if ((*(short *)((int)&DAT_80065c2a + iVar13) != 0) &&
         (*(short *)((int)&DAT_80065c2a + iVar13) = sVar4 + -1, sVar4 == 1)) {
        *(undefined4 *)((int)&DAT_80065c2c + iVar13) = 0;
      }
    }
    else {
      gte_ldLZCS(uVar9 >> 1);
      uVar9 = gte_stLZCR();
      *(uint *)((int)&DAT_80065c2c + iVar13) = *(int *)((int)&DAT_80065c2c + iVar13) << 4 | uVar9;
      *(undefined2 *)((int)&DAT_80065c2a + iVar13) = 0x14;
    }
    if (2 < *psVar11) {
      uVar3 = *(ushort *)(&DAT_800567d4 + *psVar11 * 2 + iVar14 * 4);
      uVar7 = *(undefined4 *)((int)&DAT_80065c38 + iVar13);
      *(undefined4 *)((int)&DAT_80065c38 + iVar13) = 0x80808080;
      *(undefined4 *)((int)&DAT_80065c3c + iVar13) = uVar7;
      if ((uVar3 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 & 0xf) + 0xf) =
             (&DAT_80065c58)[(uint)(byte)pcVar10[4] + iVar8];
      }
      if ((uVar3 >> 4 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 4 & 0xf) + 0xf) =
             (&DAT_80065d58)[(uint)(byte)pcVar10[5] + iVar8];
      }
      if ((uVar3 >> 8 & 0xf) != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 8 & 0xf) + 0xf) =
             (&DAT_80065e58)[(uint)(byte)pcVar10[6] + iVar8];
      }
      if (uVar3 >> 0xc != 0) {
        *(undefined *)((int)psVar11 + (uVar3 >> 0xc) + 0xf) =
             (&DAT_80065f58)[(uint)(byte)pcVar10[7] + iVar8];
      }
    }
    iVar8 = iVar8 + 0x400;
    pcVar10 = pcVar10 + 0x22;
    iVar14 = iVar14 + 3;
    iVar12 = iVar12 + 0x30;
    uVar5 = uVar5 + 1;
    iVar13 = iVar13 + 0x18;
  } while (uVar5 < 2);
  return;
}

/* --- SLUS_005.10 FUN_8001265c  (from analysis/SLUS_005.10/decomp/8001265c.c) --- */
// addr: 0x8001265c  name: FUN_8001265c

void FUN_8001265c(void)

{
  if (iRam00000618 == 2) {
    *puRam00000610 = 0xffff;
  }
  iRam00000618 = 0;
  return;
}

/* --- SLUS_005.10 FUN_800126c8  (from analysis/SLUS_005.10/decomp/800126c8.c) --- */
// addr: 0x800126c8  name: FUN_800126c8

void FUN_800126c8(void)

{
  if (iRam00000634 != 0) {
    FUN_80045088();
  }
  iRam00000634 = 0;
  return;
}

/* --- SLUS_005.10 FUN_800126f0  (from analysis/SLUS_005.10/decomp/800126f0.c) --- */
// addr: 0x800126f0  name: FUN_800126f0

void FUN_800126f0(void)

{
  FUN_800120d4();
  return;
}

#endif  /* GHIDRA REF */
