/* ball.c -- Ski Resort destructible-and-area-broadcast handler.
 *
 * Source: SKIRESRT.DLL  FUN_801019d8.
 *
 * Same shape as HD_TransfBoxBroadcast but with state code 0 (a
 * "release" or "reset" -- the gondola Ball_1 / Ball_2 swings free
 * when the support transformer is destroyed rather than entering an
 * "off" state).
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern void Object_SetState(int obj, int state);

uint32_t SR_BallDestroy(int obj, uint32_t mode, void *impactCtx)
{
    if (mode == 3 || mode != 8) {
        Damage_FromImpulse((uint32_t *)(intptr_t)obj, (int *)impactCtx);
    }
    int killed = Damage_AccumulateOrFire((uint32_t *)(intptr_t)obj, (uint16_t)(uintptr_t)impactCtx);
    if (killed == 0) return 0;

    int32_t px = *(int32_t *)(obj + 0x48);
    int32_t pz = *(int32_t *)(obj + 0x50);
    if (px < 0) px += 0xffff;
    if (pz < 0) pz += 0xffff;
    int16_t rect[4] = {
        (int16_t)((uint32_t)px >> 16),
        (int16_t)((uint32_t)pz >> 16),
        1, 1,
    };
    Object_SetState((int)(uintptr_t)rect, 0);
    return 0xffffffffu;
}
