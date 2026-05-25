/* vec_math64.c -- 64-bit-precision vector dot product + Euclidean
 * length for the i32 3-vector format used by the engine's physics
 * state (positions, velocities, deltas).
 *
 * Source: SLUS_005.10
 *   FUN_800171d4  -- Vec3_Dot64(const int32_t *a, const int32_t *b) -> int64_t
 *   FUN_80016a20  -- Vec3_Length(const int32_t *v) -> int32_t
 *
 * These are the foundational helpers that every per-tick physics
 * function downstream (speed clamp, drag, terrain probe) relies on.
 * Both are bit-exact ports of the MIPS bodies in analysis/, with the
 * pseudo-C-incomplete Vec3_Dot64 (Ghidra stubbed it to `void`) filled
 * in from the disassembly.
 *
 * Vec3_Dot64 carries a full 64-bit signed accumulator across three
 * 32x32->64 multiplies; the engine relies on the high half to detect
 * overflow during the LZCR-based sqrt normalisation in Vec3_Length.
 *
 * Vec3_Length runs Vec3_Dot64(v, v) to get mag^2 as i64, then uses
 * the GTE's LZCS/LZCR leading-zero counter to choose a shift amount
 * that lets PSY-Q's SquareRoot0 (a u32-domain sqrt) work on the
 * normalised value, then unshifts the result.  The normalisation
 * shift count is `(0x23 - LZCR) >> 1`, doubled when re-applied so
 * the final shift takes us back to the original magnitude scale.
 *
 * HIGH: structural body matches MIPS line-for-line; the LZCS/LZCR
 * + SquareRoot0 dance is exactly the published PSY-Q idiom for
 * "sqrt of an int64 with bias toward precision retention".
 */
#include <stdint.h>
#include "gte.h"

/* HIGH: 64-bit dot product of two i32 3-vectors.
 *
 * MIPS body (analysis/SLUS_005.10/mips/800171d4.s):
 *   mult v1=a[0], v0=b[0]      -> (HI:t1 / LO:t0)
 *   mult v1=a[1], v0=b[1]      -> (HI:a3 / LO:a2)
 *   v0 = t0 + a2               -> low partial sum
 *   v1 = t1 + a3 + carry(v0)   -> high partial sum
 *   mult v1=a[2], v0=b[2]      -> (HI:t3 / LO:t2)
 *   v0 = v0 + t2               -> final low
 *   v1 = v1 + t3 + carry(v0)   -> final high
 * Returns (HI<<32)|LO in (v1, v0) as the 64-bit result.
 */
int64_t Vec3_Dot64(const int32_t *a, const int32_t *b)
{
    int64_t s = (int64_t)a[0] * (int64_t)b[0];
    s += (int64_t)a[1] * (int64_t)b[1];
    s += (int64_t)a[2] * (int64_t)b[2];
    return s;
}

/* HIGH: Euclidean length of an i32 3-vector.
 *
 * Bit-exact port of FUN_80016a20.  The algorithm:
 *
 *   ss   = Vec3_Dot64(v, v)            // i64
 *   hi   = ss >> 32                    // signed
 *   LZCR = clz_of_sign_bit(hi)         // GTE LZCS/LZCR
 *   k    = (0x23 - LZCR) >> 1          // normalisation shift count
 *
 *   if (k & 0x10)   == bit set ->      // k negative-ish
 *       narrow = hi >> (2k)            // take from high half only
 *   else:
 *       narrow = (uint32_t)ss >> (2k)  // take from low half
 *       if k != 0:  narrow |= hi << (32 - 2k)  // OR in straddling bits
 *
 *   root = SquareRoot0(narrow)         // u32-domain sqrt
 *   return root << k                   // re-apply the half-shift
 *
 * The `<< 0x1b` test (bit 4 of k after shifting up by 27) is how the
 * MIPS code checks the sign of k cheaply.  When (k << 27) is negative,
 * bit 4 of k is set, which on x86 corresponds to k >= 16; that
 * triggers the "use hi only" branch.
 */
