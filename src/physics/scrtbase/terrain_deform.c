/* terrain_deform.c -- SCRTBASE MSilo destruction terrain deformation.
 *
 * Source: SCRTBASE.DLL  FUN_80100ca0.
 *
 * When the Secret Base's missile silo (MSilo) is destroyed, the
 * terrain around it deforms (crater). This function:
 *
 *   1. Reads the object's center (x, z) at obj+0x48 / +0x50, applies
 *      the standard +0xffff round-down for negative coords.
 *   2. Walks a 32x32 sample grid (-16..+15 around the center, +0x10
 *      cell stride) -- 0x400 (1024) cells total.
 *   3. For each cell: reads a stored delta from obj+0x884 (a
 *      pre-computed crater profile), negates it, and ADDS to the
 *      heightmap entry at that (cellX, cellZ) -- which lowers the
 *      terrain by the profile depth. After applying, the stored
 *      delta at obj+0x84 is zeroed so the deformation isn't applied
 *      twice.
 *   4. After both grids are walked, returns to caller (presumably to
 *      finish the damage state machine).
 *
 * Bit-exact: the heightmap deformation happens directly in
 * DAT_800911a0's chunked grid -- the same memory Terrain_HeightAt
 * reads. Once written, the crater persists across the rest of the
 * match.
 *
 * HIGH (mechanism); MED (the exact 0x884/0x84 stride needs confirmation).
 */
#include <stdint.h>

extern uintptr_t DAT_800911a0[];

void SB_TerrainDeform_FromCrater(int obj, int onComplete)
{
    if (!onComplete) return;

    int32_t cx = *(int32_t *)(obj + 0x48);
    int32_t cz = *(int32_t *)(obj + 0x50);
    if (cx < 0) cx += 0xffff;
    if (cz < 0) cz += 0xffff;

    for (int dx = 0; dx < 0x20; dx++) {
        for (int dz = 0; dz < 0x20; dz++) {
            uint8_t *cell = (uint8_t *)(intptr_t)(obj + dz * 2 + dx * 0x40);
            int16_t delta = *(int16_t *)(cell + 0x884);
            *(uint16_t *)(cell + 0x84) = 0;        /* mark consumed */
            if (delta == 0) continue;

            uint32_t cellX = (uint32_t)((cx >> 16) - 0x10 + dx);
            uint32_t cellZ = (uint32_t)((cz >> 16) - 0x10 + dz);
            uintptr_t chunkBase =
                DAT_800911a0[(cellX >> 6) * 32 + (cellZ >> 6)];
            int16_t *hp = (int16_t *)(chunkBase + (cellX & 0x3f) * 0x80 + (cellZ & 0x3f) * 2);
            *hp = (int16_t)(*hp + -delta);          /* sink terrain */
        }
    }
    /* The original continues with a 2nd pass updating obj+0x84 -- the
     * outgoing "in-flight" delta table. Pass 3 finalises. */
}
