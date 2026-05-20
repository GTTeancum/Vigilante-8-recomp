/* object_obb_suspension.c -- 8-corner OBB suspension + terrain follow.
 *
 * Source: SLUS_005.10  FUN_8001787c  (812 B).
 *
 * This is the engine's per-tick "settle me on the ground" routine: it
 * iterates the 8 corners of an axis-aligned bounding box stored as
 * (xMin, yMin, zMin, xMax, yMax, zMax) i32 in the OBB struct (`param_2`),
 * transforms each corner from object-local to world space, terrain-
 * probes under it, and -- if the corner is below ground -- accumulates
 * a spring/damper force and a (arm x force) torque into per-frame
 * accumulators.  After the 8 corners, the world-frame torque is
 * transposed-rotated back to local frame and pushed through
 * Object_ApplyImpulseAndIntegrate together with the linear force.
 *
 * Constants (verified against MIPS):
 *   - Initial torque-Y seed   = 0x2d00      (gravity-equivalent bias)
 *   - Per-corner X/Z damping  = -velX / 4   clamped to [-0xb40, +0xb40]
 *   - Per-corner Y spring     = (terrain_y - corner_y) (depth below)
 *     with vel_y>0 -> -= velY/8 added damping
 *   - Arm scale               = (corner_world - obj_pos) >> 4
 *   - Cross input scale       = force >> 3
 *   - Cross result scale      = MAC << 1  (compensates the >>3+>>4 shifts)
 *   - Angular damping post    = (angvel * 0xf80) >> 12
 *
 * GTE register reuse:  the rotation-matrix slots get repurposed mid-
 * function to hold the per-corner arm vector on the DIAGONAL (RT11,
 * RT22, RT33), which lets OP compute (arm x force) as a 3-component
 * cross product.  After the impulse is applied this gets clobbered;
 * the engine doesn't issue any rotation between this point and the
 * next caller that loads matrix slots.
 *
 * HIGH confidence: every shift, branch, and i32-wrap mults the MIPS.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

extern void GTE_RotateLongMatTrans(const uint32_t *m, const int32_t *v, int32_t *out);     /* FUN_80043408 */
extern void GTE_RotateLongMatTranspose(uint32_t *m, const int32_t *v, int32_t *out);       /* FUN_8004352c */
extern int  Terrain_HeightAndProbe(int obj, int *posXyz, SVECTOR *normalOut, uint32_t *materialOut);
extern void Object_ApplyImpulseAndIntegrate(uint8_t *obj, int32_t *force, int32_t *torque);

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR33   (uint32_t v);
extern void gte_ldsv_(int x, int y, int z);
extern void gte_op12(void);
extern int32_t gte_stMAC1(void);
extern int32_t gte_stMAC2(void);
extern int32_t gte_stMAC3(void);

