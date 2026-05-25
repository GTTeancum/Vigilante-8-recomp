/* saloon_destruct.c -- Wild West saloon destructible facade.
 *
 * Source: WILDWEST.DLL  FUN_8010178c.
 *
 * Heavy loose saloon panel/chunk callback. The active host code used
 * to model this as a facade crumble shortcut, but the original overlay
 * is the same rolling/destructible physics idiom as barrels/boulders:
 * terrain bounce, spin-axis integration, impact impulse, parent detach,
 * and radius/period shrink when the piece is finalized.
 *
 * mode: 0=tick, 1/6=shrink/finalize, 2=detach-if-parented, 3=impact.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern void Object_RefitAABB(uint32_t *self);
extern void Audio_PlaySfxRelative(uint32_t bank, int sfxId, void *pos);
extern void FUN_8004366c(uint32_t *m);
extern int *FUN_80043248(int32_t *v, int32_t *out);
extern void FUN_8004cdc4(uint32_t *matA, uint32_t *matB);
extern int FUN_8001fe8c(void *listHead, uint32_t *obj);
extern void FUN_8001d4f0(uint32_t *parent, uint32_t *child);
extern uint32_t FUN_80017160(void);
extern void FUN_80020890(uint32_t *obj, int timer);
extern void FX_RingFlash_Init(uint32_t *self, void *p);
extern int FUN_8001d5a0(int self);
extern uint32_t *FUN_8001d624(int obj);
extern int FUN_8001d564(int self);
extern void FUN_8001fe50(void *listHead, void *obj);
extern uint8_t DAT_80065a18[];
extern uint32_t _DAT_80065310, _DAT_80065b3c;

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

static void saloon_scale_radius(uint32_t *self)
{
    int32_t h = rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x15], 0x93c), 12, 0xfff);
    *self |= 0x80u;
    self[0x15] = (uint32_t)h;
    int32_t den = rtz_shift_i32(mips_mult_lo_i32(h, 0x3243), 12, 0xfff);
    if (den != 0)
        *(int16_t *)(self + 0x25) = (int16_t)(0x1000000 / den);
}

static void saloon_detach_parented(uint32_t *self)
{
    if (self[0xf] == 0) return;

    int parent = FUN_8001d5a0((int)(uintptr_t)self);
    if (parent != 0) {
        int32_t v = *(int32_t *)(uintptr_t)(parent + 0xdc);
        *(int32_t *)(uintptr_t)(parent + 0xdc) = rtz_shift_i32(v, 2, 3);
    }

    uint32_t *world = FUN_8001d624((int)(uintptr_t)self);
    for (int i = 0; i < 8; i++)
        self[4 + i] = world[i];

    int detached = FUN_8001d564((int)(uintptr_t)self);
    FUN_8001fe50(DAT_80065a18, (void *)(uintptr_t)detached);
    FUN_80020890(self, 0x78);
    saloon_scale_radius(self);
}

uint32_t WW_SaloonDestruct(uint32_t *self, uint32_t mode, int *imp)
{
    switch (mode) {
    case 0: {
        if (self[0xf] != 0) return 0;

        int16_t n[3] = { 0, 0, 0 };
        uint32_t pos[3] = {
            self[9],
            (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x15]),
            self[0xb]
        };
        int gy = Terrain_QueryAt(self, pos, n, 0);
        if (gy < mips_addu_i32((int32_t)pos[1], 0x800)) {
            int32_t oldVx = (int32_t)self[0x20];
            int32_t vdot = mips_addu_i32(
                mips_addu_i32(mips_mult_lo_i32((int32_t)self[0x20], n[0]),
                              mips_mult_lo_i32((int32_t)self[0x21], n[1])),
                mips_mult_lo_i32((int32_t)self[0x22], n[2]));
            vdot = rtz_shift_i32(vdot, 11, 0x7ff);
            int32_t rollSeed = mips_sll_i32(n[0], 1);
            if (vdot < 0) {
                int32_t ax = rtz_shift_i32(mips_mult_lo_i32(vdot, n[0]), 12, 0xfff);
                int32_t ay = rtz_shift_i32(mips_mult_lo_i32(vdot, n[1]), 12, 0xfff);
                int32_t az = rtz_shift_i32(mips_mult_lo_i32(vdot, n[2]), 12, 0xfff);
                self[0x20] = (uint32_t)mips_subu_i32((int32_t)self[0x20], ax);
                int32_t vy = mips_mult_lo_i32(mips_subu_i32((int32_t)self[0x21], ay), 3);
                self[0x21] = (uint32_t)rtz_shift_i32(vy, 2, 3);
                self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], az);
                self[10] = (uint32_t)mips_subu_i32(gy, (int32_t)self[0x15]);
                if (vdot < -0x1c9) {
                    Audio_PlaySfxRelative(*(uint32_t *)(uintptr_t)(self[0x16] + 8), 3, self + 9);
                    rollSeed = 0x66e;
                } else {
                    goto tick_damping;
                }
            }
            int32_t rollX = rtz_shift_i32(mips_mult_lo_i32(mips_addu_i32(rollSeed, n[0]), 0x1e), 12, 0xfff);
            self[0x20] = (uint32_t)mips_addu_i32(oldVx, rollX);
            int32_t fz = rtz_shift_i32(mips_mult_lo_i32(n[2], 0x5a), 12, 0xfff);
            self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22], fz);
tick_damping:
            *(int16_t *)(self + 0x23) =
                (int16_t)rtz_shift_i32(mips_mult_lo_i32(mips_subu_i32(0, (int32_t)self[0x22]),
                                                        (uint16_t)self[0x25]), 12, 0xfff);
            *(int16_t *)(self + 0x24) =
                (int16_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x20],
                                                        (uint16_t)self[0x25]), 12, 0xfff);
            self[0x20] = (uint32_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x20], 0xf), 4, 0xf);
            self[0x21] = (uint32_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x21], 0xf), 4, 0xf);
        }
        self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], 0x5a);
        self[0x20] = (uint32_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x20], 0x1f), 5, 0x1f);
        self[0x21] = (uint32_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x21], 0x1f), 5, 0x1f);
        self[0x22] = (uint32_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x22], 0x1f), 5, 0x1f);
        Object_OrientByAxis(self + 4, self + 4, self + 0x23);
        self[9] = (uint32_t)mips_addu_i32((int32_t)self[9], (int32_t)self[0x20]);
        self[10] = (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x21]);
        self[0xb] = (uint32_t)mips_addu_i32((int32_t)self[0xb], (int32_t)self[0x22]);
        self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22], -0x80);
        if ((mips_subu_i32((int32_t)_DAT_80065310, *((uint8_t *)self + 9)) & 0xf) == 0)
            MatrixNormal(self + 4, self + 4);
        if ((uint32_t)self[0xb] < _DAT_80065b3c) {
            Object_RefitAABB(self);
            self[0x20] = 0;
            self[0x21] = 0;
            self[0x22] = 0;
            *(int16_t *)(self + 0x23) = 0;
            *(int16_t *)((uint8_t *)self + 0x8e) = 0;
            *(int16_t *)(self + 0x24) = 0;
        }
        return 0;
    }
    case 1:
    case 6:
        saloon_scale_radius(self);
        return 0;
    case 2:
        saloon_detach_parented(self);
        return 0;
    case 3: {
        uint32_t impObj = (uint32_t)*imp;
        uint32_t *ringSelf = self;
        if (*(uint8_t *)(uintptr_t)(impObj + 4) == 2 && ((*self & 1) == 0)) {
            int32_t local[3] = {
                mips_subu_i32((int32_t)self[9], *(int32_t *)(uintptr_t)(impObj + 0x24)),
                mips_subu_i32((int32_t)self[10], *(int32_t *)(uintptr_t)(impObj + 0x28)),
                mips_subu_i32((int32_t)self[0xb], *(int32_t *)(uintptr_t)(impObj + 0x2c))
            };
            FUN_8004366c((uint32_t *)(uintptr_t)(impObj + 0x10));
            FUN_80043248(local, (int32_t *)(self + 9));
            FUN_8004cdc4(self + 4, self + 4);
            FUN_8001fe8c(DAT_80065a18, self);
            FUN_8001d4f0((uint32_t *)(uintptr_t)impObj, self);
            *(int32_t *)(uintptr_t)(impObj + 0xdc) =
                mips_sll_i32(*(int32_t *)(uintptr_t)(impObj + 0xdc), 2);
            int32_t r = (int32_t)FUN_80017160();
            int32_t delay = (int32_t)((mips_mult_lo_i32(r, 0x168) >> 15) + 0x168);
            FUN_80020890(self, delay);
            ringSelf = (uint32_t *)1;
        }
        FX_RingFlash_Init(ringSelf, imp);
        int32_t vdot = mips_addu_i32(
            mips_addu_i32(mips_mult_lo_i32((int32_t)self[0x20], *(int16_t *)(imp + 8)),
                          mips_mult_lo_i32((int32_t)self[0x21], *(int16_t *)((uint8_t *)imp + 0x22))),
            mips_mult_lo_i32((int32_t)self[0x22], *(int16_t *)(imp + 9)));
        vdot = rtz_shift_i32(vdot, 11, 0x7ff);
        if (vdot < 0) {
            int32_t nx = (int32_t)*(int16_t *)(imp + 8) + ((int32_t)FUN_80017160() & 0xff);
            int32_t bx = rtz_shift_i32(mips_mult_lo_i32(vdot, nx), 12, 0xfff);
            int32_t by = rtz_shift_i32(mips_mult_lo_i32(vdot, *(int16_t *)((uint8_t *)imp + 0x22)), 12, 0xfff);
            int32_t bz = rtz_shift_i32(mips_mult_lo_i32(vdot, *(int16_t *)(imp + 9)), 12, 0xfff);
            self[0x20] = (uint32_t)mips_subu_i32((int32_t)self[0x20], bx);
            self[0x21] = (uint32_t)mips_subu_i32((int32_t)self[0x21], by);
            self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], bz);
        }
        saloon_detach_parented(self);
        return 0;
    }
    default: return 0;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- WILDWEST.DLL FUN_8010178c  (from analysis/dll/WILDWEST/decomp/8010178c.c) --- */
