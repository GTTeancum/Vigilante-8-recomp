/* bone_anim.c -- Bone animation channel helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8001b1f8  BoneAnim_FindInputChan(obj)         (~16 instr)
 *   FUN_8001b3d4  BoneAnim_TouchSlot(chain, idx)       (~18 instr)
 *   FUN_8001b270  BoneAnim_TouchInputChan(obj)         (~28 instr)
 *
 * FUN_8001b1f8:
 *   Searches the bone chain of the template pointed to by obj[0x16]
 *   (at obj+0x58) for the first bone record whose high nibble is 0xb
 *   (input-mapped animation type).  The chain root index is read from
 *   the slot's own bone entry at offset 0x36 (a u16 sibling/child
 *   link). Returns the matching ushort* record, or NULL.
 *
 * FUN_8001b3d4:
 *   Increments the reference count (short at slot*12+12 from chain
 *   base) for the given animation slot idx.  If the count just
 *   became 1 (first touch), calls FUN_800187e4 (Font_DecodeFNT) to
 *   initialise the slot's internal data from the template table at
 *   *chain+0x14.
 *
 * FUN_8001b270:
 *   Combines both: finds the input-channel bone for obj, looks up
 *   the channel's animation-table index, touches the slot via
 *   FUN_8001b3d4, then sets bit 0x20 in the slot's flags word (+8).
 *   PSX calling-convention note: Ghidra shows FUN_8001b1f8 called
 *   with no explicit args from FUN_8001b270, but MIPS a0 (= param_1
 *   of the caller) is passed unchanged -- correct call is
 *   FUN_8001b1f8(param_1).
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>

/* Dependencies */
extern void  FUN_800187e4(uint32_t src, void *dst);   /* Font_DecodeFNT */
extern uint16_t *FUN_8001b49c(int *bank, uint32_t key);
extern int16_t sRam000005e4;
extern uint8_t bRam000006cf;

static inline uintptr_t obj_ptr32(uintptr_t base, uint32_t off)
{
    return (uintptr_t)*(uint32_t *)(base + off);
}

/* ================================================================
 * FUN_8001b1f8 -- BoneAnim_FindInputChan
 * ================================================================ */
ushort *FUN_8001b1f8(int param_1)
{
    ushort  uVar1;
    ushort *puVar2;
    uintptr_t obj = (uintptr_t)param_1;
    uintptr_t bank = obj_ptr32(obj, 0x58);
    uintptr_t tmpl = obj_ptr32(bank, 0);

    uVar1 = *(ushort *)(tmpl + (uint)*(ushort *)(obj + 10) * 0x1c + 0x36);
    while (1) {
        if (uVar1 == 0xffff) {
            return (ushort *)0x0;
        }
        puVar2 = (ushort *)(tmpl + (uint)uVar1 * 0x1c + 0x1c);
        if (*puVar2 >> 0xc == 0xb) break;
        uVar1 = puVar2[0xc];
    }
    return puVar2;
}

/* ================================================================
 * FUN_8001b3d4 -- BoneAnim_TouchSlot
 * ================================================================ */
int *FUN_8001b3d4(int *param_1, uint param_2)
{
    short  sVar1;
    int   *piVar2;

    piVar2 = param_1 + (param_2 & 0xffffu) * 3 + 3;
    sVar1  = (short)*piVar2 + 1;
    *(short *)piVar2 = sVar1;
    if (sVar1 == 1) {
        uintptr_t tmpl = (uintptr_t)*(uint32_t *)param_1;
        FUN_800187e4(
            *(uint32_t *)((uintptr_t)*(uint32_t *)(tmpl + 0x14)
                          + (param_2 & 0xffffu) * 4),
            piVar2);
    }
    return piVar2;
}

/* ================================================================
 * FUN_8001b270 -- BoneAnim_TouchInputChan
 * ================================================================ */
int FUN_8001b270(int param_1)
{
    ushort *puVar1;
    uintptr_t slot;
    uintptr_t obj = (uintptr_t)param_1;
    uintptr_t bank = obj_ptr32(obj, 0x58);
    uintptr_t tmpl = obj_ptr32(bank, 0);

    /* PSX: a0 unchanged from caller -- pass param_1 explicitly */
    puVar1 = FUN_8001b1f8(param_1);
    if (puVar1 == (ushort *)0x0) {
        slot = 0;
    } else {
        uintptr_t chan = (uintptr_t)*(uint32_t *)((uintptr_t)*(uint32_t *)(tmpl + 4)
                                                 + (*puVar1 & 0xfffu) * 4);
        slot = (uintptr_t)FUN_8001b3d4(
                    (int *)bank,
                    (uint)((int)*(short *)(chan + 0x12)
                           + (int)(*(ushort *)((uintptr_t)*(uint32_t *)(chan + 0x14) + 0x12)
                                   & 0x3fffu)));
        *(ushort *)(slot + 8) = *(ushort *)(slot + 8) | 0x20u;
    }
    return (int)slot;
}

