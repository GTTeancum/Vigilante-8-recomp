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

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80043aec  (from analysis/SLUS_005.10/decomp/80043aec.c) --- */
// addr: 0x80043aec  name: FUN_80043aec

uint FUN_80043aec(void)

{
  int iVar1;
  
  CdDiskReady(0);
  iVar1 = CdGetDiskType();
  if (iVar1 == 1) {
    uRam000005ac = 2;
  }
  else if (iVar1 < 2) {
    if (iVar1 == 0) {
      uRam000005ac = 1;
      uRam000008f8 = 0;
    }
  }
  else if (iVar1 == 2) {
    iVar1 = FUN_80015610();
    uRam000005ac = 3;
    if (iVar1 != 0) {
      uRam000005ac = 4;
    }
  }
  else if (iVar1 == 0x10) {
    uRam000005ac = 0;
    uRam000008f8 = 0;
  }
  if (1 < uRam000005ac) {
    FUN_80043a74();
  }
  return uRam000005ac;
}

/* --- SLUS_005.10 FUN_80043bb4  (from analysis/SLUS_005.10/decomp/80043bb4.c) --- */
// addr: 0x80043bb4  name: FUN_80043bb4

byte FUN_80043bb4(void)

{
  byte local_10 [8];
  
  CdControlB('\x01',(u_char *)0x0,local_10);
  return local_10[0] >> 4 & 1;
}

/* --- SLUS_005.10 FUN_80043be4  (from analysis/SLUS_005.10/decomp/80043be4.c) --- */
// addr: 0x80043be4  name: FUN_80043be4

void FUN_80043be4(void)

{
  CdControl('\t',(u_char *)0x0,(u_char *)0x0);
  return;
}

/* --- SLUS_005.10 FUN_80043c0c  (from analysis/SLUS_005.10/decomp/80043c0c.c) --- */
// addr: 0x80043c0c  name: FUN_80043c0c

void FUN_80043c0c(void)

{
  CdControl('\x03',(u_char *)0x0,(u_char *)0x0);
  return;
}

/* --- SLUS_005.10 FUN_80043ce0  (from analysis/SLUS_005.10/decomp/80043ce0.c) --- */
// addr: 0x80043ce0  name: FUN_80043ce0

void FUN_80043ce0(int param_1)

{
  u_char local_10 [8];
  
  local_10[0] = '\x05';
  uRam000008f0 = (undefined1)param_1;
  if (uRam000005ac < 3) {
    param_1 = param_1 + 1;
  }
  else {
    param_1 = param_1 + 2;
  }
  DAT_80065be0 = *(undefined4 *)(&DAT_800a3090 + param_1 * 4);
  iRam000008e0 = CdPosToInt((CdlLOC *)(&UNK_800a3094 + param_1 * 4));
  iRam000008e0 = iRam000008e0 + -0x96;
  CdControl('\x0e',local_10,(u_char *)0x0);
  CdControl('\x03',(u_char *)&DAT_80065be0,(u_char *)0x0);
  CdReadyCallback(FUN_80043c34);
  return;
}

/* --- SLUS_005.10 FUN_80043df8  (from analysis/SLUS_005.10/decomp/80043df8.c) --- */
// addr: 0x80043df8  name: FUN_80043df8

undefined4 FUN_80043df8(undefined4 param_1,undefined1 param_2)

{
  int iVar1;
  undefined4 uVar2;
  CdlLOC aCStack_20 [2];
  u_char local_18;
  undefined1 local_17;
  
  iVar1 = FUN_800157d4();
  uRam000008f4 = 0;
  iRam000008ec = 0;
  iRam000008e8 = 0;
  uVar2 = 0;
  if (iVar1 != 0) {
    CdIntToPos(*(int *)(iVar1 + 0xc),aCStack_20);
    uRam000008df = 0xff;
    iRam000008e0 = *(int *)(iVar1 + 0xc) + (*(uint *)(iVar1 + 0x10) >> 0xb) + -2;
    do {
      local_18 = '\x01';
      local_17 = param_2;
      while (iVar1 = CdControl('\r',&local_18,(u_char *)0x0), iVar1 == 0) {
        iRam000008e8 = iRam000008e8 + 1;
      }
      while (iVar1 = CdControl('\x02',&aCStack_20[0].minute,(u_char *)0x0), iVar1 == 0) {
        iRam000008ec = iRam000008ec + 1;
      }
      iVar1 = CdRead2(0xe8);
    } while (iVar1 == 0);
    CdReadyCallback(FUN_80043d94);
    uVar2 = 1;
  }
  return uVar2;
}

