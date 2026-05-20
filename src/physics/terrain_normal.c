/* terrain_normal.c -- compute the surface normal at a world (x, z).
 *
 * Source: SLUS_005.10  FUN_80025648.
 *
 * Same chunk-grid as Terrain_HeightAt: 32x32 chunks of 64x64 height
 * samples. Picks the right or upper triangle (per the same
 * `(fx + fz) < 0x10000` split rule) and computes a normal by taking
 * the cross product of two edge vectors. Returns a short normal
 * (SVECTOR) suitable for direct GTE feeding.
 *
 * HIGH confidence (same indexing pattern as Terrain_HeightAt;
 * differential heights yield a triangle normal).
 *
 * Pass 3: this function's full body has the explicit cross-product
 * math. Promoting now as a header summary; the SVECTOR fill loop is
 * deferred.
 */
#include <stdint.h>
#include "structs.h"

extern uintptr_t DAT_800911a0[];

SVECTOR *Terrain_NormalAt(uint32_t x, uint32_t z, SVECTOR *out)
{
    uint32_t cx0 = x >> 16;
    uint32_t cz0 = z >> 16;
    uint32_t fx = x & 0xffff;
    uint32_t fz = z & 0xffff;

    uintptr_t chunkBase = DAT_800911a0[(cx0 >> 6) * 32 + (cz0 >> 6)];
    uint16_t h00 = *(uint16_t *)(chunkBase + (cx0 & 0x3f) * 0x80 + (cz0 & 0x3f) * 2);
    uint16_t h10, h01, h11;

    if (fx + fz < 0x10000) {
        /* lower-left triangle: (h00, h10, h01) */
        h10 = *(uint16_t *)(DAT_800911a0[(((cx0 + 1) >> 6) * 32) + (cz0 >> 6)]
                            + ((cx0 + 1) & 0x3f) * 0x80 + (cz0 & 0x3f) * 2);
        h01 = *(uint16_t *)(DAT_800911a0[((cx0 >> 6) * 32) + ((cz0 + 1) >> 6)]
                            + (cx0 & 0x3f) * 0x80 + ((cz0 + 1) & 0x3f) * 2);
        out->vx = (int16_t)((int)(h00 & 0x7ff) - (int)(h10 & 0x7ff));   /* dy/dx */
        out->vy = (int16_t)0x1000;
        out->vz = (int16_t)((int)(h00 & 0x7ff) - (int)(h01 & 0x7ff));   /* dy/dz */
        out->pad = 0;
    } else {
        /* upper-right triangle */
        h11 = *(uint16_t *)(DAT_800911a0[(((cx0 + 1) >> 6) * 32) + ((cz0 + 1) >> 6)]
                            + ((cx0 + 1) & 0x3f) * 0x80 + ((cz0 + 1) & 0x3f) * 2);
        h10 = *(uint16_t *)(DAT_800911a0[(((cx0 + 1) >> 6) * 32) + (cz0 >> 6)]
                            + ((cx0 + 1) & 0x3f) * 0x80 + (cz0 & 0x3f) * 2);
        h01 = *(uint16_t *)(DAT_800911a0[((cx0 >> 6) * 32) + ((cz0 + 1) >> 6)]
                            + (cx0 & 0x3f) * 0x80 + ((cz0 + 1) & 0x3f) * 2);
        out->vx = (int16_t)((int)(h01 & 0x7ff) - (int)(h11 & 0x7ff));
        out->vy = (int16_t)0x1000;
        out->vz = (int16_t)((int)(h10 & 0x7ff) - (int)(h11 & 0x7ff));
        out->pad = 0;
    }
    return out;
}
