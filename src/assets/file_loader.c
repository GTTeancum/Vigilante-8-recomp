/* file_loader.c -- top-level file loader: locate by name, read into heap.
 *
 * Source: SLUS_005.10
 *   FUN_80015948  -- Asset_LoadFile(no-arg form; uses last-opened name)
 *   FUN_800116f4  -- Heap_AllocOrRetry  (malloc + OOM recovery)
 *
 * The descriptor returned by Asset_OpenFile (FUN_800157d4 -- not yet
 * decomp'd, lives in src/assets/auto/) carries the on-disc start sector
 * at +0xc and the byte length at +0x10. Asset_LoadFile rounds the byte
 * length up to a multiple of 0x800 (the PSX CD sector size), allocates
 * a buffer of that size, reads the whole file in one CdRead call, then
 * realloc-shrinks the buffer to the true length to free the read pad.
 *
 * HIGH confidence: clean two-step pattern (alloc + read + trim) matches
 * standard PSY-Q sample code.
 */
#include <stdint.h>
#include "structs.h"

extern void *Asset_OpenFile(void);                        /* FUN_800157d4 */
extern void *Heap_AllocOrRetry(uint32_t n);               /* FUN_800116f4 */
extern void *Heap_Realloc(void *p, uint32_t n);
extern void *V8_CdReadSectors(void *buf, int sector, int mode);

void *Asset_LoadFile(void)
{
    /* Descriptor: { ..., u32 startSector @+0xc, u32 fileSize @+0x10, ... }. */
    uint8_t *desc = (uint8_t *)Asset_OpenFile();
    if (desc == NULL) return NULL;

    uint32_t startSector = *(uint32_t *)(desc + 0x0c);
    uint32_t fileSize    = *(uint32_t *)(desc + 0x10);

    uint32_t paddedBytes = (fileSize + 0x7ffu) & 0xfffff800u;
    uint32_t nSectors    = (fileSize + 0x7ffu) >> 11;

    void *buf = Heap_AllocOrRetry(paddedBytes);
    V8_CdReadSectors(buf, (int)startSector, (int)nSectors);
    return Heap_Realloc(buf, fileSize);
}
