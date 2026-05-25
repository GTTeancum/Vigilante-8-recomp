/* ice_chunk.c -- Ski Resort falling-ice chunk physics.
 *
 * Source: SKIRESRT.DLL  FUN_80101a94.
 *
 * Variant of the boulder-roll idiom for icicle chunks that detach
 * from the slope ceiling. Same Terrain_QueryAt+bounce flow with the
 * ski layout, but the spin axis is held constant (icicles spin
 * around their long axis rather than aligned to motion).
 *
 * mode: 0=tick, 3=impact (forwards to Damage_StandardVehicle).
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_RefitAABB(uint32_t *self);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern void Damage_StandardVehicle(uint32_t *self, uint32_t *imp);
extern void Object_RetireDeferred(uint32_t *self);
extern uint32_t *FUN_8003fd24(uint32_t *parent, uint16_t kind);
extern uint32_t FUN_8004410c(void);
extern int FUN_8004483c(uint32_t voice, uint32_t bank, int clip, const void *xyz);
extern void FUN_800176f8(int obj, int32_t *vec, int32_t *pos);
extern int FUN_8002c958(uint32_t *self, int impulse, const int32_t *vec, int flag);
extern uint32_t _DAT_80065310;

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint64_t)(uint32_t)a * (uint64_t)(uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int32_t rtz_shift_i32(int32_t v, unsigned sh, int32_t bias)
{
    if (v < 0) v = mips_addu_i32(v, bias);
    return v >> sh;
}

static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    if (hi < v) v = hi;
    if (lo < v) return v;
    return lo;
}

uint32_t SK_IceChunk(uint32_t *self, int mode, uint32_t *imp)
{
    if (mode == 3) goto damage;

    int16_t  n[3] = { 0, 0, 0 };
    uint32_t pos[3] = {
        self[9],
        (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x15]),
        self[0xb]
    };
    int gy = Terrain_QueryAt(self, pos, n, 0);
    if (gy < (int)pos[1] + 0x800) {
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
            self[0x21] = (uint32_t)mips_subu_i32((int32_t)self[0x21], ay);
            self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], az);
            self[0x21] = (uint32_t)rtz_shift_i32((int32_t)self[0x21], 2, 3);
            self[10] = (uint32_t)mips_subu_i32(gy, (int32_t)self[0x15]);
            if (vdot < 0x1c9) {
                FUN_8003fd24(pos, 0x11);
                uint32_t voice = FUN_8004410c();
                FUN_8004483c(voice, *(uint32_t *)(uintptr_t)(self[0x16] + 8), 4, self + 9);
                rollSeed = 0x72e;
            } else {
                goto spin_update;
            }
        }
        int32_t rollX = rtz_shift_i32(mips_mult_lo_i32(mips_addu_i32(rollSeed, n[0]), 0x1e), 12, 0xfff);
        self[0x20] = (uint32_t)mips_addu_i32(oldVx, rollX);
        int32_t fz = rtz_shift_i32(mips_mult_lo_i32(n[2], 0x5a), 12, 0xfff);
        self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22], fz);
spin_update:
        *(int16_t *)(self + 0x23) =
            (int16_t)rtz_shift_i32(mips_mult_lo_i32(mips_subu_i32(0, (int32_t)self[0x22]),
                                                    (uint16_t)self[0x25]), 12, 0xfff);
        *(int16_t *)(self + 0x24) =
            (int16_t)rtz_shift_i32(mips_mult_lo_i32((int32_t)self[0x20], (uint16_t)self[0x25]), 12, 0xfff);
        if ((int32_t)self[0xb] < 0x3b60000) {
            FUN_8003fd24(self + 9, 0x11);
            Object_RetireDeferred(self);
            return 0;
        }
    } else {
        self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], 0x5a);
    }
    Object_OrientByAxis(self + 4, self + 4, self + 0x23);
    self[9]  = (uint32_t)mips_addu_i32((int32_t)self[9], (int32_t)self[0x20]);
    self[10] = (uint32_t)mips_addu_i32((int32_t)self[10], (int32_t)self[0x21]);
    self[0xb]= (uint32_t)mips_addu_i32((int32_t)self[0xb], (int32_t)self[0x22]);
    if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xf) == 0)
        MatrixNormal(self + 4, self + 4);
    return 0;
damage:
    {
        uint32_t impObj = *imp;
        if (*(uint8_t *)(uintptr_t)(impObj + 4) == 2) {
            uint32_t scale = 0x32000u / *(uint16_t *)(uintptr_t)(impObj + 0xa2);
            int32_t vec[3];
            vec[0] = clamp_i32(mips_mult_lo_i32((int32_t)self[0x20], (int32_t)scale), -0x100000, 0x100000);
            vec[1] = clamp_i32(mips_mult_lo_i32((int32_t)self[0x21], (int32_t)scale), -0x100000, 0x100000);
            vec[2] = clamp_i32(mips_mult_lo_i32((int32_t)self[0x22], (int32_t)scale), -0x100000, 0x100000);
            FUN_800176f8((int)(uintptr_t)impObj, vec, (int32_t *)(self + 9));
            if ((*self & 0x10000) == 0) {
                static const int32_t iceImpulse[3] = { 0, 0, 0 };
                *self |= 0x10000u;
                FUN_8002c958((uint32_t *)(uintptr_t)impObj, -0xfa, iceImpulse, 1);
            }
        }
    }
    return 0;
}

uint32_t FUN_80101a94(uint32_t *self, int mode, int *imp)
{
    return SK_IceChunk(self, mode, (uint32_t *)imp);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SKIRESRT.DLL FUN_80101a94  (from analysis/dll/SKIRESRT/decomp/80101a94.c) --- */
