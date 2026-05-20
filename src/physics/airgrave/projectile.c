/* projectile.c -- AirGrave dropped-projectile per-tick physics.
 *
 * Source: AIRGRAVE.DLL  FUN_8010059c.
 *
 * Registered as the tick callback (at object+0x19=0x64) on the
 * projectile that AGTracker_Tick spawns. Each tick the projectile:
 *
 *   1. Integrates position += velocity (x,y,z at +0x48..+0x50 ;
 *      vx,vy,vz at +0x88..+0x90).
 *   2. Re-aligns its yaw to point in the direction of vy via
 *      ratan2(vy, 0x1dcd) -- the 0x1dcd constant is the fixed forward
 *      speed scalar, so this picks a pitch angle from the current
 *      vertical velocity.
 *   3. Applies gravity: vy += 0x38 (PSX +Y = down, so this is
 *      acceleration toward the ground).
 *   4. Samples the terrain height at the projectile's (x, z) via
 *      Terrain_HeightAt. If the projectile has dipped below the
 *      ground (terrainY < projY), invokes the impact handlers
 *      FUN_8003fc50 (explosion effect) and FUN_800205f8 (damage
 *      query / scoring) and returns -1 to despawn.
 *   5. Otherwise, if the parent's lifetime byte (parent+4) hit 7,
 *      returns 0 (continue ticking) -- the explicit "kept alive" path.
 *
 * Bit-exact: gravity constant 0x38 and forward-speed scalar 0x1dcd are
 * preserved verbatim. The negated yaw store (`obj[0x40] = -ratan2(...)`)
 * matches the PSY-Q convention for pitch-down trajectories.
 *
 * Pass 3:
 *   - rename FUN_8003fc50 -> Effects_SpawnExplosion (when promoted).
 *   - rename FUN_800205f8 -> Damage_Apply (when promoted).
 *   - retype param_3 to the parent AGTracker* (currently int*).
 */
#include <stdint.h>

extern long ratan2(int y, int x);                  /* func_0x8004ecd4 */
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern void  Effects_SpawnExplosion(void *obj);   /* FUN_8003fc50 */
extern void  Damage_Apply           (void *obj);   /* FUN_800205f8 */
extern void  ImpactSparks_Spawn    (void *obj);   /* FUN_8001d708 */

#define PROJ_VX_OFF     0x88
#define PROJ_VY_OFF     0x8c
#define PROJ_VZ_OFF     0x90
#define PROJ_POSX_OFF   0x48
#define PROJ_POSY_OFF   0x4c
#define PROJ_POSZ_OFF   0x50
#define PROJ_YAW_OFF    0x40
#define GRAVITY_PER_TICK  0x38
#define FORWARD_SCALAR    0x1dcd

uint32_t AGProjectile_Tick(uint8_t *obj, int mode, int *parent)
{
    if (mode == 0 || mode != 3) {
        /* Integrate position. */
        int32_t *p = (int32_t *)obj;
        *(int32_t *)(obj + PROJ_POSX_OFF) += *(int32_t *)(obj + PROJ_VX_OFF);
        *(int32_t *)(obj + PROJ_POSY_OFF) += *(int32_t *)(obj + PROJ_VY_OFF);
        *(int32_t *)(obj + PROJ_POSZ_OFF) += *(int32_t *)(obj + PROJ_VZ_OFF);

        /* Re-aim yaw to point in current direction. */
        int32_t vy = *(int32_t *)(obj + PROJ_VY_OFF);
        *(int16_t *)(obj + PROJ_YAW_OFF) = (int16_t)-ratan2(vy, FORWARD_SCALAR);

        /* Apply gravity. */
        *(int32_t *)(obj + PROJ_VY_OFF) += GRAVITY_PER_TICK;

        int32_t terrainY = Terrain_HeightAt(*(uint32_t *)(obj + PROJ_POSX_OFF),
                                            *(uint32_t *)(obj + PROJ_POSZ_OFF));
        if (terrainY < *(int32_t *)(obj + PROJ_POSY_OFF)) {
            ImpactSparks_Spawn(obj);
            goto impact;
        }
        ImpactSparks_Spawn(obj);    /* hovering sparks while in flight */
        (void)p;
        parent = (int *)(obj + PROJ_VX_OFF);
    }

    if (*(int8_t *)(*parent + 4) == 7) return 0;

impact:
    Effects_SpawnExplosion(obj);
    Damage_Apply(obj);
    return 0xffffffffu;
}
