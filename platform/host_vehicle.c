/* host_vehicle.c -- allocate a real Vehicle struct and register it
 * with the engine's world-object linked list.
 *
 * Physics_Step (FUN_8002131c) walks the list at piRam0000075c and
 * calls each entry's +0x64 callback.  We populate the list with a
 * single Vehicle whose tick is the engine's universal Object_GeneralTick
 * (FUN_80030c08): terrain probe + spring/drag + the cleaned bit-exact
 * Object_IntegrateAndOrient + post-integrate damping.
 *
 * The host's only job per tick is to translate pad bits into the
 * per-tick input fields the engine reads from the object struct.
 * Everything past that is the engine's own math.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *Heap_Alloc(uint32_t n);

/* Engine globals. */
extern void    *puRam000007d0;            /* player vehicle pointer */
extern uint32_t uRam0000062c;             /* P1 pad bits */

/* The bit-exact engine integrator (src/physics/object_integrate.c).
 * Routes through Object_ApplyAngularVelocity + Object_IntegrateAndOrient
 * + MatrixNormal -- the PSX physics core.
 *
 * ARCHITECTURE NOTE: per notes/unknowns.md "Player vehicle physics
 * architecture", the engine itself uses a HIERARCHICAL model: chassis
 * uses LAB_8002e2bc (audio-only tick, no integration), and motion
 * comes from 4 child WHEEL objects each with their own per-frame
 * tick + joint linkage via FUN_8001b2fc.  Vehicle_RollingTick
 * (gap_80030f34) IS in v8core and IS bit-exact, but installing it on
 * a flat (no-wheels) chassis produces wrong behavior because
 * Object_GeneralTick's +-0x200 angular nudges are for tumbling
 * objects, not driving cars.
 *
 * Until the wheel tick + joint chain is wired, the host drives the
 * integrator directly with host-side pad-to-vel mapping. */
extern void Object_IntegrateAndOrient(uint8_t *obj);
extern int32_t Vec3_Length(const int32_t *v);

/* The Vehicle struct is allocated in the engine heap (low memory).
 *
 * Multi-vehicle support will require a proper pack(4) linked-list at
 * piRam0000075c that the cleaned Physics_Step walks; for now
 * physics_shim.c invokes puRam000007d0's tick directly. */
static uint8_t *g_vehicle = NULL;

/* Vehicle tick callback.  Receives (self, mode, catchupFlag); the
 * host-side dispatcher calls with mode==0 per frame.
 *
 * Implementation: host-side pad-to-vel mapping (NOT the engine's per-
 * frame controller, which lives in the unwired wheel chain).  The
 * integrator chain it calls IS the engine's bit-exact one:
 *    Object_IntegrateAndOrient
 *        -> Object_ApplyAngularVelocity (GTE small-angle)
 *        -> pos += vel/128  with RTZ rounding
 *        -> MatrixNormal (per-row q12 renormalisation)
 *
 * Per-tick math is bit-exact; the INPUTS to it are host-driven until
 * the engine's wheel chain is decompiled. */
static void vehicle_tick(uint8_t *self, int mode, int catchupFlag)
{
    (void)catchupFlag;
    if (mode != 0) return;

    /* Pad bits -> velocity along the vehicle's current forward axis.
     * (Forward = matrix col 3 = (R13, R23, R33).)  The matrix sits
     * at obj+0x10 in packed form: u32[1] = R13|R21, u32[2] = R22|R23,
     * u32[4] = R33|pad. */
    uint32_t *m = (uint32_t *)(self + 0x10);
    int16_t r13 = (int16_t)(m[1] & 0xffff);
    int16_t r23 = (int16_t)(m[2] & 0xffff);
    int16_t r33 = (int16_t)(m[4] & 0xffff);

    int32_t accel = 0;
    if (uRam0000062c & 0x10000000) accel += 0x40000;
    if (uRam0000062c & 0x40000000) accel -= 0x40000;
    if (accel) {
        *(int32_t *)(self + 0x80) += (accel * r13) >> 12;
        *(int32_t *)(self + 0x84) += (accel * r23) >> 12;
        *(int32_t *)(self + 0x88) += (accel * r33) >> 12;
    }
    /* Linear damping -- 1/32 per tick. */
    *(int32_t *)(self + 0x80) -= *(int32_t *)(self + 0x80) >> 5;
    *(int32_t *)(self + 0x84) -= *(int32_t *)(self + 0x84) >> 5;
    *(int32_t *)(self + 0x88) -= *(int32_t *)(self + 0x88) >> 5;

    /* Yaw from steering. */
    int32_t yawRate = 0;
    if (uRam0000062c & 0x80000000) yawRate = -0x4000;
    if (uRam0000062c & 0x20000000) yawRate = +0x4000;
    *(int32_t *)(self + 0x90) = 0;
    *(int32_t *)(self + 0x94) = yawRate;
    *(int32_t *)(self + 0x98) = 0;

    /* Engine constants from FUN_80022d54 / FUN_80022e38 (object init):
     * obj+0xa4 baseline = 0, obj+0xa6 = 0x3c.  Set anyway so any future
     * engine-code path that consumes them sees the right values. */
    *(int16_t *)(self + 0xa4) = 0;
    *(int16_t *)(self + 0xa6) = 0x3c;

    /* The bit-exact integrator. */
    Object_IntegrateAndOrient(self);
}

