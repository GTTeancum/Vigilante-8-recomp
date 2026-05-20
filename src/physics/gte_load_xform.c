/* gte_load_xform.c -- load a full {rotation, translation} matrix into GTE.
 *
 * Source: SLUS_005.10  FUN_80043974.
 *
 * Takes a packed 8-u32 transform (5 u32 rotation + 3 u32 translation)
 * and loads it into the GTE's R11R12..R33 + TRX/TRY/TRZ registers.
 * The rotation layout matches GTE_LoadMatrixPacked.
 *
 * HIGH confidence: single-purpose, 8 GTE ops in order.
 */
#include <stdint.h>

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR13R21(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR31R32(uint32_t v);
extern void gte_ldR33   (uint32_t v);
extern void gte_ldTRX   (uint32_t v);
extern void gte_ldTRY   (uint32_t v);
extern void gte_ldTRZ   (uint32_t v);

void GTE_LoadTransform(const uint32_t *m)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);
    gte_ldTRX   (m[5]);
    gte_ldTRY   (m[6]);
    gte_ldTRZ   (m[7]);
}
