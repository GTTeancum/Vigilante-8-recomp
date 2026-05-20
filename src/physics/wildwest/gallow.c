/* gallow.c -- Wild West Gallow_1 destruction + event broadcast.
 *
 * Source: WILDWEST.DLL  FUN_80100fa4.
 *
 * Multi-mode handler:
 *   mode 0 (tick): if obj.health == 0 (already destroyed), apply
 *     damage to self; fall into damage processing.
 *   mode 3 (weapon hit): Damage_FromImpulse.
 *   mode 8 (forced hit): same as 3 sans the impulse check.
 *   Mode 2: Effects_SpawnExplosion(self) + broadcast.
 *   mode 9: when broadcast lands on us, set flag 0x10000 if id matches.
 *   mode 1 / 6: pass-through.
 *
 * Confirms the +0xc field as health (zero == dead).
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern void Damage_Apply(void *obj);
extern int  Object_DispatchList(void *listHead, uint32_t event, uint32_t arg);
extern int  Effects_SpawnExplosion(int obj);
extern uint8_t DAT_80065a18[];

uint32_t WW_GallowTick(uint32_t *self, uint32_t mode, uint32_t *impulse)
{
    switch (mode) {
    case 0:
        if (self[0xc] == 0) Damage_Apply(self);
        /* fall through */
    case 3:
        Damage_FromImpulse(self, (int *)impulse);
        /* fall through */
    case 8: {
        int hit = Damage_AccumulateOrFire(self, (uint16_t)(uintptr_t)impulse);
        if (hit != 0) {
            impulse = self;
            Object_DispatchList(DAT_80065a18, 9, 0);
            goto case2;
        }
        break;
    }
case2:
    case 2:
        Effects_SpawnExplosion((int)(uintptr_t)self);
        goto case9;
case9:
    case 9:
        if (*(int16_t *)((uintptr_t)impulse + 6) != *(int16_t *)((uintptr_t)self + 6)) return 0;
        self[0] |= 0x10000u;
        break;
    case 6:
    case 1:
    default:
        break;
    }
    return 0;
}
