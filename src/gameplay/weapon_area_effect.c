/* weapon_area_effect.c -- AABB intersection test + rotated-matrix callback.
 *
 * Source: SLUS_005.10  FUN_8002b610
 *
 * FUN_8002b610(param_1, param_2, param_3, param_4, param_5, param_6):
 *   param_1  int     source point (PSX address of xyz int32[3])
 *   param_2  int     radius (half-extent for AABB test)
 *   param_3  int     object handle for FUN_8001be5c callback
 *   param_4  int     rotation matrix (PSX address of packed MATRIX)
 *   param_5  uint    rotation angle index / scale factor
 *   param_6  int     draw-config arg for FUN_8001be5c callback
 *
 * Operation:
 *   1. Transform param_1 through matrix param_4 via GTE_RotateLongMatTrans,
 *      storing the 3-int result (X, Y, Z) into local MATRIX translation fields.
 *   2. Scale each component: component *= param_5; component >>= 8.
 *   3. AABB intersection test (XZ within 0xa0 radius, Y within 0x78 radius):
 *        (X - r) * 256 < Z * 0xa0   AND   Z * -0xa0 < (X + r) * 256
 *        (Y - r) * 256 < Z * 0x78   AND   Z * -0x78 < (Y + r) * 256
 *   4. If all 4 conditions pass, build a Y-axis rotation matrix from the
 *      sin/cos table (using param_5 as angle index, stride 0x10 entries):
 *        m[0] = { cos, -sin, 0 }
 *        m[1] = { sin,  cos, 0 }
 *        m[2] = {   0,    0, 0x1000 }
 *      with sin/cos = RTZ(table[idx] * param_2, 0xfffff) >> 20
 *   5. Call FUN_8001be5c(param_3, &local_mat, param_6) — renderer callback
 *      (OUT OF SCOPE; stub does nothing).
 *
 * Sin/cos table: DAT_800607b4[] interleaved {sin, cos} per entry.
 *   sin[(param_5 & 0x1ff) * 0x10]       = DAT_800607b4[(param_5 & 0x1ff) * 0x10]
 *   cos[(param_5 & 0x1ff) * 0x10 + 1]  = DAT_800607b4[(param_5 & 0x1ff) * 0x10 + 1]
 *
 * PSX RTZ pattern for >> n: if (val < 0) val += (1 << n) - 1; val >>= n.
 *   0xfffff = (1 << 20) - 1 for >> 20.
 *
 * HIGH confidence: direct Ghidra ref port with MIPS-verified MATRIX layout.
 */
#include <stdint.h>
#include "structs.h"

extern int16_t DAT_800607b4[];                           /* sin/cos table (panic_stubs.c)     */
extern void FUN_80043408(const uint32_t *m,
                          const int32_t *v,
                          int32_t       *out);           /* GTE_RotateLongMatTrans (gte_long_vec.c) */
extern void FUN_8001be5c(int param_1,
                          void *param_2,
                          int   param_3);               /* Renderer callback (skipped)         */

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

static inline int32_t rtz_shift_i32(int32_t v, unsigned sh, int32_t bias)
{
    if (v < 0) v = mips_addu_i32(v, bias);
    return v >> sh;
}

/* ================================================================
 * FUN_8002b610  -- WeaponAreaEffect_Apply
 *
 * HIGH confidence: verified instruction-for-instruction against MIPS.
 * MATRIX struct layout matches PSX stack frame exactly:
 *   m[3][3] at sp+0x10 (local_30 .. local_20)
 *   pad      at sp+0x22 (undefined)
 *   t[3]     at sp+0x24 (local_1c, local_18, local_14) ← FUN_80043408 output
 * ================================================================ */
