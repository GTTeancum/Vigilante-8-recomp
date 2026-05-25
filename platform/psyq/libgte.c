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
    int32_t src[3] = {
        vec_elem(v, 0),
        vec_elem(v, 1),
        vec_elem(v, 2)
    };

    /* Clear FLAG before op (FLAG is sticky; nocash says it's clobbered
     * per-op). Real PSX clears all sticky flags at the start of each
     * GTE op. */
    g_ctl[FLAG] = 0;

    for (int r = 0; r < 3; r++) {
        int64_t sum = (int64_t)cv_elem(cv, r) << 12;
        for (int c = 0; c < 3; c++) {
            int64_t prod = (int64_t)mtx_elem(mx, r, c) * (int64_t)src[c];
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

/* MAC1/2/3 readbacks -- the i32 accumulators behind the IR registers.
 * Used in long-vector reconstruction paths (e.g. FUN_80043408 splits a
 * q15.16 vector into hi/lo, runs two MVMVAs, and recombines via these). */
int32_t gte_stMAC1(void) { return g_data[MAC1]; }
int32_t gte_stMAC2(void) { return g_data[MAC2]; }
int32_t gte_stMAC3(void) { return g_data[MAC3]; }

/* Load the packed VX0/VY0 i16 pair (from memory pointed by ptr) plus
 * VZ0 separately.  These match PSY-Q's gte_ldvxy0/gte_ldvz0 macros. */
void gte_ldVXY0(const void *ptr) {
    uint32_t v = *(const uint32_t *)ptr;
    g_data[V0_XY] = (int32_t)v;
}
void gte_ldVZ0(const void *ptr) {
    /* PSY-Q's macro loads u32 then sign-extends low i16 to i32. */
    uint32_t v = *(const uint32_t *)ptr;
    g_data[V0_Z]  = (int32_t)(int16_t)v;
}

/* Store the long-vector form of IR (i.e. the MAC1/2/3 32-bit accumulator). */
void gte_stlvnl(VECTOR *out) {
    out->vx = g_data[MAC1];
    out->vy = g_data[MAC2];
    out->vz = g_data[MAC3];
}

/* LZCS/LZCR: leading bit-count.  Writing to LZCS triggers a count of
 * leading bits that MATCH the sign bit (= leading zeros for v >= 0,
 * leading ones for v < 0).  LZCR holds the count, range [0, 32].
 * Used by FUN_80016a20's variable-shift sqrt of dot(v, v). */
void gte_ldLZCS(int32_t v) {
    g_data[LZCS] = v;
    uint32_t u = (v < 0) ? (~(uint32_t)v) : (uint32_t)v;
    int n = 0;
    while (n < 32 && (u & 0x80000000u) == 0) { n++; u <<= 1; }
    g_data[LZCR] = n;
}
int32_t gte_stLZCR(void) { return g_data[LZCR]; }

static const int16_t s_sqrt_tbl[192] = {
    4096, 4127, 4159, 4190, 4222, 4252, 4283, 4314,
    4344, 4374, 4404, 4434, 4463, 4492, 4521, 4550,
    4579, 4608, 4636, 4664, 4692, 4720, 4748, 4775,
    4802, 4830, 4857, 4884, 4910, 4937, 4964, 4990,
    5016, 5042, 5068, 5094, 5120, 5145, 5170, 5196,
    5221, 5246, 5271, 5296, 5320, 5345, 5369, 5394,
    5418, 5442, 5466, 5490, 5514, 5538, 5561, 5585,
    5608, 5632, 5655, 5678, 5701, 5724, 5747, 5769,
    5792, 5815, 5837, 5860, 5882, 5904, 5926, 5948,
    5970, 5992, 6014, 6036, 6058, 6079, 6101, 6122,
    6144, 6165, 6186, 6207, 6228, 6249, 6270, 6291,
    6312, 6333, 6353, 6374, 6394, 6415, 6435, 6456,
    6476, 6496, 6516, 6536, 6556, 6576, 6596, 6616,
    6636, 6656, 6675, 6695, 6714, 6734, 6753, 6773,
    6792, 6811, 6830, 6850, 6869, 6888, 6907, 6926,
    6945, 6963, 6982, 7001, 7020, 7038, 7057, 7075,
    7094, 7112, 7131, 7149, 7168, 7186, 7204, 7222,
    7240, 7258, 7276, 7294, 7312, 7330, 7348, 7366,
    7384, 7401, 7419, 7437, 7454, 7472, 7489, 7507,
    7524, 7542, 7559, 7576, 7594, 7611, 7628, 7645,
    7662, 7680, 7697, 7714, 7731, 7747, 7764, 7781,
    7798, 7815, 7832, 7848, 7865, 7882, 7898, 7915,
    7931, 7948, 7964, 7981, 7997, 8014, 8030, 8046,
    8062, 8079, 8095, 8111, 8127, 8143, 8159, 8175,
};

/* HIGH: direct port of SLUS_005.10 SquareRoot0 at 0x8004c6e4. */
long SquareRoot0(long n) {
    int32_t a = (int32_t)n;
    gte_ldLZCS(a);
    int32_t lz = gte_stLZCR();
    if (lz == 0x20)
        return 0;

    lz &= ~1;
    int32_t norm;
    if ((int32_t)(lz - 0x18) < 0)
        norm = a >> ((0x18 - lz) & 31);
    else
        norm = (int32_t)((uint32_t)a << ((lz - 0x18) & 31));

    int32_t idx = norm - 0x40;
    if (idx < 0 || idx >= 192)
        return 0;

    int32_t shift = ((0x1f - lz) >> 1) & 31;
    uint32_t scaled = (uint32_t)((int32_t)s_sqrt_tbl[idx] << shift);
    return (long)(scaled >> 12);
}

/* MVMVA convenience forms.  Per the PSY-Q macro naming:
 *   gte_rtir       = MVMVA mx=R, v=IR, cv=none, sf=1, lm=0
 *   gte_rtir_b     = same, "_b" suffix indicates "no error abort"
 *                    (PSY-Q would otherwise read FLAG and trap; the
 *                    bit-exact result is identical, only the host
 *                    flag handling differs)
 *   gte_rtir_sf0_b = mx=R, v=IR, cv=none, sf=0, lm=0
 *   gte_rtirtr_b   = mx=R, v=IR, cv=TR, sf=1, lm=0
 *   gte_rtv0_b     = mx=R, v=V0, cv=none, sf=1, lm=0
 */
void gte_rtir       (void) { mvmva(0, 3, 3, 1, 0); }
void gte_rtir_b     (void) { mvmva(0, 3, 3, 1, 0); }
void gte_rtir_sf0_b (void) { mvmva(0, 3, 3, 0, 0); }
void gte_rtir_sf0   (void) { mvmva(0, 3, 3, 0, 0); }   /* alias: RTIR_SF0 */
void gte_rtirtr_b   (void) { mvmva(0, 3, 0, 1, 0); }
void gte_rtirtr     (void) { mvmva(0, 3, 0, 1, 0); }   /* alias: RTIRTR */
void gte_rtv0_b     (void) { mvmva(0, 0, 3, 1, 0); }
void gte_rtv0       (void) { mvmva(0, 0, 3, 1, 0); }   /* RTV0: V0 by R, no translation */

/* GTE OP: outer product (cross-product) of two 3-vectors.  Per nocash:
 *
 *   MAC1 = (IR3*RT22 - IR2*RT33)
 *   MAC2 = (IR1*RT33 - IR3*RT11)
 *   MAC3 = (IR2*RT11 - IR1*RT22)
 *   IR[1..3] = saturated MAC[1..3] >> (sf*12)
 *
 * The "matrix" is interpreted as a diagonal vector D = (RT11, RT22, RT33).
 * To compute cross(A, B), the caller loads A = (D1, D2, D3) into the
 * matrix-diagonal slots (via gte_ldR11R12/22R23/33 with the off-diagonals
 * left at zero) and B into IR1/IR2/IR3, then issues OP.
 *
 * lm=0 (signed sat), the only mode the engine uses.
 */
static void gte_OP_impl(int sf)
{
    int32_t d1 = (int16_t)g_ctl[RT_11_12];   /* RT11 (low half) */
    int32_t d2 = (int16_t)(g_ctl[RT_22_23]); /* RT22 (low half) */
    int32_t d3 = (int16_t)g_ctl[RT_33];      /* RT33 (low half) */
    int32_t a  = g_data[IR1_];
    int32_t b  = g_data[IR2_];
    int32_t c  = g_data[IR3_];

    g_ctl[FLAG] = 0;
    int64_t m1 = (int64_t)c * d2 - (int64_t)b * d3;
    int64_t m2 = (int64_t)a * d3 - (int64_t)c * d1;
    int64_t m3 = (int64_t)b * d1 - (int64_t)a * d2;
    int shift = sf ? 12 : 0;
    int32_t mac1 = (int32_t)(m1 >> shift);
    int32_t mac2 = (int32_t)(m2 >> shift);
    int32_t mac3 = (int32_t)(m3 >> shift);
    g_data[MAC1] = mac1;
    g_data[MAC2] = mac2;
    g_data[MAC3] = mac3;
    g_data[IR1_] = (mac1 < -0x8000) ? -0x8000 : (mac1 > 0x7fff) ? 0x7fff : mac1;
    g_data[IR2_] = (mac2 < -0x8000) ? -0x8000 : (mac2 > 0x7fff) ? 0x7fff : mac2;
    g_data[IR3_] = (mac3 < -0x8000) ? -0x8000 : (mac3 > 0x7fff) ? 0x7fff : mac3;
}
void gte_OP(int sf) { gte_OP_impl(sf); }
void gte_op12(void) { gte_OP_impl(1); }      /* PSY-Q macro alias used in
                                                FUN_8001787c MIPS dump */
void gte_op12_b(void) { gte_OP_impl(1); }    /* "no error abort" alias */

/* GTE SQR: square each IR component into its MAC.
 *   MAC[1-3] = IR[1-3] * IR[1-3]
 *   MAC[r] >>= (sf * 12)
 *   IR[r]   = sat_i16(MAC[r])
 * Per nocash PSX-SPX § "OP/SQR/...".
 */
static void gte_SQR_impl(int sf)
{
    g_ctl[FLAG] = 0;
    int shift = sf ? 12 : 0;
    static const uint32_t MP[3] = { FLAG_MAC1_POS, FLAG_MAC2_POS, FLAG_MAC3_POS };
    static const uint32_t MN[3] = { FLAG_MAC1_NEG, FLAG_MAC2_NEG, FLAG_MAC3_NEG };
    static const uint32_t IS[3] = { FLAG_IR1_SAT,  FLAG_IR2_SAT,  FLAG_IR3_SAT  };
    for (int r = 0; r < 3; r++) {
        int64_t v = (int64_t)g_data[IR1_+r] * g_data[IR1_+r];
        v = mac_overflow_check(v, MP[r], MN[r]);
        if (shift) v >>= shift;
        int32_t mac = mac_clip32(v);
        g_data[MAC1+r] = mac;
        g_data[IR1_+r] = ir_saturate(mac, 0, IS[r]);
    }
    flag_update_summary();
}
void gte_SQR(int sf) { gte_SQR_impl(sf); }

/* GTE GPF: general polynomial function.
 *   MAC[r] = IR0 * IR[r]
 *   MAC[r] >>= (sf * 12)
 *   IR[r]   = sat_i16(MAC[r])
 * Per nocash PSX-SPX.
 */
static void gte_GPF_impl(int sf)
{
    g_ctl[FLAG] = 0;
    int shift = sf ? 12 : 0;
    int32_t ir0 = g_data[IR0];
    static const uint32_t MP[3] = { FLAG_MAC1_POS, FLAG_MAC2_POS, FLAG_MAC3_POS };
    static const uint32_t MN[3] = { FLAG_MAC1_NEG, FLAG_MAC2_NEG, FLAG_MAC3_NEG };
    static const uint32_t IS[3] = { FLAG_IR1_SAT,  FLAG_IR2_SAT,  FLAG_IR3_SAT  };
    for (int r = 0; r < 3; r++) {
        int64_t v = (int64_t)ir0 * g_data[IR1_+r];
        v = mac_overflow_check(v, MP[r], MN[r]);
        if (shift) v >>= shift;
        int32_t mac = mac_clip32(v);
        g_data[MAC1+r] = mac;
        g_data[IR1_+r] = ir_saturate(mac, 0, IS[r]);
    }
    flag_update_summary();
}
void gte_GPF(int sf) { gte_GPF_impl(sf); }

/* IR0 load/store. */
void gte_ldIR0(int v) { g_data[IR0] = (int32_t)(int16_t)v; }

/* Rotation-matrix register readbacks (used by MatrixNormal). */
uint32_t gte_stR11R12(void) { return (uint32_t)g_ctl[RT_11_12]; }
uint32_t gte_stR22R23(void) { return (uint32_t)g_ctl[RT_22_23]; }
uint32_t gte_stR33   (void) { return (uint32_t)g_ctl[RT_33];    }

/* V0/V1 short-vector register load/store (Ghidra ldVXY0/VZ0/VXY1 macros).
 * VXY0: packed (vx,vy) in a single 32-bit word, each as i16.
 * VZ0 : vz as i16 in low 16 bits.
 * VXY1: packed (vx,vy) of V1 (same format). */
void gte_ldVXY1(int v)
{
    g_data[V1_XY] = (int32_t)v;     /* raw 32-bit load, same as VXY0 */
}
/* Integer-argument variants for use when the MIPS passes a register
 * value directly (mtc2 <reg>, C2_VXY0 / C2_VZ0).  Used by MatrixNormal
 * (FUN_8004c934) to stash row1 components across GTE OP12 calls. */
void gte_ldVXY0_i(int v) { g_data[V0_XY] = (int32_t)v; }
void gte_ldVZ0_i (int v) { g_data[V0_Z]  = (int32_t)(int16_t)v; }

/* Store V0/V1 back (Ghidra stVXY0/VZ0/VXY1 macros - read from IR regs after
 * RTIR, which writes IR1/IR2 → these macros actually read back MAC/IR). */
int32_t gte_stVXY0(void) { return (int32_t)((uint16_t)g_data[IR1_] | ((uint32_t)(uint16_t)g_data[IR2_] << 16)); }
int32_t gte_stVZ0 (void) { return (int32_t)(uint16_t)g_data[IR3_]; }
int32_t gte_stVXY1(void) { return gte_stVXY0(); }   /* uses current IR like VXY0 */

/* V0/V1 readback variants that read the actual V0/V1 data registers.
 * Used by FUN_8004c934 to retrieve scratch-stashed row1 components after
 * a FUN_8004c874 call (mfc2 <reg>, C2_VXY0 / C2_VZ0 / C2_VXY1). */
int32_t gte_stVXY0_V0(void) { return g_data[V0_XY]; }
int32_t gte_stVZ0_V0 (void) { return g_data[V0_Z];  }
int32_t gte_stVXY1_V1(void) { return g_data[V1_XY]; }

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
VECTOR *ApplyMatrix(const MATRIX *m, const SVECTOR *v0, VECTOR *v1) {
    SetRotMatrix(m);
    gte_ldv0(v0);
    mvmva(0, 0, 3, 1, 0);   /* mx=R, v=V0, cv=none, sf=1, lm=0 */
    v1->vx = g_data[MAC1];
    v1->vy = g_data[MAC2];
    v1->vz = g_data[MAC3];
    return v1;
}

/* ApplyMatrixSV: V_out (SVECTOR) = M * V_in (SVECTOR), q12.  Identical
 * to ApplyMatrix for the GTE math, but the result is stored as i16
 * (truncating MAC1/2/3 to short) via the IR registers.  PSY-Q symbol
 * at SLUS:0x8004d2b4. */
SVECTOR *ApplyMatrixSV(const MATRIX *m, const SVECTOR *v0, SVECTOR *v1) {
    SetRotMatrix(m);
    gte_ldv0(v0);
    mvmva(0, 0, 3, 1, 0);
    v1->vx = (int16_t)gte_stIR1();
    v1->vy = (int16_t)gte_stIR2();
    v1->vz = (int16_t)gte_stIR3();
    return v1;
}

VECTOR *ApplyMatrixLV(const MATRIX *m, const VECTOR *v0, VECTOR *v1) {
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
    return v1;
}

/* HIGH: direct port of SLUS_005.10 RotMatrixYXZ_gte at 0x8004dab4.
 * The source builds the matrix through staged GTE GPF12 operations; the
 * intermediate IR saturation is observable and is therefore preserved. */
MATRIX *RotMatrixYXZ_gte(const SVECTOR *r, MATRIX *m)
{
    int32_t sx = rsin(r->vx);
    int32_t cx = rcos(r->vx);
    int32_t sy = rsin(r->vy);
    int32_t cy = rcos(r->vy);
    int32_t sz = rsin(r->vz);
    int32_t cz = rcos(r->vz);
    int32_t cy_sx, cy_sz, cy_cz;
    int32_t sy_sx, sy_sz, sy_cz;
    int32_t cz_cx, cz_sy_sx, cz_cy_sx;
    int32_t sz_cx, sz_sy_sx, sz_cy_sx;

    gte_ldIR0(cy);
    gte_ldIR1(sx);
    gte_ldIR2(sz);
    gte_ldIR3(cz);
    gte_GPF(1);
    cy_sx = gte_stIR1();
    cy_sz = gte_stIR2();
    cy_cz = gte_stIR3();

    gte_ldIR0(sy);
    gte_ldIR1(sx);
    gte_ldIR2(sz);
    gte_ldIR3(cz);
    gte_GPF(1);
    sy_sx = gte_stIR1();
    sy_sz = gte_stIR2();
    sy_cz = gte_stIR3();

    m->m[2][2] = (int16_t)((cx * cy) >> 12);

    gte_ldIR0(cz);
    gte_ldIR1(cx);
    gte_ldIR2(sy_sx);
    gte_ldIR3(cy_sx);
    gte_GPF(1);
    cz_cx = gte_stIR1();
    cz_sy_sx = gte_stIR2();
    cz_cy_sx = gte_stIR3();

    gte_ldIR0(sz);
    gte_ldIR1(cx);
    gte_ldIR2(sy_sx);
    gte_ldIR3(cy_sx);
    gte_GPF(1);
    sz_cx = gte_stIR1();
    sz_sy_sx = gte_stIR2();
    sz_cy_sx = gte_stIR3();

    m->m[0][0] = (int16_t)(cy_cz + sz_sy_sx);
    m->m[0][1] = (int16_t)(cz_sy_sx - cy_sz);
    m->m[0][2] = (int16_t)((cx * sy) >> 12);
    m->m[1][0] = (int16_t)sz_cx;
    m->m[1][1] = (int16_t)cz_cx;
    m->m[1][2] = (int16_t)-sx;
    m->m[2][0] = (int16_t)(sz_cy_sx - sy_cz);
    m->m[2][1] = (int16_t)(cz_cy_sx + sy_sz);
    return m;
}

void RotMatrix(const SVECTOR *r, MATRIX *m)
{
    (void)RotMatrixYXZ_gte(r, m);
}

static void load_rot_regs(const MATRIX *m)
{
    const uint32_t *w = (const uint32_t *)m;
    gte_ldR11R12(w[0]);
    gte_ldR13R21(w[1]);
    gte_ldR22R23(w[2]);
    gte_ldR31R32(w[3]);
    gte_ldR33(w[4]);
}

static void load_matrix_column_v0(const MATRIX *m, int col)
{
    uint32_t xy;

    if (col == 0) {
        xy = (uint16_t)m->m[0][0] | ((uint32_t)(uint16_t)m->m[1][0] << 16);
        gte_ldVXY0_i((int32_t)xy);
        gte_ldVZ0_i(m->m[2][0]);
    } else if (col == 1) {
        xy = (uint16_t)m->m[0][1] | ((uint32_t)(uint16_t)m->m[1][1] << 16);
        gte_ldVXY0_i((int32_t)xy);
        gte_ldVZ0_i(m->m[2][1]);
    } else {
        xy = (uint16_t)m->m[0][2] | ((uint32_t)(uint16_t)m->m[1][2] << 16);
        gte_ldVXY0_i((int32_t)xy);
        gte_ldVZ0_i(m->m[2][2]);
    }
}

/* HIGH: direct port of SLUS_005.10 MulMatrix0 at 0x8004ccb4.
 * The source rotates each column of m1 through the GTE and stores only
 * the 3x3 rotation block; translation words are intentionally untouched. */
MATRIX *MulMatrix0(const MATRIX *m0, const MATRIX *m1, MATRIX *m2)
{
    int32_t c0x, c0y, c0z;
    int32_t c1x, c1y, c1z;

    load_rot_regs(m0);

    load_matrix_column_v0(m1, 0);
    gte_rtv0_b();
    c0x = gte_stIR1();
    c0y = gte_stIR2();
    c0z = gte_stIR3();

    load_matrix_column_v0(m1, 1);
    gte_rtv0_b();
    c1x = gte_stIR1();
    c1y = gte_stIR2();
    c1z = gte_stIR3();

    load_matrix_column_v0(m1, 2);
    gte_rtv0_b();

    m2->m[0][0] = (int16_t)c0x;
    m2->m[1][0] = (int16_t)c0y;
    m2->m[2][0] = (int16_t)c0z;
    m2->m[0][1] = (int16_t)c1x;
    m2->m[1][1] = (int16_t)c1y;
    m2->m[2][1] = (int16_t)c1z;
    m2->m[0][2] = (int16_t)gte_stIR1();
    m2->m[1][2] = (int16_t)gte_stIR2();
    m2->m[2][2] = (int16_t)gte_stIR3();
    return m2;
}

void MulMatrix(const MATRIX *a, MATRIX *b) { (void)MulMatrix0(a, b, b); }

static void split_compmatrix_component(int32_t v, int32_t *hi, int32_t *lo)
{
    if (v < 0) {
        uint32_t mag = (uint32_t)-v;
        *hi = -(int32_t)(mag >> 15);
        *lo = -(int32_t)(mag & 0x7fff);
    } else {
        *hi = v >> 15;
        *lo = v & 0x7fff;
    }
}

/* HIGH: direct port of SLUS_005.10 CompMatrixLV at 0x8004cf04. */
MATRIX *CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2)
{
    int32_t hi[3], lo[3];
    int32_t up[3], low[3];

    (void)MulMatrix0(m0, m1, m2);

    split_compmatrix_component(m1->t[0], &hi[0], &lo[0]);
    split_compmatrix_component(m1->t[1], &hi[1], &lo[1]);
    split_compmatrix_component(m1->t[2], &hi[2], &lo[2]);

    gte_ldsv_(hi[0], hi[1], hi[2]);
    gte_rtir_sf0_b();
    up[0] = gte_stMAC1();
    up[1] = gte_stMAC2();
    up[2] = gte_stMAC3();

    gte_ldsv_(lo[0], lo[1], lo[2]);
    gte_rtir_b();
    low[0] = gte_stMAC1();
    low[1] = gte_stMAC2();
    low[2] = gte_stMAC3();

    m2->t[0] = low[0] + up[0] * 8 + m0->t[0];
    m2->t[1] = low[1] + up[1] * 8 + m0->t[1];
    m2->t[2] = low[2] + up[2] * 8 + m0->t[2];
    return m2;
}

MATRIX *CompMatrix(const MATRIX *m0, const MATRIX *m1, MATRIX *m2) {
    return CompMatrixLV(m0, m1, m2);
}

extern void FUN_8004c934(MATRIX *m, MATRIX *n);

void MatrixNormal(const MATRIX *m_in, MATRIX *m_out)
{
    FUN_8004c934((MATRIX *)m_in, m_out);
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
extern int16_t DAT_800647b4[1025];

int rsin(int a) { return g_v8_sincostbl[((a) & 0xfff) * 2 + 0]; }
int rcos(int a) { return g_v8_sincostbl[((a) & 0xfff) * 2 + 1]; }


static int32_t neg32_wrap(int32_t v)
{
    return (int32_t)(0u - (uint32_t)v);
}

/* HIGH: direct port of SLUS_005.10 ratan2 at 0x8004ecd4.
 * Returns signed PSY-Q angle units; callers commonly truncate to int16_t. */
int ratan2(int y, int x)
{
    int xNeg = 0;
    int yNeg = 0;
    int32_t ax = x;
    int32_t ay = y;
    int32_t angle = 0;

    if (ax < 0) {
        xNeg = 1;
        ax = neg32_wrap(ax);
    }
    if (ay < 0) {
        yNeg = 1;
        ay = neg32_wrap(ay);
    }

    if (ax != 0 || ay != 0) {
        int32_t ratio;
        if (ay < ax) {
            if (((uint32_t)ay & 0x7fe00000u) != 0) {
                ratio = ay / (ax >> 10);
            } else {
                ratio = (int32_t)((uint32_t)ay << 10) / ax;
            }
            angle = DAT_800647b4[ratio];
        } else {
            if (((uint32_t)ax & 0x7fe00000u) != 0) {
                ratio = ax / (ay >> 10);
            } else {
                ratio = (int32_t)((uint32_t)ax << 10) / ay;
            }
            angle = 0x400 - DAT_800647b4[ratio];
        }
        if (xNeg)
            angle = 0x800 - angle;
        if (yNeg)
            angle = -angle;
    }
    return angle;
}

/* ============================================================
 * Misc helpers (less hot paths; minimal but correct).
 * ============================================================ */

static const int16_t s_vnormal_tbl[192] = {
    4096, 4064, 4033, 4003, 3973, 3944, 3916, 3888,
    3861, 3835, 3809, 3783, 3758, 3734, 3710, 3686,
    3663, 3640, 3618, 3596, 3575, 3554, 3533, 3513,
    3493, 3473, 3454, 3435, 3416, 3397, 3379, 3361,
    3344, 3327, 3310, 3293, 3276, 3260, 3244, 3228,
    3213, 3197, 3182, 3167, 3153, 3138, 3124, 3110,
    3096, 3082, 3069, 3055, 3042, 3029, 3016, 3003,
    2991, 2978, 2966, 2954, 2942, 2930, 2919, 2907,
    2896, 2885, 2873, 2862, 2852, 2841, 2830, 2820,
    2809, 2799, 2789, 2779, 2769, 2759, 2749, 2740,
    2730, 2721, 2711, 2702, 2693, 2684, 2675, 2666,
    2657, 2649, 2640, 2631, 2623, 2615, 2606, 2598,
    2590, 2582, 2574, 2566, 2558, 2550, 2543, 2535,
    2528, 2520, 2513, 2505, 2498, 2491, 2484, 2477,
    2469, 2462, 2456, 2449, 2442, 2435, 2428, 2422,
    2415, 2409, 2402, 2396, 2389, 2383, 2377, 2371,
    2364, 2358, 2352, 2346, 2340, 2334, 2328, 2322,
    2317, 2311, 2305, 2299, 2294, 2288, 2283, 2277,
    2272, 2266, 2261, 2255, 2250, 2245, 2239, 2234,
    2229, 2224, 2219, 2214, 2209, 2204, 2199, 2194,
    2189, 2184, 2179, 2174, 2170, 2165, 2160, 2155,
    2151, 2146, 2142, 2137, 2133, 2128, 2124, 2119,
    2115, 2110, 2106, 2102, 2097, 2093, 2089, 2084,
    2080, 2076, 2072, 2068, 2064, 2060, 2056, 2052,
};

static int32_t host_lzcr32(int32_t v)
{
    uint32_t u = (v < 0) ? ~(uint32_t)v : (uint32_t)v;
    int n = 0;
    while (n < 32 && (u & 0x80000000u) == 0) {
        n++;
        u <<= 1;
    }
    return n;
}

extern void FUN_8004c874(int x, int y, int z, int *ox, int *oy, int *oz);

/* HIGH: direct wrapper for SLUS_005.10 VectorNormalSS at 0x8004c844. */
long VectorNormalSS(const SVECTOR *v0, SVECTOR *v1) {
    int32_t x = (int16_t)v0->vx;
    int32_t y = (int16_t)v0->vy;
    int32_t z = (int16_t)v0->vz;
    int ox, oy, oz;

    FUN_8004c874(x, y, z, &ox, &oy, &oz);
    v1->vx = (int16_t)ox;
    v1->vy = (int16_t)oy;
    v1->vz = (int16_t)oz;
    return (long)(int32_t)((uint32_t)(x * x + y * y) + (uint32_t)(z * z));
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
