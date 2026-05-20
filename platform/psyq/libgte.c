/* libgte.c -- bit-exact PSX GTE (cop2) software model.
 *
 * Implementation references nocash PSX-SPX (public hardware spec).
 * Goal: byte-for-byte equivalent output to PSX hardware for every
 * operation the cleaned engine code invokes.
 *
 * Register file layout:
 *   data[0..5]   V0, V1, V2 vectors (xy packed lo, z separate)
 *   data[6]      RGBC
 *   data[7]      OTZ (average Z for sort)
 *   data[8..11]  IR0, IR1, IR2, IR3
 *   data[12..15] SXY0..SXY2, SXYP (screen XY ring)
 *   data[16..19] SZ0..SZ3 (depth ring)
 *   data[20..22] RGB0..RGB2 (color ring)
 *   data[24..27] MAC0..MAC3 (accumulators)
 *   data[28..29] IRGB, ORGB (color subset; not implemented yet)
 *   data[30..31] LZCS, LZCR (leading-zero count)
 *
 *   ctl[0..4]    Rotation matrix RT (5 u32 packed; R11|R12, R13|R21, ...)
 *   ctl[5..7]    Translation vector TR (3 i32)
 *   ctl[8..12]   Light matrix L
 *   ctl[13..15]  Background color BK
 *   ctl[16..20]  Light-color matrix LC
 *   ctl[21..23]  Far color FC
 *   ctl[24..25]  Screen offset OFX/OFY (i32, q16)
 *   ctl[26]      H (projection plane distance, u16)
 *   ctl[27..28]  DQA, DQB (depth queue)
 *   ctl[29..30]  ZSF3, ZSF4 (depth scale factor)
 *   ctl[31]      FLAG (sticky error/sat bits)
 *
 * MVMVA fields:
 *   sf  (bit 19): 0 = no shift, 1 = right-shift result by 12
 *   mx  (bit 17-18): 0=R, 1=L, 2=LC, 3=invalid
 *   v   (bit 15-16): 0=V0, 1=V1, 2=V2, 3=IR
 *   cv  (bit 13-14): 0=TR, 1=BK, 2=FC-buggy, 3=none
 *   lm  (bit 10): 0 = signed i16 sat, 1 = unsigned 0..0x7fff sat
 *
 * The 43-bit MAC overflow flags are tracked per the PSX-SPX § "GTE Flag
 * Register" definitions. IR saturation flags set on clamp.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "psyq_stubs.h"
#include "gte.h"

/* ---- register file ---- */
static int32_t g_data[32];
static int32_t g_ctl[32];

#define V0_XY     0
#define V0_Z      1
#define V1_XY     2
#define V1_Z      3
#define V2_XY     4
#define V2_Z      5
#define RGBC      6
#define OTZ       7
#define IR0       8
#define IR1_      9
#define IR2_     10
#define IR3_     11
#define SXY0     12
#define SXY1     13
#define SXY2     14
#define SXYP     15
#define SZ0      16
#define SZ1      17
#define SZ2      18
#define SZ3      19
#define RGB0     20
#define RGB1     21
#define RGB2     22
#define MAC0     24
#define MAC1     25
#define MAC2     26
#define MAC3     27
#define IRGB     28
#define ORGB     29
#define LZCS     30
#define LZCR     31

#define RT_11_12  0
#define RT_13_21  1
#define RT_22_23  2
#define RT_31_32  3
#define RT_33     4
#define TR_X      5
#define TR_Y      6
#define TR_Z      7
#define L_11_12   8
#define L_13_21   9
#define L_22_23  10
#define L_31_32  11
#define L_33     12
#define BK_R     13
#define BK_G     14
#define BK_B     15
#define LC_11_12 16
#define LC_13_21 17
#define LC_22_23 18
#define LC_31_32 19
#define LC_33    20
#define FC_R     21
#define FC_G     22
#define FC_B     23
#define OFX      24
#define OFY      25
#define H_DIST   26
#define DQA      27
#define DQB      28
#define ZSF3     29
#define ZSF4     30
#define FLAG     31

