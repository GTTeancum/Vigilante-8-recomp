/* bone_debris_tick.c -- spinning/rolling debris bone-object per-frame tick.
 *
 * Source: SLUS_005.10  LAB_8003ed38  (67 instr).
 *
 * This callback is installed at bone-type physics objects (debris shards,
 * rolling fragments) spawned by the effect system.  On each tick it:
 *   1. If a child-list node with type==1 exists: interpolate a target
 *      position vector from that node's data.
 *   2. Else: derive a fall vector from the object's vertical speed
 *      stored at +0x54.
 *   3. Apply per-frame angular velocity via FUN_800439b8 (small-angle
 *      GTE OP12 rotation: pitchRate=s0[0x40], yawRate=0, rollRate=0).
 *   4. Advance the spin angle: s0[0x40] += s0[0x42] >> 3  (RTZ).
 *   5. Transform the position vector through the rotation matrix
 *      via GTE_RotateLongMatTrans (FUN_80043408).
 *   6. Terrain-height probe at (world_x, world_z).
 *   7. If still above terrain (PSX Y-down: world_y < terrain_y): return 0.
 *   8. Landing / collision handler (also entered on event 3):
 *      a. Object_ClearBackBufferFlag.
 *      b. Effects_SpawnExplosion (FUN_8003fc50) -- spawns a landing spark.
 *      c. If obj[0x30] != 0: return 0 (still linked, keep alive).
 *      d. Else: Damage_Apply (remove from scene), return -1.
 *
 * Object field layout (offsets):
 *   +0x10 (16)  : rotation matrix (packed int16[9] + pad + int32[3])
 *   +0x30 (48)  : "link" / alive counter (0 = standalone, destroy on land)
 *   +0x40 (64)  : current spin angle (int16)
 *   +0x42 (66)  : spin rate (int16, RTZ >> 3 per tick)
 *   +0x54 (84)  : fall speed (int32, negated to get downward component)
 *   +0x5c (92)  : first-child list pointer
 *
 * HIGH confidence: line-for-line MIPS port.
 */
#include <stdint.h>

extern void FUN_800439b8(uint32_t *m, int pitch, int yaw, int roll);  /* Object_ApplyAngularVelocity */
extern void GTE_RotateLongMatTrans(const uint32_t *m,
                                   const int32_t *v, int32_t *out);   /* FUN_80043408 */
extern int  FUN_80025400(int x, int z);                               /* Terrain_HeightAt */
extern int  FUN_8003fc50(int obj);                                    /* Effects_SpawnExplosion */
extern void FUN_800205f8(int obj);                                    /* Damage_Apply */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* RTZ (round-toward-zero) arithmetic shift right. */
static inline int32_t rtz_sra(int32_t x, int n)
{
    if (x < 0) x = mips_addu_i32(x, (1 << n) - 1);
    return x >> n;
}

static int landing_handler(uint8_t *s0, int obj)
{
    /* Step a: clear back-buffer pending flag. */
    uint32_t flags = *(uint32_t *)s0;
    if (flags & 0x80u) {
        *(uint32_t *)s0 = flags & ~0x80u;
        /* Object_ClearBackBufferFlag -- list removal stubbed on host. */
    }

    /* Step b: try to spawn landing explosion effect. */
    FUN_8003fc50(obj);

    /* Step c: if still linked to something, stay alive. */
    if (*(int32_t *)(s0 + 0x30) != 0) return 0;

    /* Step d: unlinked -- remove from scene. */
    FUN_800205f8(obj);
    return -1;
}

int LAB_8003ed38(int obj, int event, int param3)
{
    uint8_t *s0 = (uint8_t *)(uintptr_t)obj;
    (void)param3;

    if (event != 0) {
        if (event == 3) return landing_handler(s0, obj);
        return 0;
    }

    /* ------ Event 0: tick ------ */

    /* Determine position/velocity vector for this tick. */
    int32_t vec[3];                  /* sp+16..24 in MIPS */
    int child = *(int32_t *)(s0 + 0x5c);

    if (child != 0 && *(uint16_t *)(uintptr_t)child == 1) {
        /* Child node type 1: read interpolated position. */
        uint8_t *cn = (uint8_t *)(uintptr_t)(child + 4); /* advance 4 */
        int32_t a  = *(int32_t *)cn;          /* child+4  */
        int32_t b  = *(int32_t *)(cn + 8);    /* child+12 */
        int32_t sum = mips_addu_i32(a, b);
        /* RTZ average: (a+b)/2 */
        vec[0] = mips_addu_i32(sum, sum >> 31) >> 1;
        vec[1] = *(int32_t *)(cn + 4);        /* child+8  */
        vec[2] = *(int32_t *)(cn + 16);       /* child+20 */
    } else {
        /* No child: fall straight down at rate s0[0x54]. */
        vec[0] = 0;
        vec[1] = mips_subu_i32(0, *(int32_t *)(s0 + 0x54));
        vec[2] = 0;
    }

    /* Apply angular velocity to rotation matrix (pitch only, yaw=roll=0). */
    int16_t angvel = *(int16_t *)(s0 + 0x40);
    FUN_800439b8((uint32_t *)(s0 + 0x10), (int)angvel, 0, 0);

    /* Advance spin angle: += (s0[0x42] >> 3)  RTZ. */
    {
        int16_t dangle = *(int16_t *)(s0 + 0x42);
        int16_t cur    = (int16_t)*(uint16_t *)(s0 + 0x40);
        cur = (int16_t)((uint16_t)cur +
                        (uint16_t)rtz_sra((int32_t)dangle, 3));
        *(uint16_t *)(s0 + 0x40) = (uint16_t)cur;
    }

    /* Transform position vector through rotation matrix to world space. */
    GTE_RotateLongMatTrans((const uint32_t *)(s0 + 0x10), vec, vec);

    /* Terrain height at (world_x, world_z). */
    int32_t terrain_y = FUN_80025400(vec[0], vec[2]);

    /* PSX Y-down: if world_y < terrain_y the object is above the terrain. */
    if (vec[1] < terrain_y) return 0;   /* still in flight */

    return landing_handler(s0, obj);
}
