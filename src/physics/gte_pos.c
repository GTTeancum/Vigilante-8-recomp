/* gte_pos.c -- GTE position helper.
 *
 * Source: SLUS_005.10  FUN_80043224 (gap-recovered).
 *
 * Loads an SVECTOR (16-bit packed xyz) into the GTE's V0 register,
 * issues the GTE "long-vector store" copFunction(2, 0x480012)
 * (= gte_avs/gte_stlvnl variant -- preserves saturation), then reads
 * out the result as a VECTOR (long form).
 *
 * Used by Terrain_ColorAt to convert the current GTE-side world pos
 * (after the world->camera transform) back to a CPU-side VECTOR.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern void gte_ldv0(const SVECTOR *v);
extern void copFunction(int cop, uint32_t op);
extern void gte_stlvnl(VECTOR *out);

VECTOR *GTE_GetCurrentPos(const SVECTOR *in, VECTOR *out)
{
    gte_ldv0(in);
    copFunction(2, 0x480012u);
    gte_stlvnl(out);
    return out;
}