void FUN_8002b610(int param_1, int param_2, int param_3,
                  int param_4, uint32_t param_5, int param_6)
{
    int    iVar1;
    MATRIX local_mat;   /* PSX MATRIX: m[3][3] + pad (2 B) + t[3] = 32 bytes  */

    /* Step 1: transform source point through rotation matrix → local_mat.t[]. */
    FUN_80043408((const uint32_t *)(uintptr_t)param_4,
                 (const int32_t  *)(uintptr_t)param_1,
                 local_mat.t);

    /* Step 2: scale each component by param_5 / 256. */
    local_mat.t[0] = mips_mult_lo_i32(local_mat.t[0], (int32_t)param_5) >> 8;
    local_mat.t[1] = mips_mult_lo_i32(local_mat.t[1], (int32_t)param_5) >> 8;
    local_mat.t[2] = mips_mult_lo_i32(local_mat.t[2], (int32_t)param_5) >> 8;

    /* Step 3: AABB intersection check (XZ × 0xa0, Y × 0x78). */
    int32_t xMinus = mips_mult_lo_i32(mips_subu_i32(local_mat.t[0], param_2), 0x100);
    int32_t xPlus  = mips_mult_lo_i32(mips_addu_i32(local_mat.t[0], param_2), 0x100);
    int32_t yMinus = mips_mult_lo_i32(mips_subu_i32(local_mat.t[1], param_2), 0x100);
    int32_t yPlus  = mips_mult_lo_i32(mips_addu_i32(local_mat.t[1], param_2), 0x100);
    int32_t zA0    = mips_mult_lo_i32(local_mat.t[2], 0xa0);
    int32_t zNegA0 = mips_mult_lo_i32(local_mat.t[2], -0xa0);
    int32_t z78    = mips_mult_lo_i32(local_mat.t[2], 0x78);
    int32_t zNeg78 = mips_mult_lo_i32(local_mat.t[2], -0x78);
    if ((((xMinus < zA0) && (zNegA0 < xPlus)) &&
         (yMinus < z78)) &&
        (zNeg78 < yPlus)) {

        /* Step 4a: compute cos = RTZ(table_cos * radius, 0xfffff) >> 20. */
        iVar1 = mips_mult_lo_i32((int16_t)DAT_800607b4[(param_5 & 0x1ff) * 0x10 + 1],
                                 param_2);
        /* local_30 = cos value (equivalent to local_mat.m[0][0] / m[1][1]) */
        int16_t cos_val = (int16_t)rtz_shift_i32(iVar1, 20, 0xfffff);

        /* Step 4b: compute sin = RTZ(table_sin * radius, 0xfffff) >> 20.
         * Ghidra reuses param_2 as the intermediate. */
        param_2 = mips_mult_lo_i32((int16_t)DAT_800607b4[(param_5 & 0x1ff) * 0x10],
                                   param_2);
        int16_t sin_val = (int16_t)rtz_shift_i32(param_2, 20, 0xfffff);

        /* Step 4c: assemble Y-axis rotation matrix. */
        local_mat.m[0][0] =  cos_val;   /* local_30 */
        local_mat.m[0][1] = (int16_t)mips_subu_i32(0, sin_val);   /* local_2e */
        local_mat.m[0][2] =  0;         /* local_2c */
        local_mat.m[1][0] =  sin_val;   /* local_2a */
        local_mat.m[1][1] =  cos_val;   /* local_28 = local_30 */
        local_mat.m[1][2] =  0;         /* local_26 */
        local_mat.m[2][0] =  0;         /* local_24 */
        local_mat.m[2][1] =  0;         /* local_22 */
        local_mat.m[2][2] =  0x1000;    /* local_20 */

        /* Step 5: invoke renderer callback with the assembled matrix. */
        FUN_8001be5c(param_3, &local_mat, param_6);
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8002b610  (from analysis/SLUS_005.10/decomp/8002b610.c) --- */
// addr: 0x8002b610  name: FUN_8002b610

void FUN_8002b610(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,uint param_5,
                 undefined4 param_6)

{
  int iVar1;
  short local_30;
  short local_2e;
  undefined2 local_2c;
  short local_2a;
  short local_28;
  undefined2 local_26;
  undefined2 local_24;
  undefined2 local_22;
  undefined2 local_20;
  int local_1c;
  int local_18;
  int local_14;

  FUN_80043408(param_4,param_1,&local_1c);
  local_1c = (int)(local_1c * param_5) >> 8;
  local_18 = (int)(local_18 * param_5) >> 8;
  local_14 = (int)(local_14 * param_5) >> 8;
  if (((((local_1c - param_2) * 0x100 < local_14 * 0xa0) &&
       (local_14 * -0xa0 < (local_1c + param_2) * 0x100)) &&
      ((local_18 - param_2) * 0x100 < local_14 * 0x78)) &&
     (local_14 * -0x78 < (local_18 + param_2) * 0x100)) {
    iVar1 = (short)(&DAT_800607b6)[(param_5 & 0x1ff) * 0x10] * param_2;
    local_30 = (short)((uint)iVar1 >> 0x10);
    if (iVar1 < 0) {
      local_30 = (short)((uint)(iVar1 + 0xfffff) >> 0x10);
    }
    local_30 = local_30 >> 4;
    param_2 = (short)(&DAT_800607b4)[(param_5 & 0x1ff) * 0x10] * param_2;
    if (param_2 < 0) {
      param_2 = param_2 + 0xfffff;
    }
    local_2a = (short)(param_2 >> 0x14);
    local_2e = -local_2a;
    local_20 = 0x1000;
    local_22 = 0;
    local_24 = 0;
    local_26 = 0;
    local_2c = 0;
    local_28 = local_30;
    FUN_8001be5c(param_3,&local_30,param_6);
  }
  return;
}

#endif  /* GHIDRA REF */
