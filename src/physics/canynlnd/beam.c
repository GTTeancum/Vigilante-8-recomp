/* beam.c -- Canyonlands BeamUp/overpass tick.
 *
 * Source: CANYNLND.DLL  FUN_80100c50.
 *
 * Each tick on mode 0:
 *   1. Run the universal physics step Object_IntegrateAndOrient.
 *   2. If status bit 1 is clear, subtract 0x1680 from obj[0x21]
 *      (the vertical velocity-Y? the level's flying objects fall
 *      under gravity at 0x1680/tick which is much heavier than the
 *      AirGrave drop -- this is a "lifting beam" yanking the
 *      object UP, so the subtraction acts as upward thrust).
 *   3. Cache last-frame pos (obj[9..0xb]) into prev-pos slots
 *      (obj[0x12..0x14]) for the next tick's interpolation.
 *
 * Used by `BeamUp` and `overpass_1`.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

extern void Object_IntegrateAndOrient(uint8_t *obj);   /* FUN_80017324 */

uint32_t CL_BeamTick(uint32_t *obj, int mode)
{
    if (mode != 0) return 0;
    Object_IntegrateAndOrient((uint8_t *)obj);
    if ((obj[0] & 2u) == 0) {
        obj[0x21] -= 0x1680;       /* upward thrust */
    }
    obj[0x12] = obj[9];
    obj[0x13] = obj[10];
    obj[0x14] = obj[11];
    return 0;
}
