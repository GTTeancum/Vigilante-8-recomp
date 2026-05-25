/* smoke_child_tick.c -- smoke child object tick callbacks.
 *
 * LAB_8002bfb8: smoke child object death event handler.
 *   Installed as the tick callback for the smoke child object spawned when
 *   a vehicle takes damage.  On event 5 (death), clears the parent vehicle's
 *   "has active smoke child" flag (bit 17 = 0x20000) at parent+0, detaches
 *   this child from the parent, then frees and unregisters it.
 *
 * LAB_8002bdd0: wrecked-vehicle animation tick / damage-event dispatch.
 *   Installed by Vehicle_SoftKill as the tick callback for a totalled vehicle.
 *   Dispatches on event type:
 *     0 -> normal frame update (animation tick + audio position update)
 *     2 -> vehicle destroyed message + rebind (FUN_8002bd84)
 *     3 -> handle event-3 (FUN_8002d82c)
 *   All other events return 0 with no action.
 */

#include <stdint.h>

/* Forward declarations for functions not yet named */
extern uint32_t *Object_Parent(uint32_t obj);       /* FUN_8001d5a0 */
extern void      Object_DetachFromParent(uint32_t obj); /* FUN_8001d564 */
extern int       Object_FreeAndUnregister(uint32_t obj); /* FUN_800204dc */

/* Wrecked-vehicle helpers */
extern int FUN_8002bd84(uint32_t obj);   /* vehicle destroyed message + rebind */
extern int FUN_8002d82c(uint32_t obj, uint32_t param2); /* handle event-3 */

/* Animation / audio helpers called during event 0 */
extern int FUN_80030c08(uint32_t obj);            /* animation tick for type 12 */
extern int FUN_8002f998(uint32_t obj);            /* normal frame update        */
extern int FUN_800446dc(uint32_t *pos_ptr);       /* audio position update (SfxPan_For3DPosDelayed) */
extern int FUN_80044574(int voice, int result);   /* audio voice update         */

/*
 * LAB_8002bfb8 -- smoke child death handler
 *
 * param_1 (a0) = smoke child object handle
 * param_2 (a1) = event type (5 = death)
 * Returns -1 on death (object destroyed), 0 otherwise.
 */
int LAB_8002bfb8(uint32_t param_1, int param_2)
{
    uint32_t *parent_flags_ptr;

    if (param_2 != 5)
        return 0;

    /* Get pointer to parent's first word (flags) and clear bit 0x20000
     * ("has active smoke child" flag). */
    parent_flags_ptr = Object_Parent(param_1);
    *parent_flags_ptr = *parent_flags_ptr & ~0x20000u;

    /* Detach this child from the parent hierarchy, then free it. */
    Object_DetachFromParent(param_1);
    Object_FreeAndUnregister(param_1);

    return -1;
}

/*
 * LAB_8002bdd0 -- wrecked-vehicle animation / damage-event dispatch
 *
 * param_1 (a0/s0) = vehicle object handle
 * param_2 (a1)    = event type
 * param_3 (a2)    = event parameter (used for event 3)
 * Returns 0 in all cases.
 */
int LAB_8002bdd0(uint32_t param_1, int param_2, uint32_t param_3)
{
    uint8_t *obj_bytes = (uint8_t *)param_1;
    int result;

    switch (param_2) {
    case 2:
        FUN_8002bd84(param_1);
        break;

    case 3:
        FUN_8002d82c(param_1, param_3);
        break;

    case 0:
        /* Animation tick: type byte at obj+0xd0 selects which routine runs. */
        if (obj_bytes[0xd0] == 0x0c)
            FUN_80030c08(param_1);
        else
            FUN_8002f998(param_1);

        /* Audio position update: pass pointer to position field at obj+0x24. */
        result = FUN_800446dc((uint32_t *)(obj_bytes + 0x24));

        /* Voice handle is a signed byte at obj+5. */
        FUN_80044574((int)(int8_t)obj_bytes[5], result);
        break;

    default:
        break;
    }

    return 0;
}
