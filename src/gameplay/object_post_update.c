/* object_post_update.c -- "fast insert" pool helpers + projectile post-tick.
 *
 * Source: SLUS_005.10
 *   FUN_8001fe50  -- ObjList_FastInsert
 *   FUN_80020744  -- Object_RegisterPostUpdate
 *   FUN_8003eab0  -- Projectile_GravityTick (universal callback for the
 *                    XOBF-spawned dropped projectiles across DLLs).
 *
 * ObjList_FastInsert: O(1) head insert into the chain rooted at
 * `listHead`, pulling a free node from the global scratch pool
 * piRam0000076c.
 *
 * Object_RegisterPostUpdate: set the "back-buf pending" bit (0x80)
 * on the object's flags and link it into the back-buf list at
 * DAT_80065a60. Called by every projectile spawner after the new
 * object is configured.
 *
 * Projectile_GravityTick (FUN_8003eab0): the universal "I'm a
 * gravity-affected projectile" callback used by every non-AI
 * projectile across DLLs. Each tick:
 *   1. integrate position by velocity (no scale -- already in world units)
 *   2. apply rotation via Object_ApplyAngularVelocity (small-angle GTE)
 *   3. apply gravity to vy (+0x5a/tick)
 *   4. if below terrain at (posX, posZ), return 0 to despawn
 *
 * HIGH-MED.
 */
#include <stdint.h>

extern int32_t **piRam0000076c;
extern uint8_t  DAT_80065a70[];
extern uint8_t  DAT_80065a60[];
extern void Object_ApplyAngularVelocity(uint32_t *m, int pitchRate, int yawRate, int rollRate);  /* FUN_800439b8 */
extern int  Terrain_HeightAndProbe(int self, int *posXyz, int u1, int u2);    /* FUN_8001d748 */

/* HIGH: head-insert `payload` into `listSentinel`'s chain. */
void ObjList_FastInsert(int listSentinel, int payload)
{
    int32_t *node = *piRam0000076c;
    int32_t  scratchHead = (int32_t)(uintptr_t)node;
    *(int32_t **)(scratchHead + 4) = (int32_t *)DAT_80065a70;
    piRam0000076c = (int32_t **)node;
    node[2] = payload;

    int32_t **chainTail = *(int32_t ***)(listSentinel + 8);
    *(int32_t ***)(listSentinel + 8) = (int32_t **)scratchHead;
    *chainTail = (int32_t *)scratchHead;
    *(int32_t **)(scratchHead + 4) = (int32_t *)chainTail;
    *(int32_t *)scratchHead       = listSentinel + 4;
}

void Object_RegisterPostUpdate(uint32_t *obj)
{
    obj[0] |= 0x80u;
    ObjList_FastInsert((int)(uintptr_t)DAT_80065a60, (int)(uintptr_t)obj);
}

/* HIGH-MED: gravity + rotation + terrain-collide per-tick. */
uint32_t Projectile_GravityTick(uint8_t *obj, int mode, int *unused)
{
    if (mode != 0) return 0xffffffffu;

    *(int32_t *)(obj + 0x24) += *(int32_t *)(obj + 0x88);
    *(int32_t *)(obj + 0x28) += *(int32_t *)(obj + 0x8c);
    *(int32_t *)(obj + 0x2c) += *(int32_t *)(obj + 0x90);

    Object_ApplyAngularVelocity((uint32_t *)(obj + 0x10),
                                (int)*(int16_t *)(obj + 0x80),
                                (int)*(int16_t *)(obj + 0x82),
                                (int)*(int16_t *)(obj + 0x84));

    int32_t vy = *(int32_t *)(obj + 0x8c) + 0x5a;
    *(int32_t *)(obj + 0x8c) = vy;
    if (vy > 0) {
        int32_t terrainY = Terrain_HeightAndProbe((int)(intptr_t)obj, (int *)(obj + 0x24), 0, 0);
        if (*(int32_t *)(obj + 0x28) > terrainY) return 0;   /* below ground = despawn */
    }
    (void)unused;
    return 0xffffffffu;
}
