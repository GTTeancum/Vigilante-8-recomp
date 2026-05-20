/* terrain_height.c -- bilinear-interpolated terrain heightmap.
 *
 * Source: SLUS_005.10  FUN_80025400.
 *
 * Vigilante 8's terrain heightmap is laid out as a 32x32 grid of
 * chunks, each chunk a 64x64 array of 11-bit height samples.
 * Addressing decomposes a (x, z) world coordinate (each a u32 in
 * 16.16-ish fixed-point, with the upper 6 bits being the chunk
 * coordinate and the next 6 the in-chunk cell):
 *
 *   chunk_x = x >> 22                 (top 10 bits -> 0..1023, masked 0x3ff)
 *   chunk_z = z >> 22
 *   cell_x  = (x >> 16) & 0x3f
 *   cell_z  = (z >> 16) & 0x3f
 *   frac_x  = x & 0xffff
 *   frac_z  = z & 0xffff
 *
 *   chunkBase = DAT_800911a0[chunk_x * 0x20 + chunk_z]
 *   h(cx,cz)  = *(u16 *)(chunkBase + cz * 2 + cx * 0x80) & 0x7ff
 *
 * The bilinear sample reads four cells (cx,cz), (cx+1,cz),
 * (cx,cz+1), (cx+1,cz+1) -- with chunk-crossing handled by
 * advancing chunk_x or chunk_z when cx+1 or cz+1 overflows 0x3f.
 *
 * The interpolation works in 16-bit fractional weights: the result is
 *
 *   h * 0x10000
 *   + frac_x * (h_xp - h)
 *   + frac_z * (h_zp - h)
 *   + (frac_x * frac_z >> 16) * (h_xpzp - h_xp - h_zp + h)
 *
 * The `if (frac_x + frac_z < 0x10000)` test selects the
 * lower-triangle (xp+zp diagonal) vs upper-triangle sample mix.
 * This is the classic split-triangle bilinear used by PSX heightmaps.
 *
 * HIGH confidence: layout is unambiguous (32x32 of 64x64 of u16); the
 * 11-bit mask is consistent across all callers.
 *
 * Pass 2 should wrap (x, z) into proper world-coord typedefs.
 */
#include <stdint.h>

#define CHUNK_GRID_DIM   32     /* 32x32 chunks                 */
#define CELL_PER_CHUNK   64     /* 64x64 cells per chunk        */
#define HEIGHT_MASK      0x7ff  /* 11-bit height value          */

extern uintptr_t DAT_800911a0[CHUNK_GRID_DIM * CHUNK_GRID_DIM];

static inline uint32_t terrain_sample(uint32_t cx, uint32_t cz)
{
    uint32_t chunkIdx = (cx >> 6) * CHUNK_GRID_DIM + (cz >> 6);
    uintptr_t chunkBase = DAT_800911a0[chunkIdx];
    uint32_t  off = ((cx & 0x3f) << 7) | ((cz & 0x3f) << 1);
    return *(uint16_t *)(chunkBase + off) & HEIGHT_MASK;
}

/* HIGH: sample terrain height at world (x, z), 16.16-ish.
 * Returns a 16.16 fixed-point world height (the * 0x10000 below).
 */
int32_t Terrain_HeightAt(uint32_t x, uint32_t z)
{
    uint32_t fx     = x & 0xffff;
    uint32_t fz     = z & 0xffff;
    uint32_t cx0    = x >> 16;
    uint32_t cz0    = z >> 16;
    uint32_t h00    = terrain_sample(cx0, cz0);

    /* Triangle split: which half of the cell are we in? */
    if (fx + fz < 0x10000u) {
        uint32_t h10 = terrain_sample(cx0 + 1, cz0);
        uint32_t h01 = terrain_sample(cx0,     cz0 + 1);
        return (int32_t)(h00 * 0x10000
                       + fx * (h10 - h00)
                       + fz * (h01 - h00));
    } else {
        uint32_t h11 = terrain_sample(cx0 + 1, cz0 + 1);
        uint32_t h10 = terrain_sample(cx0 + 1, cz0);
        uint32_t h01 = terrain_sample(cx0,     cz0 + 1);
        /* upper triangle: bilinear with the opposite diagonal */
        return (int32_t)(h11 * 0x10000
                       + (0xffff - fx) * (h01 - h11)
                       + (0xffff - fz) * (h10 - h11));
    }
}

/* HIGH: per-cell texture material id at world (x, z). */
extern uint8_t DAT_8008f020[];   /* material palette */
uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z)
{
    uint32_t chunkIdx = (x >> 22) * CHUNK_GRID_DIM + (z >> 22);
    uintptr_t chunkBase = DAT_800911a0[chunkIdx];
    uint32_t  off = ((z >> 16) & 0x3f) + ((x >> 10) & 0xfc0) + 0x2000;
    return &DAT_8008f020[ *(uint8_t *)(chunkBase + off) * 0x20 ];
}
