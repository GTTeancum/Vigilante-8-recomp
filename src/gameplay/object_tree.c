/* object_tree.c -- object-list free, kd-tree walker.
 *
 * Source: SLUS_005.10
 *   FUN_80020658  -- Tree_Free: walk a doubly-linked object-list ring and
 *                    free every entry (Object_Free + recycle to dead pool).
 *   FUN_8002123c  -- Tree_Apply: recurse a kd-tree calling FUN_800200b8 at
 *                    each leaf (passes callback + arg through).
 *   FUN_800200b8  -- ObjChain_WalkCb: walk a singly-linked chain, calling
 *                    a callback on each node; stop on non-zero return.
 *
 * HIGH confidence: direct Ghidra port.
 *
 * Node layout for the object-list ring (sentinel + entries):
 *   node[0]  (int) = next ptr
 *   node[1]  (int) = prev ptr
 *   node[2]  (int) = payload / object ptr
 *
 * Sentinel[2] is the sentinel itself when the list is empty; the loop
 * condition "sentinel[2] != sentinel" detects a non-empty list.
 */
#include <stdint.h>
#include <stddef.h>

/* ---- dead-node recycler (shared with frame_tick.c, object_lifecycle_extra.c) ---- */
extern int32_t  *piRam00000774;    /* dead-node pool tail */
extern uint8_t   DAT_80065a74[];   /* dead-node pool sentinel value */
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

/* ---- object destructor ---- */
extern void FUN_80020540(int param_1);  /* Object_Free */

typedef struct ObjectTreeHostNode {
    struct ObjectTreeHostNode *next;
    struct ObjectTreeHostNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} ObjectTreeHostNode;

static int object_tree_host_ptr_valid(const void *p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + Host_HeapSize();
    uintptr_t v = (uintptr_t)p;

    return v == 0 || (v >= base && v < end);
}

/* ================================================================
 * FUN_80020658  -- Tree_Free
 *
 * Walk the doubly-linked object-list ring rooted at param_1.
 * For each node: free the object payload, unlink the node from
 * the ring, then recycle the node into the dead-node pool.
 * Stops when the sentinel's [2] field equals the sentinel itself
 * (i.e. the ring is empty).
 * ================================================================ */
void FUN_80020658(uint32_t *param_1)
{
    ObjectTreeHostNode *sentinel = (ObjectTreeHostNode *)param_1;
    ObjectTreeHostNode *node;

    (void)piRam00000774;
    (void)DAT_80065a74;
    if (sentinel == NULL || sentinel->next == NULL || sentinel->prev == NULL)
        return;

    node = sentinel->next;
    while (node != NULL && node != sentinel) {
        ObjectTreeHostNode *next;
        if (!object_tree_host_ptr_valid(node)) {
            break;
        }
        next = node->next;
        if (!object_tree_host_ptr_valid(next) ||
            !object_tree_host_ptr_valid(node->prev))
            break;
        if (node->payload != 0)
            FUN_80020540((int)node->payload);
        node->next = NULL;
        node->prev = NULL;
        node->payload = 0;
        node = next;
    }
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    sentinel->payload = 0;
}

/* Public alias used by level_teardown.c. */
void Tree_Free(void *root) { FUN_80020658((uint32_t *)root); }

/* ================================================================
 * FUN_80020968 -- Tree_FreeTerrain
 *
 * Recursively frees the terrain kd-tree.  Each interior node has
 * children at param_1[2] / param_1[3].  Leaf nodes (param_1[0] == 0)
 * carry an object list at param_1+1 which is drained via FUN_80020658
 * (Tree_Free).  After children are processed, the node itself is heap-
 * freed.
 *
 * HIGH confidence (direct Ghidra port).
 * ================================================================ */
extern void Heap_Free(void *p);                                 /* FUN_80045088 */

static void Tree_FreeSourceLeafList(uint32_t *list)
{
    uint32_t sentinel;

    if (list == NULL)
        return;
    sentinel = (uint32_t)(uintptr_t)list;
    while (list[2] != sentinel) {
        uint32_t nodeAddr = list[0];
        uint32_t *node = (uint32_t *)(uintptr_t)nodeAddr;
        uint32_t next;
        uint32_t prev;

        if (node == NULL)
            break;
        next = node[0];
        prev = node[1];
        if (node[2] != 0)
            FUN_80020540((int)node[2]);
        if (next != 0)
            *(uint32_t *)(uintptr_t)(next + 4u) = prev;
        if (prev != 0)
            *(uint32_t *)(uintptr_t)prev = next;
        node[0] = 0;
        node[1] = 0;
        node[2] = 0;
        Heap_Free(node);
    }
    list[0] = (uint32_t)(uintptr_t)(list + 1);
    list[1] = 0;
    list[2] = sentinel;
}

