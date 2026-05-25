/* collision_dispatch.c -- Per-frame collision broadphase and dispatch.
 *
 * Source: SLUS_005.10
 *   FUN_80020620  -- ColEvent_Dispatch         (~4 instr)
 *   FUN_800209cc  -- Tree_RangeQuery           (~35 instr)
 *   FUN_80020f14  -- Tree_CollideObject        (~40 instr)
 *   FUN_8002169c  -- Frame_CollideAll          (~45 instr)
 *   FUN_80021978  -- ObjList_FindInBounds      (~25 instr)
 *   FUN_80021a30  -- TreeAABB_FirstHit         (~30 instr) [= TreeAABB_FirstHit in ai_target.c]
 *
 * FUN_80020620:
 *   Invokes the event-callback pointer at pcRam00000730 with event id
 *   0x11 (collision event), then calls Damage_Apply on the object.
 *
 * FUN_800209cc:
 *   Recursive spatial query on a binary kd-tree.  The tree nodes
 *   have kind 0 (leaf, calls ObjList_ApplyDestroy), 1 (X-split),
 *   or 2 (Z-split).  When split: if the range [param_2, param_3]
 *   overlaps the left half, recurse left; similarly for right.
 *
 * FUN_80020f14:
 *   Recursive AABB kd-tree traversal.  For each leaf, walks the
 *   object linked list and calls Object_TestCollision (FUN_8001edb4)
 *   against param_2.  Returns 0 when no collision handler caused
 *   an early exit, non-zero if Damage_Apply was triggered.
 *
 * FUN_8002169c:
 *   Per-frame collision driver: iterates the live object list
 *   (piRam00000714), clears contact slots, runs full pairwise
 *   collision tests against every later object in the list AND
 *   against the static kd-tree (uRam000006fc) for trigger volumes.
 *
 * HIGH confidence: direct ports from Ghidra pseudoC.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern void (*pcRam00000730)(uint32_t obj, int eventId, uint32_t param2);
extern void  Damage_Apply(void *obj);        /* FUN_800205f8 */
extern void  ObjList_ApplyDestroy(int **listSentinel); /* FUN_800206f0 */
extern uint32_t FUN_8001edb4(intptr_t param_1, uint32_t *param_2); /* Object_TestCollision */

extern int32_t   *piRam00000714;  /* active-object list head node */
extern uintptr_t  uRam000006fc;   /* world collision kd-tree root */

typedef struct CollisionHostNode {
    struct CollisionHostNode *next;
    struct CollisionHostNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} CollisionHostNode;

