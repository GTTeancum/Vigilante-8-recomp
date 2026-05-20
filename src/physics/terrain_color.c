/* terrain_color.c -- terrain color sampler keyed off the heightmap.
 *
 * Source: LOAD.DLL  FUN_80104c68.
 *
 * Reads a (x, z) world coordinate via the GTE pos-from-matrix helper
 * (func_0x80043224 -- gets the current world-pos from the GTE
 * register file), normalises negative-fractional rounds, indexes the
 * exact same DAT_800911a0-rooted 32x32 chunk grid as Terrain_HeightAt
 * (see src/physics/terrain_height.c), and turns the high bits of the
 * 11-bit height into a greyscale RGB triplet.
 *
 * Formula:
 *   chunk_x = (x >> 16) >> 6
 *   chunk_z = (z >> 16) >> 6
 *   cell_x  = (x >> 16) & 0x3f
 *   cell_z  = (z >> 16) & 0x3f
 *   h11     = *((u16*)(chunkBase + cell_x*0x80 + cell_z*2)) & 0x7ff
 *   shade   = (h11 >> 11) << 2     -- the high bits scaled into 0..3F<<2
 *
 * The shade is replicated into all three RGB channels (greyscale).
 *
 * Note the high right-shift `>> 0xb` (11) followed by `<< 2`: that's
 * effectively taking the top 5 bits of the 16-bit height word, then
 * shifting left 2 to put them in the 4..7 bit range of an 8-bit
 * channel value. Bit-exact.
 *
 * HIGH-MED confidence (same offset/mask pattern as Terrain_HeightAt).
 */
#include <stdint.h>

extern uintptr_t DAT_800911a0[];
extern void XobfGTE_GetCurrentPos(void *gteStateOpaque, int32_t *xyz);  /* func_0x80043224 */

#define CHUNK_GRID_DIM   32

void Terrain_ColorAt(uint8_t rgb[3], void *gteStateOpaque)
{
    int32_t xz[2];
    XobfGTE_GetCurrentPos(gteStateOpaque, xz);
    int32_t x = xz[0];
    int32_t z = xz[1];

    if (x < 0) x += 0xffff;
    if (z < 0) z += 0xffff;

    uint32_t cx     = (uint32_t)x >> 16;
    uint32_t cz     = (uint32_t)z >> 16;
    uintptr_t chunkBase = DAT_800911a0[(cx >> 6) * CHUNK_GRID_DIM + (cz >> 6)];
    uint16_t  h     = *(uint16_t *)(chunkBase + (cx & 0x3f) * 0x80 + (cz & 0x3f) * 2);
    uint8_t   shade = (uint8_t)((h >> 11) << 2);

    rgb[0] = shade;
    rgb[1] = shade;
    rgb[2] = shade;
}
