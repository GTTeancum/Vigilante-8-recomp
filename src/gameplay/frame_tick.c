/* frame_tick.c -- Per-frame object-list walkers (tick dispatch).
 *
 * Source: SLUS_005.10
 *   FUN_800212c4  -- Frame_PreTickAll: walk piRam0000077c (anim-channel list),
 *                    call Object_FrameCounterBump on each payload.
 *   FUN_8002131c  -- Frame_TickAll: walk piRam0000075c (active-tick list),
 *                    call each object's tick callback with event 0.
 *   FUN_80021394  -- TriggerVol_TickAll: walk the trigger-volume chain
 *                    (piRam000007bc / puRam000007c4), fire event 2 when the
 *                    timer elapses.
 *
 * HIGH confidence: direct Ghidra port.
 */
#include <stdint.h>

/* ---- object lists ---- */
extern int32_t  *piRam0000077c;   /* anim-channel list head (int* sentinel) */
extern int32_t  *piRam0000075c;   /* active-tick list head */
extern int32_t  *piRam000007bc;   /* trigger-vol chain head */
extern void     *puRam000007c4;   /* trigger-vol chain tail (sentinel) */
extern uint8_t   DAT_80065ac0[];  /* trigger-vol sentinel value */

/* ---- dead-node recycler ---- */
extern int32_t  *piRam00000774;
extern uint8_t   DAT_80065a74[];

/* ---- pre-tick ---- */
extern void FUN_8001fcb4(int param_1, uint16_t param_2);  /* Object_FrameCounterBump */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* ================================================================
 * FUN_800212c4  -- Frame_PreTickAll
 *
 * Walk the animation-channel list (piRam0000077c) and call
 * Object_FrameCounterBump (FUN_8001fcb4) on each payload.
 * ================================================================ */
void FUN_800212c4(uint16_t param_1)
{
    typedef struct HostObjListNode {
        struct HostObjListNode *next;
        struct HostObjListNode *prev;
        uintptr_t payload;
        uint32_t deadline;
    } HostObjListNode;
    HostObjListNode *sentinel = (HostObjListNode *)piRam0000077c;
    HostObjListNode *node;

    if (sentinel == NULL || sentinel->prev == NULL)
        return;
    for (node = sentinel->next; node != NULL; node = node->next) {
        if (node->payload != 0)
            FUN_8001fcb4((int)node->payload, param_1);
    }
}

/* ================================================================
 * FUN_8002131c  -- Frame_TickAll
 *
 * Walk the active-tick list (piRam0000075c) and call each object's
 * tick callback (obj+100 = obj+0x64) with event 0 and param_1.
 * ================================================================ */
void FUN_8002131c(uint32_t param_1)
{
    typedef struct HostObjListNode {
        struct HostObjListNode *next;
        struct HostObjListNode *prev;
        uintptr_t payload;
        uint32_t deadline;
    } HostObjListNode;
    typedef void (*TickFn)(uint8_t *obj, int event, uint32_t arg);

    HostObjListNode *sentinel = (HostObjListNode *)piRam0000075c;
    HostObjListNode *node;

    if (sentinel == NULL || sentinel->prev == NULL)
        return;

    for (node = sentinel->next; node != NULL; node = node->next) {
        uintptr_t payload = node->payload;

        if (payload == 0)
            continue;

        TickFn cb = (TickFn)Object_CallbackFromPsxSlot((const void *)payload);
        if (cb != (TickFn)0)
            cb((uint8_t *)payload, 0, param_1);
    }
}

/* ================================================================
 * FUN_80021394  -- TriggerVol_TickAll
 *
 * Walk the trigger-volume chain (piRam000007bc head, sentinel
 * DAT_80065ac0).  For each entry whose timer field (node[3]) has
 * elapsed (param_1 >= node[3]):
 *   - Clear bit 0 of the object's flags.
 *   - Unlink the node and recycle it into the dead-node pool.
 *   - Fire tick callback (obj+0x19*4) with event 2.
 * ================================================================ */
void FUN_80021394(uint32_t param_1)
{
    extern void ObjectEventQueue_Tick(uint32_t tick);
    ObjectEventQueue_Tick(param_1);
}
