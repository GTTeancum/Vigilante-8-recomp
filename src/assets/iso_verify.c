/* iso_verify.c -- "is this the original disc?" check.
 *
 * Source: SLUS_005.10  FUN_80015610.
 *
 * Reads sector 16 (the ISO9660 Primary Volume Descriptor) into a 32-byte
 * stack buffer via FUN_8001555c (a thin sector reader), then memcmps
 * the first 32 bytes against a saved copy at DAT_8006f608 captured at
 * boot. Returns non-zero (truthy) when they match.
 *
 * The original "Insert Original CD" prompt is at @ 0x80010070 -- this
 * is the function that drives that screen.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void *FUN_8001555c(void *dst);      /* sector-16 reader -- TBD pass 2 */
extern int   Util_MemEq(const void *a, const void *b, int n);   /* FUN_800523a0 */
extern uint8_t DAT_8006f608[];

int Iso_VerifyOriginalDisc(void)
{
    uint8_t pvd[32];
    void   *p = FUN_8001555c(pvd);
    return Util_MemEq(p, DAT_8006f608, 32) == 0;
}
