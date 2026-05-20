/* iff_chunk_data.c -- read an IFF chunk's payload into a heap buffer.
 *
 * Source: SLUS_005.10  FUN_800225d4.
 *
 * Reads the next IFF chunk header via Iff_ReadChunkHeader.  If it was
 * a FORM (sentinel 0xffffffff) returns NULL -- forms have no inline
 * payload, the caller is expected to recurse into the sub-chunks.
 * Otherwise allocates a buffer (max(size, 4) bytes -- the 4-byte floor
 * is there so a zero-size chunk still produces a non-NULL handle that
 * Heap_Free can release without specialcasing) and streams the payload.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern uint32_t Iff_ReadChunkHeader(uint32_t *hdrOut, uint32_t *parentRemaining);
extern void    *Heap_AllocOrRetry(uint32_t n);
extern void     Stream_Read(void *dst, uint32_t nBytes);

void *Iff_ReadChunkData(uint32_t *parentRemaining)
{
    uint32_t hdr[2];
    uint32_t sz = Iff_ReadChunkHeader(hdr, parentRemaining);
    if (sz == 0xffffffffu) return NULL;     /* it was a FORM; no payload */

    uint32_t alloc = (sz != 0) ? sz : 4;
    void    *buf   = Heap_AllocOrRetry(alloc);
    Stream_Read(buf, sz);
    return buf;
}
