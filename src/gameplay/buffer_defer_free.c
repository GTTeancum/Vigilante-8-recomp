/* buffer_defer_free.c -- enqueue a pointer for free at end-of-frame.
 *
 * Source: SLUS_005.10  FUN_800118b4.
 *
 * Allocates a 12-byte deferred-free node, links it onto the tail of
 * the per-parity deferred-free chain (head/sentinel/tail triple at
 * DAT_8006eca0[parity * 3]), then stores `payload` in the new node's
 * payload slot. The chain is drained by Buffer_FlushDeferredFree.
 *
 * Used during draw to free heap memory that the renderer is still
 * reading; the actual free happens after the renderer flips parity.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void *Heap_AllocOrRetry(uint32_t n);   /* FUN_800116f4 */
extern int32_t iRam00000004;                   /* g_dispBufIndex */
extern void   *DAT_8006eca0;                   /* deferred-free anchor (3-tuple x 2 parities) */
extern void   *DAT_8006eca4;
extern void   *DAT_8006eca8;

void Buffer_DeferFree(void *payload)
{
    /* node[0]=next sentinel, node[1]=back-link, node[2]=payload */
    uint32_t *node = (uint32_t *)Heap_AllocOrRetry(0xc);
    int parity = (int)iRam00000004;
    node[2] = (uintptr_t)payload;

    uint32_t **anchorBase = (uint32_t **)&DAT_8006eca0;
    uint32_t *prevTail = anchorBase[parity * 3 + 2];   /* tail */
    anchorBase[parity * 3 + 2] = node;
    *prevTail = (uintptr_t)node;
    node[1] = (uintptr_t)prevTail;
    node[0] = (uintptr_t)&((uint32_t **)anchorBase)[parity * 3 + 1];   /* sentinel */
}
