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
extern void Host_WeaponFire(uint8_t *vehicle, uint32_t pad_bits);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
/* Host-side terrain fallback. Host_TerrainLoad expands ZONE chunks into
 * the original runtime 0x3000-byte format, so use the bit-exact engine
 * Terrain_HeightAt directly. */
extern uintptr_t DAT_800911a0[32 * 32];
extern uint8_t   g_terrain_tile_x_min, g_terrain_tile_x_max;
extern uint8_t   g_terrain_tile_z_min, g_terrain_tile_z_max;
extern int32_t   Terrain_HeightAt(uint32_t x, uint32_t z);

/* ZONE-based height (6-chunk fallback for areas not covered by XOBF). */
static int32_t Host_TerrainH8At(int32_t pos_x, int32_t pos_z)
{
    return Terrain_HeightAt((uint32_t)pos_x, (uint32_t)pos_z);
}

/* XOBF mesh height query (terrain_mesh.c).  Returns 1 on low/upward ground. */
extern int TerrainMesh_HeightAt(float wx, float wz, float *out_gl_y);
extern int TerrainMesh_ObstacleHeightAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                        int32_t terrain_y, int32_t *out_y);

/* Primary host floor query.
 * The decoded XOBF visual triangle store includes object roofs/walls and is
 * not a safe physics source.  Keep the host vehicle on the original
 * ZONE/table floor until the object obstacle tree is wired 1:1. */
static int32_t Host_GroundAt(int32_t pos_x, int32_t pos_y, int32_t pos_z)
{
    int32_t terrain_y = Host_TerrainH8At(pos_x, pos_z);
    int32_t obstacle_y = 0;
    if (TerrainMesh_ObstacleHeightAt(pos_x, pos_y, pos_z, terrain_y, &obstacle_y)) {
        return obstacle_y;
    }
    return terrain_y;
}

/* The Vehicle struct is allocated in the engine heap (low memory).
 *
 * Multi-vehicle support will require a proper pack(4) linked-list at
 * piRam0000075c that the cleaned Physics_Step walks; for now
 * physics_shim.c invokes puRam000007d0's tick directly. */
static uint8_t *g_vehicle = NULL;

