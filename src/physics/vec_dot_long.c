/* vec_dot_long.c -- FUN_80017240 (31 MIPS instructions).
 *
 * Dot product of a 32-bit long vector with an SVECTOR (i16 q12).
 * Returns a 64-bit signed integer in {v1:v0}.
 *
 *   dot = a[0]*vx + a[1]*vy + a[2]*vz
 *
 * where vx = (int16_t)(svec[0] & 0xffff),
 *       vy = (int16_t)(svec[0] >> 16),
 *       vz = *(int16_t *)(svec + 4)
 *
 * The MIPS performs three signed 32*32 multiplies (hi*lo products from
 * mult; mfhi/mflo), accumulates with carry-propagation, and returns
 * the 64-bit result in {v1, v0}.  The C int64_t arithmetic is exact
 * because the maximum magnitude is 2^31 * 2^15 * 3 = 2^48 (fits well
 * within 63 bits).
 *
 * Source: SLUS_005.10  0x80017240
 * HIGH confidence: pure integer arithmetic, no GTE.
 */
#include <stdint.h>

int64_t FUN_80017240(const int32_t *v, const void *svec)
{
    /* SVECTOR layout in memory (PSX little-endian):
     *   bytes 0-1: vx (int16_t)
     *   bytes 2-3: vy (int16_t)
     *   bytes 4-5: vz (int16_t)
     *   bytes 6-7: pad
     *
     * The MIPS loads a 32-bit word at a1+0 and extracts vx (lo16) and vy
     * (hi16) with sll/sra sign-extension, then uses lh at a1+4 for vz. */
    const uint8_t *p = (const uint8_t *)svec;
    uint32_t w0 = *(const uint32_t *)p;

    /* vx: lo16 sign-extended -- MIPS: sll v0,v0,0x10; sra v0,v0,0x10 */
    int32_t vx = (int32_t)(int16_t)(uint16_t)(w0 & 0xffffu);
    /* vy: hi16 sign-extended -- MIPS: sra v0,v0,0x10 */
    int32_t vy = (int32_t)(int16_t)(uint16_t)(w0 >> 16);
    /* vz: sign-extending halfword load -- MIPS: lh v0,0x4(a1) */
    int32_t vz = (int32_t)*(const int16_t *)(p + 4);

    /* Three 32*16 multiplies accumulated in 64-bit. */
    return (int64_t)v[0] * (int64_t)vx
         + (int64_t)v[1] * (int64_t)vy
         + (int64_t)v[2] * (int64_t)vz;
}
