/* proximity_dispatch.c -- broadcast event to objects within a Chebyshev radius.
 *
 * Source: SLUS_005.10  FUN_800422d8.
 *
 * Walks the per-frame object chain at piRam000008c4. For each node,
 * computes the Chebyshev (max) distance between (param_1, param_2)
 * and the node's (+3, +5) position fields. If that distance is less
 * than (+6 + param_3) (the per-object radius + caller's tolerance),
 * dispatches to FUN_800420f4 (the per-frame audio / trigger handler).
 *
 * Used for proximity-based audio cues (engine echo, wind tunnel, etc).
 *
 * HIGH.
 */
#include <stdint.h>

extern void Proximity_Trigger(void);   /* FUN_800420f4 */
extern int32_t **piRam000008c4;

void Proximity_DispatchInRange(int posX, int posZ, int tolerance)
{
    int32_t *node = (int32_t *)piRam000008c4[0];
    int32_t **prev = piRam000008c4;
    while (node != NULL) {
        int32_t dx = prev[3] - posX;
        int32_t dz = prev[5] - posZ;
        if (dx < 0) dx = -dx;
        if (dz < 0) dz = -dz;
        int32_t cheby = (dx > dz) ? dx : dz;
        if (cheby < (int32_t)prev[6] + tolerance) {
            Proximity_Trigger();
        }
        prev = (int32_t **)node;
        node = (int32_t *)*node;
    }
}
