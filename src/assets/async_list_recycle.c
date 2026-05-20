/* async_list_recycle.c -- compact the async-alloc free chain.
 *
 * Source: SLUS_005.10  FUN_80011834.
 *
 * piRamffff9a48 is the head pointer of a singly-linked free list of
 * async-alloc nodes (each node has its `next` ptr at +0 and an
 * "in-use" flag at +1). The function:
 *   1. Walks forward to the last entry whose flag is non-zero (the
 *      "active tail").
 *   2. Then re-walks the chain from that tail forward, latching the
 *      head pointer at each step.
 *
 * The net effect is that the global head ends up pointing at the
 * youngest in-use node, with the older idle nodes implicitly skipped
 * over for future traversals -- a single-pass list compaction.
 *
 * HIGH on mechanism; MED on intended use until pass 2 confirms the
 * exact node struct layout.
 */
#include <stdint.h>

extern int32_t **piRamffff9a48;

void AsyncList_RecycleHead(void)
{
    int32_t *p = (int32_t *)piRamffff9a48[0];
    while (p[1] != 0) {
        p = (int32_t *)p[0];
    }
    /* p now points at the first idle entry past the active tail. */
    int32_t *q = (int32_t *)p[0];
    piRamffff9a48 = (int32_t **)p;

    while (q != NULL && q[1] != 0) {
        int32_t *next = (int32_t *)q[0];
        piRamffff9a48 = (int32_t **)p;
        p = q;
        q = next;
    }
}
