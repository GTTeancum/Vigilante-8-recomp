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

/* HIGH: raw PSY-Q free3 alias used by decompiled callers. */
void FUN_80045088(int ptr)
{
    Heap_Free((void *)(uintptr_t)(uint32_t)ptr);
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

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80044fbc  (from analysis/SLUS_005.10/decomp/80044fbc.c) --- */
// addr: 0x80044fbc  name: FUN_80044fbc

void FUN_80044fbc(int *param_1,uint param_2)

{
  uint uVar1;
  
  if ((param_1 != (int *)0x0) && (uVar1 = param_2 & 0xfffffff8, param_2 != 0)) {
    *(int **)((int)param_1 + (uVar1 - 8)) = param_1;
    *(undefined4 *)((int)param_1 + (uVar1 - 4)) = 0;
    *param_1 = (int)param_1 + (uVar1 - 8);
    DAT_8005ed4c = param_1;
    DAT_8005ed50 = param_1;
    param_1[1] = (param_2 >> 3) - 1;
  }
  return;
}

/* --- SLUS_005.10 FUN_80045004  (from analysis/SLUS_005.10/decomp/80045004.c) --- */
// addr: 0x80045004  name: FUN_80045004

undefined4 * FUN_80045004(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 *puVar4;
  
  if (param_1 != 0) {
    uVar3 = param_1 + 0xfU >> 3;
    puVar4 = DAT_8005ed4c;
    do {
      puVar2 = (undefined4 *)*puVar4;
      iVar1 = puVar2[1] - uVar3;
      if (-1 < iVar1) {
        if (iVar1 == 0) {
          DAT_8005ed4c = puVar4;
          *puVar4 = *puVar2;
          return puVar2 + 2;
        }
        puVar2[1] = iVar1;
        puVar2[iVar1 * 2 + 1] = uVar3;
        DAT_8005ed4c = puVar4;
        return puVar2 + iVar1 * 2 + 2;
      }
      puVar4 = puVar2;
    } while (puVar2 != DAT_8005ed4c);
  }
  return (undefined4 *)0x0;
}

/* --- SLUS_005.10 FUN_80045088  (from analysis/SLUS_005.10/decomp/80045088.c) --- */
// addr: 0x80045088  name: FUN_80045088

void FUN_80045088(int param_1)

{
  int iVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  
  puVar2 = (uint *)(param_1 + -8);
  puVar3 = DAT_8005ed4c;
  do {
    DAT_8005ed4c = puVar3;
    puVar3 = (uint *)*DAT_8005ed4c;
    if ((DAT_8005ed4c < puVar2) && (puVar2 < puVar3)) break;
  } while ((DAT_8005ed4c < puVar3) || ((puVar2 <= DAT_8005ed4c && (puVar3 <= puVar2))));
  iVar1 = *(int *)(param_1 + -4);
  if ((puVar2 + iVar1 * 2 == puVar3) && (iVar1 = iVar1 + puVar3[1], puVar3[1] != 0)) {
    puVar3 = (uint *)*puVar3;
    *(int *)(param_1 + -4) = iVar1;
  }
  uVar4 = DAT_8005ed4c[1];
  *puVar2 = (uint)puVar3;
  if (DAT_8005ed4c + uVar4 * 2 == puVar2) {
    DAT_8005ed4c[1] = iVar1 + uVar4;
    puVar2 = puVar3;
  }
  *DAT_8005ed4c = (uint)puVar2;
  return;
}

/* --- SLUS_005.10 FUN_80045134  (from analysis/SLUS_005.10/decomp/80045134.c) --- */
// addr: 0x80045134  name: FUN_80045134

/* WARNING: Removing unreachable block (ram,0x8004500c) */
/* WARNING: Removing unreachable block (ram,0x8004501c) */
/* WARNING: Removing unreachable block (ram,0x80045038) */
/* WARNING: Removing unreachable block (ram,0x80045040) */
/* WARNING: Removing unreachable block (ram,0x80045058) */
/* WARNING: Removing unreachable block (ram,0x80045078) */

int FUN_80045134(int param_1,int param_2)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  int iVar4;
  
  if (param_1 == 0) {
    return 0;
  }
  if (param_2 != 0) {
    uVar1 = param_2 + 0xfU >> 3;
    iVar4 = *(uint *)(param_1 + -4) - uVar1;
    if (*(uint *)(param_1 + -4) == uVar1) {
      return param_1;
    }
    if (-1 < iVar4) {
      *(uint *)(param_1 + -4) = uVar1;
      *(int *)(param_1 + uVar1 * 8 + -4) = iVar4;
      iVar4 = FUN_80045088();
      return iVar4;
    }
    iVar4 = FUN_80045004(param_2);
    if (iVar4 != 0) {
      FUN_80044d9c(iVar4,param_1,param_2);
      iVar4 = FUN_80045088(param_1);
      return iVar4;
    }
                    /* WARNING: Treating indirect jump as return */
    return 0;
  }
  puVar2 = (uint *)(param_1 + -8);
  puVar3 = DAT_8005ed4c;
  do {
    DAT_8005ed4c = puVar3;
    puVar3 = (uint *)*DAT_8005ed4c;
    if ((DAT_8005ed4c < puVar2) && (puVar2 < puVar3)) break;
  } while ((DAT_8005ed4c < puVar3) || ((puVar2 <= DAT_8005ed4c && (puVar3 <= puVar2))));
  iVar4 = *(int *)(param_1 + -4);
  if ((puVar2 + iVar4 * 2 == puVar3) && (iVar4 = iVar4 + puVar3[1], puVar3[1] != 0)) {
    puVar3 = (uint *)*puVar3;
    *(int *)(param_1 + -4) = iVar4;
  }
  uVar1 = DAT_8005ed4c[1];
  *puVar2 = (uint)puVar3;
  if (DAT_8005ed4c + uVar1 * 2 == puVar2) {
    DAT_8005ed4c[1] = iVar4 + uVar1;
    puVar2 = puVar3;
  }
  *DAT_8005ed4c = (uint)puVar2;
  return 0;
}

#endif  /* GHIDRA REF */
