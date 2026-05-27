/* weapon_split.c -- detach a weapon's first child as its own projectile.
 *
 * Source: SLUS_005.10  FUN_8003ff28.
 *
 * Called when a weapon parent (carrier object) needs to spawn its
 * payload child as a standalone gravity-ticked projectile.  Sequence:
 *
 *   1. Locate the parent's first child via Object_FindFirstChild
 *      (FUN_8001d5a0).
 *   2. Fetch the child's world matrix via FUN_8001d624.
 *   3. Unregister the carrier from its scene list (Object_UnregisterFromScene
 *      at FUN_800203fc).
 *   4. Compose carrier-matrix into the carrier's local-matrix slot
 *      (param_1+4) using PsyQ's CompMatrixLV.
 *   5. Walk-and-restore parent links via FUN_8001d564.
 *   6. Flip a state byte at +0x04, swap the tick callback to
 *      Projectile_GravityTick (FUN_8003eab0), clear flag 0x4000 and
 *      set 0x80 (now-active projectile).
 *   7. Shrink the heap block to 0x94 bytes -- this MAY return a new
 *      pointer, so all subsequent accesses use the returned iVar3.
 *   8. If the (possibly relocated) record still has a child, patch the
 *      child's back-link.
 *   9. Compute the projectile's launch velocity from the spawn pos
 *      (scaled << 9 >> 12 = << -3, i.e. divide by 8).
 *  10. Randomise the colour (3x 8-bit channels at +0x80/82/84).
 *  11. Reset the world-pos accumulator (+0x48..0x50) to the current
 *      world pos (+0x24..0x2c).
 *  12. Set substate byte at +0x87 to 2.
 *  13. Rotate the velocity vector through the captured matrix m0.
 *  14. Re-register via Object_PostUpdate2 (FUN_8002036c).
 *
 * HIGH confidence (direct Ghidra port).  Ghidra dropped:
 *   - the arg on FUN_8001d5a0 (it's param_1)
 *
 * Bit-exact: the `<< 9 >> 12` chain is preserved as the original signed
 * shift sequence (not collapsed to `>> 3`) since C right-shift of signed
 * negative values is implementation-defined whereas the MIPS sequence
 * is well-defined.
 */
#include <stdint.h>
#include "structs.h"

/* ---- Hierarchy helpers ---- */
extern intptr_t  FUN_8001d5a0(intptr_t parent);             /* Object_FindFirstChild */
extern MATRIX   *FUN_8001d624(intptr_t obj);                /* Object_GetWorldMatrix */
extern intptr_t  FUN_8001d564(intptr_t obj);                /* Object_ResetParentChain */

/* ---- Object-list management ---- */
extern uint32_t *FUN_800203fc(uint32_t *obj);               /* Object_UnregisterFromScene */
extern void      FUN_8002036c(int obj);                     /* Object_PostUpdate2 */

/* ---- Heap + RNG + GTE ---- */
extern void     *FUN_80045134(void *p, int newSize);        /* Heap_Shrink */
extern int       FUN_80017160(void);                        /* v8_rand */
extern void      FUN_80043358(uint32_t *m,
                              int32_t *src, int32_t *dst);  /* Mat_RotateVec */
extern MATRIX   *CompMatrixLV(MATRIX *m0, MATRIX *m1, MATRIX *out);

/* ---- The new tick callback installed by this routine. ---- */
extern void      FUN_8003eab0(void);                        /* Projectile_GravityTick */
extern void      Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* ================================================================
 * FUN_8003ff28 -- Weapon_SplitChildToProjectile
 *
 * Returns the (possibly relocated) projectile record pointer.
 * ================================================================ */
