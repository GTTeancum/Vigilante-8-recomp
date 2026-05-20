/* sat_projection.c -- SAT (Separating Axis Theorem) projection for a
 * candidate axis between an obstacle AABB and a candidate OBB.
 *
 * Source: SLUS_005.10  FUN_8001e6dc  (708 B).
 *
 * Computes the "amount of overlap" along a candidate face-normal axis
 * for the SAT test that selects the deepest-penetrating axis between
 * the vehicle (in vehicle-local-frame) and an obstacle's AABB.
 *
 * Inputs (packed AxisQuery layout the caller builds on stack):
 *
 *     +0   i16  face_normal.x   (candidate-local q12)
 *     +2   i16  face_normal.y
 *     +4   i16  face_normal.z
 *     +6   i16  (pad / alignment)
 *     +8   i32  half_extent     (candidate's signed half-thickness
 *                                along this axis)
 *
 * Algorithm (every operation matches the MIPS line-for-line):
 *
 *   1. Apply candidate matrix M to the local face normal to get the
 *      WORLD-space axis (ApplyMatrixSV with sf=1 -- result is i16 q12
 *      in the IR registers, stored as SVECTOR for later reload).
 *
 *   2. Compute the world-space delta from candidate center to the
 *      midpoint of the obstacle AABB *expressed in vehicle local
 *      coords*:
 *          obstacle_mid_local = ((min + max) / 2) with RTZ rounding
 *          delta = vehicle.pos + obstacle_mid_local - candidate.pos
 *
 *   3. Project: dot(world_axis, delta) as a 64-bit signed accumulator,
 *      arithmetic-right-shift by 12 to get q12 -> i32 projection.
 *      MIPS uses `multu` + manual sign-corrections; the same value
 *      is produced by an i64 signed multiply.
 *
 *   4. Subtract candidate's half_extent from the projection.
 *
 *   5. Transform the *original* local face normal by the VEHICLE matrix
 *      transposed (vehicle_mat^T * local_axis -> vehicle-local axis)
 *      so we can dot it against the obstacle's axis-aligned extents.
 *
 *   6. obstacle_proj_radius = ( |axis_vehicle_local.x| * (maxX - minX)
 *                             + |axis_vehicle_local.y| * (maxY - minY)
 *                             + |axis_vehicle_local.z| * (maxZ - minZ) )
 *                             with RTZ shift >>13.
 *
 *   7. Return (proj_q12 - half_extent) - obstacle_proj_radius.
 *      Positive = separating axis; negative = overlapping along this axis.
 *
 * The caller (FUN_8001f5a0) tracks the MAXIMUM return value across all
 * candidate face-normal axes; that maximum is the deepest separation
 * (or, if negative everywhere, the *least* overlapping axis = the one
 * the collision response uses).
 *
 * HIGH confidence: structural match line-for-line with MIPS.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

extern void ApplyMatrixSV(const MATRIX *m, const SVECTOR *v0, SVECTOR *v1);
extern void GTE_LoadMatrixTransposed(uint32_t *m);   /* FUN_8004366c */
extern void gte_ldv0(const SVECTOR *sv);
extern void gte_rtv0(void);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);

/* RTZ adjustment for arithmetic right shift on signed values: matches
 * MIPS `(srl x,31)`-then-add-then-`sra`/2 idiom for "divide by 2 with
 * round-toward-zero". */
static inline int32_t rtz_div2(int32_t x)
{
    /* (x + (x >> 31)) >> 1  is round-toward-zero divide by 2. */
    return (x + (int32_t)((uint32_t)x >> 31)) >> 1;
}

