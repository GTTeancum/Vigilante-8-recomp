/* gte_long_vec.c -- long-vector rotation helpers built on top of the
 * PSX GTE.
 *
 * PSY-Q's standard rotate-vector ops accept SVECTOR inputs (i16 q12);
 * the engine has long-vector positions (i32 q15.16) that need rotating
 * through the same matrix without precision loss.  The technique used
 * across SLUS:0x80043358-0x800434f8 is:
 *
 *   1. Split each i32 component into upper (>>15 arithmetic) and lower
 *      (& 0x7fff) halves.
 *   2. Load upper halves into IR via gte_ldsv_, issue RTIR_SF0 (sf=0, no
 *      shift).  Save MAC1/2/3.
 *   3. Load lower halves, issue RTIR (sf=1, shift>>12) or RTIRTR (with
 *      translation).  Read MAC1/2/3.
 *   4. Combine: (upper_MAC << 3) + lower_MAC.  The (<<3) comes from the
 *      mismatch between sf=0 (upper kept at q24) and sf=1 (lower at q12);
 *      the 15-bit hi-shift on input compensates for 12-bit sf-shift on
 *      output, leaving a residual factor of 3 bits.
 *
 * This file binds the five symbols:
 *
 *   FUN_80043248  GTE_RotateCurrentLong     -- rotate long v by current M
 *   FUN_80043358  GTE_RotateLongMat        -- rotate long v by M
 *   FUN_80043408  GTE_RotateLongMatTrans   -- rotate long v by M + translate
 *   FUN_8004352c  GTE_RotateLongMatTranspose -- rotate long v by M^T
 *   FUN_8004366c  GTE_LoadMatrixTransposed -- transpose & load M into GTE
 *   FUN_800434f8  GTE_RotateShortMatTranspose -- rotate short v by M^T
 *
 * All five are bit-exact ports of the MIPS bodies at
 * analysis/SLUS_005.10/mips/.  HIGH confidence.
 */
#include <stdint.h>
#include "gte.h"

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR13R21(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR31R32(uint32_t v);
extern void gte_ldR33   (uint32_t v);
extern void gte_ldtr(int x, int y, int z);
extern void gte_ldsv_(int x, int y, int z);
extern void gte_rtir_sf0(void);
extern void gte_rtir(void);
extern void gte_rtirtr(void);
extern void gte_rtv0(void);
extern void gte_ldVXY0(const void *p);
extern void gte_ldVZ0(const void *p);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);
extern int32_t gte_stMAC1(void);
extern int32_t gte_stMAC2(void);
extern int32_t gte_stMAC3(void);

/* HIGH: Transpose-load M into GTE rotation regs.
 * MIPS rearranges the packed (Rij_lo, Rkl_hi) pairs so that the GTE's
 * rotation matrix becomes M^T -- this lets the subsequent RTIR/RTV0
 * compute M^T * v without an explicit transpose buffer. */
void GTE_LoadMatrixTransposed(uint32_t *m)
{
    /* Original packed slots:
     *   m[0] = R11_lo | R12_hi
     *   m[1] = R13_lo | R21_hi
     *   m[2] = R22_lo | R23_hi
     *   m[3] = R31_lo | R32_hi
     *   m[4] = R33_lo | pad_hi
     *
     * Transposed packed slots we want loaded:
     *   t[0] = R11 | R21
     *   t[1] = R31 | R12
     *   t[2] = R22 | R32
     *   t[3] = R13 | R23
     *   t[4] = R33 | pad
     */
    uint32_t v0 = m[0], v1 = m[1];
    uint32_t r11 = v0 & 0xffff;        /* (m[0] & 0xffff) */
    uint32_t r12 = v0 - r11;           /* (R12 << 16)     */
    uint32_t r13 = v1 & 0xffff;
    uint32_t r21 = v1 - r13;           /* (R21 << 16)     */
    gte_ldR11R12(r11 | r21);           /* t[0] = R11 | R21 */

    uint32_t v2 = m[2];
    uint32_t r22 = v2 & 0xffff;
    uint32_t r23 = v2 - r22;
    /* t[1] = R31 | R12.  R31 is the LOW half of m[3]. */
    gte_ldR31R32(r13 | r23);           /* INTENTIONAL: per MIPS, the slot
                                          loaded here is the original
                                          ldR31R32 GTE register being
                                          stuffed with R13|R23 -- effectively
                                          remapping the rotation. */

    uint32_t v3 = m[3];
    uint32_t r31 = v3 & 0xffff;
    uint32_t r32 = v3 - r31;
    uint32_t t0  = m[4];
    gte_ldR13R21(r12 | r31);           /* t (GTE-relative R13|R21) gets R12 | R31 */
    gte_ldR22R23(r22 | r32);
    gte_ldR33(t0);
}

/* Helper: 15-hi / 0x7fff-lo split of an i32 q15.16-ish long vector. */
static inline void split_long(const int32_t v[3],
                              int32_t hi[3], int32_t lo[3])
{
    hi[0] = v[0] >> 15;  lo[0] = v[0] & 0x7fff;
    hi[1] = v[1] >> 15;  lo[1] = v[1] & 0x7fff;
    hi[2] = v[2] >> 15;  lo[2] = v[2] & 0x7fff;
}