/* FLAG register bit positions per nocash. */
#define FLAG_MAC1_POS   (1u << 30)
#define FLAG_MAC2_POS   (1u << 29)
#define FLAG_MAC3_POS   (1u << 28)
#define FLAG_MAC1_NEG   (1u << 27)
#define FLAG_MAC2_NEG   (1u << 26)
#define FLAG_MAC3_NEG   (1u << 25)
#define FLAG_IR1_SAT    (1u << 24)
#define FLAG_IR2_SAT    (1u << 23)
#define FLAG_IR3_SAT    (1u << 22)
#define FLAG_COLOR_R    (1u << 21)
#define FLAG_COLOR_G    (1u << 20)
#define FLAG_COLOR_B    (1u << 19)
#define FLAG_SZ3_SAT    (1u << 18)
#define FLAG_DIV0       (1u << 17)
#define FLAG_MAC0_POS   (1u << 16)
#define FLAG_MAC0_NEG   (1u << 15)
#define FLAG_SX2_SAT    (1u << 14)
#define FLAG_SY2_SAT    (1u << 13)
#define FLAG_IR0_SAT    (1u << 12)
#define FLAG_ERROR_SUMMARY (1u << 31)
#define FLAG_ERROR_MASK 0x7f87e000u   /* bits whose union sets bit 31 */

static void flag_update_summary(void) {
    if (((uint32_t)g_ctl[FLAG]) & FLAG_ERROR_MASK)
        g_ctl[FLAG] |= FLAG_ERROR_SUMMARY;
    else
        g_ctl[FLAG] &= ~FLAG_ERROR_SUMMARY;
}

/* Check 43-bit signed range. PSX sets a MAC overflow flag if the
 * intermediate sum exceeds the 43-bit signed range. */
static int64_t mac_overflow_check(int64_t v, uint32_t pos_flag, uint32_t neg_flag) {
    if (v >=  (int64_t)1 << 43) g_ctl[FLAG] |= pos_flag;
    if (v <  -((int64_t)1 << 43)) g_ctl[FLAG] |= neg_flag;
    return v;
}

/* Clamp MAC1/2/3 from 64-bit accumulator to 32-bit MAC value (after
 * SF shift). Per nocash the MAC value is "truncated to signed 32 bits"
 * after shift. */
static int32_t mac_clip32(int64_t v) {
    /* simple cast -- PSX wraps i32 on overflow */
    return (int32_t)v;
}

/* IR saturation per LM flag.
 *   LM=0: clamp to signed i16 [-0x8000, +0x7fff]
 *   LM=1: clamp to unsigned 0..0x7fff
 * Returns the saturated value and sets the saturation flag if clamped. */
static int32_t ir_saturate(int32_t v, int lm, uint32_t sat_flag) {
    if (lm) {
        if (v < 0)       { g_ctl[FLAG] |= sat_flag; return 0; }
        if (v > 0x7fff)  { g_ctl[FLAG] |= sat_flag; return 0x7fff; }
        return v;
    }
    if (v < -0x8000) { g_ctl[FLAG] |= sat_flag; return -0x8000; }
    if (v >  0x7fff) { g_ctl[FLAG] |= sat_flag; return  0x7fff; }
    return v;
}

/* ---- helpers: read packed matrix elements ---- */

static int16_t mtx_elem(int mx, int r, int c) {
    /* mx: 0=R, 1=L, 2=LC; base is ctl[0], ctl[8], ctl[16] respectively. */
    int base = (mx == 0) ? RT_11_12 : (mx == 1) ? L_11_12 : LC_11_12;
    int idx  = r * 3 + c;
    /* Each ctl[base+k] holds two i16 (low=even-element, high=odd-element).
     * Mapping: idx 0->ctl[base+0].lo, 1->ctl[base+0].hi, 2->ctl[base+1].lo,
     * 3->ctl[base+1].hi, 4->ctl[base+2].lo, 5->ctl[base+2].hi,
     * 6->ctl[base+3].lo, 7->ctl[base+3].hi, 8->ctl[base+4].lo. */
    int word = base + (idx >> 1);
    int hi   = idx & 1;
    uint32_t u = (uint32_t)g_ctl[word];
    return (int16_t)(hi ? (u >> 16) : (u & 0xffff));
}

/* Read a vector for MVMVA's V field.
 *   v=0..2: V0/V1/V2 (i16 q12)
 *   v=3:    IR (i32 q12)  */
