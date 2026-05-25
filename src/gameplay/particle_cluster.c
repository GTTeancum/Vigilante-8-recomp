/* particle_cluster.c -- spawn a parent + N children particle cluster.
 *
 * Source: SLUS_005.10  FUN_80040c40.
 *
 * Allocates a parent object (0x9c bytes) with the cluster tick callback
 * at LAB_80040b38, then spawns `count` child particles around it.  Each
 * child has:
 *   - a randomised initial velocity within +/- bounds (X,Y) and a
 *     forward-biased Z velocity in [Z, Z+Z/2),
 *   - the velocity rotated into world space via the current GTE matrix
 *     (which the caller pre-loaded via SetRotMatrix),
 *   - a 5-bit-per-channel random colour at +0x80/+0x82/+0x84,
 *   - 12 bytes of zero at +0x48 (clears world-pos accumulator),
 *   - a lifetime tick counter at +0x94 in [0xf, 0x1e).
 *
 * Used by weapon explosions, vehicle wreck sparks, and effect spawns.
 *
 * HIGH confidence (direct Ghidra port).  The Ghidra pseudo-C dropped the
 * `child` arg on the FUN_8001d4f0 tail-call; the MIPS preserves it in
 * $a1 from iVar3 -- restored here.
 *
 */
#include <stdint.h>
#include "structs.h"

/* ---- Object allocator + bone integration ---- */
extern void    *FUN_8001d470(uint32_t size);                /* Object_AllocSmall */
extern int      FUN_8001ac44(int *pool, uint16_t kind,
                             uint32_t flags, uint32_t mode);/* BoneObj_BuildTree */
extern void     FUN_8001d708(void *obj);                    /* BuildBoneMatrix */
extern void     FUN_8001d4f0(uint32_t *parent, uint32_t *child); /* Object_AppendChild */
extern int      FUN_8001d564(int self);                     /* Object_DetachFromParent */
extern void     FUN_8001af48(int obj);                      /* Object_FreeTreeQuiet */
extern void     FUN_800205f8(void *obj);                    /* Object_Remove */

/* ---- GTE / math helpers ---- */
extern void     SetRotMatrix(const MATRIX *m);
extern int     *FUN_80043248(int *src, int *dst);           /* GTE_RotateLong */

/* ---- memset + RNG ---- */
extern void    *FUN_80044efc(void *dst, uint32_t fill, int n); /* memset */
extern int      FUN_80017160(void);                         /* v8_rand (0..0x7fff) */

extern void     Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* LAB_80040b38 -- ParticleCluster_Tick.
 * MED: decoded from the raw gap immediately preceding FUN_80040c40. */
int LAB_80040b38(uint32_t *obj, int event, int param3)
{
    (void)param3;

    if (event != 0)
        return 0;

    uint32_t *child = (uint32_t *)(uintptr_t)obj[0x0e];
    if (child == 0) {
        FUN_800205f8(obj);
        return 0;
    }

    while (child != 0) {
        uint32_t *next = (uint32_t *)(uintptr_t)child[0x0d];
        int32_t life = mips_subu_i32((int32_t)child[0x25], 1);
        child[0x25] = (uint32_t)life;
        if (life == 0) {
            FUN_8001d564((int)(uintptr_t)child);
            FUN_8001af48((int)(uintptr_t)child);
        } else {
            child[0x12] = (uint32_t)mips_addu_i32((int32_t)child[0x12], (int32_t)child[0x22]);
            child[0x13] = (uint32_t)mips_addu_i32((int32_t)child[0x13], (int32_t)child[0x23]);
            child[0x14] = (uint32_t)mips_addu_i32((int32_t)child[0x14], (int32_t)child[0x24]);

            *(uint16_t *)((uint8_t *)child + 0x40) =
                (uint16_t)mips_addu_i32(
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x40),
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x80));
            *(uint16_t *)((uint8_t *)child + 0x42) =
                (uint16_t)mips_addu_i32(
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x42),
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x82));
            *(uint16_t *)((uint8_t *)child + 0x44) =
                (uint16_t)mips_addu_i32(
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x44),
                    (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)child + 0x84));

            FUN_8001d708(child);
            child[0x23] = (uint32_t)mips_addu_i32((int32_t)child[0x23], 0x5a);
        }
        child = next;
    }

    return 0;
}

/* ================================================================
 * FUN_80040c40 -- ParticleCluster_Spawn
 *
 * Returns the parent object pointer.
 * ================================================================ */
