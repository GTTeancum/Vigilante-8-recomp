/* weapon_projectile_tick.c -- Weapon projectile position update and state dispatch.
 *
 * Source: SLUS_005.10
 *   FUN_8003c288  -- WeaponProjectile_Tick
 *   FUN_8003c538  -- WeaponProjectile_Dispatch
 *
 * FUN_8003c288 (param_1 = projectile obj ptr, param_2 = target struct ptr):
 *   - Computes dx/dy/dz from target pos (param_2+4/8/0xc) minus current pos
 *     (param_1+0x48/0x4c/0x50).
 *   - If kind byte (param_1+8) >= 0 (dumb projectile): proximity check.
 *     Returns 1 if max(|dx|,|dy|,|dz|) < 0x801.
 *   - Advances position using PSX RTZ arithmetic shifts:
 *       x += (RTZ(dx,0x1f)>>5) + (RTZ(dz,7)>>3)
 *       y += RTZ(dy,0xf)>>4
 *       z += (RTZ(dz,0x1f)>>5) - (RTZ(dx,7)>>3)
 *   - Copies updated local pos (0x48..0x50) to world pos (0x24..0x2c).
 *   - If kind < 0 and (frame - spawn_byte) & 3 == 0:
 *       Walks 3 weapon slots of parent vehicle, calls damage callback on
 *       matching-kind slots, increments slot ammo counter if callback returns 0.
 *       Decrements projectile ammo (param_1+0xc); if ammo reaches 0, plays
 *       destroy SFX and frees the projectile.  Returns 0xffffffff on destroy.
 *   - Returns 0 (still alive) or 1 (hit) otherwise.
 *
 * FUN_8003c538 (param_1 = projectile obj uint* ptr, param_2 = proximity value):
 *   - Returns 1 if the active bit (0x10000) is not set in *param_1.
 *   - MIPS correction: Ghidra shows "param_2 < 0x80000001" but the actual
 *     MIPS instruction is `sltu v0(=0x10000), a1(=param_2)` which means the
 *     correct C condition for skipping FUN_8003c288 is param_2 > 0x10000u.
 *   - If param_2 <= 0x10000u: calls FUN_8003c288(param_1, param_1[0x20]).
 *     Returns 0 if projectile still alive.
 *   - Otherwise (param_2 > 0x10000u or FUN_8003c288 returned non-zero):
 *     explosion path -- plays SFX 0x2c, snaps obj to target pos,
 *     clears active bit, removes from back-buffer.  Returns 0.
 *
 * Dropped-arg correction vs Ghidra (confirmed from MIPS):
 *   FUN_8003c288(param_1, param_1[0x20]) -- Ghidra shows param_1[0x20] as 2nd arg ✓
 *   FUN_80020778(param_1)                -- Ghidra drops the argument
 *
 * Symbol name mapping (impl file → PSX address):
 *   FUN_8001d5a0         = 0x8001d5a0  Object_FindFirstChild  (weapon_slot.c)
 *   Object_DetachFromParent = 0x8001d564  (object_hierarchy.c)
 *   FUN_800204dc         = 0x800204dc  Object_HeapFree        (object_list.c)
 *   Object_ClearBackBufferFlag = 0x80020778  (damage_apply.c)
 *   Matrix_ComposeParentChain  = 0x8001d624  (matrix_chain.c)
 *
 * HIGH confidence: both functions verified instruction-for-instruction vs MIPS.
 */
#include <stdint.h>
#include <stddef.h>
#include "structs.h"