static int32_t vec_elem(int v, int i) {
    if (v == 3) {
        return g_data[IR1_ + i];      /* IR1, IR2, IR3 are i32 */
    }
    /* V0/V1/V2 are packed: data[V0_XY] holds (Vx.lo, Vy.hi), data[V0_Z] holds Vz.lo */
    int base = (v == 0) ? V0_XY : (v == 1) ? V1_XY : V2_XY;
    if (i < 2) {
        uint32_t u = (uint32_t)g_data[base];
        return (int16_t)(i ? (u >> 16) : (u & 0xffff));
    }
    return (int16_t)(g_data[base + 1] & 0xffff);
}

/* Read the translation vector for MVMVA's CV field.
 *   cv=0: TR (32-bit per component)
 *   cv=1: BK
 *   cv=2: FC (PSX-buggy; we honor the spec but it's rarely useful)
 *   cv=3: zero  */
static int32_t cv_elem(int cv, int i) {
    if (cv == 3) return 0;
    int base = (cv == 0) ? TR_X : (cv == 1) ? BK_R : FC_R;
    return g_ctl[base + i];
}

/* ---- the central MVMVA implementation ---- */

/* Run MVMVA with parameter fields. Per nocash:
 *   For each row r in {0,1,2}:
 *     sum = (CV[r] << 12) + M[r][0]*V[0] + M[r][1]*V[1] + M[r][2]*V[2]
 *           (sum is 43-bit signed; overflow flags if outside that range)
 *     MAC[r] = (sum >> 12*sf) truncated to i32
 *     IR[r]  = saturate(MAC[r], lm)
 *
 * Note the "<<12" on CV is because CV is q16 (i32 packed at scale 1)
 * while the products are q24 (i16*i16 = q24); to add them in MAC the
 * CV is upshifted by 12 so both are at q24 scale, then the result is
 * (optionally) shifted right by 12 to get q12.  When CV is FC (cv=2)
 * there is a documented quirk; we replicate by setting IR=sat(MAC)
 * with the same path. */
static void mvmva(int mx, int v, int cv, int sf, int lm)
{
    static const uint32_t MAC_POS[3] = { FLAG_MAC1_POS, FLAG_MAC2_POS, FLAG_MAC3_POS };
    static const uint32_t MAC_NEG[3] = { FLAG_MAC1_NEG, FLAG_MAC2_NEG, FLAG_MAC3_NEG };
    static const uint32_t IR_SAT[3]  = { FLAG_IR1_SAT,  FLAG_IR2_SAT,  FLAG_IR3_SAT  };

    int shift = sf ? 12 : 0;

    /* Clear FLAG before op (FLAG is sticky; nocash says it's clobbered
     * per-op). Real PSX clears all sticky flags at the start of each
     * GTE op. */
    g_ctl[FLAG] = 0;

    for (int r = 0; r < 3; r++) {
        int64_t sum = (int64_t)cv_elem(cv, r) << 12;
        for (int c = 0; c < 3; c++) {
            int64_t prod = (int64_t)mtx_elem(mx, r, c) * (int64_t)vec_elem(v, c);
            sum += prod;
        }
        sum = mac_overflow_check(sum, MAC_POS[r], MAC_NEG[r]);
        if (shift) sum >>= shift;
        int32_t mac = mac_clip32(sum);
        g_data[MAC1 + r] = mac;
        g_data[IR1_ + r] = ir_saturate(mac, lm, IR_SAT[r]);
    }
    flag_update_summary();
}

/* ===========================================================
 * The convenience entry points the cleaned engine code calls.
 * Each is implemented in terms of mvmva() or direct register
 * read/writes.  Naming follows the PSY-Q inline-asm macros that
 * Ghidra surfaced for cop2 instructions.
 * =========================================================== */

/* Matrix loads -- write packed (Rij_lo, Rkl_hi) into the rotation
 * matrix words (ctl[0..4]).  The cleaned code calls these to copy
 * an in-memory MATRIX struct into the GTE for the next op. */
void gte_ldR11R12(uint32_t v) { g_ctl[RT_11_12] = (int32_t)v; }
void gte_ldR13R21(uint32_t v) { g_ctl[RT_13_21] = (int32_t)v; }
void gte_ldR22R23(uint32_t v) { g_ctl[RT_22_23] = (int32_t)v; }
void gte_ldR31R32(uint32_t v) { g_ctl[RT_31_32] = (int32_t)v; }
void gte_ldR33   (uint32_t v) { g_ctl[RT_33]    = (int32_t)v; }

