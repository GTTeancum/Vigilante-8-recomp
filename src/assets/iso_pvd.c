/* iso_pvd.c -- read the ISO9660 Primary Volume Descriptor into a buffer.
 *
 * Source: SLUS_005.10  FUN_8001555c.
 *
 * Reads sector 0x10 (the PVD) via V8_CdReadSectors into a stack buffer,
 * then copies the first 32 bytes out to the caller's destination using
 * an unaligned byte-by-u32 sequence. The unaligned-copy is the standard
 * PSY-Q misaligned-store idiom -- preserved as a memcpy of 32 bytes,
 * which is the semantic equivalent regardless of alignment.
 *
 * Returns the original `dst` pointer.
 *
 * HIGH confidence: sector 16 is the PVD by ISO9660 spec, and the first
 * 32 bytes hold the volume signature ("CD001" + version + system id
 * prefix) which Iso_VerifyOriginalDisc compares against the boot copy.
 */
#include <stdint.h>

extern void *V8_CdReadSectors(void *buf, int sector, int mode);
extern void *V8_MemCopy      (void *dst, const void *src, int n);

void *Iso_ReadPVD(void *dst)
{
    uint8_t sector[0x800] __attribute__((aligned(4)));
    V8_CdReadSectors(sector, 0x10, 1);
    V8_MemCopy(dst, sector + 8, 32);   /* PVD header starts at +8 */
    return dst;
}
