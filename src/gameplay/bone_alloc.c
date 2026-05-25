/* bone_alloc.c -- Bone-tree object allocation.
 *
 * Source: SLUS_005.10
 *   FUN_8001aaa8  BoneObj_AllocNode(bank, entry, size, flags)  (~48 instr)
 *   FUN_8001ac44  BoneObj_BuildTree(bank, slot, size, flags)   (~54 instr)
 *
 * These two functions co-operate to build a linked tree of bone-object nodes
 * from a bone-bank entry.
 *
 * FUN_8001aaa8:
 *   Allocates `size` bytes via the PSX heap (FUN_8001178c mode=1), then fills
 *   the new object from the bone entry pointed to by `entry`:
 *     obj[0]    = 0 or 0x10 (sign flag from entry[0])
 *     obj+0x43  = packed byte from entry+0x13 area (misaligned copy)
 *     obj+0x40  = packed byte from entry+0x20 area (misaligned copy)
 *     obj+0x44  = entry[10] (ushort, model reference)
 *     obj+0x48  = entry[2..6] (3 × int, bounding box)
 *     obj[0x16] = bank ptr
 *     obj[0x0c] = FUN_8001b49c result (bone-level sub-object, if entry[0]>=0)
 *     obj[0x17] = bone-channel data (if entry[1]>=0)
 *   Then calls FUN_8001d708 (Object_InitBoneMatrix).
 *
 * FUN_8001ac44 (recursive):
 *   Looks up bone entry for slot `slot` in bank.  If the entry is "skipped"
 *   (entry[0] < 0), recurse on the next sibling (if flags & 1).  Otherwise
 *   build the node with FUN_8001aaa8, link next sibling at +0x34 and first
 *   child at +0x38, and clear parent ptr (+0x3c).
 *
 * PSX calling convention note: FUN_8001ac44 passes a2/a3 unchanged when it
 * calls FUN_8001aaa8, so FUN_8001aaa8 receives param_3 (size) and param_4
 * (flags) from FUN_8001ac44's own arguments.  Ghidra shows only 2 explicit
 * args for the call; we pass all 4 here to match MIPS register state.
 *
 * HIGH: direct port from Ghidra pseudoC (8001aaa8.c, 8001ac44.c);
 * misaligned byte copies preserved exactly.
 */
#include <stdint.h>
#include <stdio.h>

/* Dependencies */
extern void    *FUN_8001178c(uint32_t size, uint32_t mode);  /* Heap_AllocOrRetry */
extern int      FUN_8001b49c(int *bank, unsigned key);        /* Bone_AllocLevel */
extern int      FUN_8001ab98(int *bank, uint16_t slot);       /* BoneBank_FindByKey */
extern void     FUN_8001d708(uint32_t *obj);                  /* Object_InitBoneMatrix */
extern int      g_v8_vehicle_spawn_probe_trace;

/* Global: PSX gp-relative, render-slot sequence counter */
extern uint32_t uRam000006cc;

/* ================================================================
 * FUN_8001aaa8 -- BoneObj_AllocNode
 * ================================================================ */
