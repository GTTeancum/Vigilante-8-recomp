/* ai_opponent.c -- second vehicle controlled by a simple host AI.
 *
 * The engine's per-level AI tick callbacks are panic-stubbed; this
 * is a host-side stand-in so we have something visible to shoot at
 * and to demonstrate vehicle-vs-vehicle damage in Phase 7.
 *
 * Behavior:
 *   - Spawns at a fixed offset from the player at boot.
 *   - Each tick, steers toward the player and accelerates.
 *   - Same bicycle dynamics as the player.
 *   - Fires every 60 frames at the player's direction.
 *
 * Health: tracked here; decremented when a projectile gets close.
 * On death (HP <= 0), respawns after a delay so smoke can keep
 * checking the AI is "alive".
 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define AI_MAX_SPEED  18.0f
#define AI_ACCEL       6.0f
#define AI_STEER       1.4f
#define AI_FIRE_PERIOD  60

/* Player pose (read-only). */
extern float g_veh_x, g_veh_y, g_veh_z, g_veh_yaw;

/* AI pose (exported for the renderer). */
float g_ai_x   = 12.0f;
float g_ai_y   = 0.0f;
float g_ai_z   = 12.0f;
float g_ai_yaw = 0.0f;
float g_ai_speed = 0.0f;
int   g_ai_hp  = 5;
int   g_ai_alive = 1;
int   g_ai_respawn_in = 0;

extern int g_v8_frame_count;
extern int Projectile_GetActive(float *out_xyz, int max);

/* Forward-declared projectile spawn (currently only player-fed; we
 * extend it to AI in a moment). */
int AI_GetActive(float *out_xyz);   /* returns 0 if dead, 1 if alive (xyz filled) */

/* Called once per game tick, after the player integrates. */
void AI_Tick(void)
{
    if (!g_ai_alive) {
        if (--g_ai_respawn_in <= 0) {
            g_ai_x = 12.0f; g_ai_y = 0.0f; g_ai_z = 12.0f;
            g_ai_yaw = 0.0f; g_ai_speed = 0.0f;
            g_ai_hp = 5;
            g_ai_alive = 1;
        }
        return;
    }

    const float dt = 1.0f / 60.0f;

    /* Steer toward player. */
    float dx = g_veh_x - g_ai_x;
    float dz = g_veh_z - g_ai_z;
    float target_yaw = atan2f(dx, dz);
    float delta = target_yaw - g_ai_yaw;
    /* wrap to [-pi, +pi] */
    while (delta >  3.14159f) delta -= 2.0f * 3.14159f;
    while (delta < -3.14159f) delta += 2.0f * 3.14159f;
    float steer = AI_STEER * dt;
    if      (delta >  steer) g_ai_yaw += steer;
    else if (delta < -steer) g_ai_yaw -= steer;
    else                     g_ai_yaw  = target_yaw;

    g_ai_speed += AI_ACCEL * dt;
    if (g_ai_speed > AI_MAX_SPEED) g_ai_speed = AI_MAX_SPEED;
    g_ai_speed *= (1.0f - 0.6f * dt);

    g_ai_x += sinf(g_ai_yaw) * g_ai_speed * dt;
    g_ai_z += cosf(g_ai_yaw) * g_ai_speed * dt;

    /* Damage check: any player projectile within 1.5 units of the AI? */
    float proj[64 * 3];
    int n = Projectile_GetActive(proj, 64);
    for (int i = 0; i < n; i++) {
        float pdx = proj[i*3+0] - g_ai_x;
        float pdy = proj[i*3+1] - g_ai_y - 0.5f;
        float pdz = proj[i*3+2] - g_ai_z;
        float d2 = pdx*pdx + pdy*pdy + pdz*pdz;
        if (d2 < 1.5f * 1.5f) {
            g_ai_hp--;
            fprintf(stderr, "v8: AI hit! hp=%d\n", g_ai_hp);
            if (g_ai_hp <= 0) {
                g_ai_alive = 0;
                g_ai_respawn_in = 60;
                fprintf(stderr, "v8: AI down (will respawn in 60 frames)\n");
            }
            break;
        }
    }
}

int AI_GetActive(float *out_xyz)
{
    if (!g_ai_alive) return 0;
    out_xyz[0] = g_ai_x;
    out_xyz[1] = g_ai_y;
    out_xyz[2] = g_ai_z;
    return 1;
}

int AI_Yaw(float *out_yaw)
{
    if (!g_ai_alive) return 0;
    *out_yaw = g_ai_yaw;
    return 1;
}

int AI_Hp(void)        { return g_ai_hp; }
int AI_Alive(void)     { return g_ai_alive; }