// addr: 0x80101a94  name: FUN_80101a94

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101a94(uint *param_1,int param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int extraout_v1;
  uint uVar5;
  int iVar6;
  uint *puVar7;
  uint local_40;
  int local_3c;
  uint local_38;
  short local_30;
  short local_2e;
  short local_2c;
  int local_20;
  int local_1c;
  int local_18;
  
  if ((param_2 != 0) && (param_2 == 3)) goto LAB_80101dc8;
  local_40 = param_1[9];
  local_3c = param_1[10] + param_1[0x15];
  local_38 = param_1[0xb];
  iVar1 = FUN_8001d748/*0x8001d748*/(param_1,&local_40,&local_30,0);
  if (iVar1 < local_3c + 0x800) {
    iVar6 = (int)local_30;
    puVar7 = (uint *)param_1[0x20];
    iVar2 = (int)puVar7 * iVar6 + param_1[0x21] * (int)local_2e + param_1[0x22] * (int)local_2c;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0x7ff;
    }
    iVar2 = iVar2 >> 0xb;
    iVar4 = iVar6 << 1;
    if (iVar2 < 0) {
      iVar6 = iVar2 * iVar6;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x20] = (int)puVar7 - (iVar6 >> 0xc);
      iVar6 = iVar2 * local_2e;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x21] = param_1[0x21] - (iVar6 >> 0xc);
      iVar6 = iVar2 * local_2c;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      uVar5 = param_1[0x21];
      param_1[0x22] = param_1[0x22] - (iVar6 >> 0xc);
      if ((int)uVar5 < 0) {
        uVar5 = uVar5 + 3;
      }
      param_1[0x21] = (int)uVar5 >> 2;
      param_1[10] = iVar1 - param_1[0x15];
      if (iVar2 < 0x1c9) {
        FUN_8003fd24/*0x8003fd24*/(&local_40,0x11);
        uVar3 = FUN_8004410c/*0x8004410c*/();
        iVar6 = *(int *)(param_1[0x16] + 8);
        puVar7 = param_1 + 9;
        FUN_8004483c/*0x8004483c*/(uVar3,iVar6,4);
        iVar4 = extraout_v1;
        goto LAB_80101c58;
      }
    }
    else {
LAB_80101c58:
      iVar1 = (iVar4 + iVar6) * 0x1e;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      param_1[0x20] = (int)puVar7 + (iVar1 >> 0xc);
      iVar1 = local_2c * 0x5a;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      param_1[0x22] = param_1[0x22] + (iVar1 >> 0xc);
    }
    iVar1 = -param_1[0x22] * (uint)(ushort)param_1[0x25];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    iVar6 = param_1[0x20] * (uint)(ushort)param_1[0x25];
    *(short *)(param_1 + 0x23) = (short)(iVar1 >> 0xc);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    *(short *)(param_1 + 0x24) = (short)(iVar6 >> 0xc);
    if ((int)param_1[0xb] < 0x3b60000) {
      FUN_8003fd24/*0x8003fd24*/(param_1 + 9,0x11);
      FUN_800205f8/*0x800205f8*/(param_1);
      _DAT_00000095 = *(int *)(_DAT_800659fc + 0x94) + -1;
      goto LAB_80101d48;
    }
  }
  else {
LAB_80101d48:
    param_1[0x21] = param_1[0x21] + 0x5a;
  }
  puVar7 = param_1 + 4;
  func_0x800172b4(puVar7,puVar7,param_1 + 0x23);
  param_3 = (int *)param_1[0x20];
  uVar5 = _DAT_80065310 - (uint)*(byte *)((int)param_1 + 9);
  param_1[9] = param_1[9] + (int)param_3;
  param_1[10] = param_1[10] + param_1[0x21];
  param_1[0xb] = param_1[0xb] + param_1[0x22];
  if ((uVar5 & 0xf) != 0) {
    return 0;
  }
  MatrixNormal/*0x8004c934*/(puVar7,puVar7);
LAB_80101dc8:
  iVar1 = *param_3;
  if (*(char *)(iVar1 + 4) == '\x02') {
    uVar5 = 0x32000 / *(ushort *)(iVar1 + 0xa2);
    iVar6 = 0x100000;
    if ((int)(param_1[0x20] * uVar5) < 0x100000) {
      iVar6 = param_1[0x20] * uVar5;
    }
    local_20 = -0x100000;
    if (-0x100000 < iVar6) {
      local_20 = iVar6;
    }
    iVar6 = 0x100000;
    if ((int)(param_1[0x21] * uVar5) < 0x100000) {
      iVar6 = param_1[0x21] * uVar5;
    }
    local_1c = -0x100000;
    if (-0x100000 < iVar6) {
      local_1c = iVar6;
    }
    iVar6 = 0x100000;
    if ((int)(param_1[0x22] * uVar5) < 0x100000) {
      iVar6 = param_1[0x22] * uVar5;
    }
    local_18 = -0x100000;
    if (-0x100000 < iVar6) {
      local_18 = iVar6;
    }
    FUN_800176f8/*0x800176f8*/(iVar1,&local_20,param_1 + 9);
    if ((*param_1 & 0x10000) == 0) {
      *param_1 = *param_1 | 0x10000;
      FUN_8002c958/*0x8002c958*/(iVar1,0xffffff06,&DAT_80100148,1);
    }
  }
  return 0;
}

#endif  /* GHIDRA REF */
