/* barrel_roll.c -- Oil Fields rolling-barrel physics+impact.
 *
 * Source: OILFIELD.DLL  FUN_80100a30.
 *
 * Same rolling-object idiom as Canyonlands boulder_roll.c, applied
 * to an oil barrel. Different offsets (self+0x24..2c position vs
 * +9..b on Canyonlands) because the Oil Fields object layout omits
 * the cached short-position alias.
 *
 * Velocity:   self+0x80..88 (X,Y,Z)
 * Spin axis:  self+0x8c, +0x90 (X,Z); inv-radius self+0x94
 * Bounce:     v -= 2 * (v.n) * n; Y bounce halved (Y *= 0.5)
 * Gravity:    +0x5a on Y per frame
 * Friction:   horizontal roll-back +0x1e via (vy + vx)*0x1e/4096
 *
 * Damage path:
 *   - impactor.kind == 7 (vehicle): standard Damage_VsImpactor;
 *     compute small relative-velocity impulse scaled 8x; on damage
 *     death, detach submodel and retire.
 *   - impactor.kind == 2 (shrapnel): face-normal bounce only.
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis); /* func_0x800172b4 */
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_VsImpactor(int self, int kind);
extern void SubModel_Detach(int self);
extern void Object_RetireDeferred(int self);
extern void FX_RingFlash_Init(int self, int *imp);
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

