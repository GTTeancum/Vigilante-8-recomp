/* snow_emit.c -- Ski Resort snowflake emitter (rate-limited).
 *
 * Source: SKIRESRT.DLL  FUN_80101efc.
 *
 * On mode 9 (= "snowflake-tick"), if our spawn id matches the event's
 * param_3 (selecting only the chosen emitter) AND the global emitter
 * concurrency counter at _DAT_800659fc+0x94 is below 24, spawns a
 * 0x98-byte child snowflake of the parent's kind (+0xa) with random
 * rotation, scaled-down LOD (0x93c/0x1000 = ~57%), spawnId 1000, and
 * status flags 0x180.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern void Object_RandomizeRotation(uint32_t *obj);
extern int8_t *_DAT_800659fc;

uint32_t SR_SnowEmit(uint32_t *self, int mode, int eventId)
{
    if (mode != 1) {
        if (mode != 9) return 0;
        if (eventId != *(int16_t *)((uintptr_t)self + 6)) return 0;
        if (*(int32_t *)((uintptr_t)_DAT_800659fc + 0x94) > 0x17) return 0;

        uint32_t *p = Object_Pool_AllocFromBank((void *)(uintptr_t)self[0x16],
            *(uint16_t *)((uintptr_t)self + 0xa), 0x98, 0);
        Object_RandomizeRotation(p);
        int32_t lod = (int32_t)p[0x15] * 0x93c;
        if (lod < 0) lod += 0xfff;
        p[0x15] = (uint32_t)(lod >> 12);
        *(uint16_t *)((uintptr_t)p + 6) = 1000;
        *(uint8_t *)((uintptr_t)p + 9) = (uint8_t)V8_RandNext();
        *(uint16_t *)(p + 3) = *(uint16_t *)((uintptr_t)self + 0xe);
        p[0] |= 0x180u;
        p[0x12] = self[0x12];
        p[0x13] = self[0x13];
        p[0x14] = self[0x14];
    }
    return 0;
}