/* --- SLUS_005.10 FUN_800157d4  (from analysis/SLUS_005.10/decomp/800157d4.c) --- */
// addr: 0x800157d4  name: FUN_800157d4

int FUN_800157d4(byte *param_1)

{
  int iVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  byte local_28 [16];
  
  if (*param_1 == 0x5c) {
    param_1 = param_1 + 1;
  }
  if (iRam000006b4 == 0) {
    return 0;
  }
  iVar3 = 0;
  iVar5 = iRam000006b4;
LAB_8001581c:
  do {
    bVar2 = *param_1;
    param_1 = param_1 + 1;
    if (0x60 < bVar2) {
      bVar2 = bVar2 - 0x20;
    }
    if ((bVar2 != 0) && (bVar2 != 0x5c)) {
      local_28[iVar3] = bVar2;
      iVar3 = iVar3 + 1;
      if (iVar3 < 0xc) goto LAB_8001581c;
    }
    for (; iVar3 < 0xc; iVar3 = iVar3 + 1) {
      local_28[iVar3] = 0x20;
    }
    if (bVar2 != 0x5c) {
      iVar3 = 0;
      if (*(int *)(iVar5 + 0x10) < 1) {
        return 0;
      }
      iVar4 = 0x14;
      do {
        iVar1 = thunk_FUN_800523a0(iVar5 + iVar4,local_28,0xc);
        iVar3 = iVar3 + 1;
        if (iVar1 == 0) {
          return iVar5 + iVar4;
        }
        iVar4 = iVar4 + 0x14;
      } while (iVar3 < *(int *)(iVar5 + 0x10));
      return 0;
    }
    iVar5 = *(int *)(iVar5 + 8);
    while( true ) {
      if (iVar5 == 0) {
        return 0;
      }
      iVar3 = thunk_FUN_800523a0(iVar5,local_28,8);
      if (iVar3 == 0) break;
      iVar5 = *(int *)(iVar5 + 0xc);
    }
    iVar3 = 0;
    if (iVar5 == 0) {
      return 0;
    }
  } while( true );
}

/* --- SLUS_005.10 FUN_80043a74  (from analysis/SLUS_005.10/decomp/80043a74.c) --- */
// addr: 0x80043a74  name: FUN_80043a74

void FUN_80043a74(void)

{
  uint uVar1;
  int iVar2;
  
  do {
    uVar1 = CdGetToc((CdlLOC *)&DAT_800a3090);
    bRam000008f8 = (byte)uVar1;
  } while ((uVar1 & 0xff) == 0);
  iVar2 = CdPosToInt((CdlLOC *)&DAT_800a3090);
  CdIntToPos(iVar2 + -0x96,(CdlLOC *)(&UNK_800a3094 + (uint)bRam000008f8 * 4));
  bRam000008f8 = bRam000008f8 - (2 < uRam000005ac);
  return;
}

/* --- SLUS_005.10 FUN_80015610  (from analysis/SLUS_005.10/decomp/80015610.c) --- */
// addr: 0x80015610  name: FUN_80015610

bool FUN_80015610(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 auStack_28 [32];
  
  uVar1 = FUN_8001555c(auStack_28);
  iVar2 = thunk_FUN_800523a0(uVar1,&DAT_8006f608,0x20);
  return iVar2 == 0;
}

#endif  /* GHIDRA REF */
