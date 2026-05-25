/* sound_alloc.c -- Sound/bone slot allocation helper.
 *
 * Source: SLUS_005.10  FUN_80021b80.
 *
 * Attempts to acquire a sound/bone slot via a callback, then either
 * allocates a small object or builds a bone tree based on param_2:
 *
 *   1. If param_1 (callback) is non-NULL:
 *        - Call param_1(param_2, 7, param_3) to probe the slot.
 *          If it returns non-zero, return that value immediately.  Vehicle
 *          callbacks use this path to construct and return the player car.
 *        - Call param_1(0, 6, 0) to get a free slot index.
 *          If it returns non-zero, use that index.
 *   2. If no slot found from callback, use fallback index 0x80.
 *   3. If param_2 == 0: call Object_AllocSmall(slot_index).
 *      Else:            call BoneObj_BuildTree(param_2, param_3, slot_index, param_4).
 *
 * MED confidence: direct Ghidra ref translation.
 */
#include <stdint.h>
#include <stddef.h>

extern void *FUN_8001d470(uint32_t size);  /* Object_AllocSmall */
extern void *FUN_8001ac44(intptr_t param_1, uint16_t param_2, uint32_t param_3, uint32_t param_4); /* BoneObj_BuildTree */

/* FUN_80021b80 -- Bone_AllocWithCallback.
 * Returns the allocated object handle (MIPS transparent-return: last call
 * leaves v0 set to the allocated object, caller captures it). */
intptr_t FUN_80021b80(intptr_t (*param_1)(intptr_t, int, int),
                      intptr_t param_2, uint16_t param_3, uint32_t param_4)
{
    intptr_t iVar1;

    if (param_1 != NULL) {
        iVar1 = param_1(param_2, 7, (int)param_3);
        if (iVar1 != 0)
            return iVar1;
        iVar1 = param_1(0, 6, 0);
        if (iVar1 != 0)
            goto LAB_80021bdc;
    }
    iVar1 = 0x80;
LAB_80021bdc:
    if (param_2 == 0)
        return (intptr_t)FUN_8001d470((uint32_t)iVar1);
    else
        return (intptr_t)FUN_8001ac44(param_2, param_3, (uint32_t)iVar1, param_4);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80021b80  (from analysis/SLUS_005.10/decomp/80021b80.c) --- */
// addr: 0x80021b80  name: FUN_80021b80

void FUN_80021b80(code *param_1,int param_2,undefined2 param_3,undefined4 param_4)

{
  int iVar1;

  if (param_1 != (code *)0x0) {
    iVar1 = (*param_1)(param_2,7,param_3);
    if (iVar1 != 0) {
      return;
    }
    iVar1 = (*param_1)(0,6,0);
    if (iVar1 != 0) goto LAB_80021bdc;
  }
  iVar1 = 0x80;
LAB_80021bdc:
  if (param_2 == 0) {
    FUN_8001d470(iVar1);
  }
  else {
    FUN_8001ac44(param_2,param_3,iVar1,param_4);
  }
  return;
}

#endif  /* GHIDRA REF */
