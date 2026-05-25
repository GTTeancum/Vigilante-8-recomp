/* heap_shim.c -- low-memory PSX heap backing the V8 runtime.
 *
 * The cleaned src/assets/heap.c uses native pointers in V8HeapBlock; that
 * widens the original 8-byte PSX header on x64.  This shim keeps the exact
 * 32-bit header layout in the low heap:
 *   word0 next-free-block address
 *   word1 size in 8-byte units, including the header
 * and ports the original first-fit/free/coalesce algorithm directly.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

extern void    *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

static uint8_t *g_arena          = NULL;   /* base of the low-memory arena */
static uint32_t g_arena_addr     = 0;
static uint32_t g_arena_size     = 0;
static uint32_t g_heap_free_head = 0;
static uint64_t g_alloc_count    = 0;
static uint64_t g_free_count     = 0;
static uint64_t g_alloc_bytes    = 0;

static void heap_shim_init_once(void)
{
    if (g_arena) return;
    g_arena      = (uint8_t *)Host_HeapBase();
    g_arena_addr = (uint32_t)(uintptr_t)g_arena;
    g_arena_size = Host_HeapSize();
    g_heap_free_head = 0;
}

void Heap_Init(void *base, uint32_t size)
{
    uint32_t aligned_size;
    uint32_t base_addr;
    uint32_t tail_addr;

    heap_shim_init_once();
    if (base != NULL)
        g_arena = (uint8_t *)base;
    if (size != 0)
        g_arena_size = size;
    g_arena_addr = (uint32_t)(uintptr_t)g_arena;

    aligned_size = g_arena_size & 0xfffffff8u;
    base_addr = g_arena_addr;
    tail_addr = base_addr + aligned_size - 8u;
    *(uint32_t *)(uintptr_t)tail_addr = base_addr;
    *(uint32_t *)(uintptr_t)(tail_addr + 4u) = 0;
    *(uint32_t *)(uintptr_t)base_addr = tail_addr;
    *(uint32_t *)(uintptr_t)(base_addr + 4u) = (aligned_size >> 3) - 1u;
    g_heap_free_head = base_addr;
    g_alloc_count = 0;
    g_free_count = 0;
    g_alloc_bytes = 0;

    fprintf(stderr, "v8: heap_shim active (arena=%p, size=0x%x, free-list=psx32)\n",
            g_arena, g_arena_size);
}

static int heap_ptr_in_arena(uint32_t addr)
{
    return addr >= g_arena_addr && addr < g_arena_addr + g_arena_size;
}

static uint32_t heap_next(uint32_t block)
{
    return *(uint32_t *)(uintptr_t)block;
}

static void heap_set_next(uint32_t block, uint32_t next)
{
    *(uint32_t *)(uintptr_t)block = next;
}

static uint32_t heap_size_units(uint32_t block)
{
    return *(uint32_t *)(uintptr_t)(block + 4u);
}

static void heap_set_size_units(uint32_t block, uint32_t units)
{
    *(uint32_t *)(uintptr_t)(block + 4u) = units;
}

void *Heap_Alloc(uint32_t nbytes)
{
    uint32_t units;
    uint32_t p;

    if (nbytes == 0) return NULL;
    heap_shim_init_once();
    if (g_heap_free_head == 0)
        Heap_Init(g_arena, g_arena_size);

    units = (nbytes + 0x0fu) >> 3;
    p = g_heap_free_head;
    for (;;) {
        uint32_t q = heap_next(p);
        int32_t remain;

        if (!heap_ptr_in_arena(q)) {
            fprintf(stderr, "v8: heap_shim corrupt freelist q=0x%x p=0x%x\n", q, p);
            return NULL;
        }
        remain = (int32_t)heap_size_units(q) - (int32_t)units;
        if (remain >= 0) {
            uint32_t user;

            if (remain == 0) {
                g_heap_free_head = p;
                heap_set_next(p, heap_next(q));
                user = q + 8u;
            } else {
                uint32_t header;

                heap_set_size_units(q, (uint32_t)remain);
                header = q + (uint32_t)remain * 8u;
                heap_set_size_units(header, units);
                g_heap_free_head = p;
                user = header + 8u;
            }
            g_alloc_count++;
            g_alloc_bytes += (uint64_t)(units << 3);
            return (void *)(uintptr_t)user;
        }
        p = q;
        if (q == g_heap_free_head) {
            fprintf(stderr, "v8: heap_shim OOM: need=%u units=%u\n",
                    nbytes, units);
            return NULL;
        }
    }
}