void FUN_80020968(int *param_1)
{
    if (param_1 != (int *)0) {
        if (*param_1 == 0) {
            Tree_FreeSourceLeafList((uint32_t *)(param_1 + 1));
        } else {
            FUN_80020968((int *)(uintptr_t)param_1[2]);
            FUN_80020968((int *)(uintptr_t)param_1[3]);
        }
        Heap_Free(param_1);
    }
}

/* Public alias used by level_teardown.c. */
void Tree_FreeTerrain(void *root) { FUN_80020968((int *)root); }

/* ================================================================
 * FUN_800200b8  -- ObjChain_WalkCb
 *
 * Walk a singly-linked chain starting at *param_1 (the second
 * link is **param_1).  Call param_2(node, param_3) on each node;
 * stop when the next link is NULL or callback returns non-zero.
 *
 * Returns the last callback result (0 if never called or always 0).
 * ================================================================ */
typedef int (*ObjCb)(int *node, int32_t arg);

int FUN_800200b8(int *param_1, ObjCb param_2, int32_t param_3)
{
    int  iVar3;
    int *piVar1;
    int *piVar2;

    piVar1 = (int *)(uintptr_t)*param_1;
    piVar2 = (int *)(uintptr_t)*(int32_t *)(uintptr_t)*param_1;
    iVar3  = 0;
    while (piVar2 != (int *)0 &&
           (iVar3 = param_2(piVar1, param_3), iVar3 == 0)) {
        piVar1 = piVar2;
        piVar2 = (int *)(uintptr_t)*piVar2;
    }
    return iVar3;
}

/* ================================================================
 * FUN_8002123c  -- Tree_Apply
 *
 * Recurse a kd-tree (nodes where param_1[0] < 3 have children at
 * param_1[2] / param_1[3]).  At leaf nodes (param_1[0] == 0) call
 * ObjChain_WalkCb on the leaf chain (param_1+1) with the supplied
 * callback and argument.  Short-circuits on first non-zero return.
 * ================================================================ */
int FUN_8002123c(uint32_t *param_1, ObjCb param_2, int32_t param_3)
{
    int iVar1;

    if (*param_1 == 0) {
        iVar1 = FUN_800200b8((int *)(param_1 + 1), param_2, param_3);
    } else {
        iVar1 = 0;
        if ((*param_1 < 3) &&
            (iVar1 = FUN_8002123c((uint32_t *)(uintptr_t)param_1[2], param_2, param_3),
             iVar1 == 0) &&
            (iVar1 = FUN_8002123c((uint32_t *)(uintptr_t)param_1[3], param_2, param_3),
             iVar1 == 0)) {
            iVar1 = 0;
        }
    }
    return iVar1;
}

/* Public alias for callers that use the semantic name. */
int Tree_Apply(uint32_t *node, ObjCb cb, int32_t arg)
    { return FUN_8002123c(node, cb, arg); }

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80020658 --- */
void FUN_80020658(undefined4 *param_1)
{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;

  puVar2 = (undefined4 *)param_1[2];
  while (puVar2 != param_1) {
    piVar5 = (int *)*param_1;
    FUN_80020540(piVar5[2]);
    piVar4 = (int *)piVar5[1];
    iVar3 = *piVar5;
    *(int **)(iVar3 + 4) = piVar4;
    *piVar4 = iVar3;
    piVar4 = piRam00000774;
    piVar1 = piVar5;
    *piRam00000774 = (int)piVar5;
    piRam00000774 = piVar1;
    piVar5[1] = (int)piVar4;
    *piVar5 = (int)&DAT_80065a74;
    piVar5[2] = 0;
    puVar2 = (undefined4 *)param_1[2];
  }
}

/* --- SLUS_005.10 FUN_800200b8 --- */
void FUN_800200b8(int *param_1,code *param_2,undefined4 param_3)
{
  int *piVar1;
  int *piVar2;
  int iVar3;

  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while ((piVar2 != (int *)0x0 && (iVar3 = (*param_2)(piVar1,param_3), iVar3 == 0))) {
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return;
}

/* --- SLUS_005.10 FUN_8002123c --- */
int FUN_8002123c(uint *param_1,undefined4 param_2,undefined4 param_3)
{
  int iVar1;

  if (*param_1 == 0) {
    iVar1 = FUN_800200b8(param_1 + 1);
  }
  else {
    iVar1 = 0;
    if (((*param_1 < 3) && (iVar1 = FUN_8002123c(param_1[2],param_2,param_3), iVar1 == 0)) &&
       (iVar1 = FUN_8002123c(param_1[3],param_2,param_3), iVar1 == 0)) {
      iVar1 = 0;
    }
  }
  return iVar1;
}

#endif  /* GHIDRA REF */
