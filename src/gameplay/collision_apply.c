/* collision_apply.c -- the core damage-on-collision arbiter.
 *
 * Source: SLUS_005.10  (gap-recovered)
 *   FUN_80022320  -- Damage_AccumulateOrFire
 *   FUN_8002239c  -- Damage_FromImpulse
 *
 * Every per-level destructible callback ends in a call to either
 * Damage_FromImpulse(self, impulse) or Damage_AccumulateOrFire(self,
 * amount).
 *
 * Damage_AccumulateOrFire(self, amount):
 *   - if self has the "invulnerable" bit 0x8000 set, ignore.
 *   - else if accumulated damage (`self.health` @ +0xc, i16) is less
 *     than `amount` (the new hit), reset health back to its max
 *     (stored at +0xe) and fire the explosion via Effects_SpawnExplosion;
 *     on success, broadcast event 0x11 on the world callback (`pcRam00000730`)
 *     and return 1 (destroyed).
 *   - else subtract `amount` from `self.health` and return 0.
 *
 * Damage_FromImpulse(self, impulse):
 *   - The impulse object's state byte at +4 must be 7 (= "weapon hit"
 *     state). The damage value is the i16 at impulse+0xc (weapon
 *     strength); pass through to Damage_AccumulateOrFire.
 *   - Returns true iff the destructible was killed.
 *
 * HIGH confidence: this closes the universal collision contract used
 * by every level's bridge/turret/transformer/etc. handler in pass 2.
 *
 * Vehicle struct updates: +0xc is now confirmed as the `health` field
 * (we previously had it as `statusFlags` LOW); +0xe is its max-health.
 * Pass 3 should rename Vehicle.statusFlags accordingly.
 */
#include <stdint.h>

extern int  Effects_SpawnExplosion(int obj);                /* FUN_8003fc50 */
extern void (*pcRam00000730)(void *self, int eventId, int);  /* world callback */

int Damage_AccumulateOrFire(uint32_t *self, uint16_t amount)
{
    if ((self[0] & 0x8000u) != 0) return 0;

    uint16_t hp = ((uint16_t *)self)[6];        /* +0xc, lo u16 of self[3] */
    if (hp < amount) {
        /* Reset to max -- self[3] high half = max-hp at +0xe */
        ((uint16_t *)self)[6] = ((uint16_t *)self)[7];
        if (Effects_SpawnExplosion((int)(intptr_t)self) != 0) {
            pcRam00000730(self, 0x11, 0);
            return 1;
        }
    } else {
        ((uint16_t *)self)[6] = (uint16_t)(hp - amount);
    }
    return 0;
}

int Damage_FromImpulse(uint32_t *self, int *impulse)
{
    if (*(int8_t *)(*impulse + 4) != 7) return 0;
    uint16_t weaponStrength = *(uint16_t *)(*impulse + 0xc);
    return Damage_AccumulateOrFire(self, weaponStrength);
}