static void trace_obj_matrix(const char *tag, const uint8_t *obj)
{
    if (obj == NULL || getenv("V8_TRACE_SPAWN_MATRIX") == NULL)
        return;

    const int16_t *m = (const int16_t *)(obj + 0x10);
    fprintf(stderr,
            "v8: spawn_matrix %s obj=%p id=%d euler=(%d,%d,%d) "
            "pos=(0x%x,0x%x,0x%x) mat=[%d,%d,%d;%d,%d,%d;%d,%d,%d]\n",
            tag, (const void *)obj, (int)*(const int16_t *)(obj + 0x06),
            (int)*(const int16_t *)(obj + 0x40),
            (int)*(const int16_t *)(obj + 0x42),
            (int)*(const int16_t *)(obj + 0x44),
            (unsigned)*(const uint32_t *)(obj + 0x24),
            (unsigned)*(const uint32_t *)(obj + 0x28),
            (unsigned)*(const uint32_t *)(obj + 0x2c),
            m[0], m[1], m[2],
            m[3], m[4], m[5],
            m[6], m[7], m[8]);
}

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

    /* Gravity: accumulate downward velocity each tick (PSX Y-down: positive = down).
     * 0xC000 per tick gives ~10.5 m/s² effective free-fall acceleration. */
    *(int32_t *)(self + 0x84) += 0xC000;

    int32_t accel = 0;
    if (uRam0000062c & 0x10000000) accel += 0x40000;
    if (uRam0000062c & 0x40000000) accel -= 0x40000;
    if (accel) {
        /* Horizontal drive only -- Y component is controlled by gravity +
         * terrain snap, not by the pad. On flat ground r23=0 anyway. */
        *(int32_t *)(self + 0x80) += (accel * r13) >> 12;
        *(int32_t *)(self + 0x88) += (accel * r33) >> 12;
    }
    /* Lateral damping (X and Z only -- do NOT damp vy so gravity accumulates). */
    *(int32_t *)(self + 0x80) -= *(int32_t *)(self + 0x80) >> 5;
    *(int32_t *)(self + 0x88) -= *(int32_t *)(self + 0x88) >> 5;

    /* Yaw from steering. */
    int32_t yawRate = 0;
    if (uRam0000062c & 0x80000000) yawRate = -0x4000;
    if (uRam0000062c & 0x20000000) yawRate = +0x4000;
    *(int32_t *)(self + 0x90) = 0;
    *(int32_t *)(self + 0x94) = yawRate;
    *(int32_t *)(self + 0x98) = 0;

    /* Steering debug: log every 60 frames when a direction key is held. */
    static int s_dbg_tick = 0;
    s_dbg_tick++;
    if (yawRate != 0 && (s_dbg_tick % 60) == 0) {
        /* R13, R33 = forward vector components (local Z -> world X, world Z).
         * yaw applied at +0x94.  pos_x/z from struct. */
        int32_t px = *(int32_t *)(self + 0x24);
        int32_t pz = *(int32_t *)(self + 0x2c);
        int32_t vx = *(int32_t *)(self + 0x80);
        int32_t vz = *(int32_t *)(self + 0x88);
        fprintf(stderr,
            "v8: steer pad=0x%08x yawRate=%+d"
            "  fwd=(R13=%+d,R33=%+d)"
            "  vel=(vx=%+d,vz=%+d)"
            "  pos=(0x%x,0x%x)\n",
            uRam0000062c, yawRate,
            (int)r13, (int)r33,
            vx, vz,
            (unsigned)px, (unsigned)pz);
    }

    /* Engine constants from FUN_80022d54 / FUN_80022e38 (object init):
     * obj+0xa4 baseline = 0, obj+0xa6 = 0x3c.  Set anyway so any future
     * engine-code path that consumes them sees the right values. */
    *(int16_t *)(self + 0xa4) = 0;
    *(int16_t *)(self + 0xa6) = 0x3c;

    /* Integrate position + orientation. */
    Object_IntegrateAndOrient(self);

    /* Terrain grounding: use the original runtime ZONE/table path while
     * object obstacle leaves are being wired separately. */
    {
        int32_t pos_x = *(int32_t *)(self + 0x24);
        int32_t pos_z = *(int32_t *)(self + 0x2c);
        int32_t pos_y = *(int32_t *)(self + 0x28);
        int32_t terr_y = Host_GroundAt(pos_x, pos_y, pos_z);
        if (terr_y > 0 && pos_y >= terr_y) {
            *(int32_t *)(self + 0x28) = terr_y;
            if (*(int32_t *)(self + 0x84) > 0)
                *(int32_t *)(self + 0x84) = 0;
        }
    }

    /* Phase 5: weapon firing.  Host_WeaponFire reads pad_bits for the
     * fire button (0x08000000 = space / X), manages the projectile
     * spawn log signal, and after a 5-tick travel time calls
     * Damage_AccumulateOrFire (the real engine damage path). */
    Host_WeaponFire(self, uRam0000062c);
}

