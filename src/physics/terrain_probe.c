/* terrain_probe.c -- combined terrain + obstacle height probe.
 *
 * Source: SLUS_005.10  FUN_8001d748.
 *
 * Given an object and a world (x, z) point, returns the highest
 * surface y the object would stand on, considering:
 *
 *   1. The terrain heightmap (Terrain_HeightAt).
 *   2. The object's two attached obstacle chains (+0x74 and +0x78) --
 *      each is a kd-tree of trigger volumes that Object_FindObstacleAt
 *      (FUN_8001f51c) tests against and returns the override y for.
 *
 * If the obstacle test misses, optionally fills the normal (param_3)
 * via Terrain_NormalAt and the material id (param_4) via
 * Terrain_MaterialAt. If the obstacle test hits, the override y
 * displaces the terrain y entirely.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z);
extern void  Terrain_NormalAt(uint32_t x, uint32_t z, SVECTOR *out);  /* FUN_80025648 */
extern long  VectorNormalSS(SVECTOR *a, SVECTOR *out);
extern int   Object_FindObstacleAt(int chainHead, int terrainY, int *posXyz, SVECTOR *normalOut);  /* FUN_8001f51c */

int Terrain_HeightAndProbe(int obj, int *posXyz, SVECTOR *normalOut, uint32_t *materialOut)
{
    int terrainY = Terrain_HeightAt((uint32_t)posXyz[0], (uint32_t)posXyz[2]);

    int yResult;
    int chainA = *(int *)(obj + 0x74);
    if (chainA != 0
        && (yResult = Object_FindObstacleAt(chainA, terrainY, posXyz, normalOut)) != 0) {
        return yResult;
    }
    int chainB = *(int *)(obj + 0x78);
    if (chainB != 0
        && (yResult = Object_FindObstacleAt(chainB, terrainY, posXyz, normalOut)) != 0) {
        return yResult;
    }

    /* No obstacle hit -- use terrain. Fill normal + material if requested. */
    if (normalOut != NULL) {
        Terrain_NormalAt((uint32_t)posXyz[0], (uint32_t)posXyz[2], normalOut);
        VectorNormalSS(normalOut, normalOut);
    }
    if (materialOut != NULL) {
        *materialOut = (uint32_t)(uintptr_t)Terrain_MaterialAt(
            (uint32_t)posXyz[0], (uint32_t)posXyz[2]);
    }
    return terrainY;
}
