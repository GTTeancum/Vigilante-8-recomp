/* heap_shim.c -- low-memory bump allocator backing the V8 heap.
 *
 * AUDIT NOTE: the cleaned src/assets/heap.c is a faithful 1:1
 * decompilation of the PSX first-fit allocator, but its block layout
 * (8-byte header: ptr + size) is structurally tied to 32-bit pointer
 * size. On x64 V8HeapBlock balloons to 16 bytes and the literal-8
 * arithmetic everywhere goes wrong.
 *
 * For Phase 2 we replace it with a simple bump allocator over the
 * Host_HeapBase() arena (which we reserved in low memory <4GiB so
 * cleaned code that stores pointers as uint32_t round-trips).
 *
 * Frees are no-ops. The arena is 8 MiB and we expect Phase 2-4 to
 * stay within budget; bigger phases (multi-level loads) will need a
 * real free-list. Marked as TODO in audit_log.md.
 *
 * Allocation walk order is no longer 1:1 with PSX -- if any gameplay
 * code depends on malloc side-effects (AI seed leak?) it will
 * diverge. Marked as TODO.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

extern void    *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

static uint8_t *g_arena       = NULL;   /* base of the low-memory arena */
static uint32_t g_arena_size  = 0;
static uint32_t g_arena_used  = 0;
static uint64_t g_alloc_count = 0;
static uint64_t g_free_count  = 0;

static void heap_shim_init_once(void)
{
    if (g_arena) return;
    g_arena      = (uint8_t *)Host_HeapBase();
    g_arena_size = Host_HeapSize();
    g_arena_used = 0;
}

void Heap_Init(void *base, uint32_t size)
{
    (void)base; (void)size;
    heap_shim_init_once();
    fprintf(stderr, "v8: heap_shim active (arena=%p, size=0x%x)\n",
            g_arena, g_arena_size);
}

void *Heap_Alloc(uint32_t nbytes)
{
    if (nbytes == 0) return NULL;
    heap_shim_init_once();

    /* 8-byte align the request. */
    uint32_t need = (nbytes + 7u) & ~7u;
    if (g_arena_used + need > g_arena_size) {
        fprintf(stderr, "v8: heap_shim OOM: need=%u used=%u size=%u\n",
                need, g_arena_used, g_arena_size);
        return NULL;
    }
    void *p = g_arena + g_arena_used;
    g_arena_used += need;
    g_alloc_count++;
    return p;
}

void Heap_Free(void *ptr)
{
    /* Bump allocator: free is a no-op. */
    if (!ptr) return;
    g_free_count++;
}

void *Heap_Realloc(void *ptr, uint32_t nbytes)
{
    /* No real realloc: allocate fresh and copy. We don't know the old
     * size, so we copy `nbytes` (safe-but-may-read-past-old-block).
     * The realloc-shrink path that the cleaned heap exercised (after
     * a CdRead) is the common case; copying the truncated size is OK
     * because the buffer is still valid. */
    if (!ptr)         return Heap_Alloc(nbytes);
    if (nbytes == 0)  { Heap_Free(ptr); return NULL; }
    void *p = Heap_Alloc(nbytes);
    if (p) memcpy(p, ptr, nbytes);
    return p;
}

void *Heap_AllocOrRetry(uint32_t nbytes)
{
    void *p = Heap_Alloc(nbytes);
    if (!p) fprintf(stderr, "v8: Heap_AllocOrRetry(%u) -> OOM\n", nbytes);
    return p;
}

void *Heap_Calloc(uint32_t nbytes)
{
    void *p = Heap_Alloc(nbytes);
    if (p) memset(p, 0, nbytes);
    return p;
}

void *Heap_CallocOrRetry(uint32_t nbytes)
{
    void *p = Heap_AllocOrRetry(nbytes);
    if (p) memset(p, 0, nbytes);
    return p;
}

void Heap_Report(void)
{
    fprintf(stderr, "v8: heap report -- allocs=%llu  frees=%llu  used=%u/%u bytes\n",
            (unsigned long long)g_alloc_count,
            (unsigned long long)g_free_count,
            g_arena_used, g_arena_size);
}
