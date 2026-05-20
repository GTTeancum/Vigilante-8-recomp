/* heap_calloc.c -- calloc-like helper.
 *
 * Source: SLUS_005.10  FUN_800451c0.
 *
 * Allocates n*size bytes via Heap_Alloc and zero-fills in 8-byte
 * chunks. Returns silently on OOM via Ghidra's "indirect jump as
 * return" -- in practice the caller checks for NULL.
 *
 * HIGH confidence: trivial multiply-alloc-bzero pattern.
 */
#include <stdint.h>

extern void *Heap_Alloc(uint32_t n);

void *Heap_Calloc(int count, int size)
{
    int total = count * size;
    uint32_t *p = (uint32_t *)Heap_Alloc((uint32_t)total);
    if (p == NULL) return NULL;
    uint32_t *q = p;
    while (total > 0) {
        q[0] = 0;
        q[1] = 0;
        total -= 8;
        q     += 2;
    }
    return p;
}
