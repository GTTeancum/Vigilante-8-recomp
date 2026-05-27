/* vehicle_collision.c -- Heavy vehicle-to-vehicle collision handler.
 *
 * Source: SLUS_005.10  FUN_8002d82c  (677 MIPS instructions, ~2.7 KB).
 *
 * Called from Vehicle_RollingTick mode 3 when two vehicles' OBBs overlap
 * (param_1 = self, param_2 = collision_node).  Dispatches one of four
 * impulse / damage paths keyed on the OTHER vehicle's kind byte at
 * other+4, after first short-circuiting on a sub-object kind == 3.
 *
 * collision_node layout (param_2):
 *   +0x00  uint32_t*  other vehicle
 *   +0x04  int32_t    event/impulse data
 *   +0x10  uint32_t*  sub-object  (early-exit if (u8)sub_obj+4 == 3)
 *   +0x14  int32_t[3] contact-velocity vector (passed as 'vec' to
 *                     velocity-impulse and torque helpers)
 *   +0x20  SVECTOR    collision normal {i16 vx, vy, vz, pad}
 *   +0x30  int32_t    restitution coefficient
 *
 * Four dispatch paths (kind = (u8)other+4):
 *   kind==7  rolling-vehicle kill + combo-hit bookkeeping.
 *            Tracks repeat hits via a 6-byte stat block at
 *            other_linked_obj+0xa4+0x12..0x17.  On totalling,
 *            sprintfs a "%s TOTALED!" message, applies a particle
 *            burst, and zeroes out kinematic state.
 *   kind==1  speed-based light damage.  impulse = speed * mass-arm,
 *            capped at hp/4, applied as a single linear hit.
 *   kind==2  full elastic collision -- both bodies get a q12 impulse
 *            computed from the relative momentum projected onto the
 *            collision normal, with restitution doubled in.  This is
 *            the only path that returns 1.
 *   default  generic contact: depth-penetration impulse on self
 *            only, plus a health-keyed SFX from the DAT_8001057c LUT.
 *
 * Returns 1 only on the kind==2 path; 0 elsewhere (including all
 * early-outs).  The caller (Vehicle_RollingTick) discards the value
 * but downstream wiring relies on the 1 to chain restitution effects.
 *
 * HIGH confidence: line-by-line MIPS port, cross-checked against
 * Ghidra pseudo-C at analysis/SLUS_005.10/decomp/8002d82c.c.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ---- in-tree helpers (live definitions) ---------------------------- */
extern void  *FUN_8001f5a0(intptr_t self_int, intptr_t collision_node_int); /* SAT axis setup */
extern int32_t FUN_8002b940(uint32_t *obj);                          /* Vehicle_GetStateName */
extern void   FUN_8002bc18(uint32_t *self);                          /* Vehicle_Kill */
extern void   FUN_80043408(uint32_t *m, int32_t *v, int32_t *out);   /* GTE_RotateLongMatTrans */

/* ---- panic stubs (platform/panic_stubs.c) -------------------------- */
extern int     FUN_8002c018(uint32_t *self);
extern int     FUN_8002c6fc(uint32_t *self, int impulse, const int32_t *vec, int flag);
extern int     FUN_8002c958(uint32_t *self, int impulse, const int32_t *vec, int flag);
extern int64_t FUN_80017240(const int32_t *v, const void *svec_normal);
extern void    FUN_80017594(uint32_t *self, const int32_t *impulse_vec, const int32_t *contact);
extern void    FUN_800434d0(uint32_t *self_local_mat, const void *normal, int32_t *out_vec);
extern void    FUN_800435c0(uint32_t *other_local_mat, int32_t *vec_in, int32_t *vec_out);
extern int     FUN_80012028(int idx_neg, int a, int b, int c, int d);
extern int     FUN_80012068(int idx_neg, int a, int b, int c);
extern int     FUN_8003fea8(uint32_t *xyz, uint32_t color);
extern uint32_t FUN_8004410c(void);
extern int     FUN_8004483c(uint32_t voice, uint32_t bank, int clip, const void *xyz);
extern int     FUN_80016a20(const int32_t *v);
extern int     FUN_8002d44c(uint32_t *linked_obj);
extern int     FUN_800129e8(int mode, const char *msg);
extern int     FUN_80053004(char *buf, const char *fmt, ...);
extern void   *puRam000007d0;

