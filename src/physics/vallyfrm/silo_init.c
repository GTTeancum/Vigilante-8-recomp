/* silo_init.c -- Valley Farms silo_1 / Windmill_1 multi-mode dispatch.
 *
 * Source: VALLYFRM.DLL  FUN_80100c1c.
 *
 * Multi-mode dispatcher:
 *   mode 1 (init): clear back-buffer + child back-buffer flags via
 *     FUN_800207f8 / FUN_800207c4; set child sub-state to 0xe;
 *     allocate a free bone slot and stash its template entry at
 *     +0x1e (used as the spawn anchor); snap posY to terrain at
 *     (posX, posZ).
 *   mode 3 (weapon hit): if hitting our own child, run damage
 *     arbiter; on kill set sub-state 0x1e / +0x46 = global counter,
 *     clear back-buf, sub-state 0x78.
 *   mode 2: same teardown.
 *
 * MED.
 */
#include <stdint.h>

extern void Object_ClearBackBufBit_Flag(uint32_t *obj);    /* FUN_800207f8 */
extern void Object_ClearBackBufBit_Child(uint32_t obj);     /* FUN_800207c4 */
extern uint32_t Bone_FindFreeSlot(int obj);                 /* FUN_8003fbc8 */
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern void Object_SetSubState(int obj, int sub);
extern uint32_t _DAT_800659d0;

uint32_t VF_SiloInit(uint32_t *self, int mode, int *impulse)
{
    switch (mode) {
    case 1: {
        Object_ClearBackBufBit_Flag(self);
        Object_ClearBackBufBit_Child(self[0xe]);
        *(uint16_t *)(self[0xe] + 0xc) = 0xe;
        uint32_t slot = Bone_FindFreeSlot((int)(intptr_t)self);
        self[0x1e] = *(uint32_t *)self[0x16] + slot * 0x1c + 0x1c;
        self[0x13] = (uint32_t)Terrain_HeightAt(self[0x12], self[0x14]);
        return 0;
    }
    case 3: {
        int8_t needState = 7;
        if ((uint32_t)impulse[3] == self[0xe]) {
            int killed = Damage_FromImpulse((uint32_t *)(intptr_t)impulse[3], impulse);
            if (killed == 0) return 0;
            *(int8_t *)(self + 2) = 0x1e;
            *(uint16_t *)((uintptr_t)self + 0x46) = (uint16_t)_DAT_800659d0;
            Object_ClearBackBufBit_Child((uint32_t)(intptr_t)self);
            Object_SetSubState((int)(intptr_t)self, 0x78);
            needState = 0;
        }
        if (*(int8_t *)(*impulse + 4) != needState) return 0;
        /* impulse now = (int)*(uint16_t *)(*impulse + 0xc); -- caller uses */
        return 0;
    }
    case 2:
        /* same teardown */
        return 0;
    default:
        return 0;
    }
}
