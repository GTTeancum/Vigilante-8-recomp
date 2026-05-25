/* boulder_roll.c -- Canyonlands rolling-boulder physics+impact.
 *
 * Source: CANYNLND.DLL  FUN_80100244.
 *
 * Per-tick handler for a rolling-boulder destructible: integrates
 * velocity against terrain with surface-normal bounce response,
 * applies +0x5a gravity to Y, computes spin axis from horizontal
 * velocity, retires on hard impact or after first vehicle hit.
 *
 * mode dispatch:
 *   0  -- per-tick: terrain query + bounce + integrate + spin axis
 *   2  -- final retire (spawn rubble + free)
 *   3  -- impactor collision (param_3 = impactor):
 *         kind 7 = vehicle hit; scale Y vel by 0x93c/4096, recompute
 *                  spin period = 0x1000000 / (Y * 0x3243); retire if
 *                  not already breaking.
 *
 * Offsets: self+9..b = world pos (mirrors +0x12..14);
 *          self+0x15 gravity Y offset; self+0x1c = debris cell ptr;
 *          self+0x20..22 vel; self+0x23..24 spin axis;
 *          self+0x25 inv spin radius (4.12).
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_VsImpactor(void *self, int kind);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void Damage_RetireSelf(uint32_t *self);
extern int64_t GTE_Dot32x16(uint32_t *v, uint32_t *axis);
extern void FX_RingFlash(int imp, void *p, uint32_t *xyz);
extern void Damage_VsImpactorAlt(int imp, int kind, void *p, int n);
extern void FX_RingFlash_Init(uint32_t *self, void *p);
extern void Object_RubbleSpawn(uint8_t *bp, uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern uint32_t Pool_AllocProjectile(uint32_t kind);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int u, void *xyz);
extern uint8_t  DAT_80100044, DAT_801012a0;
extern int8_t   DAT_801012ac, _DAT_000012ad;
extern uint32_t _DAT_80065310;

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int32_t rtz_shift_i32(int32_t v, unsigned sh, int32_t bias)
{
    if (v < 0) v = mips_addu_i32(v, bias);
    return v >> sh;
}

static int32_t mips_abs_i32(int32_t v)
{
    return (v < 0) ? mips_subu_i32(0, v) : v;
}

uint32_t CL_RollingBoulder(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    uint32_t hit = 2;
    int16_t  n[3] = { 0, 0, 0 };

    if (mode == 2) goto retire;
    if (mode != 3) {
        uint32_t pos[3] = { self[9], (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x15]), self[0xb] };
        int gy = Terrain_QueryAt(self, pos, n, 0);
        if (gy < mips_addu_i32((int32_t)pos[1], 0x800)) {
            int32_t vdot = mips_addu_i32(
                mips_addu_i32(mips_mult_lo_i32((int32_t)self[0x20], n[0]),
                              mips_mult_lo_i32((int32_t)self[0x21], n[1])),
                mips_mult_lo_i32((int32_t)self[0x22], n[2]));
            vdot = rtz_shift_i32(vdot, 11, 0x7ff);
            int32_t vy2 = mips_sll_i32((int32_t)self[0x21], 1);
            if (vdot < 0) {
                int32_t ax = rtz_shift_i32(mips_mult_lo_i32(vdot, n[0]), 12, 0xfff);
                int32_t ay = rtz_shift_i32(mips_mult_lo_i32(vdot, n[1]), 12, 0xfff);
                int32_t az = rtz_shift_i32(mips_mult_lo_i32(vdot, n[2]), 12, 0xfff);
                self[0x20] = (uint32_t)mips_subu_i32((int32_t)self[0x20], ax);
                self[0x21] = (uint32_t)mips_subu_i32((int32_t)self[0x21], ay);
                self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], az);
                vy2 = (int32_t)self[0x21] / 2;
                self[0x21]   = vy2;
                self[10]     = (uint32_t)mips_subu_i32(gy, (int32_t)self[0x15]);
            }
            int32_t rollX = rtz_shift_i32(mips_mult_lo_i32(mips_addu_i32(vy2, (int32_t)self[0x20]), 0x1e), 12, 0xfff);
            self[0x20] = (uint32_t)mips_addu_i32((int32_t)self[0x20], rollX);
            int32_t fz = rtz_shift_i32(mips_mult_lo_i32(n[2], 0x5a), 12, 0xfff);
            self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22], fz);
            int32_t sx = rtz_shift_i32(mips_mult_lo_i32(mips_subu_i32(0, (int32_t)self[0x22]),
                                                        (uint16_t)self[0x25]), 12, 0xfff);
            int32_t sz = rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x20], (uint16_t)self[0x25]), 12, 0xfff);
            *(int16_t *)(self + 0x23) = (int16_t)sx;
            *(int16_t *)(self + 0x24) = (int16_t)sz;
            uint32_t hard = (vdot < -0x1c9);
            if (n[1] < -0xe66) {
                uint32_t ax = (uint32_t)mips_abs_i32((int32_t)self[0x21]);
                uint32_t bx = (uint32_t)mips_abs_i32((int32_t)self[0x20]);
                if (ax < bx) ax = bx;
                uint32_t cx = (uint32_t)mips_abs_i32((int32_t)self[0x22]);
                if (cx < ax) cx = ax;
                if (cx < 0x42c) { hard = (uint32_t)Damage_AccumulateOrFire(self, 0); hit = 1; }
            }
            if (hard) {
                uint32_t h = Pool_AllocProjectile(hit);
                Pool_LaunchProjectile(h, *(uint32_t *)(self[0x16] + 8), 0, self + 9);
                goto post_gravity;
            }
        } else {
post_gravity:
            self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], 0x5a);
        }
        arg = self + 4;
        Object_OrientByAxis(arg, arg, self + 0x23);
        self[9]  = (uint32_t)mips_addu_i32((int32_t)self[9], (int32_t)self[0x20]);
        self[10] = (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x21]);
        self[0xb]= (uint32_t)mips_addu_i32((int32_t)self[0xb], (int32_t)self[0x22]);
        if ((mips_subu_i32((int32_t)_DAT_80065310, *((uint8_t *)self + 9)) & 0xf) != 0)
            return 0;
        MatrixNormal(arg, arg);
    }
    /* Damage / impactor handling. */
    uint32_t imp  = *arg;
    uint8_t  kind = *(uint8_t *)(imp + 4);
    if (kind == 7) {
        if (Damage_VsImpactor(self, (int)*(uint16_t *)(imp + 0xc)) == 0) return 0;
        uint32_t g = self[0x15];
        if (g == 0) goto retire;
        uint32_t debris = self[0x1c];
        if ((int)g < 0) g = (uint32_t)mips_addu_i32((int32_t)g, 0xf);
        *(int32_t *)(debris + 0x28) = (int32_t)g >> 4;
        *(int32_t *)(debris + 0x24) = (int32_t)g >> 4;
        int32_t sc = rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x15], 0x93c), 12, 0xfff);
        self[0x15] = (uint32_t)sc;
        int32_t inv = mips_mult_lo_i32(sc, 0x3243);
        if (inv < 0) inv = mips_addu_i32(inv, 0xfff);
        *(int16_t *)(self + 0x25) = (int16_t)(0x1000000 / (inv >> 12));
        if ((*self & 0x80) != 0) return 0;
        Damage_RetireSelf(self);
    }
    if ((*self & 0x80) == 0) return 0;
    if (kind == 7) {
        uint32_t scale = 0x10000 / *(uint16_t *)(imp + 0xa2);
        int64_t inSide = GTE_Dot32x16(self + 0x20, (uint32_t *)(imp + 0x80));
        if (inSide > 0) return 0;
        int32_t ix = mips_mult_lo_i32((int32_t)self[0x20], (int32_t)scale);
        int32_t iy = mips_mult_lo_i32((int32_t)self[0x21], (int32_t)scale);
        int32_t iz = mips_mult_lo_i32((int32_t)self[0x22], (int32_t)scale);
        int32_t c[3];
        c[0] = (ix < -0x100000) ? -0x100000 : (ix > 0x100000) ? 0x100000 : ix;
        c[1] = (iy < -0x100000) ? -0x100000 : (iy > 0x100000) ? 0x100000 : iy;
        c[2] = (iz < -0x100000) ? -0x100000 : (iz > 0x100000) ? 0x100000 : iz;
        FX_RingFlash((int)imp, c, self + 9);
        Damage_VsImpactorAlt(*arg, -100, &DAT_80100044, 1);
    }
    FX_RingFlash_Init(self, (void *)(uintptr_t)imp);
    int32_t vdot = mips_addu_i32(
        mips_addu_i32(mips_mult_lo_i32((int32_t)self[0x20], (int16_t)*(uint32_t *)(imp + 0x20)),
                      mips_mult_lo_i32((int32_t)self[0x21], *(int16_t *)(imp + 0x22))),
        mips_mult_lo_i32((int32_t)self[0x22], (int16_t)*(uint32_t *)(imp + 0x24)));
    vdot = rtz_shift_i32(vdot, 11, 0x7ff);
    if (vdot >= 0) return 0;
    int32_t bx = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)*(uint32_t *)(imp + 0x20)), 12, 0xfff);
    int32_t by = rtz_shift_i32(mips_mult_lo_i32(vdot, *(int16_t *)(imp + 0x22)), 12, 0xfff);
    int32_t bz = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)*(uint32_t *)(imp + 0x24)), 12, 0xfff);
    self[0x20] = (uint32_t)mips_subu_i32((int32_t)self[0x20], bx);
    self[0x21] = (uint32_t)mips_subu_i32((int32_t)self[0x21], by);
    self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], bz);
