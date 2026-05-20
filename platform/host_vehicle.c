/* host_vehicle.c -- allocate a real Vehicle struct and register it
 * with the engine's world-object linked list.
 *
 * Physics_Step (FUN_8002131c) walks the list at piRam0000075c and
 * calls each entry's +0x64 callback. We populate the list with a
 * single Vehicle whose tick reads pad bits -> ang/lin velocity, then
 * delegates to the cleaned engine integrator Object_IntegrateAndOrient.
 *
 * The vehicle's per-frame "drive" logic here (pad -> velocity) is a
 * thin host shim that stands in for the engine's real vehicle tick
 * (which involves suspension, terrain following, traction model, AI
 * integration). The PHYSICS (matrix orientation, position integration,
 * matrix normalization) is the engine's Object_IntegrateAndOrient
 * running on real engine data. Replace this shim with the engine's
 * actual vehicle tick when that decomp completes.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *Heap_Alloc(uint32_t n);

/* Engine globals. */
extern void    *puRam000007d0;            /* player vehicle pointer */
extern uint32_t uRam0000062c;             /* P1 pad bits */

/* Engine integrator (cleaned, src/physics/object_integrate.c). */
extern void Object_IntegrateAndOrient(uint8_t *obj);

/* The Vehicle struct is allocated in the engine heap (low memory).
 *
 * Multi-vehicle support will require a proper pack(4) linked-list at
 * piRam0000075c that the cleaned Physics_Step walks; for now
 * physics_shim.c invokes puRam000007d0's tick directly. */
static uint8_t *g_vehicle = NULL;

/* Vehicle tick callback. Receives (self, mode, catchupFlag). The
 * engine calls with mode==0 per frame. We read pad input, compute
 * angular + linear velocity, then run the engine's integrator. */
static void vehicle_tick(uint8_t *self, int mode, int catchupFlag)
{
    (void)catchupFlag;
    if (mode != 0) return;

    /* --- Drive shim: pad bits -> Vehicle velocity & ang-velocity --- */
    int32_t pitchRate = 0;
    int32_t yawRate   = 0;
    int32_t rollRate  = 0;
    int32_t fwdAccel  = 0;
    int32_t fwdBrake  = 0;

    if (uRam0000062c & 0x10000000) fwdAccel = 0x40000;  /* Up    -> forward accel */
    if (uRam0000062c & 0x40000000) fwdBrake = 0x40000;  /* Down  -> reverse accel */
    if (uRam0000062c & 0x80000000) yawRate  = -0x4000;  /* Left  -> yaw negative */
    if (uRam0000062c & 0x20000000) yawRate  =  0x4000;  /* Right -> yaw positive */

    *(int32_t *)(self + 0x90) = pitchRate;
    *(int32_t *)(self + 0x94) = yawRate;
    *(int32_t *)(self + 0x98) = rollRate;

    /* Forward-direction acceleration: use rotation matrix col 3 (Z forward).
     * MATRIX layout per gte.h: 3x3 i16 rotation + i16 pad + 3 i32 translation.
     * Element layout (packed):
     *   u32[0] = R11 | R12
     *   u32[1] = R13 | R21
     *   u32[2] = R22 | R23
     *   u32[3] = R31 | R32
     *   u32[4] = R33 | <pad>
     * Forward (local +Z) maps to world (R13, R23, R33). Each is i16 q12. */
    uint32_t *m = (uint32_t *)(self + 0x10);
    int16_t r13 = (int16_t)(m[1] & 0xffff);          /* low half of u32[1] */
    int16_t r23 = (int16_t)(m[2] & 0xffff);          /* low half of u32[2] */
    int16_t r33 = (int16_t)(m[4] & 0xffff);          /* low half of u32[4] */

    int32_t signedAccel = fwdAccel - fwdBrake;
    if (signedAccel != 0) {
        /* vel += accel * forward_dir / 0x1000  (q12 scaled) */
        *(int32_t *)(self + 0x80) += (signedAccel * r13) >> 12;
        *(int32_t *)(self + 0x84) += (signedAccel * r23) >> 12;
        *(int32_t *)(self + 0x88) += (signedAccel * r33) >> 12;
    }
    /* Drag: shrink velocity by ~3% per tick so the vehicle doesn't drift forever. */
    *(int32_t *)(self + 0x80) -= *(int32_t *)(self + 0x80) >> 5;
    *(int32_t *)(self + 0x84) -= *(int32_t *)(self + 0x84) >> 5;
    *(int32_t *)(self + 0x88) -= *(int32_t *)(self + 0x88) >> 5;

    /* --- Real engine integrator --- */
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

    /* Start at world origin, slightly elevated. Position is i32 in
     * fixed-point; pick something visible. */
    *(int32_t *)(g_vehicle + 0x24) = 0;            /* x */
    *(int32_t *)(g_vehicle + 0x28) = 0;            /* y */
    *(int32_t *)(g_vehicle + 0x2c) = 0;            /* z */

    /* Health field (used by damage path; not relevant for driving). */
    *(int16_t *)(g_vehicle + 0x0c) = 1000;         /* health */
    *(int16_t *)(g_vehicle + 0x0e) = 1000;         /* maxHealth */

    /* Tick callback pointer. The engine calls this every Physics_Step. */
    *(void (**)(uint8_t *, int, int))(g_vehicle + 0x64) = vehicle_tick;

    /* Set player-1 pointer. physics_shim.c's Physics_Step dispatches
     * via this pointer directly. */
    puRam000007d0 = g_vehicle;

    fprintf(stderr, "v8: host_vehicle initialized at %p (size 0x200)\n",
            (void *)g_vehicle);
}