void Host_VehicleInit(void)
{
    {
        extern uint8_t DAT_80065674[];
        extern void Audio_BankSelect(uint32_t mask);
        extern uintptr_t DAT_800737a0[];
        extern intptr_t Host_TerrainFindPlaceholderById(int spawn_id);
        extern intptr_t FUN_80021f30(uint8_t *record);
        extern uint32_t FUN_8002036c(uint32_t *obj);

        uint32_t slot = DAT_80065674[0];
        if (slot >= 12) slot = 0;
        intptr_t placeholder = Host_TerrainFindPlaceholderById(-1);
        trace_obj_matrix("placeholder-player", (const uint8_t *)(uintptr_t)placeholder);
        trace_obj_matrix("placeholder-ai", (const uint8_t *)(uintptr_t)Host_TerrainFindPlaceholderById(-2));
        if (placeholder != 0) {
            uint8_t record[6] = {
                0xff, 0x00, 0xff, 0xff, 0x00, 0x00
            };
            DAT_80065674[0] = (uint8_t)slot;
            if (DAT_800737a0[slot] == 0 || DAT_800737a0[13] == 0 ||
                DAT_800737a0[14] == 0 || DAT_800737a0[15] == 0) {
                Audio_BankSelect(0x0000e000u | (1u << slot));
            }
            g_vehicle = (uint8_t *)FUN_80021f30(record);
            if (g_vehicle != NULL) {
                trace_obj_matrix("spawned-before-post", g_vehicle);
                FUN_8002036c((uint32_t *)g_vehicle);
                trace_obj_matrix("spawned-after-post", g_vehicle);
                puRam000007d0 = g_vehicle;
                {
                    int32_t sx = *(int32_t *)(g_vehicle + 0x24);
                    int32_t sy = *(int32_t *)(g_vehicle + 0x28);
                    int32_t sz = *(int32_t *)(g_vehicle + 0x2c);
                    int32_t gy = Host_GroundAt(sx, sy, sz);
                    if (getenv("V8_TRACE_SPAWN_MATRIX") != NULL) {
                        fprintf(stderr,
                                "v8: original vehicle ground delta pos=(0x%x,0x%x,0x%x) ground=0x%x delta=%d\n",
                                (unsigned)sx, (unsigned)sy, (unsigned)sz,
                                (unsigned)gy, sy - gy);
                        for (int w = 0; w < 4; w++) {
                            uint8_t *wheel = (uint8_t *)(uintptr_t)*(uint32_t *)(g_vehicle + 0xfc + w * 4);
                            if (wheel) {
                                fprintf(stderr,
                                        "v8:   wheel[%d] local=(0x%x,0x%x,0x%x) ride=(0x%x,0x%x,0x%x) flags=0x%x\n",
                                        w,
                                        (unsigned)*(uint32_t *)(wheel + 0x48),
                                        (unsigned)*(uint32_t *)(wheel + 0x4c),
                                        (unsigned)*(uint32_t *)(wheel + 0x50),
                                        (unsigned)*(uint32_t *)(wheel + 0x84),
                                        (unsigned)*(uint32_t *)(wheel + 0x90),
                                        (unsigned)*(uint32_t *)(wheel + 0x94),
                                        (unsigned)*(uint32_t *)wheel);
                            }
                        }
                    }
                }
                fprintf(stderr,
                        "v8: original vehicle spawned slot=%u obj=%p spawn_id=%d state=%u cb=%p\n",
                        slot, (void *)g_vehicle,
                        (int)*(int16_t *)(g_vehicle + 0x06),
                        (unsigned)*(uint8_t *)(g_vehicle + 0xd0),
                        (void *)Object_CallbackFromPsxSlot(g_vehicle));
                return;
            }
        }
        fprintf(stderr,
                "v8: original vehicle spawn unavailable for slot=%u; using host fallback\n",
                slot);
    }

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

    /* Provisional host spawn: center of the loaded ZMAP footprint.  This keeps
     * the test vehicle inside the decoded AIMP/JUNC route cluster on the
     * audited levels until the original match/vehicle spawn records are wired. */
    int32_t spawn_x =
        (int32_t)((((int)g_terrain_tile_x_min * 64
                  + ((int)g_terrain_tile_x_max + 1) * 64) / 2) << 16);
    int32_t spawn_z =
        (int32_t)((((int)g_terrain_tile_z_min * 64
                  + ((int)g_terrain_tile_z_max + 1) * 64) / 2) << 16);
    /* Use XOBF terrain height if available; ZONE is the bootstrap fallback
     * because XOBF is loaded lazily by renderer init_once on the first frame. */
    int32_t base_ground_y = Host_TerrainH8At(spawn_x, spawn_z);
    int32_t ground_y = Host_GroundAt(spawn_x, base_ground_y - 0x40000, spawn_z);
    *(int32_t *)(g_vehicle + 0x24) = spawn_x;
    /* Spawn 2m above ground so the gravity + terrain-snap in vehicle_tick
     * lands the vehicle cleanly on the first few frames. */
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
    Object_SetCallbackPsxSlot(g_vehicle, (uintptr_t)&vehicle_tick);

    /* Set player-1 pointer. physics_shim.c's Physics_Step dispatches
     * via this pointer directly. */
    puRam000007d0 = g_vehicle;

    fprintf(stderr, "v8: host_vehicle initialized at %p (size 0x200)\n",
            (void *)g_vehicle);
}