retire:
    Object_RubbleSpawn(&DAT_801012a0, self);
    Object_RetireDeferred(self);
    _DAT_000012ad = (int8_t)mips_addu_i32(DAT_801012ac, -1);
    return 0;
}

uint32_t FUN_80100244(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    return CL_RollingBoulder(self, mode, arg);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- CANYNLND.DLL FUN_80100244  (from analysis/dll/CANYNLND/decomp/80100244.c) --- */
// addr: 0x80100244  name: FUN_80100244

/* WARNING: Removing unreachable block (ram,0x80100704) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100244(uint *param_1,uint param_2,uint *param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  uint extraout_v1;
  uint uVar7;
  uint *puVar8;
  longlong lVar9;
  uint local_48;
  int local_44;
  uint local_40;
  short local_38;
  short local_36;
  short local_34;
  undefined4 local_28;
  int local_24;
  int local_20;
  
  uVar7 = 2;
  if (param_2 == 2) goto LAB_801008f0;
  uVar1 = 3;
  if (((param_2 < 3) && (uVar1 = 0, param_2 == 0)) || (param_2 != uVar1)) {
    local_48 = param_1[9];
    local_44 = param_1[10] + param_1[0x15];
    local_40 = param_1[0xb];
    iVar2 = FUN_8001d748/*0x8001d748*/(param_1,&local_48,&local_38,0);
    if (iVar2 < local_44 + 0x800) {
      uVar7 = (uint)local_38;
      uVar1 = param_1[0x20];
      iVar3 = uVar1 * uVar7 + param_1[0x21] * (int)local_36 + param_1[0x22] * (int)local_34;
      if (iVar3 < 0) {
        iVar3 = iVar3 + 0x7ff;
      }
      iVar3 = iVar3 >> 0xb;
      uVar6 = uVar7 << 1;
      if (iVar3 < 0) {
        iVar4 = iVar3 * uVar7;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x20] = uVar1 - (iVar4 >> 0xc);
        iVar4 = iVar3 * local_36;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x21] = param_1[0x21] - (iVar4 >> 0xc);
        iVar4 = iVar3 * local_34;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        uVar6 = (int)param_1[0x21] / 2;
        uVar7 = iVar2 - param_1[0x15];
        param_1[0x22] = param_1[0x22] - (iVar4 >> 0xc);
        param_1[0x21] = uVar6;
        param_1[10] = uVar7;
      }
      iVar2 = (uVar6 + uVar7) * 0x1e;
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      param_1[0x20] = uVar1 + (iVar2 >> 0xc);
      iVar2 = local_34 * 0x5a;
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      param_1[0x22] = param_1[0x22] + (iVar2 >> 0xc);
      uVar7 = -param_1[0x22] * (uint)(ushort)param_1[0x25];
      if ((int)uVar7 < 0) {
        uVar7 = uVar7 + 0xfff;
      }
      iVar2 = param_1[0x20] * (uint)(ushort)param_1[0x25];
      *(short *)(param_1 + 0x23) = (short)((int)uVar7 >> 0xc);
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      *(short *)(param_1 + 0x24) = (short)(iVar2 >> 0xc);
      uVar1 = (uint)(iVar3 < -0x1c9);
      if (local_36 < -0xe66) {
        uVar7 = param_1[0x21];
        uVar1 = param_1[0x20];
        if ((int)uVar7 < 0) {
          uVar7 = -uVar7;
        }
        if ((int)uVar1 < 0) {
          uVar1 = -uVar1;
        }
        if ((int)uVar7 < (int)uVar1) {
          uVar7 = uVar1;
        }
        uVar6 = param_1[0x22];
        if ((int)uVar6 < 0) {
          uVar6 = -uVar6;
        }
        if ((int)uVar6 < (int)uVar7) {
          uVar6 = uVar7;
        }
        uVar1 = (uint)(iVar3 < -0x1c9);
        if ((int)uVar6 < 0x42c) {
          uVar1 = FUN_80020778/*0x80020778*/(param_1);
          uVar7 = 1;
        }
      }
      if (uVar1 != 0) {
        uVar5 = FUN_8004410c/*0x8004410c*/(uVar7);
        FUN_8004483c/*0x8004483c*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),0,param_1 + 9);
        goto LAB_8010054c;
      }
    }
    else {
LAB_8010054c:
      param_1[0x21] = param_1[0x21] + 0x5a;
    }
    param_3 = param_1 + 4;
    func_0x800172b4(param_3,param_3,param_1 + 0x23);
    uVar7 = param_1[0x20];
    uVar1 = _DAT_80065310 - (uint)*(byte *)((int)param_1 + 9);
    param_1[9] = param_1[9] + uVar7;
    param_1[10] = param_1[10] + param_1[0x21];
    param_1[0xb] = param_1[0xb] + param_1[0x22];
    if ((uVar1 & 0xf) != 0) {
      return 0;
    }
    MatrixNormal/*0x8004c934*/(param_3,param_3);
  }
  uVar6 = *param_3;
  uVar1 = (uint)*(byte *)(uVar6 + 4);
  if (uVar1 == 7) {
    iVar2 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(uVar6 + 0xc));
    if (iVar2 == 0) {
      return 0;
    }
    uVar1 = param_1[0x15];
    if (uVar1 == 0) goto LAB_801008f0;
    uVar6 = param_1[0x1c];
    if ((int)uVar1 < 0) {
      uVar1 = uVar1 + 0xf;
    }
    *(int *)(uVar6 + 0x28) = (int)uVar1 >> 4;
    *(int *)(uVar6 + 0x24) = (int)uVar1 >> 4;
    iVar2 = param_1[0x15] * 0x93c;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0xfff;
    }
    param_1[0x15] = iVar2 >> 0xc;
    iVar2 = (iVar2 >> 0xc) * 0x3243;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0xfff;
    }
    *(short *)(param_1 + 0x25) = (short)(0x1000000 / (iVar2 >> 0xc));
    if ((*param_1 & 0x80) != 0) {
      return 0;
    }
    FUN_80020744/*0x80020744*/(param_1);
    uVar6 = 1;
    uVar1 = extraout_v1;
  }
  if ((*param_1 & 0x80) == 0) {
    return 0;
  }
  puVar8 = param_3;
  if (uVar1 == uVar7) {
    puVar8 = (uint *)(0x10000 / *(ushort *)(uVar6 + 0xa2));
    lVar9 = FUN_800171d4/*0x800171d4*/(param_1 + 0x20,uVar6 + 0x80);
    if (0 < lVar9) {
      return 0;
    }
    if (((int)(param_1[0x20] * (int)puVar8) < -0x100000) ||
       (local_28 = 0x100000, (int)(param_1[0x20] * (int)puVar8) < 0x100001)) {
      local_28 = 0xfff00000;
    }
    iVar2 = param_1[0x21] * (int)puVar8;
    local_24 = -0x100000;
    if ((-0x100001 < iVar2) && (local_24 = 0x100000, iVar2 < 0x100001)) {
      local_24 = iVar2;
    }
    iVar2 = param_1[0x22] * (int)puVar8;
    iVar3 = -0x100000;
    if ((iVar2 < -0x100000) || (iVar3 = iVar2, local_20 = 0x100000, iVar2 < 0x100001)) {
      local_20 = iVar3;
    }
    FUN_800176f8/*0x800176f8*/(uVar6,&local_28,param_1 + 9);
    FUN_8002c958/*0x8002c958*/(*param_3,0xffffff9c,&DAT_80100044,1);
  }
  FUN_8001f5a0/*0x8001f5a0*/(param_1,puVar8);
  iVar2 = param_1[0x20] * (int)(short)puVar8[8] +
          param_1[0x21] * (int)*(short *)((int)puVar8 + 0x22) +
          param_1[0x22] * (int)(short)puVar8[9];
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7ff;
  }
  iVar2 = iVar2 >> 0xb;
  if (-1 < iVar2) {
    return 0;
  }
  iVar3 = iVar2 * (short)puVar8[8];
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  param_1[0x20] = param_1[0x20] - (iVar3 >> 0xc);
  iVar3 = iVar2 * *(short *)((int)puVar8 + 0x22);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  param_1[0x21] = param_1[0x21] - (iVar3 >> 0xc);
  iVar2 = iVar2 * (short)puVar8[9];
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  param_1[0x22] = param_1[0x22] - (iVar2 >> 0xc);
LAB_801008f0:
  FUN_8001fe8c/*0x8001fe8c*/(&DAT_801012a0,param_1);
  FUN_800205f8/*0x800205f8*/(param_1);
  _DAT_000012ad = DAT_801012ac + -1;
  return 0;
}

#endif  /* GHIDRA REF */