/* Implemented elsewhere -- use real C symbol names (not hex aliases). */
extern int      FUN_8001d5a0(int param_1);               /* Object_FindFirstChild   (weapon_slot.c)       */
extern MATRIX  *Matrix_ComposeParentChain(int param_1);  /* FUN_8001d624            (matrix_chain.c)      */
extern void     Object_DetachFromParent(int self);       /* FUN_8001d564            (object_hierarchy.c)  */
extern void     FUN_800204dc(int param_1);               /* Object_HeapFree         (object_list.c)       */
extern uint32_t Object_ClearBackBufferFlag(uint32_t *obj); /* FUN_80020778          (damage_apply.c)      */
extern int      FUN_8004410c(void);                      /* Audio_AllocVoice        (panic_stubs.c)       */
extern int      FUN_8004483c(int voice, uint32_t bank,
                              int sfxId, void *pos3d);   /* Audio_PlaySfx3DPos      (panic_stubs.c)       */
extern uint32_t uRam000005f8;                            /* SFX bank ptr            (panic_stubs.c)       */
extern int32_t  iRam0000000c;                            /* frame counter           (panic_stubs.c)       */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* ================================================================
 * FUN_8003c288  -- WeaponProjectile_Tick
 *
 * param_1 = projectile object (as PSX address stored in int)
 * param_2 = target struct    (as PSX address stored in int)
 *
 * Returns: 1 on hit, 0xffffffff on destroy, 0 still alive.
 * HIGH confidence: direct Ghidra ref port with uintptr_t casts and
 * MIPS-confirmed dropped arg for FUN_8001d5a0(param_1).
 * ================================================================ */
