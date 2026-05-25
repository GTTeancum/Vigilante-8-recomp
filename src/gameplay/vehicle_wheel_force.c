/* vehicle_wheel_force.c -- FUN_8002ee94 (85 MIPS instructions).
 *
 * "Wheel-force dispatcher" -- per-tick gear-shift sound and counter
 * bookkeeping for player vehicles.  Called from Vehicle_DamagedPhysics
 * (FUN_8002efe0) with a flags word derived from the input-LUT bit fields.
 *
 * Source: SLUS_005.10  0x8002ee94
 *
 * Register map (leaf-like; 4 saved regs on a 0x20 frame):
 *   s2 = self (a0)
 *   s0 = self + 0xa4  (vehicle sub-state block)
 *   s1 = voice channel returned by FUN_8004410c, live across FUN_80017160
 *
 * Counter at self+0xb5 (= sub[0x11]):
 *   Positive   -> gear engaged, counts down toward 0 when no input.
 *   0 or neg   -> no decrement on idle.
 *   -39 (0xd9) -> set on gear-shift; counts UP each active frame toward -1,
 *                 then resets to 15 (gear-engaged state).
 *
 * HIGH confidence: all branches, loads, delay-slot effects reproduced.
 */
#include <stdint.h>

/* FUN_80017594: apply angular impulse (self, torque-arm vec, contact vec). */
extern void FUN_80017594(uint32_t *self,
                         const int32_t *vec_a,
                         const int32_t *vec_b);

/* FUN_8004410c: Audio_AllocVoice -- returns a free SPU voice channel. */
extern uint32_t FUN_8004410c(void);

/* FUN_80017160: unknown selector; LSB picks SFX clip 0x1c vs 0x1b. */
extern uint32_t FUN_80017160(void);

/* FUN_8004483c: Audio_PlaySfx3DPos(voice, bank, clip, world_pos). */
extern void FUN_8004483c(uint32_t voice, uint32_t bank,
                         uint32_t clip, const void *pos);

/* GP-relative globals. */
extern uint32_t uRam000005f8;           /* audio bank handle */

/* Angular-impulse data embedded in EXE at 0x80065748 / 0x80065754.
 * Exact sizes unknown; at minimum 3 int32 each (SVECTOR-like). */
extern const int32_t DAT_80065748[3];
extern const int32_t DAT_80065754[3];

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

void FUN_8002ee94(uint32_t *self, uint32_t flags)
{
    uint8_t *s   = (uint8_t *)self;
    uint8_t *sub = s + 0xa4;          /* vehicle sub-state block */

    /* ---- No lower-word input bits: idle -- count gear-counter down. ---- */
    if (!(flags & 0xffffu)) {
        /*
         * MIPS 0x8002efb4:
         *   lb  v0,0x11(s0)      -- signed
         *   lbu v1,0x11(s0)      -- unsigned
         *   blez v0,exit         -- if <= 0, leave counter unchanged
         *   addiu v0,v1,-1       -- (delay slot) unsigned - 1
         *   sb v0,0x11(s0)       -- store
         */
        int8_t cnt = *(int8_t *)(s + 0xb5);
        if (cnt > 0)
            *(int8_t *)(s + 0xb5) = (int8_t)mips_subu_i32((int32_t)(uint8_t)cnt, 1);
        return;
    }

    /* ---- Lower-word bits active: gear-shift SFX check. ---- */
    if ((flags & 0xffff0000u) && (self[0] & 0x100000u)) {
        /*
         * Upper-flags AND self[0]&0x100000 both set.
         * First block: gear-shift angular impulse + SFX.
         */
        int8_t cnt = *(int8_t *)(s + 0xb5);   /* sub[0x11] signed */
        if (cnt > 0) {
            int32_t speed = *(int32_t *)(s + 0x8c);
            if (speed < 0x20c8) {
                int8_t dir_b2 = *(int8_t *)(s + 0xb2); /* sub[0x0e] */
                if (dir_b2 > 0) {
                    /*
                     * MIPS 0x8002ef18--0x8002ef50:
                     *   jal FUN_80017594  (delay: a2 = 0x80065754)
                     *   li v0,-39
                     *   jal FUN_8004410c  (delay: sb -39 to sub[0x11])
                     *   jal FUN_80017160  (delay: s1 = voice from 8004410c)
                     *   ...select clip 0x1b or 0x1c based on bit 0...
                     *   jal FUN_8004483c(voice, bank, clip, self+0x24)
                     *
                     * Delay-slot order: sub[0x11]=-39 stored BEFORE
                     * FUN_8004410c executes.
                     */
                    FUN_80017594(self, DAT_80065748, DAT_80065754);
                    *(int8_t *)(s + 0xb5) = (int8_t)-39; /* 0xd9 */

                    uint32_t voice1 = FUN_8004410c();
                    uint32_t sel1   = FUN_80017160();
                    FUN_8004483c(voice1, uRam000005f8,
                                 (sel1 & 1u) ? 0x1cu : 0x1bu,
                                 s + 0x24);
                }
            }
        }

        /*
         * MIPS 0x8002ef54: second SFX block, independent of gear-shift.
         * Fires whenever speed is below the low-speed threshold.
         */
        if (*(int32_t *)(s + 0x8c) < 0x8f0) {
            uint32_t voice2 = FUN_8004410c();
            uint32_t sel2   = FUN_80017160();
            FUN_8004483c(voice2, uRam000005f8,
                         (sel2 & 1u) ? 0x1cu : 0x1bu,
                         s + 0x24);
        }
    }

    /*
     * MIPS 0x8002ef98: counter update (always reached when lower flags != 0).
     *
     *   lb  v0,0x11(s0)      -- signed
     *   lbu v1,0x11(s0)      -- unsigned
     *   slti v0,v0,-1        -- v0 = (signed < -1)
     *   bne  v0,zero,store   -- branch with delay: v0 = unsigned + 1
     *   j    store           -- fall: v0 = 15
     *
     * Result: if cnt < -1, increment unsigned byte (count toward -1);
     *         else (cnt == -1 or positive), reset to 15.
     */
    {
        int8_t  cnt_s = *(int8_t  *)(s + 0xb5);
        uint8_t cnt_u = *(uint8_t *)(s + 0xb5);
        *(int8_t *)(s + 0xb5) = (cnt_s < -1)
                               ? (int8_t)mips_addu_i32((int32_t)cnt_u, 1)
                               : (int8_t)15;
    }
}
