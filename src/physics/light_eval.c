/* light_eval.c -- per-vertex lighting evaluation (color clamp at 255).
 *
 * Source: LOAD.DLL  FUN_801011a0.
 *
 * Per-vertex color computation:
 *   1. GTE_GetCurrentPos(srcPos, &local_40)
 *   2. GTE_RotateSV(srcNormal, &local_30)
 *   3. NormalColorCol(rotatedNormal, baseColor, &local_28) -- the
 *      PSY-Q libgte function that computes color = base * dot(N, L).
 *   4. Walk the light source chain at DAT_80107d90 (idle loop -- the
 *      caller doesn't actually act, but the iteration is preserved
 *      from the original).
 *   5. Clamp each output channel to [0..0xff]: if signed value is -1
 *      (= overflow), use 0xff.
 *
 * Used by the obj-prim per-vertex lighting at draw time.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>
#include "structs.h"

extern VECTOR *GTE_GetCurrentPos(const SVECTOR *in, VECTOR *out);
extern SVECTOR *GTE_RotateSV(const SVECTOR *src, SVECTOR *dst);
extern void NormalColorCol(SVECTOR *normal, void *baseColor, void *outColor);  /* PSY-Q libgte */
extern int **DAT_80107d90;

void Light_VertexColor(int8_t outRGB[3], const void *baseColor,
                       const SVECTOR *srcPos, const SVECTOR *srcNormal)
{
    VECTOR scratchPos;
    SVECTOR scratchNormal;
    int8_t  computed[4];

    GTE_GetCurrentPos(srcPos, &scratchPos);
    GTE_RotateSV(srcNormal, &scratchNormal);
    NormalColorCol(&scratchNormal, (void *)baseColor, computed);

    /* Idle light-chain walk preserved from original (no side effects). */
    for (int *p = (int *)DAT_80107d90[0]; p != NULL; p = (int *)*p) { /* nop */ }

    outRGB[0] = (computed[0] == -1) ? -1 : computed[0];
    outRGB[1] = (computed[1] == -1) ? -1 : computed[1];
    outRGB[2] = (computed[2] == -1) ? -1 : computed[2];
}