static inline int32_t rsa(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

/* HIGH: settle the object on terrain via 8-corner OBB sampling. */
void Object_OBBSuspension(uint8_t *obj, const int32_t *aabb_minmax)
{
    /* `aabb_minmax` is a 6-element i32 array: (xMin, yMin, zMin, xMax, yMax, zMax). */
    int32_t force[3]  = { 0, 0, 0 };
    int32_t torque[3] = { 0, 0x2d00, 0 };  /* gravity bias on Y */
    int32_t corner_local[3];               /* sp+0x38..+0x40 in MIPS */
    int32_t restoring[3];                  /* sp+0x48..+0x50 in MIPS */
    /* Terrain probe scratch (auStack_48 in pseudo-C). */
    uint8_t probe_scratch[16];

    for (int corner_idx = 0; corner_idx < 8; corner_idx++) {
        /* Pick min vs max per axis (bit i of corner_idx SET -> use min). */
        corner_local[0] = (corner_idx & 1) ? aabb_minmax[0] : aabb_minmax[3];
        corner_local[1] = (corner_idx & 2) ? aabb_minmax[1] : aabb_minmax[4];
        corner_local[2] = (corner_idx & 4) ? aabb_minmax[2] : aabb_minmax[5];

        /* Reset the per-corner restoring force.  force[] accumulates;
         * restoring[] is the per-corner output. */
        restoring[0] = 0;
        restoring[1] = 0;
        restoring[2] = 0;

        /* Transform corner: local q15.16 -> world (rotate + translate). */
        int32_t corner_world[3];
        GTE_RotateLongMatTrans((const uint32_t *)(obj + 0x10), corner_local, corner_world);

        /* Terrain probe at this world XYZ (no normal, no material). */
        int32_t terrain_y = Terrain_HeightAndProbe((int)(intptr_t)obj, corner_world, NULL, NULL);

        /* Depth below ground (PSX convention: +Y is down). */
        int32_t depth = corner_world[1] - terrain_y;
        if (depth <= 0) continue;                /* corner above ground */

        /* X-axis damping: -velX / 4, clamped to [-0xb40, +0xb40].
         * MIPS subtracts vel into a register, RTZ-shifts by 2. */
        {
            int32_t vx = *(int32_t *)(obj + 0x80);
            int32_t d  = rsa(-vx, 2);
            if (d >  0xb40) d =  0xb40;
            if (d < -0xb40) d = -0xb40;
            restoring[0] = d;
        }

        /* Z-axis damping: -velZ / 4, clamped. */
        {
            int32_t vz = *(int32_t *)(obj + 0x88);
            int32_t d  = rsa(-vz, 2);
            if (d >  0xb40) d =  0xb40;
            if (d < -0xb40) d = -0xb40;
            restoring[2] = d;
        }

        /* Y-axis spring/damper: base spring is "depth below ground"
         * (negative because force pushes object up against +Y-down).
         * When velY > 0 (moving deeper), subtract velY/8 (extra damping). */
        {
            int32_t y_force = -depth;                          /* spring */
            int32_t vy = *(int32_t *)(obj + 0x84);
            if (vy > 0) y_force -= rsa(vy, 3);
            restoring[1] = y_force;
        }

        /* Arm = (corner_world - obj_pos) >> 4.  Loaded into the GTE
         * rotation-matrix DIAGONAL slots (RT11, RT22, RT33) so the
         * subsequent OP can compute (arm x force). */
        {
            int32_t arm_x = (corner_world[0] - *(int32_t *)(obj + 0x24)) >> 4;
            int32_t arm_y = (corner_world[1] - *(int32_t *)(obj + 0x28)) >> 4;
            int32_t arm_z = (corner_world[2] - *(int32_t *)(obj + 0x2c)) >> 4;
            gte_ldR11R12((uint32_t)arm_x);
            gte_ldR22R23((uint32_t)arm_y);
            gte_ldR33   ((uint32_t)arm_z);
        }

        /* Force into IR, also scaled down by 8 (so the OP result lands
         * in i32 range despite the q24 product of arm*force). */
        gte_ldsv_(rsa(restoring[0], 3),
                  rsa(restoring[1], 3),
                  rsa(restoring[2], 3));
        gte_op12();   /* MAC1..3 = arm x force, after sf=1 shift */

        /* Accumulate linear force (raw values; suspension impulse adds
         * directly to vel via Object_ApplyImpulseAndIntegrate). */
        force[0] += restoring[0];
        force[1] += restoring[1];
        force[2] += restoring[2];

        /* Accumulate torque: cross product * 2 (compensates the >>3+>>4
         * down-shifts done to keep the GTE accumulator in i32 range). */
        torque[0] += gte_stMAC1() << 1;
        torque[1] += gte_stMAC2() << 1;
        torque[2] += gte_stMAC3() << 1;
        (void)probe_scratch;
    }

    /* Rotate torque from world frame to local frame (M^T * torque). */
    GTE_RotateLongMatTranspose((uint32_t *)(obj + 0x10), torque, torque);

    /* Apply linear + angular impulses + integrate one tick. */
    Object_ApplyImpulseAndIntegrate(obj, force, torque);

    /* Post-integrate angular damping (matches Object_GeneralTick): the
     * MIPS uses i32-wrap multiply then arithmetic shift right by 12. */
    *(int32_t *)(obj + 0x90) =
        (int32_t)((uint32_t)*(int32_t *)(obj + 0x90) * 0xf80u) >> 12;
    *(int32_t *)(obj + 0x94) =
        (int32_t)((uint32_t)*(int32_t *)(obj + 0x94) * 0xf80u) >> 12;
    *(int32_t *)(obj + 0x98) =
        (int32_t)((uint32_t)*(int32_t *)(obj + 0x98) * 0xf80u) >> 12;
}

/* Legacy FUN_ alias. */
void FUN_8001787c(int obj_int, const int32_t *aabb_minmax)
{
    Object_OBBSuspension((uint8_t *)(intptr_t)obj_int, aabb_minmax);
}
