/* lift.c -- Ski Resort lift station event handler.
 *
 * Source: SKIRESRT.DLL  FUN_80101284.
 *
 * Per-event dispatch:
 *   mode 3 (collision with weapon): if collider state == 3 and we
 *     have a paired ski-lift attached at obj[0x20], swap the collider
 *     to that paired object and trigger its tick callback with mode 3.
 *   mode 8: damage circular hit.
 *   mode 2: clear flag bit 0x20.
 *   mode 6 / 1 (init / re-init): claim a slot in the global lift
 *     register at _DAT_800659fc+0x98 (up to 2 simultaneous lifts) and
 *     set roll angle to 0x800 if slot index = 1.
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern int8_t *_DAT_800659fc;

uint32_t SR_LiftStation(uint32_t *self, int mode, uint32_t *impulse)
{
    switch (mode) {
    case 3: {
        uint32_t collider = impulse[0];
        if (*(int8_t *)(collider + 4) == 3 && self[0x20] != 0) {
            impulse[0] = self[0x20];
            typedef int16_t (*TickFn)(uint32_t, int);
            TickFn fn = *(TickFn *)(collider + 100);
            if (fn != NULL) fn(collider, 3);
        }
        Damage_FromImpulse(self, (int *)impulse);
        /* fall through */
    }
    case 8:
        Damage_AccumulateOrFire(self, (uint16_t)(uintptr_t)impulse);
        /* fall through */
    case 2:
        self[0] &= ~0x20u;
        /* fall through */
    case 6:
    case 1: {
        int registerSlot = (*(int *)((uintptr_t)_DAT_800659fc + 0x98) != 0);
        self[0] |= 0x108u;
        *(uint32_t **)((uintptr_t)_DAT_800659fc + registerSlot * 4 + 0x98) = self;
        *(uint16_t *)((uint8_t *)self + 0x42) = (uint16_t)(registerSlot << 0xb);
        return 0;
    }
    default:
        return 0;
    }
}