/* Translation-vector loads. */
void gte_ldTRX(uint32_t v) { g_ctl[TR_X] = (int32_t)v; }
void gte_ldTRY(uint32_t v) { g_ctl[TR_Y] = (int32_t)v; }
void gte_ldTRZ(uint32_t v) { g_ctl[TR_Z] = (int32_t)v; }
void gte_ldtr (int x, int y, int z) { g_ctl[TR_X]=x; g_ctl[TR_Y]=y; g_ctl[TR_Z]=z; }

/* IR loads (write i16 sat 16-bit values into the IR registers, which
 * are stored as i32 sign-extended).  PSY-Q's gte_ldIRn loads the value
 * verbatim; if it doesn't fit i16 the result is wrap, no flag. */
void gte_ldIR1(int v) { g_data[IR1_] = (int32_t)(int16_t)v; }
void gte_ldIR2(int v) { g_data[IR2_] = (int32_t)(int16_t)v; }
void gte_ldIR3(int v) { g_data[IR3_] = (int32_t)(int16_t)v; }

/* Load short-vector to V0 (used by RTPS-family). */
void gte_ldv0(const SVECTOR *sv) {
    uint32_t xy = (uint16_t)sv->vx | ((uint32_t)(uint16_t)sv->vy << 16);
    g_data[V0_XY] = (int32_t)xy;
    g_data[V0_Z]  = (int32_t)(uint16_t)sv->vz;
}

/* "Load short vector" -- write (x, y, z) into IR1/2/3 directly,
 * truncating to i16. */
void gte_ldsv_(int x, int y, int z) {
    g_data[IR1_] = (int32_t)(int16_t)x;
    g_data[IR2_] = (int32_t)(int16_t)y;
    g_data[IR3_] = (int32_t)(int16_t)z;
}

/* IR readbacks. */
int32_t gte_stIR1(void) { return (int16_t)g_data[IR1_]; }
int32_t gte_stIR2(void) { return (int16_t)g_data[IR2_]; }
int32_t gte_stIR3(void) { return (int16_t)g_data[IR3_]; }

/* Store the long-vector form of IR (i.e. the MAC1/2/3 32-bit accumulator). */
void gte_stlvnl(VECTOR *out) {
    out->vx = g_data[MAC1];
    out->vy = g_data[MAC2];
    out->vz = g_data[MAC3];
}

/* MVMVA convenience forms.  Per the PSY-Q macro naming:
 *   gte_rtir       = MVMVA mx=R, v=IR, cv=none, sf=1, lm=0
 *   gte_rtir_b     = same, "_b" suffix indicates "no error abort"
 *                    (PSY-Q would otherwise read FLAG and trap; the
 *                    bit-exact result is identical, only the host
 *                    flag handling differs)
 *   gte_rtir_sf0_b = mx=R, v=IR, cv=none, sf=0, lm=0
 *   gte_rtirtr_b   = mx=R, v=IR, cv=TR, sf=1, lm=0
 *   gte_rtv0_b     = mx=R, v=V0, cv=TR, sf=1, lm=0
 */
void gte_rtir       (void) { mvmva(0, 3, 3, 1, 0); }
void gte_rtir_b     (void) { mvmva(0, 3, 3, 1, 0); }
void gte_rtir_sf0_b (void) { mvmva(0, 3, 3, 0, 0); }
void gte_rtirtr_b   (void) { mvmva(0, 3, 0, 1, 0); }
void gte_rtv0_b     (void) { mvmva(0, 0, 0, 1, 0); }

/* ============================================================
 * PSY-Q library functions (built on top of the GTE).  Each matches
 * the PSY-Q docs and the Ghidra refs of the corresponding FUN_*.
 * ============================================================ */

/* SetRotMatrix / ReadRotMatrix copy between an in-memory MATRIX and
 * the cop2 rotation control registers.  PSY-Q layout exactly. */
void SetRotMatrix(const MATRIX *m)
{
    const uint16_t *e = (const uint16_t *)m->m;
    /* m->m[0][0..2] = R11 R12 R13 (3 i16);  m->m[1][0..2] = R21 R22 R23 */
    g_ctl[RT_11_12] = (int32_t)((uint32_t)e[0] | ((uint32_t)e[1] << 16));
    g_ctl[RT_13_21] = (int32_t)((uint32_t)e[2] | ((uint32_t)e[3] << 16));
    g_ctl[RT_22_23] = (int32_t)((uint32_t)e[4] | ((uint32_t)e[5] << 16));
    g_ctl[RT_31_32] = (int32_t)((uint32_t)e[6] | ((uint32_t)e[7] << 16));
    g_ctl[RT_33]    = (int32_t)((uint32_t)e[8]);
}
void gte_SetRotMatrix(MATRIX *m) { SetRotMatrix(m); }

