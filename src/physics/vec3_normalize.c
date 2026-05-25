/* vec3_normalize.c -- 3-D fixed-point vector normalize and delta helpers.
 *
 * Source: SLUS_005.10
 *   FUN_80016b08  -- Vec3_NormalizeToShort(int32_t *vec, int16_t *out)
 *   FUN_80016bd8  -- Vec3_Delta(int16_t *out, int32_t *from, int32_t *to)
 *
 * FUN_80016b08:
 *   Normalises the integer 3-vector *vec and writes a Q12 unit-length
 *   short[3] into *out.  Uses GTE LZCS/LZCR to measure the shift needed
 *   to bring the magnitude into a safe range before dividing.
 *   Called by the sphere-intersection test (sphere_test.c) and Vec3_Delta.
 *
 * FUN_80016bd8:
 *   Computes the direction unit vector from `from` to `to` and stores
 *   the Q12 int16[3] result into `out`.  Internally subtracts the two
 *   int32 positions, then calls FUN_80016b08.
 *
 * HIGH confidence: direct Ghidra port; GTE LZCR idiom is canonical PSX
 * normalise-by-leading-zeros pattern.
 */
#include <stdint.h>
#include "structs.h"

extern int32_t Vec3_Length(const int32_t *v);   /* FUN_80016a20 -- computes magnitude */

/* GTE leading-zero primitives (from gte.h / libgte).
 * LZCS loads a word into the GTE LZC register.
 * LZCR reads back the count of leading zeros. */
extern void gte_ldLZCS(int val);
extern int  gte_stLZCR(void);

/* FUN_80016b08 -- Vec3_NormalizeToShort
 *
 * param_1: source int32_t[3] vector (signed fixed-point)
 * param_2: destination int16_t[3] (Q12 unit-vector output)
 * Returns: param_2.
 *
 * Algorithm: measure magnitude via Vec3_Length; if zero, write zeros.
 * Otherwise use GTE LZCS/LZCR to determine how many extra bits we can
 * shift param_1 left before dividing, giving maximum precision.
 * Clamp shift to 0..12 (Q12 output range).
 */
int16_t *FUN_80016b08(int32_t *param_1, int16_t *param_2)
{
    int32_t iVar1;
    int32_t iVar2;
    uint32_t uVar3;

    iVar1 = Vec3_Length(param_1);
    if (iVar1 == 0) {
        param_2[2] = 0;
        param_2[1] = 0;
        param_2[0] = 0;
    } else {
        gte_ldLZCS(iVar1);
        iVar2 = gte_stLZCR();
        uVar3 = 0xc;
        if ((int32_t)(iVar2 - 1u) < 0xc) {
            uVar3 = (uint32_t)(iVar2 - 1);
        }
        iVar1 = iVar1 >> ((0xc - uVar3) & 0x1f);
        param_2[0] = (int16_t)((param_1[0] << (uVar3 & 0x1f)) / iVar1);
        param_2[1] = (int16_t)((param_1[1] << (uVar3 & 0x1f)) / iVar1);
        param_2[2] = (int16_t)((param_1[2] << (uVar3 & 0x1f)) / iVar1);
    }
    return param_2;
}

/* PSX name aliases */
int16_t *Vec3_Project(int32_t *v, int16_t *out) { return FUN_80016b08(v, out); }

/* FUN_80016bd8 -- Vec3_Delta
 *
 * Computes the normalised direction unit vector from `param_2` to `param_3`
 * and stores the Q12 int16[3] result into `param_1`.
 *
 * param_1: undefined4 (int16_t *) -- output unit-direction int16[3]
 * param_2: int32_t[3] -- start position
 * param_3: int32_t[3] -- end position
 */
void FUN_80016bd8(int16_t *param_1, int32_t *param_2, int32_t *param_3)
{
    int32_t local_18;
    int32_t local_14;
    int32_t local_10;

    local_18 = param_3[0] - param_2[0];
    local_14 = param_3[1] - param_2[1];
    local_10 = param_3[2] - param_2[2];
    {
        int32_t tmp[3];
        tmp[0] = local_18;
        tmp[1] = local_14;
        tmp[2] = local_10;
        FUN_80016b08(tmp, param_1);
    }
}
