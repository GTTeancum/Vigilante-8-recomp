/* boot_init.c -- boot-time CD/heap initialization.
 *
 * Source: SLUS_005.10  FUN_80015e8c.
 *
 * Boot sequence:
 *   1. CdInit() -- bring up the CD-ROM hardware.
 *   2. Read sector 0x10 (the ISO9660 Primary Volume Descriptor) into a
 *      2 KiB scratch buffer.
 *   3. Cache the first 32 bytes of the PVD into DAT_8006f608..624 for
 *      future "same disc?" checks (Iso_VerifyOriginalDisc).
 *   4. Pull the root-directory location and length out of the PVD
 *      (PVD offset 0x9c contains the root directory record; bytes
 *      +2..+9 are the LBA and size, both as the standard ISO9660
 *      both-endian pair -- only the LE half is consumed here).
 *   5. Allocate a 0x514-byte DirRecord and call Iso_ReadDir to walk
 *      the entire on-disc directory tree into RAM.
 *
 * After this returns, `iRam000006b4` points at the root DirRecord, and
 * Iso_OpenPath can resolve any file by name.
 *
 * The unaligned-read gymnastics in the Ghidra pseudo-C are caused by
 * the PVD record being a packed structure: the root-dir record sits at
 * a non-aligned offset inside the 2 KiB sector buffer. We expose the
 * intent (read u32 startSector / u32 sizeBytes from the root-dir
 * record) instead of the bit-shift dance.
 *
 * HIGH confidence on the high-level shape; MED on the exact PVD offsets
 * (0x9c+2 / 0x9c+10) -- the ISO9660 spec fixes them, but pass 2 should
 * cross-check against the actual sector layout dumped from disc.
 */
#include <stdint.h>

extern void  CdInit(void);
extern void *V8_CdReadSectors(void *buf, int sector, int mode);
extern void *Iso_ReadDir(void *parent, int startSector, int dirSize);
extern void *Heap_AllocOrRetry(uint32_t n);
extern void *V8_MemCopy(void *dst, const void *src, int n);

extern uintptr_t iRam000006b4;    /* root DirRecord pointer */
extern uint8_t   DAT_8006f608[32]; /* cached PVD signature for "same disc?" */

#define PVD_SECTOR              0x10
#define PVD_ROOT_DIR_RECORD_OFF 0x9c   /* 156-byte preamble per ISO9660 */

void Boot_Init(void)
{
    CdInit();

    /* Read the PVD sector into a 2 KiB stack buffer. */
    uint8_t sector[0x800] __attribute__((aligned(4)));
    V8_CdReadSectors(sector, PVD_SECTOR, 1);

    /* Cache the first 32 bytes (the volume signature: "CD001" + version
     * + system identifier prefix) for later disc-swap detection. */
    V8_MemCopy(DAT_8006f608, sector + 8, 32);

    /* Pull root LBA + size from the PVD's root dir record. The record
     * starts at sector + 0x9c; LBA(LE) is at +2, size(LE) at +10. */
    const uint8_t *rdr = sector + PVD_ROOT_DIR_RECORD_OFF;
    uint32_t rootLBA;  V8_MemCopy(&rootLBA, rdr + 2,  4);
    uint32_t rootSize; V8_MemCopy(&rootSize, rdr + 10, 4);

    void *root = Heap_AllocOrRetry(0x514);
    iRam000006b4 = (uintptr_t)Iso_ReadDir(root, (int)rootLBA, (int)rootSize);
}