/* ---- globals (gp-relative; declared in platform/panic_stubs.c) ----- */
extern int32_t  iRam00000010;     /* "show damage" UI flag (gp+0x10)   */
extern uint32_t uRam000005f8;     /* default SFX bank id (gp+0x5f8)    */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* DAT_8001057c: 12-entry per-health-bucket impact-SFX lookup.  The
 * default-branch SFX path indexes this by min((u16)(other+6) >> 4, 11);
 * 0xFF entries are "no SFX".  Verified byte-for-byte against the EXE
 * at offset 0xd7c (load-addr 0x80010000, file offset = vma - load). */
static const uint8_t DAT_8001057c[12] = {
    0xff, 0xff, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x06, 0x07, 0x07
};

/* ---- fixed-point helpers ------------------------------------------- */

/* PSX q12 multiply: signed 64-bit product >> 12.
 *
 * The MIPS source uses an unsigned-multu + two signed-mult sign-
 * correction passes (12 instructions per scalar; 36 per 3-vector).
 * The combined output is identical to the signed 64-bit product
 * shifted right by 12 and truncated to i32 -- see HANDOFF.md for
 * the derivation. */
static inline int32_t q12_mul(int32_t a, int32_t b)
{
    return (int32_t)(((int64_t)a * (int64_t)b) >> 12);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* Extract a signed 32-bit "approach scalar" from the int64 dot-product
 * result returned by FUN_80017240.  MIPS extracts (lo>>13) | (hi<<19);
 * this is equivalent to (int32_t)(r >> 13) for signed r. */
static inline int32_t dot_q13(int64_t r)
{
    return (int32_t)(r >> 13);
}

static int vehicle_collision_trace_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_VEHICLE_COLLISION");
        cached = (env != NULL && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

static void trace_player_vehicle_collision(uint32_t *self, const char *stage,
                                           uint32_t *other, uint8_t *node,
                                           int32_t approach, int32_t response,
                                           const int32_t *impulse)
{
    if (!vehicle_collision_trace_enabled() || (void *)self != puRam000007d0)
        return;
    fprintf(stderr,
            "v8: vehicle_collision player stage=%s other=%p kind=%u flags=0x%x layer=%d other_pos=(0x%x,0x%x,0x%x) normal=(%d,%d,%d) pen=%d approach=%d resp=%d",
            stage, (void *)other,
            other ? (unsigned)*(uint8_t *)((uint8_t *)other + 0x04) : 0,
            other ? (unsigned)other[0] : 0,
            other ? (int)*(int16_t *)((uint8_t *)other + 0x06) : 0,
            other ? (unsigned)other[9] : 0,
            other ? (unsigned)other[10] : 0,
            other ? (unsigned)other[11] : 0,
            node ? (int)*(int16_t *)(node + 0x20) : 0,
            node ? (int)*(int16_t *)(node + 0x22) : 0,
            node ? (int)*(int16_t *)(node + 0x24) : 0,
            node ? *(int32_t *)(node + 0x30) : 0,
            approach, response);
    if (impulse != NULL)
        fprintf(stderr, " impulse=(%d,%d,%d)", impulse[0], impulse[1], impulse[2]);
    fprintf(stderr, "\n");
}

/* ==================================================================== */
int FUN_8002d82c(uint32_t *self, intptr_t collision_node_int)
{
    uint32_t *collision_node = (uint32_t *)(uintptr_t)collision_node_int;
    uint8_t *s5 = (uint8_t *)self;
    uint8_t *s7 = (uint8_t *)collision_node;

    /* Sub-object kind == 3 -> short-circuit (no collision response). */
    uint32_t sub_obj = *(uint32_t *)(s7 + 0x10);
    if (sub_obj != 0 && *(uint8_t *)(uintptr_t)(sub_obj + 4) == 3) {
        trace_player_vehicle_collision(self, "sub-kind3-skip", NULL, s7, 0, 0, NULL);
        return 0;
    }

    uint32_t *other = (uint32_t *)(uintptr_t)*(uint32_t *)collision_node;
    uint8_t  *s3    = (uint8_t *)other;

    /* Always re-run SAT axis setup before reading kind. */
    FUN_8001f5a0((intptr_t)self, (intptr_t)collision_node);

    uint8_t kind = *(uint8_t *)(s3 + 4);

    /* ============================================================
     * Kind == 7 : rolling-vehicle kill + combo-hit bookkeeping.
     * ============================================================ */
    if (kind == 7) {
        if (*(uint16_t *)(s5 + 0x11e) != 0) {
            FUN_8002c018(self);
            return 0;
        }

        uint32_t other_flags = *(uint32_t *)s3;
        if (other_flags & 0x800000u) {
            uint32_t s2_word = *(uint32_t *)(s3 + 0x80);
            uint8_t *s0      = (uint8_t *)(uintptr_t)(s2_word + 0xa4);

            uint8_t  s0_12   = *(uint8_t  *)(s0 + 0x12);
            int16_t  self_st = *(int16_t  *)(s5 + 6);
            int8_t   s0_13s  = *(int8_t   *)(s0 + 0x13);
            uint16_t other_a = *(uint16_t *)(s3 + 0xa);
            uint8_t  s0_14   = *(uint8_t  *)(s0 + 0x14);
            uint8_t  store_14;

            if (s0_12 == 0
                || (int)self_st != (int)s0_13s
                || (uint32_t)other_a == (uint32_t)s0_14) {
                /* First-hit path: install seed values, leave damage alone. */
                uint8_t v = (*(uint8_t *)(s0 + 0x15) != 0) ? 0x01 : 0x1e;
                *(uint8_t *)(s0 + 0x12) = v;
                *(uint8_t *)(s0 + 0x13) = *(uint8_t *)(s5 + 6);
                store_14 = *(uint8_t *)(s3 + 0xa);
            } else {
                /* Repeat-hit path: bump combo count, scale + clamp damage. */
                uint8_t b = (uint8_t)mips_addu_i32(*(uint8_t *)(s0 + 0x15), 1);
                *(uint8_t *)(s0 + 0x15) = b;
                *(uint8_t *)(s0 + 0x12) = 0x1e;

                uint32_t scaled = (uint32_t)mips_mult_lo_i32(
                    (int32_t)(uint32_t)*(uint16_t *)(s3 + 0xc),
                    mips_addu_i32(b, 1));
                uint32_t clamped = (scaled < 0xffffu) ? scaled : 0xffffu;
                *(int16_t *)(s3 + 0xc) = (int16_t)clamped;
                store_14 = *(uint8_t *)(s3 + 0xa);
            }
            *(uint8_t *)(s0 + 0x14) = store_14;

            other_flags &= 0xff7fffffu;             /* clear bit 23 */
            *(uint32_t *)s3 = other_flags;

            /* "Totaled" branch: self_hp == 0 AND other had bit 24 set. */
            if (*(uint16_t *)(s5 + 0xc) == 0 && (other_flags & 0x1000000u)) {
                if (*(int16_t *)(uintptr_t)(s2_word + 6) < 0) {
                    char msgbuf[40];
                    int32_t name = FUN_8002b940(self);
                    FUN_80053004(msgbuf, "%s TOTALED!", name);
                    int show_mode = (iRam00000010 != 0)
                        ? mips_subu_i32(0, (int)*(int16_t *)(uintptr_t)(s2_word + 6))
                        : 0;
                    FUN_800129e8(show_mode, msgbuf);
                }
                FUN_8002bc18(self);
                FUN_8003fea8((uint32_t *)(s5 + 0x24), 0x80000ffu);
                *(uint32_t *)(s5 + 0x94)  = 0xc350u;        /* dragMass kick    */
                *(int32_t  *)(s5 + 0x84) =
                    mips_subu_i32(*(int32_t *)(s5 + 0x84), 0xee680); /* vel.y impulse */
                FUN_8002d44c((uint32_t *)(uintptr_t)s2_word);
                *(uint8_t *)(s0 + 0x17) =
                    (uint8_t)mips_addu_i32(*(uint8_t *)(s0 + 0x17), 1);
            }
        }

        /* Always-on: apply linear damage impulse keyed on other's word_at_0xc. */
        int impulse_neg = mips_subu_i32(0, (int)*(uint16_t *)(s3 + 0xc));
        FUN_8002c6fc(self, impulse_neg, (const int32_t *)(s7 + 0x14), 1);
        return 0;
    }

    /* ============================================================
     * Kind == 1 : speed-based light damage.
     * ============================================================ */
    if (kind == 1) {
        int32_t impulse;
        if (*(uint32_t *)s3 & 0x10000u) {
            /* Cached-speed path: use other+0x84 directly as the arm. */
            int32_t spd = *(int32_t *)(s3 + 0x54);
            if (spd < 0) spd = mips_addu_i32(spd, 0xff);
            spd >>= 8;
            impulse = mips_mult_lo_i32(*(int32_t *)(s3 + 0x84), spd);
        } else {
            /* Compute the arm by vec-length of the +0x88 block. */
            int32_t mag = FUN_80016a20((const int32_t *)(s3 + 0x88));
            int32_t spd = *(int32_t *)(s3 + 0x54);
            if (spd < 0) spd = mips_addu_i32(spd, 0xff);
            spd >>= 8;
            impulse = mips_mult_lo_i32(mag, spd);
        }
        if (impulse < 0) impulse = mips_addu_i32(impulse, 0xfff);
        int32_t scaled = impulse >> 12;

        int32_t cap = (int32_t)((uint32_t)*(uint16_t *)(s5 + 0xc) >> 2);
        int32_t use = (scaled < cap) ? scaled : cap;

        *(uint32_t *)s3 |= 0x20u;
        FUN_8002c958(self, mips_subu_i32(0, use), (const int32_t *)(s7 + 0x14), 0);
        return 0;
    }

    /* ============================================================
     * Kind == 2 : elastic collision (both bodies get an impulse).
     * ============================================================ */
    if (kind == 2) {
        const void *normal_svec  = (const void *)(s7 + 0x20);
        uint32_t self_m  = (uint32_t)*(uint16_t *)(s5 + 0xa2) >> 6;
        uint32_t other_m = (uint32_t)*(uint16_t *)(s3 + 0xa2) >> 6;

        /* Relative momentum vector: mass-scaled velocity difference.
         * MIPS uses signed mult (mult, not multu); the low-32 truncate
         * must be explicit in C to avoid signed-overflow UB. */
        int32_t rv[3];
        rv[0] = mips_subu_i32(mips_mult_lo_i32(*(int32_t *)(s5 + 0x80), (int32_t)self_m),
                              mips_mult_lo_i32(*(int32_t *)(s3 + 0x80), (int32_t)other_m));
        rv[1] = mips_subu_i32(mips_mult_lo_i32(*(int32_t *)(s5 + 0x84), (int32_t)self_m),
                              mips_mult_lo_i32(*(int32_t *)(s3 + 0x84), (int32_t)other_m));
        rv[2] = mips_subu_i32(mips_mult_lo_i32(*(int32_t *)(s5 + 0x88), (int32_t)self_m),
                              mips_mult_lo_i32(*(int32_t *)(s3 + 0x88), (int32_t)other_m));

        int32_t s4 = dot_q13(FUN_80017240(rv, normal_svec));

        if (s4 < 0) {
            int32_t restitution = *(int32_t *)(s7 + 0x30);
            int32_t tmp[3];

            /* ---- Self gets the negative impulse ---- */
            int32_t s2 = s4 / (int32_t)self_m;
            FUN_800434d0((uint32_t *)(s5 + 0x10), normal_svec, tmp);
            {
                int32_t mag = mips_subu_i32(
                    0, mips_addu_i32(mips_addu_i32(restitution, restitution), s2));
                tmp[0] = q12_mul(tmp[0], mag);
                tmp[1] = q12_mul(tmp[1], mag);
                tmp[2] = q12_mul(tmp[2], mag);
            }
            FUN_80017594(self, tmp, (const int32_t *)(s7 + 0x14));

            int32_t s2_round = (s2 < 0) ? mips_addu_i32(s2, 0x1fff) : s2;
            s2_round >>= 13;
            if (s2_round < -8) {
                FUN_8002c958(self, s2_round,
                             (const int32_t *)(s7 + 0x14), 1);
                int16_t self_status = *(int16_t *)(s5 + 6);
                if (self_status < 0)
                    FUN_80012028(~(int)self_status, 10, 0xc0, 0, 0x40);
            }

            /* Bounce the contact vector into both bodies' local frames. */
            FUN_80043408((uint32_t *)(s5 + 0x10),
                         (int32_t *)(s7 + 0x14),
                         (int32_t *)(s7 + 0x14));
            FUN_800435c0((uint32_t *)(s3 + 0x10),
                         (int32_t *)(s7 + 0x14),
                         (int32_t *)(s7 + 0x14));

            /* ---- Other gets the positive impulse ---- */
            int32_t s2b = s4 / (int32_t)other_m;
            FUN_800434d0((uint32_t *)(s3 + 0x10), normal_svec, tmp);
            {
                int32_t mag = mips_addu_i32(mips_addu_i32(restitution, restitution), s2b);
                tmp[0] = q12_mul(tmp[0], mag);
                tmp[1] = q12_mul(tmp[1], mag);
                tmp[2] = q12_mul(tmp[2], mag);
            }
            FUN_80017594(other, tmp, (const int32_t *)(s7 + 0x14));

            int32_t s2b_round = (s2b < 0) ? mips_addu_i32(s2b, 0x1fff) : s2b;
            s2b_round >>= 13;
            if (s2b_round < -8) {
                FUN_8002c958(other, s2b_round,
                             (const int32_t *)(s7 + 0x14), 1);
                int16_t other_status = *(int16_t *)(s3 + 6);
                if (other_status < 0)
                    FUN_80012028(~(int)other_status, 10, 0xc0, 0, 0x40);
            }

            /* Collision SFX (only when self isn't already in "fast" state
             * AND its speed-cached field > 0x1c9). */
            if (!(*(uint32_t *)s5 & 0x8000u)
                && *(int32_t *)(s5 + 0x8c) > 0x1c9) {
                uint32_t voice = FUN_8004410c();
                FUN_8004483c(voice, uRam000005f8, 0x1e,
                             (const void *)(s3 + 0x48));
            }
        }

        *(uint32_t *)s5 |= 0x18000u;
        return 1;
    }

    /* ============================================================
     * Default branch : generic contact / depth penetration + SFX.
     * ============================================================ */

    /* If other is in this vehicle's obstacle chain AND the contact normal
     * has a steep negative Y (floor contact), skip the response. */
    if ((*(uint32_t *)(s5 + 0x74) == (uint32_t)(uintptr_t)other
      || *(uint32_t *)(s5 + 0x78) == (uint32_t)(uintptr_t)other)
        && *(int16_t *)(s7 + 0x22) < -0x800) {
        trace_player_vehicle_collision(self, "floor-contact-skip", other, s7, 0, 0, NULL);
        return 0;
    }

    int32_t s4 = dot_q13(FUN_80017240((const int32_t *)(s5 + 0x80),
                                       (const void *)(s7 + 0x20)));
    if (s4 >= 0) {
        trace_player_vehicle_collision(self, "moving-away-skip", other, s7, s4, 0, NULL);
        return 0;
    }

    int32_t neg_s4 = mips_subu_i32(0, s4);
    if (neg_s4 < 0) neg_s4 = mips_addu_i32(neg_s4, 0x3fff);
    int32_t pen = mips_mult_lo_i32(neg_s4 >> 14,
                                   (int32_t)*(uint16_t *)(s5 + 0xa2));
    if (pen < 0) pen = mips_addu_i32(pen, 0xfff);

    /* Ask other vehicle's tick callback (other+0x64) whether it accepts
     * this hit; mode = 8, arg = pen >> 12.  Skip the response if it does. */
    {
        typedef int (*TickFn)(uint32_t *, int, intptr_t);
        TickFn cb = (TickFn)Object_CallbackFromPsxSlot(s3);
        int response = cb ? cb(other, 8, pen >> 12) : 0;
        if (response != 0) {
            trace_player_vehicle_collision(self, "mode8-skip", other, s7, s4, response, NULL);
            return 0;
        }
    }

    /* Apply impulse to self only (other handled it via callback or had
     * no callback to opt-out). */
    {
        int32_t tmp[3];
        FUN_800434d0((uint32_t *)(s5 + 0x10), (const void *)(s7 + 0x20), tmp);
        int32_t mag = mips_subu_i32(
            0, mips_addu_i32(*(int32_t *)(s7 + 0x30), s4));
        tmp[0] = q12_mul(tmp[0], mag);
        tmp[1] = q12_mul(tmp[1], mag);
        tmp[2] = q12_mul(tmp[2], mag);
        trace_player_vehicle_collision(self, "impulse", other, s7, s4, 0, tmp);
        FUN_80017594(self, tmp, (const int32_t *)(s7 + 0x14));
    }

    if (!(*(uint32_t *)s5 & 0x8000u)) {
        int32_t s4_round = (s4 < 0) ? mips_addu_i32(s4, 0x3fff) : s4;
        int32_t depth   = s4_round >> 14;
        int32_t cap_neg = mips_subu_i32(0, (int32_t)*(uint16_t *)(s3 + 0xc));
        if (cap_neg < depth) cap_neg = depth;
        FUN_8002c958(self, cap_neg, (const int32_t *)(s7 + 0x14), 0);

        int16_t self_status = *(int16_t *)(s5 + 6);
        if (self_status < 0)
            FUN_80012068(~(int)self_status, 0xff, 0, 0x40);

        if (*(int32_t *)(s5 + 0x8c) > 0x1c9) {
            uint32_t idx = (uint32_t)*(uint16_t *)(s3 + 6) >> 4;
            if (idx > 0xb) idx = 0xb;
            uint8_t clip = DAT_8001057c[idx];
            if (clip != 0xff) {
                uint32_t voice = FUN_8004410c();
                FUN_8004483c(voice, uRam000005f8, clip,
                             (const void *)(s3 + 0x24));
            }
        }
    }

    *(uint32_t *)s5 |= 0x18000u;
    return 0;
}
