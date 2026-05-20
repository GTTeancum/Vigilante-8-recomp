/* object_impulse.c -- apply linear + angular impulses, then integrate.
 *
 * Source: SLUS_005.10  FUN_800173fc  (408 B).
 *
 * The "impulse" form of the universal integrator: takes a pre-computed
 * world-space linear force vector and a local-space angular torque
 * vector, accumulates them into the object's velocity / angular velocity
 * (scaled by per-axis inverse-inertia at +0x9c/+0x9e/+0xa0), then runs
 * the standard Object_ApplyAngularVelocity + position-integrate +
 * MatrixNormal sequence (i.e. the body of Object_IntegrateAndOrient).
 *
 * Bit-exact rounding:
 *   - Angular: `(torque[i] * inertia[i]) >> 6` with RTZ adjust.
 *   - Angular pre-integrate: `>> 7` of angvel, RTZ adjust.
 *   - Linear pre-integrate: `>> 7` of vel, RTZ adjust (matches the
 *     integrator).
 *
 * Per-axis inertia at +0x9c/+0x9e/+0xa0 is i16 (read as `(short)`).
 *
 * HIGH confidence: line-by-line match with MIPS.
 */
#include <stdint.h>
#include "structs.h"

extern void Object_ApplyAngularVelocity(uint32_t *m, int pitchRate, int yawRate, int rollRate);
extern void MatrixNormal(MATRIX *m, MATRIX *out);

/* PSY-Q's round-toward-zero arithmetic right-shift. */
static inline int32_t rsa(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

void Object_ApplyImpulseAndIntegrate(uint8_t *obj, int32_t *force, int32_t *torque)
{
    /* 1. Linear impulse: vel += force (full precision, no scaling). */
    *(int32_t *)(obj + 0x80) += force[0];
    *(int32_t *)(obj + 0x84) += force[1];
    *(int32_t *)(obj + 0x88) += force[2];

    /* 2. Angular impulse: angvel += (torque[i] * inertia[i]) / 64. */
    int32_t ax = torque[0] * (int32_t)*(int16_t *)(obj + 0x9c);
    *(int32_t *)(obj + 0x90) += rsa(ax, 6);
    int32_t ay = torque[1] * (int32_t)*(int16_t *)(obj + 0x9e);
    *(int32_t *)(obj + 0x94) += rsa(ay, 6);
    int32_t az = torque[2] * (int32_t)*(int16_t *)(obj + 0xa0);
    *(int32_t *)(obj + 0x98) += rsa(az, 6);

    /* 3. Standard integrator body (matches Object_IntegrateAndOrient). */
    MATRIX *m = (MATRIX *)(obj + 0x10);
    int32_t pr = *(int32_t *)(obj + 0x90);
    int32_t yr = *(int32_t *)(obj + 0x94);
    int32_t rr = *(int32_t *)(obj + 0x98);
    Object_ApplyAngularVelocity((uint32_t *)m, rsa(pr, 7), rsa(yr, 7), rsa(rr, 7));

    int32_t vx = *(int32_t *)(obj + 0x80);
    int32_t vy = *(int32_t *)(obj + 0x84);
    int32_t vz = *(int32_t *)(obj + 0x88);
    *(int32_t *)(obj + 0x24) += rsa(vx, 7);
    *(int32_t *)(obj + 0x28) += rsa(vy, 7);
    *(int32_t *)(obj + 0x2c) += rsa(vz, 7);

    MatrixNormal(m, m);
}

/* Legacy FUN_ alias for direct MIPS-derived call sites. */
void FUN_800173fc(int obj_int, int32_t *force, int32_t *torque)
{
    Object_ApplyImpulseAndIntegrate((uint8_t *)(intptr_t)obj_int, force, torque);
}
