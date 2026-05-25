/* windmill.c -- Valley Farms windmill spin tick.
 *
 * Source: VALLYFRM.DLL  FUN_801005e8.
 *
 * Windmill_1 spinning/falling callback. Non-mode-2/non-mode-3 events
 * spin and steer the windmill. Mode 2 retires after hitting terrain.
 * Mode 3 handles collision impact, impulse transfer, SFX, and delayed
 * state transition.
 *
 * MED confidence.
 */
#include <stdint.h>

extern int FUN_800170c8(uint32_t lo, int hi);
extern void Object_RefitAABB(uint32_t *obj);              /* FUN_8001d708 */
extern int Terrain_HeightAt(uint32_t x, uint32_t z);
extern uint32_t *FUN_8003fd24(uint32_t *parent, uint16_t kind);
extern void FUN_8004483c(uint32_t voice, uint32_t bank, int kind, uint32_t *xyz);
extern void FUN_800205f8(uint32_t *obj);
extern void Audio_VoiceStop(int ch);
extern void FUN_800176f8(int obj, int32_t *vec, int32_t *pos);
extern uint32_t SfxChannel_Acquire(void);
extern uint32_t *FUN_8003fea8(uint32_t *xyz, uint32_t color);
extern void FUN_80020890(uint32_t *obj, int timer);
extern uint32_t _DAT_800658fc;

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
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int32_t rtz_shift_i32(int32_t v, unsigned sh, int32_t bias)
{
    if (v < 0) v = mips_addu_i32(v, bias);
    return v >> sh;
}

uint32_t FUN_801005e8(uint32_t *obj, uint32_t mode, uint32_t *arg)
{
    if (mode == 2) {
        uint32_t *pos = obj + 9;
        FUN_8003fd24(pos, 0x11);
        FUN_8004483c((uint32_t)(int)*(int8_t *)((uint8_t *)obj + 5),
                     _DAT_800658fc, 0xc, pos);
        FUN_800205f8(obj);
        return 0;
    }

    if (mode == 3)
        goto impact;

    *(int16_t *)(obj + 0x11) =
        (int16_t)mips_addu_i32(*(int16_t *)(obj + 0x11), 0x88);

    if ((obj[0] & 1u) == 0) {
        obj[0x12] = (uint32_t)mips_addu_i32((int32_t)obj[0x12], (int32_t)obj[0x22]);
        obj[0x13] = (uint32_t)mips_addu_i32((int32_t)obj[0x13], (int32_t)obj[0x23]);
        obj[0x14] = (uint32_t)mips_addu_i32((int32_t)obj[0x14], (int32_t)obj[0x24]);
        obj[0x23] = (uint32_t)mips_addu_i32((int32_t)obj[0x23], 0x38);

        uint32_t target = obj[0x21];
        if (target != 0) {
            int32_t delta[3];
            delta[0] = mips_subu_i32(*(int32_t *)(uintptr_t)(target + 0x48),
                                     (int32_t)obj[0x12]);
            delta[1] = mips_subu_i32(*(int32_t *)(uintptr_t)(target + 0x4c),
                                     (int32_t)obj[0x13]);
            delta[2] = mips_subu_i32(*(int32_t *)(uintptr_t)(target + 0x50),
                                     (int32_t)obj[0x14]);
            int64_t xz = (int64_t)delta[0] * delta[0] + (int64_t)delta[2] * delta[2];
            (void)FUN_800170c8((uint32_t)xz, (int)(uint64_t)((uint64_t)xz >> 32));
            int64_t yterm = (int64_t)(int32_t)obj[0x23] * (int32_t)obj[0x23] +
                            (int64_t)delta[1] * 0x70;
            int32_t denom = FUN_800170c8((uint32_t)yterm,
                                         (int)(uint64_t)((uint64_t)yterm >> 32));
            denom = mips_subu_i32(denom, (int32_t)obj[0x23]);
            if (denom != 0) {
                int32_t vx = mips_subu_i32((mips_mult_lo_i32(delta[0], 0x38) / denom),
                                           (int32_t)obj[0x22]);
                obj[0x22] = (uint32_t)mips_addu_i32((int32_t)obj[0x22],
                                                    rtz_shift_i32(vx, 4, 0xf));
                int32_t vz = mips_subu_i32((mips_mult_lo_i32(delta[2], 0x38) / denom),
                                           (int32_t)obj[0x24]);
                obj[0x24] = (uint32_t)mips_addu_i32((int32_t)obj[0x24],
                                                    rtz_shift_i32(vz, 4, 0xf));
            }
        }
    }

    Object_RefitAABB(obj);
    if ((int32_t)obj[10] < Terrain_HeightAt(obj[9], obj[11]))
        return 0;

    {
        uint32_t *pos = obj + 9;
        FUN_8003fd24(pos, 0x11);
        FUN_8004483c((uint32_t)(int)*(int8_t *)((uint8_t *)obj + 5),
                     _DAT_800658fc, 0xc, pos);
        FUN_800205f8(obj);
        return 0;
    }

impact:
    if (*(uint8_t *)(uintptr_t)(*arg + 4) == 1)
        return 0;

    Audio_VoiceStop((int)*(int8_t *)((uint8_t *)obj + 5));
    uint32_t impactObj = *arg;
    if (*(uint8_t *)(uintptr_t)(impactObj + 4) == 2) {
        uint32_t mass = *(uint16_t *)(uintptr_t)(impactObj + 0xa2);
        if (mass != 0) {
            uint32_t scale = ((uint32_t)*(uint16_t *)(obj + 3) << 11) / mass;
            int32_t impulse[3];
            impulse[0] = mips_mult_lo_i32((int32_t)obj[0x22], (int32_t)scale);
            impulse[1] = mips_mult_lo_i32((int32_t)obj[0x23], (int32_t)scale);
            impulse[2] = mips_mult_lo_i32((int32_t)obj[0x24], (int32_t)scale);
            FUN_800176f8((int)(uintptr_t)impactObj, impulse, (int32_t *)(obj + 9));
        }
    }
    uint32_t voice = SfxChannel_Acquire();
    FUN_8004483c(voice, *(uint32_t *)(uintptr_t)(obj[0x16] + 8), 0, obj + 9);
    FUN_8003fea8(obj + 9, 0x08000040u);
    obj[0] |= 0x20u;
    FUN_80020890(obj, 0x1e);
    return 0;
}

