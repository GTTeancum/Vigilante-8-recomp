/* math_helpers.c -- small 4.12 trig / linear-algebra helpers.
 *
 * Source: SLUS_005.10
 *   FUN_80016c54  -- Math_Atan2FromField(+10,+0x10)
 *   FUN_80016c88  -- Math_Atan2FromField(+4, +0x10)
 *   FUN_80016dfc  -- Matrix_InverseRigid
 *   FUN_80016e64  -- Math_NormalizeXZAndReorthogonalize
 *
 * ratan2 (PSY-Q) returns an angle in 4.12 format (4096 = 360 deg).
 *
 * HIGH-MED confidence: control flow and math operations match standard
 * idioms; the per-struct field offsets (+4, +10, +0x10) are inferred
 * from the call sites in this binary -- pass 2 should retype the
 * argument to the actual struct.
 */
#include <stdint.h>
#include "structs.h"

extern long ratan2(int y, int x);
extern long SquareRoot0(int n);
extern MATRIX *TransposeMatrix(const MATRIX *src, MATRIX *dst);
extern int32_t *GTE_RotateLongMat(const uint32_t *m, const uint32_t *src, int32_t *dst);

/* HIGH: read atan2 of two i16 fields in `obj` and return -angle as i16. */
int16_t Math_Atan2_PosNeg(const uint8_t *obj)
{
    int16_t y = *(int16_t *)(obj + 0x0a);
    int16_t x = *(int16_t *)(obj + 0x10);
    return (int16_t)-ratan2(y, x);
}

/* HIGH: companion -- positive sign. */
int16_t Math_Atan2_Pos(const uint8_t *obj)
{
    int16_t y = *(int16_t *)(obj + 0x04);
    int16_t x = *(int16_t *)(obj + 0x10);
    return (int16_t)ratan2(y, x);
}

/* HIGH: inverse of a rigid-body matrix = transpose(R) | -R^T * t.
 * Implementation: transpose, then rotate the original translation
 * through the (now-transposed) rotation, then negate. */
void Matrix_InverseRigid(const MATRIX *src, MATRIX *dst)
{
    TransposeMatrix(src, dst);
    GTE_RotateLongMat((const uint32_t *)dst, (const uint32_t *)src->t, dst->t);
    dst->t[0] = -dst->t[0];
    dst->t[1] = -dst->t[1];
    dst->t[2] = -dst->t[2];
}

/* HIGH-MED: re-orthogonalise an XZ basis. `v` is interpreted as a
 * compact 9-i16 structure containing two row vectors and a magnitude
 * slot. After this returns:
 *   v[0],v[6] are the X/Z components of the new normalised X axis.
 *   v[8]      is the magnitude of (v[0], v[6]).
 *   v[3],v[5] are the rotated old Y/Z axis (Y row).
 *   v[2]      is reset to zero (so the basis is purely planar in XZ).
 *
 * Used by vehicle / camera basis maintenance to keep the heading
 * vector unit length and orthogonal to the up axis.
 *
 * Bit-exact: the (n*0x1000000/len) divisions and 0xfff round-up biases
 * are preserved as in the binary. They implement a fixed-point reciprocal
 * normalisation that rounds toward positive infinity for negative
 * numerators (the classic 4.12 trick).
 */
void Math_NormalizeXZ(int16_t *v)
{
    long len = SquareRoot0((int)v[0] * v[0] + (int)v[6] * v[6]);
    int32_t inv = (len != 0) ? (0x1000000 / (int32_t)len) : 0;

    int32_t cx = (int32_t)v[0] * inv;
    int32_t cz = (int32_t)v[6] * inv;
    if (cx < 0) cx += 0xfff;
    if (cz < 0) cz += 0xfff;
    cx >>= 12;
    cz >>= 12;

    v[8] = (int16_t)len;
    v[6] = 0;
    int32_t r3 = cx * v[3] - cz * v[5];   if (r3 < 0) r3 += 0xfff;
    int32_t r4 = cz * v[3] + cx * v[5];   if (r4 < 0) r4 += 0xfff;
    v[3] = (int16_t)(r3 >> 12);
    v[5] = (int16_t)(r4 >> 12);
    int32_t r5 = cx * v[0] - cz * v[2];   if (r5 < 0) r5 += 0xfff;
    v[0] = (int16_t)(r5 >> 12);
    v[2] = 0;
}
