/* vec_scale_pack.c -- pack/scale a 5-element vec3-pair structure.
 *
 * Source: SLUS_005.10  FUN_80043754, FUN_80043864.
 *
 * The pair format is 5 u32s (10 i16 halves) representing two
 * cross-multiplied 4.12-fixed vec3s. Both functions multiply by a
 * 3-component scale (i32 / i16) and pack two products per output
 * u32 via the (low << 0 | high << 16) interleave.
 *
 * The 4.12 multiply uses `(short)a * b >> 0xc`, preserving the chip's
 * arithmetic-shift sign behavior. Bit-exact.
 *
 * FUN_80043754 takes a u32* scale (4.12 stored as two halves per word)
 * and a (long) i32 axis triple.
 * FUN_80043864 is identical except the axis triple is i16.
 *
 * HIGH confidence on the math; MED on the semantic name -- the
 * 5-u32 pair is most likely a {tangent, bitangent}-style scaled basis
 * used by the renderer / collision normals.
 */
#include <stdint.h>

uint32_t *Vec5_ScaleI32(const uint32_t *pair, const int32_t *scale, uint32_t *out)
{
    int32_t sx = scale[0], sy = scale[1], sz = scale[2];

    int32_t a0 = (int32_t)pair[0];
    int32_t a1 = (int32_t)pair[1];
    int32_t a2 = (int32_t)pair[2];
    int32_t a3 = (int32_t)pair[3];
    int32_t a4 = (int32_t)pair[4];

    out[0] = ((uint32_t)((int16_t) a0      * sx >> 12) & 0xffffu) |
             (((uint32_t)((a0 >> 16) * sy >> 12)) << 16);
    out[1] = ((uint32_t)((int16_t) a1      * sz >> 12) & 0xffffu) |
             (((uint32_t)((a1 >> 16) * sx >> 12)) << 16);
    out[2] = ((uint32_t)((int16_t) a2      * sy >> 12) & 0xffffu) |
             (((uint32_t)((a2 >> 16) * sz >> 12)) << 16);
    out[3] = ((uint32_t)((int16_t) a3      * sx >> 12) & 0xffffu) |
             (((uint32_t)((a3 >> 16) * sy >> 12)) << 16);
    out[4] = (uint32_t)((int16_t) a4 * sz >> 12);
    return (uint32_t *)pair;
}

uint32_t *Vec5_ScaleI16(const uint32_t *pair, const int16_t *scale, uint32_t *out)
{
    int16_t sx = scale[0], sy = scale[1], sz = scale[2];

    int32_t a0 = (int32_t)pair[0];
    int32_t a1 = (int32_t)pair[1];
    int32_t a2 = (int32_t)pair[2];
    int32_t a3 = (int32_t)pair[3];

    out[0] = ((uint32_t)((int16_t) a0      * sx >> 12) & 0xffffu) |
             (((uint32_t)((a0 >> 16) * sy >> 12)) << 16);
    out[1] = ((uint32_t)((int16_t) a1      * sz >> 12) & 0xffffu) |
             (((uint32_t)((a1 >> 16) * sx >> 12)) << 16);
    out[2] = ((uint32_t)((int16_t) a2      * sy >> 12) & 0xffffu) |
             (((uint32_t)((a2 >> 16) * sz >> 12)) << 16);
    out[3] = ((uint32_t)((int16_t) a3      * sx >> 12) & 0xffffu) |
             (((uint32_t)((a3 >> 16) * sy >> 12)) << 16);
    /* FUN_80043864 also writes out[4], same pattern with pair[4] and sz; preserved. */
    int32_t a4 = (int32_t)pair[4];
    out[4] = (uint32_t)((int16_t) a4 * sz >> 12);
    return (uint32_t *)pair;
}
