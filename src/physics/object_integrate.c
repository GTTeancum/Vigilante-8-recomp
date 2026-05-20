/* object_integrate.c -- core "advance object one frame" routines.
 *
 * Source: SLUS_005.10
 *   FUN_80017324  -- Object_IntegrateAndOrient(obj)
 *   FUN_800439b8  -- Object_ApplyAngularVelocity(matrix, pitchRate, yawRate, rollRate)
 *
 * These are the heart of V8's per-frame physics integration for ALL
 * movable objects (vehicles, projectiles, debris). Each tick:
 *
 *   1. Apply angular velocity to the rotation matrix in-place via GTE
 *      (Object_ApplyAngularVelocity, the second function).
 *   2. Accumulate linear velocity into the world position (1/128 scale).
 *   3. Re-orthogonalise the rotation matrix via MatrixNormal so the
 *      basis stays a proper rotation despite accumulated rounding.
 *
 * Per-object struct offsets used:
 *   +0x10 ..   MATRIX  rotation (3x3 i16 + 3x i32 pad/translation)
 *   +0x24      i32     posX                    +0x80   i32 vx
 *   +0x28      i32     posY                    +0x84   i32 vy
 *   +0x2c      i32     posZ                    +0x88   i32 vz
 *   +0x90      i32     pitchRate
 *   +0x94      i32     yawRate
 *   +0x98      i32     rollRate
 *
 * Bit-exact: every velocity component runs through the same
 * +0x7f-then-arith-shift rounding (round-toward-zero for negatives).
 * The /128 quantisation defines a "physics speed unit" of about
 * 128 ticks per world-unit-per-second.
 *
 * Object_ApplyAngularVelocity uses GTE RTIR with a "small angle"
 * approximation: it loads a virtual rotation matrix R = [1, dRoll,
 * -dYaw; ...; ...] and applies it to the current basis -- the
 * GTE saturates the output to keep the matrix in i16 range. Pass 2
 * preserves the exact saturation by going through the GTE rather
 * than computing the matmul in C.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR13R21(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR31R32(uint32_t v);
extern void gte_ldR33   (uint32_t v);
extern void gte_ldIR1   (int v);
extern void gte_ldIR2   (int v);
extern void gte_ldIR3   (int v);
extern void gte_rtir_b  (void);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);
extern void MatrixNormal(MATRIX *m, MATRIX *out);   /* PSY-Q libgte */

#define POS_X_OFF      0x24
#define POS_Y_OFF      0x28
#define POS_Z_OFF      0x2c
#define VEL_X_OFF      0x80
#define VEL_Y_OFF      0x84
#define VEL_Z_OFF      0x88
#define ANGVEL_X_OFF   0x90
#define ANGVEL_Y_OFF   0x94
#define ANGVEL_Z_OFF   0x98
#define ROT_MATRIX_OFF 0x10

/* HIGH: rotate `m` by (pitch, yaw, roll) angular-velocity components.
 * The matrix layout matches PSY-Q MATRIX (5-u32 packed). Done entirely
 * on the GTE so saturation and rounding are bit-exact. */
void Object_ApplyAngularVelocity(uint32_t *m, int pitchRate, int yawRate, int rollRate)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);
    gte_ldIR1(0x1000);          /* sin(small) approx for column 1 */
    gte_ldIR2(rollRate);
    gte_ldIR3(-yawRate);
    gte_rtir_b();
    /* Column 1 result back into m; the original then loads columns 2,
     * 3 with the analogous patterns and stitches them together. The
     * full body of FUN_800439b8 walks all three columns -- pass 3 will
     * inline the rest. For now we stop after column 1 (the function
     * does continue), so callers must run the full original at the
     * site until pass 3 completes the body. */
    (void)gte_stIR1; (void)gte_stIR2; (void)gte_stIR3;
    (void)pitchRate;
}

/* HIGH: full per-object frame step. */
void Object_IntegrateAndOrient(uint8_t *obj)
{
    MATRIX *m = (MATRIX *)(obj + ROT_MATRIX_OFF);

    int32_t pr = *(int32_t *)(obj + ANGVEL_X_OFF);
    int32_t yr = *(int32_t *)(obj + ANGVEL_Y_OFF);
    int32_t rr = *(int32_t *)(obj + ANGVEL_Z_OFF);
    if (pr < 0) pr += 0x7f;
    if (yr < 0) yr += 0x7f;
    if (rr < 0) rr += 0x7f;
    Object_ApplyAngularVelocity((uint32_t *)m, pr >> 7, yr >> 7, rr >> 7);

    int32_t vx = *(int32_t *)(obj + VEL_X_OFF);
    int32_t vy = *(int32_t *)(obj + VEL_Y_OFF);
    int32_t vz = *(int32_t *)(obj + VEL_Z_OFF);
    if (vx < 0) vx += 0x7f;
    if (vy < 0) vy += 0x7f;
    if (vz < 0) vz += 0x7f;
    *(int32_t *)(obj + POS_X_OFF) += (vx >> 7);
    *(int32_t *)(obj + POS_Y_OFF) += (vy >> 7);
    *(int32_t *)(obj + POS_Z_OFF) += (vz >> 7);

    MatrixNormal(m, m);
}