static inline int32_t rsa(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

static inline int32_t i32abs(int32_t x)
{
    return (x < 0) ? -x : x;
}

/* HIGH: project obstacle along candidate axis; positive return =
 * separation along this axis, negative = overlap depth. */
int SAT_ProjectAxis(int32_t *obstacle_aabb, MATRIX *vehicle_mat,
                    const int16_t *axis_query, MATRIX *candidate_mat)
{
    /* Step 1: rotate the local face normal to world frame via the
     * candidate's matrix.  Input is i16[3] face normal at axis_query+0;
     * output is an SVECTOR (we keep the i16 form). */
    SVECTOR axis_world;
    ApplyMatrixSV(candidate_mat, (const SVECTOR *)axis_query, &axis_world);

    /* Step 2: 64-bit-clean delta from candidate origin to obstacle
     * midpoint, expressed in world coords. */
    int32_t mid_x = rtz_div2(obstacle_aabb[0] + obstacle_aabb[3]);
    int32_t mid_y = rtz_div2(obstacle_aabb[1] + obstacle_aabb[4]);
    int32_t mid_z = rtz_div2(obstacle_aabb[2] + obstacle_aabb[5]);
    int32_t dx = vehicle_mat->t[0] + mid_x - candidate_mat->t[0];
    int32_t dy = vehicle_mat->t[1] + mid_y - candidate_mat->t[1];
    int32_t dz = vehicle_mat->t[2] + mid_z - candidate_mat->t[2];

    /* Step 3: dot product as i64, then >>12 for q12 result. */
    int64_t dot = (int64_t)(int32_t)axis_world.vx * (int64_t)dx
                + (int64_t)(int32_t)axis_world.vy * (int64_t)dy
                + (int64_t)(int32_t)axis_world.vz * (int64_t)dz;
    int32_t proj_q12 = (int32_t)(dot >> 12);

    /* Step 4: subtract candidate's half_extent (i32 at axis_query+8). */
    int32_t half_extent = *(const int32_t *)((const uint8_t *)axis_query + 8);
    int32_t signed_proj = proj_q12 - half_extent;

    /* Step 5: bring the local face normal into vehicle-local frame via
     * vehicle_mat^T.  Reuse axis_world buffer as the output (matches
     * MIPS which feeds &axis_world to FUN_8004316c with a1=a0 in-place).
     *
     * GTE_LoadMatrixTransposed updates the rotation regs only; V0 is
     * preserved from the prior ApplyMatrixSV.  We then load V0 from
     * the *original* local axis (NOT the world version) and RTV0. */
    GTE_LoadMatrixTransposed((uint32_t *)vehicle_mat);
    gte_ldv0((const SVECTOR *)axis_query);
    gte_rtv0();
    axis_world.vx = (int16_t)gte_stIR1();
    axis_world.vy = (int16_t)gte_stIR2();
    axis_world.vz = (int16_t)gte_stIR3();

    /* Step 6: obstacle's projected radius =
     *   Sum_i  |axis_local[i]| * (max[i] - min[i])
     * shifted >>13 with RTZ adjust (i.e. /2 from each i16*i32 product
     * via the q13 axis encoding; matches MIPS's `>>13`). */
    int32_t extX = obstacle_aabb[3] - obstacle_aabb[0];
    int32_t extY = obstacle_aabb[4] - obstacle_aabb[1];
    int32_t extZ = obstacle_aabb[5] - obstacle_aabb[2];

    int32_t pX = (int32_t)axis_world.vx * extX;
    int32_t pY = (int32_t)axis_world.vy * extY;
    int32_t pZ = (int32_t)axis_world.vz * extZ;

    int32_t radius_sum = i32abs(pX) + i32abs(pY) + i32abs(pZ);
    int32_t radius = rsa(radius_sum, 13);

    /* Step 7: return (proj - half_extent) - obstacle_radius. */
    return signed_proj - radius;
}

/* Legacy FUN_ alias. */
int FUN_8001e6dc(int *obstacle_aabb, int vehicle_mat_int,
                 const int16_t *axis_query, MATRIX *candidate_mat)
{
    return SAT_ProjectAxis(obstacle_aabb,
                           (MATRIX *)(intptr_t)vehicle_mat_int,
                           axis_query, candidate_mat);
}
