/* radar.c -- Secret Base radar dish per-tick spin.
 *
 * Source: SCRTBASE.DLL  FUN_801004a8.
 *
 * The Secret Base level features a rotating radar dish (`radar_1`).
 * Its per-tick callback increments the dish's child-object roll
 * (object referenced at obj[0xe] = a sub-object pointer) by 0x11 each
 * tick, producing a slow spin. When an external impulse arrives
 * (param_3 != NULL), spawns impact sparks (FUN_8001d708) at the
 * child's position and, if the top-level flag bit 0x80 is set, runs a
 * brief secondary effect.
 *
 * Sub-object layout (offset within the child @obj[0xe]):
 *   +0x42  i16  roll angle (the dish's spin)
 *
 * Cases 3 (impact) and 4 (retire) are handled by the level main
 * dispatcher in radar_sweep.c (FUN_80100200); the per-dish handler
 * itself only spins.
 *
 * MED confidence.
 */
#include <stdint.h>

extern void ImpactSparks_Spawn(void *obj);    /* FUN_8001d708 */

uint32_t SCRT_RadarTick(uint32_t *obj, int mode, int *impulse)
{
    switch (mode) {
    case 0: {
        uint8_t *child = (uint8_t *)(uintptr_t)obj[0xe];
        *(int16_t *)(child + 0x42) = (int16_t)(*(int16_t *)(child + 0x42) + 0x11);
        if (impulse == NULL) return 0;
        ImpactSparks_Spawn(child);
        if ((*obj & 0x80) == 0) return 0;
        /* fall through to a small effect step (state 3/4 in original) */
        break;
    }
    case 3:
        /* impactor event -- forwarded to the SB damage dispatcher in
         * radar_sweep.c (FUN_80100200). This dish handler only spins;
         * damage routes through the level's main handler. */
        break;
    case 4:
        /* silent retire -- nothing to free; the spinning dish has no
         * heap or FX allocation of its own. */
        break;
    default: break;
    }
    return 0;
}
