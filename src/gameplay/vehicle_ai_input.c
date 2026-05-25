/* vehicle_ai_input.c -- two physics-input helpers used by the enemy AI.
 *
 * Source: SLUS_005.10
 *   FUN_80016aac  -- Vec3_Distance(a, b) -> i32  Euclidean distance (15 instr)
 *   FUN_80030a88  -- Vehicle_ApplyAILut(obj, lut) (98 instr)
 *
 * Vec3_Distance subtracts two i32[3] vectors and returns Vec3_Length of
 * the delta.  Simple wrapper around Vec3_Length (FUN_80016a20) which is
 * the LZCR-driven sqrt.
 *
 * Vehicle_ApplyAILut dispatches on a LUT-entry kind field and updates
 * the vehicle's physics-input halfwords at self+0xa4 (angYPreBake /
 * longitudinal force) and self+0xa6 (steerNudge) each frame.
 *
 * LUT entry layout at (const uint8_t *)lut:
 *   [+0x00] i16  kind     -- record type
 *   [+0x08] u32  flags    -- control bits
 *   [+0x10] u8   throttle -- raw throttle (0x80 = neutral)
 *   [+0x11] u8   steerL
 *   [+0x12] u8   steerR
 *
 * Kind dispatch:
 *   0, 1    -> return (no update)
 *   2       -> steer nudge from flags; longitudinal force drifts ±16/frame
 *              or decays 15/16 per frame
 *   3       -> linear: long = (throttle-0x80)*5, steer = (steerL-steerR)/2
 *   4, 5    -> cubic: long = (|t|*t)/24, steer from steerL or 0x80
 *   >= 6    -> return
 *
 * HIGH: line-for-line MIPS verified against 80016aac.s + 80030a88.s.
 * Ghidra pseudo-C in analysis/SLUS_005.10/decomp/80030a88.c confirms
 * the /24 and /2 divisors.
 */
#include <stdint.h>

/* Vec3_Length is the LZCR-based sqrt from vec_math64.c. */
extern int32_t Vec3_Length(const int32_t *v);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* HIGH: Euclidean distance between two i32[3] points. */
int32_t FUN_80016aac(const int32_t *a, const int32_t *b)
{
    int32_t delta[3];
    delta[0] = mips_subu_i32(a[0], b[0]);
    delta[1] = mips_subu_i32(a[1], b[1]);
    delta[2] = mips_subu_i32(a[2], b[2]);
    return Vec3_Length(delta);
}

/* ---- Vehicle_ApplyAILut -------------------------------------------- */

/* RTZ (round-toward-zero) arithmetic right shift -- matches MIPS
 * `bgez v,skip; addiu v,v,((1<<n)-1); sra v,v,n`. */
static inline int32_t rtz_sra(int32_t v, int n)
{
    if (v < 0) v = mips_addu_i32(v, (1 << n) - 1);
    return v >> n;
}

/*
 * HIGH: apply a LUT-encoded AI driving record to the vehicle.
 *
 * a0 = obj  -- uint32_t * to the vehicle object
 * a1 = lut  -- pointer to the LUT entry (byte-addressable)
 */
void FUN_80030a88(uint32_t *obj, const void *lut)
{
    const uint8_t *b = (const uint8_t *)lut;

    int16_t  kind  = *(const int16_t *)b;
    uint32_t flags = *(const uint32_t *)(b + 8);

    /* a2 = obj + 0xa4 */
    int16_t *a4 = (int16_t *)((uint8_t *)obj + 0xa4);   /* angYPreBake */
    int16_t *a6 = (int16_t *)((uint8_t *)obj + 0xa6);   /* steerNudge  */

    if (kind == 3) {
        /* Linear throttle: long = (throttle-0x80)*5 */
        int t = mips_subu_i32((int)b[0x10], 0x80);
        *a4   = (int16_t)mips_mult_lo_i32(t, 5);
        /* Steer = floor((steerL - steerR) / 2), C's /2 is RTZ for signed. */
        *a6   = (int16_t)(mips_subu_i32((int)b[0x11], (int)b[0x12]) / 2);
        return;
    }

    if (kind > 3) {
        if (kind < 6) {
            /* Cubic throttle: long = (|t| * t) / 24 */
            int t  = mips_subu_i32((int)b[0x10], 0x80);
            int ta = (t < 0) ? mips_subu_i32(0, t) : t;
            *a4 = (int16_t)(mips_mult_lo_i32(ta, t) / 24);
            if ((flags & 0x100u) == 0) {
                *a6 = (int16_t)mips_subu_i32(0x80, (int)b[0x11]);
            } else {
                *a6 = 0x80;
            }
        }
        return;
    }

    /* kind == 0 or 1 -> no-op */
    if (kind != 2) {
        return;
    }

    /* kind == 2: steer from flags, longitudinal tracks/drifts/decays */
    {
        uint32_t u = (flags >> 8) & 1u;
        if ((flags & 0x200u) == 0) {
            *a6 = (int16_t)(u << 7);              /* 0 or 128 */
        } else {
            *a6 = (int16_t)mips_mult_lo_i32(mips_subu_i32((int)u, 1), 0x80);
        }

        /* Decrease longitudinal if above lower bound */
        if ((flags & 0x800u) != 0 && *a4 > (int16_t)-0x2aa) {
            *a4 = (int16_t)mips_subu_i32(*a4, 0x10);
            return;
        }
        /* Increase longitudinal if below upper bound */
        if ((flags & 0x1000u) != 0 && *a4 < (int16_t)0x2aa) {
            *a4 = (int16_t)mips_addu_i32(*a4, 0x10);
            return;
        }
        /* Decay: self[0xa4] *= 15/16 (RTZ) */
        *a4 = (int16_t)mips_subu_i32((int)*a4, rtz_sra((int)*a4, 4));
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80016aac  (from analysis/SLUS_005.10/decomp/80016aac.c) ---
 * void FUN_80016aac(void) { FUN_80016a20(); return; }
 * (Ghidra sees no args because MIPS a0/a1 pass through from caller unchanged) */

/* --- SLUS_005.10 FUN_80030a88  (from analysis/SLUS_005.10/decomp/80030a88.c) ---
 * See analysis/SLUS_005.10/decomp/80030a88.c for full Ghidra pseudo-C.
 * Confirmed: /24 = (|t|*t)/24 for kind 4/5; /2 for kind 3 steer. */

#endif  /* GHIDRA REF */
