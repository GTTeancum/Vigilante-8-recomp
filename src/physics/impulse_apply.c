/* impulse_apply.c -- physics impulse + collision-response helpers.
 *
 * Source: SLUS_005.10
 *   FUN_800176f8  -- Object_ApplyImpulseAtPoint: add `vec` to linear
 *                    velocity accumulator (+0x80) and (arm x vec)
 *                    rotated into local frame to angular velocity
 *                    accumulator (+0x90), scaled by inertia inverse
 *                    at +0x9c.
 *   FUN_80017ba8  -- Object_ResolveCollisionImpulse: standard
 *                    inelastic collision response.  Reads relative
 *                    velocity dot normal (FUN_80017240 = 64-bit dot,
 *                    >> 0xf to q.15), and if negative computes
 *                    impulse = (-2*v_n + (-restitution)) * normal
 *                    rotated into object's frame, then forwards to
 *                    FUN_80017594 (Object_ApplyAngularImpulse).
 *
 * Bit-exact:
 *   - The 64-bit dot extraction `(lo>>15) | (hi<<17)` is preserved
 *     byte-for-byte from the MIPS sequence.
 *   - The (q12 * q20) multiplies in the impulse-scale step are kept
 *     in 64-bit width as in the MIPS dmult sequence.
 *   - The Ghidra-shown 4th-arg of FUN_800434f8 is a Ghidra artefact
 *     (the function takes 3 args); preserved as 3-arg call.
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>

/* ---- GTE (cross-product, transposed-matrix rotate) ---- */
extern void gte_ldR11R12(int32_t v);
extern void gte_ldR22R23(int32_t v);
extern void gte_ldR33  (int32_t v);
extern void gte_ldsv_  (int32_t x, int32_t y, int32_t z);
extern void gte_op12   (void);
extern int32_t gte_stMAC1(void);
extern int32_t gte_stMAC2(void);
extern int32_t gte_stMAC3(void);

extern void FUN_800434f8(uint32_t *m, void *sv_in, void *sv_out);   /* GTE_RotateShortMatTranspose */
extern void FUN_800434d0(uint32_t *m, const void *sv, int32_t *out);/* GTE_RotateNormalTranspose */
extern int64_t FUN_80017240(const int32_t *v, const void *svec);    /* GTE_CrossPlanar / vec3 64-bit dot */
extern void FUN_80017594(uint32_t *self,
                         const int32_t *vec_a,
                         const int32_t *vec_b);                     /* Angular impulse apply */

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint64_t)(uint32_t)a * (uint64_t)(uint32_t)b);
}

/* ================================================================
 * FUN_800176f8 -- Object_ApplyImpulseAtPoint
 *
 *   param_1: object base pointer
 *   param_2: impulse vector (int32[3])
 *   param_3: contact-point world position (int32[3])
 *
 * Adds the impulse to obj+0x80 (linear vel).
 * Computes (contact - obj_pos) x impulse via GTE OP12 in q.3
 *   (shift inputs right by 3 to keep 64-bit safety),
 * then transforms that cross product into object-local frame via
 * FUN_800434f8 and scales each axis by obj+0x9c[i] (inverse inertia,
 * q.7) using a positive-bias RTZ shift >> 7.
 * The result is added into obj+0x90 (angular vel).
 *
 * HIGH confidence.
 * ================================================================ */
void FUN_800176f8(intptr_t param_1, int32_t *param_2, int32_t *param_3)
{
    int32_t  cross1, cross2, cross3;
    int16_t  local_10, local_e, local_c;
    int      iVar2;

    /* Linear velocity accumulator. */
    *(int32_t *)(uintptr_t)(param_1 + 0x80) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x80), param_2[0]);
    *(int32_t *)(uintptr_t)(param_1 + 0x84) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x84), param_2[1]);
    *(int32_t *)(uintptr_t)(param_1 + 0x88) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x88), param_2[2]);

    /* Cross product (arm x impulse) via GTE OP12.  Inputs scaled >>3 to
     * keep MAC1/2/3 from saturating; original loads each component
     * separately into R11R12 / R22R23 / R33 (re-using rot-matrix slots
     * as scratch for the second-vector axis trio). */
    gte_ldR11R12(mips_subu_i32(param_3[0], *(int32_t *)(uintptr_t)(param_1 + 0x48)) >> 3);
    gte_ldR22R23(mips_subu_i32(param_3[1], *(int32_t *)(uintptr_t)(param_1 + 0x4c)) >> 3);
    gte_ldR33  (mips_subu_i32(param_3[2], *(int32_t *)(uintptr_t)(param_1 + 0x50)) >> 3);
    gte_ldsv_(param_2[0] >> 3, param_2[1] >> 3, param_2[2] >> 3);
    gte_op12();
    cross1 = gte_stMAC1();
    cross2 = gte_stMAC2();
    cross3 = gte_stMAC3();

    local_10 = (int16_t)cross1;
    local_e  = (int16_t)cross2;
    local_c  = (int16_t)cross3;

    /* Rotate cross product into object-local frame (transposed matrix).
     * Ghidra showed a 4th arg (obj+0x48) which is a Ghidra artefact --
     * FUN_800434f8 only takes 3 args in the actual MIPS. */
    FUN_800434f8((uint32_t *)(uintptr_t)(param_1 + 0x10),
                 &local_10, &local_10);

    /* Scale each component by inverse-inertia (signed q.7) and add to
     * angular velocity accumulator.  RTZ shift requires positive bias
     * (+0x7f) when the value is negative. */
    iVar2 = mips_mult_lo_i32((int)local_10, (int)*(int16_t *)(uintptr_t)(param_1 + 0x9c));
    if (iVar2 < 0) iVar2 = iVar2 + 0x7f;
    *(int32_t *)(uintptr_t)(param_1 + 0x90) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x90), iVar2 >> 7);

    iVar2 = mips_mult_lo_i32((int)local_e, (int)*(int16_t *)(uintptr_t)(param_1 + 0x9e));
    if (iVar2 < 0) iVar2 = iVar2 + 0x7f;
    *(int32_t *)(uintptr_t)(param_1 + 0x94) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x94), iVar2 >> 7);

    iVar2 = mips_mult_lo_i32((int)local_c, (int)*(int16_t *)(uintptr_t)(param_1 + 0xa0));
    if (iVar2 < 0) iVar2 = iVar2 + 0x7f;
    *(int32_t *)(uintptr_t)(param_1 + 0x98) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x98), iVar2 >> 7);
}