int32_t Vec3_Length(const int32_t *v)
{
    uint64_t ss = (uint64_t)Vec3_Dot64(v, v);
    int32_t  hi = (int32_t)(ss >> 32);
    uint32_t lo = (uint32_t)ss;

    gte_ldLZCS(hi);
    int32_t  lzcr = gte_stLZCR();
    uint32_t k    = (uint32_t)(0x23 - lzcr) >> 1;

    uint32_t narrow;
    if ((int32_t)(k << 0x1b) < 0) {
        /* k has bit 4 set -> shift the high half down. */
        narrow = (uint32_t)hi >> (k * 2 & 0x1f);
    } else {
        narrow = lo >> (k * 2 & 0x1f);
        if ((k << 0x1b) != 0) {
            /* k != 0 -> shift in the bits that straddle hi/lo. */
            narrow |= (uint32_t)hi << (((uint32_t)-(int32_t)k * 2) & 0x1f);
        }
    }
    long root = SquareRoot0((long)narrow);
    return (int32_t)((uint32_t)root << (k & 0x1f));
}

/* ============================================================
 * // GHIDRA REF (audit ground truth -- DO NOT EDIT MANUALLY)
 * ============================================================ */
/* FUN_ aliases so callers that use the raw address name link correctly. */
int64_t FUN_800171d4(const int32_t *a, const int32_t *b) { return Vec3_Dot64(a, b); }
int32_t FUN_80016a20(const int32_t *v)                   { return Vec3_Length(v); }

#if 0

/* --- SLUS_005.10 FUN_800171d4 (MIPS, Ghidra pseudo-C empty) ---
 * 800171d4:  lw v1,0x0(a0)        a[0]
 * 800171d8:  lw v0,0x0(a1)        b[0]
 * 800171e0:  mult v1,v0           HI:t1 / LO:t0 = a[0]*b[0]
 * 800171e4:  lw v1,0x4(a0)        a[1]
 * 800171e8:  mfhi t1
 * 800171ec:  mflo t0
 * 800171f0:  lw v0,0x4(a1)        b[1]
 * 800171f8:  mult v1,v0           HI:a3 / LO:a2 = a[1]*b[1]
 * 800171fc:  lw v1,0x8(a0)        a[2]
 * 80017200:  mfhi a3
 * 80017204:  mflo a2
 * 80017208:  lw v0,0x8(a1)        b[2]
 * 80017210:  mult v1,v0           HI:t3 / LO:t2 = a[2]*b[2]
 * 80017214:  addu v0,t0,a2        lo += a2
 * 80017218:  sltu a0,v0,a2        carry = lo<a2
 * 8001721c:  addu v1,t1,a3        hi += a3
 * 80017220:  addu v1,v1,a0        hi += carry
 * 80017224:  mfhi t3
 * 80017228:  mflo t2
 * 8001722c:  addu v0,v0,t2        lo += t2
 * 80017230:  sltu a0,v0,t2        carry = lo<t2
 * 80017234:  addu v1,v1,t3        hi += t3
 * 80017238:  jr ra
 * 8001723c:  _addu v1,v1,a0       hi += carry      <- delay slot
 */

/* --- SLUS_005.10 FUN_80016a20 (from analysis/SLUS_005.10/decomp/80016a20.c) ---
 * int FUN_80016a20(undefined4 param_1) {
 *   ...
 *   uVar5 = FUN_800171d4(param_1, param_1);
 *   iVar2 = (int)((ulonglong)uVar5 >> 0x20);
 *   gte_ldLZCS(iVar2);
 *   iVar3 = gte_stLZCR();
 *   uVar4 = 0x23 - iVar3 >> 1;
 *   if ((int)(uVar4 << 0x1b) < 0) {
 *     a = iVar2 >> (uVar4 * 2 & 0x1f);
 *   } else {
 *     a = (uint)uVar5 >> (uVar4 * 2 & 0x1f);
 *     if (uVar4 << 0x1b != 0) {
 *       a = a | iVar2 << (uVar4 * -2 & 0x1f);
 *     }
 *   }
 *   lVar1 = SquareRoot0(a);
 *   return lVar1 << (uVar4 & 0x1f);
 * }
 */

#endif  /* GHIDRA REF */
