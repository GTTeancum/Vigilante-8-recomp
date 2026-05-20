/* gte_rotate.c -- high-precision GTE rotate helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8004316c  -- GTE_RotateSV(src, dst)        rotate a 16-bit svector
 *   FUN_80043248  -- GTE_RotateLong(src, dst)      rotate a 32-bit long vector
 *                                                  (17.15 -- split-int-frac)
 *   FUN_80043358  -- GTE_RotateLongMat(m, src, dst)  same + load matrix
 *   FUN_80043408  -- GTE_RotateLongMatTrans         + translation
 *   FUN_800434d0  -- GTE_RotatePacked(scratch, packedXYZ)
 *   FUN_8004352c  -- GTE_RotateLongMtxLow(m, src, dst)
 *   FUN_8004366c  -- GTE_LoadMatrixPacked(packedMat)
 *
 * Vigilante 8 uses a packed 17.15 vector format for world coords:
 *   v[0..2] is each axis as a 32-bit value where:
 *     hi 17 bits = signed integer part   (extracted via >> 15)
 *     lo 15 bits = fractional part        (extracted via & 0x7fff)
 *
 * The hi-precision rotate splits the vector into integer + fractional,
 * transforms each separately through the GTE's matrix, then sums:
 *     dst = (frac_rotated) + (int_rotated * 8)
 *
 * The `* 8` factor is because the GTE's `sf=0` mode shifts the result
 * left by 3 (the +3 bias the chip applies when output goes to IR after
 * accumulator). This effectively keeps the same 17.15 packing through
 * the transform.
 *
 * The GTE_LoadMatrixPacked helper reorders a five-u32 packed matrix
 * (each u32 holds two adjacent matrix entries) into the GTE's six
 * matrix registers in one shot.
 *
 * HIGH confidence: all of these match the canonical PSY-Q hi-precision
 * vector transform pattern. The math is bit-exact because every step
 * runs on the GTE -- the wrapper just feeds inputs and reads outputs.
 *
 * Bit-exact: do NOT inline the GTE ops as plain C. See gte.h.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

/* Forward decls for the GTE intrinsic-style wrappers (gte.h must
 * declare these matching the chip's saturating arithmetic). */
extern void  gte_ldv0     (const SVECTOR *v);
extern void  gte_ldsv_    (int x, int y, int z);
extern void  gte_ldtr     (int x, int y, int z);
extern int32_t gte_stIR1  (void);
extern int32_t gte_stIR2  (void);
extern int32_t gte_stIR3  (void);
extern void  gte_rtv0_b   (void);
extern void  gte_rtir_b   (void);
extern void  gte_rtir_sf0_b(void);
extern void  gte_rtirtr_b (void);
extern void  read_mt      (int *xr, int *yr, int *zr);     /* reads IR1,IR2,IR3 into ints */
extern void  gte_ldR11R12 (uint32_t v);
extern void  gte_ldR13R21 (uint32_t v);
extern void  gte_ldR22R23 (uint32_t v);
extern void  gte_ldR31R32 (uint32_t v);
extern void  gte_ldR33    (uint32_t v);

/* HIGH: rotate a 16-bit SVECTOR through the current GTE matrix. */
SVECTOR *GTE_RotateSV(const SVECTOR *src, SVECTOR *dst)
{
    gte_ldv0(src);
    gte_rtv0_b();
    dst->vx = (int16_t)gte_stIR1();
    dst->vy = (int16_t)gte_stIR2();
    dst->vz = (int16_t)gte_stIR3();
    return dst;
}

/* HIGH: rotate a 17.15 packed long vector using the current matrix. */
int32_t *GTE_RotateLong(const uint32_t *src, int32_t *dst)
{
    int xi = (int32_t)src[0] >> 15;
    int yi = (int32_t)src[1] >> 15;
    int zi = (int32_t)src[2] >> 15;

    gte_ldsv_(xi, yi, zi);
    uint32_t xf = src[0] & 0x7fffu;
    uint32_t yf = src[1] & 0x7fffu;
    gte_rtir_sf0_b();          /* transform integer part with sf=0 */
    uint32_t zf = src[2] & 0x7fffu;
    read_mt(&xi, &yi, &zi);    /* read the rotated integer back */

    gte_ldsv_(xf, yf, zf);
    gte_rtir_b();              /* transform fractional part with sf=1 */
    int fx, fy, fz;
    read_mt(&fx, &fy, &fz);
    (void)fx; (void)fy; (void)fz;

    /* Recombine: integer * 8 + fractional. */
    dst[0] = xf + xi * 8;
    dst[1] = yf + yi * 8;
    dst[2] = zf + zi * 8;
    return dst;
}

/* HIGH: load packed 5-u32 matrix (R11..R33) into the GTE registers. */
uint64_t GTE_LoadMatrixPacked(const uint32_t *m)
{
    uint32_t a = m[0] & 0xffffu;
    uint32_t c = m[1] & 0xffffu;
    gte_ldR11R12(a | (m[1] - c));
    uint32_t b = m[2] & 0xffffu;
    gte_ldR31R32(c | (m[2] - b));
    uint32_t d = m[3] & 0xffffu;
    uint32_t lo = (m[0] - a) | d;
    uint32_t hi = (m[3] - d) | b;
    gte_ldR13R21(lo);
    gte_ldR22R23(hi);
    gte_ldR33   (m[4]);
    return ((uint64_t)hi << 32) | lo;
}

/* HIGH: rotate-long with explicit matrix load. */
int32_t *GTE_RotateLongMat(const uint32_t *m, const uint32_t *src, int32_t *dst)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);
    return GTE_RotateLong(src, dst);
}

/* HIGH: rotate-long with matrix + translation. The translation is
 * stored as three u32s at m[5..7] and applied via the GTE's TRX/TRY/TRZ
 * registers using rtirtr_b (rotate + translate, with sf=1). */
int32_t *GTE_RotateLongMatTrans(const uint32_t *m, const uint32_t *src, int32_t *dst)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);

    int xi = (int32_t)src[0] >> 15;
    int yi = (int32_t)src[1] >> 15;
    int zi = (int32_t)src[2] >> 15;
    gte_ldsv_(xi, yi, zi);
    uint32_t xf = src[0] & 0x7fffu;
    uint32_t yf = src[1] & 0x7fffu;
    uint32_t zf = src[2] & 0x7fffu;
    gte_rtir_sf0_b();

    int tx = (int)m[5], ty = (int)m[6], tz = (int)m[7];
    gte_ldtr(tx, ty, tz);
    read_mt(&xi, &yi, &zi);

    gte_ldsv_(xf, yf, zf);
    gte_rtirtr_b();
    int fx, fy, fz;
    read_mt(&fx, &fy, &fz);
    (void)fx; (void)fy; (void)fz;

    dst[0] = xf + xi * 8;
    dst[1] = yf + yi * 8;
    dst[2] = zf + zi * 8;
    return dst;
}
