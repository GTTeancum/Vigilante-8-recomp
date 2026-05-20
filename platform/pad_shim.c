/* pad_shim.c -- pad input stubs + frame-cap enforcement + Phase 4 input.
 *
 * Pad_Tick is called once per game-tick. We use it as a single point to:
 *   1. Count frames against --frames N cap
 *   2. Read SDL keyboard (or --auto-drive synth) into the pad bit word
 *   3. Update the stub vehicle position based on pad input (Phase 4
 *      bypass for the panic-stubbed physics step)
 *
 * Bit layout (from src/skipped/pad_input.c):
 *   0x10000000 = Up      (accel)
 *   0x40000000 = Down    (brake)
 *   0x80000000 = Left    (steer)
 *   0x20000000 = Right   (steer)
 *   0x00000100 = Start
 *   0x08000000 = Cross / X (fire)
 *   0x00000040 = Select / "accept" used by some load screens
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#if defined(V8_HAVE_SDL)
#include <SDL.h>
#endif

extern int      g_v8_frame_count;
extern int      g_v8_frame_limit;
extern int      g_v8_auto_drive_frames;   /* Phase 4 synth-input cap */
extern int      g_v8_auto_fire_period;    /* Phase 5: synth fire every N frames */
extern uint32_t uRam0000062c;
extern uint32_t uRam00000630;
extern void     Platform_FrameTick(void);
extern void     Projectile_Tick(int fire_held);
extern int      Projectile_SpawnCount(void);

/* Stub vehicle storage (offsets per Vehicle struct evidence). */
extern uint8_t *puRam000007d0;   /* points at g_stub_vehicle_p1 */

/* --- Phase 4: vehicle pose (read by renderer) -------------------- */
float    g_veh_x = 0.0f;
float    g_veh_y = 0.0f;
float    g_veh_z = 0.0f;
float    g_veh_yaw = 0.0f;   /* radians */
float    g_veh_speed = 0.0f;

static uint32_t read_pad_input(void)
{
    uint32_t pad = 0;

#if defined(V8_HAVE_SDL)
    /* SDL keyboard scan. */
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    if (k) {
        if (k[SDL_SCANCODE_UP]    || k[SDL_SCANCODE_W]) pad |= 0x10000000;
        if (k[SDL_SCANCODE_DOWN]  || k[SDL_SCANCODE_S]) pad |= 0x40000000;
        if (k[SDL_SCANCODE_LEFT]  || k[SDL_SCANCODE_A]) pad |= 0x80000000;
        if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D]) pad |= 0x20000000;
        if (k[SDL_SCANCODE_SPACE]) pad |= 0x08000000;
        if (k[SDL_SCANCODE_RETURN])pad |= 0x00000100;
    }
#endif

    /* Auto-drive synth: forward + slight steer for N frames. */
    if (g_v8_auto_drive_frames > 0 && g_v8_frame_count < g_v8_auto_drive_frames) {
        pad |= 0x10000000;            /* accelerate */
        if ((g_v8_frame_count / 30) & 1) pad |= 0x20000000;  /* steer right intermittently */
    }
    /* Auto-fire synth: press fire every N frames. */
    if (g_v8_auto_fire_period > 0 && (g_v8_frame_count % g_v8_auto_fire_period) == 0) {
        pad |= 0x08000000;
    }
    return pad;
}

static void integrate_vehicle(uint32_t pad)
{
    /* Simple bicycle dynamics in renderer's world units.
     * Phase 4: visual feedback only. Phase 5 ties to engine physics. */
    const float dt = 1.0f / 60.0f;
    const float steer_rate = 1.6f;       /* rad/s */
    const float accel       = 8.0f;       /* units/s^2 */
    const float brake       = 16.0f;
    const float drag        = 0.6f;
    const float max_speed   = 25.0f;

    if (pad & 0x10000000) g_veh_speed += accel * dt;
    if (pad & 0x40000000) g_veh_speed -= brake * dt;
    if (pad & 0x80000000) g_veh_yaw   -= steer_rate * dt;
    if (pad & 0x20000000) g_veh_yaw   += steer_rate * dt;

    g_veh_speed -= g_veh_speed * drag * dt;
    if (g_veh_speed >  max_speed) g_veh_speed =  max_speed;
    if (g_veh_speed < -max_speed) g_veh_speed = -max_speed;

    g_veh_x += sinf(g_veh_yaw) * g_veh_speed * dt;
    g_veh_z += cosf(g_veh_yaw) * g_veh_speed * dt;
    /* y left at 0 (terrain sampling is Phase 5+) */
}

void Pad_Tick(void)
{
    g_v8_frame_count++;

    uint32_t pad = read_pad_input();
    uRam0000062c = pad;        /* P1 pad bits */
    uRam00000630 = 0;          /* P2 not driven */

    static int first_log = 1;
    if (first_log) {
        fprintf(stderr, "v8: Pad_Tick first call (frame=%d auto=%d pad=0x%08x)\n",
                g_v8_frame_count, g_v8_auto_drive_frames, pad);
        first_log = 0;
    }

    integrate_vehicle(pad);
    Projectile_Tick((pad & 0x08000000) != 0);
    extern void AI_Tick(void);
    extern int  AI_Hp(void);
    AI_Tick();
    Platform_FrameTick();      /* render + present + event pump */

    if (g_v8_frame_limit > 0 && g_v8_frame_count >= g_v8_frame_limit) {
        fprintf(stderr, "v8: frame cap %d reached in game-tick loop; exit\n",
                g_v8_frame_limit);
        fprintf(stderr, "v8: vehicle pos = (%.2f, %.2f, %.2f) yaw=%.2f speed=%.2f\n",
                g_veh_x, g_veh_y, g_veh_z, g_veh_yaw, g_veh_speed);
        fprintf(stderr, "v8: projectiles spawned = %d\n", Projectile_SpawnCount());
        fprintf(stderr, "v8: AI hp = %d\n", AI_Hp());
        extern float g_ai_x, g_ai_z;
        fprintf(stderr, "v8: AI pos = (%.2f, %.2f)\n", g_ai_x, g_ai_z);
        fflush(stderr);
        exit(0);
    }
}

void Pad_TickWithMode(int mode)
{
    (void)mode;
    /* Sub-tick variant; don't double-count or double-step the vehicle. */
}
