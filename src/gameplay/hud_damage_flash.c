/* hud_damage_flash.c -- HUD damage-flash table writer.
 *
 * Source: SLUS_005.10
 *   FUN_80012068  -- HudFlash_SetEntry(idx, b5, b6, b7)  (9 instr)
 *
 * Writes three bytes into the per-slot HUD damage-flash table at
 * 0x80065940.  Each entry is 8 bytes wide; this function sets bytes
 * [5], [6], [7] of entry idx.
 *
 * MIPS (no stack frame -- leaf):
 *   sll  a0, a0, 3         ; a0 = idx * 8
 *   lui  v0, 0x8006
 *   addiu v0, v0, 0x5940   ; v0 = 0x80065940 (table base)
 *   addu a0, a0, v0        ; a0 = &table[idx]
 *   sb   a1, 5(a0)         ; table[idx][5] = b5
 *   sb   a2, 6(a0)         ; table[idx][6] = b6
 *   jr   ra
 *   _sb  a3, 7(a0)         ; delay: table[idx][7] = b7
 *
 * HIGH: 9 instruction leaf, line-for-line MIPS port.
 */
#include <stdint.h>

/* 512-byte placeholder for the HUD damage-flash table at 0x80065940.
 * Real data is embedded in the EXE; zero-filled here.
 * Each entry is 8 bytes; 64 entries covers all vehicle slots. */
extern uint8_t DAT_80065940[512];

/*
 * HIGH: write damage-flash colour/intensity bytes into slot idx.
 */
void FUN_80012068(int idx, uint8_t b5, uint8_t b6, uint8_t b7)
{
    uint8_t *row = &DAT_80065940[(unsigned)idx * 8];
    row[5] = b5;
    row[6] = b6;
    row[7] = b7;
}

/* --- SLUS_005.10 FUN_80012028  (from analysis/SLUS_005.10/mips/80012028.s) ---
 *
 * HudFlash_WriteSlot: write bytes [4..7] of a damage-flash table slot.
 * Companion to FUN_80012068 which writes only bytes [5..7].
 *
 * Signature: FUN_80012028(idx, b4, b5, b6, b7)
 *   param_1 (a0): slot index
 *   param_2 (a1): byte to write at slot[4]
 *   param_3 (a2): byte to write at slot[5]
 *   param_4 (a3): byte to write at slot[6]
 *   param_5 (stack+0x10): byte to write at slot[7]
 *
 * MIPS (10 instructions, no stack frame despite 5th stack arg):
 *   80012028: lw  v0,0x10(sp)       ; v0 = 5th arg (b7, passed on stack)
 *   8001202c: sll a0,a0,3           ; a0 = idx * 8
 *   80012030: lui v1,0x8006
 *   80012034: addiu v1,v1,0x5940   ; v1 = DAT_80065940
 *   80012038: addu a0,a0,v1        ; a0 = &slot[0]
 *   8001203c: sb  a1,4(a0)
 *   80012040: sb  a2,5(a0)
 *   80012044: sb  a3,6(a0)
 *   80012048: jr  ra
 *   8001204c: _sb v0,7(a0)
 *
 * HIGH: line-for-line from MIPS. */
void FUN_80012028(int idx, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7)
{
    uint8_t *row = &DAT_80065940[(unsigned)idx * 8];
    row[4] = b4;
    row[5] = b5;
    row[6] = b6;
    row[7] = b7;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80012068  (from analysis/SLUS_005.10/mips/80012068.s) ---
 * 80012068: sll  a0,a0,3
 * 8001206c: lui  v0,0x8006
 * 80012070: addiu v0,v0,0x5940
 * 80012074: addu a0,a0,v0
 * 80012078: sb   a1,5(a0)
 * 8001207c: sb   a2,6(a0)
 * 80012080: jr   ra
 * 80012084: _sb  a3,7(a0)   ; delay slot
 */

#endif  /* GHIDRA REF */
