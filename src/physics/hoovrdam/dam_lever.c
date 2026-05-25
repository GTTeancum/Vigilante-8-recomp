/* dam_lever.c -- Hoover Dam moving-platform integrator.
 *
 * Source: HOOVRDAM.DLL  FUN_801006cc.
 *
 * The Hoover Dam level has several moving objects (parking_meter
 * impacts, Dam_Lever, transformer doors). They share the same 1/128-
 * scaled position integrator as Sand Factory's conveyor (see
 * src/physics/sandfact/conveyor.c). The function:
 *
 *   1. Integrates position (obj[9..11]) by velocity (obj[0x20..22])
 *      divided by 128, with negative-correction +0x7f rounding.
 *   2. On external impulse (param_3 != 0), kicks a sound at obj+0x24
 *      via FUN_800446dc (likely SoundEmit_FromBlob) and applies a
 *      mix-down via FUN_80044574 keyed by obj[0xd3] (sound slot id).
 *   3. Advances the per-object frame counter at obj+8 by 1; the state
 *      machine continues into branches matching the counter value
 *      against the impulse param (state transitions deferred to
 *      pass 3).
 *
 * MED confidence on the integrator + sound emit; LOW on the state
 * machine branches (cases 1, 8 and the matched case = pass-3 work).
 */
#include <stdint.h>
#include <string.h>

extern uint32_t SoundEmit_FromBlob(void *posXyz);            /* FUN_800446dc */
extern void     SoundChannel_Apply(uint8_t slot, uint32_t pan); /* FUN_80044574 */
extern int      FUN_80021888(int pathId);
extern intptr_t FUN_8001b038(int obj, uint16_t kind);
extern void     FUN_8001d68c(void *out, int obj, intptr_t bone);
extern void     FUN_80020890(uint32_t *obj, int timer);
extern uint32_t SfxChannel_Acquire(void);
extern void     FUN_800447e8(uint32_t voice, uint32_t bank, int slot, uint32_t *xyz);
extern void     Audio_VoiceStop(int ch);
extern void     gap_80031294(uint32_t *self);
extern uintptr_t _DAT_800737e8;

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

uint32_t HD_DamLeverTick(uint32_t *obj, int mode, int impulse)
{
    int stateMatch = mode;

    if (mode != 2) {
        obj[9] = (uint32_t)mips_addu_i32((int32_t)obj[9],
            rtz_shift_i32((int32_t)obj[0x20], 7, 0x7f));
        obj[10] = (uint32_t)mips_addu_i32((int32_t)obj[10],
            rtz_shift_i32((int32_t)obj[0x21], 7, 0x7f));
        obj[11] = (uint32_t)mips_addu_i32((int32_t)obj[11],
            rtz_shift_i32((int32_t)obj[0x22], 7, 0x7f));

        if (impulse == 0) return 0;

        uint32_t pan = SoundEmit_FromBlob(obj + 9);
        SoundChannel_Apply(((uint8_t *)obj)[0xd3], pan);
        stateMatch = (int)pan;
    }

    {
        uint8_t oldCounter = ((uint8_t *)obj)[8];
        ((uint8_t *)obj)[8] = (uint8_t)(oldCounter + 1);
        int cur = (int)(int8_t)oldCounter;

        if (cur == 1)
            goto release;
        if (cur < 2) {
            if (cur != 0)
                goto compareState;
        } else {
compareState:
            if (cur == stateMatch)
                goto resetState;
            if (cur != 8)
                return 0;
        }

        int shift = 7;
        int source = FUN_80021888(((uint8_t *)obj)[0xd2]);
        if ((int8_t)((uint8_t *)obj)[8] == 1)
            shift = 8;
        intptr_t bone = FUN_8001b038(source, 0x8000);
        ((uint8_t *)obj)[8] = 1;

        uint8_t local[0x30];
        memset(local, 0, sizeof local);
        FUN_8001d68c(local, source, bone);
        memcpy((uint8_t *)obj + 0x10, local, 0x10);
        *(uint16_t *)((uint8_t *)obj + 0x20) = *(uint16_t *)(local + 0x10);

        obj[0] |= 2u;
        for (uint32_t node = obj[0xe]; node != 0; node = *(uint32_t *)(uintptr_t)(node + 0x34)) {
            uint32_t child = *(uint32_t *)(uintptr_t)(node + 0x30);
            *(uint16_t *)(uintptr_t)(child + 0x28) = 0;
        }
        if (obj[0x1a] != 0)
            *(uint16_t *)(uintptr_t)(obj[0x1a] + 0x28) = 0;

        obj[0x20] = (uint32_t)(mips_sll_i32(
            mips_subu_i32(*(int32_t *)(local + 0x14), (int32_t)obj[9]), 7) >> shift);
        obj[0x21] = (uint32_t)(mips_sll_i32(
            mips_subu_i32(*(int32_t *)(local + 0x18), (int32_t)obj[10]), 7) >> shift);
        obj[0x22] = (uint32_t)(mips_sll_i32(
            mips_subu_i32(*(int32_t *)(local + 0x1c), (int32_t)obj[11]), 7) >> shift);
        obj[0x24] = 0;
        obj[0x25] = 0;
        obj[0x26] = 0;
        FUN_80020890(obj, 1 << shift);
    }

release:
    FUN_800447e8((uint32_t)((uint8_t *)obj)[0xd3],
                 *(uint32_t *)(uintptr_t)(_DAT_800737e8 + 8), 5, obj + 9);
    ((uint8_t *)obj)[0xd3] = 0;
    obj[0] &= 0xfefffffdu;
    obj[0x20] = (uint32_t)rtz_shift_i32(
        mips_mult_lo_i32(*(int16_t *)(obj + 5), 0x23c3), 5, 0x1f);
    obj[0x21] = (uint32_t)rtz_shift_i32(
        mips_mult_lo_i32(*(int16_t *)((uint8_t *)obj + 0x1a), 0x23c3), 5, 0x1f);
    obj[0x22] = (uint32_t)rtz_shift_i32(
        mips_mult_lo_i32(*(int16_t *)(obj + 8), 0x23c3), 5, 0x1f);
    FUN_80020890(obj, 0x1e);

resetState:
    ((uint8_t *)obj)[8] = 0;
    obj[0] &= 0xffffffdfu;
    gap_80031294(obj);
    return 0;
}

