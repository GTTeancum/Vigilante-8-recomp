/* sphere_test.c -- sphere-vs-obstacle proximity test + result post.
 *
 * Source: LOAD.DLL  FUN_80100fa8.
 *
 * For an obstacle at obj+0x12/+0x18/+0x1e (3 i16 packed: yaw +
 * extent +pitch?) checks if its bounding sphere (via Sphere_Test
 * at func_0x80016810) is non-zero w.r.t. param_3. On non-hit,
 * computes the delta from `param_2` (probe pos) to obj+0x48..0x50
 * (collision center), normalises (FUN_80016a20), transforms via
 * FUN_80016b08, and re-runs the sphere test with the new direction.
 *
 * Used in collision-cast pipelines (e.g., projectile vs static
 * obstacles).
 *
 * MED.
 */
#include <stdint.h>

extern int  Sphere_Test(uint32_t obstacleHandle, void *probe);   /* func_0x80016810 */
extern void Vec3_Normalize(int32_t *vec);                         /* FUN_80016a20 */
extern uint32_t Vec3_Project(int32_t *vec, void *outScratch);     /* FUN_80016b08 */

uint32_t Sphere_TestWithBacktrack(int obj, int *probePos, uint32_t obstacle)
{
    int16_t packed[3];
    packed[0] = *(int16_t *)(obj + 0x12);
    packed[1] = *(int16_t *)(obj + 0x18);
    packed[2] = *(int16_t *)(obj + 0x1e);

    int hit = Sphere_Test(obstacle, packed);
    if (hit >= 0) return 0;       /* clean hit -- no need to backtrack */

    int32_t delta[3];
    delta[0] = probePos[0] - *(int32_t *)(obj + 0x48);
    delta[1] = probePos[1] - *(int32_t *)(obj + 0x4c);
    delta[2] = probePos[2] - *(int32_t *)(obj + 0x50);

    Vec3_Normalize(delta);
    uint8_t scratch[8];
    uint32_t projected = Vec3_Project(delta, scratch);
    Sphere_Test(projected, packed);
    return 0;
}
