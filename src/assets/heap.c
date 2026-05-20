/* heap.c -- Vigilante 8 PSX dynamic heap (K&R-style first-fit free list).
 *
 * Source: SLUS_005.10, functions @ 0x80044fbc, 0x80045004, 0x80045088, 0x80045134.
 * HIGH confidence: matches the canonical PSY-Q `InitHeap` / `malloc3` /
 * `free3` / `realloc3` allocator exactly. The block header is 8 bytes:
 * { struct V8HeapBlock *next; uint32_t size_in_8byte_units; }.
 *
 * Userland gets the address right after the header. The free list is
 * circular and ordered by increasing address; freed blocks are coalesced
 * with their neighbours.
 *
 * Bit-exact notes: the allocator's first-fit walk order is part of the
 * 1:1 contract -- AI code that uses g_rngSeed via malloc-side-effects
 * (does V8?) would diverge if we changed it. Keep the algorithm verbatim.
 */
#include <stdint.h>
#include <stddef.h>
#include "structs.h"
#include "globals.h"

/* Globals defined here. */
V8HeapBlock *g_heapFreeList;   /* @ 0x8005ed4c */
V8HeapBlock *g_heapFreeBase;   /* @ 0x8005ed50 */

/* HIGH: initialize the heap with a single free block spanning the whole arena.
 *   base : pointer to arena (must be 8-byte aligned in practice)
 *   size : arena size in bytes (rounded down to 8)
 */
void Heap_Init(V8HeapBlock *base, uint32_t size)
{
    if (base == NULL || size == 0) return;

    uint32_t alignedSize = size & 0xfffffff8u;
    V8HeapBlock *tail = (V8HeapBlock *)((char *)base + (alignedSize - 8));

    tail->next        = base;            /* circular self-loop via tail */
    tail->size        = 0;
    base->next        = tail;
    g_heapFreeList    = base;
    g_heapFreeBase    = base;
    base->size        = (alignedSize >> 3) - 1;
}

/* HIGH: first-fit allocate. Rounds the request up to an 8-byte unit count
 * (plus the 1-unit header). Returns NULL on OOM. */
void *Heap_Alloc(uint32_t nbytes)
{
    if (nbytes == 0) return NULL;

    uint32_t units = (nbytes + 0x0fu) >> 3;   /* +15 then /8 -> ceil + header */
    V8HeapBlock *p = g_heapFreeList;
    for (;;) {
        V8HeapBlock *q = p->next;
        int32_t       remain = (int32_t)q->size - (int32_t)units;
        if (remain >= 0) {
            if (remain == 0) {
                g_heapFreeList = p;
                p->next        = q->next;
                return (void *)(q + 1);
            }
            q->size = (uint32_t)remain;
            /* Carve the tail of q for the user. */
            V8HeapBlock *u = q + (remain * 2);   /* q + 2*remain in u32 units == q + remain*8 bytes header offset */
            /* Note: PSY-Q lays this out as &q[remain*2+1] which is the
             * (remain*2+1)'th u32 from q -- equivalently the header of the
             * carved tail. Encode as direct pointer arithmetic for clarity: */
            V8HeapBlock *user = (V8HeapBlock *)((uint32_t *)q + remain * 2);
            user->size = units;                  /* size word @ +4 */
            g_heapFreeList = p;
            return (void *)(user + 1);
        }
        p = q;
        if (q == g_heapFreeList) return NULL;    /* full circular sweep */
    }
}

/* HIGH: free + coalesce with both neighbours when adjacent. */
void Heap_Free(void *ptr)
{
    if (ptr == NULL) return;

    V8HeapBlock *b = ((V8HeapBlock *)ptr) - 1;   /* back up over header */
    V8HeapBlock *p = g_heapFreeList;

    /* Walk free list to find insertion point. */
    for (;;) {
        g_heapFreeList = p;
        V8HeapBlock *q = g_heapFreeList->next;
        if (p < b && b < q) break;
        if (p < q || (b <= p && q <= b)) {
            p = q;
            continue;
        }
        p = q;
    }

    V8HeapBlock *q = g_heapFreeList->next;
    uint32_t  bsize = b->size;

    /* Forward-merge: b immediately precedes q (in u8-byte units). */
    if (b + bsize * 2 == q && q->size != 0) {
        bsize += q->size;
        q      = q->next;
        b->size = bsize;
    }

    /* Backward-merge: g_heapFreeList immediately precedes b. */
    uint32_t psize = g_heapFreeList->size;
    b->next = q;
    if (g_heapFreeList + psize * 2 == b) {
        g_heapFreeList->size = bsize + psize;
        b = q;
    }
    g_heapFreeList->next = b;
}

/* HIGH: realloc, shrinking/growing in-place when possible, else
 * malloc+memcpy+free. */
void *Heap_Realloc(void *ptr, uint32_t nbytes)
{
    if (ptr == NULL) return NULL;
    if (nbytes == 0) { Heap_Free(ptr); return NULL; }

    uint32_t      units    = (nbytes + 0x0fu) >> 3;
    V8HeapBlock  *header   = ((V8HeapBlock *)ptr) - 1;
    int32_t       diff     = (int32_t)header->size - (int32_t)units;

    if (diff == 0)  return ptr;
    if (diff > 0) {
        header->size                                   = units;
        ((V8HeapBlock *)((uint32_t *)header + units * 2))->size = (uint32_t)diff;
        Heap_Free((V8HeapBlock *)((uint32_t *)header + units * 2) + 1);
        return ptr;
    }
    void *nb = Heap_Alloc(nbytes);
    if (nb == NULL) return NULL;
    /* Copy old payload (header->size * 8 - 8 bytes of user data). */
    extern void *V8_MemCopy(void *dst, const void *src, int n);
    V8_MemCopy(nb, ptr, (int)((header->size - 1) << 3));
    Heap_Free(ptr);
    return nb;
}
