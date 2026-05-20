/* iso9660.c -- in-memory ISO9660 directory cache for asset lookup.
 *
 * Source: SLUS_005.10
 *   FUN_80015c68 -- Iso_ReadDir   reads a CD directory recursively.
 *   FUN_800157d4 -- Iso_OpenPath  resolves "Sub\Path\NAME.EXT" to a
 *                                 cached 0x14-byte file record.
 *
 * The cache lives in a chain of structs allocated on the heap. Each
 * record produced by Iso_ReadDir is one of:
 *
 *   FileRecord (0x14 bytes, packed in arrays of size *(int *)(parent+0x10)):
 *     0x00  char  name[12]       space-padded 8.3-style
 *     0x0c  u32   startSector    LBA on disc
 *     0x10  u32   sizeBytes      raw file size
 *   DirRecord (heap-allocated 0x28+ bytes):
 *     0x00  char  name[8]        space-padded
 *     0x08  ptr   firstChildDir  (DirRecord chain via +0xc next pointer)
 *     0x0c  ptr   nextSibling
 *     0x10  u32   nFiles         count of FileRecord children
 *     0x14  FileRecord  files[nFiles]
 *     0x20  u32   startSector    of this subdirectory on disc
 *     0x24  u32   sizeBytes
 *
 * Iso_ReadDir streams sectors from the disc (Stream_Open), parses
 * ISO9660 directory records, and:
 *   - File entries (flags-byte @ +0x19 has bit 1 clear, name-length @
 *     +0x20 >= 1, and the first name char @ +0x21 != ';') become inline
 *     entries inside the parent's `files[]` array.
 *   - Directory entries (flags bit 1 set, name length >= 2 -- skips
 *     "." and "..") become heap-allocated child DirRecords, recursing.
 *
 * Each ISO9660 directory record begins with its length byte at +0x00;
 * the parser advances by that length to the next record. Sector
 * crossings are not explicit -- the streamer feeds 2 KiB at a time and
 * the parser walks past sector boundaries naturally because directory
 * records do not span sectors per the ISO9660 spec.
 *
 * After the directory's full size has been consumed, the parent record
 * is realloc-trimmed to its final compact size (0x14 + nFiles * 0x14)
 * and each child DirRecord is recursively read.
 *
 * Iso_OpenPath walks a backslash-separated path against this in-memory
 * tree. Name compare uses Util_MemEq (a thin 12-byte memcmp wrapper).
 * Names are case-folded to uppercase on input (`byte - 0x20` if byte >= 'a').
 *
 * HIGH confidence: the directory entry offsets match ISO9660 exactly:
 *   - record length    @ 0x00
 *   - flags            @ 0x19  (bit 1 = directory)
 *   - name length      @ 0x20
 *   - name             @ 0x21
 *   - extent LBA (LE)  @ 0x02
 *   - data length (LE) @ 0x0a
 */
#include <stdint.h>
#include <stddef.h>

extern void  Stream_Open(int sector);
extern void  Stream_Close(void);
extern int32_t V8_WaitVsync(void);
extern void *Heap_Realloc(void *p, uint32_t n);
extern void *Heap_AllocOrRetry(uint32_t n);
extern void *V8_MemSet(void *dst, int v, int n);
extern void *V8_MemCopy(void *dst, const void *src, int n);

#define ISO_FLAG_DIR  0x02

#define DR_NEXT_SIB(p)   (*(void **)((uint8_t *)(p) + 0x0c))
#define DR_FIRST_CHILD(p) (*(void **)((uint8_t *)(p) + 0x08))
#define DR_NFILES(p)     (*(int  *)((uint8_t *)(p) + 0x10))
#define DR_FILES(p)      ((uint8_t *)(p) + 0x14)

/* HIGH-MED: read a single directory from disc into `parent`, then
 * recurse into each child. */
