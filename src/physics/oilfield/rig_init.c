/* rig_init.c -- Oil Fields rig_1 init.
 *
 * Source: OILFIELD.DLL  FUN_801001cc.
 *
 * On mode-1 (init), allocates a child object (size 0x24, type 0x80,
 * flags 8) from the vehicle pool at _DAT_800737d8 via FUN_8001ac44,
 * zeroes the child's +0x5c (last-state slot), sets the child's roll
 * angle via `V8_RandNext() * parent_x_extent >> 15` (random spin
 * offset), then attaches the child to the parent via FUN_8001b038
 * (alloc bone slot) + FUN_8001b2fc (link). Installs the default per-
 * tick handler SUB_800223dc and sets parent flag 0x4 (active). Then
 * unconditionally falls through to the default handler.
 *
 * Used by `rig_1`, `pipe_end_1`, `OilPump_1` -- all the same physics
 * (random initial yaw, then default child-management).
 *
 * MED.
 */
#include <stdint.h>

extern int   Object_Pool_AllocFromBank(void *bank, int type, int u, int flags);  /* FUN_8001ac44 */
extern uint32_t V8_RandNext(void);
extern int   Bone_AllocSlot(uint32_t *obj, int slot);     /* FUN_8001b038 */
extern void  Bone_AttachChild(uint32_t *obj, int slot, int child);  /* FUN_8001b2fc */
extern void  SUB_800223dc(void);
extern void  Object_DefaultDispatch(uint32_t *obj, int mode, uint32_t arg);  /* func_0x800223dc */

extern void **_DAT_800737d8;

void OF_RigInit(uint32_t *obj, int mode, uint32_t arg)
{
    if (mode == 1) {
        int child = Object_Pool_AllocFromBank(_DAT_800737d8, 0x24, 0x80, 8);
        *(int *)(child + 0x5c) = 0;
        int extent = **(int **)((uintptr_t)_DAT_800737d8 + 4);
        *(int16_t *)(child + 0x46) = (int16_t)((int)V8_RandNext() * extent >> 15);
        int slot = Bone_AllocSlot(obj, 0x8000);
        Bone_AttachChild(obj, slot, child);
        obj[0x19] = (uintptr_t)SUB_800223dc;
        obj[0]  |= 4u;
    }
    Object_DefaultDispatch(obj, mode, arg);
}
