/* bone_findfree.c -- find the first free bone slot.
 *
 * Source: SLUS_005.10  FUN_8003fbc8.
 *
 * Walks an object's bone-template chain (rooted at template+0x36,
 * chained via +0x34) and returns the index of the first bone whose
 * kind field has the high byte 0xff (the "free slot" marker) AND
 * is not 0xffff (the end-of-chain sentinel).
 *
 * HIGH confidence.
 */
#include <stdint.h>

uint32_t Bone_FindFreeSlot(int obj)
{
    int templateBody = **(int **)(intptr_t)(obj + 0x58);
    uint16_t idx = *(uint16_t *)(intptr_t)(templateBody
                                + *(uint16_t *)(intptr_t)(obj + 10) * 0x1c + 0x36);
    while (idx != 0xffff) {
        uint16_t kind = *(uint16_t *)(intptr_t)(templateBody + idx * 0x1c + 0x1c);
        if ((kind >> 8) == 0xff && kind != 0xffff) return idx;
        idx = *(uint16_t *)(intptr_t)(templateBody + idx * 0x1c + 0x34);
    }
    return 0;
}
