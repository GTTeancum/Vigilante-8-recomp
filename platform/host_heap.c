/* host_heap.c -- backing storage for the V8 heap.
 *
 * IMPORTANT: the heap MUST live below 4 GiB so that the cleaned code's
 * 32-bit pointer stores (residue from PSX 32-bit semantics) round-trip
 * correctly. quest_loader.c, main_loop.c and many others store pointers
 * as uint32_t inside data tables. A static buffer on x64 Windows
 * typically loads at 0x7FFxxxxxxxxx -- truncating to 32 bits crashes.
 *
 * On Windows we reserve via VirtualAlloc with explicit low-memory
 * hints. On Linux we use mmap(MAP_32BIT). Falls back to malloc with a
 * warning if low memory is unavailable.
 *
 * Size: 8 MiB. The PSX original had ~1.5 MiB; we are more generous to
 * survive bigger asset loads on host (no SPU split, etc.).
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define V8_HOST_HEAP_SIZE  (8 * 1024 * 1024)

#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <sys/mman.h>
  #ifndef MAP_32BIT
    #define MAP_32BIT 0
  #endif
#endif

static void *g_host_heap_ptr = NULL;

static void *alloc_low_memory(uint32_t size)
{
#if defined(_WIN32)
    static const uintptr_t hints[] = {
        0x10000000, 0x20000000, 0x30000000, 0x40000000, 0x50000000,
        0x60000000, 0x70000000,
    };
    for (size_t i = 0; i < sizeof(hints)/sizeof(hints[0]); i++) {
        void *p = VirtualAlloc((void *)hints[i], size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (p) return p;
    }
    fprintf(stderr, "v8: WARNING: low-memory heap reservation failed; "
                    "falling back to high address (32-bit ptr trunc may break)\n");
    return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    void *p = mmap((void *)0x10000000, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    return (p == MAP_FAILED) ? NULL : p;
#endif
}

void *Host_HeapBase(void)
{
    if (!g_host_heap_ptr) {
        g_host_heap_ptr = alloc_low_memory(V8_HOST_HEAP_SIZE);
        if (!g_host_heap_ptr) {
            fprintf(stderr, "v8: FATAL: could not allocate host heap\n");
            exit(2);
        }
        fprintf(stderr, "v8: host heap reserved at %p (size 0x%x)\n",
                g_host_heap_ptr, V8_HOST_HEAP_SIZE);
        if ((uintptr_t)g_host_heap_ptr >> 32) {
            fprintf(stderr, "v8: WARNING: heap above 4 GiB -- pointer "
                            "truncation will misbehave!\n");
        }
    }
    return g_host_heap_ptr;
}

uint32_t Host_HeapSize(void)
{
    return V8_HOST_HEAP_SIZE;
}