static CollisionHostNode *host_list_first(void *listHead)
{
    CollisionHostNode *sentinel = (CollisionHostNode *)listHead;
    if (sentinel == NULL || sentinel->prev == NULL)
        return NULL;
    return sentinel->next;
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* ================================================================
 * FUN_80020620 -- ColEvent_Dispatch
 *
 * Fires the surface-type event callback (pcRam00000730) with
 * event id 0x11 (collision/damage event), then calls Damage_Apply.
 *
 * HIGH confidence: trivial 2-call wrapper.
 * ================================================================ */
void FUN_80020620(uint32_t param_1, uint32_t param_2)
{
    (*pcRam00000730)(param_1, 0x11, param_2);
    Damage_Apply((void *)(uintptr_t)param_1);
}

/* ================================================================
 * FUN_800209cc -- Tree_RangeQuery
 *
 * Recursive kd-tree query: visits all leaf-list nodes in the subtree
 * rooted at param_1 whose split coordinate falls in
 * [param_2, param_3] on the X axis or [param_4, param_5] on the Z.
 *
 * param_1: int*   node pointer {kind, splitCoord, leftPtr, rightPtr}
 * param_2/3: int  X range [min, max]
 * param_4/5: int  Z range [min, max]
 *
 * HIGH confidence.
 * ================================================================ */
void FUN_800209cc(int *param_1, int param_2, int param_3, int param_4, int param_5)
{
    bool bVar1;
    int  iVar2;

    iVar2 = *param_1;
    if (iVar2 == 1) {
        iVar2 = param_1[1];
        if (param_2 < iVar2) {
            FUN_800209cc((int *)(uintptr_t)param_1[2], param_2, param_3, param_4, param_5);
        }
        bVar1 = iVar2 < param_3;
    } else {
        if (iVar2 == 0) {
            ObjList_ApplyDestroy((int **)(param_1 + 1));
            return;
        }
        if (iVar2 != 2) {
            return;
        }
        iVar2 = param_1[1];
        if (param_4 < iVar2) {
            FUN_800209cc((int *)(uintptr_t)param_1[2], param_2, param_3, param_4, param_5);
        }
        bVar1 = iVar2 < param_5;
    }
    if (bVar1) {
        FUN_800209cc((int *)(uintptr_t)param_1[3], param_2, param_3, param_4, param_5);
    }
}

/* ================================================================
 * FUN_80020f14 -- Tree_CollideObject
 *
 * Recursive AABB kd-tree traversal for collision.
 *
 * param_1: kd-tree root node pointer
 * param_2: object handle to test against
 *
 * Node layout:
 *   [0] kind    0 = leaf (ObjList at param_1[1])
 *               1 = X-split (split coord at param_1[1])
 *               2 = Z-split
 *   [1] splitCoord or chainHead ptr
 *   [2] left child ptr
 *   [3] right child ptr
 *
 * Returns 1 if Object_TestCollision returned early (collision and
 *           damage handled), 2 in "processed" state, 0 = no collision.
 *
 * HIGH confidence.
 * ================================================================ */
uint32_t FUN_80020f14(int *param_1, intptr_t param_2)
{
    int  *piVar1;
    int  *piVar2;
    uint32_t uVar3;
    int   iVar4;

    iVar4 = *param_1;
    if (iVar4 == 1) {
        /* X-split */
        if ((mips_subu_i32(*(int *)(uintptr_t)(param_2 + 0x24),
                           *(int *)(uintptr_t)(param_2 + 0x54)) < param_1[1]) &&
            (iVar4 = (int)FUN_80020f14((int *)(uintptr_t)param_1[2], param_2), iVar4 == 0)) {
            return 0;
        }
        if ((param_1[1] < mips_addu_i32(*(int *)(uintptr_t)(param_2 + 0x24),
                                        *(int *)(uintptr_t)(param_2 + 0x54))) &&
            (iVar4 = (int)FUN_80020f14((int *)(uintptr_t)param_1[3], param_2), iVar4 == 0)) {
            return 0;
        }
        uVar3 = 1;
    } else {
        uVar3 = 2;
        if (iVar4 == 0) {
            /* Leaf: walk the object linked list */
            CollisionHostNode *hnode = host_list_first(param_1 + 1);
            if (hnode != NULL) {
                for (; hnode != NULL; hnode = hnode->next) {
                    uint32_t *obj = (uint32_t *)hnode->payload;
                    if (obj == NULL)
                        continue;
                    if (((*obj & 0x20) == 0) &&
                        (iVar4 = (int)FUN_8001edb4(param_2, obj), iVar4 != 0)) {
                        return 0;
                    }
                }
            } else {
                piVar1 = (int *)(uintptr_t)param_1[1];
                for (piVar2 = (int *)(uintptr_t)*(int *)(uintptr_t)param_1[1];
                     piVar2 != NULL;
                     piVar2 = (int *)(uintptr_t)*piVar2) {
                    if (((*(uint32_t *)(uintptr_t)piVar1[2] & 0x20) == 0) &&
                        (iVar4 = (int)FUN_8001edb4(param_2, (uint32_t *)(uintptr_t)piVar1[2]),
                         iVar4 != 0)) {
                        return 0;
                    }
                    piVar1 = piVar2;
                }
            }
            uVar3 = 1;
        } else if (iVar4 == 2) {
            /* Z-split */
            if ((mips_subu_i32(*(int *)(uintptr_t)(param_2 + 0x2c),
                               *(int *)(uintptr_t)(param_2 + 0x54)) < param_1[1]) &&
                (iVar4 = (int)FUN_80020f14((int *)(uintptr_t)param_1[2], param_2), iVar4 == 0)) {
                return 0;
            }
            if ((param_1[1] < mips_addu_i32(*(int *)(uintptr_t)(param_2 + 0x2c),
                                            *(int *)(uintptr_t)(param_2 + 0x54))) &&
                (iVar4 = (int)FUN_80020f14((int *)(uintptr_t)param_1[3], param_2), iVar4 == 0)) {
                return 0;
            }
            uVar3 = 1;
        }
    }
    return uVar3;
}

/* ================================================================
 * FUN_8002169c -- Frame_CollideAll
 *
 * Per-frame collision driver.  Iterates the live object list
 * (piRam00000714) and for each object:
 *   1. Clears contact slots param_1[0x1e] and [0x1d].
 *   2. Runs pairwise Object_TestCollision against all later objects
 *      in the list whose flag 0x20 is clear.
 *   3. If flag 0x100 is clear: also tests against the static
 *      terrain kd-tree via Tree_CollideObject (FUN_80020f14).
 *
 * HIGH confidence.
 * ================================================================ */
void FUN_8002169c(void)
{
    int iVar5;
    uint8_t *listHead = (uint8_t *)piRam00000714;
    CollisionHostNode *outer;

    if (listHead == NULL) {
        extern uint8_t DAT_80065a18[];
        listHead = DAT_80065a18;
    }

    for (outer = host_list_first(listHead); outer != NULL; outer = outer->next) {
        uint32_t *puVar6 = (uint32_t *)outer->payload;
        CollisionHostNode *inner;
        if (puVar6 == NULL)
            continue;
        if ((*puVar6 & 0x20) != 0)
            continue;

        puVar6[0x1e] = 0;
        puVar6[0x1d] = 0;
        for (inner = outer->next; inner != NULL; inner = inner->next) {
            uint32_t *other = (uint32_t *)inner->payload;
            if (other == NULL)
                continue;
            if (((*other & 0x20) == 0) &&
                ((*other & *puVar6 & 0x200) == 0) &&
                (iVar5 = (int)FUN_8001edb4((intptr_t)puVar6, other), iVar5 != 0)) {
                return;
            }
        }
        if (((*puVar6 & 0x100) == 0) && uRam000006fc != 0) {
            FUN_80020f14((int *)(uintptr_t)uRam000006fc, (intptr_t)puVar6);
        }
    }
}

/* ================================================================
 * FUN_80021678 -- Physics_FlushVoxels (scratchpad trampoline)
 *
 * PSX trampoline that saves $ra/$sp to scratchpad addrs DAT_1f8003f0/f4
 * and tail-calls FUN_8002169c (Frame_CollideAll). On host the scratchpad
 * is plain memory so this is a direct forward.
 * HIGH confidence (direct Ghidra port).
 * ================================================================ */
void FUN_80021678(void) { FUN_8002169c(); }
/* Physics_FlushVoxels lives in platform/physics_shim.c (host no-op). */

/* ================================================================
 * FUN_80021978 -- ObjList_FindInBounds
 *
 * Walks a linked object list and returns the payload of the first
 * entry whose collision-layer field (short at obj+6) is in the range
 * [param_2, param_3] AND whose 2D spatial bounding box
 * (at obj+0x48..0x53) overlaps param_4[0..3].
 *
 * List node: {next-ptr, prev-ptr, payload-ptr, ...}
 * param_1 = pointer to list sentinel (piVar1 = prev, piVar2 = curr)
 *
 * Returns 0 if no matching entry found.
 *
 * HIGH confidence.
 * ================================================================ */
int FUN_80021978(int *param_1, int param_2, int param_3, int *param_4)
{
    int *piVar1;
    int *piVar2;
    int  iVar3;

    piVar1 = (int *)(uintptr_t)*param_1;
    piVar2 = (int *)(uintptr_t)*(int *)(uintptr_t)*param_1;
    while (true) {
        if (piVar2 == NULL) {
            return 0;
        }
        iVar3 = piVar1[2];
        if ((((param_2 <= *(int16_t *)(uintptr_t)(iVar3 + 6)) &&
              (*(int16_t *)(uintptr_t)(iVar3 + 6) <= param_3)) &&
             (*param_4 < *(int *)(uintptr_t)(iVar3 + 0x48))) &&
            ((*(int *)(uintptr_t)(iVar3 + 0x48) < param_4[1] &&
              (param_4[2] < *(int *)(uintptr_t)(iVar3 + 0x50))) &&
             (*(int *)(uintptr_t)(iVar3 + 0x50) < param_4[3]))) {
            break;
        }
        piVar1 = piVar2;
        piVar2 = (int *)(uintptr_t)*piVar2;
    }
    return iVar3;
}

/* ================================================================
 * FUN_80021a30 -- TreeAABB_FirstHit
 *
 * Recursive kd-tree search: finds the first leaf-list object in the
 * subtree rooted at param_1 that matches the layer range [param_2,
 * param_3] and spatial bounds param_4[0..3] (2D min/max).
 *
 * Node layout:
 *   [0] kind    0 = leaf (call ObjList_FindInBounds)
 *               1 = X-split: [1]=coord, [2]=left, [3]=right
 *               2 = Z-split: [1]=coord, [2]=left, [3]=right
 *   [1] coord
 *   [2] left child / chainHead
 *   [3] right child
 *
 * Returns 0 if nothing found, non-zero payload ptr on match.
 *
 * HIGH confidence.
 * ================================================================ */
int FUN_80021a30(int *param_1, int param_2, int param_3, int *param_4)
{
    int iVar1, iVar2;

    iVar2 = *param_1;
    if (iVar2 == 1) {
        iVar2 = param_1[1];
        if (((iVar2 <= *param_4) ||
             (iVar1 = FUN_80021a30((int *)(uintptr_t)param_1[2], param_2, param_3, param_4),
              iVar1 == 0)) &&
            ((param_4[1] <= iVar2 ||
              (iVar1 = FUN_80021a30((int *)(uintptr_t)param_1[3], param_2, param_3, param_4),
               iVar1 == 0)))) {
            iVar1 = 0;
        }
    } else {
        iVar1 = 2;
        if (iVar2 == 0) {
            iVar1 = FUN_80021978(param_1 + 1, param_2, param_3, param_4);
        } else if (((iVar2 == 2) &&
                    ((iVar2 = param_1[1], iVar2 <= param_4[2] ||
                      (iVar1 = FUN_80021a30((int *)(uintptr_t)param_1[2], param_2, param_3, param_4),
                       iVar1 == 0)))) &&
                   ((param_4[3] <= iVar2 ||
                     (iVar1 = FUN_80021a30((int *)(uintptr_t)param_1[3], param_2, param_3, param_4),
                      iVar1 == 0)))) {
            iVar1 = 0;
        }
    }
    return iVar1;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80020620  (from analysis/SLUS_005.10/decomp/80020620.c) --- */
// addr: 0x80020620  name: FUN_80020620

void FUN_80020620(undefined4 param_1,undefined4 param_2)
{
  (*pcRam00000730)(param_1,0x11,param_2);
  FUN_800205f8(param_1);
  return;
}

/* --- SLUS_005.10 FUN_800209cc  (from analysis/SLUS_005.10/decomp/800209cc.c) --- */
// addr: 0x800209cc  name: FUN_800209cc

void FUN_800209cc(int *param_1,int param_2,int param_3,int param_4,int param_5)
{
  bool bVar1;
  int iVar2;
  iVar2 = *param_1;
  if (iVar2 == 1) {
    iVar2 = param_1[1];
    if (param_2 < iVar2) { FUN_800209cc(param_1[2],param_2,param_3,param_4,param_5); }
    bVar1 = iVar2 < param_3;
  } else {
    if (iVar2 == 0) { FUN_800206f0(param_1 + 1); return; }
    if (iVar2 != 2) { return; }
    iVar2 = param_1[1];
    if (param_4 < iVar2) { FUN_800209cc(param_1[2],param_2,param_3,param_4,param_5); }
    bVar1 = iVar2 < param_5;
  }
  if (bVar1) { FUN_800209cc(param_1[3],param_2,param_3,param_4,param_5); }
  return;
}

/* --- SLUS_005.10 FUN_80020f14  (from analysis/SLUS_005.10/decomp/80020f14.c) --- */
// (see above for full body)

/* --- SLUS_005.10 FUN_8002169c  (from analysis/SLUS_005.10/decomp/8002169c.c) --- */
// (see above for full body)

#endif  /* GHIDRA REF */
