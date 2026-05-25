/* matrix_ops.c -- PSX rotation-matrix operations.
 *
 * Implements three functions that orthogonalise/normalise rotation matrices
 * using the GTE (cop2) via the libgte.c software model.
 *
 * Functions:
 *   FUN_8004c874  MSC02_OBJ_100 / VectorNormal   39 instr   HIGH confidence
 *   FUN_800439b8  RotMatrix_ApplyAngVel           37 instr   HIGH confidence
 *   FUN_8004c934  MatrixNormal                    55 instr   HIGH confidence
 *
 * The VectorNormal lookup table at PSX address 0x80060628 is a 192-entry
 * int16_t reciprocal-square-root table.  Values below are byte-verified
 * against SLUS_005.10, not regenerated with host floating-point rounding.
 *
 * MIPS sources:
 *   analysis/SLUS_005.10/mips/8004c874.s
 *   analysis/SLUS_005.10/mips/800439b8.s
 *   analysis/SLUS_005.10/mips/8004c934.s
 *
 * Ghidra references:
 *   analysis/SLUS_005.10/decomp/800439b8.c
 *   analysis/SLUS_005.10/decomp/8004c934.c
 */

#include <stdint.h>
#include "gte.h"

/* FUN_8004366c -- GTE_LoadMatrixTransposed: in src/physics/gte_long_vec.c */
/* FUN_80043408 -- GTE_RotateLongMatTrans:  in src/physics/gte_long_vec.c */
/* FUN_8004352c -- GTE_RotateLongMatTranspose: in src/physics/gte_long_vec.c */

/* ---- GTE function declarations (implemented in platform/psyq/libgte.c) ---- */
extern void     gte_ldR11R12(uint32_t v);
extern void     gte_ldR13R21(uint32_t v);
extern void     gte_ldR22R23(uint32_t v);
extern void     gte_ldR31R32(uint32_t v);
extern void     gte_ldR33   (uint32_t v);
extern void     gte_ldIR0(int v);
extern void     gte_ldIR1(int v);
extern void     gte_ldIR2(int v);
extern void     gte_ldIR3(int v);
extern void     gte_ldsv_(int x, int y, int z);
extern void     gte_rtir_b(void);
extern void     gte_op12_b(void);
extern void     gte_SQR(int sf);
extern void     gte_GPF(int sf);
extern void     gte_ldLZCS(int32_t v);
extern int32_t  gte_stLZCR(void);
extern int32_t  gte_stIR1(void);
extern int32_t  gte_stIR2(void);
extern int32_t  gte_stIR3(void);
extern int32_t  gte_stMAC1(void);
extern int32_t  gte_stMAC2(void);
extern int32_t  gte_stMAC3(void);
extern uint32_t gte_stR11R12(void);
extern uint32_t gte_stR22R23(void);
extern uint32_t gte_stR33   (void);
/* Integer-argument V0/V1 register stash (for scratch across GTE calls). */
extern void     gte_ldVXY0_i(int v);
extern void     gte_ldVZ0_i (int v);
extern void     gte_ldVXY1  (int v);
extern int32_t  gte_stVXY0_V0(void);
extern int32_t  gte_stVZ0_V0 (void);
extern int32_t  gte_stVXY1_V1(void);

/* ==========================================================================
 * VectorNormal lookup table: 192 int16 entries at PSX 0x80060628.
 * Each entry is the approximate reciprocal sqrt of a number in [64, 256),
 * scaled to q12 (4096 = 1.0).  Used to normalise a vector whose squared
 * magnitude has been brought into [0x40, 0x80) by the leading-zero count.
 * ========================================================================== */