void *Iso_ReadDir(void *parent, int startSector, int dirSize)
{
    Stream_Open(startSector);
    DR_NFILES(parent)      = 0;
    DR_FIRST_CHILD(parent) = NULL;

    while (dirSize > 0) {
        uint8_t *rec = (uint8_t *)V8_WaitVsync();
        while (rec[0] != 0) {     /* end-of-records marker is a zero length */
            uint32_t flags = rec[0x19];
            uint32_t nameLen = rec[0x20];

            if ((flags & ISO_FLAG_DIR) == 0) {
                /* File. Inline into parent's files[] array. */
                uint8_t *fr = DR_FILES(parent) + DR_NFILES(parent) * 0x14;
                DR_NFILES(parent)++;
                uint32_t i;
                for (i = 0; i < 12; i++) {
                    uint8_t c = rec[0x21 + i];
                    if (c == ';' || i >= nameLen) break;
                    fr[i] = c;
                }
                for (; i < 12; i++) fr[i] = 0x20;          /* space-pad */

                /* Misaligned u32 read of the LSB-first LBA at rec+2 and
                 * size at rec+10. The original Ghidra output expresses
                 * this via shift/mask; we use V8_MemCopy here because
                 * the underlying buffer alignment is already known. */
                V8_MemCopy(fr + 0x0c, rec + 2,  4);
                V8_MemCopy(fr + 0x10, rec + 10, 4);
            } else if (nameLen > 1) {
                /* Subdirectory, but skip "." (nameLen==1) and ".." (==1). */
                void *child = Heap_AllocOrRetry(0x514);
                V8_MemSet(child, 0x20, 8);                  /* name field: 8 spaces */
                uint32_t copy = (nameLen < 8) ? nameLen : 8;
                V8_MemCopy(child, rec + 0x21, (int)copy);
                V8_MemCopy((uint8_t *)child + 0x20, rec + 2,  4);  /* startSector */
                V8_MemCopy((uint8_t *)child + 0x24, rec + 10, 4);  /* sizeBytes  */
                DR_NEXT_SIB(child) = DR_FIRST_CHILD(parent);
                DR_FIRST_CHILD(parent) = child;
            }
            rec += rec[0];
        }
        dirSize -= 0x800;
    }

    Stream_Close();
    void *trimmed = Heap_Realloc(parent, (uint32_t)(DR_NFILES(parent) * 0x14 + 0x14));
    /* Recurse into each child dir to populate its files / grandchildren. */
    for (void *c = DR_FIRST_CHILD(trimmed); c != NULL; c = DR_NEXT_SIB(c)) {
        Iso_ReadDir(c,
                    *(int *)((uint8_t *)c + 0x20),
                    *(int *)((uint8_t *)c + 0x24));
    }
    return trimmed;
}

/* Util_MemEq: 12/8-byte fixed-length compare used by Iso_OpenPath. */
extern int Util_MemEq(const void *a, const void *b, int n);   /* FUN_800523a0 */

extern void *iRam000006b4;   /* root DirRecord; set by the bootstrap directory load */

/* HIGH: resolve a backslash-separated path against the cached tree.
 * Names are compared case-folded to uppercase, space-padded to 12 chars
 * (files) or 8 chars (subdirectories). Returns a pointer to the matched
 * file's 0x14-byte FileRecord, or NULL.
 */
void *Iso_OpenPath(const uint8_t *path)
{
    if (*path == '\\') path++;
    if (iRam000006b4 == NULL) return NULL;

    void *dir = iRam000006b4;
    for (;;) {
        uint8_t name[16];
        int     n = 0;
        uint8_t  c;
        do {
            c = *path++;
            if (c >= 0x61 && c <= 0x7a) c -= 0x20;     /* a..z -> A..Z */
            if (c != 0 && c != '\\') {
                if (n < 12) {
                    name[n++] = c;
                    continue;
                }
            }
            for (; n < 12; n++) name[n] = ' ';
            break;
        } while (1);

        if (c != '\\') {
            /* File lookup. */
            int nFiles = DR_NFILES(dir);
            if (nFiles <= 0) return NULL;
            uint8_t *fr = DR_FILES(dir);
            for (int i = 0; i < nFiles; i++, fr += 0x14) {
                if (Util_MemEq(fr, name, 12) == 0) return fr;
            }
            return NULL;
        }

        /* Subdirectory lookup (8-byte name). */
        void *child = DR_FIRST_CHILD(dir);
        while (child != NULL) {
            if (Util_MemEq(child, name, 8) == 0) break;
            child = DR_NEXT_SIB(child);
        }
        if (child == NULL) return NULL;
        dir = child;
    }
}