int FUN_8003c288(int param_1, int param_2)
{
    /* Slot callback function-pointer type (host-width, stored at obj+0x64). */
    typedef int (*SlotTickFn)(int slot_obj, int a1, int proj);

    uint16_t uVar1;
    int16_t  sVar2;
    int      iVar3;
    int      iVar4;
    int      iVar5;
    int      uVar6;
    int      iVar7;
    int      iVar8;
    MATRIX  *matPtr;

    /* Compute delta: target pos minus current pos. */
    iVar8 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 4),
                          *(int32_t *)(uintptr_t)(param_1 + 0x48));
    iVar3 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 8),
                          *(int32_t *)(uintptr_t)(param_1 + 0x4c));
    iVar4 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 0xc),
                          *(int32_t *)(uintptr_t)(param_1 + 0x50));

    /* Dumb projectile (kind >= 0): proximity check. */
    if (-1 < *(int8_t *)(uintptr_t)(param_1 + 8)) {
        iVar5 = iVar8;
        if (iVar8 < 0) {
            iVar5 = mips_subu_i32(0, iVar8);
        }
        iVar7 = iVar3;
        if (iVar3 < 0) {
            iVar7 = mips_subu_i32(0, iVar3);
        }
        if (iVar7 < iVar5) {
            iVar7 = iVar5;
        }
        iVar5 = iVar4;
        if (iVar4 < 0) {
            iVar5 = mips_subu_i32(0, iVar4);
        }
        if (iVar5 < iVar7) {
            iVar5 = iVar7;
        }
        if (iVar5 < 0x801) {
            return 1;
        }
    }

    /* Advance X: (RTZ(dx,0x1f)>>5) + (RTZ(dz,7)>>3).
     * PSX RTZ: if value < 0, add (divisor-1) before shifting. */
    iVar5 = iVar8;
    if (iVar8 < 0) {
        iVar5 = mips_addu_i32(iVar8, 0x1f);
    }
    iVar7 = iVar4;
    if (iVar4 < 0) {
        iVar7 = mips_addu_i32(iVar4, 7);
    }
    *(int *)(uintptr_t)(param_1 + 0x48) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x48),
                      mips_addu_i32(iVar5 >> 5, iVar7 >> 3));

    /* Advance Y: RTZ(dy,0xf)>>4. */
    if (iVar3 < 0) {
        iVar3 = mips_addu_i32(iVar3, 0xf);
    }
    *(int *)(uintptr_t)(param_1 + 0x4c) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x4c), iVar3 >> 4);

    /* Advance Z: (RTZ(dz,0x1f)>>5) - (RTZ(dx,7)>>3). */
    if (iVar4 < 0) {
        iVar4 = mips_addu_i32(iVar4, 0x1f);
    }
    if (iVar8 < 0) {
        iVar8 = mips_addu_i32(iVar8, 7);
    }
    *(int *)(uintptr_t)(param_1 + 0x50) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x50),
                      mips_subu_i32(iVar4 >> 5, iVar8 >> 3));

    /* Copy updated local pos (0x48..0x50) to world pos (0x24..0x2c). */
    *(uint32_t *)(uintptr_t)(param_1 + 0x24) = *(uint32_t *)(uintptr_t)(param_1 + 0x48);
    *(uint32_t *)(uintptr_t)(param_1 + 0x28) = *(uint32_t *)(uintptr_t)(param_1 + 0x4c);
    *(uint32_t *)(uintptr_t)(param_1 + 0x2c) = *(uint32_t *)(uintptr_t)(param_1 + 0x50);

    /* Homing projectile (kind < 0): per-slot damage tick every 4 frames. */
    uVar6 = 0;
    if ((*(int8_t *)(uintptr_t)(param_1 + 8) < 0) &&
        (uVar6 = 0,
         ((mips_subu_i32(iRam0000000c,
                         (int32_t)(uint32_t)*(uint8_t *)(uintptr_t)(param_1 + 9)) & 3) == 0))) {

        /* Get parent vehicle by walking the sibling/parent chain. */
        iVar3 = FUN_8001d5a0(param_1);

        /* Iterate weapon slots at parent + 0xec + {0x24, 0x28, 0x2c}
         * (slots 9, 10, 11 in the 32-entry descriptor table). */
        iVar8 = 0;
        iVar4 = 0x24;
        do {
            /* Slot obj ptr stored at parent + 0xec + slot_offset. */
            iVar5 = *(int *)(uintptr_t)(iVar3 + iVar4 + 0xec);

            /* Only process if slot kind == negated projectile kind. */
            if ((int)*(int8_t *)(uintptr_t)(iVar5 + 8) ==
                mips_subu_i32(0, (int)*(int8_t *)(uintptr_t)(param_1 + 8))) {

                /* Callback is stored as a host function pointer at slot+0x64. */
                SlotTickFn fn = (SlotTickFn)Object_CallbackFromPsxSlot((const void *)(uintptr_t)iVar5);
                if (fn == NULL) {
                    iVar5 = 0;
                } else {
                    iVar5 = fn(iVar5, 0xf, param_1);
                }

                /* If callback returned 0, increment slot ammo counter (cap at 98). */
                if (iVar5 == 0) {
                    iVar5 = *(int *)(uintptr_t)(iVar3 + iVar4 + 0xec);
                    uVar1 = *(uint16_t *)(uintptr_t)(iVar5 + 0xc);
                    if (uVar1 < 99) {
                        *(uint16_t *)(uintptr_t)(iVar5 + 0xc) = (uint16_t)(uVar1 + 1);
                    }
                }
            }
            iVar8 = mips_addu_i32(iVar8, 1);
            iVar4 = mips_addu_i32(iVar4, 4);
        } while (iVar8 < 3);

        /* Decrement projectile ammo; destroy if it reaches zero. */
        sVar2 = (int16_t)mips_addu_i32((int)*(int16_t *)(uintptr_t)(param_1 + 0xc), -1);
        *(int16_t *)(uintptr_t)(param_1 + 0xc) = sVar2;
        if (sVar2 == 0) {
            /* Play destruction SFX at matrix +0x14. */
            uVar6 = FUN_8004410c();
            matPtr = Matrix_ComposeParentChain(param_1);
            FUN_8004483c(uVar6, uRam000005f8, 0x28, (uint8_t *)matPtr + 0x14);
            Object_DetachFromParent(param_1);
            FUN_800204dc(param_1);
            uVar6 = 0xffffffff;
        } else {
            uVar6 = 0;
        }
    }
    return uVar6;
}

