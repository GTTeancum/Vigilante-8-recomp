/* bone_bank_lookup.c -- look up a bone record in a vehicle's template bank.
 *
 * Source: SLUS_005.10
 *   FUN_8001affc  -- BoneBank_FindByKey(bank, kind, key)  (10 instr)
 *
 * Wraps FUN_8001afa0 (= Bone_FindByKind in bone_lookup.c).  Dereferences
 * the bank pointer to get the template base, then reads the head-index
 * for the given `kind` entry at template[kind*28 + 0x36], and delegates
 * the linked-list walk to Bone_FindByKind.
 *
 * Bank/template layout (byte strides):
 *   bank[0]                  -- uint32_t: pointer to template data
 *   template + kind*0x1c     -- 28-byte kind record
 *   template + kind*0x1c + 0x36 -- u16 headIdx for the bone chain
 *
 * Within each chain entry (28 bytes each):
 *   entry[0]   (u16) -- bone kind / ID
 *   entry[24]  (u16) -- next-link index (0xffff = end)
 *
 * HIGH: line-for-line MIPS port of FUN_8001affc (8001affc.s).
 * Ghidra decomp at analysis/SLUS_005.10/decomp/8001affc.c confirms.
 */
#include <stdint.h>
#include <stddef.h>

/* FUN_8001afa0 -- implemented in bone_lookup.c as Bone_FindByKind. */
extern int16_t *Bone_FindByKind(intptr_t tmpl, uint16_t headIdx, int16_t targetKind);

/*
 * HIGH: look up the bone record for `key` under `kind` in `bank`.
 *
 * MIPS:
 *   lw   a0,  0(a0)              ; a0 = *bank  (template ptr)
 *   andi a1,  a1, 0xffff         ; a1 = kind (u16)
 *   sll/subu/sll v0 = kind*28
 *   addu v0,  a0, v0             ; v0 = template + kind*28
 *   lhu  a1,  0x36(v0)           ; a1 = headIdx
 *   jal  FUN_8001afa0 (Bone_FindByKind)
 *   _andi a2, a2, 0xffff         ; delay: a2 = key (u16)
 */
intptr_t FUN_8001affc(int *bank, uint16_t kind, uint16_t key)
{
    intptr_t tmpl    = *(uint32_t *)(uintptr_t)bank;  /* *bank */
    uint16_t headIdx = *(uint16_t *)(uintptr_t)(tmpl + (uint32_t)kind * 28u + 0x36u);
    return (intptr_t)Bone_FindByKind(tmpl, headIdx, (int16_t)key);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001affc  (from analysis/SLUS_005.10/decomp/8001affc.c) --- */
// addr: 0x8001affc  name: FUN_8001affc

void FUN_8001affc(int *param_1,uint param_2,undefined2 param_3)

{
  FUN_8001afa0(*param_1,*(undefined2 *)(*param_1 + (param_2 & 0xffff) * 0x1c + 0x36),param_3);
  return;
}

#endif  /* GHIDRA REF */