uint32_t OF_BarrelRoll(int self, int mode, int *arg)
{
    int16_t n[3] = { 0, 0, 0 };
    if (mode == 0 || mode != 3) {
        uint32_t pos[3];
        pos[0] = *(uint32_t *)(self + 0x24);
        pos[1] = (uint32_t)mips_addu_i32(*(int32_t *)(self + 0x28),
                                         *(int32_t *)(self + 0x54));
        pos[2] = *(uint32_t *)(self + 0x2c);
        int gy = Terrain_QueryAt((uint32_t *)(intptr_t)self, pos, n, 0);
        if (gy < mips_addu_i32((int32_t)pos[1], 0x800)) {
            int vx = *(int *)(self + 0x80);
            int vy = *(int *)(self + 0x84);
            int vz = *(int *)(self + 0x88);
            int vdot = mips_addu_i32(
                mips_addu_i32(mips_mult_lo_i32(vx, n[0]), mips_mult_lo_i32(vy, n[1])),
                mips_mult_lo_i32(vz, n[2]));
            vdot = rtz_shift_i32(vdot, 11, 0x7ff);
            int dy = mips_sll_i32(n[0], 1);
            if (vdot < 0) {
                int ax = rtz_shift_i32(mips_mult_lo_i32(vdot, n[0]), 12, 0xfff);
                int ay = rtz_shift_i32(mips_mult_lo_i32(vdot, n[1]), 12, 0xfff);
                int az = rtz_shift_i32(mips_mult_lo_i32(vdot, n[2]), 12, 0xfff);
                *(int *)(self + 0x80) = mips_subu_i32(vx, ax);
                *(int *)(self + 0x84) =
                    (mips_addu_i32(mips_subu_i32(*(int *)(self + 0x84), ay),
                                   (int32_t)((uint32_t)mips_subu_i32(*(int *)(self + 0x84), ay) >> 31)) >> 1);
                *(int *)(self + 0x88) = mips_subu_i32(vz, az);
                *(int *)(self + 0x28) = 1;
                dy = az;
            }
            int rollX = rtz_shift_i32(mips_mult_lo_i32(mips_addu_i32(dy, n[0]), 0x1e), 12, 0xfff);
            *(int *)(self + 0x80) = mips_addu_i32(vx, rollX);
            int fz = rtz_shift_i32(mips_mult_lo_i32(n[2], 0x5a), 12, 0xfff);
            *(int *)(self + 0x88) = mips_addu_i32(*(int *)(self + 0x88), fz);
            int sx = rtz_shift_i32(mips_mult_lo_i32(mips_subu_i32(0, *(int *)(self + 0x88)),
                                                    (uint16_t)*(uint16_t *)(self + 0x94)), 12, 0xfff);
            int sz = mips_mult_lo_i32(*(int *)(self + 0x80), (uint16_t)*(uint16_t *)(self + 0x94));
            *(int16_t *)(self + 0x8c) = (int16_t)sx;
            *(int16_t *)(self + 0x90) = (int16_t)rtz_shift_i32(sz, 12, 0xfff);
        }
        *(int *)(self + 0x84) = mips_addu_i32(*(int *)(self + 0x84), 0x5a);
        int *m = (int *)(self + 0x10);
        Object_OrientByAxis((uint32_t *)m, (uint32_t *)m, (uint32_t *)(self + 0x8c));
        *(int *)(self + 0x24) = mips_addu_i32(*(int *)(self + 0x24), *(int *)(self + 0x80));
        *(int *)(self + 0x28) = mips_addu_i32(*(int *)(self + 0x28), *(int *)(self + 0x84));
        *(int *)(self + 0x2c) = mips_addu_i32(*(int *)(self + 0x2c), *(int *)(self + 0x88));
        if ((mips_subu_i32((int32_t)_DAT_80065310, *(uint8_t *)(self + 9)) & 0xf) != 0)
            return 0;
        MatrixNormal((uint32_t *)m, (uint32_t *)m);
        arg = m;
    }
    int imp  = *arg;
    uint8_t k = *(uint8_t *)(imp + 4);
    uint8_t want = 2;
    if (k == 7) {
        if (Damage_VsImpactor(self, (int)*(int16_t *)(imp + 0xc)) == 0) return 0;
        want = 0;
        imp = 1;
    }
    if (k == want) {
        int local_20 = mips_sll_i32(mips_subu_i32(*(int *)(imp + 0x24), *(int *)(self + 0x24)), 3);
        int local_1c = mips_sll_i32(mips_subu_i32(*(int *)(imp + 0x28), *(int *)(self + 0x28)), 3);
        int local_18 = mips_sll_i32(mips_subu_i32(*(int *)(imp + 0x2c), *(int *)(self + 0x2c)), 3);
        (void)local_20;
        (void)local_1c;
        (void)local_18;
        SubModel_Detach(self);
        Object_RetireDeferred(self);
    }
    FX_RingFlash_Init(self, arg);
    int vdot = mips_addu_i32(
        mips_addu_i32(mips_mult_lo_i32(*(int *)(self + 0x80), (int16_t)arg[8]),
                      mips_mult_lo_i32(*(int *)(self + 0x84), *(int16_t *)((char *)arg + 0x22))),
        mips_mult_lo_i32(*(int *)(self + 0x88), (int16_t)arg[9]));
    vdot = rtz_shift_i32(vdot, 11, 0x7ff);
    if (vdot >= 0) return 0;
    int bx = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)arg[8]), 12, 0xfff);
    int by = rtz_shift_i32(mips_mult_lo_i32(vdot, *(int16_t *)((char *)arg + 0x22)), 12, 0xfff);
    int bz = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)arg[9]), 12, 0xfff);
    *(int *)(self + 0x80) = mips_subu_i32(*(int *)(self + 0x80), bx);
    *(int *)(self + 0x84) = mips_subu_i32(*(int *)(self + 0x84), by);
    *(int *)(self + 0x88) = mips_subu_i32(*(int *)(self + 0x88), bz);
    return 0;
}

