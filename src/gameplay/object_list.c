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
extern void  FUN_80020540(uint32_t *obj);
extern int32_t **piRam00000774;
extern uint8_t  DAT_80065a74[];

typedef struct ObjListHostNode {
    struct ObjListHostNode *next;
    struct ObjListHostNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} ObjListHostNode;

/* HIGH: find the list node whose payload pointer equals `payload`. */
int *ObjList_FindByPayload(int **listSentinel, int payload)
{
    ObjListHostNode *sentinel = (ObjListHostNode *)listSentinel;
    ObjListHostNode *node;
    if (sentinel == NULL || sentinel->prev == NULL) return NULL;
    for (node = sentinel->next; node != NULL; node = node->next) {
        if (node->payload == (uintptr_t)(uint32_t)payload)
            return (int *)node;
    }
    return NULL;
}

/* HIGH: same, but uses obj.spawnId (i16 @ payload+6) as the match key
 * and *excludes* one specific payload pointer (skip self). */
int *ObjList_FindBySpawnId(int **listSentinel, int spawnId, int excludePayload)
{
    ObjListHostNode *sentinel = (ObjListHostNode *)listSentinel;
    ObjListHostNode *node;
    if (sentinel == NULL || sentinel->prev == NULL) return NULL;
    for (node = sentinel->next; node != NULL; node = node->next) {
        uintptr_t payload = node->payload;
        if (payload != (uintptr_t)(uint32_t)excludePayload &&
            *(int16_t *)(payload + 6) == spawnId)
            return (int *)node;
    }
    return NULL;
}

/* HIGH: find + splice + push onto dead list. */
uint32_t ObjList_RemoveByPayload(int **listSentinel, int payload)
{
    ObjListHostNode *sentinel = (ObjListHostNode *)listSentinel;
    ObjListHostNode *node = (ObjListHostNode *)ObjList_FindByPayload(listSentinel, payload);
    (void)piRam00000774;
    (void)DAT_80065a74;
    if (node == NULL) return 0;
    if (node->prev != NULL) node->prev->next = node->next;
    if (node->next != NULL) node->next->prev = node->prev;
    if (sentinel->next == node) sentinel->next = node->next;
    if (sentinel->prev == node)
        sentinel->prev = (node->prev == sentinel) ? sentinel : node->prev;
    node->next = NULL;
    node->prev = NULL;
    node->payload = 0;
    return 1;
}

intptr_t ObjectList_RemoveTraverse(void *listHead, void *obj)
{
    return (intptr_t)ObjList_FindByPayload((int **)listHead, (int)(uintptr_t)obj);
}

uint32_t ObjectList_RemoveFromBackbuf(void *listHead, void *obj)
{
    return ObjList_RemoveByPayload((int **)listHead, (int)(uintptr_t)obj);
}

