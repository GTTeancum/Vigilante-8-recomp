/* weapon_subobject_spawn.c -- spawn a weapon sub-component as a child of a vehicle/object.
 *
 * Source: SLUS_005.10  FUN_80031300.
 *
 * Called when arming a weapon mount: spawns an "attachment" object
 * positioned either at a named bone slot of the parent or at the
 * parent's world matrix root.
 *
 *   param_1: parent object (the "carrier" weapon will be tagged with
 *            this at +0x80).
 *   param_2: parent vehicle (used to resolve bone slot and matrix).
 *   param_3: bone index (negative = use small alloc + parent matrix).
 *   param_4: size hint for the small allocator OR mode/flags passed
 *            to FUN_8001ac44 when bone is positive.
 *   param_5: optional joint-template ptr; if non-NULL, calls
 *            FUN_8001b2fc to set up the child<->parent joint.
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>
#include "structs.h"

extern void    *FUN_8001d470(uint32_t size);
extern void    *FUN_8001ac44(int *bank, uint16_t idx, uint32_t flags, uint32_t mode);
extern intptr_t FUN_8001b038(int parent, uint32_t mask);          /* BoneAnim_LookupSlot */
extern MATRIX  *FUN_8001d624(int obj);                            /* Matrix_ComposeParentChain */
extern void     FUN_8001d68c(MATRIX *out, int parent, int joint); /* Matrix_ComposeRelativeToObject */
extern void     FUN_8001b2fc(uint32_t *parent, const void *jointTpl, uint32_t *child);

uint32_t *FUN_80031300(int param_1, int param_2, uint16_t param_3,
                       uint32_t param_4, int param_5)
{
    uint16_t  uVar1;
    intptr_t  iVar2;
    uint32_t *puVar3;
    uint32_t *puVar4;
    uint32_t  uVar5, uVar6, uVar7;

    iVar2 = FUN_8001b038(param_2, 0x8000);

    if ((int)((uint32_t)param_3 << 0x10) < 0) {
        /* High bit of bone index set => small alloc, no template lookup. */
        puVar3 = (uint32_t *)FUN_8001d470(param_4);
    } else {
        puVar3 = (uint32_t *)FUN_8001ac44(
            (int *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_2 + 0x58),
            param_3, param_4, 8);
    }

    puVar3[0x20] = (uint32_t)param_1;
    *puVar3 = 0x800000u;
    uVar1 = *(uint16_t *)(uintptr_t)(param_1 + 6);
    *(uint8_t *)(puVar3 + 1) = 7;
    *(uint16_t *)((uint8_t *)puVar3 + 6) = uVar1;

    if (iVar2 == 0) {
        /* No bone slot: copy the parent's world matrix wholesale into
         * the new object's matrix slot (+0x10..+0x2c). */
        puVar4 = (uint32_t *)FUN_8001d624(param_2);
        uVar5 = puVar4[1];
        uVar6 = puVar4[2];
        uVar7 = puVar4[3];
        puVar3[4] = puVar4[0];
        puVar3[5] = uVar5;
        puVar3[6] = uVar6;
        puVar3[7] = uVar7;
        uVar5 = puVar4[5];
        uVar6 = puVar4[6];
        uVar7 = puVar4[7];
        puVar3[8]  = puVar4[4];
        puVar3[9]  = uVar5;
        puVar3[10] = uVar6;
        puVar3[0xb] = uVar7;
    } else {
        /* Have a bone slot: compose parent chain * joint transform. */
        FUN_8001d68c((MATRIX *)(puVar3 + 4), param_2, iVar2);
    }

    /* Copy matrix translation into world-pos cache. */
    puVar3[0x12] = puVar3[9];
    puVar3[0x13] = puVar3[10];
    puVar3[0x14] = puVar3[0xb];

    if (param_5 != 0) {
        FUN_8001b2fc((uint32_t *)(uintptr_t)param_2,
                     (const void *)iVar2,
                     (uint32_t *)(intptr_t)param_5);
    }

    return puVar3;
}