uint32_t FUN_801006cc(uint32_t *obj, int mode, int impulse)
{
    return HD_DamLeverTick(obj, mode, impulse);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- HOOVRDAM.DLL FUN_801006cc  (from analysis/dll/HOOVRDAM/decomp/801006cc.c) --- */
// addr: 0x801006cc  name: FUN_801006cc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801006cc(uint *param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  undefined2 local_20;
  int local_1c;
  int local_18;
  int local_14;
  
  if ((param_2 == 0) || (param_2 != 2)) {
    uVar3 = param_1[0x20];
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    uVar5 = param_1[0x21];
    param_1[9] = param_1[9] + ((int)uVar3 >> 7);
    if ((int)uVar5 < 0) {
      uVar5 = uVar5 + 0x7f;
    }
    uVar3 = param_1[0x22];
    param_1[10] = param_1[10] + ((int)uVar5 >> 7);
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    param_1[0xb] = param_1[0xb] + ((int)uVar3 >> 7);
    if (param_3 == 0) {
      return 0;
    }
    param_2 = FUN_800446dc/*0x800446dc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/(*(undefined1 *)((int)param_1 + 0xd3));
  }
  uVar3 = param_1[2];
  *(char *)(param_1 + 2) = (char)uVar3 + '\x01';
  iVar4 = (int)(char)uVar3;
  if (iVar4 != 1) {
    if ((1 < iVar4) || (iVar4 != 0)) {
      if (iVar4 == param_2) goto LAB_801009e0;
      if (iVar4 != 8) {
        return 0;
      }
    }
    iVar4 = 7;
    uVar1 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)param_1 + 0xd2));
    if ((char)param_1[2] == '\x01') {
      iVar4 = 8;
    }
    *(undefined1 *)(param_1 + 2) = 1;
    uVar2 = FUN_8001b038/*0x8001b038*/(uVar1,0x8000);
    FUN_8001d68c/*0x8001d68c*/(&local_30,uVar1,uVar2);
    param_1[4] = local_30;
    param_1[5] = local_2c;
    param_1[6] = local_28;
    param_1[7] = local_24;
    *(undefined2 *)(param_1 + 8) = local_20;
    uVar3 = param_1[0xe];
    *param_1 = *param_1 | 2;
    for (; uVar3 != 0; uVar3 = *(uint *)(uVar3 + 0x34)) {
      *(undefined2 *)(*(int *)(uVar3 + 0x30) + 0x28) = 0;
    }
    if (param_1[0x1a] != 0) {
      *(undefined2 *)(param_1[0x1a] + 0x28) = 0;
    }
    param_1[0x20] = (int)((local_1c - param_1[9]) * 0x80) >> iVar4;
    param_1[0x21] = (int)((local_18 - param_1[10]) * 0x80) >> iVar4;
    param_1[0x22] = (int)((local_14 - param_1[0xb]) * 0x80) >> iVar4;
    param_1[0x24] = 0;
    param_1[0x25] = 0;
    param_1[0x26] = 0;
    FUN_80020890/*0x80020890*/(param_1,1 << iVar4);
  }
  FUN_800447e8/*0x800447e8*/(*(undefined1 *)((int)param_1 + 0xd3),*(undefined4 *)(_DAT_800737e8 + 8),5,
                  param_1 + 9);
  *(undefined1 *)((int)param_1 + 0xd3) = 0;
  iVar4 = (short)param_1[5] * 0x23c3;
  *param_1 = *param_1 & 0xfefffffd;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x20] = iVar4 >> 5;
  iVar4 = *(short *)((int)param_1 + 0x1a) * 0x23c3;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x21] = iVar4 >> 5;
  iVar4 = (short)param_1[8] * 0x23c3;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x22] = iVar4 >> 5;
  FUN_80020890/*0x80020890*/(param_1,0x1e);
LAB_801009e0:
  *(undefined1 *)(param_1 + 2) = 0;
  *param_1 = *param_1 & 0xffffffdf;
  func_0x80031294();
  return 0;
}

#endif  /* GHIDRA REF */
