/* buffer.c -- per-frame draw-buffer ordering table + deferred-free lists.
 *
 * Source: SLUS_005.10
 *   FUN_800119c0 -- Buffer_StartOTag(parity)
 *   FUN_80011914 -- Buffer_FlushDeferredFree(parity)
 *   FUN_800118b4 -- Buffer_DeferFree(ptr)
 *
 * V8 uses standard PSX double-buffered rendering. Two 0x4400-byte
 * ordering-table arenas live at UNK_800664a0 and that+0x4400. A
 * deferred-free list head/tail/sentinel triple lives at
 * DAT_8006eca0 (parity 0) and DAT_8006eca0 + 12 (parity 1).
 *
 * Lifecycle per frame:
 *   1. game-tick code calls Buffer_DeferFree(p) for any heap blocks
 *      that hold draw-list-referenced data this frame.
 *   2. at frame boundary the renderer drains the OT and we flip parity.
 *   3. Buffer_StartOTag(newParity) clears the new arena and calls
 *      Buffer_FlushDeferredFree(newParity), releasing everything
 *      pinned by the *previous-frame-of-this-parity* draw.
 *
 * Bit-exact: this is renderer-adjacent (puRam0000060c points at the OT
 * the renderer consumes). The renderer is rewritten per CLAUDE.md so
 * the new layer can use any OT format; what matters for 1:1 is when
 * the deferred frees fire -- preserve that timing exactly.
 */
#include <stdint.h>

extern void *Heap_Free(void *p);            /* FUN_80045088 */
extern void *Heap_AllocOrRetry(uint32_t n); /* FUN_800116f4 */
extern void  ClearOTagR(uint32_t *otag, int n);

extern int32_t   iRam00000004;     /* current display buffer parity (0/1) */
extern uint32_t *puRam0000060c;    /* per-frame OT base */
extern uint8_t   UNK_800664a0[];   /* OT arena base (4400 bytes per parity) */
extern uint32_t  DAT_8006eca0[];   /* deferred-free triples: head, tail, sentinel x 2 */

typedef struct DeferredFreeNode {
    struct DeferredFreeNode *next;
    struct DeferredFreeNode *sentinelOrPrev;
    void                    *payload;
} DeferredFreeNode;

/* HIGH: drain and free all deferred-free entries for `parity`. */
void Buffer_FlushDeferredFree(int parity)
{
    DeferredFreeNode *cur  = (DeferredFreeNode *)DAT_8006eca0[parity * 3];
    DeferredFreeNode *next = (DeferredFreeNode *)cur->next;
    while (next != NULL) {
        Heap_Free(cur->payload);
        Heap_Free(cur);
        cur  = next;
        next = (DeferredFreeNode *)next->next;
    }
    /* Reinitialise: head -> sentinel, sentinel -> NULL, tail -> &head. */
    DAT_8006eca0[parity * 3 + 0] = (uint32_t)&DAT_8006eca0[parity * 3 + 1];
    DAT_8006eca0[parity * 3 + 1] = 0;
    DAT_8006eca0[parity * 3 + 2] = (uint32_t)&DAT_8006eca0[parity * 3 + 0];
}

/* HIGH: enqueue `payload` for free at the end of the current draw frame. */
void Buffer_DeferFree(void *payload)
{
    DeferredFreeNode *n = (DeferredFreeNode *)Heap_AllocOrRetry(12);
    n->payload = payload;
    int parity = iRam00000004;
    DeferredFreeNode *tail = (DeferredFreeNode *)DAT_8006eca0[parity * 3 + 2];
    DAT_8006eca0[parity * 3 + 2] = (uint32_t)n;
    tail->next = n;
    n->sentinelOrPrev = tail;
    n->next = (DeferredFreeNode *)&DAT_8006eca0[parity * 3 + 1];
}

/* HIGH: start a new frame on `parity` -- swap OT arena, flush deferred frees. */
void Buffer_StartOTag(int parity)
{
    puRam0000060c = (uint32_t *)(UNK_800664a0 + parity * 0x4400);
    iRam00000004  = parity;
    ClearOTagR(puRam0000060c, 0x1000);
    Buffer_FlushDeferredFree(parity);
}