int FUN_8001b0c4(uint32_t *param_1)
{
    uintptr_t obj = (uintptr_t)param_1;
    int *bank = (int *)(uintptr_t)param_1[0x16];

    if (bank != NULL) {
        uintptr_t tmpl = (uintptr_t)*(uint32_t *)bank;
        uint16_t next = *(uint16_t *)(tmpl + (uint32_t)*(uint16_t *)(obj + 0x0a) * 0x1cu + 0x36u);
        uint32_t slot_count = *(uint32_t *)(tmpl + 0x18);
        int guard = 0;
        while (next != 0xffffu && next < slot_count && guard++ < 4096) {
            uintptr_t node = tmpl + (uint32_t)next * 0x1cu;
            uint16_t flags = *(uint16_t *)(node + 0x1c);
            if ((flags & 0xf000u) == 0xc000u) {
                if (flags & 0x0800u)
                    param_1[0] |= 0x1000u;
                param_1[0x1a] = (uint32_t)(uintptr_t)FUN_8001b49c(bank, flags & 0x07ffu);

                int32_t rate = (int32_t)*(uint16_t *)(node + 0x16) << 16;
                if (*(uint16_t *)(node + 0x16) == 0)
                    rate = (int32_t)param_1[0x15] * (int32_t)sRam000005e4;
                rate *= (uint32_t)bRam000006cf;
                if (rate < 0)
                    rate += 0xff;
                param_1[0x1b] = (uint32_t)(rate >> 8);
                return 1;
            }
            next = *(uint16_t *)(node + 0x34);
        }
    }

    param_1[0x1a] = 0;
    param_1[0x1b] = 0;
    return 0;
}

/* ================================================================
 * FUN_8001b038 -- Bone_FindByKindOnObj
 *
 * Looks up the template attached to obj, finds the first bone chain
 * entry rooted at the bone at slot *(u16*)(obj+10) with kind `param_2`,
 * and returns that record pointer.  PSX calling convention: the return
 * value is FUN_8001afa0's v0 passing through (Ghidra incorrectly marks
 * the outer function as void; callers use the return).
 * ================================================================ */
extern int16_t *Bone_FindByKind(int tmpl, uint16_t headIdx, int16_t kind); /* FUN_8001afa0 */

intptr_t FUN_8001b038(int param_1, uint16_t param_2)
{
    uintptr_t obj = (uintptr_t)param_1;
    uintptr_t bank = obj_ptr32(obj, 0x58);
    uintptr_t tmpl = obj_ptr32(bank, 0);
    uint16_t headIdx = *(uint16_t *)(tmpl
                                     + (uint)*(uint16_t *)(obj + 10) * 0x1cu + 0x36u);
    return (intptr_t)Bone_FindByKind((int)tmpl, headIdx, (int16_t)param_2);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth -- DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001b038  (from analysis/SLUS_005.10/decomp/8001b038.c) --- */
// addr: 0x8001b038  name: FUN_8001b038
// void FUN_8001b038(int param_1, undefined2 param_2) {
//   FUN_8001afa0(**(int **)(param_1 + 0x58),
//                *(undefined2 *)(**(int **)(param_1+0x58) + (uint)*(ushort *)(param_1+10)*0x1c+0x36),
//                param_2);
// }
// NOTE: return is FUN_8001afa0's v0 (PSX calling convention leakage; Ghidra marks void)

/* --- SLUS_005.10 FUN_8001b1f8  (from analysis/SLUS_005.10/decomp/8001b1f8.c) --- */
// addr: 0x8001b1f8  name: FUN_8001b1f8

ushort * FUN_8001b1f8(int param_1)

{
  ushort uVar1;
  ushort *puVar2;

  uVar1 = *(ushort *)(**(int **)(param_1 + 0x58) + (uint)*(ushort *)(param_1 + 10) * 0x1c + 0x36);
  while( true ) {
    if (uVar1 == 0xffff) {
      return (ushort *)0x0;
    }
    puVar2 = (ushort *)(**(int **)(param_1 + 0x58) + (uint)uVar1 * 0x1c + 0x1c);
    if (*puVar2 >> 0xc == 0xb) break;
    uVar1 = puVar2[0xc];
  }
  return puVar2;
}

/* --- SLUS_005.10 FUN_8001b3d4  (from analysis/SLUS_005.10/decomp/8001b3d4.c) --- */
// addr: 0x8001b3d4  name: FUN_8001b3d4

int * FUN_8001b3d4(int *param_1,uint param_2)

{
  short sVar1;
  int *piVar2;

  piVar2 = param_1 + (param_2 & 0xffff) * 3 + 3;
  sVar1 = (short)*piVar2 + 1;
  *(short *)piVar2 = sVar1;
  if (sVar1 == 1) {
    FUN_800187e4(*(undefined4 *)((param_2 & 0xffff) * 4 + *(int *)(*param_1 + 0x14)),piVar2);
  }
  return piVar2;
}

/* --- SLUS_005.10 FUN_8001b270  (from analysis/SLUS_005.10/decomp/8001b270.c) --- */
// addr: 0x8001b270  name: FUN_8001b270

int FUN_8001b270(int param_1)

{
  ushort *puVar1;
  int iVar2;

  puVar1 = (ushort *)FUN_8001b1f8();
  if (puVar1 == (ushort *)0x0) {
    iVar2 = 0;
  }
  else {
    iVar2 = *(int *)((*puVar1 & 0xfff) * 4 + *(int *)(**(int **)(param_1 + 0x58) + 4));
    iVar2 = FUN_8001b3d4(*(int **)(param_1 + 0x58),
                         *(short *)(iVar2 + 0x12) +
                         (*(ushort *)(*(int *)(iVar2 + 0x14) + 0x12) & 0x3fff));
    *(ushort *)(iVar2 + 8) = *(ushort *)(iVar2 + 8) | 0x20;
  }
  return iVar2;
}

#endif  /* GHIDRA REF */
