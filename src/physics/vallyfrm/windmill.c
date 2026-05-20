/* windmill.c -- Valley Farms windmill spin tick.
 *
 * Source: VALLYFRM.DLL  FUN_801005e8.
 *
 * The Windmill_1 object spins around an axis: each tick it adds 0x88
 * to its angular velocity field at +0x44 (cast as i16) and -- when
 * not impulsed (state bit 0 clear) -- also accumulates that into
 * angles at +0x48 (yaw) and +0x4c (pitch). The two destination
 * angles share the same velocity register (obj[0x22/0x23] = vx/vy at
 * +0x88/+0x8c), so the windmill blades rotate around a tilted axis.
 *
 * MED confidence.
 */
#include <stdint.h>

uint32_t VF_WindmillTick(uint32_t *obj, int mode)
{
    if (mode == 2) return 0;
    if (mode != 0 && mode != 2 && mode != 3) return 0;

    *(int16_t *)(obj + 0x11) = (int16_t)((int16_t)obj[0x11] + 0x88);

    if ((obj[0] & 1) == 0) {
        obj[0x12] += obj[0x22];
        obj[0x13] += obj[0x23];
    }
    return 0;
}
