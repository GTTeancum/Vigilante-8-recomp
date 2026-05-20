/* projectile_shim.c -- host-side projectile pool for Phase 5.
 *
 * The engine's Pool_AllocProjectile / projectile-tick callbacks are
 * panic stubs. To get "semi-believable" weapon fire, we maintain a
 * simple host-side projectile pool that:
 *   - Spawns on space press (debounced)
 *   - Integrates position by per-frame velocity
 *   - Decays a TTL so projectiles don't accumulate forever
 *   - Exposes the active set to the renderer
 *
 * Phase 5+ would wire this back through the engine's projectile path
 * (Pool_AllocProjectile + per-projectile tick callbacks across the
 * level DLLs) once those are no longer stubs.
 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define MAX_PROJ 64
#define PROJ_TTL 90        /* ~1.5 seconds at 60 Hz */
#define PROJ_SPEED 45.0f   /* world units/sec */

typedef struct {
    float x, y, z;
    float vx, vy, vz;
    int   ttl;             /* 0 = inactive */
} Projectile;

static Projectile g_proj[MAX_PROJ];
static int        g_fire_held_last = 0;
static int        g_proj_spawn_count = 0;   /* lifetime spawn count for smoke */

extern float g_veh_x, g_veh_y, g_veh_z, g_veh_yaw, g_veh_speed;

/* Spawn one projectile from current vehicle pose. Returns slot or -1. */
static int spawn_projectile(void)
{
    for (int i = 0; i < MAX_PROJ; i++) {
        if (g_proj[i].ttl == 0) {
            g_proj[i].x = g_veh_x + sinf(g_veh_yaw) * 1.5f;
            g_proj[i].y = g_veh_y + 1.0f;
            g_proj[i].z = g_veh_z + cosf(g_veh_yaw) * 1.5f;
            g_proj[i].vx = sinf(g_veh_yaw) * PROJ_SPEED;
            g_proj[i].vy = 0.0f;
            g_proj[i].vz = cosf(g_veh_yaw) * PROJ_SPEED;
            g_proj[i].ttl = PROJ_TTL;
            g_proj_spawn_count++;
            return i;
        }
    }
    return -1;
}

/* Called once per game tick from pad_shim.c, AFTER vehicle integration.
 * `fire_held` is 1 if the fire-bit is currently held. */
void Projectile_Tick(int fire_held)
{
    /* Edge-detect: fire only on a fresh press (rising edge). */
    if (fire_held && !g_fire_held_last) {
        spawn_projectile();
        extern void Audio_PlaySfx(int kind);
        Audio_PlaySfx(1);   /* fire beep */
    }
    g_fire_held_last = fire_held;

    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < MAX_PROJ; i++) {
        if (g_proj[i].ttl == 0) continue;
        g_proj[i].x += g_proj[i].vx * dt;
        g_proj[i].y += g_proj[i].vy * dt;
        g_proj[i].z += g_proj[i].vz * dt;
        g_proj[i].ttl--;
    }
}

/* Read-only accessor for the renderer. Returns the active set. */
int Projectile_GetActive(float *out_xyz, int max)
{
    int n = 0;
    for (int i = 0; i < MAX_PROJ && n < max; i++) {
        if (g_proj[i].ttl == 0) continue;
        out_xyz[n*3 + 0] = g_proj[i].x;
        out_xyz[n*3 + 1] = g_proj[i].y;
        out_xyz[n*3 + 2] = g_proj[i].z;
        n++;
    }
    return n;
}

int Projectile_SpawnCount(void) { return g_proj_spawn_count; }
