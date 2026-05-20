/* libgte_soft.c -- software PSX GTE implementation.
 *
 * Implements the GTE register set + the small subset of GTE ops the
 * cleaned engine code actually invokes (matrix * vector with q12
 * scaling, IR load/store, ratan2, RotMatrix). Not PSX-bit-exact yet
 * (no overflow flags, no specific BIOS rounding); the goal here is
 * correct math so the cleaned physics actually produces motion.
 *
 * Audit promotion path:
 *   1. Get gameplay working with this software GTE.
 *   2. Run side-by-side with a reference PSX GTE (PCSX-Redux) on
 *      recorded input streams to identify divergence.
 *   3. Tighten saturation/rounding to match.
 */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "psyq_stubs.h"
#include "gte.h"

/* ---- GTE register file. Only what we use. ---- */
static int16_t g_R[3][3];   /* rotation matrix in cop2 reg B */
static int32_t g_IR[3];     /* IR1, IR2, IR3 -- intermediate result vector */

static int32_t clamp_i16(int64_t v) {
    if (v >  0x7fff) return  0x7fff;
    if (v < -0x8000) return -0x8000;
    return (int32_t)v;
}

/* Load a packed { Rij_lo | Rkl_hi } u32 into two adjacent matrix slots.
 * The PSX GTE has these specific load instructions because rotation
 * matrices ship as 5 packed i32s (10 i16s, last is pad).
 *
 * In each pair, LOW i16 is the LOWER-numbered element. */
void gte_ldR11R12(uint32_t v) { g_R[0][0] = (int16_t)(v & 0xffff); g_R[0][1] = (int16_t)(v >> 16); }
void gte_ldR13R21(uint32_t v) { g_R[0][2] = (int16_t)(v & 0xffff); g_R[1][0] = (int16_t)(v >> 16); }
void gte_ldR22R23(uint32_t v) { g_R[1][1] = (int16_t)(v & 0xffff); g_R[1][2] = (int16_t)(v >> 16); }
void gte_ldR31R32(uint32_t v) { g_R[2][0] = (int16_t)(v & 0xffff); g_R[2][1] = (int16_t)(v >> 16); }
void gte_ldR33   (uint32_t v) { g_R[2][2] = (int16_t)(v & 0xffff); /* pad ignored */ }

/* IR load. */
void gte_ldIR1(int v) { g_IR[0] = v; }
void gte_ldIR2(int v) { g_IR[1] = v; }
void gte_ldIR3(int v) { g_IR[2] = v; }

/* "load short vector" -- write all three IRs at once. */
void gte_ldsv_(int x, int y, int z) { g_IR[0] = x; g_IR[1] = y; g_IR[2] = z; }

/* RTIR-with-B: IR = (R * IR) >> 12, saturated to i16.
 * Used by Object_ApplyAngularVelocity for the small-rotation update. */
void gte_rtir_b(void) {
    int64_t r0 = (int64_t)g_R[0][0] * g_IR[0]
               + (int64_t)g_R[0][1] * g_IR[1]
               + (int64_t)g_R[0][2] * g_IR[2];
    int64_t r1 = (int64_t)g_R[1][0] * g_IR[0]
               + (int64_t)g_R[1][1] * g_IR[1]
               + (int64_t)g_R[1][2] * g_IR[2];
    int64_t r2 = (int64_t)g_R[2][0] * g_IR[0]
               + (int64_t)g_R[2][1] * g_IR[1]
               + (int64_t)g_R[2][2] * g_IR[2];
    g_IR[0] = clamp_i16(r0 >> 12);
    g_IR[1] = clamp_i16(r1 >> 12);
    g_IR[2] = clamp_i16(r2 >> 12);
}

/* IR readbacks. */
int32_t gte_stIR1(void) { return g_IR[0]; }
int32_t gte_stIR2(void) { return g_IR[1]; }
int32_t gte_stIR3(void) { return g_IR[2]; }

/* ---- libgte high-level helpers ---- */

/* MatrixNormal: re-orthogonalize via Gram-Schmidt. PSY-Q's spec says
 * each row gets normalized to length 0x1000 (q12 "unit"). */
void MatrixNormal(const MATRIX *m_in, MATRIX *m_out)
{
    if (m_in != m_out) *m_out = *m_in;
    for (int r = 0; r < 3; r++) {
        int64_t a = m_out->m[r][0];
        int64_t b = m_out->m[r][1];
        int64_t c = m_out->m[r][2];
        int64_t mag2 = a*a + b*b + c*c;
        if (mag2 <= 0) {
            /* Degenerate -- replace with identity row to recover. */
            m_out->m[r][0] = (r == 0) ? 0x1000 : 0;
            m_out->m[r][1] = (r == 1) ? 0x1000 : 0;
            m_out->m[r][2] = (r == 2) ? 0x1000 : 0;
            continue;
        }
        double mag = sqrt((double)mag2);
        double scale = 4096.0 / mag;
        m_out->m[r][0] = (int16_t)((double)a * scale);
        m_out->m[r][1] = (int16_t)((double)b * scale);
        m_out->m[r][2] = (int16_t)((double)c * scale);
    }
}

/* PSY-Q rsin/rcos: angle in [0, 4096), return q12 sin/cos. */
int rsin(int a) {
    double rad = (double)(a & 0xfff) * (2.0 * 3.14159265358979 / 4096.0);
    return (int)(sin(rad) * 4096.0);
}
int rcos(int a) {
    double rad = (double)(a & 0xfff) * (2.0 * 3.14159265358979 / 4096.0);
    return (int)(cos(rad) * 4096.0);
}
int ratan2(int y, int x) {
    if (x == 0 && y == 0) return 0;
    double rad = atan2((double)y, (double)x);
    /* Wrap to [0, 4096). PSY-Q convention: 0 = +X axis, CCW positive. */
    double turns = rad / (2.0 * 3.14159265358979);
    if (turns < 0) turns += 1.0;
    return (int)(turns * 4096.0) & 0xfff;
}
