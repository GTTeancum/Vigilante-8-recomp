/* vehicle_catchup.c -- FUN_8002d054 (255 MIPS instructions).
 *
 * Per-frame audio and gear update for damaged vehicles.  Called from
 * Vehicle_Tick (LAB_8002e2bc) when arg2 (catchupFlag) is non-zero, after
 * Vehicle_DamagedPhysics and Vehicle_EventDispatch have run.
 *
 * Source: SLUS_005.10  0x8002d054
 *
 * Register map:
 *   s0 = self
 *   s1 = stereo pan result (FUN_800446dc) -- live from 0x2d234 to end
 *   s2 = SPU voice register base (0x1f801bf0 + voice*16), reused
 *        at 0x2d3dc for the secondary voice.
 *
 * SPU writes (0x1f801bf0 family) are redirected to a scratch buffer on
 * host since that physical address is unmapped.  The audio engine is
 * out-of-scope; these writes are ignored by the host audio layer.
 *
 * HIGH confidence: all branches and delay-slot effects reproduced.
 */
#include <stdint.h>

/* --- RTZ helper (matches MIPS bgez/addiu/sra pattern). --- */
static inline int32_t rtz_sra(int32_t v, unsigned n) {
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

static inline int32_t mips_sll_i32(int32_t value, unsigned shift)
{
    return (int32_t)((uint32_t)value << (shift & 31));
}

extern uint32_t FUN_800446dc(const int32_t *xyz); /* SfxCurve_2P_Stereo */
extern void     FUN_80044574(int32_t voice, uint32_t pan);
extern uint32_t FUN_8004410c(void);               /* Audio_AllocVoice */
extern int32_t  FUN_80017160(void);               /* unknown selector */
extern void     FUN_8004483c(uint32_t voice, uint32_t bank,
                             uint32_t clip, const void *pos);
extern void     FUN_80012068(int32_t arg0, int32_t arg1,
                             int32_t arg2, int32_t arg3);
extern uint32_t uRam000005f8;                     /* audio bank handle */
extern const uint8_t  DAT_80065c28[];             /* status LUT, 24-byte stride */
extern const uint16_t DAT_8005ec68[];             /* gear pitch table (u16 per gear) */

/* PSX SPU voice registers at 0x1f801bf0 are unmapped on host.
 * Redirect all reads/writes to a local scratch buffer.
 * Layout: 24 voices × 16 bytes = 384 bytes, plus a small guard. */
static uint8_t g_spu_scratch[24 * 16 + 16];
#define SPU_SCRATCH(voice) (g_spu_scratch + (uint32_t)(voice) * 16u)

void FUN_8002d054(uint32_t *self, uint32_t caller_s1)
{
    uint8_t  *s    = (uint8_t *)self;
    uint32_t  f0   = self[0];

    /* s2 = 0x1f801bf0 + voice5*16 (SPU registers for main engine voice).
     * Redirected to scratch buffer on host. */
    uint8_t  voice5 = *(uint8_t *)(s + 5);     /* lb self+5, sign-extend → voice id */
    uint8_t *spu5   = SPU_SCRATCH(voice5);

    /* Stereo pan value, set in the main (non-0x8000000) path.
     * In the 0x8000000 path, source MIPS leaves callee s1 unchanged and
     * later stores the caller's s1 into the secondary SPU voice. */
    uint32_t pan = caller_s1;

    /* ------------------------------------------------------------------ */
    /* Flag 0x8000000: simple 3D pan update -- no pitch ramp, skip block.  */
    /* ------------------------------------------------------------------ */
    if (f0 & 0x8000000u) {
        pan = FUN_800446dc((const int32_t *)(s + 0x24));
        FUN_80044574((int32_t)(int8_t)(s[5]), pan);
        /* MIPS: j 0x8002d27c (skip pitch ramp + SPU vol section). */
        goto L_post_spu_vol;
    }

    /* ------------------------------------------------------------------ */
    /* Compute target engine pitch from gear table or input multiplier.    */
    /* ------------------------------------------------------------------ */
    {
        int32_t target_pitch;

        if (f0 & 0x100000u) {
            /*
             * Gear-shifted pitch.
             * gear byte at self+0xb2 (signed for gear level, unsigned
             * for table index).  Pitch entry = DAT_8005ec68[gear + 1].
             * Result = rtz_sra(speed * pitch_entry, 12).
             */
            int8_t  gear_s = *(int8_t  *)(s + 0xb2);
            uint8_t gear_u = *(uint8_t *)(s + 0xb2);
            int32_t speed  = *(int32_t *)(s + 0x8c);
            uint16_t pitch_entry = DAT_8005ec68[(int32_t)gear_s + 1];
            int32_t pp = rtz_sra(mips_mult_lo_i32(speed, (int32_t)pitch_entry), 12);

            /* Gear-down: pp < 0x300 AND gear_signed >= 2.
             * MIPS: slti pp,0x300; beq skip; slti gear,2; bne skip; addiu gear,-1. */
            if (pp < 0x300 && gear_s >= 2)
                *(int8_t *)(s + 0xb2) = (int8_t)(gear_u - 1u);

            /* Gear-up: pp > 0x800 AND gear_unsigned < 3 AND bit7 == 0.
             * MIPS: slti pp,0x801; bne skip; sltiu gear,3; beq skip;
             *       sll gear,24; bltz skip; addiu gear,1. */
            if (pp > 0x800) {
                gear_u = *(uint8_t *)(s + 0xb2);  /* re-read after possible decrement */
                if (gear_u < 3u && !(gear_u & 0x80u))
                    *(int8_t *)(s + 0xb2) = (int8_t)(gear_u + 1u);
            }

            /* Floor pitch at 0x300 (minimum): max(pp, 0x300). */
            target_pitch = (pp > 0x300) ? pp : 0x300;
        } else {
            /* No gear: pitch = 0x800 if inputMul > 0, else 0x300.
             * MIPS: lh sub[0x02]; blez → 0x300; delay li 0x300; li 0x800. */
            int16_t inputMul = *(int16_t *)(s + 0xa6);  /* sub[0x02] */
            target_pitch = (inputMul > 0) ? 0x800 : 0x300;
        }

        /* ---- Engine pitch ramp: step = clamp(target-current, -128, 128). ---- */
        /*
         * MIPS 0x8002d17c:
         *   lh  v0,0xd4   current signed
         *   lhu a1,0xd4   current unsigned
         *   subu a0,s1,v0  delta = target - current_signed
         *   slt v0,v1,a0  (v1=-128) → max(-128, delta)
         *   li a0,0x80
         *   slt v0,v1,a0  → if step >= 128: new = unsigned+128 (delay)
         *                   else: new = unsigned+step
         */
        int16_t  cur_s = *(int16_t  *)(s + 0xd4);
        uint16_t cur_u = *(uint16_t *)(s + 0xd4);
        int32_t  delta = mips_subu_i32(target_pitch, (int32_t)cur_s);
        int32_t  step  = (delta > -128) ? delta : -128;  /* max(-128, delta) */
        uint16_t new_pitch;
        if (step >= 128) {
            new_pitch = (uint16_t)mips_addu_i32((int32_t)cur_u, 128);
        } else {
            new_pitch = (uint16_t)mips_addu_i32((int32_t)cur_u, step);
        }
        *(uint16_t *)(s + 0xd4)  = new_pitch;
        *(int16_t  *)(spu5 + 4)  = (int16_t)new_pitch;  /* SPU voice pitch reg */
    }

    /* ---- Pitch bend from LUT flag 0x100 at lut_entry+8. ---- */
    /*
     * LUT index: idx = ~status (= ~*(i16*)(self+6)).
     * LUT entry at DAT_80065c28[idx * 24] (24-byte stride).
     * flags word at entry[+8] = lut_entry[2] (uint32 index 2).
     * MIPS: nor v1,0,status → ~status; *3; <<3 = *24; lw +8.
     */
    {
        int16_t  status    = *(int16_t *)(s + 6);
        uint32_t idx       = (uint32_t)~(int32_t)status;
        const uint32_t *lut_entry =
            (const uint32_t *)(&DAT_80065c28[idx * 24u]);
        uint32_t lut_flags = lut_entry[2];   /* +8 offset */

        int16_t bend = *(int16_t *)(s + 0xd6);
        if (lut_flags & 0x100u) {
            /* Ramp pitch bend UP toward 0x1000. */
            int32_t nb = mips_addu_i32((int32_t)bend, 0x80);
            *(int16_t *)(s + 0xd6) = (int16_t)((nb < 0x1000) ? nb : 0x1000);
        } else {
            /* Ramp pitch bend DOWN toward 0x800. */
            int32_t nb = mips_subu_i32((int32_t)bend, 0x80);
            *(int16_t *)(s + 0xd6) = (int16_t)((nb > 0x800) ? nb : 0x800);
        }
    }

    /* ---- Pan/volume: scale L and R channel by pitch bend, write to SPU. ---- */
    /*
     * MIPS 0x8002d228: s1 = FUN_800446dc result (live through rest of fn).
     * MIPS 0x8002d238: andi v0,s1,0xffff  → lo16 (unsigned).
     * MIPS 0x8002d25c: sra v0,s1,0x10     → hi16 (signed, arithmetic shift).
     * Both multiplied by pitch_bend, RTZ >>12, stored to spu5+0 and spu5+2.
     */
    {
        pan = FUN_800446dc((const int32_t *)(s + 0x24));
        int16_t bend    = *(int16_t *)(s + 0xd6);
        int32_t pan_lo  = (int32_t)(pan & 0xffffu);   /* zero-extend lo16 */
        int32_t pan_hi  = (int32_t)pan >> 16;          /* sign-extend hi16 */
        int32_t lch     = mips_mult_lo_i32(pan_lo, (int32_t)bend);
        int32_t rch     = mips_mult_lo_i32(pan_hi, (int32_t)bend);
        *(int16_t *)(spu5 + 0) = (int16_t)rtz_sra(lch, 12);
        *(int16_t *)(spu5 + 2) = (int16_t)rtz_sra(rch, 12);
    }

L_post_spu_vol:
    /* ------------------------------------------------------------------ */
    /* Flag 0x400000: collision-impact SFX + HUD flash.                    */
    /* ------------------------------------------------------------------ */
    /*
     * MIPS 0x8002d27c:
     *   lw a3,0x0(s0)  reload f0
     *   lui v0,0x40    v0 = 0x400000
     *   and v0,a3,v0
     *   beq v0,zero    skip
     */
    f0 = self[0];
    if (f0 & 0x400000u) {
        uint32_t voice_c = FUN_8004410c();
        /* mat[1][1] at self+0x18: > 0 = upright (clip 0x1d), else 0x1e. */
        int16_t mat11    = *(int16_t *)(s + 0x18);
        uint32_t clip    = (mat11 > 0) ? 0x1du : 0x1eu;
        FUN_8004483c(voice_c, uRam000005f8, clip, s + 0x24);
        /* HUD flash: arg0 = ~status, rest are severity/color params. */
        int16_t status = *(int16_t *)(s + 6);
        FUN_80012068(~(int32_t)status, 0xc0, 0, 0x40);
    }

    /* ------------------------------------------------------------------ */
    /* Flag 0x100000 + speed >= 0xbec: tire-skid SFX.                     */
    /* ------------------------------------------------------------------ */
    /*
     * Skid check: project world velocity onto vehicle forward axis (mat col2).
     *   dot = rtz_sra(vx,7)*mat02 + rtz_sra(vy,7)*mat12 + rtz_sra(vz,7)*mat22
     * If |dot| < speed*3072 (speed*3*1024): vehicle is skidding.
     * MIPS: sll speed,1; addu speed; sll speed,0xa → speed*3072.
     */
    f0 = self[0];
    if ((f0 & 0x100000u) && (*(int32_t *)(s + 0x8c) > 0xbebu)) {
        int32_t vx   = *(int32_t *)(s + 0x80);
        int32_t vy   = *(int32_t *)(s + 0x84);
        int32_t vz   = *(int32_t *)(s + 0x88);
        int16_t mat02 = *(int16_t *)(s + 0x14);
        int16_t mat12 = *(int16_t *)(s + 0x1a);
        int16_t mat22 = *(int16_t *)(s + 0x20);
        int32_t speed = *(int32_t *)(s + 0x8c);

        /* RTZ >>7 for each velocity component (MIPS bgez/addiu 0x7f/sra 7). */
        int32_t vx7 = rtz_sra(vx, 7);
        int32_t vy7 = rtz_sra(vy, 7);
        int32_t vz7 = rtz_sra(vz, 7);

        /* a0 = velX_proj + velY_proj in delay slot of bgez(vz). */
        int32_t dot = mips_addu_i32(
            mips_addu_i32(mips_mult_lo_i32(vx7, (int32_t)mat02),
                          mips_mult_lo_i32(vy7, (int32_t)mat12)),
            mips_mult_lo_i32(vz7, (int32_t)mat22));
        if (dot < 0) dot = mips_subu_i32(0, dot);    /* abs */

        int32_t threshold = mips_sll_i32(mips_addu_i32(mips_sll_i32(speed, 1), speed), 10);

        if (dot < threshold) {
            /* Skidding. */
            if (!(f0 & 0x80000u)) {
                /*
                 * Newly entering skid: set flag 0x80000.
                 * MIPS 0x8002d38c: or v0,a3,v1 (0x80000) in delay slot of
                 * bne, then sw v0 before jal FUN_8004410c.
                 */
                self[0] = f0 | 0x80000u;
                uint32_t voice_sk = FUN_8004410c();
                int32_t  sel      = FUN_80017160();
                uint32_t clip_sk  = (sel & 1u) ? 0x19u : 0x1au;
                FUN_8004483c(voice_sk, uRam000005f8, clip_sk, s + 0x24);
            }
            goto L_skid_done;   /* skip the flag-clear below */
        }
    }
    /* Not skidding (or flag check failed): clear skid flag 0x80000. */
    self[0] = self[0] & ~0x00080000u;

L_skid_done:
    /* ------------------------------------------------------------------ */
    /* Secondary rolling-surface voice (self+0xd3 voice channel).          */
    /* ------------------------------------------------------------------ */
    /*
     * MIPS 0x8002d3dc: if self[0xd3] == 0, skip.
     * s2 = 0x1f801bf0 + self[0xd3]*16 (new SPU base for this voice).
     * half_speed = rtz_sra(speed, 1).
     * if half_speed < 0x300: sw zero (silence)
     * else: sh min(half_speed, 0xc00), 0x4(s2); sw s1, 0x0(s2) (pan vol).
     *
     * Note: MIPS 'sw s1' uses the s1 register set at 0x2d234 (FUN_800446dc
     * result).  In the 0x8000000 fast-path, s1 is the caller's saved s1
     * value; Vehicle_Tick passes that value in as caller_s1.
     */
    {
        uint8_t voice_d3 = *(uint8_t *)(s + 0xd3);
        if (voice_d3 != 0u) {
            uint8_t *spu_d3  = SPU_SCRATCH(voice_d3);
            int32_t  speed   = *(int32_t *)(s + 0x8c);
            int32_t  halfspd = rtz_sra(speed, 1);

            if (halfspd < 0x300) {
                /* Silence: sw zero,0x0(s2) = zero both L and R channel volumes. */
                *(uint32_t *)spu_d3 = 0u;
            } else {
                int32_t pitch_d3 = (halfspd < 0xc00) ? halfspd : 0xc00;
                *(int16_t *)(spu_d3 + 4) = (int16_t)pitch_d3;  /* SPU pitch */
                *(uint32_t *)spu_d3       = pan;                 /* L+R vol  */
            }
        }
    }
}
