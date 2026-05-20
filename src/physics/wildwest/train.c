/* train.c -- Wild West M1train_engine / M1train_coalcar init handler.
 *
 * Source: WILDWEST.DLL  FUN_801004cc.
 *
 * On mode 1 / non-3 init: spawns a 0x80-byte child of kind 0x26 from
 * the vehicle template bank, sets its state to 3, randomises its roll
 * angle (V8_RandNext × parent_x_extent >> 15), then attaches it to
 * this object via the bone-slot pair. Sets parent flag 4 (= "has
 * child object").
 *
 * Always: if the event source is in state 2, calls FUN_8002c3ac (game
 * event handler -- pass 3 names). Falls through to the default tick.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern int Bone_AllocSlot(uint32_t parent, uint16_t slotKey);
extern void Bone_AttachChild(uint32_t parent, int slot, uint32_t *child);
extern void GameEvent_State2(void);   /* FUN_8002c3ac */
extern void Object_DefaultDispatch(uint32_t *obj, int mode, uint32_t *impulse);
extern uint32_t _DAT_800737d8;

void WW_TrainInit(uint32_t *self, int mode, uint32_t *impulse)
{
    if (mode == 1 || mode != 3) {
        uint32_t *child = Object_Pool_AllocFromBank((void *)(uintptr_t)_DAT_800737d8,
                                                    0x26, 0x80, 8);
        *((int8_t *)child + 4) = 3;
        int rand = (int)V8_RandNext();
        int parentExtent = **(int **)((uintptr_t)_DAT_800737d8 + 4);
        *(int16_t *)((uint8_t *)child + 0x46) = (int16_t)(rand * parentExtent >> 15);
        int slot = Bone_AllocSlot((uint32_t)(uintptr_t)self, 0x8000);
        Bone_AttachChild((uint32_t)(uintptr_t)self, slot, child);
        self[0] |= 4u;
    }
    if (*(int8_t *)(impulse[0] + 4) == 2) {
        GameEvent_State2();
    }
    Object_DefaultDispatch(self, mode, impulse);
}
