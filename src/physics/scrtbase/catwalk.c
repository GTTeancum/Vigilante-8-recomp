/* catwalk.c -- Secret Base catwalk bounding-box update.
 *
 * Source: SCRTBASE.DLL  FUN_80100624.
 *
 * Each tick, computes a 4-corner i16 AABB by adding the parent's
 * position to the per-frame offset slots from a linked child struct
 * (at obj+0x5c), rounds to 16.16, then writes the 4 i16 corner
 * coordinates into a local for the bounds-check code in the caller.
 *
 * Used by `catwalk_1` which has movable segments; the precomputed
 * bounding box accelerates point-in-rect tests for vehicles walking
 * across it.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

void SB_CatwalkBounds(int obj)
{
    int *child = *(int **)(obj + 0x5c);

    int32_t x0 = *(int32_t *)(obj + 0x48) + child[1];
    int32_t z0 = *(int32_t *)(obj + 0x50) + child[3];
    int32_t x1 = *(int32_t *)(obj + 0x48) + child[4];
    int32_t z1 = *(int32_t *)(obj + 0x50) + child[5];

    if (x0 < 0) x0 += 0xffff;
    if (z0 < 0) z0 += 0xffff;
    if (x1 < 0) x1 += 0xffff;
    if (z1 < 0) z1 += 0xffff;

    int16_t cornerX0 = (int16_t)((uint32_t)x0 >> 16);
    int16_t cornerZ0 = (int16_t)((uint32_t)z0 >> 16);
    int16_t cornerX1 = (int16_t)((uint32_t)x1 >> 16);
    int16_t cornerZ1 = (int16_t)((uint32_t)z1 >> 16);
    (void)cornerX0; (void)cornerZ0; (void)cornerX1; (void)cornerZ1;
    /* Original continues -- writes the 4 corners to a contiguous u16
     * block on the stack and likely passes the pointer to a bounds-
     * test helper. Pass 3 finalises the caller chain. */
}