void Host_VehicleInit(void)
{
    /* Allocate 0x200 bytes from the engine heap (low memory). */
    g_vehicle = (uint8_t *)Heap_Alloc(0x200);
    if (!g_vehicle) {
        fprintf(stderr, "v8: Host_VehicleInit -- OOM\n");
        return;
    }
    memset(g_vehicle, 0, 0x200);

    /* Identity rotation matrix (q12 -> 0x1000 = 1.0). MATRIX layout:
     *   u32[0] = R11|R12   = 0x00001000   (R11=0x1000, R12=0)
     *   u32[1] = R13|R21   = 0x00000000
     *   u32[2] = R22|R23   = 0x10000000   (R22=0x1000, R23=0)
     *   u32[3] = R31|R32   = 0x00000000
     *   u32[4] = R33|pad   = 0x00001000   (R33=0x1000)
     * (each u32 packs hi=odd-offset, lo=even-offset; i16 little-endian
     * means the LOW i16 occupies the LOW 16 bits) */
    uint32_t *m = (uint32_t *)(g_vehicle + 0x10);
    m[0] = 0x00001000;     /* R11=0x1000 */
    m[1] = 0x00000000;
    m[2] = 0x10000000;     /* R22=0x1000 (R22 is the HIGH half of u32[2]) */
    m[3] = 0x00000000;
    m[4] = 0x00001000;     /* R33=0x1000 */

    /* Spawn inside OilField's populated tile range (chunks cx=18-19,
     * cz=13-15). Pick the centre of chunk (18,14), cell (32,32):
     *   global cell x = 18*64 + 32 = 1184  -> world x = 1184 << 16 = 0x4a00000
     *   global cell z = 14*64 + 32 =  928  -> world z =  928 << 16 = 0x3a00000 */
    extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
    int32_t spawn_x = 0x4a00000;
    int32_t spawn_z = 0x3a00000;
    int32_t ground_y = Terrain_HeightAt((uint32_t)spawn_x, (uint32_t)spawn_z);
    *(int32_t *)(g_vehicle + 0x24) = spawn_x;
    /* Spawn a few units above ground.  PSX convention is +Y-down
     * (Projectile_GravityTick: gravity adds positive vy; obj.y >
     * terrain_y = below ground), so "above" means SMALLER Y.  The host
     * pad-to-vel driver doesn't apply gravity, so the spawn height
     * stays put until pad input. */
    *(int32_t *)(g_vehicle + 0x28) = ground_y - 0x40000;
    *(int32_t *)(g_vehicle + 0x2c) = spawn_z;

    /* Health field (used by damage path; not relevant for driving). */
    *(int16_t *)(g_vehicle + 0x0c) = 1000;         /* health */
    *(int16_t *)(g_vehicle + 0x0e) = 1000;         /* maxHealth */

    /* State byte (+0xd0) = 12 ("rolling").  This is the engine's
     * marker that triggers Vehicle_StateTransition to install
     * Vehicle_RollingTick as the per-frame callback.  We install it
     * directly below so the state byte is for forward-compat with
     * any engine code that reads it. */
    *(uint8_t *)(g_vehicle + 0xd0) = 12;

    /* +0x9c..+0xa0 = per-axis inverse inertia (i16 each).
     * Engine values read directly from the EXE template at
     * SLUS:0x8005ec10 (+0x14..+0x19):
     *    invInertiaX = invInertiaY = invInertiaZ = 0x40
     * That template is what Vehicle_RollingTick's mode-7 spawn path
     * (`lwl/lwr 0x14(s3); swl/swr 0x9c(s1)`) copies into newly-spawned
     * sub-projectiles.  Factual, no empirical fitting. */
    *(int16_t *)(g_vehicle + 0x9c) = 0x40;
    *(int16_t *)(g_vehicle + 0x9e) = 0x40;
    *(int16_t *)(g_vehicle + 0xa0) = 0x40;

    /* Tick callback pointer. The engine calls this every Physics_Step. */
    *(void (**)(uint8_t *, int, int))(g_vehicle + 0x64) = vehicle_tick;

    /* Set player-1 pointer. physics_shim.c's Physics_Step dispatches
     * via this pointer directly. */
    puRam000007d0 = g_vehicle;

    fprintf(stderr, "v8: host_vehicle initialized at %p (size 0x200)\n",
            (void *)g_vehicle);
}
