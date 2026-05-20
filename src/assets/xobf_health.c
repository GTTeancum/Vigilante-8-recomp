/* xobf_health.c -- XOBF chunk loader: per-object health/maxHealth.
 *
 * Source: LOAD.DLL  FUN_80100d1c.
 *
 * Reads 1 or 2 i32 values from the XOBF stream (a payload-size > 4
 * means the second one is present) for health and maxHealth. If the
 * object type at +4 == 5 (= "scaled by difficulty") AND spawnId >= 0,
 * scales both values by (g_difficulty + 2)/4 = round-up integer
 * scaling. Then writes:
 *
 *   obj + 0xc = health     (matches our struct field)
 *   obj + 0xe = maxHealth  (matches our struct field)
 *
 * Also propagates parent's health to any child whose own health was
 * still 0 -- inheritance of the parent's HP when the child wasn't
 * explicitly given one.
 *
 * HIGH confidence: the +0xc / +0xe layout matches Damage_AccumulateOrFire
 * and our previously-fixed struct.
 */
#include <stdint.h>

extern int32_t XobfStream_ReadI32(void *stream);   /* func_0x800224ec */
extern int32_t DAT_8006531a;                        /* g_difficulty */

void XOBF_LoadHealth(int obj, void *stream, int payloadSize)
{
    int32_t health    = XobfStream_ReadI32(stream);
    int32_t maxHealth = health;
    if (payloadSize > 4) {
        maxHealth = XobfStream_ReadI32(stream);
    }

    if (*(int8_t *)(obj + 4) == 5 && *(int16_t *)(obj + 6) >= 0) {
        int32_t difficulty = DAT_8006531a + 2;
        int32_t v0 = health * difficulty;
        if (v0 < 0) v0 += 3;
        v0 >>= 2;
        int32_t v1 = maxHealth * difficulty;
        if (v1 < 0) v1 += 3;
        v1 >>= 2;
        health    = v0;
        maxHealth = v1;
    }

    *(int16_t *)(obj + 0xc) = (int16_t)health;
    *(int16_t *)(obj + 0xe) = (int16_t)maxHealth;

    /* Propagate to any child whose own health is still 0. */
    for (int child = *(int *)(obj + 0x38); child != 0; child = *(int *)(child + 0x34)) {
        if (*(int16_t *)(child + 0xc) == 0) {
            *(int16_t *)(child + 0xc) = (int16_t)health;
        }
    }
}