uint32_t *FUN_80040c40(int *param_1, uint16_t param_2, MATRIX *param_3,
                       int *param_4, int param_5)
{
    uint32_t *puVar2;
    int       iVar3;
    int       iVar4;
    uint16_t  uVar1;
    int       iVar7;
    int       local_38, local_34, local_30;

    /* Allocate + initialise the parent. */
    puVar2 = (uint32_t *)FUN_8001d470(0x9c);
    *puVar2 |= 0xa0u;
    puVar2[0x12] = (uint32_t)param_3->t[0];
    puVar2[0x13] = (uint32_t)param_3->t[1];
    puVar2[0x14] = (uint32_t)param_3->t[2];
    Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_80040b38);
    puVar2[0x15] = 0x10000;
    FUN_8001d708(puVar2);
    SetRotMatrix(param_3);

    /* Spawn `param_5` children. */
    iVar7 = 0;
    if (0 < param_5) {
        do {
            iVar3 = FUN_8001ac44(param_1, param_2, 0xa0u, 0);

            iVar4 = FUN_80017160();
            local_38 = mips_subu_i32(
                mips_mult_lo_i32(mips_mult_lo_i32(iVar4, 2), *param_4) >> 15,
                *param_4);
            iVar4 = FUN_80017160();
            local_34 = mips_subu_i32(
                mips_mult_lo_i32(mips_mult_lo_i32(iVar4, 2), param_4[1]) >> 15,
                param_4[1]);
            iVar4 = FUN_80017160();
            local_30 = mips_addu_i32(
                mips_mult_lo_i32(iVar4, param_4[2] / 2) >> 15,
                param_4[2]);

            FUN_80043248(&local_38, (int *)(uintptr_t)(iVar3 + 0x88));

            uVar1 = (uint16_t)FUN_80017160();
            *(uint16_t *)(uintptr_t)(iVar3 + 0x80) = uVar1 & 0x1f;
            uVar1 = (uint16_t)FUN_80017160();
            *(uint16_t *)(uintptr_t)(iVar3 + 0x82) = uVar1 & 0x1f;
            uVar1 = (uint16_t)FUN_80017160();
            *(uint16_t *)(uintptr_t)(iVar3 + 0x84) = uVar1 & 0x1f;

            FUN_80044efc((void *)(uintptr_t)(iVar3 + 0x48), 0, 0xc);

            iVar4 = FUN_80017160();
            *(int32_t *)(uintptr_t)(iVar3 + 0x94) =
                mips_addu_i32(mips_mult_lo_i32(iVar4, 0xf) >> 15, 0xf);

            /* Ghidra dropped `iVar3` (child) from the call -- restored. */
            FUN_8001d4f0(puVar2, (uint32_t *)(uintptr_t)iVar3);

            iVar7 = mips_addu_i32(iVar7, 1);
        } while (iVar7 < param_5);
    }

    return puVar2;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80040c40 --- */
uint * FUN_80040c40(undefined4 param_1,undefined2 param_2,MATRIX *param_3,int *param_4,int param_5)
{
  ushort uVar1;
  uint *puVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int local_38;
  int local_34;
  int local_30;

  puVar2 = (uint *)FUN_8001d470(0x9c);
  iVar7 = 0;
  *puVar2 = *puVar2 | 0xa0;
  uVar5 = param_3->t[1];
  uVar6 = param_3->t[2];
  puVar2[0x12] = param_3->t[0];
  puVar2[0x13] = uVar5;
  puVar2[0x14] = uVar6;
  puVar2[0x19] = (uint)&LAB_80040b38;
  puVar2[0x15] = 0x10000;
  FUN_8001d708(puVar2);
  SetRotMatrix(param_3);
  if (0 < param_5) {
    do {
      iVar3 = FUN_8001ac44(param_1,param_2,0xa0,0);
      iVar4 = FUN_80017160();
      local_38 = (iVar4 * 2 * *param_4 >> 0xf) - *param_4;
      iVar4 = FUN_80017160();
      local_34 = (iVar4 * 2 * param_4[1] >> 0xf) - param_4[1];
      iVar4 = FUN_80017160();
      iVar7 = iVar7 + 1;
      local_30 = (iVar4 * (param_4[2] / 2) >> 0xf) + param_4[2];
      FUN_80043248(&local_38,iVar3 + 0x88);
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x80) = uVar1 & 0x1f;
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x82) = uVar1 & 0x1f;
      uVar1 = FUN_80017160();
      *(ushort *)(iVar3 + 0x84) = uVar1 & 0x1f;
      FUN_80044efc(iVar3 + 0x48,0,0xc);
      iVar4 = FUN_80017160();
      *(int *)(iVar3 + 0x94) = (iVar4 * 0xf >> 0xf) + 0xf;
      FUN_8001d4f0(puVar2);
    } while (iVar7 < param_5);
  }
  return puVar2;
}

#endif  /* GHIDRA REF */
