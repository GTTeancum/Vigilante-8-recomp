/* matrix_identity.c -- identity-rotation matrix builder.
 *
 * Source: SLUS_005.10  FUN_80016da8.
 *
 * Initializes an 8-u32 packed matrix (5 rotation + 3 translation = the
 * format consumed by GTE_LoadTransform / GTE_LoadMatrixPacked) to the
 * identity rotation. In packed form:
 *   [0] = 0x1000   (R11=0x1000, R12=0)
 *   [1] = 0
 *   [2] = 0x1000   (R22=0x1000, R23=0)
 *   [3] = 0
 *   [4] = 0x1000   (R33=0x1000)
 *   [5] = 0        (TRX)
 *   [6] = 0        (TRY)
 *   [7] = 0        (TRZ)
 *
 * 0x1000 is "1.0" in 4.12 fixed-point. This is the canonical "fresh
 * matrix" each object's draw step starts with.
 *
 * HIGH confidence.
 */
#include <stdint.h>

void Matrix_PackedIdentity(uint32_t *m)
{
    m[0] = 0x1000;
    m[1] = 0;
    m[2] = 0x1000;
    m[3] = 0;
    m[4] = 0x1000;
    m[5] = 0;
    m[6] = 0;
    m[7] = 0;
}