uint32_t VF_WindmillTick(uint32_t *obj, int mode)
{
    return FUN_801005e8(obj, (uint32_t)mode, 0);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- VALLYFRM.DLL FUN_801005e8  (from analysis/dll/VALLYFRM/decomp/801005e8.c) --- */
// addr: 0x801005e8  name: FUN_801005e8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801005e8(uint *param_1,uint param_2,uint *param_3)

{
  longlong lVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  int local_28;
  int local_24;
  int local_20;
  longlong local_18;
  
  if (param_2 != 2) {
    uVar2 = 3;
    if (((2 < param_2) || (uVar2 = 0, param_2 != 0)) && (param_2 == uVar2)) goto LAB_80100868;
    *(short *)(param_1 + 0x11) = (short)param_1[0x11] + 0x88;
    if ((*param_1 & 1) == 0) {
      param_1[0x12] = param_1[0x12] + param_1[0x22];
      param_1[0x13] = param_1[0x13] + param_1[0x23];
      param_1[0x14] = param_1[0x14] + param_1[0x24];
      uVar2 = param_1[0x21];
      param_1[0x23] = param_1[0x23] + 0x38;
      if (uVar2 != 0) {
        local_28 = *(int *)(uVar2 + 0x48) - param_1[0x12];
        local_24 = *(int *)(uVar2 + 0x4c) - param_1[0x13];
        local_20 = *(int *)(uVar2 + 0x50) - param_1[0x14];
        local_18 = (longlong)local_20 * (longlong)local_20;
        lVar1 = (longlong)local_28 * (longlong)local_28 + local_18;
        FUN_800170c8/*0x800170c8*/((int)lVar1,(int)((ulonglong)lVar1 >> 0x20));
        local_18 = (longlong)(int)param_1[0x23] * (longlong)(int)param_1[0x23];
        lVar1 = local_18 + local_24 * 0x70;
        iVar3 = FUN_800170c8/*0x800170c8*/((int)lVar1,(int)((ulonglong)lVar1 >> 0x20));
        iVar3 = iVar3 - param_1[0x23];
        if (iVar3 != 0) {
          iVar4 = (local_28 * 0x38) / iVar3 - param_1[0x22];
          if (iVar4 < 0) {
            iVar4 = iVar4 + 0xf;
          }
          param_1[0x22] = param_1[0x22] + (iVar4 >> 4);
          iVar3 = (local_20 * 0x38) / iVar3 - param_1[0x24];
          if (iVar3 < 0) {
            iVar3 = iVar3 + 0xf;
          }
          param_1[0x24] = param_1[0x24] + (iVar3 >> 4);
        }
      }
    }
    FUN_8001d708/*0x8001d708*/(param_1);
    iVar3 = Terrain_HeightAt/*0x80025400*/(param_1[9],param_1[0xb]);
    if ((int)param_1[10] < iVar3) {
      return 0;
    }
  }
  param_3 = param_1 + 9;
  FUN_8003fd24/*0x8003fd24*/(param_3,0x11);
  FUN_8004483c/*0x8004483c*/((int)*(char *)((int)param_1 + 5),_DAT_800658fc,0xc,param_3);
  FUN_800205f8/*0x800205f8*/(param_1);
LAB_80100868:
  if (*(char *)(*param_3 + 4) != '\x01') {
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    uVar2 = *param_3;
    if (*(char *)(uVar2 + 4) == '\x02') {
      uVar6 = ((uint)(ushort)param_1[3] << 0xb) / (uint)*(ushort *)(uVar2 + 0xa2);
      local_28 = param_1[0x22] * uVar6;
      local_24 = param_1[0x23] * uVar6;
      local_20 = param_1[0x24] * uVar6;
      FUN_800176f8/*0x800176f8*/(uVar2,&local_28,param_1 + 9);
    }
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),0,param_1 + 9);
    FUN_8003fea8/*0x8003fea8*/(param_1 + 9,0x8000040);
    *param_1 = *param_1 | 0x20;
    FUN_80020890/*0x80020890*/(param_1,0x1e);
  }
  return 0;
}

#endif  /* GHIDRA REF */