void Heap_Free(void *ptr)
{
    uint32_t block;
    uint32_t p;
    uint32_t q;
    uint32_t block_size;
    uint32_t p_size;
    uint32_t guard = 0;

    if (ptr == NULL)
        return;
    heap_shim_init_once();
    if (g_heap_free_head == 0)
        return;

    block = (uint32_t)(uintptr_t)ptr - 8u;
    if (!heap_ptr_in_arena(block)) {
        fprintf(stderr, "v8: heap_shim free ignored non-heap ptr=%p\n", ptr);
        return;
    }

    p = g_heap_free_head;
    for (;;) {
        g_heap_free_head = p;
        q = heap_next(g_heap_free_head);
        if (g_heap_free_head < block && block < q)
            break;
        if (g_heap_free_head < q || (block <= g_heap_free_head && q <= block)) {
            p = q;
            if (++guard > (g_arena_size >> 3)) {
                fprintf(stderr, "v8: heap_shim free walk overflow ptr=%p\n", ptr);
                return;
            }
            continue;
        } else {
            break;
        }
    }

    q = heap_next(g_heap_free_head);
    block_size = heap_size_units(block);
    if (block + block_size * 8u == q && heap_size_units(q) != 0) {
        block_size += heap_size_units(q);
        q = heap_next(q);
        heap_set_size_units(block, block_size);
    }
    p_size = heap_size_units(g_heap_free_head);
    heap_set_next(block, q);
    if (g_heap_free_head + p_size * 8u == block) {
        heap_set_size_units(g_heap_free_head, block_size + p_size);
        block = q;
    }
    heap_set_next(g_heap_free_head, block);
    g_free_count++;
}

void FUN_80045088(int ptr)
{
    Heap_Free((void *)(uintptr_t)(uint32_t)ptr);
}

void *Heap_Realloc(void *ptr, uint32_t nbytes)
{
    uint32_t header;
    uint32_t units;
    int32_t diff;
    void *next;

    if (ptr == NULL)
        return NULL;
    if (nbytes == 0) {
        Heap_Free(ptr);
        return NULL;
    }
    header = (uint32_t)(uintptr_t)ptr - 8u;
    if (!heap_ptr_in_arena(header))
        return NULL;

    units = (nbytes + 0x0fu) >> 3;
    diff = (int32_t)heap_size_units(header) - (int32_t)units;
    if (diff == 0)
        return ptr;
    if (diff > 0) {
        uint32_t split = header + units * 8u;

        heap_set_size_units(header, units);
        heap_set_size_units(split, (uint32_t)diff);
        Heap_Free((void *)(uintptr_t)(split + 8u));
        return ptr;
    }
    next = Heap_Alloc(nbytes);
    if (next != NULL) {
        uint32_t old_payload = (heap_size_units(header) - 1u) << 3;
        memcpy(next, ptr, old_payload);
        Heap_Free(ptr);
    }
    return next;
}

void *Heap_AllocOrRetry(uint32_t nbytes)
{
    void *p = Heap_Alloc(nbytes);
    if (!p && nbytes != 0)
        fprintf(stderr, "v8: Heap_AllocOrRetry(%u) -> OOM\n", nbytes);
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

/* FUN_8001178c -- PSX heap-alloc-with-retry, two-arg form.
 * param_1 = size in bytes; param_2 = mode (1 = retry once, 2 = stream).
 * On host, both modes map to Heap_AllocOrRetry. */
void *FUN_8001178c(uint32_t nbytes, uint32_t mode)
{
    (void)mode;
    return Heap_AllocOrRetry(nbytes);
}

/* FUN_80045134 -- Heap_Shrink: attempt to shrink a live heap block.
 * PSX first-fit allocator detail; on host (bump allocator) shrinking is
 * a no-op: return the original pointer. */
int FUN_80045134(int ptr, int newSize)
{
    return (int)(uintptr_t)Heap_Realloc((void *)(uintptr_t)ptr, (uint32_t)newSize);
}

void Heap_Report(void)
{
    fprintf(stderr, "v8: heap report -- allocs=%llu  frees=%llu  bytes=%llu arena=%u\n",
            (unsigned long long)g_alloc_count,
            (unsigned long long)g_free_count,
            (unsigned long long)g_alloc_bytes,
            g_arena_size);
}
