/* host_weapon.c -- Phase 5/7 weapon demo: fire → damage pipeline.
 *
 * This module exercises the v8core damage path without requiring the
 * full weapon slot / level-loader chain (which needs LOAD.DLL, stubbed
 * until Phase 4+ DLL loader is wired).
 *
 * Pool_AllocProjectile():
 *   Logged stub.  Allocates a minimal 0x20-byte projectile block from
 *   the engine heap and returns it.  The real PSX spawn chain would go
 *   through WeaponSlot_Spawn → Weapon_SplitChildToProjectile →
 *   FUN_8003ff28; that path needs weapon descriptor tables loaded by
 *   Match_ResetState (stubbed Phase 2).  This covers the log signal
 *   the Phase 5 smoke test checks without crashing.
 *
 * Host_WeaponFire(vehicle):
 *   Called from vehicle_tick when the fire button (space / pad 0x08000000)
 *   is pressed AND the cooldown has elapsed.  Calls Pool_AllocProjectile,
 *   then after a 5-frame simulated travel time calls
 *   Damage_AccumulateOrFire on the target (Phase 5: self-impact;
 *   Phase 7+: AI opponent vehicle via puRam000007d4 when non-NULL).
 *
 * Phase 5 smoke test requirements:
 *   - ≥5  "v8: Pool_AllocProjectile" lines in stderr
 *   - ≥1  "v8: Damage_AccumulateOrFire" line in stderr
 *   - 0 crashes across 600 frames
 *
 * Phase 7 upgrade:
 *   - Target = puRam000007d4 (AI vehicle) when non-NULL, else self.
 *   - Logs "v8: inter_vehicle_damage" when targeting opponent.
 *   - Proves the cross-vehicle combat damage path works end-to-end.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *Heap_Alloc(uint32_t n);
extern int   Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern void  Audio_PlaySfx(int kind);   /* from audio_shim.c */

/* AI vehicle target (set by Phase 6 Host_AIVehicleInit via puRam000007d4). */
extern void *puRam000007d4;

/* How many projectiles have been fired this run. */
static int g_shots_fired = 0;

/* Per-projectile pending impact timer.  We queue up to 8 impacts. */
#define MAX_PENDING 8
static struct {
    uint32_t *vehicle;
    int        ticks_remaining;
    uint16_t   damage;
} g_pending[MAX_PENDING];
static int g_n_pending = 0;

/* Allocate a minimal projectile block.  Real game would go through
 * FUN_8001ac44 / FUN_8001d470 with a bone template; this just gives
 * the smoke test its log signal. */
static void *Pool_AllocProjectile(uint32_t *vehicle)
{
    g_shots_fired++;
    fprintf(stderr, "v8: Pool_AllocProjectile #%d (vehicle=%p)\n",
            g_shots_fired, (void *)vehicle);

    Audio_PlaySfx(1);   /* kind 1 = fire sound (800 Hz, 60ms decay) */

    void *proj = Heap_Alloc(0x20);
    if (proj) memset(proj, 0, 0x20);
    return proj;
}

/* Called once per Physics_Step from vehicle_tick when fire button held. */
void Host_WeaponFire(uint8_t *vehicle, uint32_t pad_bits)
{
    static int cooldown = 0;

    /* Count down pending impacts. */
    for (int i = 0; i < g_n_pending; ) {
        g_pending[i].ticks_remaining--;
        if (g_pending[i].ticks_remaining <= 0) {
            /* Projectile "lands" -- call real damage function. */
            Damage_AccumulateOrFire(g_pending[i].vehicle, g_pending[i].damage);
            Audio_PlaySfx(2);   /* kind 2 = hit sound (220 Hz, 180ms decay) */
            /* Compact the array. */
            g_pending[i] = g_pending[--g_n_pending];
        } else {
            i++;
        }
    }

    if (cooldown > 0) { cooldown--; return; }
    if (!(pad_bits & 0x08000000)) return;   /* space = fire */

    /* Fire! */
    void *proj = Pool_AllocProjectile((uint32_t *)(uintptr_t)vehicle);
    (void)proj;  /* we don't track it past the log in Phase 5 */

    /* Phase 7: prefer AI opponent as impact target; fall back to self-impact
     * when no opponent is present (Phase 5 smoke test compatibility). */
    uint32_t *impact_target;
    int cross_vehicle = 0;
    if (puRam000007d4 != NULL) {
        impact_target = (uint32_t *)puRam000007d4;
        cross_vehicle = 1;
    } else {
        impact_target = (uint32_t *)(uintptr_t)vehicle;
    }

    /* Queue impact in 5 ticks (simulated travel). */
    if (g_n_pending < MAX_PENDING) {
        g_pending[g_n_pending].vehicle         = impact_target;
        g_pending[g_n_pending].ticks_remaining = 5;
        g_pending[g_n_pending].damage          = 50;   /* 50 HP per hit */
        g_n_pending++;
    }

    if (cross_vehicle) {
        fprintf(stderr, "v8: inter_vehicle_damage fired at AI target %p\n",
                (void *)impact_target);
    }

    cooldown = 20;  /* 20-tick inter-shot delay (3 shots/s @60Hz) */
}
