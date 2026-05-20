/* damage_apply.c -- damage-apply path and object-list movement helpers.
 *
 * Source: SLUS_005.10
 *   FUN_800205f8  -- Damage_Apply: trivial dispatcher.
 *   FUN_8002179c  -- Damage_RouteByTree: find owning spatial tree node.
 *   FUN_800205a0  -- Object_RetireToDeadList: unhook + link into dead list.
 *   FUN_80020778  -- Object_ClearBackBufferFlag.
 *
 * When something takes damage (collision, weapon hit, etc.), the chain:
 *   Damage_Apply           (entry point)
 *     -> Damage_RouteByTree (finds the relevant chunk in the kd-tree)
 *     -> Object_RetireToDeadList (unhooks the object from its live list
 *                                 and appends to the dead chain so the
 *                                 next visibility pass can free it)
 *
 * Object list-link layout (4 ints):
 *   [0]  back ptr (prev node's ptr-to-next is updated to skip self)
 *   [1]  next ptr
 *   [2]  payload pointer (the actual object struct)
 *   [3]  (unused on the dead list; set to 0)
 *
 * Object_ClearBackBufferFlag clears bit 7 of obj.flags then removes the
 * object from the back-buffer-pending list at DAT_80065a60.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

extern int  ObjectList_RemoveTraverse(void *listHead, void *obj);     /* FUN_8001ff0c */
extern void ObjectList_RemoveFromChain(void *root, void *obj);        /* FUN_800210a4 */
extern void ObjectList_RemoveFromBackbuf(void *listHead);             /* FUN_8001fe8c */
extern void FreeAfterNFrames(int handle);                             /* FUN_80020540 */

extern uint8_t  DAT_80065a18[];
extern uintptr_t uRam000006fc;     /* terrain kd-tree root */
extern int32_t  iRam0000000c;      /* current frame counter epoch */
extern int32_t **piRam0000076c;    /* free-pool list head */
extern int32_t **piRam00000774;    /* dead list tail */
extern uint8_t  DAT_80065a70[];    /* sentinel for dead list */
extern uint8_t  DAT_80065a74[];    /* sentinel for retired list */

/* HIGH: trivial dispatcher. */
void Damage_Apply(void *obj)
{
    /* FUN_8002179c returns void in the binary (extracted from $v0
     * via Ghidra-inserted aux variable); the chained call to
     * FUN_800205a0 takes that "result" implicitly. Preserve verbatim. */
    Damage_RouteByTree(obj);
    Object_RetireToDeadList((int *)obj);
}

/* HIGH: try removing the object from the level-wide list; if it
 * wasn't there, fall back to the terrain kd-tree's containing chunk. */
extern void Object_RetireToDeadList(int *node);
void Damage_RouteByTree(void *obj)
{
    if (ObjectList_RemoveTraverse(DAT_80065a18, obj) == 0) {
        ObjectList_RemoveFromChain((void *)uRam000006fc, obj);
    }
}

/* HIGH: unlink `node` from its current chain and append to the dead
 * list. The dead list is processed each tick by the "retired sweeper"
 * which actually frees the payload after a brief grace period. */
void Object_RetireToDeadList(int *node)
{
    if (node == NULL) return;
    int32_t *prevTail = (int32_t *)node[1];
    int32_t  back     = node[0];
    int32_t  payload  = node[2];

    /* Patch out: prevTail->next = back; back->next ptr = prevTail. */
    *(int32_t **)(back + 4) = prevTail;
    *prevTail               = back;

    /* Push onto the dead list. */
    int32_t **deadTail = piRam00000774;
    *piRam00000774 = (int32_t *)node;
    piRam00000774 = (int32_t **)node;
    node[1] = (int32_t)(uintptr_t)deadTail;
    node[0] = (int32_t)(uintptr_t)DAT_80065a74;
    node[2] = 0;

    FreeAfterNFrames(payload);
}

/* HIGH: clear the "back-buffer pending" bit (bit 7) of an object.
 * If it was set, also dequeue the object from the back-buffer list. */
uint32_t Object_ClearBackBufferFlag(uint32_t *obj)
{
    extern uint8_t DAT_80065a60[];
    if ((obj[0] & 0x80u) == 0) return 0;
    obj[0] &= ~0x80u;
    ObjectList_RemoveFromBackbuf(DAT_80065a60);
    return 0;
}
