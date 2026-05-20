/* heap_retry2.c -- calloc-with-OOM-recovery.
 *
 * Source: SLUS_005.10  FUN_8001178c.
 *
 * Parallel to Heap_AllocOrRetry, but using Heap_Calloc as the underlying
 * allocator. Used by Stream_Open() for the 2 KiB sector ring buffer.
 *
 * HIGH confidence: identical control flow to Heap_AllocOrRetry, just
 * with Heap_Calloc(count, size) instead of Heap_Alloc(n).
 */
#include <stdint.h>

extern void *Heap_Calloc(int count, int size);
extern void  DrawSync(int mode);
extern void  Buffer_FlushDeferredFree(int parity);
extern void  Async_StopAllocCallback(int unused);
extern int   Asset_VisibilityEvictTick(void);   /* FUN_80020d3c */
extern void  _boot(void);
extern int32_t g_dispBufIndex;

void *Heap_CallocOrRetry(int count, int size)
{
    void *p = Heap_Calloc(count, size);
    if (p != NULL || count == 0) return p;

    DrawSync(0);
    Buffer_FlushDeferredFree(1 - g_dispBufIndex);
    Async_StopAllocCallback(1);

    for (;;) {
        p = Heap_Calloc(count, size);
        if (p != NULL) return p;
        if (Asset_VisibilityEvictTick() == 0) break;
    }
    _boot();
    return NULL;
}