/* ================================================================
 * FUN_80017ba8 -- Object_ResolveCollisionImpulse
 *
 *   param_1: object base pointer
 *   param_2: contact descriptor.
 *            +0x14  contact world position
 *            +0x20  contact normal (SVECTOR i16)
 *            +0x30  restitution coefficient
 *
 * If the relative velocity dotted with the contact normal is
 * negative (objects moving together), apply an impulse equal to
 * (-2 * v_n - restitution) along the normal (in object-local frame)
 * via FUN_80017594.
 *
 * HIGH confidence; 64-bit math preserved byte-for-byte.
 * ================================================================ */
void FUN_80017ba8(intptr_t param_1, intptr_t param_2)
{
    uint32_t  uVar1, uVar2, uVar3;
    int       iVar4;
    int64_t   uVar5;
    uint32_t  local_28, local_24, local_20;

    uVar5 = FUN_80017240((const int32_t *)(uintptr_t)(param_1 + 0x80),
                         (const void  *)(uintptr_t)(param_2 + 0x20));
    /* Extract q.15 dot from 64-bit product: lo>>15 | hi<<17. */
    uVar2 = ((uint32_t)uVar5 >> 0xf)
          | ((uint32_t)((uint64_t)uVar5 >> 0x20) << 0x11);

    if ((int32_t)uVar2 < 0) {
        FUN_800434d0((uint32_t *)(uintptr_t)(param_1 + 0x10),
                     (const void *)(uintptr_t)(param_2 + 0x20),
                     (int32_t *)&local_28);

        uVar1 = (uint32_t)mips_subu_i32(0, *(int32_t *)(uintptr_t)(param_2 + 0x30));
        uVar3 = (uint32_t)mips_addu_i32((int32_t)uVar1,
                                        mips_mult_lo_i32((int32_t)uVar2, -2));
        /* High 32-bit half of the q31 add: signs + borrow propagation. */
        iVar4 = (((int32_t)uVar1 >> 0x1f)
              - ((int32_t)(uVar2 * 2u) >> 0x1f))
              - (uint32_t)(uVar1 < (uVar2 * 2u));

        /* For each component of local_2x: (hi * scalar << 20) | (lo>>12). */
        local_28 = ((uint32_t)((uint64_t)local_28 * (uint64_t)uVar3) >> 0xc) |
                   ((uint32_t)((int32_t)((uint64_t)((int64_t)local_28 * (int64_t)uVar3) >> 0x20)
                                + (int32_t)local_28 * iVar4
                                + (int32_t)(uVar3 * ((int32_t)local_28 >> 0x1f))) * 0x100000u);
        local_24 = ((uint32_t)((uint64_t)local_24 * (uint64_t)uVar3) >> 0xc) |
                   ((uint32_t)((int32_t)((uint64_t)((int64_t)local_24 * (int64_t)uVar3) >> 0x20)
                                + (int32_t)local_24 * iVar4
                                + (int32_t)(uVar3 * ((int32_t)local_24 >> 0x1f))) * 0x100000u);
        local_20 = ((uint32_t)((uint64_t)local_20 * (uint64_t)uVar3) >> 0xc) |
                   ((uint32_t)((int32_t)((uint64_t)((int64_t)local_20 * (int64_t)uVar3) >> 0x20)
                                + (int32_t)local_20 * iVar4
                                + (int32_t)(uVar3 * ((int32_t)local_20 >> 0x1f))) * 0x100000u);

        FUN_80017594((uint32_t *)(uintptr_t)param_1,
                     (const int32_t *)&local_28,
                     (const int32_t *)(uintptr_t)(param_2 + 0x14));
    }
}