void ReadRotMatrix(MATRIX *m)
{
    uint16_t *e = (uint16_t *)m->m;
    e[0] = (uint16_t)((uint32_t)g_ctl[RT_11_12]);
    e[1] = (uint16_t)((uint32_t)g_ctl[RT_11_12] >> 16);
    e[2] = (uint16_t)((uint32_t)g_ctl[RT_13_21]);
    e[3] = (uint16_t)((uint32_t)g_ctl[RT_13_21] >> 16);
    e[4] = (uint16_t)((uint32_t)g_ctl[RT_22_23]);
    e[5] = (uint16_t)((uint32_t)g_ctl[RT_22_23] >> 16);
    e[6] = (uint16_t)((uint32_t)g_ctl[RT_31_32]);
    e[7] = (uint16_t)((uint32_t)g_ctl[RT_31_32] >> 16);
    e[8] = (uint16_t)((uint32_t)g_ctl[RT_33]);
}

void SetTransMatrix(const MATRIX *m) {
    g_ctl[TR_X] = m->t[0];
    g_ctl[TR_Y] = m->t[1];
    g_ctl[TR_Z] = m->t[2];
}

/* ApplyMatrix: V_out = M * V_in (q12 multiply, sf=1).
 * Uses MVMVA with v=V0, cv=none. */
void ApplyMatrix(const MATRIX *m, const SVECTOR *v0, VECTOR *v1) {
    SetRotMatrix(m);
    gte_ldv0(v0);
    mvmva(0, 0, 3, 1, 0);   /* mx=R, v=V0, cv=none, sf=1, lm=0 */
    v1->vx = g_data[MAC1];
    v1->vy = g_data[MAC2];
    v1->vz = g_data[MAC3];
}

void ApplyMatrixLV(const MATRIX *m, const VECTOR *v0, VECTOR *v1) {
    SetRotMatrix(m);
    /* Long-vector input goes through IR.  Saturate-to-i16 on load is
     * the PSY-Q behavior. */
    g_data[IR1_] = (int32_t)(int16_t)v0->vx;
    g_data[IR2_] = (int32_t)(int16_t)v0->vy;
    g_data[IR3_] = (int32_t)(int16_t)v0->vz;
    mvmva(0, 3, 3, 1, 0);
    v1->vx = g_data[MAC1];
    v1->vy = g_data[MAC2];
    v1->vz = g_data[MAC3];
}

/* RotMatrix: build a rotation matrix from a YXZ Euler-angle SVECTOR.
 * Order on PSX (PSY-Q convention): M = Rz * Ry * Rx applied to a column
 * vector, equivalent to R = Rx * Ry * Rz applied as a row-vector basis.
 * Each angle is q12 in [0, 4096).
 *
 * The hardware doesn't have a single "build rotation" op; PSY-Q
 * synthesizes it from rsin/rcos and matrix multiplies.  We mirror
 * that here: compute the 3 angle sines/cosines, fill the matrix
 * elements with the closed-form expressions, then i16-clamp.
 */
