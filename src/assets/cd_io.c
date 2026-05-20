/* cd_io.c -- Low-level CD-ROM I/O for asset loading.
 *
 * Source: SLUS_005.10
 *   FUN_800154f4 -- HIGH: synchronous-read-N-sectors at a given logical sector.
 *
 * The function takes (buf, sector, mode) and:
 *   1. Converts the sector number to BCD MM:SS:FF via CdIntToPos.
 *   2. Issues CdControl(0x15 == CdlSetloc, &pos, NULL) to seek.
 *   3. Calls CdRead(mode, buf, 0x80) -- mode passes through (e.g. 0x80
 *      requests "double-speed mode 1 data" if the upper-bit semantics
 *      match PSY-Q's `CdMode`).
 *   4. Blocks on CdReadSync(0, NULL) until the read finishes.
 *
 * 0x80 as the third arg of CdRead is the count: the original passes the
 * fixed constant 0x80, meaning 128 sectors (256 KiB at 2048 bytes/sector)
 * are always requested. Callers therefore know their target buffers must
 * be that large -- or that they will truncate via mode flags. Pass 2
 * should confirm by looking at every callsite of this function.
 *
 * Bit-exact: do NOT replace with libcd's CdReadSync wrapper macros; the
 * caller's reliance on exact sector counts and modes matters for asset
 * positioning on disc.
 */
#include <stdint.h>
#include "structs.h"

/* PSY-Q declarations. Implementation comes from libcd in the port. */
extern void   CdIntToPos(int sector, CdlLOC *pos);
extern int    CdControl(unsigned char com, unsigned char *param, unsigned char *result);
extern int    CdRead(int count, void *buf, int mode);
extern int    CdReadSync(int mode, unsigned char *result);

#define CdlSetloc  0x15

/* HIGH: synchronously load 0x80 sectors (256 KiB max) from `sector` into `buf`
 * using the supplied `mode`. Returns `buf` unchanged. */
void *V8_CdReadSectors(void *buf, int sector, int mode)
{
    CdlLOC pos;
    CdIntToPos(sector, &pos);
    CdControl(CdlSetloc, (unsigned char *)&pos, (unsigned char *)0);
    CdRead(mode, buf, 0x80);
    CdReadSync(0, (unsigned char *)0);
    return buf;
}
