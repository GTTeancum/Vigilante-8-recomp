/* cd_toc.c -- read the CD table of contents into the per-track position table.
 *
 * Source: SLUS_005.10  FUN_80043a74.
 *
 * Busy-polls CdGetToc until it returns a non-zero track count (the BIOS
 * is asynchronous; the first call may return 0 while it waits for the
 * drive). Pops the per-track start position from the BIOS into the
 * fixed table at DAT_800a3090 (which is read by CD_PlayTrack).
 *
 * The trailing "-2 if the disc has a data + audio mix" tweak (subtract
 * the data-track count from the track count) makes the disc-type=4
 * (PSX with audio) numbering line up with the disc-type=3 (PSX only)
 * numbering used by the rest of the audio code.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern int    CdGetToc   (CdlLOC *tocBuf);
extern int    CdPosToInt (const CdlLOC *pos);
extern void   CdIntToPos (int sector, CdlLOC *pos);
extern CdlLOC DAT_800a3090[];   /* track-position table (track 0 = first index) */
extern uint8_t bRam000008f8;     /* track count latch */
extern uint8_t uRam000005ac;     /* disc-type code (see cd_audio.c) */

void Audio_RefreshKeyState(void)
{
    uint32_t n;
    do {
        n = (uint32_t)CdGetToc(DAT_800a3090);
        bRam000008f8 = (uint8_t)n;
    } while ((n & 0xff) == 0);

    /* DAT_800a3090[0] is the leadout position; convert to int sector,
     * subtract the 150-sector lead-in offset, and store back into the
     * "leadout" slot (just past the actual tracks). */
    int leadoutSec = CdPosToInt(&DAT_800a3090[0]) - 0x96;
    CdIntToPos(leadoutSec, &DAT_800a3090[bRam000008f8]);

    /* Compensate for the data track on mixed-mode discs. */
    bRam000008f8 = (uint8_t)(bRam000008f8 - (uRam000005ac > 2 ? 1 : 0));
}
