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

extern intptr_t ObjectList_RemoveTraverse(void *listHead, void *obj); /* FUN_8001ff0c */
extern intptr_t ObjectList_RemoveFromChain(void *root, void *obj);    /* FUN_800210a4 */
extern uint32_t ObjectList_RemoveFromBackbuf(void *listHead, void *obj); /* FUN_8001fe8c */
extern void FreeAfterNFrames(int handle);                             /* FUN_80020540 */

extern uint8_t  DAT_80065a18[];
extern uint8_t  DAT_80065a60[];
extern uint8_t  DAT_80065a80[];
extern uint8_t  DAT_80065ac0[];
extern uintptr_t uRam000006fc;     /* terrain kd-tree root */
extern int32_t  iRam0000000c;      /* current frame counter epoch */
extern int32_t **piRam0000076c;    /* free-pool list head */
extern int32_t **piRam00000774;    /* dead list tail */
extern uint8_t  DAT_80065a70[];    /* sentinel for dead list */
extern uint8_t  DAT_80065a74[];    /* sentinel for retired list */

typedef struct DamageHostObjListNode {
    struct DamageHostObjListNode *next;
    struct DamageHostObjListNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} DamageHostObjListNode;

/* Forward declarations -- definitions below. */
intptr_t Damage_RouteByTree(void *obj);
void Object_RetireToDeadList(intptr_t node);

static void damage_update_sentinel(uint8_t *head,
                                   DamageHostObjListNode *node,
                                   DamageHostObjListNode *prev,
                                   DamageHostObjListNode *next)
{
    DamageHostObjListNode *sentinel = (DamageHostObjListNode *)head;
    if (sentinel->next == node)
        sentinel->next = next;
    if (sentinel->prev == node)
        sentinel->prev = (prev == sentinel) ? sentinel : prev;
}

/* HIGH: trivial dispatcher. */
void Damage_Apply(void *obj)
{
    intptr_t node = Damage_RouteByTree(obj);
    Object_RetireToDeadList(node);
}

/* Hex-name alias -- callers (effect_death_ticks.c etc.) use PSX address. */
void FUN_800205f8(int obj) { Damage_Apply((void *)(uintptr_t)(uint32_t)obj); }

/* HIGH: try removing the object from the level-wide list; if it
 * wasn't there, fall back to the terrain kd-tree's containing chunk. */
intptr_t Damage_RouteByTree(void *obj)
{
    intptr_t node = ObjectList_RemoveTraverse(DAT_80065a18, obj);
    if (node == 0)
        node = ObjectList_RemoveFromChain((void *)uRam000006fc, obj);
    return node;
}

/* HIGH: unlink `node` from its current chain and append to the dead
 * list. The dead list is processed each tick by the "retired sweeper"
 * which actually frees the payload after a brief grace period. */
void Object_RetireToDeadList(intptr_t nodePtr)
{
    DamageHostObjListNode *node = (DamageHostObjListNode *)(uintptr_t)nodePtr;
    uintptr_t payload;

    if (node == NULL) return;

    payload = node->payload;
    DamageHostObjListNode *prev = node->prev;
    DamageHostObjListNode *next = node->next;
    if (prev != NULL)
        prev->next = next;
    if (next != NULL)
        next->prev = prev;
    damage_update_sentinel(DAT_80065a18, node, prev, next);
    damage_update_sentinel(DAT_80065a60, node, prev, next);
    damage_update_sentinel(DAT_80065a80, node, prev, next);
    damage_update_sentinel(DAT_80065ac0, node, prev, next);
    node->next = NULL;
    node->prev = NULL;
    node->payload = 0;

    (void)iRam0000000c;
    (void)piRam0000076c;
    (void)piRam00000774;
    (void)DAT_80065a70;
    (void)DAT_80065a74;
    if (payload != 0)
        FreeAfterNFrames((int)payload);
}

/* HIGH: clear the "back-buffer pending" bit (bit 7) of an object.
 * If it was set, also dequeue the object from the back-buffer list. */
uint32_t Object_ClearBackBufferFlag(uint32_t *obj)
{
    extern uint8_t DAT_80065a60[];
    if ((obj[0] & 0x80u) == 0) return 0;
    obj[0] &= ~0x80u;
    ObjectList_RemoveFromBackbuf(DAT_80065a60, obj);
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800205f8  (from analysis/SLUS_005.10/decomp/800205f8.c) --- */
// addr: 0x800205f8  name: FUN_800205f8

void FUN_800205f8(void)

{
  undefined4 uVar1;
  
  uVar1 = FUN_8002179c();
  FUN_800205a0(uVar1);
  return;
}

/* --- SLUS_005.10 FUN_8002179c  (from analysis/SLUS_005.10/decomp/8002179c.c) --- */
// addr: 0x8002179c  name: FUN_8002179c

void FUN_8002179c(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_8001ff0c(&DAT_80065a18,param_1);
  if (iVar1 == 0) {
    FUN_800210a4(uRam000006fc,param_1);
  }
  return;
}

/* --- SLUS_005.10 FUN_800205a0  (from analysis/SLUS_005.10/decomp/800205a0.c) --- */
// addr: 0x800205a0  name: FUN_800205a0

void FUN_800205a0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  if (param_1 != (int *)0x0) {
    piVar3 = (int *)param_1[1];
    iVar2 = *param_1;
    iVar4 = param_1[2];
    *(int **)(iVar2 + 4) = piVar3;
    *piVar3 = iVar2;
    piVar3 = piRam00000774;
    piVar1 = param_1;
    *piRam00000774 = (int)param_1;
    piRam00000774 = piVar1;
    param_1[1] = (int)piVar3;
    *param_1 = (int)&DAT_80065a74;
    param_1[2] = 0;
    FUN_80020540(iVar4);
  }
  return;
}

/* --- SLUS_005.10 FUN_80020778  (from analysis/SLUS_005.10/decomp/80020778.c) --- */
// addr: 0x80020778  name: FUN_80020778

undefined4 FUN_80020778(uint *param_1)

{
  undefined4 uVar1;
  
  if ((*param_1 & 0x80) == 0) {
    uVar1 = 0;
  }
  else {
    *param_1 = *param_1 & 0xffffff7f;
    uVar1 = FUN_8001fe8c(&DAT_80065a60);
  }
  return uVar1;
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

#endif  /* GHIDRA REF */
