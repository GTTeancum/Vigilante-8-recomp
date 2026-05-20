/* cd_audio.c -- CD audio (redbook + XA) control.
 *
 * Source: SLUS_005.10
 *   FUN_80043aec  -- CD_DetectDiskType
 *   FUN_80043bb4  -- CD_IsCoverOpen
 *   FUN_80043be4  -- CD_Pause            (CdlPause)
 *   FUN_80043c0c  -- CD_Play             (CdlPlay)
 *   FUN_80043ce0  -- CD_PlayTrack(trackIdx)
 *   FUN_80043df8  -- Audio_PlayXa(path, channel)
 *
 * On-disc track positions live in a static table at DAT_800a3090
 * (alternating MM:SS:FF entries) -- pass 2 should extract that table
 * into src/data/cd_tracks.h.
 *
 * NOTE: CdAudio control is OUT OF SCOPE per CLAUDE.md (audio engine is
 * being rewritten). These functions are kept here as the seam contract
 * the new audio layer must satisfy.
 */
#include <stdint.h>
#include "structs.h"

extern int  CdDiskReady(int mode);
extern int  CdGetDiskType(void);
extern void CdControl  (unsigned char com, unsigned char *param, unsigned char *result);
extern void CdControlB (unsigned char com, unsigned char *param, unsigned char *result);
extern void CdControlF (unsigned char com, unsigned char *param);
extern void CdIntToPos (int sector, CdlLOC *pos);
extern int  CdPosToInt (const CdlLOC *pos);
extern int  CdRead2    (int mode);
extern void CdReadyCallback(void (*cb)(int, unsigned char *));

extern void *Iso_OpenPath_NoArg(void);     /* FUN_800157d4 -- arg latched globally */
extern void  Audio_RefreshKeyState(void);  /* FUN_80043a74 */

/* Globals (status bytes the rest of the binary consults). */
extern uint8_t  uRam000005ac;   /* disc state: 0=unknown 1=noCD 2=non-PSX 3=PSX-no-audio 4=PSX+audio */
extern uint8_t  uRam000008f0;   /* current track index latch */
extern uint8_t  uRam000008f8;   /* aux state used by pause/cd-cover handlers */
extern int32_t  iRam000008e0;
extern int32_t  iRam000008e8;
extern int32_t  iRam000008ec;
extern uint32_t uRam000008f4;
extern uint32_t uRam000008df;   /* play-volume / mute byte */
extern uint32_t DAT_80065be0;
extern uint32_t DAT_800a3090[];  /* track LBA table */

extern void LAB_80043c34(int, unsigned char *);  /* CD-ready CB for track-play */
extern void LAB_80043d94(int, unsigned char *);  /* CD-ready CB for XA-play   */

#define CdlNop      0x01
#define CdlPlay     0x03
#define CdlReadS    0x1b
#define CdlPause    0x09
#define CdlSetmode  0x0e
#define CdlSetfilter 0x0d
#define CdlSetloc   0x02

/* HIGH: poll CD shell open status (bit 4 of CdlNop result byte). */
uint8_t CD_IsCoverOpen(void)
{
    unsigned char st[8];
    CdControlB(CdlNop, (unsigned char *)0, st);
    return (uint8_t)((st[0] >> 4) & 1);
}

/* HIGH-MED: identify the current disc, set the disc-state global. */
uint8_t CD_DetectDiskType(void)
{
    CdDiskReady(0);
    int t = CdGetDiskType();
    if (t == 1) {
        uRam000005ac = 2;
    } else if (t == 0) {
        uRam000005ac = 1;
        uRam000008f8 = 0;
    } else if (t == 2) {
        extern int FUN_80015610(void);   /* TBD: ISO root parse */
        int hasAudio = FUN_80015610();
        uRam000005ac = (hasAudio != 0) ? 4 : 3;
    } else if (t == 0x10) {
        uRam000005ac = 0;
        uRam000008f8 = 0;
    }
    if (uRam000005ac > 1) Audio_RefreshKeyState();
    return uRam000005ac;
}

void CD_Pause(void) { CdControl(CdlPause, (unsigned char *)0, (unsigned char *)0); }
void CD_Play (void) { CdControl(CdlPlay,  (unsigned char *)0, (unsigned char *)0); }

/* HIGH-MED: queue a CD-audio track. Uses the per-disc-type table at
 * DAT_800a3090, choosing the offset by disc-state. */
void CD_PlayTrack(int trackIdx)
{
    unsigned char mode[8] = { 0x05 };   /* CdlSetmode arg: CDDA report bit */

    uRam000008f0 = (uint8_t)trackIdx;
    int idx = trackIdx + ((uRam000005ac < 3) ? 1 : 2);

    DAT_80065be0 = DAT_800a3090[idx];
    iRam000008e0 = CdPosToInt((const CdlLOC *)(((uint8_t *)DAT_800a3090) + idx * 4 + 4));
    iRam000008e0 -= 0x96;

    CdControl(CdlSetmode, mode, (unsigned char *)0);
    CdControl(CdlPlay,    (unsigned char *)&DAT_80065be0, (unsigned char *)0);
    CdReadyCallback(LAB_80043c34);
}

/* HIGH-MED: stream XA audio from an EXP/XA file located via Iso_OpenPath.
 * `channel` selects the XA filter channel. */
uint32_t Audio_PlayXa(uint32_t pathHandle, uint8_t channel)
{
    void *desc = Iso_OpenPath_NoArg();
    uRam000008f4 = 0;
    iRam000008ec = 0;
    iRam000008e8 = 0;
    if (desc == NULL) return 0;

    int      startSector = *(int *)((uint8_t *)desc + 0x0c);
    uint32_t sizeBytes   = *(uint32_t *)((uint8_t *)desc + 0x10);
    CdlLOC   pos;
    CdIntToPos(startSector, &pos);

    uRam000008df = 0xff;
    iRam000008e0 = startSector + (int)(sizeBytes >> 11) - 2;

    /* Filter (CdlSetfilter): { fileNo=1, channel }. */
    do {
        unsigned char filt[2] = { 1, channel };
        while (CdControl(CdlSetfilter, filt, (unsigned char *)0), 0) {}
        /* original retries until non-zero return */
        unsigned int  ok;
        do { ok = CdControl(CdlSetfilter, filt, (unsigned char *)0); iRam000008e8++; }
            while (ok == 0);
        do { ok = CdControl(CdlSetloc, (unsigned char *)&pos, (unsigned char *)0); iRam000008ec++; }
            while (ok == 0);
    } while (CdRead2(0xe8) == 0);

    CdReadyCallback(LAB_80043d94);
    return 1;
}
