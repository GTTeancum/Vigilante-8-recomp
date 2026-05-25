/* bone_fill_template.c -- fill a pre-allocated bone object from a bank template.
 *
 * Source: SLUS_005.10  FUN_8001add0.
 *
 * Twin of FUN_8001ac44 (BoneObj_BuildTree) but operates on a
 * pre-allocated object record `param_1` instead of allocating a new
 * one.  Used during piecewise reattachment (e.g. when a vehicle wreck
 * re-binds a single bone child to an existing carrier).
 *
 * param_1: pre-allocated 0x9c-byte object to fill in
 * param_2: bone-bank (int *bank).  Bone table at *bank + idx*0x1c + 0x1c.
 * param_3: bone index (ushort)
 * param_4: flags
 *   bit 0 (0x01): traverse `next-child` link  (psVar4[0xc])
 *   bit 1 (0x02): if SET, do NOT traverse `next-sibling` (psVar4[0xd])
 *   bit 3 (0x08): also call FUN_8001ab98 to fill animation slot +0x60
 *
 * Layout written into param_1:
 *   +0x0a (u16)  bone index
 *   +0x30        bone-level sub-object (via FUN_8001b49c, or 0)
 *   +0x34        first-child obj (recursive call result)
 *   +0x38        next-sibling obj (recursive call result)
 *   +0x3c        parent ptr (cleared to 0)
 *   +0x46 (u16)  frame counter snapshot (uRam000006cc)
 *   +0x58        bone bank ptr
 *   +0x5c        per-bone mesh entry, or 0 if next short is negative
 *   +0x60        bone-anim slot (if flag 8 set), or 0
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>

extern uint16_t *FUN_8001b49c(int *bank, uint32_t key);   /* Bone_AllocLevel */
extern int       FUN_8001ab98(int *bank, uint16_t idx);   /* BoneAnim_AllocSlot */
extern int       FUN_8001ac44(int *bank, uint16_t idx,
                              uint32_t flags3, uint32_t flags4); /* BoneObj_BuildTree */

extern uint16_t uRam000006cc;

static inline void obj_write32(uintptr_t base, uint32_t off, uintptr_t value)
{
    *(uint32_t *)(base + off) = (uint32_t)value;
}

int FUN_8001add0(int param_1, int *param_2, uint16_t param_3, uint32_t param_4)
{
    uint16_t *puVar3;
    uint32_t  uVar1;
    int       iVar2;

    puVar3 = (uint16_t *)(uintptr_t)(*param_2 + (uint32_t)param_3 * 0x1c + 0x1c);

    if ((int16_t)*puVar3 < 0) {
        obj_write32((uintptr_t)param_1, 0x30, 0);
    } else {
        uVar1 = (uint32_t)(uintptr_t)FUN_8001b49c(param_2, *puVar3 & 0x7ffu);
        obj_write32((uintptr_t)param_1, 0x30, uVar1);
    }

    if ((int16_t)puVar3[1] < 0) {
        obj_write32((uintptr_t)param_1, 0x5c, 0);
    } else {
        obj_write32((uintptr_t)param_1, 0x5c,
            *(uint32_t *)(uintptr_t)((int16_t)puVar3[1] * 4
                                     + *(int *)(uintptr_t)(*param_2 + 0xc)));
    }

    obj_write32((uintptr_t)param_1, 0x58, (uintptr_t)param_2);
    *(uint16_t *)(uintptr_t)(param_1 + 0xa) = param_3;

    if ((param_4 & 8u) == 0) {
        obj_write32((uintptr_t)param_1, 0x60, 0);
    } else {
        uVar1 = (uint32_t)FUN_8001ab98(param_2, param_3);
        obj_write32((uintptr_t)param_1, 0x60, uVar1);
    }

    *(uint16_t *)(uintptr_t)(param_1 + 0x46) = uRam000006cc;

    if (((param_4 & 1u) == 0) || (puVar3[0xc] == 0xffff)) {
        obj_write32((uintptr_t)param_1, 0x34, 0);
    } else {
        iVar2 = FUN_8001ac44(param_2, puVar3[0xc], 0x80u, param_4);
        obj_write32((uintptr_t)param_1, 0x34, (uintptr_t)iVar2);
        obj_write32((uintptr_t)iVar2, 0x3c, (uintptr_t)param_1);
    }

    if (((param_4 & 2u) == 0) && (puVar3[0xd] != 0xffff)) {
        iVar2 = FUN_8001ac44(param_2, puVar3[0xd], 0x80u, param_4 | 1u);
        obj_write32((uintptr_t)param_1, 0x38, (uintptr_t)iVar2);
        obj_write32((uintptr_t)iVar2, 0x3c, (uintptr_t)param_1);
    } else {
        obj_write32((uintptr_t)param_1, 0x38, 0);
    }

    obj_write32((uintptr_t)param_1, 0x3c, 0);
    return param_1;
}