/* HIGH: rotate long vector v by M (no translation).
 * v: i32[3] q15.16; out: i32[3] q15.16. */
void GTE_RotateLongMat(const uint32_t *m, const int32_t *v, int32_t *out)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);

    int32_t hi[3], lo[3];
    split_long(v, hi, lo);

    /* Upper half: sf=0, no shift -> MAC has the q24 value. */
    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    /* Lower half: sf=1, shift >>12. */
    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtir();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    /* Combine: upper <<3 (the 15-bit hi-shift on input combined with
     * the 12-bit sf=1 shift on output leaves 3 bits). */
    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
}

/* HIGH: rotate long vector by M then add M->t (the matrix's translation
 * row at +0x14..+0x1c).  Same hi/lo split, RTIRTR for the lower pass. */
void GTE_RotateLongMatTrans(const uint32_t *m, const int32_t *v, int32_t *out)
{
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);

    int32_t hi[3], lo[3];
    split_long(v, hi, lo);

    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    /* For the lower-half pass we load TR from the matrix's t[] entries
     * (at byte offsets +0x14, +0x18, +0x1c which is u32 index 5/6/7) and
     * use RTIRTR (sf=1, cv=TR). */
    int32_t tx = (int32_t)m[5];
    int32_t ty = (int32_t)m[6];
    int32_t tz = (int32_t)m[7];
    gte_ldtr(tx, ty, tz);

    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtirtr();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
}

/* HIGH: rotate long vector by M^T (no translation). */
void GTE_RotateLongMatTranspose(uint32_t *m, const int32_t *v, int32_t *out)
{
    GTE_LoadMatrixTransposed(m);

    int32_t hi[3], lo[3];
    split_long(v, hi, lo);

    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtir();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
}

/* HIGH: rotate short vector by M^T (transpose-load + RTV0).
 * MIPS: ldVXY0/ldVZ0 from a1, jal 0x8004366c (transpose-load M from a0),
 * RTV0, store IR1/2/3 as i16 at a2.
 * Source: SLUS_005.10  FUN_800434f8. */
void GTE_RotateShortMatTranspose(uint32_t *m, const void *sv_in, void *sv_out)
{
    gte_ldVXY0(sv_in);
    gte_ldVZ0((const uint8_t *)sv_in + 4);
    GTE_LoadMatrixTransposed(m);
    gte_rtv0();
    *(int16_t *)((uint8_t *)sv_out + 0) = (int16_t)gte_stIR1();
    *(int16_t *)((uint8_t *)sv_out + 2) = (int16_t)gte_stIR2();
    *(int16_t *)((uint8_t *)sv_out + 4) = (int16_t)gte_stIR3();
}

/* HIGH: rotate an SVECTOR by M^T, output as int32_t[3] (sign-extended i16).
 *
 * FUN_800434d0 (11 MIPS instructions).  Same transpose-load + RTV0 as
 * GTE_RotateShortMatTranspose, but writes 32-bit words to out[] instead of
 * 16-bit.  Used by Vehicle_Collision to transform a contact normal from
 * world space into the vehicle's local frame.
 *
 * Source: SLUS_005.10  0x800434d0
 *
 * MIPS sequence:
 *   gte_ldVXY0 0x0(a1)    -- load V0.VX|VY from SVECTOR word 0
 *   jal 0x8004366c         -- transpose-load M (a0=matrix)
 *   _gte_ldVZ0 0x4(a1)    -- delay: load V0.VZ from SVECTOR word 1
 *   nop
 *   RTV0                   -- M^T * V0, sf=1 -> IR1/2/3
 *   gte_stIR1 0x0(a2)     -- sw (32-bit) to out[0]
 *   gte_stIR2 0x4(a2)     -- sw to out[1]
 *   jr t7
 *   _gte_stIR3 0x8(a2)    -- delay: sw to out[2]
 */
void GTE_RotateNormalTranspose(uint32_t *m, const void *sv_in, int32_t *out)
{
    /* V0 loaded before and during the jal to FUN_8004366c (delay slot).
     * FUN_8004366c does not modify V0, so both loads are safe in any order
     * before gte_rtv0(). */
    gte_ldVXY0(sv_in);
    gte_ldVZ0((const uint8_t *)sv_in + 4);
    GTE_LoadMatrixTransposed(m);
    gte_rtv0();
    out[0] = gte_stIR1();   /* sign-extended i16 stored as i32 */
    out[1] = gte_stIR2();
    out[2] = gte_stIR3();
}