void *FUN_8003ff28(uint32_t *param_1)
{
    uint16_t  uVar1;
    intptr_t  uVar2;
    MATRIX   *m0;
    intptr_t  iVar3;

    uVar2 = FUN_8001d5a0((intptr_t)param_1);
    m0    = FUN_8001d624(uVar2);

    FUN_800203fc(param_1);
    CompMatrixLV(m0, (MATRIX *)(param_1 + 4), (MATRIX *)(param_1 + 4));
    FUN_8001d564((intptr_t)param_1);

    *(uint8_t *)(param_1 + 1) = 1;
    Object_SetCallbackPsxSlot(param_1, (uintptr_t)&FUN_8003eab0);
    *param_1 = (*param_1 & 0xffffbfffu) | 0x80u;

    iVar3 = (intptr_t)FUN_80045134(param_1, 0x94);

    /* If still has a child, repair the child's parent back-link. */
    if (*(int *)(uintptr_t)(iVar3 + 0x38) != 0) {
        *(int *)(uintptr_t)(*(int *)(uintptr_t)(iVar3 + 0x38) + 0x3c) = iVar3;
    }

    /* Launch velocity = spawn pos scaled << 9 >> 12. */
    *(int32_t *)(uintptr_t)(iVar3 + 0x88) =
        (*(int32_t *)(uintptr_t)(iVar3 + 0x48) << 9) >> 12;
    *(int32_t *)(uintptr_t)(iVar3 + 0x8c) =
        (*(int32_t *)(uintptr_t)(iVar3 + 0x4c) << 9) >> 12;
    *(int32_t *)(uintptr_t)(iVar3 + 0x90) =
        (*(int32_t *)(uintptr_t)(iVar3 + 0x50) << 9) >> 12;

    /* Random 8-bit-per-channel colour. */
    uVar1 = (uint16_t)FUN_80017160();
    *(uint16_t *)(uintptr_t)(iVar3 + 0x80) = uVar1 & 0xff;
    uVar1 = (uint16_t)FUN_80017160();
    *(uint16_t *)(uintptr_t)(iVar3 + 0x82) = uVar1 & 0xff;
    uVar1 = (uint16_t)FUN_80017160();
    *(uint16_t *)(uintptr_t)(iVar3 + 0x84) = uVar1 & 0xff;

    /* Copy current world pos (+0x24..0x2c) to spawn-pos slot (+0x48..). */
    *(uint32_t *)(uintptr_t)(iVar3 + 0x48) =
        *(uint32_t *)(uintptr_t)(iVar3 + 0x24);
    *(uint32_t *)(uintptr_t)(iVar3 + 0x4c) =
        *(uint32_t *)(uintptr_t)(iVar3 + 0x28);
    *(uint32_t *)(uintptr_t)(iVar3 + 0x50) =
        *(uint32_t *)(uintptr_t)(iVar3 + 0x2c);

    *(uint8_t *)(uintptr_t)(iVar3 + 0x87) = 2;

    FUN_80043358((uint32_t *)m0,
                 (int32_t *)(uintptr_t)(iVar3 + 0x88),
                 (int32_t *)(uintptr_t)(iVar3 + 0x88));
    FUN_8002036c((int)(uintptr_t)iVar3);
    return (void *)(uintptr_t)iVar3;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8003ff28 --- */
int FUN_8003ff28(uint *param_1)
{
  ushort uVar1;
  undefined4 uVar2;
  MATRIX *m0;
  int iVar3;

  uVar2 = FUN_8001d5a0();
  m0 = (MATRIX *)FUN_8001d624(uVar2);
  FUN_800203fc(param_1);
  CompMatrixLV(m0,(MATRIX *)(param_1 + 4),(MATRIX *)(param_1 + 4));
  FUN_8001d564(param_1);
  *(undefined1 *)(param_1 + 1) = 1;
  param_1[0x19] = (uint)FUN_8003eab0;
  *param_1 = *param_1 & 0xffffbfff | 0x80;
  iVar3 = FUN_80045134(param_1,0x94);
  if (*(int *)(iVar3 + 0x38) != 0) {
    *(int *)(*(int *)(iVar3 + 0x38) + 0x3c) = iVar3;
  }
  *(int *)(iVar3 + 0x88) = (*(int *)(iVar3 + 0x48) << 9) >> 0xc;
  *(int *)(iVar3 + 0x8c) = (*(int *)(iVar3 + 0x4c) << 9) >> 0xc;
  *(int *)(iVar3 + 0x90) = (*(int *)(iVar3 + 0x50) << 9) >> 0xc;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x80) = uVar1 & 0xff;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x82) = uVar1 & 0xff;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x84) = uVar1 & 0xff;
  *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(iVar3 + 0x24);
  *(undefined4 *)(iVar3 + 0x4c) = *(undefined4 *)(iVar3 + 0x28);
  *(undefined4 *)(iVar3 + 0x50) = *(undefined4 *)(iVar3 + 0x2c);
  *(undefined1 *)(iVar3 + 0x87) = 2;
  FUN_80043358(m0,iVar3 + 0x88,iVar3 + 0x88);
  FUN_8002036c(iVar3);
  return iVar3;
}

#endif  /* GHIDRA REF */
