/* vehicle_damaged_physics.c -- per-frame physics for vehicles in damaged state.
 *
 * Source: SLUS_005.10  FUN_8002efe0  (620 MIPS instructions).
 *
 * Called from Vehicle_Tick (LAB_8002e2bc) mode-0 damaged path:
 *   FUN_8002efe0(self, lut);
 * where lut = &DAT_80065c28[((~status)*3)*8] -- pointer into the
 * per-status physics-attribute table.  Entries are 0x18 (24) bytes apart.
 *
 * LUT entry layout (byte offsets from entry base):
 *   +0x00  int16_t   mode       (2=normal, 3=init, 4/5=player-aim)
 *   +0x08  uint32_t  flags      (physics-control switches; s3 in MIPS)
 *   +0x0c  uint32_t  flags2     (secondary flags; high nibble gates call_flags)
 *   +0x10  uint8_t   steer_raw  (joystick byte, 0x80 = centre)
 *   +0x11  uint8_t   throttle   (joystick byte, 0x80 = centre)
 *   +0x12  uint8_t   field12    (dead-band / gear byte)
 *   +0x13  uint8_t   field13    (secondary longitudinal input)
 *   +0x14  uint8_t   field14    (reference byte for rotation delta)
 *   +0x15  int8_t    field15s   (signed: direction modifier)
 *
 * Vehicle sub-state block at self+0xa4:
 *   +0x00  int16_t   steer      (self+0xa4)
 *   +0x02  int16_t   inputMul   (self+0xa6)
 *   +0x04  int16_t   sub04      (self+0xa8)
 *   +0x06  int16_t   sub06      (self+0xaa)
 *   +0x08  uint16_t  maxInput   (self+0xac)
 *   +0x0e  int8_t    dir        (self+0xb2, direction sign)
 *
 * flags bit assignments (verified from MIPS branch patterns):
 *   0x0100   my-dir bit A (forward if dir>=0, reverse if dir<0)
 *   0x0200   my-dir bit B
 *   0x0400   steer-force active (direct steer increment path)
 *   0x0800   right-turn flag
 *   0x1000   tilt bit A  }  combined = 0x1800
 *   0x0800   tilt bit B  }  (actually 0x1800 = bits 11+12)
 *   0x8000000 enemy/boss route (use FUN_8002c59c instead of FUN_8002ee94)
 *   0x8000000 elevation-down (when upside-down)  [same bit reused by context]
 *   0x10000000 elevation-up
 *   0x18000000 lateral-skid-enable bits
 *
 * HIGH confidence: line-by-line MIPS port, cross-checked against
 * Ghidra pseudo-C at analysis/SLUS_005.10/decomp/8002efe0.c.
 */
#include <stdint.h>

/* ---- helpers (panic stubs until decompiled) ---- */
extern void FUN_8002ee94(uint32_t *self, uint32_t flags);   /* wheel-force dispatch */
extern void FUN_8002c59c(uint32_t *self, uint32_t flags);   /* enemy variant */

/* ---- fixed-point: RTZ arithmetic-right-shift ---- */
static inline int32_t rtz_sra(int32_t v, unsigned n)
{
    /* MIPS pattern: bgez v,skip; addiu v,v,((1<<n)-1); sra v,v,n */
    if (v < 0) v += (int32_t)((1u << n) - 1u);
    return v >> (int)n;
}

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

/* Shared dragMass-from-steer computation used in several paths.
 * MIPS: speed*sub06 q12, add sub04, clamp>=0, sign by dir,
 *       mult by steer, q4, add to drag.  Writes self+0x94. */
static void apply_steer_drag(uint8_t *s, uint8_t *sub)
{
    int32_t speed  = *(int32_t  *)(s   + 0x8c);
    int16_t sub06  = *(int16_t  *)(sub + 0x06);
    int16_t sub04  = *(int16_t  *)(sub + 0x04);
    int16_t steer  = *(int16_t  *)(sub + 0x00);
    int8_t  dir    = *(int8_t   *)(sub + 0x0e);
    uint32_t drag  = *(uint32_t *)(s   + 0x94);

    int32_t torque = mips_mult_lo_i32(speed, (int32_t)sub06);
    int32_t base   = mips_addu_i32((int32_t)sub04, rtz_sra(torque, 12));
    if (base < 0) base = 0;
    int32_t arm    = (dir < 0) ? mips_subu_i32(0, base) : base;
    int32_t delta  = rtz_sra(mips_mult_lo_i32((int32_t)steer, arm), 4);
    *(uint32_t *)(s + 0x94) = (uint32_t)mips_addu_i32((int32_t)drag, delta);
}

