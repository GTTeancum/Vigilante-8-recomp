/* particle_spawn.c -- Particle-effect object allocator.
 *
 * Source: SLUS_005.10
 *   FUN_800407b4  Particle_SpawnFromPool(pool, type, pos)  (~35 instr)
 *
 * Allocates a 0x9c-byte particle object via Object_AllocSmall, copies
 * the XYZ world position from pos[], sets the per-frame tick callback
 * to LAB_80040540, initialises the bone matrix, and writes type-specific
 * flags into the object's fields.
 *
 * PSX calling-convention note: Ghidra marks the function void, but the
 * MIPS leaves v0 = FUN_8001d470's return in v0 at ret — callers use it.
 *
 * The misaligned byte writes at +0x90 / +0x93 follow the PSX SWL/SWR
 * pattern (preserved verbatim from Ghidra).
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>

extern uint32_t *FUN_8001d470(uint32_t size);   /* Object_AllocSmall */
extern void      FUN_8001d708(uint32_t *obj);   /* Object_InitBoneMatrix */
extern void      LAB_80040540(void);            /* particle tick callback */
extern void      Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* ================================================================
 * FUN_800407b4 -- Particle_SpawnFromPool
 * ================================================================ */
uint32_t *FUN_800407b4(uint32_t param_1, uint16_t param_2, uint32_t *param_3)
{
    uint32_t *puVar1;
    uint32_t *puVar2;
    uint32_t  uVar3;
    uint32_t  uVar4;

    puVar2 = FUN_8001d470(0x9c);
    *puVar2 = *puVar2 | 0xa4u;

    uVar3 = param_3[1];
    uVar4 = param_3[2];
    puVar2[0x12] = *param_3;
    puVar2[0x13] = uVar3;
    puVar2[0x14] = uVar4;
    Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_80040540);
    puVar2[0x15] = 0x10000u;
    FUN_8001d708(puVar2);
    *(uint16_t *)((uint8_t *)puVar2 + 0x82) = 7;
    puVar2[0x26] = param_1;
    *(uint16_t *)((uint8_t *)puVar2 + 0x96) = param_2;
    puVar2[0x21] = 0x100u;
    puVar2[0x22] = 0xfffffe00u;
    puVar2[0x23] = 0u;

    /* Misaligned byte write at puVar2+0x93 (PSX SWL/SWR pattern) */
    uVar3 = (uint32_t)(uintptr_t)puVar2 + 0x93u;
    uVar3 &= 3u;
    puVar1 = (uint32_t *)(((uint8_t *)puVar2 + 0x93u) - uVar3);
#pragma warning(suppress: 4293)
    *puVar1 = (*puVar1 & (uint32_t)(-1 << (int)((uVar3 + 1u) * 8u)))
              | (0x400000u >> (int)((3u - uVar3) * 8u));

    /* Misaligned word write at puVar2+0x90 (puVar2+0x24 as uint32_t*) */
    uVar3 = (uint32_t)(uintptr_t)(puVar2 + 0x24u);
    uVar3 &= 3u;
    puVar1 = (uint32_t *)((uint8_t *)(puVar2 + 0x24u) - uVar3);
    *puVar1 = (*puVar1 & (0xffffffffu >> (int)((4u - uVar3) * 8u)))
              | (0x400000u << (int)(uVar3 * 8u));

    *(uint16_t *)(puVar2 + 0x25u) = 0x20u;

    return puVar2;   /* PSX v0 pass-through; Ghidra missed the return */
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800407b4  (from analysis/SLUS_005.10/decomp/800407b4.c) --- */
// addr: 0x800407b4  name: FUN_800407b4

void FUN_800407b4(uint param_1,undefined2 param_2,uint *param_3)

{
  uint *puVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;

  puVar2 = (uint *)FUN_8001d470(0x9c);
  *puVar2 = *puVar2 | 0xa4;
  uVar3 = param_3[1];
  uVar4 = param_3[2];
  puVar2[0x12] = *param_3;
  puVar2[0x13] = uVar3;
  puVar2[0x14] = uVar4;
  puVar2[0x19] = (uint)&LAB_80040540;
  puVar2[0x15] = 0x10000;
  FUN_8001d708(puVar2);
  *(undefined2 *)((int)puVar2 + 0x82) = 7;
  puVar2[0x26] = param_1;
  *(undefined2 *)((int)puVar2 + 0x96) = param_2;
  puVar2[0x21] = 0x100;
  puVar2[0x22] = 0xfffffe00;
  puVar2[0x23] = 0;
  uVar3 = (int)puVar2 + 0x93U & 3;
  puVar1 = (uint *)(((int)puVar2 + 0x93U) - uVar3);
  *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | 0x400000U >> (3 - uVar3) * 8;
  uVar3 = (uint)(puVar2 + 0x24) & 3;
  puVar1 = (uint *)((int)(puVar2 + 0x24) - uVar3);
  *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | 0x400000 << uVar3 * 8;
  *(undefined2 *)(puVar2 + 0x25) = 0x20;
  return;
}

#endif  /* GHIDRA REF */
