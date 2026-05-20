/* heap_retry.c -- malloc wrapper with one-shot recovery.
 *
 * Source: SLUS_005.10  FUN_800116f4.
 *
 * Tries Heap_Alloc(n). On OOM (returns NULL) when the request was
 * non-zero, the routine:
 *   1. Drains the GPU draw queue (DrawSync 0).
 *   2. Flips the front/back buffer parity and runs the deferred-free
 *      list for the now-idle buffer (Buffer_FlushDeferredFree).
 *   3. Disarms an existing VSync-callback-driven allocator
 *      (Async_StopAllocCallback).
 *   4. Retries Heap_Alloc(n) inside a loop that, while still failing,
 *      drives the dynamic asset eviction tick (Asset_EvictTick).
 *   5. On final failure, calls _boot() -- which the binary aliases to
 *      the PSY-Q `_boot` ROM entry, i.e. a hard reset.
 *
 * Bit-exact: the _boot tail call is preserved. If the port's heap is
 * larger this branch never runs, but we leave it as the spec-correct
 * behaviour.
 */
#include <stdint.h>

extern void *Heap_Alloc(uint32_t n);
extern void  DrawSync(int mode);
extern void  Buffer_FlushDeferredFree(int parity);   /* FUN_80011914 */
extern void  Async_StopAllocCallback(int unused);    /* FUN_800165cc */
extern int   Asset_EvictTick(void);                  /* FUN_80020d3c */
extern void  _boot(void);

extern int32_t g_dispBufIndex;   /* iRam00000004 */

void *Heap_AllocOrRetry(uint32_t n)
{
    void *p = Heap_Alloc(n);
    if (p != NULL || n == 0) return p;

    DrawSync(0);
    Buffer_FlushDeferredFree(1 - g_dispBufIndex);
    Async_StopAllocCallback(1);

    for (;;) {
        p = Heap_Alloc(n);
        if (p != NULL) return p;
        if (Asset_EvictTick() == 0) break;
    }
    _boot();
    return NULL;   /* unreachable */
}