uint32_t FUN_80100a30(int self, int mode, int *arg)
{
    return OF_BarrelRoll(self, mode, arg);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- OILFIELD.DLL FUN_80100a30  (from analysis/dll/OILFIELD/decomp/80100a30.c) --- */
// addr: 0x80100a30  name: FUN_80100a30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100a30(int param_1,int param_2,int *param_3)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined8 uVar7;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  short local_30;
  short local_2e;
  short local_2c;
  int local_20;
  int local_1c;
  int local_18;
  
  if ((param_2 == 0) || (param_2 != 3)) {
    local_40 = *(undefined4 *)(param_1 + 0x24);
    local_3c = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x54);
    local_38 = *(undefined4 *)(param_1 + 0x2c);
    iVar1 = FUN_8001d748/*0x8001d748*/(param_1,&local_40,&local_30,0);
    if (iVar1 < local_3c + 0x800) {
      iVar5 = (int)local_30;
      iVar6 = *(int *)(param_1 + 0x80);
      iVar1 = iVar6 * iVar5 + *(int *)(param_1 + 0x84) * (int)local_2e +
              *(int *)(param_1 + 0x88) * (int)local_2c;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x7ff;
      }
      iVar1 = iVar1 >> 0xb;
      iVar3 = iVar5 << 1;
      if (iVar1 < 0) {
        iVar3 = iVar1 * iVar5;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        *(int *)(param_1 + 0x80) = iVar6 - (iVar3 >> 0xc);
        iVar3 = iVar1 * local_2e;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        *(int *)(param_1 + 0x84) = (*(int *)(param_1 + 0x84) - (iVar3 >> 0xc)) / 2;
        iVar1 = iVar1 * local_2c;
        if (iVar1 < 0) {
          iVar1 = iVar1 + 0xfff;
        }
        iVar3 = iVar1 >> 0xc;
        *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - iVar3;
        *(undefined4 *)(param_1 + 0x28) = 1;
      }
      iVar1 = (iVar3 + iVar5) * 0x1e;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      *(int *)(param_1 + 0x80) = iVar6 + (iVar1 >> 0xc);
      iVar1 = local_2c * 0x5a;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + (iVar1 >> 0xc);
      iVar1 = -*(int *)(param_1 + 0x88) * (uint)*(ushort *)(param_1 + 0x94);
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      iVar5 = *(int *)(param_1 + 0x80) * (uint)*(ushort *)(param_1 + 0x94);
      *(short *)(param_1 + 0x8c) = (short)(iVar1 >> 0xc);
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xfff;
      }
      *(short *)(param_1 + 0x90) = (short)(iVar5 >> 0xc);
    }
    *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) + 0x5a;
    param_3 = (int *)(param_1 + 0x10);
    func_0x800172b4(param_3,param_3,param_1 + 0x8c);
    uVar2 = _DAT_80065310 - (uint)*(byte *)(param_1 + 9);
    *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + *(int *)(param_1 + 0x80);
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x84);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + *(int *)(param_1 + 0x88);
    if ((uVar2 & 0xf) != 0) {
      return 0;
    }
    MatrixNormal/*0x8004c934*/(param_3,param_3);
  }
  iVar1 = *param_3;
  uVar4 = (uint)*(byte *)(iVar1 + 4);
  uVar2 = 2;
  if (uVar4 == 7) {
    uVar7 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(iVar1 + 0xc));
    uVar4 = (uint)((ulonglong)uVar7 >> 0x20);
    uVar2 = 0;
    if ((int)uVar7 == 0) {
      return 0;
    }
    iVar1 = 1;
  }
  if (uVar4 == uVar2) {
    local_20 = (*(int *)(iVar1 + 0x24) - *(int *)(param_1 + 0x24)) * 8;
    local_1c = (*(int *)(iVar1 + 0x28) - *(int *)(param_1 + 0x28)) * 8;
    local_18 = (*(int *)(iVar1 + 0x2c) - *(int *)(param_1 + 0x2c)) * 8;
    FUN_8003fc50/*0x8003fc50*/(param_1);
    FUN_800205f8/*0x800205f8*/(param_1);
  }
  FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
  iVar1 = *(int *)(param_1 + 0x80) * (int)(short)param_3[8] +
          *(int *)(param_1 + 0x84) * (int)*(short *)((int)param_3 + 0x22) +
          *(int *)(param_1 + 0x88) * (int)(short)param_3[9];
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7ff;
  }
  iVar1 = iVar1 >> 0xb;
  if (iVar1 < 0) {
    iVar5 = iVar1 * (short)param_3[8];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) - (iVar5 >> 0xc);
    iVar5 = iVar1 * *(short *)((int)param_3 + 0x22);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xfff;
    }
    *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) - (iVar5 >> 0xc);
    iVar1 = iVar1 * (short)param_3[9];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) - (iVar1 >> 0xc);
  }
  return 0;
}

#endif  /* GHIDRA REF */
