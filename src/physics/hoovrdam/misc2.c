/* misc2.c -- more Hoover Dam handlers.
 *
 * Source: HOOVRDAM.DLL
 *   FUN_80101a98 -- HD_TransfBoxBroadcast: on Damage event, compute
 *                   the destructible's grid cell (i16 of posX/Z high
 *                   half via the standard +0xffff round, then 1x1 cell
 *                   size) and Object_SetState(cellRect, 0x8f80).
 *                   This is the "powerline transformer collapses ->
 *                   surrounding area goes dark" effect trigger.
 *   FUN_80100b40 -- HD_PipeChildSpawn: small generic projectile/child
 *                   spawner with explicit (bank, kind, pos, animSlot,
 *                   spriteSlot, lifetimeMaybe) args. Sets state 0xa4,
 *                   tick = FUN_80100a18 (HD's standard projectile tick).
 *
 * MED-HIGH confidence.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern void Object_SetState(int obj, int state);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern void Object_RandomizeRotation(uint32_t *obj);   /* FUN_80016da8 */
extern void Object_RegisterInScene(uint32_t *obj);     /* FUN_800202f4 */
extern uint32_t FUN_80100a18;                           /* HD projectile tick */

uint32_t HD_TransfBoxBroadcast(int obj, uint32_t mode, void *impactCtx)
{
    int hit = 0;
    if ((mode == 3 || mode != 8) && (hit = Damage_FromImpulse((uint32_t *)(intptr_t)obj, (int *)impactCtx)) != 0) {
        goto fire;
    }
    if ((hit = Damage_AccumulateOrFire((uint32_t *)(intptr_t)obj, (uint16_t)(uintptr_t)impactCtx)) == 0) return 0;
fire:
    {
        int32_t px = *(int32_t *)(obj + 0x48);
        int32_t pz = *(int32_t *)(obj + 0x50);
        if (px < 0) px += 0xffff;
        if (pz < 0) pz += 0xffff;
        int16_t rect[4] = {
            (int16_t)((uint32_t)px >> 16),
            (int16_t)((uint32_t)pz >> 16),
            1, 1,
        };
        Object_SetState((int)(uintptr_t)rect, 0x8f80);
    }
    return 0;
}

uint32_t *HD_PipeChildSpawn(uint32_t *parentPos, uint32_t bank, uint16_t kind,
                            uint16_t animSlot, uint16_t spriteSlot, uint32_t lifetime)
{
    uint32_t *c = Object_Pool_AllocFromBank((void *)(uintptr_t)bank, kind, 0x9c, 8);
    Object_RandomizeRotation(c + 4);
    c[9]  = parentPos[0];
    c[10] = parentPos[1];
    c[11] = parentPos[2];
    c[0x19] = (uintptr_t)&FUN_80100a18;
    c[0x15] = 0x8000;
    c[0x26] = bank;
    *(uint16_t *)((uint8_t *)c + 0x96) = animSlot;
    c[0] |= 0xa4u;
    *(uint16_t *)((uint8_t *)c + 0x82) = spriteSlot;
    Object_RegisterInScene(c);
    (void)lifetime;
    return c;
}
