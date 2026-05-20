/* object_list.c -- linked-list helpers for the per-frame object lists.
 *
 * Source: SLUS_005.10
 *   FUN_8001fe8c  -- ObjList_RemoveByPayload
 *   FUN_8001ff58  -- ObjList_FindBySpawnId
 *   FUN_8001ff0c  -- ObjList_FindByPayload
 *   FUN_800210a4  -- ObjList_RemoveFromTree (kd-tree variant)
 *   FUN_80020540  -- Object_Free
 *
 * V8 keeps live objects in singly-linked-list chains anchored at
 * various sentinels (DAT_80065a18 = "world", DAT_80065ac0 = "trigger
 * volumes", DAT_80065a60 = "back-buffer pending"). The 12-byte node
 * layout matches the deferred-free / damage path:
 *
 *   +0  i32 *back     (pointer back to the previous next-link slot)
 *   +1  i32 *next     (next node)
 *   +2  void *payload (object struct)
 *
 * ObjList_RemoveByPayload finds the node with matching payload,
 * splices it out, and re-links it onto the per-frame dead chain (the
 * tail at piRam00000774). ObjList_FindBy* return the node so the
 * caller can do the splice itself.
 *
 * ObjList_RemoveFromTree walks the spatial kd-tree (kind 1/2 split
 * nodes, kind 0 leaves containing a chain head) and removes the
 * payload from the first leaf that contains it.
 *
 * Object_Free runs the per-object teardown: if the object owns a
 * draw prim (obj[0x1a]), free it via Object_FreeAndUnhook; if it has
 * a HandlePair (bit 3 of flags), free that too via HandlePair_Free;
 * then deallocate via FUN_800204dc (the generic object Heap_Free).
 *
 * HIGH confidence: each function is a tight specialised loop.
 */
#include <stdint.h>
#include <stddef.h>

extern void  Object_FreeAndUnhook(void *p);             /* FUN_8001bddc */
extern void  HandlePair_Free(void **handle);            /* FUN_8003e2c4 */
extern void  Object_HeapFree(void *p);                  /* FUN_800204dc */
extern int32_t **piRam00000774;
extern uint8_t  DAT_80065a74[];

/* HIGH: find the list node whose payload pointer equals `payload`. */
int *ObjList_FindByPayload(int **listSentinel, int payload)
{
    int *node = (int *)listSentinel[0];
    int *next = (int *)*(int *)listSentinel[0];
    for (;;) {
        if (next == NULL) return NULL;
        if (node[2] == payload) return node;
        node = next;
        next = (int *)*next;
    }
}

/* HIGH: same, but uses obj.spawnId (i16 @ payload+6) as the match key
 * and *excludes* one specific payload pointer (skip self). */
int *ObjList_FindBySpawnId(int **listSentinel, int spawnId, int excludePayload)
{
    int *node = (int *)listSentinel[0];
    int *next = (int *)*(int *)listSentinel[0];
    for (;;) {
        if (next == NULL) return NULL;
        if (node[2] != excludePayload && *(int16_t *)(node[2] + 6) == spawnId)
            return node;
        node = next;
        next = (int *)*next;
    }
}

/* HIGH: find + splice + push onto dead list. */
uint32_t ObjList_RemoveByPayload(int **listSentinel, int payload)
{
    int *node = (int *)listSentinel[0];
    int *next = (int *)*(int *)listSentinel[0];
    for (;;) {
        if (next == NULL) return 0;
        if (node[2] == payload) break;
        node = next;
        next = (int *)*next;
    }
    int32_t *prevTail = (int32_t *)node[1];
    int32_t  back     = node[0];
    *(int32_t **)(back + 4) = prevTail;
    *prevTail               = back;

    int32_t **deadTail = piRam00000774;
    *piRam00000774 = (int32_t *)node;
    piRam00000774 = (int32_t **)node;
    node[1] = (int32_t)(uintptr_t)deadTail;
    node[0] = (int32_t)(uintptr_t)DAT_80065a74;
    node[2] = 0;
    return 1;
}

/* HIGH: recurse into a spatial-tree (kind 0=leaf, 1/2=split). At a
 * leaf node, defer to the linear ObjList_FindByPayload. */
int ObjList_RemoveFromTree(uint32_t *node, int payload)
{
    while (node[0] != 0) {
        if (node[0] > 2) return 0;
        if (ObjList_RemoveFromTree((uint32_t *)(uintptr_t)node[2], payload) != 0)
            return 1;
        node = (uint32_t *)(uintptr_t)node[3];
    }
    ObjList_FindByPayload((int **)(node + 1), payload);
    return 0;
}

/* HIGH: per-object teardown. */
void Object_Free(uint32_t *obj)
{
    if (obj[0x1a] != 0) Object_FreeAndUnhook((void *)(uintptr_t)obj[0x1a]);
    if ((obj[0] & 8u) != 0) HandlePair_Free((void **)(uintptr_t)obj[0x1c]);
    Object_HeapFree(obj);
}
