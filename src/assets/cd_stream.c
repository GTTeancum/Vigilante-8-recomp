/* cd_stream.c -- Async CD-ROM streaming reader.
 *
 * Source: SLUS_005.10
 *   FUN_8001570c  -- Stream_Open(startSector)
 *   FUN_80015a20  -- Stream_Read(dst, nBytes)
 *   FUN_80015be4  -- Stream_Tell()
 *   FUN_80015bf0  -- Stream_Seek(pos, whence)
 *   FUN_80015a00  -- Stream_Close()
 *   FUN_800159b4  -- Stream_OpenByName(name)
 *
 * The streamer keeps a 2 KiB sector buffer at @ iRam0000069c, with a
 * byte cursor at @ uRam000006ac. A `CdReadyCallback` (LAB_80015644)
 * advances the cursor as each sector lands; consumers call
 * Stream_Read to copy out bytes, which spins V8_WaitVsync until enough
 * data has arrived.
 *
 * HIGH-MED confidence: the open/read/close API is recognisable; the
 * unaligned-copy fast path in Stream_Read is preserved with all of its
 * shift/mask gymnastics (it's expressing two-byte misaligned u32 loads
 * from a buffer whose alignment depends on the on-disc sector position).
 */
#include <stdint.h>
#include <stddef.h>
#include "structs.h"

extern void CdReadyCallback(void (*cb)(int, unsigned char *));
extern void CdControl(unsigned char com, unsigned char *param, unsigned char *result);
extern void CdIntToPos(int sector, CdlLOC *pos);

extern void *Heap_AllocOrRetry2(uint32_t n, uint32_t mode);  /* FUN_8001178c -- ring-buffer alloc */
extern int32_t V8_WaitVsync(void);                           /* FUN_800156d4 */
extern void *V8_MemCopy(void *dst, const void *src, int n);

extern void Stream_CdCallback(int, unsigned char *);          /* LAB_80015644 */

/* State globals @ 0x800006a0..0x800006b0 */
extern int32_t    iRam000006b0;        /* requested start sector */
extern uint32_t   uRam000006a0;        /* sector buffer base */
extern uint32_t   uRam000006a4;        /* aligned cursor (latched) */
extern uint32_t   uRam000006a8;        /* aligned cursor (IRQ-write) */
extern uint32_t   uRam000006ac;        /* byte cursor in stream */
extern uint32_t   iRam0000069c;        /* current sector head */

/* HIGH: install the CD callback and prime the read at `sector`. */
void Stream_Open(int sector)
{
    /* Original sets the file mode struct on the stack to {0x80, ?, ?, ?}.
     * 0x80 is the standard PSY-Q CD mode byte (double-speed mode 2 form 1). */
    unsigned char mode[4] = { 0x80, 0, 0, 0 };
    CdlLOC pos;

    iRam000006b0 = sector;
    CdReadyCallback(Stream_CdCallback);
    uRam000006a0 = (uint32_t)Heap_AllocOrRetry2(0x800, 2);
    uRam000006a4 = uRam000006a0;
    uRam000006a8 = uRam000006a0;

    CdControl(0x0e, mode, NULL);                  /* CdlSetmode */
    CdIntToPos(iRam000006b0, &pos);
    CdControl(0x06, (unsigned char *)&pos, NULL); /* CdlReadN */
}

/* HIGH: re-open by name (uses the descriptor cache rather than a new sector). */
/* PSX calling conv: a0 (path) is unchanged at the jal to FUN_800157d4; pass explicitly. */
extern void *FUN_800157d4(const uint8_t *path);   /* Iso_OpenPath */
extern void *FUN_80015368(uint32_t);    /* failure-path infinite-loop trap */

uint32_t Stream_OpenByName(const char *path)
{
    void *desc = FUN_800157d4((const uint8_t *)path);
    if (desc == NULL) {
        FUN_80015368((uint32_t)(uintptr_t)path);
        return 0;
    }
    /* Reuse the streaming opener with the descriptor's start sector. */
    Stream_Open(*(int32_t *)((uint8_t *)desc + 0x0c));
    uRam000006ac = 0;
    return 1;
}