// addr: 0x8010178c  name: FUN_8010178c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010178c(uint *param_1,undefined4 param_2,int *param_3)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int extraout_v1;
  int iVar6;
  uint *puVar7;
  uint uVar8;
  uint uVar9;
  uint local_40;
  int local_3c;
  uint local_38;
  short local_30;
  short local_2e;
  short local_2c;
  int local_20;
  int local_1c;
  int local_18;
  
  switch(param_2) {
  case 0:
    if (param_1[0xf] != 0) {
      return 0;
    }
    local_40 = param_1[9];
    local_3c = param_1[10] + param_1[0x15];
    local_38 = param_1[0xb];
    iVar4 = FUN_8001d748/*0x8001d748*/(param_1,&local_40,&local_30,0);
    if (iVar4 < local_3c + 0x800) {
      iVar6 = (int)local_30;
      uVar2 = param_1[0x20];
      iVar1 = uVar2 * iVar6 + param_1[0x21] * (int)local_2e + param_1[0x22] * (int)local_2c;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x7ff;
      }
      iVar1 = iVar1 >> 0xb;
      iVar5 = iVar6 << 1;
      if (iVar1 < 0) {
        iVar6 = iVar1 * iVar6;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        param_1[0x20] = uVar2 - (iVar6 >> 0xc);
        iVar6 = iVar1 * local_2e;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (param_1[0x21] - (iVar6 >> 0xc)) * 3;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 3;
        }
        param_1[0x21] = iVar6 >> 2;
        iVar6 = iVar1 * local_2c;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        param_1[0x22] = param_1[0x22] - (iVar6 >> 0xc);
        param_1[10] = iVar4 - param_1[0x15];
        if (iVar1 < -0x1c9) {
          iVar6 = 3;
          func_0x80044ac8(*(undefined4 *)(param_1[0x16] + 8),3,param_1 + 9);
          iVar5 = extraout_v1;
          goto LAB_80101958;
        }
      }
      else {
LAB_80101958:
        iVar4 = (iVar5 + iVar6) * 0x1e;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x20] = uVar2 + (iVar4 >> 0xc);
        iVar4 = local_2c * 0x5a;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x22] = param_1[0x22] + (iVar4 >> 0xc);
      }
      iVar4 = -param_1[0x22] * (uint)(ushort)param_1[0x25];
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      iVar6 = param_1[0x20] * (uint)(ushort)param_1[0x25];
      *(short *)(param_1 + 0x23) = (short)(iVar4 >> 0xc);
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      *(short *)(param_1 + 0x24) = (short)(iVar6 >> 0xc);
      iVar4 = param_1[0x20] * 0xf;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xf;
      }
      param_1[0x20] = iVar4 >> 4;
      iVar4 = param_1[0x21] * 0xf;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xf;
      }
      param_1[0x21] = iVar4 >> 4;
    }
    iVar4 = param_1[0x20] * 0x1f;
    param_1[0x21] = param_1[0x21] + 0x5a;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x1f;
    }
    param_1[0x20] = iVar4 >> 5;
    iVar4 = param_1[0x21] * 0x1f;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x1f;
    }
    param_1[0x21] = iVar4 >> 5;
    iVar4 = param_1[0x22] * 0x1f;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x1f;
    }
    param_1[0x22] = iVar4 >> 5;
    puVar7 = param_1 + 4;
    func_0x800172b4(puVar7,puVar7,param_1 + 0x23);
    param_3 = (int *)(param_1[0x22] - 0x80);
    uVar2 = _DAT_80065310 - (uint)*(byte *)((int)param_1 + 9);
    param_1[9] = param_1[9] + param_1[0x20];
    param_1[10] = param_1[10] + param_1[0x21];
    param_1[0xb] = param_1[0xb] + param_1[0x22];
    param_1[0x22] = (uint)param_3;
    if ((uVar2 & 0xf) == 0) {
      MatrixNormal/*0x8004c934*/(puVar7,puVar7);
    }
    if (_DAT_80065b3c <= param_1[0xb]) {
      return 0;
    }
    FUN_8001d708/*0x8001d708*/(param_1);
    param_1[0x20] = 0;
    param_1[0x21] = 0;
    param_1[0x22] = 0;
    *(undefined2 *)(param_1 + 0x23) = 0;
    *(undefined2 *)((int)param_1 + 0x8e) = 0;
    *(undefined2 *)(param_1 + 0x24) = 0;
  case 3:
    iVar4 = *param_3;
    puVar7 = param_1;
    if ((*(char *)(iVar4 + 4) == '\x02') && ((*param_1 & 1) == 0)) {
      local_20 = param_1[9] - *(int *)(iVar4 + 0x24);
      param_3 = &local_20;
      local_1c = param_1[10] - *(int *)(iVar4 + 0x28);
      local_18 = param_1[0xb] - *(int *)(iVar4 + 0x2c);
      GTE_LoadMatrixPacked/*0x8004366c*/(iVar4 + 0x10);
      GTE_RotateLong/*0x80043248*/(param_3,param_1 + 9);
      MulRotMatrix0/*0x8004cdc4*/(param_1 + 4,param_1 + 4);
      FUN_8001fe8c/*0x8001fe8c*/(0x80065a18,param_1);
      FUN_8001d4f0/*0x8001d4f0*/(iVar4,param_1);
      *(int *)(iVar4 + 0xdc) = *(int *)(iVar4 + 0xdc) << 2;
      iVar4 = FUN_80017160/*0x80017160*/();
      FUN_80020890/*0x80020890*/(param_1,(iVar4 * 0x168 >> 0xf) + 0x168);
      puVar7 = (uint *)0x1;
    }
    FUN_8001f5a0/*0x8001f5a0*/(puVar7,param_3);
    iVar4 = param_1[0x20] * (int)(short)param_3[8] +
            param_1[0x21] * (int)*(short *)((int)param_3 + 0x22) +
            param_1[0x22] * (int)(short)param_3[9];
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x7ff;
    }
    iVar4 = iVar4 >> 0xb;
    if (iVar4 < 0) {
      uVar2 = FUN_80017160/*0x80017160*/();
      iVar6 = iVar4 * ((int)(short)param_3[8] + (uVar2 & 0xff));
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x20] = param_1[0x20] - (iVar6 >> 0xc);
      iVar6 = iVar4 * *(short *)((int)param_3 + 0x22);
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x21] = param_1[0x21] - (iVar6 >> 0xc);
      iVar4 = iVar4 * (short)param_3[9];
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      param_1[0x22] = param_1[0x22] - (iVar4 >> 0xc);
    }
