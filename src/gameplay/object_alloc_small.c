/* object_alloc_small.c -- small object heap allocator wrapper.
 *
 * Source: SLUS_005.10
 *   FUN_8001d470  -- Object_AllocSmall(size)  (10 instr)
 *
 * Thin wrapper around FUN_8001178c (the PSX heap-alloc-with-retry).
 * FUN_8001d470 always calls FUN_8001178c(size, 1), where
 * the second arg (mode=1) means "retry once if OOM" (vs mode=2 for streaming).
 *
 * On host, FUN_8001178c is replaced by the heap_shim bump allocator;
 * we delegate to Heap_AllocOrRetry which logs on OOM.
 *
 * MIPS (all 10 instructions):
 *   addiu sp,sp,-0x18
 *   sw    ra, 0x10(sp)
 *   jal   0x8001178c         ; FUN_8001178c(a0=size, a1=1)
 *   _li   a1, 0x1            ; delay: a1 = 1 (retry mode)
 *   lw    ra, 0x10(sp)
 *   nop
 *   jr    ra
 *   _addiu sp,sp,0x18
 *
 * HIGH: trivial trampoline, line-for-line.
 */
#include <stdint.h>

/* host heap shim: Heap_AllocOrRetry maps PSX malloc-with-retry mode=1. */
extern void *Heap_AllocOrRetry(uint32_t nbytes);

/*
 * HIGH: allocate `size` bytes from the persistent heap (with retry).
 */
void *FUN_8001d470(uint32_t size)
{
    return Heap_AllocOrRetry(size);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001d470  (from analysis/SLUS_005.10/mips/8001d470.s) ---
 * 8001d470: addiu sp,sp,-0x18
 * 8001d474: sw    ra,0x10(sp)
 * 8001d478: jal   0x8001178c    ; heap_alloc_with_retry(size, mode)
 * 8001d47c: _li   a1,0x1        ; delay: a1 = 1 (mode = retry)
 * 8001d480: lw    ra,0x10(sp)
 * 8001d484: nop
 * 8001d488: jr    ra
 * 8001d48c: _addiu sp,sp,0x18
 */

#endif  /* GHIDRA REF */