int *FUN_8001aaa8(int *param_1, uint16_t *param_2,
                  uint32_t param_3, uint32_t param_4)
{
    uint16_t uVar1;
    uint32_t uVar2, uVar6;
    uint32_t *puVar3;
    int      *piVar4;
    int       iVar5, iVar7;

    piVar4 = (int *)FUN_8001178c(param_3, 1);
    iVar5 = 0;
    if ((int16_t)*param_2 >= 0) {
        iVar5 = (uint32_t)((*param_2 & 0x800u) != 0) << 4;
    }
    *piVar4 = iVar5;

    /* Misaligned read: byte at param_2+0x13 */
    uVar2 = (uint32_t)(uintptr_t)param_2 + 0x13u;
    uVar6 = (uint32_t)(uintptr_t)(param_2 + 8);
    uVar2 &= 3u;
    uVar6 &= 3u;
    uVar6 = (uint32_t)(
             (*(uint32_t *)((uint8_t *)param_2 + 0x13 - uVar2) << ((3 - uVar2) * 8) |
              param_4 & (0xffffffffu >> ((uVar2 + 1) * 8)))
             & (0xffffffffu << ((4 - uVar6) * 8))
             | *(uint32_t *)((uint8_t *)(param_2 + 8) - uVar6) >> (uVar6 * 8));

    uVar1 = param_2[10];

    /* Misaligned write to piVar4+0x43 */
    uVar2 = (uint32_t)(uintptr_t)piVar4 + 0x43u;
    uVar2 &= 3u;
    puVar3 = (uint32_t *)((uint8_t *)piVar4 + 0x43 - uVar2);
    *puVar3 = (*puVar3 & (0xffffffffu << ((uVar2 + 1) * 8)))
              | (uVar6 >> ((3 - uVar2) * 8));

    /* Misaligned write to piVar4+0x40 */
    uVar2 = (uint32_t)(uintptr_t)(piVar4 + 0x10);
    uVar2 &= 3u;
    puVar3 = (uint32_t *)((uint8_t *)(piVar4 + 0x10) - uVar2);
    *puVar3 = (*puVar3 & (0xffffffffu >> ((4 - uVar2) * 8)))
              | (uVar6 << (uVar2 * 8));

    *(uint16_t *)(piVar4 + 0x11) = uVar1;

    iVar5 = *(int *)(param_2 + 4);
    iVar7 = *(int *)(param_2 + 6);
    piVar4[0x12] = *(int *)(param_2 + 2);
    piVar4[0x13] = iVar5;
    piVar4[0x14] = iVar7;
    piVar4[0x16] = (int)(uintptr_t)param_1;

    if ((int16_t)*param_2 >= 0) {
        iVar5 = FUN_8001b49c(param_1, *param_2 & 0x7ffu);
        piVar4[0x0c] = iVar5;
    }
    if ((int16_t)param_2[1] >= 0) {
        uintptr_t tmpl = (uintptr_t)*(uint32_t *)param_1;
        piVar4[0x17] = *(int *)((int16_t)param_2[1] * 4 +
                                 (uintptr_t)*(uint32_t *)(tmpl + 0x0c));
    }
    FUN_8001d708((uint32_t *)piVar4);
    return piVar4;
}

/* ================================================================
 * FUN_8001ac44 -- BoneObj_BuildTree (recursive)
 * ================================================================ */
