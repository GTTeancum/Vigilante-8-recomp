/* object_broadcast.c -- per-object event broadcast helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8002185c  -- Object_BroadcastToTree
 *   FUN_80021924  -- Object_BroadcastWorldOrTree
 *   FUN_80022c54  -- Object_AppendToScratchList
 *
 * Object_BroadcastToTree:
 *   Walks the level kd-tree (uRam000006fc) and invokes the per-object
 *   dispatch (FUN_800211a4 -- still in pass-3 backlog) on every node
 *   whose payload matches the given event.
 *
 * Object_BroadcastWorldOrTree:
 *   First tries the world list (DAT_80065a18); only on miss falls
 *   through to the tree walk. This is the *fast-path* version used by
 *   handlers that expect the event recipient to be a top-level world
 *   object.
 *
 * Object_AppendToScratchList:
 *   Push a single object payload onto the head of piRam0000076c (the
 *   per-frame scratch list). 12-byte node layout: back-link, payload,
 *   reserved.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void Object_DispatchTree(uintptr_t treeRoot, uint32_t event, uint32_t arg);  /* FUN_800211a4 */
extern int  Object_DispatchList(void *listHead, uint32_t event, uint32_t arg);      /* FUN_8002002c */
extern uintptr_t uRam000006fc;
extern uint8_t   DAT_80065a18[];
extern uint8_t   DAT_80065a70[];
extern int32_t **piRam0000076c;

void Object_BroadcastToTree(uint32_t event, uint32_t arg)
{
    Object_DispatchTree(uRam000006fc, event, arg);
}

void Object_BroadcastWorldOrTree(uint32_t event, uint32_t arg)
{
    if (Object_DispatchList(DAT_80065a18, event, arg) == 0) {
        Object_DispatchTree(uRam000006fc, event, arg);
    }
}

void Object_AppendToScratchList(int payload)
{
    int32_t *node = *piRam0000076c;
    *(int32_t **)((uintptr_t)node + 4) = (int32_t *)DAT_80065a70;
    piRam0000076c = (int32_t **)node;
    node[2] = payload;
}
