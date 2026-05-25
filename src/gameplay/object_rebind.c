/* object_rebind.c -- re-attach an object to a new bone-bank template.
 *
 * Source: SLUS_005.10  FUN_8003f89c.
 *
 * Used when an object's bone template changes (vehicle wreck stage,
 * weapon-state advance, etc.).  The function:
 *   1. Detaches from world list if it was registered (flag 0x04).
 *   2. Frees the current bone-bank slot (+0xc) and the entire child
 *      subtree (+0xe).
 *   3. Frees the secondary bone bank (+0x1a) if present.
 *   4. Transforms the new bone-template position vector through the
 *      object's local matrix (param_1+0x10) via FUN_80043408.
 *   5. Copies the transformed pos into both world-pos (+0x24..0x2c)
 *      and matrix-trans (+0x48..0x50) slots.
 *   6. Looks up the new bone-level sub-object for the new template
 *      (FUN_8001b49c), or 0 if unmapped.
 *   7. Stores the new bone-index at +0xa, parent at +0xe.
 *   8. Patches the new parent's `child` back-link if non-NULL.
 *   9. Looks up the new mesh entry (+0x17) or 0 if template is "skip".
 *  10. Runs the collision-test FUN_8001ec48; if zero, sets flag 0x20.
 *  11. Walks the parent's anim-slot chain via +0x34, copying the
 *      global frame counter snapshot uRam000006cc into +0x46 of each
 *      animated entry; tracks whether ANY entry had an anim slot.
 *  12. If at least one entry was animated, re-registers in the world
 *      list via FUN_800207c4.
 *  13. Clears flag 0x8000 (in-progress marker).
 *  14. Recomputes the bounding radius via FUN_8001dc1c.
 *
 * HIGH confidence (direct Ghidra port).  The +0x46 frame-counter
 * shuffle is preserved verbatim -- Ghidra's pseudo-C splits the
 * apparent `*p = uRam000006cc` into a temp swap sequence due to
 * MIPS register-pressure spill which Ghidra didn't collapse.
 */
#include <stdint.h>
#include <stdbool.h>

extern uint16_t uRam000006cc;

extern void     FUN_800207f8(uint32_t *obj);                   /* clear flag 4 + remove from world list */
extern void     FUN_800207c4(uint32_t *obj);                   /* set flag 4 + add to world list */
extern void     FUN_8001bddc(int boneBank);                    /* BoneBank_Free */
extern void     FUN_800204dc(int objHead);                     /* Object_HeapFreeRecursive */
extern uint16_t *FUN_8001b49c(int *bank, uint32_t key);        /* Bone_AllocLevel */
extern void     FUN_80043408(uint32_t *m, int16_t *vec_in,
                             int32_t *vec_out);                /* GTE_RotateLongMatTrans */
extern int      FUN_8001ec48(uint32_t *obj);                   /* CollTree_HasContact */
extern int      FUN_8001dc1c(uint32_t *obj);                   /* Object_RecomputeBoundingRadius */

void FUN_8003f89c(uint32_t *param_1, int *param_2,
                  uint16_t param_3, uint32_t param_4)
{
    bool      bVar1;
    uint16_t  uVar2, uVar3;
    uint32_t  uVar4;
    int       iVar5;
    uint16_t *puVar6;

    bVar1 = false;
    puVar6 = (uint16_t *)(uintptr_t)(*param_2 + (uint32_t)param_3 * 0x1c + 0x1c);

    if ((*param_1 & 4u) != 0) {
        FUN_800207f8(param_1);
    }
    FUN_8001bddc((int)param_1[0xc]);
    FUN_800204dc((int)param_1[0xe]);

    if (param_1[0x1a] != 0) {
        FUN_8001bddc((int)param_1[0x1a]);
        param_1[0x1a] = 0;
        param_1[0x1b] = 0;
    }

    /* Rotate new template-relative pos into world frame; store into
     * +0x48 (matrix trans) and copy to +0x24 (world pos cache). */
    FUN_80043408(param_1 + 4,
                 (int16_t  *)(puVar6 + 2),
                 (int32_t  *)(param_1 + 0x12));
    param_1[9]  = param_1[0x12];
    param_1[10] = param_1[0x13];
    param_1[0xb] = param_1[0x14];

    if (*puVar6 < 0xfffeu) {
        uVar4 = (uint32_t)(uintptr_t)FUN_8001b49c(param_2, *puVar6 & 0xffu);
        param_1[0xc] = uVar4;
    } else {
        param_1[0xc] = 0;
    }
    param_1[0x18] = 0;

    *(uint16_t *)((uint8_t *)param_1 + 10) = param_3;
    param_1[0xe] = param_4;
    if (param_4 != 0) {
        *(uint32_t **)(uintptr_t)(param_4 + 0x3c) = param_1;
    }

    if ((int16_t)puVar6[1] < 0) {
        param_1[0x17] = 0;
    } else {
        param_1[0x17] = *(uint32_t *)(uintptr_t)
            ((int16_t)puVar6[1] * 4 + *(int *)(uintptr_t)(*param_2 + 0xc));
    }

    iVar5 = FUN_8001ec48(param_1);
    uVar3  = uRam000006cc;
    if (iVar5 == 0) {
        *param_1 |= 0x20u;
        uVar3 = uRam000006cc;
    }

    /* Walk the sibling chain at +0x34 of `param_4` (the new parent
     * chain head was just installed); for each entry whose anim slot
     * +0x60 is non-zero, snapshot the frame counter into +0x46 and
     * set bVar1 so we re-register at the end. */
    for (; uVar2 = uRam000006cc, param_4 != 0;
         param_4 = *(uint32_t *)(uintptr_t)(param_4 + 0x34))
    {
        if (*(int *)(uintptr_t)(param_4 + 0x60) != 0) {
            *(uint16_t *)(uintptr_t)(param_4 + 0x46) = uRam000006cc;
            uRam000006cc = uVar3;
            bVar1 = true;
            uVar3 = uRam000006cc;
        }
        uRam000006cc = uVar3;
        uVar3 = uRam000006cc;
        uRam000006cc = uVar2;
    }
    uRam000006cc = uVar3;

    if (bVar1) {
        FUN_800207c4(param_1);
    }
    *param_1 &= 0xffff7fffu;   /* clear in-progress flag 0x8000 */
    FUN_8001dc1c(param_1);
}