/* Shared: compute inputMul for forward direction from LUT throttle bytes.
 * Returns the (signed) value; caller stores to sub+0x02.
 * MIPS: if (lut[0x12] >= 0xf1 && |steer| >= 0xaa) use lut[0x11]*v1
 *       else use (lut[0x11]-lut[0x12])*v1, both >> 8 RTZ.            */
static int16_t compute_inputMul_fwd(const uint8_t *lut, uint16_t maxI,
                                     int16_t steer)
{
    uint32_t l11  = *(uint8_t *)(lut + 0x11);
    uint32_t l12  = *(uint8_t *)(lut + 0x12);
    int32_t  st_abs = (steer < 0) ? mips_subu_i32(0, (int32_t)steer) : (int32_t)steer;
    int32_t  eff;
    if (l12 >= 0xf1u && (uint32_t)st_abs >= 0xaau) {
        /* High dead-band and steer off-centre: use raw throttle. */
        eff = (int32_t)(l11 * (uint32_t)maxI);
    } else {
        /* Normal: throttle above dead-band drives inputMul. */
        eff = (int32_t)((l11 - l12) * (uint32_t)maxI);
    }
    return (int16_t)rtz_sra(eff, 8);
}

/* Shared: call FUN_8002c59c or FUN_8002ee94 based on enemy bit. */
static void dispatch_input(uint32_t *self, uint32_t flags, uint32_t s1)
{
    if (self[0] & 0x8000000u)
        FUN_8002c59c(self, s1);
    else
        FUN_8002ee94(self, s1);
}