int FUN_8001ac44(int *param_1, uint16_t param_2,
                 uint32_t param_3, uint32_t param_4)
{
    uintptr_t iVar1;
    uint32_t  uVar2;
    uintptr_t iVar3;
    int16_t  *psVar4;

    /* Bone entry: base = *param_1, stride = 0x1c, index = param_2 */
    psVar4 = (int16_t *)((uintptr_t)*(uint32_t *)param_1 + (uint32_t)param_2 * 0x1c + 0x1c);
    if (g_v8_vehicle_spawn_probe_trace) {
        fprintf(stderr,
                "v8: bone_alloc trace bank=%p slot=%u size=0x%x flags=0x%x "
                "e0=%d sib=%d child=%d\n",
                (void *)param_1, (unsigned)param_2, (unsigned)param_3,
                (unsigned)param_4, (int)psVar4[0], (int)psVar4[0xc],
                (int)psVar4[0xd]);
    }

    if ((*psVar4 < 0) && (*psVar4 != -1 || ((param_4 & 4) != 0))) {
        /* Bone is "skipped" */
        iVar1 = 0;
        if ((param_4 & 1) != 0) {
            if (psVar4[0xc] == -1) {
                iVar1 = 0;
            } else {
                iVar1 = FUN_8001ac44(param_1, (uint16_t)psVar4[0xc],
                                     0x80u, param_4);
            }
        }
    } else {
        /* Valid bone -- allocate and link */
        iVar1 = (uintptr_t)FUN_8001aaa8(param_1, (uint16_t *)psVar4,
                                        param_3, param_4);
        *(uint16_t *)((uint8_t *)iVar1 + 10) = param_2;
        *(int16_t  *)((uint8_t *)iVar1 +  6) = psVar4[0xb];

        if ((param_4 & 8) == 0) {
            *(uint32_t *)((uint8_t *)iVar1 + 0x60) = 0;
        } else {
            uVar2 = (uint32_t)FUN_8001ab98(param_1, param_2);
            *(uint32_t *)((uint8_t *)iVar1 + 0x60) = uVar2;
        }

        *(uint32_t *)((uint8_t *)iVar1 + 0x46) = uRam000006cc;

        if (((param_4 & 1) == 0) || (psVar4[0xc] == -1)) {
            *(uint32_t *)((uint8_t *)iVar1 + 0x34) = 0;
        } else {
            iVar3 = FUN_8001ac44(param_1, (uint16_t)psVar4[0xc],
                                  0x80u, param_4);
            if (g_v8_vehicle_spawn_probe_trace && iVar3 == 0) {
                fprintf(stderr,
                        "v8: bone_alloc trace null sibling parent_slot=%u "
                        "child_slot=%d flags=0x%x\n",
                        (unsigned)param_2, (int)psVar4[0xc], (unsigned)param_4);
            }
            *(int *)((uint8_t *)iVar1 + 0x34) = iVar3;
            if (iVar3 != 0)
                *(int *)((uint8_t *)iVar3 + 0x3c) = iVar1;
        }

        if (((param_4 & 2) == 0) && (psVar4[0xd] != -1)) {
            iVar3 = FUN_8001ac44(param_1, (uint16_t)psVar4[0xd],
                                  0x80u, param_4 | 1u);
            if (g_v8_vehicle_spawn_probe_trace && iVar3 == 0) {
                fprintf(stderr,
                        "v8: bone_alloc trace null child parent_slot=%u "
                        "child_slot=%d flags=0x%x\n",
                        (unsigned)param_2, (int)psVar4[0xd], (unsigned)(param_4 | 1u));
            }
            *(int *)((uint8_t *)iVar1 + 0x38) = iVar3;
            if (iVar3 != 0)
                *(int *)((uint8_t *)iVar3 + 0x3c) = iVar1;
        } else {
            *(uint32_t *)((uint8_t *)iVar1 + 0x38) = 0;
        }

        *(uint32_t *)((uint8_t *)iVar1 + 0x3c) = 0;
    }
    return (int)iVar1;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001aaa8  (from analysis/SLUS_005.10/decomp/8001aaa8.c) ---
 * 8001aaa8: jal FUN_8001178c(param_3, 1)  ; alloc param_3 bytes, mode=1
 * 8001aab4: *piVar4 = 0 or 0x10           ; sign flag from entry[0]
 * 8001aac0: misaligned read entry+0x13    ; uVar6 = packed byte
 * 8001aad0: misaligned write to piVar4+0x43
 * 8001aade: misaligned write to piVar4+0x40
 * 8001aae4: piVar4[0x11] = entry[10]      ; model ref
 * 8001aaf0: piVar4[0x12..14] = entry[2..6] ; bbox
 * 8001aaf8: piVar4[0x16] = param_1        ; bank back-ref
 * 8001ab04: if entry[0]>=0: piVar4[0xc] = FUN_8001b49c(bank, entry[0]&0x7ff)
 * 8001ab1c: if entry[1]>=0: piVar4[0x17] = bone-channel data
 * 8001ab30: jal FUN_8001d708(piVar4)      ; Object_InitBoneMatrix
 */

/* --- SLUS_005.10 FUN_8001ac44  (from analysis/SLUS_005.10/decomp/8001ac44.c) ---
 * 8001ac44: psVar4 = *param_1 + param_2*0x1c + 0x1c
 * 8001ac50: if (*psVar4<0 && (*psVar4!=-1 || param_4&4)):
 * 8001ac60:   if param_4&1 and psVar4[0xc]!=-1: recurse(next_sibling)
 * 8001ac74: else:
 * 8001ac78:   iVar1 = FUN_8001aaa8(param_1, psVar4, [param_3], [param_4])
 * 8001ac84:   *(u16*)(iVar1+10) = param_2; *(s16*)(iVar1+6) = psVar4[0xb]
 * 8001ac90:   if param_4&8: iVar1+0x60 = FUN_8001ab98(bank, param_2)
 * 8001aca0:   iVar1+0x46 = uRam000006cc
 * 8001acb0:   if param_4&1 && psVar4[0xc]!=-1: recurse next_sibling; link +0x34
 * 8001acc4:   if !(param_4&2) && psVar4[0xd]!=-1: recurse first_child; link +0x38
 * 8001acd8:   iVar1+0x3c = 0
 */

#endif  /* GHIDRA REF */
