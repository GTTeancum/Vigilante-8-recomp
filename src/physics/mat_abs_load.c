/* mat_abs_load.c -- load absolute-value of packed matrix into GTE.
 *
 * Source: SLUS_005.10  FUN_800436c8.
 *
 * Takes a packed 5-u32 matrix (same layout as GTE_LoadMatrixPacked)
 * and loads it into the GTE registers with every i16 component
 * replaced by its absolute value. The negation pattern includes the
 * "flip if MSB set after the negate" step (`uVar ^= 0xffff`) which
 * handles INT_MIN saturation in a way that matches the chip's
 * absolute-value semantics exactly.
 *
 * Used (presumably) for axis-aligned bound box checks where the
 * matrix's *magnitudes* alone matter -- e.g. projecting an AABB
 * extent onto an arbitrary rotation.
 *
 * HIGH confidence on the operation; MED on the intended call site.
 */
#include <stdint.h>

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR13R21(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR31R32(uint32_t v);
extern void gte_ldR33   (uint32_t v);

static inline uint32_t absi16_pair(uint32_t v)
{
    uint32_t r = v;
    if ((int32_t)r < 0)         r = -r;
    if ((int32_t)(r << 16) < 0) r ^= 0xffff;
    return r;
}

uint64_t GTE_LoadMatrixPackedAbs(const uint32_t *m)
{
    uint32_t a = absi16_pair(m[0]);
    uint32_t b = absi16_pair(m[1]);
    uint32_t c = absi16_pair(m[2]);
    uint32_t d = absi16_pair(m[3]);
    int32_t  e = (int32_t)(int16_t)m[4];
    if (e < 0) e = -e;

    gte_ldR11R12(a);
    gte_ldR13R21(b);
    gte_ldR22R23(c);
    gte_ldR31R32(d);
    gte_ldR33   ((uint32_t)e);
    return ((uint64_t)e << 32) | d;
}
