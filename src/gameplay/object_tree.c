/* object_tree.c -- kd-tree traversal and per-frame object-tick walks.
 *
 * Source: SLUS_005.10
 *   FUN_800204dc  -- Tree_Free (recursive kd-tree free)
 *   FUN_8002123c  -- Tree_Apply (recursive Apply-to-every-leaf)
 *   FUN_8002131c  -- ObjList_TickAll (per-frame tick of live objects)
 *   FUN_800212c4  -- ObjList_PreTickAll (with a flag argument)
 *   FUN_80021394  -- TriggerVol_ExpireFromHead (timed expiry)
 *   FUN_800215d0  -- Tree_VisibilityPass
 *   FUN_80021600  -- Frame_DispatchAllObjectTicks
 *
 * The kd-tree node layout (recurring across the binary):
 *   +0  i32 kind   (0 = leaf, 1 = split-X, 2 = split-Z)
 *   +1  i32 split  (split coord) or chainHead (leaf)
 *   +2  ptr left   child (split nodes) or per-leaf data
 *   +3  ptr right  child or chain
 *
 * The ObjList nodes (+0x30 prim, +0x34 next-sibling, +0x38 first-child)
 * are the per-object record stored as the tree leaf payload.
 *
 * HIGH confidence on the small helpers; MED on FrameTick because it
 * pulls in several other globals (the puRam000007c4 trigger list, the
 * piRam0000075c live list, etc.) that are each ~LOW until pass 3.
 */
#include <stdint.h>
#include <stddef.h>

extern void Heap_Free(void *p);
extern void Object_FreeAndUnhook(void *p);    /* FUN_8001bddc */
extern int  Tree_LeafApply(uint32_t *leafData);  /* FUN_800200b8 */
extern void TriggerVol_FreeOne(int *node);    /* FUN_800203fc -- aliased */
extern int32_t **piRam00000774;
extern uint8_t   DAT_80065a74[];

/* HIGH: free entire kd-tree (post-order). */
void Tree_Free(int node)
{
    while (node != 0) {
        TriggerVol_FreeOne((int *)(intptr_t)node);
        Object_FreeAndUnhook(*(void **)(node + 0x30));
        Tree_Free(*(int *)(node + 0x38));
        int next = *(int *)(node + 0x34);
        Heap_Free((void *)(intptr_t)node);
        node = next;
    }
}

/* HIGH: recurse into the kd-tree, applying `LeafApply` to each leaf
 * payload. Short-circuit on the first non-zero return. */
int Tree_Apply(uint32_t *node, void *unused1, void *unused2)
{
    if (node[0] == 0) {
        return Tree_LeafApply(node + 1);
    }
    if (node[0] < 3) {
        int r = Tree_Apply((uint32_t *)(uintptr_t)node[2], unused1, unused2);
        if (r != 0) return r;
        r = Tree_Apply((uint32_t *)(uintptr_t)node[3], unused1, unused2);
        if (r != 0) return r;
    }
    (void)unused1; (void)unused2;
    return 0;
}

/* HIGH: tick every live object whose record has a callback at +0x64. */
extern int32_t **piRam0000075c;
void ObjList_TickAll(uint32_t tickArg)
{
    int *node = (int *)piRam0000075c[0];
    int **prev = piRam0000075c;
    while (node != NULL) {
        int payload = prev[2] != NULL ? *((int *)prev + 2) : 0;
        if (payload == 0) break;
        typedef void (*TickFn)(int payload, int mode, uint32_t arg);
        TickFn fn = *(TickFn *)((uintptr_t)payload + 100);
        if (fn != NULL) fn(payload, 0, tickArg);
        prev = (int **)node;
        node = (int *)*node;
    }
}

/* HIGH: pre-tick pass (sets a frame-counter via FUN_8001fcb4). */
extern int32_t **piRam0000077c;
extern void Object_FrameCounterBump(int payload, uint16_t arg);  /* FUN_8001fcb4 */
void ObjList_PreTickAll(uint16_t arg)
{
    int *node = (int *)piRam0000077c[0];
    int **prev = piRam0000077c;
    while (node != NULL) {
        Object_FrameCounterBump(*((int *)prev + 2), arg);
        prev = (int **)node;
        node = (int *)*node;
    }
}

/* MED: timed-expiry walker for the trigger-volume list. */
extern int   **piRam000007bc;
extern void   *puRam000007c4;
extern uint8_t DAT_80065ac0[];
void TriggerVol_ExpireFromHead(uint32_t now)
{
    while (puRam000007c4 != DAT_80065ac0) {
        int *node = (int *)piRam000007bc;
        if (now < (uint32_t)node[3]) return;
        uint32_t *payload = (uint32_t *)(intptr_t)node[2];
        *payload &= ~1u;
        int32_t *prevTail = (int32_t *)node[1];
        int32_t  back     = node[0];
        *(int32_t **)(back + 4) = prevTail;
        *prevTail               = back;
        int32_t **deadTail = piRam00000774;
        piRam00000774 = (int32_t **)node;
        *prevTail = (int32_t)(intptr_t)node;
        node[0] = (int32_t)(uintptr_t)DAT_80065a74;
        node[1] = (int32_t)(uintptr_t)deadTail;
    }
}
