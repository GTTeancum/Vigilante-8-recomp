/* factory_collide.c -- Sand Factory generic terrain-collide stub.
 *
 * Source: SANDFACT.DLL  FUN_801010c4.
 *
 * The shortest possible per-object handler. On any mode != 3, the obj
 * pointer is clobbered to 1 (the preserved sentinel-as-fake-obj idiom
 * we've seen elsewhere) before falling through to the universal
 * terrain-collision test. On mode 3, the real obj is passed.
 *
 * Used by `factory_door` and the small static fixtures that need only
 * the terrain test (no spin, no damage broadcast).
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern int Collision_AgainstTerrain(int obj, int impact);   /* func_0x8002239c */

void SF_TerrainOnlyImpact(int obj, int mode, int impact)
{
    if (mode != 3) obj = 1;
    Collision_AgainstTerrain(obj, impact);
}
