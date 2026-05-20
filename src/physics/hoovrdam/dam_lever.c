/* dam_lever.c -- Hoover Dam moving-platform integrator.
 *
 * Source: HOOVRDAM.DLL  FUN_801006cc.
 *
 * The Hoover Dam level has several moving objects (parking_meter
 * impacts, Dam_Lever, transformer doors). They share the same 1/128-
 * scaled position integrator as Sand Factory's conveyor (see
 * src/physics/sandfact/conveyor.c). The function:
 *
 *   1. Integrates position (obj[9..11]) by velocity (obj[0x20..22])
 *      divided by 128, with negative-correction +0x7f rounding.
 *   2. On external impulse (param_3 != 0), kicks a sound at obj+0x24
 *      via FUN_800446dc (likely SoundEmit_FromBlob) and applies a
 *      mix-down via FUN_80044574 keyed by obj[0xd3] (sound slot id).
 *   3. Advances the per-object frame counter at obj+8 by 1; the state
 *      machine continues into branches matching the counter value
 *      against the impulse param (state transitions deferred to
 *      pass 3).
 *
 * MED confidence on the integrator + sound emit; LOW on the state
 * machine branches (cases 1, 8 and the matched case = pass-3 work).
 */
#include <stdint.h>

extern uint32_t SoundEmit_FromBlob(void *posXyz);            /* FUN_800446dc */
extern void     SoundChannel_Apply(uint8_t slot);            /* FUN_80044574 */

uint32_t HD_DamLeverTick(uint32_t *obj, int mode, int impulse)
{
    if (mode != 0 && mode != 2) goto stateMachine;

    int32_t vx = (int32_t)obj[0x20];
    int32_t vy = (int32_t)obj[0x21];
    int32_t vz = (int32_t)obj[0x22];

    if (vx < 0) vx += 0x7f;
    if (vy < 0) vy += 0x7f;
    if (vz < 0) vz += 0x7f;

    obj[9]  += (uint32_t)(vx >> 7);
    obj[10] += (uint32_t)(vy >> 7);
    obj[11] += (uint32_t)(vz >> 7);

    if (impulse == 0) return 0;

    uint32_t soundId = SoundEmit_FromBlob(obj + 9);
    SoundChannel_Apply(((uint8_t *)obj)[0xd3]);
    (void)soundId;

stateMachine:
    {
        int8_t  *frameCounter = (int8_t *)(obj + 2);
        int      cur = *frameCounter;
        *frameCounter = (int8_t)(cur + 1);
        /* States 0, 1, 8 and the impulse-triggered "case" transition
         * for the lever open/close arc are handled by the central
         * level dispatcher in spillway_grab.c (FUN_80101734), which
         * owns the arm/disarm flag bit 0x10000 used to gate this
         * handler's sub-states. */
        (void)cur;
    }
    return 0;
}