static const int16_t s_vnormal_tbl[192] = {
    /* i=  0..  7 */ 4096, 4064, 4033, 4003, 3973, 3944, 3916, 3888,
    /* i=  8.. 15 */ 3861, 3835, 3809, 3783, 3758, 3734, 3710, 3686,
    /* i= 16.. 23 */ 3663, 3640, 3618, 3596, 3575, 3554, 3533, 3513,
    /* i= 24.. 31 */ 3493, 3473, 3454, 3435, 3416, 3397, 3379, 3361,
    /* i= 32.. 39 */ 3344, 3327, 3310, 3293, 3276, 3260, 3244, 3228,
    /* i= 40.. 47 */ 3213, 3197, 3182, 3167, 3153, 3138, 3124, 3110,
    /* i= 48.. 55 */ 3096, 3082, 3069, 3055, 3042, 3029, 3016, 3003,
    /* i= 56.. 63 */ 2991, 2978, 2966, 2954, 2942, 2930, 2919, 2907,
    /* i= 64.. 71 */ 2896, 2885, 2873, 2862, 2852, 2841, 2830, 2820,
    /* i= 72.. 79 */ 2809, 2799, 2789, 2779, 2769, 2759, 2749, 2740,
    /* i= 80.. 87 */ 2730, 2721, 2711, 2702, 2693, 2684, 2675, 2666,
    /* i= 88.. 95 */ 2657, 2649, 2640, 2631, 2623, 2615, 2606, 2598,
    /* i= 96..103 */ 2590, 2582, 2574, 2566, 2558, 2550, 2543, 2535,
    /* i=104..111 */ 2528, 2520, 2513, 2505, 2498, 2491, 2484, 2477,
    /* i=112..119 */ 2469, 2462, 2456, 2449, 2442, 2435, 2428, 2422,
    /* i=120..127 */ 2415, 2409, 2402, 2396, 2389, 2383, 2377, 2371,
    /* i=128..135 */ 2364, 2358, 2352, 2346, 2340, 2334, 2328, 2322,
    /* i=136..143 */ 2317, 2311, 2305, 2299, 2294, 2288, 2283, 2277,
    /* i=144..151 */ 2272, 2266, 2261, 2255, 2250, 2245, 2239, 2234,
    /* i=152..159 */ 2229, 2224, 2219, 2214, 2209, 2204, 2199, 2194,
    /* i=160..167 */ 2189, 2184, 2179, 2174, 2170, 2165, 2160, 2155,
    /* i=168..175 */ 2151, 2146, 2142, 2137, 2133, 2128, 2124, 2119,
    /* i=176..183 */ 2115, 2110, 2106, 2102, 2097, 2093, 2089, 2084,
    /* i=184..191 */ 2080, 2076, 2072, 2068, 2064, 2060, 2056, 2052,
};

static int32_t host_lzcr(int32_t v)
{
    uint32_t u = (v < 0) ? ~(uint32_t)v : (uint32_t)v;
    int n = 0;
    while (n < 32 && (u & 0x80000000u) == 0) {
        n++;
        u <<= 1;
    }
    return n;
}

static int32_t sar32(int32_t v, int sh)
{
    return v >> (sh & 31);
}

