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
extern void *FUN_800157d4(void);
extern void *FUN_80015368(uint32_t);    /* failure-path infinite-loop trap */

uint32_t Stream_OpenByName(uint32_t pathHandle)
{
    void *desc = FUN_800157d4();
    if (desc == NULL) {
        FUN_80015368(pathHandle);
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
