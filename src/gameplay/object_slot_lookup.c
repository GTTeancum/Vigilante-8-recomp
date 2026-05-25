/* object_slot_lookup.c -- audio-slot lookup helpers for vehicle objects.
 *
 * Source: SLUS_005.10
 *   FUN_8001ab98  -- SndSlot_FindInTable(sndCfg, idx)  (12 instr, leaf)
 *   FUN_8001ac08  -- Object_InitSndSlot(self)           (17 instr)
 *
 * FUN_8001ab98 is a two-level dereference into a sound-config slot table:
 *   sndCfg->at_4  is a pointer to a slot array (each entry 4 bytes).
 *   slot[idx + 1] is a relative offset; the return value is the absolute
 *   address sndCfg->at_4 + slot[idx+1] (or NULL if either is 0).
 *
 * FUN_8001ac08 writes the result of that lookup into self+0x60 and also
 * snapshots the current frame-time byte (uRam000006cc) into self+0x46.
 *
 * Object layout (byte offsets):
 *   +0x0a  u16  kind/type index for the slot table
 *   +0x46  u16  frame-time snapshot (set from uRam000006cc)
 *   +0x58  i32  pointer to sound-config block
 *   +0x60  i32  resolved slot pointer (from SndSlot_FindInTable)
 *
 * Sound-config block layout (byte offsets):
 *   +0x04  i32  pointer to slot array (each element is an i32 offset)
 *
 * HIGH: line-for-line MIPS port of both functions (8001ab98.s, 8001ac08.s).
 * Ghidra decomps at analysis/SLUS_005.10/decomp/{8001ab98,8001ac08}.c confirm.
 */
#include <stdint.h>

/* uRam000006cc: frame-time byte from main_loop.c (gp+0x6cc). */
extern uint8_t uRam000006cc;

/* ---- FUN_8001ab98 -- SndSlot_FindInTable -------------------------------- */
/*
 * HIGH: two-level pointer dereference into a slot table.
 *
 * MIPS (leaf, no stack frame):
 *   lw  v1, 4(a0)         ; v1 = a0[4] (slot array base ptr)
 *   beq v1, 0, return_0   ; if null, return 0 (delay: a0=0)
 *   andi v0, a1, 0xffff   ; v0 = idx (u16)
 *   sll/addu v0 = a1*4    ; v0 = idx * 4
 *   addu v0, v1, v0       ; v0 = slot_array + idx*4
 *   lw  v0, 4(v0)         ; v0 = slot_array[idx + 1]
 *   beq v0, 0, return_0   ; if null, return 0
 *   addu a0, v1, v0       ; a0 = slot_array + offset
 *   jr ra; _move v0,a0    ; return a0
 */
int FUN_8001ab98(int sndCfg, uint16_t idx)
{
    int slot_base = *(int *)(uintptr_t)(sndCfg + 4);
    if (slot_base == 0) return 0;
    int offset = *(int *)(uintptr_t)(slot_base + (uint32_t)(idx & 0xffffu) * 4 + 4);
    if (offset == 0) return 0;
    return slot_base + offset;
}

/* ---- FUN_8001ac08 -- Object_InitSndSlot --------------------------------- */
/*
 * HIGH: resolve and cache the sound slot pointer for an object.
 *
 * MIPS:
 *   lw   a0, 0x58(s0)     ; a0 = self[0x58] (sndCfg ptr)
 *   lhu  a1, 0xa(s0)      ; a1 = self[0xa]  (kind)
 *   jal  FUN_8001ab98
 *   _nop
 *   lhu  v1, 0x6cc(gp)    ; v1 = uRam000006cc (frame-time byte)
 *   sw   v0, 0x60(s0)     ; self[0x60] = slot ptr
 *   sh   v1, 0x46(s0)     ; self[0x46] = frame-time snapshot
 */
void FUN_8001ac08(uint32_t *self)
{
    uint8_t *s = (uint8_t *)self;

    int      sndCfg = *(int *)(s + 0x58);
    uint16_t kind   = *(uint16_t *)(s + 0x0a);

    *(int *)    (s + 0x60) = FUN_8001ab98(sndCfg, kind);
    *(uint16_t *)(s + 0x46) = (uint16_t)uRam000006cc;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001ab98  (from analysis/SLUS_005.10/decomp/8001ab98.c) --- */
// addr: 0x8001ab98  name: FUN_8001ab98

int FUN_8001ab98(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;

  iVar2 = *(int *)(param_1 + 4);
  iVar3 = 0;
  if ((iVar2 != 0) && (iVar1 = *(int *)(iVar2 + (param_2 & 0xffff) * 4 + 4), iVar1 != 0)) {
    iVar3 = iVar2 + iVar1;
  }
  return iVar3;
}

/* --- SLUS_005.10 FUN_8001ac08  (from analysis/SLUS_005.10/decomp/8001ac08.c) --- */
// addr: 0x8001ac08  name: FUN_8001ac08

void FUN_8001ac08(int param_1)

{
  undefined2 uVar1;
  undefined4 uVar2;

  uVar2 = FUN_8001ab98(*(undefined4 *)(param_1 + 0x58),*(undefined2 *)(param_1 + 10));
  uVar1 = uRam000006cc;
  *(undefined4 *)(param_1 + 0x60) = uVar2;
  *(undefined2 *)(param_1 + 0x46) = uVar1;
  return;
}

#endif  /* GHIDRA REF */
