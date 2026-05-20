/* missile_chain.c -- SCRTBASE multi-stage missile chain spawner.
 *
 * Source: SCRTBASE.DLL  FUN_8010076c.
 *
 * Recursive multi-stage projectile that fires one child per countdown
 * expiry, decrementing a remaining-stage counter at obj+0x4a (i16).
 * Each stage:
 *   - Decrement counter at obj+0x96 each tick.
 *   - On reaching 1: if obj+0x4a (stages left) is non-zero, spawn a
 *     child of kind 0x1e1 (same type) chained recursively to this
 *     function as its tick callback.
 *   - Decrement stages-left counter at obj+0x4a on each spawn.
 *
 * The "self-recursive callback" pattern creates a chain reaction:
 *   missile 1 -> missile 2 -> ... -> missile N
 * each firing 0x96 ticks after the previous.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern int Bone_AllocSlot(int parent, uint16_t slotKey);
extern void Object_PostUpdate2(uint32_t obj);

uint32_t SB_MissileChain(uint32_t *self, uint32_t mode, int *impulse)
{
    if (mode == 3) goto bypass;
    if (mode != 0 && mode != 5) goto bypass;

    int16_t *cd = (int16_t *)((uint8_t *)self + 0x96);
    int16_t prev = *cd;
    *cd = (int16_t)(prev - 1);
    if (prev != 1) return 0;
    if (*(int16_t *)(self + 0x25) == 0) return 0;

    /* Spawn next stage. */
    uint32_t *child = Object_Pool_AllocFromBank(
        (void *)(uintptr_t)self[0x16], 0x1e1, 0x98, 8);
    int slot = Bone_AllocSlot((int)(uintptr_t)self, 0x8000);
    (void)slot;

    *((int8_t *)child + 4) = 7;
    child[0]    = 0x84;
    *((uint16_t *)child + 6) = *(uint16_t *)((uint8_t *)self + 6);
    *(int16_t *)(child + 3)   = (int16_t)self[3];
    child[0x19] = (uintptr_t)SB_MissileChain;       /* recursive */

    Object_PostUpdate2((uint32_t)(uintptr_t)child);

    *(int16_t *)((uint8_t *)child + 0x96) = 4;
    *(int16_t *)(child + 0x25) = (int16_t)(self[0x25] - 1);

    /* Carry velocity + pos. */
    child[0x22] = self[0x22];
    child[0x23] = self[0x23];
    child[0x24] = self[0x24];

bypass:
    (void)impulse;
    return 0;
}
