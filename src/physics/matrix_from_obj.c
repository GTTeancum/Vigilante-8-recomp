/* matrix_from_obj.c -- build a MATRIX from an object's euler+pos.
 *
 * Source: SLUS_005.10  FUN_8001b07c.
 *
 * Two-step:
 *   1. RotMatrixYXZ_gte fills the rotation part from the i16 euler
 *      angles at obj+0x10 (Yaw, Pitch, Roll order -- "YXZ").
 *   2. FUN_8004d314 (TBD: TransMatrix) writes the translation from
 *      obj+0x04 (i32 px, py, pz).
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern void RotMatrixYXZ_gte(const SVECTOR *euler, MATRIX *m);
extern MATRIX *TransMatrix(MATRIX *m, const VECTOR *t);   /* FUN_8004d314 */

MATRIX *Matrix_FromObjectTransform(MATRIX *out, int obj)
{
    RotMatrixYXZ_gte((const SVECTOR *)(intptr_t)(obj + 0x10), out);
    TransMatrix(out, (const VECTOR *)(intptr_t)(obj + 4));
    return out;
}
