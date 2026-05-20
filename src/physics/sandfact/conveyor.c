/* conveyor.c -- Sand Factory M2_Conveyor & M2_elevator motion tick.
 *
 * Source: SANDFACT.DLL  FUN_80100c6c.
 *
 * Integrates a position vector (u32[9..11] -> world x,y,z) by a
 * velocity vector (u32[0x20..0x22]) scaled by 1/128. The /128 rounding
 * uses the negative-correction trick (`+ 0x7f` before arith right
 * shift) to round toward zero.
 *
 * Used by the Sand Factory's slow moving conveyor and elevator
 * platforms -- velocity is shallow enough that 7-bit fractional
 * precision is sufficient for visible smoothness.
 *
 * Bit-exact: the +0x7f / >>7 sequence is preserved.
 *
 * HIGH-MED confidence: the integration pattern is unambiguous; the
 * function only handles modes 0 and 2 in this branch.
 */
#include <stdint.h>

uint32_t SF_ConveyorTick(uint32_t *obj, int mode)
{
    if (mode != 0 && mode != 2) return 0;

    int32_t vx = (int32_t)obj[0x20];
    int32_t vy = (int32_t)obj[0x21];
    int32_t vz = (int32_t)obj[0x22];

    if (vx < 0) vx += 0x7f;
    if (vy < 0) vy += 0x7f;
    if (vz < 0) vz += 0x7f;

    obj[9]  += (uint32_t)(vx >> 7);
    obj[10] += (uint32_t)(vy >> 7);
    obj[11] += (uint32_t)(vz >> 7);
    return 0;
}