void RotMatrix(const SVECTOR *r, MATRIX *m)
{
    /* PSY-Q RotMatrix is the Y*X*Z order. */
    int cx = rcos(r->vx), sx = rsin(r->vx);
    int cy = rcos(r->vy), sy = rsin(r->vy);
    int cz = rcos(r->vz), sz = rsin(r->vz);

    /* All sines/cosines are q12. Products are q24; right-shift by 12
     * (PSY-Q matches PSX by truncating, not rounding) to fit each
     * matrix element back into q12.
     *
     * Combined Y*X rotation first. */
    int m11_yx = ( cy * 0x1000) >> 12;    /* row1*col1 of (Ry*Rx) -> cy */
    int m13_yx = ( sy * 0x1000) >> 12;    /* ... -> sy */
    int m21_yx = ( sx * sy)     >> 12;    /* sx*sy */
    int m22_yx = ( cx * 0x1000) >> 12;    /* cx */
    int m23_yx = (-sx * cy)     >> 12;    /* -sx*cy */
    int m31_yx = (-cx * sy)     >> 12;
    int m32_yx = ( sx * 0x1000) >> 12;
    int m33_yx = ( cx * cy)     >> 12;

    /* Then multiply by Rz on the left. */
    int M11 = ( cz * m11_yx +  sz * m21_yx) >> 12;
    int M12 = ( cz * 0      +  sz * m22_yx) >> 12;
    int M13 = ( cz * m13_yx +  sz * m23_yx) >> 12;
    int M21 = (-sz * m11_yx +  cz * m21_yx) >> 12;
    int M22 = (-sz * 0      +  cz * m22_yx) >> 12;
    int M23 = (-sz * m13_yx +  cz * m23_yx) >> 12;
    int M31 = m31_yx;
    int M32 = m32_yx;
    int M33 = m33_yx;

    m->m[0][0] = (int16_t)M11; m->m[0][1] = (int16_t)M12; m->m[0][2] = (int16_t)M13;
    m->m[1][0] = (int16_t)M21; m->m[1][1] = (int16_t)M22; m->m[1][2] = (int16_t)M23;
    m->m[2][0] = (int16_t)M31; m->m[2][1] = (int16_t)M32; m->m[2][2] = (int16_t)M33;
}

/* RotMatrixYXZ_gte: load Y, X, Z Euler angles in degrees-q12 from
 * an SVECTOR-laid-out source, produce a YXZ rotation matrix into m.
 * Mirrors PSY-Q's "rotation about Y first, then X, then Z" convention. */
void RotMatrixYXZ_gte(const SVECTOR *r, MATRIX *m) { RotMatrix(r, m); }

/* MulMatrix0: m_out = m0 * m1.  We do this in C (no GTE), since
 * PSY-Q's MulMatrix uses CompMatrix which is itself a multi-MVMVA. */
void MulMatrix0(const MATRIX *a, const MATRIX *b, MATRIX *out) {
    int16_t r[3][3];
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        int64_t s = 0;
        for (int k = 0; k < 3; k++) s += (int64_t)a->m[i][k] * b->m[k][j];
        s >>= 12;
        if (s >  0x7fff) s =  0x7fff;
        if (s < -0x8000) s = -0x8000;
        r[i][j] = (int16_t)s;
    }
    memcpy(out->m, r, sizeof r);
    out->t[0] = a->t[0]; out->t[1] = a->t[1]; out->t[2] = a->t[2];
}
void MulMatrix(const MATRIX *a, MATRIX *b) { MulMatrix0(a, b, b); }

long CompMatrix(const MATRIX *m0, const MATRIX *m1, MATRIX *m2) {
    MulMatrix0(m0, m1, m2);
    return 0;
}
long CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2) {
    return CompMatrix(m0, m1, m2);
}

/* MatrixNormal: re-orthogonalise via Gram-Schmidt with q12 normalization.
 * PSY-Q computes each row's length squared, takes sqrt via SqrtRR, then
 * divides each component by length/0x1000.  We do equivalent integer
 * math with a binary-search sqrt to stay deterministic. */
static uint32_t isqrt32(uint64_t n) {
    if (n == 0) return 0;
    uint64_t x = n, y = (x + 1) >> 1;
    while (y < x) { x = y; y = (x + n / x) >> 1; }
    return (uint32_t)x;
}
void MatrixNormal(const MATRIX *m_in, MATRIX *m_out)
{
    if (m_in != m_out) *m_out = *m_in;
    for (int r = 0; r < 3; r++) {
        int64_t a = m_out->m[r][0];
        int64_t b = m_out->m[r][1];
        int64_t c = m_out->m[r][2];
        uint64_t mag2 = (uint64_t)(a*a + b*b + c*c);
        if (mag2 == 0) {
            m_out->m[r][0] = (r == 0) ? 0x1000 : 0;
            m_out->m[r][1] = (r == 1) ? 0x1000 : 0;
            m_out->m[r][2] = (r == 2) ? 0x1000 : 0;
            continue;
        }
        uint32_t mag = isqrt32(mag2);
        if (mag == 0) continue;
        /* Scale each component so that the row length becomes 0x1000. */
        int64_t na = (a * 0x1000) / mag;
        int64_t nb = (b * 0x1000) / mag;
        int64_t nc = (c * 0x1000) / mag;
        if (na >  0x7fff) na =  0x7fff;  if (na < -0x8000) na = -0x8000;
        if (nb >  0x7fff) nb =  0x7fff;  if (nb < -0x8000) nb = -0x8000;
        if (nc >  0x7fff) nc =  0x7fff;  if (nc < -0x8000) nc = -0x8000;
        m_out->m[r][0] = (int16_t)na;
        m_out->m[r][1] = (int16_t)nb;
        m_out->m[r][2] = (int16_t)nc;
    }
}

