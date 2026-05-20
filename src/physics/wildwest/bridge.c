/* bridge.c -- Wild West TBridge_1 / Gallow_1 destroy-on-impact handler.
 *
 * Source: WILDWEST.DLL  FUN_801003ec.
 *
 * Triggered when a vehicle / projectile collides with the level
 * object. The dispatch:
 *
 *   - mode == 3 (collision with bullet/missile): try a quick
 *     ground-check via func_0x8002239c (HIGH-confidence: collision
 *     test against terrain heightmap); if that fails, try a circular
 *     overlap via func_0x80022320.
 *   - mode == 1: passthrough.
 *   - mode == 8: forced impact; equivalent to falling into the
 *     terrain-check.
 *
 * On any successful hit, write 0x14 into obj.flagsB.+0x28 (signal
 * destroyed) and queue a particle effect via func_0x80042cdc (likely
 * Effects_QueueParticles).
 *
 * MED confidence.
 */
#include <stdint.h>

extern int  Collision_AgainstTerrain(int obj, int impact);    /* func_0x8002239c */
extern int  Collision_Circular     (int obj, int impact);    /* func_0x80022320 */
extern void *Effects_QueueParticles(int posXyzAddr, int);    /* func_0x80042cdc */

uint32_t WW_BridgeDestroyTick(int obj, uint32_t mode, void *impactCtx)
{
    int  collisionPosAddr;
    int  hit;

    if (mode == 3) {
        hit = Collision_AgainstTerrain(obj, (int)(intptr_t)impactCtx);
        collisionPosAddr = obj + 0x48;
        if (hit != 0) goto fired;
        hit = Collision_Circular(obj, (int)(intptr_t)impactCtx);
        if (hit != 0) goto fired;
    } else if (mode > 3 || mode == 1) {
        if (mode != 8) {
            hit = Collision_AgainstTerrain(obj, (int)(intptr_t)impactCtx);
            collisionPosAddr = obj + 0x48;
            if (hit != 0) goto fired;
        }
        hit = Collision_Circular(obj, (int)(intptr_t)impactCtx);
        collisionPosAddr = obj + 0x48;
        if (hit != 0) goto fired;
    }
    collisionPosAddr = 1;
    *(uint16_t *)(*(int *)(obj + 0x30) + 0x28) = 0x14;

fired: {
    void *fx = Effects_QueueParticles(collisionPosAddr, 0);
    *(uint16_t *)((uint8_t *)fx + 8) = 0xffff;
    }
    return 0;
}