/* HIGH: M^T * (pos - MATRIX.t) -- rotate world delta into object local frame.
 *
 * FUN_800435c0 (36 MIPS instructions).  Transpose-loads M (a0[0..4]),
 * computes delta = a1[0..2] - a0[0x14..0x1c] (= MATRIX.t[0..2]),
 * then applies M^T to the delta using the same 17.15 hi/lo-split technique
 * as GTE_RotateLongMat/Transpose.
 *
 * Used by Vehicle_Collision to transform a world-space contact point into
 * the vehicle's local frame (needed for the cross-product angular impulse).
 *
 * Source: SLUS_005.10  0x800435c0
 *
 * MIPS 0x800435c4: jal FUN_8004366c; _nop  (delay slot NOP -- V0 not loaded)
 * MIPS 0x800435cc-0x800435ec: load pos and MATRIX.t, compute delta.
 * Then hi/lo split and RTIR_SF0/RTIR identical to GTE_RotateLongTranspose.
 */
void GTE_RotateWorldDelta(uint32_t *m, const int32_t *pos, int32_t *out)
{
    GTE_LoadMatrixTransposed(m);

    /* delta = pos - MATRIX.t (translation at byte offset +0x14 from m). */
    const int32_t *t = (const int32_t *)((const uint8_t *)m + 0x14u);
    int32_t d0 = pos[0] - t[0];
    int32_t d1 = pos[1] - t[1];
    int32_t d2 = pos[2] - t[2];

    /* Hi/lo split: same precision technique as GTE_RotateLongMatTranspose. */
    int32_t hi[3], lo[3];
    hi[0] = d0 >> 15;  lo[0] = d0 & 0x7fff;
    hi[1] = d1 >> 15;  lo[1] = d1 & 0x7fff;
    hi[2] = d2 >> 15;  lo[2] = d2 & 0x7fff;

    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtir();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
}

/* =========================================================================
 * Aliases under their FUN_ names -- main_loop and physics call these by
 * raw FUN address (the cleanup pass renamed them but legacy refs remain).
 * ========================================================================= */

void FUN_80043358(uint32_t *m, int32_t *v, int32_t *out)
    { GTE_RotateLongMat(m, v, out); }
int *FUN_80043248(int32_t *v, int32_t *out)
{
    int32_t hi[3], lo[3];
    split_long(v, hi, lo);

    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtir();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
    return out;
}
void FUN_80043408(uint32_t *m, int32_t *v, int32_t *out)
    { GTE_RotateLongMatTrans(m, v, out); }
void FUN_8004352c(uint32_t *m, int32_t *v, int32_t *out)
    { GTE_RotateLongMatTranspose(m, v, out); }
void FUN_8004366c(uint32_t *m)
    { GTE_LoadMatrixTransposed(m); }
void FUN_800434f8(uint32_t *m, void *sv_in, void *sv_out)
    { GTE_RotateShortMatTranspose(m, sv_in, sv_out); }
void FUN_800434d0(uint32_t *m, const void *sv, int32_t *out)
    { GTE_RotateNormalTranspose(m, sv, out); }
void FUN_800435c0(uint32_t *m, const int32_t *pos, int32_t *out)
    { GTE_RotateWorldDelta(m, pos, out); }

/* HIGH: rotate long vector by the GTE's CURRENT matrix and TR (no matrix reload).
 *
 * FUN_800432d0 (22 MIPS instructions, leaf).  Same hi/lo-split technique as
 * GTE_RotateLongMatTrans, but the matrix is already loaded by the caller
 * (caller does gte_set_rot_matrix + gte_set_translation first).  The lower-
 * half pass uses RTIRTR to fold in the currently-loaded translation vector.
 *
 * Source: SLUS_005.10  0x800432d0
 *
 * MIPS:
 *   sra t3,t0,0xf / sra t4,t1,0xf / sra t5,t2,0xf
 *   ldsv_ t3,t4,t5 ; load hi parts into GTE V0
 *   andi t0..t2, 0x7fff
 *   RTIR_SF0        ; R * V0_hi, SF=0 -> MAC
 *   read_mt t3,t4,t5
 *   ldsv_ t0,t1,t2  ; load lo parts
 *   sll t3..t5, 3
 *   RTIRTR           ; R * V0_lo + TR, SF=1 -> MAC
 *   read_mt t0,t1,t2
 *   addu t0,t0,t3 etc ; result = lo + (hi << 3)
 *   store to a1; return a1
 */
void *GTE_RotateCurrentLongMatTrans(const int32_t *v, int32_t *out)
{
    int32_t hi[3], lo[3];
    split_long(v, hi, lo);

    /* Upper half: SF=0, no translation. */
    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0();
    int32_t up1 = gte_stMAC1();
    int32_t up2 = gte_stMAC2();
    int32_t up3 = gte_stMAC3();

    /* Lower half: SF=1, WITH translation (RTIRTR). */
    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtirtr();
    int32_t lo1 = gte_stMAC1();
    int32_t lo2 = gte_stMAC2();
    int32_t lo3 = gte_stMAC3();

    out[0] = (up1 << 3) + lo1;
    out[1] = (up2 << 3) + lo2;
    out[2] = (up3 << 3) + lo3;
    return out;
}

void *FUN_800432d0(const int32_t *v, int32_t *out)
    { return GTE_RotateCurrentLongMatTrans(v, out); }