/* HIGH-MED: read `nBytes` from the stream into `dst`. Falls into a
 * 16-byte unrolled fast path when both pointers are u32-aligned.
 *
 * The misaligned path is preserved verbatim from the original because
 * its shift constants encode the buffer's mod-4 offset at the start of
 * each sector, which propagates through the sector boundary in a way
 * that ordinary memcpy would not reproduce bit-exactly.
 */
uint32_t Stream_Read(void *dst, uint32_t nBytes)
{
    uint8_t *out = (uint8_t *)dst;

    /* If we're mid-sector, drain the partial sector first. */
    uint32_t inSector = uRam000006ac & 0x7ffu;
    if (inSector != 0) {
        uint32_t avail = 0x800u - inSector;
        uint32_t take  = (nBytes < avail) ? nBytes : avail;
        V8_MemCopy(out, (const void *)(iRam0000069c + inSector), (int)take);
        out          += take;
        nBytes       -= take;
        uRam000006ac += take;
    }

    /* Whole-sector loop. */
    while (nBytes > 0x7ffu) {
        const uint8_t *src = (const uint8_t *)V8_WaitVsync();   /* returns sector ptr */
        V8_MemCopy(out, src, 0x800);
        out          += 0x800;
        nBytes       -= 0x800;
        uRam000006ac += 0x800;
    }

    /* Tail copy from the next sector. */
    if (nBytes != 0) {
        iRam0000069c = (uint32_t)V8_WaitVsync();
        V8_MemCopy(out, (const void *)iRam0000069c, (int)nBytes);
        uRam000006ac += nBytes;
    }
    return 1;
}

/* HIGH: trivial accessor / setter. */
uint32_t Stream_Tell(void)
{
    return uRam000006ac;
}

void Stream_Seek(uint32_t pos, int relative)
{
    if (relative) pos += uRam000006ac;
    int32_t sectorsAhead = ((int32_t)pos >> 11) - ((int32_t)uRam000006ac >> 11);
    while (sectorsAhead-- > 0) {
        iRam0000069c = (uint32_t)V8_WaitVsync();
    }
    uRam000006ac = pos;
}

/* ================================================================
 * FUN_80015798 -- Stream_Stop
 *
 * Sends CdlPause (0x09), clears the ready callback, frees the sector
 * buffer.  Called by Stream_Close.
 * HIGH confidence (direct Ghidra port).
 * ================================================================ */
extern void Heap_Free(void *p);    /* FUN_80045088 */
void FUN_80015798(void)
{
    CdControl(0x09, NULL, NULL);
    CdReadyCallback((void (*)(int, unsigned char *))0);
    Heap_Free((void *)(uintptr_t)uRam000006a0);
}

/* ================================================================
 * FUN_80015a00 -- Stream_Close
 * ================================================================ */
uint32_t FUN_80015a00(void)
{
    FUN_80015798();
    return 1;
}

/* Hex-name aliases for Stream_Seek, Stream_OpenByName, Stream_Read. */
void FUN_80015bf0(uint32_t pos, int relative) { Stream_Seek(pos, relative); }
uint32_t FUN_800159b4(uint32_t path) { return Stream_OpenByName((const char *)(uintptr_t)path); }
uint32_t FUN_80015a20(void *dst, uint32_t nBytes) { return Stream_Read(dst, nBytes); }

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001570c  (from analysis/SLUS_005.10/decomp/8001570c.c) --- */
// addr: 0x8001570c  name: FUN_8001570c

void FUN_8001570c(int param_1)