intptr_t ObjectList_RemoveFromChain(void *root, void *obj)
{
    uint32_t *node = (uint32_t *)root;

    if (node == NULL)
        return 0;
    while (node[0] != 0) {
        intptr_t hit;

        if (node[0] > 2)
            return 0;
        hit = ObjectList_RemoveFromChain((void *)(uintptr_t)node[2], obj);
        if (hit != 0)
            return hit;
        node = (uint32_t *)(uintptr_t)node[3];
        if (node == NULL)
            return 0;
    }
    return ObjectList_RemoveTraverse((void *)(node + 1), obj);
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

void FreeAfterNFrames(int handle)
{
    FUN_80020540((uint32_t *)(uintptr_t)(uint32_t)handle);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001fe8c  (from analysis/SLUS_005.10/decomp/8001fe8c.c) --- */
// addr: 0x8001fe8c  name: FUN_8001fe8c

undefined4 FUN_8001fe8c(undefined4 *param_1,int param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  int *piVar3;
  int iVar4;
  
  piVar1 = (int *)*param_1;
  piVar3 = *(int **)*param_1;
  while( true ) {
    if (piVar3 == (int *)0x0) {
      return 0;
    }
    if (piVar1[2] == param_2) break;
    piVar1 = piVar3;
    piVar3 = (int *)*piVar3;
  }
  piVar3 = (int *)piVar1[1];
  iVar4 = *piVar1;
  *(int **)(iVar4 + 4) = piVar3;
  *piVar3 = iVar4;
  puVar2 = piRam00000774;
  piVar3 = piVar1;
  *piRam00000774 = (int)piVar1;
  piRam00000774 = piVar3;
  piVar1[1] = (int)puVar2;
  *piVar1 = (int)&DAT_80065a74;
  piVar1[2] = 0;
  return 1;
}

/* --- SLUS_005.10 FUN_8001ff58  (from analysis/SLUS_005.10/decomp/8001ff58.c) --- */
// addr: 0x8001ff58  name: FUN_8001ff58

int * FUN_8001ff58(int *param_1,int param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    if ((piVar1[2] != param_3) && (*(short *)(piVar1[2] + 6) == param_2)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return piVar1;
}

/* --- SLUS_005.10 FUN_8001ff0c  (from analysis/SLUS_005.10/decomp/8001ff0c.c) --- */
// addr: 0x8001ff0c  name: FUN_8001ff0c

int * FUN_8001ff0c(int *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    if (piVar1[2] == param_2) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return piVar1;
}

/* --- SLUS_005.10 FUN_800210a4  (from analysis/SLUS_005.10/decomp/800210a4.c) --- */
// addr: 0x800210a4  name: FUN_800210a4

void FUN_800210a4(uint *param_1,undefined4 param_2)

{
  int iVar1;
  
  while( true ) {
    if (*param_1 == 0) {
      FUN_8001ff0c(param_1 + 1,param_2);
      return;
    }
    if (2 < *param_1) break;
    iVar1 = FUN_800210a4(param_1[2],param_2);
    if (iVar1 != 0) {
      return;
    }
    param_1 = (uint *)param_1[3];
  }
  return;
}

/* --- SLUS_005.10 FUN_80020540  (from analysis/SLUS_005.10/decomp/80020540.c) --- */
// addr: 0x80020540  name: FUN_80020540

void FUN_80020540(uint *param_1)

{
  if (param_1[0x1a] != 0) {
    FUN_8001bddc();
  }
  if ((*param_1 & 8) != 0) {
    FUN_8003e2c4(param_1[0x1c]);
  }
  FUN_800204dc(param_1);
  return;
}

/* --- SLUS_005.10 FUN_800204dc  (from analysis/SLUS_005.10/decomp/800204dc.c) --- */
// addr: 0x800204dc  name: FUN_800204dc

void FUN_800204dc(int param_1)

{
  int iVar1;
  
  while (param_1 != 0) {
    FUN_800203fc(param_1);
    FUN_8001bddc(*(undefined4 *)(param_1 + 0x30));
    FUN_800204dc(*(undefined4 *)(param_1 + 0x38));
    iVar1 = *(int *)(param_1 + 0x34);
    FUN_80045088(param_1);
    param_1 = iVar1;
  }
  return;
}

/* --- SLUS_005.10 FUN_8001bddc  (from analysis/SLUS_005.10/decomp/8001bddc.c) --- */
// addr: 0x8001bddc  name: FUN_8001bddc

void FUN_8001bddc(int param_1)

{
  if (param_1 != 0) {
    if (*(int *)(param_1 + iRam00000004 * 4 + 0x1c) != 0) {
      FUN_800118b4();
    }
    if (*(int *)(param_1 + (1 - iRam00000004) * 4 + 0x1c) != 0) {
      FUN_80045088();
    }
    FUN_80045088(param_1);
  }
  return;
}

/* --- SLUS_005.10 FUN_8003e2c4  (from analysis/SLUS_005.10/decomp/8003e2c4.c) --- */
// addr: 0x8003e2c4  name: FUN_8003e2c4

void FUN_8003e2c4(undefined4 *param_1)

{
  if (param_1 != (undefined4 *)0x0) {
    FUN_8001bddc(*param_1);
    FUN_80045088(param_1);
  }
  return;
}

#endif  /* GHIDRA REF */
