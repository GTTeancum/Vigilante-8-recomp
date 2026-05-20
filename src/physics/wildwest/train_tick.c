/* train_tick.c -- Wild West M1train_engine path-following tick.
 *
 * Source: WILDWEST.DLL  FUN_8010072c.
 *
 * Train cars follow a per-level spline path. The tick:
 *   1. If currently overshooting the next waypoint (obj[0x29] = waypoint
 *      handle, obj[0x2a] = velocity), decelerate by 0xe per tick.
 *   2. Sample the spline at the rounded current `t` (obj[0x2c] >> 16)
 *      via the path sampler at func_0x80042390. Result is a delta
 *      vector in `local_28`.
 *   3. If we're going backwards (obj[0x2b] == 0), negate dx/dz.
 *   4. Normalize the delta and store it into obj+0x10..0x14 (the
 *      forward-direction columns of the rotation matrix) with the
 *      perpendicular as +/-Z.
 *   5. Compute step magnitude via SquareRoot0(dx² + dz²) and
 *      advance the path parameter by (velocity * -0x10000) / magnitude.
 *   6. On overshoot, fetch the next waypoint via func_0x80042698.
 *
 * MED.
 */
#include <stdint.h>

extern int  Path_Sample(int waypoint, int t, uint32_t *posInOut, int32_t *deltaOut);  /* func_0x80042390 */
extern int16_t *VectorNormalS(int32_t *in, int16_t *out);  /* PSY-Q */
extern long SquareRoot0(int n);
extern int  Path_NextWaypoint(int waypoint, uint32_t *direction);  /* func_0x80042698 */

uint32_t WW_TrainTick(uint32_t *self)
{
    if (self[0x29] == 0) return 0;

    /* Decelerate near the end of segment. */
    if ((self[0] & 0x10000u) != 0) {
        int32_t vel = (int32_t)self[0x2a] - 0xe;
        if (vel < 0) vel = 0;
        self[0x2a] = (uint32_t)vel;
    }

    /* Sample path. */
    uint32_t t = self[0x2c];
    if ((int32_t)t < 0) t += 0xffff;
    int32_t delta[4];
    Path_Sample((int)self[0x29], (int)t >> 16, self + 9, delta);
    delta[1] = 0;
    if (self[0x2b] == 0) {
        delta[0] = -delta[0];
        delta[2] = -delta[2];
    }
    int16_t dirS[4];
    VectorNormalS(delta, dirS);

    /* Write into rotation matrix columns. */
    *(int16_t *)(self + 4) = 0;        /* +0x10 */
    *(int16_t *)(self + 8) = 0;        /* +0x20 */
    *(int16_t *)((uint8_t *)self + 0x14) = dirS[0];
    *(int16_t *)((uint8_t *)self + 0x1c) = -dirS[0];

    int32_t mag = (int32_t)SquareRoot0(delta[0] * delta[0] + delta[2] * delta[2]);
    if (self[0x2b] != 0) mag = 1;
    uint32_t advance = self[0x2c] + (uint32_t)((int32_t)self[0x2a] * -0x10000 / mag);
    self[0x2c] = advance;
    if (advance < 0x10000001u) return 0;

    /* Overshoot -- pick next waypoint. */
    self[0x29] = (uint32_t)Path_NextWaypoint((int)self[0x29], &self[0x2b]);
    return 0;
}
