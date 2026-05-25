/* gte_aliases.c -- raw-address aliases for PSY-Q/GTE helper calls.
 *
 * MED/HIGH: wrappers delegate to decoded PSY-Q-compatible paths so physics
 * and gameplay raw-address callers no longer bind through platform fallback.
 */
#include <stdint.h>
#include "psyq/psyq_stubs.h"

extern void gte_ldVXY0_i(int v);
extern void gte_ldVZ0_i(int v);
extern void gte_rtv0(void);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);
extern void gte_ldtr(int x, int y, int z);
extern uint64_t GTE_LoadMatrixPackedAbs(const uint32_t *m);

void gte_set_rot_matrix(const MATRIX *m) { SetRotMatrix(m); }
void gte_set_translation(const VECTOR *v) { gte_ldtr(v->vx, v->vy, v->vz); }
MATRIX *FUN_8004d734(MATRIX *src, MATRIX *dst) { return TransposeMatrix(src, dst); }

MATRIX *MulRotMatrix0(MATRIX *m, MATRIX *out)
{
    int16_t *r = &m->m[0][0];

    gte_ldVXY0_i((uint16_t)r[0] | ((uint32_t)(uint16_t)r[3] << 16));
    gte_ldVZ0_i(r[6]);
    gte_rtv0();
    int c00 = gte_stIR1();
    int c10 = gte_stIR2();
    int c20 = gte_stIR3();

    gte_ldVXY0_i((uint16_t)r[1] | ((uint32_t)(uint16_t)r[4] << 16));
    gte_ldVZ0_i(r[7]);
    gte_rtv0();
    int c01 = gte_stIR1();
    int c11 = gte_stIR2();
    int c21 = gte_stIR3();

    gte_ldVXY0_i((uint16_t)r[2] | ((uint32_t)(uint16_t)r[5] << 16));
    gte_ldVZ0_i(r[8]);
    gte_rtv0();

    *(uint32_t *)((uint8_t *)out + 0x00) =
        ((uint32_t)(uint16_t)c01 << 16) | (uint16_t)c00;
    *(uint32_t *)((uint8_t *)out + 0x04) =
        ((uint32_t)(uint16_t)c10 << 16) | (uint16_t)gte_stIR1();
    *(uint32_t *)((uint8_t *)out + 0x08) =
        ((uint32_t)(uint16_t)gte_stIR2() << 16) | (uint16_t)c11;
    *(uint32_t *)((uint8_t *)out + 0x0c) =
        ((uint32_t)(uint16_t)c21 << 16) | (uint16_t)c20;
    *(int32_t *)((uint8_t *)out + 0x10) = gte_stIR3();
    return out;
}

MATRIX *FUN_8004cdc4(MATRIX *matA, MATRIX *matB) { return MulRotMatrix0(matA, matB); }
int FUN_8004ecd4(int y, int x) { return ratan2(y, x); }
MATRIX *FUN_8004d154(MATRIX *src, MATRIX *dst) { return MulMatrix0(src, dst, dst); }
MATRIX *FUN_8004d314(MATRIX *m, const VECTOR *t) { return TransMatrix(m, t); }
void FUN_8004d524(int ofx, int ofy) { SetGeomOffset(ofx, ofy); }
void FUN_8004d544(int h) { SetGeomScreen(h); }
void FUN_800436c8(MATRIX *m) { GTE_LoadMatrixPackedAbs((const uint32_t *)m); }
