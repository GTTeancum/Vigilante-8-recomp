/* angular_impulse.c -- FUN_80017594 (86 MIPS instructions).
 *
 * Apply an angular (torque-arm) impulse to a vehicle physics body.
 * Called from Vehicle_WheelForceDispatch and Vehicle_Collision.
 *
 * Source: SLUS_005.10  0x80017594
 *
 * Register map:
 *   s2 = self (a0) -- vehicle/object pointer
 *   s1 = vec_a (a1) -- impulse vector (world-space torque arm)
 *   s0 = vec_b (a2) -- contact/normal vector
 *
 * Algorithm:
 *   1. Rotate vec_a by self's local rotation matrix (M at self+0x10)
 *      via GTE_RotateLongMat → result[] (linear delta in local space).
 *   2. Load vec_b >> 4 as the GTE diagonal (D), vec_a >> 3 into IR1/2/3.
 *   3. GTE OP with sf=1: MAC1/2/3 = cross-like(IR, D)/4096
 *      (MAC_i = (IR_j*D_k - IR_k*D_j) >> 12 per nocash OP spec).
 *   4. Linear impulse: self[0x80..0x88] += result[0..2].
 *   5. Angular impulse for each axis i:
 *        ang = RTZ(MAC_i * inertia_factor_i) >> 6
 *        self[rotvel_i] += ang
 *      where inertia factors are int16 at self+0x9c/0x9e/0xa0
 *      and rotational velocity words are at self+0x90/0x94/0x98.
 *
 * RTZ(v, n) = (v + (v < 0 ? (1<<n)-1 : 0)) >> n  -- round toward zero.
 *
 * HIGH confidence: all branches and delay-slot effects reproduced.
 */
#include <stdint.h>

/* GTE_RotateLongMat: out = M * v using the 17.15 hi/lo-split technique.
 * Defined in src/physics/gte_long_vec.c (= FUN_80043358). */
extern void GTE_RotateLongMat(const uint32_t *m, const int32_t *v, int32_t *out);

/* GTE diagonal / IR write helpers (from platform/psyq/libgte.c). */
extern void    gte_ldR11R12(uint32_t v);
extern void    gte_ldR22R23(uint32_t v);
extern void    gte_ldR33   (uint32_t v);
extern void    gte_ldsv_   (int x, int y, int z);
extern void    gte_OP      (int sf);
extern int32_t gte_stMAC1  (void);
extern int32_t gte_stMAC2  (void);
extern int32_t gte_stMAC3  (void);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* RTZ right-shift by 6: equivalent to MIPS bgez/addiu 63/sra 6. */
static inline int32_t rtz6(int32_t v)
{
    if (v < 0) v = mips_addu_i32(v, 0x3f);
    return v >> 6;
}

void FUN_80017594(uint32_t *self, const int32_t *vec_a, const int32_t *vec_b)
{
    uint8_t *s = (uint8_t *)self;

    /* Step 1: linear_delta = rotation_matrix(self+0x10) * vec_a.
     *
     * MIPS 0x800175b0-0x800175bc:
     *   addiu a0,s2,0x10  -- a0 = self+0x10
     *   jal   FUN_80043358
     *   _addiu a2,sp,0x10 -- delay: a2 = output buffer on stack
     */
    int32_t lin[3];
    GTE_RotateLongMat((const uint32_t *)(s + 0x10), vec_a, lin);

    /* Step 2a: load contact-vector/16 into GTE diagonal (for OP).
     *
     * MIPS 0x800175c0-0x800175e0: sra a0,a0,4; gte_ldR11R12 a0 etc.
     * Only the diagonal entries (R11, R22, R33) are written; off-diagonal
     * entries retain whatever GTE_RotateLongMat left in place.  The OP
     * instruction only reads the diagonal. */
    gte_ldR11R12((uint32_t)(int32_t)(vec_b[0] >> 4));
    gte_ldR22R23((uint32_t)(int32_t)(vec_b[1] >> 4));
    gte_ldR33   ((uint32_t)(int32_t)(vec_b[2] >> 4));

    /* Step 2b: load torque-arm/8 into IR1/2/3 (truncated to i16).
     *
     * MIPS 0x800175e4-0x800175fc: sra a0,a0,3; ldsv_ a0,v1,v0. */
    gte_ldsv_((int)(vec_a[0] >> 3),
              (int)(vec_a[1] >> 3),
              (int)(vec_a[2] >> 3));

    /* Step 3: GTE OP with sf=1.
     *   MAC1 = (IR3*D2 - IR2*D3) >> 12
     *   MAC2 = (IR1*D3 - IR3*D1) >> 12
     *   MAC3 = (IR2*D1 - IR1*D2) >> 12
     * where D = (R11, R22, R33) from the diagonal loaded above.
     * MIPS 0x80017608: nOP12 (= OP with sf=1). */
    gte_OP(1);

    /* Step 4: linear impulse -- self[0x80..0x88] += lin[0..2].
     *
     * MIPS 0x80017614-0x8001764c. */
    *(int32_t *)(s + 0x80) = mips_addu_i32(*(int32_t *)(s + 0x80), lin[0]);
    *(int32_t *)(s + 0x84) = mips_addu_i32(*(int32_t *)(s + 0x84), lin[1]);
    *(int32_t *)(s + 0x88) = mips_addu_i32(*(int32_t *)(s + 0x88), lin[2]);

    /* Step 5: angular impulse.
     *
     * MIPS 0x80017650-0x800176dc:
     *   gte_stMAC1 v1         -- v1 = MAC1
     *   lh v0,0x9c(s2)        -- inertia factor (i16)
     *   mult v1,v0; mflo v1   -- low 32 bits of product
     *   bgez/addiu 0x3f/sra 6 -- RTZ >> 6
     *   lw v0,0x90(s2)
     *   addu v0,v0,v1
     *   sw v0,0x90(s2)
     *   (repeat for MAC2 → 0x9e/0x94, MAC3 → 0xa0/0x98)
     */
    int32_t mac1 = gte_stMAC1();
    int16_t inv1 = *(int16_t *)(s + 0x9c);
    *(int32_t *)(s + 0x90) =
        mips_addu_i32(*(int32_t *)(s + 0x90),
                      rtz6(mips_mult_lo_i32(mac1, inv1)));

    int32_t mac2 = gte_stMAC2();
    int16_t inv2 = *(int16_t *)(s + 0x9e);
    *(int32_t *)(s + 0x94) =
        mips_addu_i32(*(int32_t *)(s + 0x94),
                      rtz6(mips_mult_lo_i32(mac2, inv2)));

    int32_t mac3 = gte_stMAC3();
    int16_t inv3 = *(int16_t *)(s + 0xa0);
    *(int32_t *)(s + 0x98) =
        mips_addu_i32(*(int32_t *)(s + 0x98),
                      rtz6(mips_mult_lo_i32(mac3, inv3)));
}
