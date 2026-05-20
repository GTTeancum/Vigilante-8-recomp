/* manhole_tick.c -- Casino City manhole_1 per-frame physics.
 *
 * Source: CASNOCTY.DLL  FUN_801002bc.
 *
 * Combination spawner-tick:
 *   1. Decrement obj+0x80 countdown each frame.
 *   2. On reaching -1, spawn a particle (kind 0x20) with random aim
 *      from the rsin/rcos table, random vertical jitter on vy. The
 *      particle gets tick callback 0x8004042c (main-EXE projectile
 *      post-tick) and parent flag 0x410 (= "particle + auto-fade").
 *   3. Tick over the children chain (obj+0x38) and add +0x20 to each
 *      child's spin angle (+0x44).
 *
 * Used by manhole_1 to emit steam puffs every Nth frame.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern void Object_BumpSubstate_Or_FX(int obj);  /* FUN_8001d4f0 */
extern int16_t DAT_80060db4[];                    /* PSY-Q rsin/rcos pair table */
extern uint32_t _DAT_800737d8;

static uint32_t scale_4_12_signed(int32_t v, int32_t factor)
{
    int32_t r = v * factor;
    if (r < 0) r += 0xfff;
    return (uint32_t)(r >> 12);
}

uint32_t CC_ManholeTick(int obj, uint32_t mode, int *impulse)
{
    if (mode == 2) return 0;
    if (mode != 3 && mode != 0) return 0;

    int16_t *cd = (int16_t *)(intptr_t)(obj + 0x80);
    int16_t prev = *cd;
    *cd = (int16_t)(prev - 1);
    if (prev == 0) {
        uint32_t *p = Object_Pool_AllocFromBank(
            (void *)(uintptr_t)_DAT_800737d8, 0x20, 0xa0, 8);
        if (p != NULL) {
            uint32_t r0 = V8_RandNext();
            uint32_t aimIdx = (r0 & 0xfff);
            p[0] |= 0x410u;
            int16_t sin = DAT_80060db4[aimIdx * 2 + 0];
            int16_t cos = DAT_80060db4[aimIdx * 2 + 1];
            p[0x22] = scale_4_12_signed(*(int32_t *)(obj + 0x84), sin);
            p[0x24] = scale_4_12_signed(*(int32_t *)(obj + 0x84), cos);
            uint32_t r1 = V8_RandNext();
            int32_t vy = *(int32_t *)(obj + 0x88);
            p[0x12] = 0;
            p[0x23] = (uint32_t)(vy + ((int)r1 * vy >> 15));
            p[0x13] = 0;
            p[0x14] = 0;
            p[0x19] = 0x8004042c;       /* main-EXE projectile post-tick */
            Object_BumpSubstate_Or_FX(obj);
        }
        *cd = *(int16_t *)(obj + 0x82);
    }

    /* Update children. */
    for (int child = *(int *)(obj + 0x38); child != 0; child = *(int *)(child + 0x34)) {
        *(int16_t *)(child + 0x44) = (int16_t)(*(int16_t *)(child + 0x44) + 0x20);
    }
    (void)impulse;
    return 0;
}
