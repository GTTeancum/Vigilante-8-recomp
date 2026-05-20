/* effects.c -- particle / spark / explosion spawn helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8003fc50  -- Effects_SpawnExplosion(obj)
 *                    Polls FUN_8003fbc8 for an available effect slot;
 *                    if available, spawns one at obj+0x58's bank.
 *                    Returns 1 if spawned, 0 otherwise.
 *   FUN_8003fc94  -- Effects_QueueSnow(obj)
 *                    Walks the parent->children chain rooted at obj+0x58
 *                    via the bone-template's +0x36 next-link, counting
 *                    nodes whose flags byte == 0xff. Returns count.
 *   FUN_8003fd24  -- Effects_SpawnParticleAtParent(parent, kind)
 *                    Allocates a 0x80-byte particle prim from the
 *                    vehicle bank, sets pos to parent's pos
 *                    (parent[0..2]), installs LAB_8003e80c as the tick
 *                    callback, marks state 0x34 (active).
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t Effects_PollFreeSlot(void);                /* FUN_8003fbc8 */
extern void     Effects_BindSlot(int obj, void *bank, uint16_t slot);  /* FUN_8003fac4 */
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern void *LAB_8003e80c;
extern uint32_t _DAT_800737d8;
extern void **_DAT_800737d8_ptr;

int Effects_SpawnExplosion(int obj)
{
    uint32_t slot = Effects_PollFreeSlot();
    if (slot == 0) return 0;
    Effects_BindSlot(obj, *(void **)(obj + 0x58), (uint16_t)slot);
    return 1;
}

int Effects_QueueSnow(int obj)
{
    int *bone = *(int **)(obj + 0x58);
    uint16_t next = *(uint16_t *)((uint8_t *)bone + *(uint16_t *)(obj + 10) * 0x1c + 0x36);
    int count = 0;
    while (next != 0xffff) {
        int *node = (int *)((uint8_t *)bone + next * 0x1c);
        uint16_t flags = *(uint16_t *)((uint8_t *)node + 0x1c);
        if ((flags >> 8) == 0xff && flags != 0xffff) {
            next = *(uint16_t *)((uint8_t *)node + 0x36);
            count++;
            continue;
        }
        next = flags;
    }
    return count;
}

uint32_t *Effects_SpawnParticleAtParent(uint32_t *parent, uint16_t kind)
{
    uint32_t *p = Object_Pool_AllocFromBank((void *)(uintptr_t)_DAT_800737d8, kind, 0x80, 8);
    *((int8_t *)p + 4) = 1;
    p[0] = 0x34;
    p[0x12] = parent[0];
    p[0x13] = parent[1];
    p[0x14] = parent[2];
    p[0x19] = (uintptr_t)&LAB_8003e80c;
    return p;
}