static int32_t sal32(int32_t v, int sh)
{
    return (int32_t)((uint32_t)v << (sh & 31));
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t sat_i16(int32_t v)
{
    if (v < -0x8000)
        return -0x8000;
    if (v > 0x7fff)
        return 0x7fff;
    return v;
}

/* ==========================================================================
 * FUN_8004c874 -- MSC02_OBJ_100 / VectorNormal
 * PSX address: 0x8004c874   Instruction count: 39   Confidence: HIGH
 *
 * Normalises a 3-component integer vector to q12 unit length using the
 * GTE SQR + LZCS/LZCR + GPF pipeline.
 *
 * Calling convention (from MIPS): the function receives the three vector
 * components in MIPS t0/t1/t2 (caller-saved regs) and returns the
 * normalised components in the same three registers.  In C we model this
 * as: take (x, y, z) by value, return three int32 results through the
 * pointers (*ox, *oy, *oz).  The GTE IR1/2/3 and MAC1/2/3 state is also
 * updated as a side-effect (matching the original).
 *
 * MIPS body (8004c874.s):
 *   ldsv_ t0,t1,t2          ; IR1=x, IR2=y, IR3=z
 *   SQR0                    ; MAC[r] = IR[r]^2
 *   read_mt t3,t4,t5        ; t3=MAC1, t4=MAC2, t5=MAC3
 *   add t3,t3,t4; add v0,t3,t5  ; v0 = MAC1+MAC2+MAC3 (= |v|^2)
 *   gte_ldLZCS v0           ; LZCS = |v|^2
 *   gte_stLZCR v1           ; v1 = leading zeros of |v|^2
 *   and v1,v1,~1            ; v1 &= ~1 (even)
 *   t6 = (0x1f - v1) >> 1  ; right-shift for output
 *   t3 = v1 - 0x18
 *   if t3 < 0: t4 = v0 >> (0x18 - v1)    ; shift down to [0x40, 0x80)
 *   else:       t4 = v0 << (v1 - 0x18)   ; shift up
 *   t4 -= 0x40                            ; table index
 *   t4 *= 2                               ; byte offset
 *   t5 = table[t4]                        ; approx reciprocal sqrt
 *   gte_ldIR0 t5
 *   ldsv_ t0,t1,t2                        ; restore IR from original
 *   GPF0                                  ; MAC[r] = IR0 * IR[r]
 *   read_mt t0,t1,t2                      ; t0..t2 = MAC1..3
 *   srav t0,t0,t6; srav t1,t1,t6; srav t2,t2,t6
 * ========================================================================== */
void FUN_8004c874(int x, int y, int z, int *ox, int *oy, int *oz)
{
    int32_t ix = (int16_t)x;
    int32_t iy = (int16_t)y;
    int32_t iz = (int16_t)z;
    int32_t m1 = ix * ix;
    int32_t m2 = iy * iy;
    int32_t m3 = iz * iz;
    int32_t mag2 = (int32_t)((uint32_t)(m1 + m2) + (uint32_t)m3);
    int32_t lz = host_lzcr(mag2) & ~1;
    int32_t out_shift = (0x1f - lz) >> 1;
    int32_t norm_shift = lz - 0x18;
    int32_t norm_mag = (norm_shift < 0) ?
        sar32(mag2, 0x18 - lz) :
        sal32(mag2, norm_shift);
    int32_t idx = norm_mag - 0x40;

    if (idx < 0 || idx >= 192) {
        *ox = 0;
        *oy = 0;
        *oz = 0;
        return;
    }

    int32_t ir0 = s_vnormal_tbl[idx];
    *ox = sar32(ir0 * ix, out_shift);
    *oy = sar32(ir0 * iy, out_shift);
    *oz = sar32(ir0 * iz, out_shift);
}

/* ==========================================================================
 * FUN_800439b8 -- RotMatrix_ApplyAngVel
 * PSX address: 0x800439b8   Instruction count: 37   Confidence: HIGH
 *
 * Applies an infinitesimal angular velocity (param_2, param_3, param_4) to
 * the rotation matrix stored at param_1 (5 packed uint32 words = 9 i16 in
 * PSX MATRIX format).  The three RTIR calls compute the new columns of the
 * rotation matrix:
 *
 *   column 0 new = R * (1000, a3, -a2)    (= R * (1, omega_y, -omega_x))
 *   column 1 new = R * (-a3, 1000, a2)   (= R * (-omega_y, 1, omega_x))
 *   column 2 new = R * (a2, -a1, 1000)   (= R * (omega_x, -omega_z, 1))
 *
 * where 0x1000 = 1.0 in q12 and a1=param_2 (roll), a2=param_3 (pitch),
 * a3=param_4 (yaw) in q12.
 *
 * MIPS body (800439b8.s):
 *   lw t0..t4, 0..0x10(a0)
 *   gte_ldR11R12 t0; gte_ldR13R21 t1; gte_ldR22R23 t2; gte_ldR31R32 t3; gte_ldR33 t4
 *
 *   RTIR 1:  IR = (0x1000, a3, -a2)
 *   RTIR      -> stIR1 t1, stIR2 t2, stIR3 t3  [= result1]
 *   ldsv_(-a3, 0x1000, a1)
 *   RTIR 2:
 *   sh result1 -> [0],[6],[c]
 *   stIR1 t1, stIR2 t2, stIR3 t3               [= result2]
 *   ldsv_(a2, -a1, 0x1000)
 *   RTIR 3:
 *   sh result2 -> [2],[8],[e]
 *   stIR1 t1, stIR2 t2, stIR3 t3               [= result3]
 *   sh result3 -> [4],[a],[10]
 *
 * Matrix byte layout (uint8_t*):
 *   [0] R11, [2] R12, [4] R13
 *   [6] R21, [8] R22, [a] R23
 *   [c] R31, [e] R32, [10] R33
 * ========================================================================== */
void FUN_800439b8(uint32_t *param_1, int param_2, int param_3, int param_4)
{
    /* MIPS: lw t0..t4, 0..0x10(a0) */
    uint32_t w0 = param_1[0];
    uint32_t w1 = param_1[1];
    uint32_t w2 = param_1[2];
    uint32_t w3 = param_1[3];
    uint32_t w4 = param_1[4];

    /* Load current rotation matrix into GTE */
    gte_ldR11R12(w0);
    gte_ldR13R21(w1);
    gte_ldR22R23(w2);
    gte_ldR31R32(w3);
    gte_ldR33   (w4);

    /* RTIR 1: IR = (0x1000, param_4, -param_3) */
    gte_ldIR1(0x1000);
    gte_ldIR2(param_4);
    gte_ldIR3(mips_subu_i32(0, param_3));
    /* sub t0,zero,a3: t0 = -param_4 (prepared for next ldsv_; no GTE effect) */
    gte_rtir_b();
    /* stIR -> result1 */
    int r1x = (int16_t)gte_stIR1();
    int r1y = (int16_t)gte_stIR2();
    int r1z = (int16_t)gte_stIR3();

    /* ldsv_(-param_4, 0x1000, param_2) */
    gte_ldsv_(mips_subu_i32(0, param_4), 0x1000, param_2);

    /* RTIR 2 */
    gte_rtir_b();
    /* Write result1 to matrix columns: [0]=R11, [6]=R21, [c]=R31 */
    {
        uint8_t *p = (uint8_t *)param_1;
        *(int16_t *)(p + 0x0) = (int16_t)r1x;
        *(int16_t *)(p + 0x6) = (int16_t)r1y;
        *(int16_t *)(p + 0xc) = (int16_t)r1z;
    }
    /* sub t0,zero,a1: t0 = -param_2 (for next ldsv_) */
    /* stIR -> result2 */
    int r2x = (int16_t)gte_stIR1();
    int r2y = (int16_t)gte_stIR2();
    int r2z = (int16_t)gte_stIR3();

    /* ldsv_(param_3, -param_2, 0x1000) */
    gte_ldsv_(param_3, mips_subu_i32(0, param_2), 0x1000);

    /* RTIR 3 */
    gte_rtir_b();
    /* Write result2: [2]=R12, [8]=R22, [e]=R32 */
    {
        uint8_t *p = (uint8_t *)param_1;
        *(int16_t *)(p + 0x2) = (int16_t)r2x;
        *(int16_t *)(p + 0x8) = (int16_t)r2y;
        *(int16_t *)(p + 0xe) = (int16_t)r2z;
    }
    /* stIR -> result3 */
    int r3x = (int16_t)gte_stIR1();
    int r3y = (int16_t)gte_stIR2();
    int r3z = (int16_t)gte_stIR3();
    /* Write result3: [4]=R13, [a]=R23, [10]=R33 */
    {
        uint8_t *p = (uint8_t *)param_1;
        *(int16_t *)(p + 0x4) = (int16_t)r3x;
        *(int16_t *)(p + 0xa) = (int16_t)r3y;
        *(int16_t *)(p + 0x10) = (int16_t)r3z;
    }
}

/* ==========================================================================
 * FUN_8004c934 -- MatrixNormal
 * PSX address: 0x8004c934   Instruction count: 55   Confidence: HIGH
 *
 * Re-orthonormalises rotation matrix *m into *n using GTE OP12 (outer
 * product / cross product) and FUN_8004c874 (vector normalise).
 *
 * Algorithm (Gram-Schmidt-like, fully integer/GTE):
 *   Let row0 = m->m[0], row1 = m->m[1].
 *   1. cross1 = OP(row1, diag(row0)) = cross(row1, row0) -- new row2 candidate
 *   2. cross2 = OP(cross1, diag(row1)) = cross(cross1, row1) -- new row0 candidate
 *   3. normalize(cross2) -> n->m[0]
 *   4. normalize(row1) -> n->m[1]
 *   5. normalize(cross1) -> n->m[2]
 *
 * GTE register save/restore: the function saves R11R12/R22R23/R33 before
 * the OP12 calls (which overwrite the diagonal registers) and restores them
 * after the second OP12.  This faithfully mirrors the MIPS.
 *
 * MIPS body (8004c934.s):
 *   lh t0..t5, 0..0xa(a0)          ; row0[0..2], row1[0..2]
 *   gte_stR11R12 v0; gte_stR22R23 v1; gte_stR33 a2   [save GTE R diag]
 *   gte_ldR11R12 t0; gte_ldR22R23 t1; gte_ldR33 t2   [diag = row0]
 *   gte_ldIR3 t5; gte_ldIR1 t3; gte_ldIR2 t4         [IR = row1]
 *   OP12  -> read_mt t7,t8,t9                         [cross1 in MAC/IR]
 *   gte_ldR11R12 t3; gte_ldR22R23 t4; gte_ldR33 t5   [diag = row1]
 *   OP12  -> gte_ldVXY0 t3; gte_ldVZ0 t4; gte_ldVXY1 t5 [stash row1]
 *            read_mt t0,t1,t2                         [cross2 in t0..t2]
 *   gte_ldR11R12 v0; gte_ldR22R23 v1; gte_ldR33 a2   [restore GTE R diag]
 *   move a3,ra; jal FUN_8004c874 -> sh t0..2, 0..4(a1) [n->m[0]]
 *   gte_stVXY0 t0; gte_stVZ0 t1; gte_stVXY1 t2
 *   jal FUN_8004c874 -> sh t0..2, 6..a(a1)            [n->m[1]]
 *   move t0,t7; move t1,t8; jal FUN_8004c874 _move t2,t9 -> sh [c..10] [n->m[2]]
 *   move ra,a3
 * ========================================================================== */
void FUN_8004c934(MATRIX *m, MATRIX *n)
{
    int32_t r0x = (int32_t)m->m[0][0];
    int32_t r0y = (int32_t)m->m[0][1];
    int32_t r0z = (int32_t)m->m[0][2];
    int32_t r1x = (int32_t)m->m[1][0];
    int32_t r1y = (int32_t)m->m[1][1];
    int32_t r1z = (int32_t)m->m[1][2];

    uint32_t saved_r11r12 = gte_stR11R12();
    uint32_t saved_r22r23 = gte_stR22R23();
    uint32_t saved_r33    = gte_stR33();

    gte_ldR11R12((uint32_t)r0x);
    gte_ldR22R23((uint32_t)r0y);
    gte_ldR33((uint32_t)r0z);
    gte_ldIR3(r1z);
    gte_ldIR1(r1x);
    gte_ldIR2(r1y);
    gte_op12_b();
    int32_t c1x = gte_stMAC1();
    int32_t c1y = gte_stMAC2();
    int32_t c1z = gte_stMAC3();

    gte_ldR11R12((uint32_t)r1x);
    gte_ldR22R23((uint32_t)r1y);
    gte_ldR33((uint32_t)r1z);
    gte_op12_b();
    int32_t c2x = gte_stMAC1();
    int32_t c2y = gte_stMAC2();
    int32_t c2z = gte_stMAC3();

    gte_ldR11R12(saved_r11r12);
    gte_ldR22R23(saved_r22r23);
    gte_ldR33(saved_r33);

    int ox, oy, oz;
    FUN_8004c874(c2x, c2y, c2z, &ox, &oy, &oz);
    n->m[0][0] = (int16_t)ox;
    n->m[0][1] = (int16_t)oy;
    n->m[0][2] = (int16_t)oz;

    FUN_8004c874(r1x, r1y, r1z, &ox, &oy, &oz);
    n->m[1][0] = (int16_t)ox;
    n->m[1][1] = (int16_t)oy;
    n->m[1][2] = (int16_t)oz;

    FUN_8004c874(c1x, c1y, c1z, &ox, &oy, &oz);
    n->m[2][0] = (int16_t)ox;
    n->m[2][1] = (int16_t)oy;
    n->m[2][2] = (int16_t)oz;
}

/* NOTE: FUN_8004366c, FUN_80043408, FUN_8004352c are implemented in
 *       src/physics/gte_long_vec.c -- do not re-define here. */

/* ==========================================================================
 * Ghidra reference for FUN_800439b8 (analysis/SLUS_005.10/decomp/800439b8.c)
 * ==========================================================================
#if 0
void FUN_800439b8(undefined4 *param_1,int param_2,int param_3,int param_4)
{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;

  gte_ldR11R12(*param_1);
  gte_ldR13R21(param_1[1]);
  gte_ldR22R23(param_1[2]);
  gte_ldR31R32(param_1[3]);
  gte_ldR33(param_1[4]);
  gte_ldIR1(0x1000);
  gte_ldIR2(param_4);
  gte_ldIR3(-param_3);
  gte_rtir_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(-param_4,0x1000,param_2);
  gte_rtir_b();
  *(short *)param_1 = (short)uVar1;
  *(short *)((int)param_1 + 6) = (short)uVar2;
  *(short *)(param_1 + 3) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(param_3,-param_2,0x1000);
  gte_rtir_b();
  *(short *)((int)param_1 + 2) = (short)uVar1;
  *(short *)(param_1 + 2) = (short)uVar2;
  *(short *)((int)param_1 + 0xe) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *(short *)(param_1 + 1) = (short)uVar1;
  *(short *)((int)param_1 + 10) = (short)uVar2;
  *(short *)(param_1 + 4) = (short)uVar3;
  return;
}
#endif

 * ==========================================================================
 * Ghidra reference for FUN_8004c934 (analysis/SLUS_005.10/decomp/8004c934.c)
 * ==========================================================================
#if 0
void MatrixNormal(MATRIX *m,MATRIX *n)
{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  short sVar4;
  short sVar5;
  int iVar6;
  short sVar7;
  short sVar8;
  int iVar9;
  short sVar10;
  short sVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  undefined4 in_t7;
  undefined4 in_t8;
  undefined4 in_t9;

  iVar6 = (int)m->m[0][0];
  iVar9 = (int)m->m[0][1];
  iVar12 = (int)m->m[0][2];
  iVar13 = (int)m->m[1][0];
  iVar14 = (int)m->m[1][1];
  iVar15 = (int)m->m[1][2];
  uVar1 = gte_stR11R12();
  uVar2 = gte_stR22R23();
  uVar3 = gte_stR33();
  gte_ldR11R12(iVar6);
  gte_ldR22R23(iVar9);
  gte_ldR33(iVar12);
  gte_ldIR3(iVar15);
  gte_ldIR1(iVar13);
  gte_ldIR2(iVar14);
  gte_op12_b();
  read_mt(in_t7,in_t8,in_t9);
  gte_ldR11R12(iVar13);
  gte_ldR22R23(iVar14);
  gte_ldR33(iVar15);
  gte_op12_b();
  gte_ldVXY0(iVar13);
  gte_ldVZ0(iVar14);
  gte_ldVXY1(iVar15);
  read_mt(iVar6,iVar9,iVar12);
  gte_ldR11R12(uVar1);
  gte_ldR22R23(uVar2);
  gte_ldR33(uVar3);
  MSC02_OBJ_100(m,n,uVar3);
  sVar11 = (short)in_t9;
  sVar8 = (short)in_t8;
  sVar5 = (short)in_t7;
  n->m[0][0] = (short)iVar6;
  n->m[0][1] = (short)iVar9;
  n->m[0][2] = (short)iVar12;
  uVar1 = gte_stVXY0();
  sVar4 = (short)uVar1;
  uVar1 = gte_stVZ0();
  sVar7 = (short)uVar1;
  uVar1 = gte_stVXY1();
  sVar10 = (short)uVar1;
  MSC02_OBJ_100();
  n->m[1][0] = sVar4;
  n->m[1][1] = sVar7;
  n->m[1][2] = sVar10;
  MSC02_OBJ_100();
  n->m[2][0] = sVar5;
  n->m[2][1] = sVar8;
  n->m[2][2] = sVar11;
  return;
}
#endif
*/