/* ================================================================
 * FUN_8003c538  -- WeaponProjectile_Dispatch
 *
 * param_1 = projectile object ptr (uint32_t *)
 * param_2 = proximity / distance value (uint32_t)
 *
 * Returns: 1 (inactive), 0 (alive or exploded).
 *
 * MIPS correction: `sltu v0(=0x10000), a1(param_2)` is the
 * "skip FUN_8003c288 and go to explosion" branch.  Correct C
 * condition is (uint32_t)param_2 > 0x10000u.
 * Dropped arg: FUN_80020778(param_1) -- Ghidra shows no args.
 * HIGH confidence.
 * ================================================================ */
int FUN_8003c538(uint32_t *param_1, uint32_t param_2)
{
    int      uVar1;
    int      iVar2;
    uint32_t uVar3;
    uint32_t uVar4;
    uint32_t uVar5;
    MATRIX  *matPtr;

    if ((*param_1 & 0x10000) == 0) {
        uVar1 = 1;
    } else {
        /* MIPS: sltu v0(=0x10000), a1(param_2).
         * Branch to explosion if param_2 > 0x10000u;
         * otherwise call the projectile position tick. */
        if ((param_2 <= 0x10000u) &&
            (iVar2 = FUN_8003c288((int)(uintptr_t)param_1,
                                   (int)param_1[0x20]),
             iVar2 == 0)) {
            return 0;
        }

        /* Explosion path: play SFX 0x2c at matrix +0x14. */
        uVar1  = FUN_8004410c();
        matPtr = Matrix_ComposeParentChain((int)(uintptr_t)param_1);
        FUN_8004483c(uVar1, uRam000005f8, 0x2c, (uint8_t *)matPtr + 0x14);

        /* Snap projectile position to target pos, then clear active bit. */
        uVar3 = param_1[0x20];                              /* target struct PSX ptr */
        *param_1 = *param_1 & 0xfffeffff;                  /* clear bit 0x10000     */
        uVar4 = *(uint32_t *)(uintptr_t)(uVar3 + 8);
        uVar5 = *(uint32_t *)(uintptr_t)(uVar3 + 0xc);
        param_1[0x12] = *(uint32_t *)(uintptr_t)(uVar3 + 4); /* local X (0x48)      */
        param_1[0x13] = uVar4;                              /* local Y (0x4c)        */
        param_1[0x14] = uVar5;                              /* local Z (0x50)        */
        param_1[9]    = param_1[0x12];                      /* world X (0x24)        */
        param_1[10]   = param_1[0x13];                      /* world Y (0x28)        */
        param_1[0xb]  = param_1[0x14];                      /* world Z (0x2c)        */

        /* Remove from back-buffer (MIPS: a0=s1=param_1 at jal site). */
        Object_ClearBackBufferFlag(param_1);
        uVar1 = 0;
    }
    return uVar1;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8003c288  (from analysis/SLUS_005.10/decomp/8003c288.c) --- */
// addr: 0x8003c288  name: FUN_8003c288

undefined4 FUN_8003c288(int param_1,int param_2)

{
  ushort uVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;

  iVar8 = *(int *)(param_2 + 4) - *(int *)(param_1 + 0x48);
  iVar3 = *(int *)(param_2 + 8) - *(int *)(param_1 + 0x4c);
  iVar4 = *(int *)(param_2 + 0xc) - *(int *)(param_1 + 0x50);
  if (-1 < *(char *)(param_1 + 8)) {
    iVar5 = iVar8;
    if (iVar8 < 0) {
      iVar5 = -iVar8;
    }
    iVar7 = iVar3;
    if (iVar3 < 0) {
      iVar7 = -iVar3;
    }
    if (iVar7 < iVar5) {
      iVar7 = iVar5;
    }
    iVar5 = iVar4;
    if (iVar4 < 0) {
      iVar5 = -iVar4;
    }
    if (iVar5 < iVar7) {
      iVar5 = iVar7;
    }
    if (iVar5 < 0x801) {
      return 1;
    }
  }
  iVar5 = iVar8;
  if (iVar8 < 0) {
    iVar5 = iVar8 + 0x1f;
  }
  iVar7 = iVar4;
  if (iVar4 < 0) {
    iVar7 = iVar4 + 7;
  }
  *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + (iVar5 >> 5) + (iVar7 >> 3);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xf;
  }
  *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + (iVar3 >> 4);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  if (iVar8 < 0) {
    iVar8 = iVar8 + 7;
  }
  *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + ((iVar4 >> 5) - (iVar8 >> 3));
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x48);
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_1 + 0x4c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x50);
  uVar6 = 0;
  if ((*(char *)(param_1 + 8) < '\0') &&
     (uVar6 = 0, (iRam0000000c - (uint)*(byte *)(param_1 + 9) & 3) == 0)) {
    iVar3 = FUN_8001d5a0(param_1);
    iVar8 = 0;
    iVar4 = 0x24;
    do {
      iVar5 = *(int *)(iVar3 + iVar4 + 0xec);
      if ((int)*(char *)(iVar5 + 8) == -(int)*(char *)(param_1 + 8)) {
        if (*(code **)(iVar5 + 100) == (code *)0x0) {
          iVar5 = 0;
        }
        else {
          iVar5 = (**(code **)(iVar5 + 100))(iVar5,0xf,param_1);
        }
        if (iVar5 == 0) {
          iVar5 = *(int *)(iVar3 + iVar4 + 0xec);
          uVar1 = *(ushort *)(iVar5 + 0xc);
          if (uVar1 < 99) {
            *(ushort *)(iVar5 + 0xc) = uVar1 + 1;
          }
        }
      }
      iVar8 = iVar8 + 1;
      iVar4 = iVar4 + 4;
    } while (iVar8 < 3);
    sVar2 = *(short *)(param_1 + 0xc) + -1;
    *(short *)(param_1 + 0xc) = sVar2;
    if (sVar2 == 0) {
      uVar6 = FUN_8004410c();
      iVar3 = FUN_8001d624(param_1);
      FUN_8004483c(uVar6,uRam000005f8,0x28,iVar3 + 0x14);
      FUN_8001d564(param_1);
      FUN_800204dc(param_1);
      uVar6 = 0xffffffff;
    }
    else {
      uVar6 = 0;
    }
  }
  return uVar6;
}

