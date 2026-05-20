/* gondola.c -- Ski Resort gondola / lift moving platform integrator.
 *
 * Source: SKIRESRT.DLL  FUN_80101050.
 *
 * Variant of HD_DamLeverTick: when bit 0 of obj's status flag is set,
 * integrate position by velocity / 128 (same +0x7f sign-correction).
 * When that bit is clear, the gondola is stationary and the function
 * essentially no-ops (uVar3 = 0x10000 is dead code in that path).
 *
 * Used by gondola_1, liftpole_1, liftstation_1, Ball_1/2 (the
 * spinning lift wheels).
 *
 * MED confidence.
 */
#include <stdint.h>

uint32_t SR_GondolaTick(uint32_t *obj, int mode)
{
    if (mode != 0 && mode != 2) return 0;
    if ((obj[0] & 1) == 0) return 0;          /* moving flag */

    int32_t vx = (int32_t)obj[0x20];
    int32_t vy = (int32_t)obj[0x21];
    int32_t vz = (int32_t)obj[0x22];
    if (vx < 0) vx += 0x7f;
    if (vy < 0) vy += 0x7f;
    if (vz < 0) vz += 0x7f;

    obj[9]  += (uint32_t)(vx >> 7);
    obj[10] += (uint32_t)(vy >> 7);
    obj[11] += (uint32_t)(vz >> 7);
    return 0;
}