/* ==================================================================== */
void FUN_8002efe0(uint32_t *self, const uint8_t *lut)
{
    uint8_t  *s   = (uint8_t *)self;
    uint8_t  *sub = s + 0xa4;          /* s0 in MIPS */

    int16_t  mode  = *(int16_t  *)(lut + 0x00);
    uint32_t flags = *(uint32_t *)(lut + 0x08);   /* s3 */

    /* ------------------------------------------------------------------ */
    if (mode == 3) goto L_mode3;
    if (mode < 4) {
        if (mode != 2) return;
        /* fall through into mode 2 */
    } else {
        if (mode < 6) goto L_mode45;
        return;
    }

    /* ==================================================================
     * MODE 2 -- normal per-frame vehicle physics
     * ================================================================== */
    {
        int8_t   dir    = *(int8_t  *)(sub + 0x0e);   /* self+0xb2 */
        uint32_t my_bit = (dir < 0) ? 0x200u : 0x100u;
        uint32_t op_bit = (dir < 0) ? 0x100u : 0x200u;

        /* ---- inputMul update ---- */
        if (flags & op_bit) {
            /* Opposing direction flag: braking / direction-flip. */
            int32_t speed = *(int32_t *)(s + 0x8c);
            if (speed < 0x1da) {
                /* Low speed: flip direction sign. */
                *(int8_t *)(sub + 0x0e) = (dir < 0) ? (int8_t)1 : (int8_t)-1;
            } else {
                /* High speed: set inputMul = -(maxInput). */
                *(int16_t *)(sub + 0x02) =
                    (int16_t)mips_subu_i32(0, (int32_t)*(uint16_t *)(sub + 0x08));
            }
        } else {
            /* Normal path: build active-direction flags and dispatch. */
            uint32_t active = flags & (my_bit | (my_bit << 16));

            if (self[0] & 0x8000000u) {
                FUN_8002c59c(self, active);
            } else {
                uint32_t call_flags = active;
                if (*(uint32_t *)(lut + 0x0c) & 0xf0000000u)
                    call_flags = active & 0xffffu;
                FUN_8002ee94(self, call_flags);

                if (active & 0xffffu) {
                    /* Driving input: latch maxInput. */
                    *(int16_t *)(sub + 0x02) =
                        (int16_t)*(uint16_t *)(sub + 0x08);
                } else {
                    /* No input: decay toward zero. */
                    int16_t im = *(int16_t *)(sub + 0x02);
                    /* MIPS: if im >= 0, im -= 2; else im = 0. */
                    *(int16_t *)(sub + 0x02) =
                        (im < 0) ? (int16_t)0
                                 : (int16_t)((uint16_t)im - 2u);
                }
            }
        }

        /* ---- Tilt / elevation / steering physics ---- */
        if (!(flags & 0x1800u)) goto L_drag_only;

        /* mat[1][1] at self+0x18: > 0 means upright, <= 0 means flipped. */
        if (*(int16_t *)(s + 0x18) <= 0) {
            /* Flipped: manage elevation angle only. */
            if (flags & 0x8000000u) {
                *(int32_t *)(s + 0x98) = mips_subu_i32(*(int32_t *)(s + 0x98), 0x4000);
            } else if (flags & 0x10000000u) {
                *(int32_t *)(s + 0x98) = mips_addu_i32(*(int32_t *)(s + 0x98), 0x4000);
            }
            return;
        }

        /* Re-read dir (may have changed above). */
        dir = *(int8_t *)(sub + 0x0e);
        my_bit = (dir < 0) ? 0x200u : 0x100u;

        /* Compute / update the 0x40000000 skid flag in self[0]. */
        uint32_t a2;          /* bit 30 of self[0], skid indicator */
        if (dir > 0) {
            /* Forward: clear skid flag. */
            self[0] &= ~0x40000000u;
            a2 = 0;
        } else {
            if (flags & 0x18000000u) {
                uint32_t f0 = self[0] & ~0x40000000u;
                if (dir < 0) {
                    if (!(flags & my_bit)) {
                        /* Check dot(mat_col2, vel/128) < -0x4c4000 threshold. */
                        int32_t vx  = *(int32_t *)(s + 0x80);
                        int16_t m02 = *(int16_t *)(s + 0x14);   /* mat[0][2] */
                        int32_t vz  = *(int32_t *)(s + 0x88);
                        int16_t m22 = *(int16_t *)(s + 0x20);   /* mat[2][2] */
                        if (mips_addu_i32(mips_mult_lo_i32((int32_t)m02,
                                                           rtz_sra(vx, 7)),
                                          mips_mult_lo_i32((int32_t)m22,
                                                           rtz_sra(vz, 7))) < -0x4c4000)
                            f0 |= 0x40000000u;
                    } else {
                        f0 |= 0x40000000u;
                    }
                }
                self[0] = f0;
            }
            a2 = (self[0] >> 30) & 1u;
        }

        /* ---- Steering update ---- */
        uint32_t drag = *(uint32_t *)(s + 0x94);

        if (flags & 0x400u) {
            /* Steer-force active: increment steer and adjust dragMass. */
            if (flags & 0x800u) {
                /* Right turn: steer -= 32, clamp >= -682. */
                int16_t st = *(int16_t *)(sub + 0x00);
                st = (int16_t)mips_subu_i32((int32_t)st, 0x20);
                if (st < -0x2aa) st = -0x2aa;
                *(int16_t *)(sub + 0x00) = st;
                /* skid → +0x500, else → -0x500. */
                drag = a2 ? (uint32_t)mips_addu_i32((int32_t)drag, 0x500)
                          : (uint32_t)mips_subu_i32((int32_t)drag, 0x500);
            } else {
                /* Left turn: steer += 32, clamp <= 682. */
                int16_t st = *(int16_t *)(sub + 0x00);
                st = (int16_t)mips_addu_i32((int32_t)st, 0x20);
                if (st > 0x2aa) st = 0x2aa;
                *(int16_t *)(sub + 0x00) = st;
                /* no-skid → +0x500, skid → -0x500. */
                drag = a2 ? (uint32_t)mips_subu_i32((int32_t)drag, 0x500)
                          : (uint32_t)mips_addu_i32((int32_t)drag, 0x500);
            }
            *(uint32_t *)(s + 0x94) = drag;
            return;
        }

        /* No steer force: coast with speed-scaled decay. */
        if (flags & 0x800u) {
            /* Right drift (steer moves negative). */
            int16_t st   = *(int16_t *)(sub + 0x00);
            /* MIPS: a0 = st - 16; v0 = (st < 0) ? st : 0; frac = RTZ(v0/64);
             *       nst = (a0 - frac) clamped to >= -682.
             *       If nst >= 0, return early (no change).            */
            int32_t frac = (st < 0) ? rtz_sra((int32_t)st, 6) : 0;
            int16_t nst  = (int16_t)mips_subu_i32(
                mips_subu_i32((int32_t)st, 16), frac);
            if (nst < -0x2aa) nst = -0x2aa;
            if (nst >= 0) return;                /* still in positive zone */
            *(int16_t *)(sub + 0x00) = nst;
        } else {
            /* Left drift (steer moves positive). */
            int16_t st   = *(int16_t *)(sub + 0x00);
            int32_t frac = (st > 0) ? rtz_sra((int32_t)st, 6) : 0;
            int16_t nst  = (int16_t)mips_subu_i32(
                mips_addu_i32((int32_t)st, 16), frac);
            if (nst > 0x2aa) nst = 0x2aa;
            if (nst <= 0) return;                /* still in negative zone */
            *(int16_t *)(sub + 0x00) = nst;
        }

        /* Compute and write dragMass via steer × speed formula. */
        {
            int32_t torque_mag = mips_mult_lo_i32(*(int32_t *)(s + 0x8c),
                                                  (int32_t)*(int16_t *)(sub + 0x06));
            int32_t dir_sign   = mips_subu_i32(mips_mult_lo_i32((int32_t)a2, 2), 1); /* +1 or -1 */
            int16_t nst        = *(int16_t *)(sub + 0x00);
            int32_t scaled_st  = mips_mult_lo_i32(dir_sign, (int32_t)nst);
            int32_t base       = mips_addu_i32((int32_t)*(int16_t *)(sub + 0x04),
                                               rtz_sra(torque_mag, 12));
            if (base < 0) base = 0;
            int32_t delta = rtz_sra(mips_mult_lo_i32(scaled_st, base), 4);
            *(uint32_t *)(s + 0x94) = (uint32_t)mips_subu_i32((int32_t)drag, delta);
        }
        return;

    L_drag_only: /* flags & 0x1800 == 0 */
        {
            int16_t  st   = *(int16_t  *)(sub + 0x00);
            uint16_t stu  = *(uint16_t *)(sub + 0x00);
            int32_t  spd  = *(int32_t  *)(s   + 0x8c);
            /* new_steer = steer(u16) - RTZ(steer*speed / 32768) */
            *(int16_t *)(sub + 0x00) =
                (int16_t)mips_subu_i32((int32_t)stu,
                                       rtz_sra(mips_mult_lo_i32((int32_t)st, spd), 15));
            if (flags & 0x400u)
                *(int16_t *)(sub + 0x02) =
                    (int16_t)mips_mult_lo_i32(
                        mips_subu_i32(0, (int32_t)*(uint16_t *)(sub + 0x08)), 2);
        }
        return;
    } /* end mode 2 */

    /* ==================================================================
     * MODE 3 -- state initialisation (vehicle enters damaged/grounded state)
     * ================================================================== */
L_mode3:
    {
        /* Set steer from lut[0x10] centred at 0x80, scaled ×5. */
        int32_t raw = mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x10), 0x80);
        *(int16_t *)(s + 0xa4) = (int16_t)mips_mult_lo_i32(raw, 5);

        uint32_t l11 = *(uint8_t *)(lut + 0x11);
        int8_t   f15 = *(int8_t  *)(lut + 0x15);

        /* Build s1 flags word. */
        uint32_t s1 = (l11 < 0x81u) ? 0u : 1u;   /* 0 if l11 <= 128 */
        if (f15 >= 0) s1 |= 0x10000u;

        if (self[0] & 0x8000000u) {
            FUN_8002c59c(self, s1);
            goto L_mode3_post;
        }

        /* ---- Update direction sign at self+0xb2 ---- */
        {
            int8_t dir = *(int8_t *)(sub + 0x0e);
            /* MIPS: if dir < 0 AND l11 >= 17: set dir = +1 (forward).
             *       else: if flags & 0x100: set dir = -1 (reverse).
             *             else: leave unchanged.                       */
            if (dir < 0 && (uint32_t)l11 >= 0x11u) {
                *(int8_t *)(sub + 0x0e) = 1;
            } else if (flags & 0x100u) {
                *(int8_t *)(sub + 0x0e) = -1;
            }
        }

        /* ---- Compute inputMul (self+0xa6) ---- */
        {
            int8_t   dir  = *(int8_t  *)(sub + 0x0e);   /* re-read */
            uint16_t maxI = *(uint16_t *)(sub + 0x08);
            int16_t  im;

            if (dir < 0) {
                /* Reversing: keep maxInput when reverse flag set, else zero. */
                im = (flags & 0x100u) ? (int16_t)maxI : (int16_t)0;
            } else {
                /* Forward: throttle-scaled inputMul. */
                int16_t steer = *(int16_t *)(sub + 0x00);
                im = compute_inputMul_fwd(lut, maxI, steer);
            }
            *(int16_t *)(sub + 0x02) = im;
            FUN_8002ee94(self, s1);
        }

    L_mode3_post:
        /* mat[1][1] at self+0x18. */
        if (*(int16_t *)(s + 0x18) < 0) {
            /* Flipped: rotation delta from lut bytes. */
            int32_t delta = mips_mult_lo_i32(
                mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x10),
                              (int32_t)*(uint8_t *)(lut + 0x14)), 256);
            *(uint32_t *)(s + 0x98) =
                (uint32_t)mips_addu_i32(*(int32_t *)(s + 0x98), delta);
            return;
        }

        /* Upright: update dragMass. */
        {
            uint32_t l12 = *(uint8_t *)(lut + 0x12);
            /* if (l12 >= 0xf1 || flags & 0x400): steer-proximity check. */
            if (l12 >= 0xf1u || (flags & 0x400u)) {
                int32_t st_abs = (int32_t)*(int16_t *)(sub + 0x00);
                if (st_abs < 0) st_abs = mips_subu_i32(0, st_abs);
                if ((uint32_t)st_abs < 0xaau) {
                    /* Steer near centre: override inputMul. */
                    *(int16_t *)(sub + 0x02) =
                        (int16_t)mips_mult_lo_i32(
                            mips_subu_i32(0, (int32_t)*(uint16_t *)(sub + 0x08)), 2);
                }
                /* DragMass: simple steer*2 path. */
                {
                    int16_t st  = *(int16_t *)(sub + 0x00);
                    int8_t  dir = *(int8_t  *)(sub + 0x0e);
                    int32_t dv  = mips_mult_lo_i32((int32_t)st, 2);
                    uint32_t d  = *(uint32_t *)(s + 0x94);
                    *(uint32_t *)(s + 0x94) = (dir < 0)
                        ? (uint32_t)mips_subu_i32((int32_t)d, dv)
                        : (uint32_t)mips_addu_i32((int32_t)d, dv);
                }
            } else {
                /* Full speed-scaled drag computation. */
                apply_steer_drag(s, sub);
            }
        }
        return;
    } /* end mode 3 */

    /* ==================================================================
     * MODES 4 / 5 -- player-aim directional input
     * ================================================================== */
