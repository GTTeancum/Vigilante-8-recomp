/* obj_list_utils.c -- Object linked-list utility functions.
 *
 * Source: SLUS_005.10
 *   FUN_80024748  -- ObjList_InsertSorted    (~20 instr)
 *   FUN_8002479c  -- ObjList_ClearFlags      (~12 instr)
 *
 * FUN_80024748:
 *   Inserts param_2 into the sorted singly-linked list rooted at
 *   *param_1, maintaining ascending order by param_2[5].
 *   Walks until it finds the insertion point, splices param_2 in,
 *   and returns the new head.
 *
 * FUN_8002479c:
 *   Walks a singly-linked list (next ptr at [0]) and clears the
 *   0x6000 bits in a 16-bit field at: obj_base[byte_4*2 + 2].
 *   Used to reset the "dirty" TPage flags on a chain of draw nodes.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include <stddef.h>

/* ================================================================
 * FUN_80024748 -- ObjList_InsertSorted
 *
 * Inserts param_2 into the sorted linked list *param_1, keeping
 * ascending order on element [5].
 *
 * Returns the new list head (which may be param_2 if it sorts
 * before the old head).
 * ================================================================ */
int *FUN_80024748(int *param_1, int *param_2)
{
    int *piVar1;
    int *piVar2;

    piVar1 = NULL;
    for (piVar2 = param_1;
         (piVar2 != NULL && (piVar2[5] < param_2[5]));
         piVar2 = (int *)(uintptr_t)*piVar2) {
        piVar1 = piVar2;
    }
    *param_2 = (int)(uintptr_t)piVar2;
    if (piVar1 != NULL) {
        *piVar1 = (int)(uintptr_t)param_2;
        return param_1;
    }
    return param_2;
}

/* ================================================================
 * FUN_8002479c -- ObjList_ClearFlags
 *
 * Walks a singly-linked list via [0] and for each node clears the
 * 0x6000 bits in the u16 at: *(u16*)(node[2] + node[4]*2 + 2).
 *
 * Typically used to clear GPU TPage dirty bits on draw-packet chains.
 * ================================================================ */
void FUN_8002479c(int *param_1)
{
    int iVar1;

    for (; param_1 != NULL; param_1 = (int *)(uintptr_t)*param_1) {
        iVar1 = param_1[2] + (uint32_t)*(uint8_t *)(param_1 + 4) * 2;
        *(uint16_t *)(uintptr_t)(iVar1 + 2) =
            *(uint16_t *)(uintptr_t)(iVar1 + 2) & (uint16_t)0x9fff;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80024748  (from analysis/SLUS_005.10/decomp/80024748.c) --- */
// addr: 0x80024748  name: FUN_80024748

int * FUN_80024748(int *param_1,int *param_2)
{
  int *piVar1; int *piVar2;
  piVar1 = (int *)0x0;
  for (piVar2 = param_1; (piVar2 != (int *)0x0 && (piVar2[5] < param_2[5])); piVar2 = (int *)*piVar2) {
    piVar1 = piVar2;
  }
  *param_2 = (int)piVar2;
  if (piVar1 != (int *)0x0) { *piVar1 = (int)param_2; return param_1; }
  return param_2;
}

/* --- SLUS_005.10 FUN_8002479c  (from analysis/SLUS_005.10/decomp/8002479c.c) --- */
// addr: 0x8002479c  name: FUN_8002479c

void FUN_8002479c(int *param_1)
{
  int iVar1;
  for (; param_1 != (int *)0x0; param_1 = (int *)*param_1) {
    iVar1 = param_1[2] + (uint)*(byte *)(param_1 + 4) * 2;
    *(ushort *)(iVar1 + 2) = *(ushort *)(iVar1 + 2) & 0x9fff;
  }
  return;
}

#endif  /* GHIDRA REF */
