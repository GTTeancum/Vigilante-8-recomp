/* boulder_chain.c -- Canyonlands boulder spawn-chain state machine.
 *
 * Source: CANYNLND.DLL  FUN_80100cbc.
 *
 * State machine for the boulder/overpass collapse chain. On mode 2:
 *   - sub-state 1: kill the previously-spawned segment via
 *     Damage_Apply(parent[0x1d]), set sub-state 0x708, mark flag 0x20.
 *   - sub-state 0/2: spawn a fresh boulder (kind 0x2bf, size 0x80) at
 *     parent's pos with vy = pos.y - 0x14000 (drop offset), register
 *     it for post-update, store its handle at parent[0x1d], clear
 *     flag 0x20, sub-state 900 (= "spawned").
 * After either branch, spawn a generic dust-cloud child (kind from
 * parent[0xa], lifetime 0x24) at the boulder's pos, with the
 * generic projectile-tick at 0x8003e80c.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern void Damage_Apply(void *obj);
extern void Object_SetSubState(int obj, int sub);
extern void Object_RegisterPostUpdate(uint32_t *obj);     /* FUN_80020744 */
extern void Object_PostUpdate2(uint32_t obj);             /* FUN_8002036c */

uint32_t CL_BoulderChain(uint32_t *self, uint32_t mode, uint32_t *impulse)
{
    if (mode != 2) return 0;

    int8_t sub = *(int8_t *)((uint8_t *)self + 8);

    if (sub == 1) {
spawn_dust:
        Damage_Apply((void *)(uintptr_t)self[0x1d]);
        Object_SetSubState((int)(uintptr_t)self, 0x708);
        *(int8_t *)((uint8_t *)self + 8) = 0;
        self[0] |= 0x20u;
    } else if (sub == 0 || sub == 2) {
        /* Spawn a fresh boulder. */
        uint32_t bank = self[0x16];
        uint32_t *boulder = Object_Pool_AllocFromBank((void *)(uintptr_t)bank, 0x2bf, 0x80, 8);
        *(void **)((uint8_t *)boulder + 100) = (void *)(uintptr_t)0;   /* tick installed by FUN_80100be8 = Boulder_Tick (we previously cleaned) */
        *(uint32_t *)((uint8_t *)boulder + 0x48) = self[0x12];
        *(uint32_t *)((uint8_t *)boulder + 0x4c) = self[0x13] - 0x14000;
        *(uint32_t *)((uint8_t *)boulder + 0x50) = self[0x14];
        Object_PostUpdate2((uint32_t)(uintptr_t)boulder);
        self[0x1d] = (uintptr_t)boulder;
        self[0] &= ~0x20u;
        Object_SetSubState((int)(uintptr_t)self, 900);
        *(int8_t *)((uint8_t *)self + 8) = 1;
        goto spawn_dust;
    }

    /* Common tail: spawn dust cloud at boulder's current pos. */
    uint32_t boulderHandle = self[0x1d];
    uint32_t *dust = Object_Pool_AllocFromBank((void *)(uintptr_t)self[0x16],
                                                *(uint16_t *)((uint8_t *)self + 0xa), 0x80, 8);
    dust[0] = 0x24;
    dust[0x12] = *(uint32_t *)(boulderHandle + 0x48);
    dust[0x13] = *(uint32_t *)(boulderHandle + 0x4c);
    dust[0x14] = *(uint32_t *)(boulderHandle + 0x50);
    dust[0x19] = 0x8003e80c;
    Object_PostUpdate2((uint32_t)(uintptr_t)dust);
    (void)impulse;
    return 0;
}
