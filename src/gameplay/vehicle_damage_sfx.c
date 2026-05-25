/* vehicle_damage_sfx.c -- play damage/death sound for a vehicle.
 *
 * Source: SLUS_005.10
 *   FUN_8002d44c  -- Vehicle_PlayDamageSfx(self)  (20 instr)
 *
 * Allocates a free SPU voice, looks up the clip ID in a per-state
 * table at 0x8005ec74, and plays it with master-volume scaling.
 *
 * Object fields used:
 *   self+0x58  i32  ptr to sound-config block
 *   self+0xd0  u8   current state byte (rolling = 12)
 *
 * Sound-config block layout (byte offsets):
 *   +0x08  u32  bank handle (passed to Audio_PlaySfxVol)
 *
 * DAT_8005ec74 -- 256-entry uint8_t clip-ID lookup, indexed by state.
 *   Placeholder zeros until the real table is extracted from the EXE.
 *
 * HIGH: line-for-line MIPS port of FUN_8002d44c (8002d44c.s).
 * Ghidra decomp at analysis/SLUS_005.10/decomp/8002d44c.c confirms.
 */
#include <stdint.h>

extern int  FUN_8004410c(void);                        /* Audio_AllocVoice  */
extern void FUN_8004445c(int ch, uint32_t bank, int sfxId); /* Audio_PlaySfxVol */

/* Clip-ID lookup table indexed by vehicle state byte (self+0xd0).
 * EXE address 0x8005ec74; placeholder zeros here. */
extern uint8_t DAT_8005ec74[256];

/*
 * HIGH: allocate a voice, look up clip for current state, play it.
 *
 * MIPS: jal FUN_8004410c (delay: move s0,a0)
 *       lw v1, 0x58(s0)      ; sound-config ptr
 *       lbu a0, 0xd0(s0)     ; state byte
 *       lw a1, 0x8(v1)       ; bank handle
 *       lui+addiu v1 = 0x8005ec74
 *       addu a0, a0, v1      ; &table[state]
 *       lbu a2, 0x0(a0)      ; clip = table[state]
 *       jal FUN_8004445c (delay: move a0,v0)  ; a0=voice
 */
void FUN_8002d44c(uint32_t *self)
{
    uint8_t *s = (uint8_t *)self;

    int voice = FUN_8004410c();

    uint32_t *snd_cfg = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x58);
    uint32_t  bank    = *(const uint32_t *)((const uint8_t *)snd_cfg + 8);

    uint8_t state = *(uint8_t *)(s + 0xd0);
    int     clip  = (int)DAT_8005ec74[state];

    FUN_8004445c(voice, bank, clip);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8002d44c  (from analysis/SLUS_005.10/decomp/8002d44c.c) --- */
// addr: 0x8002d44c  name: FUN_8002d44c

void FUN_8002d44c(int param_1)

{
  undefined4 uVar1;

  uVar1 = FUN_8004410c();
  FUN_8004445c(uVar1,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),
               (&DAT_8005ec74)[*(byte *)(param_1 + 0xd0)]);
  return;
}

#endif  /* GHIDRA REF */