/* --- SLUS_005.10 FUN_8003c538  (from analysis/SLUS_005.10/decomp/8003c538.c) --- */
// addr: 0x8003c538  name: FUN_8003c538

undefined4 FUN_8003c538(uint *param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;

  if ((*param_1 & 0x10000) == 0) {
    uVar1 = 1;
  }
  else {
    if ((param_2 < 0x80000001) && (iVar2 = FUN_8003c288(param_1,param_1[0x20]), iVar2 == 0)) {
      return 0;
    }
    uVar1 = FUN_8004410c();
    iVar2 = FUN_8001d624(param_1);
    FUN_8004483c(uVar1,uRam000005f8,0x2c,iVar2 + 0x14);
    uVar3 = param_1[0x20];
    *param_1 = *param_1 & 0xfffeffff;
    uVar4 = *(uint *)(uVar3 + 8);
    uVar5 = *(uint *)(uVar3 + 0xc);
    param_1[0x12] = *(uint *)(uVar3 + 4);
    param_1[0x13] = uVar4;
    param_1[0x14] = uVar5;
    param_1[9] = param_1[0x12];
    param_1[10] = param_1[0x13];
    param_1[0xb] = param_1[0x14];
    FUN_80020778();
    uVar1 = 0;
  }
  return uVar1;
}

#endif  /* GHIDRA REF */