switchD_801017c4_caseD_2:
    if (param_1[0xf] != 0) {
      iVar4 = FUN_8001d5a0/*0x8001d5a0*/(param_1);
      iVar6 = *(int *)(iVar4 + 0xdc);
      if (iVar6 < 0) {
        iVar6 = iVar6 + 3;
      }
      *(int *)(iVar4 + 0xdc) = iVar6 >> 2;
      puVar7 = (uint *)FUN_8001d624/*0x8001d624*/(param_1);
      uVar2 = puVar7[1];
      uVar8 = puVar7[2];
      uVar9 = puVar7[3];
      param_1[4] = *puVar7;
      param_1[5] = uVar2;
      param_1[6] = uVar8;
      param_1[7] = uVar9;
      uVar2 = puVar7[5];
      uVar8 = puVar7[6];
      uVar9 = puVar7[7];
      param_1[8] = puVar7[4];
      param_1[9] = uVar2;
      param_1[10] = uVar8;
      param_1[0xb] = uVar9;
      uVar3 = FUN_8001d564/*0x8001d564*/(param_1);
      FUN_8001fe50/*0x8001fe50*/(0x80065a18,uVar3);
      FUN_80020890/*0x80020890*/(param_1,0x78);
switchD_801017c4_caseD_6:
      goto switchD_801017c4_caseD_1;
    }
    break;
  case 1:
switchD_801017c4_caseD_1:
    iVar4 = param_1[0x15] * 0x93c;
    *param_1 = *param_1 | 0x80;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_1[0x15] = iVar4 >> 0xc;
    iVar4 = (iVar4 >> 0xc) * 0x3243;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    *(short *)(param_1 + 0x25) = (short)(0x1000000 / (iVar4 >> 0xc));
  default:
    break;
  case 2:
    goto switchD_801017c4_caseD_2;
  case 6:
    goto switchD_801017c4_caseD_6;
  }
  return 0;
}

#endif  /* GHIDRA REF */
