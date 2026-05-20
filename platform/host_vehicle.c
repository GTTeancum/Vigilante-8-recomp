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

/* Engine's universal per-tick step (src/physics/object_general_tick.c).
 * The bit-exact PSX physics chain (terrain probe via Vec3_Length-aware
 * Terrain_HeightAndProbe -> spring/drag -> Object_IntegrateAndOrient
 * -> MatrixNormal -> post-integrate damping) runs inside this. */
extern void Object_GeneralTick(uint32_t *obj);

/* The Vehicle struct is allocated in the engine heap (low memory).
 *
 * Multi-vehicle support will require a proper pack(4) linked-list at
 * piRam0000075c that the cleaned Physics_Step walks; for now
 * physics_shim.c invokes puRam000007d0's tick directly. */
static uint8_t *g_vehicle = NULL;

/* Vehicle tick callback.  Receives (self, mode, catchupFlag); the
 * engine calls with mode==0 per frame.  Writes the per-tick input
 * fields the engine consumes, then delegates to Object_GeneralTick. */
static void vehicle_tick(uint8_t *self, int mode, int catchupFlag)
{
    (void)catchupFlag;
    if (mode != 0) return;

    /* Pad bits -> engine input fields.
     *
     * inputMul (+0xa6) = 0x3c: the engine's own constant.  Two
     * independent initialisers (FUN_80022d54 and FUN_80022e38) write
     * `*(u16 *)(obj + 0xa6) = 0x3c` -- confirmed via static MIPS
     * analysis, NOT empirical.  See notes/unknowns.md "Item 4".
     *
     * angYPreBake (+0xa4) baseline = 0 (same source).  Set to a
     * non-zero magnitude only while the steering pad bit is active. */
    int16_t longThrust = 0;
    if (uRam0000062c & 0x10000000) longThrust = +0x40;
    if (uRam0000062c & 0x40000000) longThrust = -0x40;
    *(int16_t *)(self + 0x20) = longThrust;
    *(int16_t *)(self + 0xa6) = 0x3c;

    if (uRam0000062c & 0x80000000) {
        *(int16_t *)(self + 0x16) = -1;
        *(int16_t *)(self + 0xa4) = -0x80;
    } else if (uRam0000062c & 0x20000000) {
        *(int16_t *)(self + 0x16) = +1;
        *(int16_t *)(self + 0xa4) = +0x80;
    } else {
        *(int16_t *)(self + 0x16) = 0;
        *(int16_t *)(self + 0xa4) = 0;
    }
    *(int16_t *)(self + 0x1a) = 1;
    /* dragMass (+0xd8): the engine computes this as -*(int*)(obj+0x4c)
     * in FUN_8002e630 (vehicle constructor), where +0x4c is a value
     * template-copied by the pool allocator from per-character data.
     * Without that template chain wired up, +0x4c is 0 here.
     * Leaving dragMass = 0 means Object_GeneralTick's spring/drag
     * term degenerates to pure gravity. */
    *(int32_t *)(self + 0xd8) = 0;

    Object_GeneralTick((uint32_t *)self);
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
    *(int32_t *)(g_vehicle + 0x28) = ground_y + 0x40000;
    *(int32_t *)(g_vehicle + 0x2c) = spawn_z;

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
