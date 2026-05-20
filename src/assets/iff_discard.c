/* iff_discard.c -- walk a FORM body, freeing every chunk's payload.
 *
 * Source: LOAD.DLL  FUN_80100e98.
 *
 * Counterpart to XOBF_Parse: when the loader doesn't care about the
 * contents of a FORM (e.g. skipping an embedded video chunk), it walks
 * the same iteration but Heap_Free's each payload and continues. The
 * stream cursor advances naturally because Iff_ReadChunkData already
 * consumes the payload bytes.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void *Iff_ReadChunkData(uint32_t *hdrOut, uint32_t *parentRemaining);
extern void  Heap_Free(void *p);

void Iff_DiscardChunks(uint32_t remaining)
{
    uint32_t hdr[2];
    while (remaining != 0) {
        void *p = Iff_ReadChunkData(hdr, &remaining);
        Heap_Free(p);
    }
}