/* ============================================================
 * PSY-Q trig.  rsin/rcos use the 4096-entry interleaved (sin,cos)
 * table that PSY-Q embedded in the binary at 0x800607b4. The table
 * formula is `round(sin/cos(2*pi*i/4096) * 4096)` -- we generate it
 * at compile time via tools/gen_sintbl.py (no copyrighted bytes from
 * the user's EXE are bundled). The generated table has been verified
 * byte-for-byte against the EXE.
 *
 * Note: the cleaned engine code references the table as DAT_80060db4,
 * which is +0x600 into the base; that's just where the highest cross-
 * reference landed in Ghidra, not the table start.
 * ============================================================ */
extern const int16_t g_v8_sincostbl[8192];   /* platform/psyq/sintbl.c */

int rsin(int a) { return g_v8_sincostbl[((a) & 0xfff) * 2 + 0]; }
int rcos(int a) { return g_v8_sincostbl[((a) & 0xfff) * 2 + 1]; }

/* PSY-Q ratan2: q12 angle in [0, 4096) corresponding to atan2(y, x).
 * Pass-3 audit target: PSY-Q's ratan2 uses a piecewise polynomial +
 * the same sin LUT; the host's atan2 will differ by up to a few LSB.
 * Not exercised on the per-tick physics path -- safe to leave float. */
#include <math.h>
int ratan2(int y, int x) {
    if (x == 0 && y == 0) return 0;
    double rad = atan2((double)y, (double)x);
    double turns = rad * (1.0 / (2.0 * 3.14159265358979323846));
    if (turns < 0) turns += 1.0;
    int a = (int)(turns * 4096.0);
    return a & 0xfff;
}

/* ============================================================
 * Misc helpers (less hot paths; minimal but correct).
 * ============================================================ */

void VectorNormalSS(const VECTOR *v0, VECTOR *v1) {
    uint64_t mag2 = (uint64_t)((int64_t)v0->vx * v0->vx
                             + (int64_t)v0->vy * v0->vy
                             + (int64_t)v0->vz * v0->vz);
    uint32_t mag = isqrt32(mag2);
    if (mag == 0) { v1->vx = v1->vy = v1->vz = 0; return; }
    v1->vx = (int32_t)(((int64_t)v0->vx * 0x1000) / mag);
    v1->vy = (int32_t)(((int64_t)v0->vy * 0x1000) / mag);
    v1->vz = (int32_t)(((int64_t)v0->vz * 0x1000) / mag);
}

long RotMatrix_ret(const SVECTOR *r, MATRIX *m) { RotMatrix(r, m); return 0; }
long TransMatrix_ret(const MATRIX *m, const VECTOR *t) { (void)m; (void)t; return 0; }
long ScaleMatrix(MATRIX *m, const VECTOR *v) {
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        int64_t s = (int64_t)m->m[i][j] * ((const int32_t *)v)[j];
        s >>= 12;
        if (s >  0x7fff) s =  0x7fff;
        if (s < -0x8000) s = -0x8000;
        m->m[i][j] = (int16_t)s;
    }
    return 0;
}

MATRIX *TransMatrix(MATRIX *m, const VECTOR *t) {
    m->t[0] = t->vx; m->t[1] = t->vy; m->t[2] = t->vz;
    return m;
}

void InitGeom(void) {
    memset(g_data, 0, sizeof g_data);
    memset(g_ctl,  0, sizeof g_ctl);
}
void SetGeomOffset(int ofx, int ofy) {
    g_ctl[OFX] = (int32_t)ofx << 16;
    g_ctl[OFY] = (int32_t)ofy << 16;
}
void SetGeomScreen(int h) { g_ctl[H_DIST] = (int32_t)(uint16_t)h; }