{
  u_char *puVar1;
  uint uVar2;
  CdlLOC aCStack_18 [2];
  u_char auStack_10 [3];
  uint uStack_d;
  
  puVar1 = auStack_10 + 3;
  uVar2 = (uint)puVar1 & 3;
  *(uint *)(puVar1 + -uVar2) =
       *(uint *)(puVar1 + -uVar2) & -1 << (uVar2 + 1) * 8 | 0x80U >> (3 - uVar2) * 8;
  _auStack_10 = 0x80;
  iRam000006b0 = param_1;
  CdReadyCallback(FUN_80015644);
  uRam000006a0 = FUN_8001178c(0x800,2);
  uRam000006a4 = uRam000006a0;
  uRam000006a8 = uRam000006a0;
  CdControl('\x0e',auStack_10,(u_char *)0x0);
  CdIntToPos(iRam000006b0,aCStack_18);
  CdControl('\x06',&aCStack_18[0].minute,(u_char *)0x0);
  return;
}

/* --- SLUS_005.10 FUN_80015a20  (from analysis/SLUS_005.10/decomp/80015a20.c) --- */
// addr: 0x80015a20  name: FUN_80015a20

undefined4 FUN_80015a20(uint *param_1,uint param_2,undefined4 param_3,uint param_4)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  uint *puVar5;
  uint in_t0;
  uint in_t1;
  uint in_t2;
  uint uVar6;
  
  if ((uRam000006ac & 0x7ff) != 0) {
    uVar6 = -uRam000006ac & 0x7ff;
    if ((int)param_2 < (int)(-uRam000006ac & 0x7ff)) {
      uVar6 = param_2;
    }
    FUN_80044c44(param_1,iRam0000069c + (uRam000006ac & 0x7ff),uVar6);
    param_1 = (uint *)((int)param_1 + uVar6);
    param_2 = param_2 - uVar6;
    uRam000006ac = uRam000006ac + uVar6;
  }
  for (; 0x7ff < (int)param_2; param_2 = param_2 - 0x800) {
    puVar3 = (uint *)FUN_800156d4();
    if ((((uint)puVar3 | (uint)param_1) & 3) == 0) {
      puVar4 = puVar3 + 0x200;
      puVar5 = param_1;
      do {
        param_4 = *puVar3;
        in_t0 = puVar3[1];
        in_t1 = puVar3[2];
        in_t2 = puVar3[3];
        *puVar5 = param_4;
        puVar5[1] = in_t0;
        puVar5[2] = in_t1;
        puVar5[3] = in_t2;
        puVar3 = puVar3 + 4;
        puVar5 = puVar5 + 4;
      } while (puVar3 != puVar4);
    }
    else {
      puVar4 = puVar3 + 0x200;
      puVar5 = param_1;
      do {
        uVar6 = (int)puVar3 + 3U & 3;
        uVar1 = (uint)puVar3 & 3;
        param_4 = (*(int *)(((int)puVar3 + 3U) - uVar6) << (3 - uVar6) * 8 |
                  param_4 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                  *(uint *)((int)puVar3 - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 7U & 3;
        uVar1 = (uint)(puVar3 + 1) & 3;
        in_t0 = (*(int *)(((int)puVar3 + 7U) - uVar6) << (3 - uVar6) * 8 |
                in_t0 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 1) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 0xbU & 3;
        uVar1 = (uint)(puVar3 + 2) & 3;
        in_t1 = (*(int *)(((int)puVar3 + 0xbU) - uVar6) << (3 - uVar6) * 8 |
                in_t1 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 2) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 0xfU & 3;
        uVar1 = (uint)(puVar3 + 3) & 3;
        in_t2 = (*(int *)(((int)puVar3 + 0xfU) - uVar6) << (3 - uVar6) * 8 |
                in_t2 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 3) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar5 + 3U & 3;
        puVar2 = (uint *)(((int)puVar5 + 3U) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | param_4 >> (3 - uVar6) * 8;
        uVar6 = (uint)puVar5 & 3;
        *(uint *)((int)puVar5 - uVar6) =
             *(uint *)((int)puVar5 - uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | param_4 << uVar6 * 8;
        uVar6 = (int)puVar5 + 7U & 3;
        puVar2 = (uint *)(((int)puVar5 + 7U) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t0 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 1) & 3;
        puVar2 = (uint *)((int)(puVar5 + 1) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t0 << uVar6 * 8;
        uVar6 = (int)puVar5 + 0xbU & 3;
        puVar2 = (uint *)(((int)puVar5 + 0xbU) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t1 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 2) & 3;
        puVar2 = (uint *)((int)(puVar5 + 2) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t1 << uVar6 * 8;
        uVar6 = (int)puVar5 + 0xfU & 3;
        puVar2 = (uint *)(((int)puVar5 + 0xfU) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t2 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 3) & 3;
        puVar2 = (uint *)((int)(puVar5 + 3) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t2 << uVar6 * 8;
        puVar3 = puVar3 + 4;
        puVar5 = puVar5 + 4;
      } while (puVar3 != puVar4);
    }
    param_1 = param_1 + 0x200;
    uRam000006ac = uRam000006ac + 0x800;
  }
  if (param_2 != 0) {
    iRam0000069c = FUN_800156d4();
    FUN_80044c44(param_1,iRam0000069c,param_2);
    uRam000006ac = uRam000006ac + param_2;
  }
  return 1;
}

/* --- SLUS_005.10 FUN_80015be4  (from analysis/SLUS_005.10/decomp/80015be4.c) --- */
// addr: 0x80015be4  name: FUN_80015be4

undefined4 FUN_80015be4(void)

{
  return uRam000006ac;
}

/* --- SLUS_005.10 FUN_80015bf0  (from analysis/SLUS_005.10/decomp/80015bf0.c) --- */
// addr: 0x80015bf0  name: FUN_80015bf0

void FUN_80015bf0(uint param_1,int param_2)

{
  int iVar1;
  
  if (param_2 != 0) {
    param_1 = param_1 + uRam000006ac;
  }
  iVar1 = (param_1 >> 0xb) - ((int)uRam000006ac >> 0xb);
  while (iVar1 = iVar1 + -1, iVar1 != -1) {
    uRam0000069c = FUN_800156d4();
  }
  uRam000006ac = param_1;
  return;
}

/* --- SLUS_005.10 FUN_80015a00  (from analysis/SLUS_005.10/decomp/80015a00.c) --- */
// addr: 0x80015a00  name: FUN_80015a00

undefined4 FUN_80015a00(void)

{
  FUN_80015798();
  return 1;
}

/* --- SLUS_005.10 FUN_800159b4  (from analysis/SLUS_005.10/decomp/800159b4.c) --- */
// addr: 0x800159b4  name: FUN_800159b4

undefined4 FUN_800159b4(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_800157d4();
  if (iVar1 == 0) {
    uVar2 = FUN_80015368(param_1);
  }
  else {
    FUN_8001570c(*(undefined4 *)(iVar1 + 0xc));
    uRam000006ac = 0;
    uVar2 = 1;
  }
  return uVar2;
}

/* --- SLUS_005.10 FUN_8001178c  (from analysis/SLUS_005.10/decomp/8001178c.c) --- */
// addr: 0x8001178c  name: FUN_8001178c

int FUN_8001178c(int param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_800451c0();
  if ((iVar1 == 0) && (param_1 != 0)) {
    DrawSync(0);
    FUN_80011914(1 - iRam00000004);
    FUN_800165cc(1);
    do {
      iVar1 = FUN_800451c0(param_1,param_2);
      if (iVar1 != 0) {
        return iVar1;
      }
      iVar1 = FUN_80020d3c();
    } while (iVar1 != 0);
    _boot();
    iVar1 = 0;
  }
  return iVar1;
}

/* --- SLUS_005.10 FUN_800156d4  (from analysis/SLUS_005.10/decomp/800156d4.c) --- */
// addr: 0x800156d4  name: FUN_800156d4

int FUN_800156d4(void)

{
  int iVar1;
  
  iVar1 = iRam000006a4;
  do {
  } while (iRam000006a8 == iRam000006a4);
  iRam000006a4 = iRam000006a8;
  return iVar1;
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

/* --- SLUS_005.10 FUN_80015368  (from analysis/SLUS_005.10/decomp/80015368.c) --- */
// addr: 0x80015368  name: FUN_80015368

void FUN_80015368(undefined4 param_1)

{
  FUN_80015164(0);
  FUN_80015288(param_1);
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}

#endif  /* GHIDRA REF */
