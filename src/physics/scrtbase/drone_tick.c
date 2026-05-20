/* drone_tick.c -- Secret Base proto-saucer / aurora drone tick.
 *
 * Source: SCRTBASE.DLL  FUN_8010100c.
 *
 * Drift-and-decay tick: position += velocity, velocity *= 0.969
 * (= 0xf80/0x1000 in 4.12), heading += yawRate. Then spawn impact
 * sparks each tick (FUN_8001d708) and call Damage_Apply.
 *
 * Used by the protoSaucer and aurora_1 -- both are slowly-decaying
 * drones that emit sparks and self-damage until destroyed.
 *
 * MED.
 */
#include <stdint.h>

extern void ImpactSparks_Spawn(void *obj);    /* FUN_8001d708 */
extern void Damage_Apply(void *obj);

uint32_t SB_DroneTick(int obj, int mode)
{
    if (mode == 0 || mode != 5) {
        *(int32_t *)(obj + 0x48) += *(int32_t *)(obj + 0x88);
        *(int32_t *)(obj + 0x4c) += *(int32_t *)(obj + 0x8c);
        *(int32_t *)(obj + 0x50) += *(int32_t *)(obj + 0x90);
        *(int32_t *)(obj + 0x88) = *(int32_t *)(obj + 0x88) * 0xf80 >> 12;
        *(int32_t *)(obj + 0x8c) = *(int32_t *)(obj + 0x8c) * 0xf80 >> 12;
        *(int32_t *)(obj + 0x90) = *(int32_t *)(obj + 0x90) * 0xf80 >> 12;
        *(int16_t *)(obj + 0x44) = (int16_t)(*(int16_t *)(obj + 0x44)
                                             + *(int16_t *)(obj + 0x96));
        ImpactSparks_Spawn((void *)(intptr_t)obj);
    }
    Damage_Apply((void *)(intptr_t)obj);
    return 0xffffffffu;
}
