/* vehicle_enemy_wheel.c -- FUN_8002c59c (89 MIPS instructions).
 *
 * Enemy engine-sound dispatcher.  The enemy-vehicle equivalent of
 * FUN_8002ee94 (vehicle_wheel_force.c).  Called from
 * Vehicle_DamagedPhysics (FUN_8002efe0) when self[0]&0x8000000 is set
 * (enemy / boss vehicle path).
 *
 * Source: SLUS_005.10  0x8002c59c
 *
 * Register map:
 *   s1 = self (a0)
 *   s0 = self + 0xa4  (vehicle sub-state block)
 *
 * Engine countdown at sub[0x0c] (= self+0xb0, u16):
 *   Each call with lower-flags active decrements it.
 *   On wrap (0 → 0xffff = -1 as i16): play per-vehicle rev SFX.
 *   On upper-flags active (before wrap): reload countdown to 19 or 300
 *   and play engine idle clip 0x21.
 *
 * HIGH confidence: all branches and delay-slot effects reproduced.
 */
#include <stdint.h>

extern void     FUN_800441c8(uint32_t arg);       /* Audio_VoiceStop */
extern uint32_t FUN_8004410c(void);               /* Audio_AllocVoice */
extern int32_t  FUN_80017160(void);               /* unknown selector; LSB picks SFX */
extern int      FUN_800443c8(int32_t voice, uint32_t bank,
                             uint32_t clip, uint32_t arg3);
extern int      FUN_800447e8(int32_t voice, uint32_t bank,
                             uint32_t kind, const int32_t *pos);
extern uint32_t uRam000005f8;                     /* audio bank handle */

/* Per-vehicle sound config table at PSX vaddr 0x800737a0.
 * Elements are pointers to vehicle config objects; the sound bank handle is
 * at offset +8 within each object. Host storage is native-width. */
extern uintptr_t DAT_800737a0[];

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

void FUN_8002c59c(uint32_t *self, uint32_t flags)
{
    uint8_t *s   = (uint8_t *)self;
    uint8_t *sub = s + 0xa4;

    /* ---- flags & 0xffff == 0: no input, stop engine voice. ---- */
    if (!(flags & 0xffffu)) {
        FUN_800441c8((uint32_t)(int32_t)*(int8_t *)(s + 5));
        return;
    }

    /* ---- Lower flags active: decrement engine countdown. ---- */
    /*
     * MIPS 0x8002c5c4:
     *   sh zero,0x2(s0)   -- clear sub[0x02] (inputMul)
     *   lhu v0,0xc(s0)    -- sub[0x0c] unsigned
     *   addiu v0,v0,-1    -- decrement
     *   sh v0,0xc(s0)     -- store
     *   andi v0,v0,0xffff -- keep 16-bit
     *   beq v0,0xffff     -- wrapped?
     */
    *(int16_t *)(sub + 0x02) = 0;
    uint16_t cnt = *(uint16_t *)(sub + 0x0c);
    cnt = (uint16_t)mips_subu_i32((int32_t)cnt, 1);
    *(uint16_t *)(sub + 0x0c) = cnt;

    if (cnt == 0xffffu) {
        /*
         * Countdown wrapped (was 0 → now 0xffff = -1 as signed i16):
         * play revving SFX from the per-vehicle sound config table.
         *
         * MIPS 0x8002c658-0x8002c660: self[0] flag-27 is cleared in the
         * DELAY SLOT of "beq a0,zero" -- so it ALWAYS executes regardless
         * of whether voice was zero.
         */
        self[0] &= ~0x08000000u;

        int8_t  voice = *(int8_t *)(s + 5);
        int8_t  ch;
        if (voice == 0) {
            uint32_t v = FUN_8004410c();
            *(int8_t *)(s + 5) = (int8_t)v;
            ch = (int8_t)v;
        } else {
            ch = voice;
        }

        /* Look up the per-vehicle SFX bank from the config table.
         * table[15 - status] is a PSX pointer; its object+8 is the bank. */
        int16_t  status     = *(int16_t *)(s + 6);
        int32_t  idx        = mips_subu_i32(15, (int32_t)status);
        uintptr_t entry_psx = DAT_800737a0[idx];
        uint32_t bank = *(uint32_t *)(entry_psx + 8u);
        FUN_800443c8((int32_t)ch, bank, 0u, 0u);

        /* One-shot 3D ambient sound (allocates a separate voice). */
        uint32_t v2 = FUN_8004410c();
        FUN_800447e8((int32_t)v2, uRam000005f8, 0x1fu,
                     (const int32_t *)(s + 0x24));
        return;
    }

    /* ---- Normal countdown (not yet expired): check upper flags. ---- */
    /*
     * MIPS 0x8002c5d8 (delay slot of bne): lui v0,0xffff → 0xffff0000.
     * Then: and v0,a1,v0 = flags & 0xffff0000.  If zero: return.
     */
    if (!(flags & 0xffff0000u))
        return;

    /*
     * Upper flags set: reload countdown based on FUN_80017160 result,
     * then play engine idle clip 0x21.
     *
     * MIPS 0x8002c5f0--0x8002c608:
     *   sll v1,v0,2; addu v1,v1,v0; sra v1,v1,0xf  → (v0*5) >> 15 signed.
     *   if result == 0: countdown = 0x13 (19)
     *   if result != 0: countdown = 0x12c (300)
     */
    {
        int32_t  r   = FUN_80017160();
        int32_t  sel = mips_mult_lo_i32(r, 5) >> 15;  /* signed arithmetic shift */
        *(uint16_t *)(sub + 0x0c) = (sel == 0) ? (uint16_t)0x13u
                                                : (uint16_t)0x12cu;
    }

    /* Get or allocate voice, then play engine idle SFX. */
    {
        int8_t voice = *(int8_t *)(s + 5);
        int8_t ch;
        if (voice == 0) {
            uint32_t v = FUN_8004410c();
            *(int8_t *)(s + 5) = (int8_t)v;
            ch = (int8_t)v;
        } else {
            ch = voice;
        }
        FUN_800443c8((int32_t)ch, uRam000005f8, 0x21u, 0u);
    }
}
