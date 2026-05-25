/* vehicle_scratchpad.c -- PSX scratchpad trampoline and SFX wrapper.
 *
 * Sources:
 *   FUN_8002f998  (10 instr, 32 B)  -- PSX scratchpad stack trampoline
 *   FUN_8004445c  (11 instr, 44 B)  -- SFX play with master-volume bake-in
 *
 * ============================================================
 * FUN_8002f998  (PSX scratchpad trampoline)
 *
 * On real PSX hardware this function redirects the stack pointer into
 * the 1 KB fast scratchpad RAM at 0x1F800000 before calling the 1069-
 * instruction wheel-physics body FUN_8002f9bc:
 *
 *   lui  at, 0x1f80
 *   sw   ra, 0x3f0(at)   ; save ra  to scratchpad[0x3f0]
 *   sw   sp, 0x3f4(at)   ; save sp  to scratchpad[0x3f4]
 *   jal  0x8002f9bc
 *   addi sp, at, 0x3e0   ; sp = 0x1f8003e0 (delay slot)
 *   lw   ra, 0x10(sp)    ; restore ra from new stack frame (written by callee)
 *   lw   sp, 0x14(sp)    ; restore sp from scratchpad slot
 *   jr   ra
 *
 * The scratchpad trick gives faster stack access for the hot inner loop.
 * On host we have no scratchpad; the host stack is fast enough.  Simply
 * call FUN_8002f9bc directly.
 *
 * ============================================================
 * FUN_8004445c  (SFX play with master-volume scale)
 *
 * Reads the master SFX volume (sRam000008e4, i16 q12), constructs a
 * combined volume word as `(vol << 16) | vol`, and forwards to the
 * engine's Audio_PlaySfx_inner (FUN_800443c8) with the original three
 * call-site arguments.
 *
 *   lh  a3, gp+0x8e4   ; a3 = sRam000008e4 (master SFX vol, i16)
 *   sll v0, a3, 0x10   ; v0 = a3 << 16
 *   jal FUN_800443c8
 *   addu a3, v0, a3    ; delay: a3 = (a3<<16) | a3  [vol baked into hi+lo word]
 *
 * HIGH confidence: line-by-line MIPS port of both bodies.
 */
#include <stdint.h>

/* FUN_8002f9bc: the real wheel-physics body (1069 instr).
 * Declared here; defined in vehicle_wheel_physics.c (pass-3 work). */
extern void FUN_8002f9bc(uint32_t *self);

/* FUN_800443c8: Audio_PlaySfx_inner.
 * ch      = voice / channel index
 * bank    = sound bank identifier
 * sfxId   = clip index within bank
 * volWord = packed (left_vol << 16) | right_vol  (i16 q12 each) */
extern void FUN_800443c8(int ch, uint32_t bank, int sfxId, uint32_t volWord);

/* Master SFX volume: i16 q12 at gp+0x8e4. */
extern int16_t sRam000008e4;

/* ============================================================
 * FUN_8002f998  PSX scratchpad trampoline  (10 instr)
 * ============================================================ */
void FUN_8002f998(uint32_t *self)
{
    /* PSX: saves ra/sp to scratchpad, redirects sp, calls FUN_8002f9bc,
     * then restores.  On host: the stack is just the normal stack. */
    FUN_8002f9bc(self);
}

/* ============================================================
 * FUN_8004445c  SFX play with master-volume bake-in  (11 instr)
 *
 * Callers pass (ch, bank, sfxId); this function adds the master-volume
 * word and calls Audio_PlaySfx_inner.
 * ============================================================ */
void FUN_8004445c(int ch, uint32_t bank, int sfxId)
{
    int16_t  vol     = sRam000008e4;
    /* MIPS: v0 = vol << 16; a3 = v0 + vol = (vol << 16) | vol */
    uint32_t volWord = ((uint32_t)(uint16_t)vol << 16) | (uint32_t)(uint16_t)vol;
    FUN_800443c8(ch, bank, sfxId, volWord);
}