L_mode45:
    {
        /* lut[0x12]: lateral aim joystick, 0x80 = centre, dead-band ±32. */
        {
            int32_t lat = mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x12), 0x80);
            int32_t lat_abs = (lat < 0) ? mips_subu_i32(0, lat) : lat;
            if (lat_abs > 0x20) {
                uint32_t *player = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xe0);
                /* For negative: use (lut[0x12]-0x7d) as raw; for positive: (lut[0x12]-0x80). */
                int32_t adj = (lat < 0)
                    ? mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x12), 0x7d)
                    : lat;
                *(int16_t *)((uint8_t *)player + 0x8e) =
                    (int16_t)mips_subu_i32(
                        *(int16_t *)((uint8_t *)player + 0x8e), adj >> 2);
            }
        }

        /* lut[0x13]: longitudinal aim joystick, 0x80 = centre, dead-band ±32. */
        {
            int32_t lon = mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x13), 0x80);
            int32_t lon_abs = (lon < 0) ? mips_subu_i32(0, lon) : lon;
            if (lon_abs > 0x20) {
                /* MIPS: scaled = lon * 3051 (by shift-subtract sequence):
                 *   2x+1x=3x, *64=192x, -1x=191x, *4=764x, -1x=763x,
                 *   *4=3052x, -1x=3051x.                              */
                int32_t scaled = mips_mult_lo_i32(lon, 3051);
                int32_t adj    = rtz_sra(scaled, 7);

                uint32_t *player   = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xe0);
                int32_t  pdrag     = *(int32_t *)((uint8_t *)player + 0x94);
                int32_t  speed_cap = mips_addu_i32(*(int32_t *)(s + 0x54),
                                                   *(int32_t *)(s + 0x54));
                int32_t  newdrag   = mips_addu_i32(pdrag, adj);
                if      (newdrag < speed_cap) newdrag = speed_cap;
                else if (newdrag > 0x140000 ) newdrag = 0x140000;
                *(int32_t *)((uint8_t *)player + 0x94) = newdrag;
            }
        }

        /* lut[0x10]: steer raw byte → self+0xa4 = signed(delta^2 / 24).
         * MIPS: uses magic-multiply for signed division by 24.         */
        {
            int32_t delta = mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x10), 0x80);
            int32_t dabs  = (delta < 0) ? mips_subu_i32(0, delta) : delta;
            /* signed: (|delta| * delta) / 24 -- integer truncation. */
            *(int16_t *)(s + 0xa4) = (int16_t)(mips_mult_lo_i32(dabs, delta) / 24);
        }

        /* Build s1 dispatch flags from lut[0x11] / lut[0x15] / flags. */
        uint32_t s1;
        {
            uint32_t l11 = *(uint8_t *)(lut + 0x11);
            uint32_t l15 = *(uint8_t *)(lut + 0x15);
            uint32_t hi  = 0u;
            if (l11 < 0x41u) {
                /* Both under 65: hi bit = !(l15 < 65). */
                if (l15 >= 0x41u) hi = 1u;
            }
            uint32_t masked = flags & 0x1000100u;
            if (l11 < 0x40u) masked |= 1u;
            s1 = (hi << 16) | masked;
        }

        /* ---- Compute inputMul and dispatch ---- */
        {
            uint32_t l11 = *(uint8_t *)(lut + 0x11);
            int8_t   dir  = *(int8_t  *)(sub + 0x0e);
            uint16_t maxI = *(uint16_t *)(sub + 0x08);
            int16_t  im;

            if (flags & 0x100u) {
                /* Reverse flag: full maxInput regardless of throttle. */
                im = (int16_t)maxI;
            } else {
                /* Scale by (0x80 - throttle) * maxInput / 128. */
                int32_t eff = mips_mult_lo_i32(
                    mips_subu_i32(0x80, (int32_t)l11), (int32_t)maxI);
                im = (int16_t)rtz_sra(eff, 7);
            }
            *(int16_t *)(sub + 0x02) = (dir < 0) ? (int16_t)mips_subu_i32(0, im) : im;

            int32_t speed = *(int32_t *)(s + 0x8c);
            if (speed < 0x1da && *(int16_t *)(sub + 0x02) < -0x10) {
                /* Low-speed reversing: flip dir sign. */
                *(int8_t *)(sub + 0x0e) =
                    (*(int8_t *)(sub + 0x0e) < 0) ? (int8_t)1 : (int8_t)-1;
            }
            dispatch_input(self, flags, s1);
        }

        /* ---- Post-call dragMass / rotation update ---- */
        if (*(int16_t *)(s + 0x18) < 0) {
            /* Flipped: rotation delta. */
            int32_t delta = mips_mult_lo_i32(
                mips_subu_i32((int32_t)*(uint8_t *)(lut + 0x10),
                              (int32_t)*(uint8_t *)(lut + 0x14)), 256);
            *(uint32_t *)(s + 0x98) =
                (uint32_t)mips_addu_i32(*(int32_t *)(s + 0x98), delta);
            return;
        }

        /* Upright: update dragMass. */
        if (flags & 0x400u) {
            /* Steer-force: steer*2 dragMass driver. */
            int16_t st    = *(int16_t *)(sub + 0x00);
            int32_t st_abs = (st < 0) ? mips_subu_i32(0, (int32_t)st) : (int32_t)st;
            if ((uint32_t)st_abs < 0xaau) {
                *(int16_t *)(sub + 0x02) =
                    (int16_t)mips_mult_lo_i32(
                        mips_subu_i32(0, (int32_t)*(uint16_t *)(sub + 0x08)), 2);
            }
            int8_t  dir = *(int8_t  *)(sub + 0x0e);
            int32_t dv  = mips_mult_lo_i32((int32_t)st, 2);
            uint32_t d  = *(uint32_t *)(s + 0x94);
            *(uint32_t *)(s + 0x94) = (dir < 0)
                ? (uint32_t)mips_subu_i32((int32_t)d, dv)
                : (uint32_t)mips_addu_i32((int32_t)d, dv);
        } else {
            /* Speed-scaled drag. */
            apply_steer_drag(s, sub);
        }
        return;
    } /* end modes 4/5 */
}
