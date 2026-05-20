/* list_walkers.c -- generic walker helpers for the object linked lists.
 *
 * Source: SLUS_005.10
 *   FUN_8001fcb4  -- Object_FrameCounterBump (pre-tick recursion)
 *   FUN_80020658  -- ObjList_FreeAllAndRetire
 *   FUN_80020968  -- Tree_FreeRecursive
 *   FUN_800206f0  -- ObjList_ApplyDestroy
 *   FUN_800200b8  -- ObjList_FindWithPredicate
 *
 * These small helpers underpin the tree traversal in object_tree.c.
 * Each walks a different chain anchored at a sentinel and either
 * frees nodes or invokes a per-node callback.
 *
 * HIGH confidence (each is a tight specialised loop).
 */
#include <stdint.h>
#include <stddef.h>

extern int  Object_PreTickRecurse(int payload, uint16_t arg);   /* FUN_8001f9cc */
extern void Object_PreTickChildren(int chainHead, uint16_t arg); /* FUN_8001fc38 */
extern void Object_Free(uint32_t *obj);                         /* FUN_80020540 */
extern void Object_Destroy(int payload);                        /* FUN_8001de08 */
extern int32_t **piRam00000774;

/* HIGH: recursive object pre-tick. Applies the per-object pre-tick to
 * the node, then if its child list is populated recurses into them. */
void Object_FrameCounterBump(int payload, uint16_t arg)
{
    int r = Object_PreTickRecurse(payload, arg);
    if (r < 0 || *(int *)(payload + 0x38) == 0) return;
    Object_PreTickChildren(*(int *)(payload + 0x38), arg);
}

/* HIGH: walk an object list, freeing each payload and re-linking the
 * node onto the dead chain. */
extern uint8_t DAT_80065a74[];
void ObjList_FreeAllAndRetire(int **listSentinel)
{
    int *next = (int *)listSentinel[2];
    while (next != (int *)listSentinel) {
        int *node = (int *)*listSentinel;
        Object_Free((uint32_t *)(intptr_t)node[2]);
        int32_t *prev = (int32_t *)node[1];
        int32_t  back = node[0];
        *(int32_t **)(back + 4) = prev;
        *prev = back;

        int32_t **deadTail = piRam00000774;
        piRam00000774 = (int32_t **)node;
        *prev = (int32_t)(intptr_t)node;
        next = (int *)listSentinel[2];
        (void)deadTail;
    }
}

/* HIGH: free a whole kd-tree, including the per-leaf object lists. */
void Tree_FreeRecursive(int *node)
{
    if (node == NULL) return;
    if (node[0] == 0) {
        ObjList_FreeAllAndRetire((int **)(node + 1));
    } else {
        Tree_FreeRecursive((int *)(intptr_t)node[2]);
        Tree_FreeRecursive((int *)(intptr_t)node[3]);
    }
    extern void Heap_Free(void *p);
    Heap_Free(node);
}

/* HIGH: walk a list, calling Object_Destroy on each payload. */
void ObjList_ApplyDestroy(int **listSentinel)
{
    int *node = (int *)listSentinel[0];
    int *next = (int *)*(int *)listSentinel[0];
    for (; next != NULL; next = (int *)*next) {
        Object_Destroy(node[2]);
        node = next;
    }
}

/* HIGH: walk a list, applying a predicate; stop at first non-zero
 * return. (`ObjList_FindWithPredicate` family used by Tree_Apply.) */
int ObjList_FindWithPredicate(int **listSentinel,
                              int (*pred)(int *node, uint32_t arg),
                              uint32_t arg)
{
    int *node = (int *)listSentinel[0];
    int *next = (int *)*(int *)listSentinel[0];
    while (next != NULL) {
        int r = pred(node, arg);
        if (r != 0) return r;
        node = next;
        next = (int *)*next;
    }
    return 0;
}
