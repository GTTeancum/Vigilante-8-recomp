/* terrain_height.c -- bilinear-interpolated terrain heightmap.
 *
 * Source: SLUS_005.10  FUN_80025400.
 *
 * Vigilante 8's terrain heightmap is laid out as a 32x32 grid of
 * chunks, each chunk a 64x64 array of 11-bit height samples.
 * Addressing decomposes a (x, z) world coordinate (each a u32 in
 * 16.16-ish fixed-point, with the upper 6 bits being the chunk
 * coordinate and the next 6 the in-chunk cell):
 *
 *   chunk_x = x >> 22                 (top 10 bits -> 0..1023, masked 0x3ff)
 *   chunk_z = z >> 22
 *   cell_x  = (x >> 16) & 0x3f
 *   cell_z  = (z >> 16) & 0x3f
 *   frac_x  = x & 0xffff
 *   frac_z  = z & 0xffff
 *
 *   chunkBase = DAT_800911a0[chunk_x * 0x20 + chunk_z]
 *   h(cx,cz)  = *(u16 *)(chunkBase + cz * 2 + cx * 0x80) & 0x7ff
 *
 * The bilinear sample reads four cells (cx,cz), (cx+1,cz),
 * (cx,cz+1), (cx+1,cz+1) -- with chunk-crossing handled by
 * advancing chunk_x or chunk_z when cx+1 or cz+1 overflows 0x3f.
 *
 * The interpolation works in 16-bit fractional weights: the result is
 *
 *   h * 0x10000
 *   + frac_x * (h_xp - h)
 *   + frac_z * (h_zp - h)
 *   + (frac_x * frac_z >> 16) * (h_xpzp - h_xp - h_zp + h)
 *
 * The `if (frac_x + frac_z < 0x10000)` test selects the
 * lower-triangle (xp+zp diagonal) vs upper-triangle sample mix.
 * This is the classic split-triangle bilinear used by PSX heightmaps.
 *
 * HIGH confidence: layout is unambiguous (32x32 of 64x64 of u16); the
 * 11-bit mask is consistent across all callers.
 *
 * Pass 2 should wrap (x, z) into proper world-coord typedefs.
 */
#include <stdint.h>

#define CHUNK_GRID_DIM   32     /* 32x32 chunks                 */
#define CELL_PER_CHUNK   64     /* 64x64 cells per chunk        */
#define HEIGHT_MASK      0x7ff  /* 11-bit height value          */

extern uintptr_t DAT_800911a0[CHUNK_GRID_DIM * CHUNK_GRID_DIM];

static inline uint32_t terrain_sample(uint32_t cx, uint32_t cz)
{
    uint32_t chunkIdx = (cx >> 6) * CHUNK_GRID_DIM + (cz >> 6);
    if (chunkIdx >= CHUNK_GRID_DIM * CHUNK_GRID_DIM)
        return 0;
    uintptr_t chunkBase = DAT_800911a0[chunkIdx];
    /* On real PSX the engine guarantees a chunk pointer is set up
     * before this is called (the ZMAP+ZONE loader fills every slot,
     * and the engine never samples unloaded zones).  On host the ZMAP
     * legitimately has empty slots (oceans/sky cells) -- the engine's
     * downstream code reads height 0 for those.  Match that with a
     * NULL guard rather than dereferencing 0+off. */
    if (chunkBase == 0) return 0;
    uint32_t  off = ((cx & 0x3f) << 7) | ((cz & 0x3f) << 1);
    return *(uint16_t *)(chunkBase + off) & HEIGHT_MASK;
}

/* HIGH: sample terrain height at world (x, z), 16.16-ish.
 * Returns a 16.16 fixed-point world height (the * 0x10000 below).
 */
int32_t Terrain_HeightAt(uint32_t x, uint32_t z)
{
    uint32_t fx     = x & 0xffff;
    uint32_t fz     = z & 0xffff;
    uint32_t cx0    = x >> 16;
    uint32_t cz0    = z >> 16;
    int32_t  h00    = (int32_t)terrain_sample(cx0, cz0);

    /* Triangle split: which half of the cell are we in? */
    if (fx + fz < 0x10000u) {
        int32_t h10 = (int32_t)terrain_sample(cx0 + 1, cz0);
        int32_t h01 = (int32_t)terrain_sample(cx0,     cz0 + 1);
        /* Ghidra ref: return iVar2 >> 5  (the *0x10000 produces 16.16,
         * >>5 converts to the engine's 17.15-ish world-Y units). */
        int32_t accum = h00 * 0x10000
                      + (int32_t)fx * (h10 - h00)
                      + (int32_t)fz * (h01 - h00);
        if (accum < 0)
            accum += 0x1f;
        return accum >> 5;
    } else {
        int32_t h11 = (int32_t)terrain_sample(cx0 + 1, cz0 + 1);
        int32_t h10 = (int32_t)terrain_sample(cx0 + 1, cz0);
        int32_t h01 = (int32_t)terrain_sample(cx0,     cz0 + 1);
        /* upper triangle: bilinear with the opposite diagonal */
        int32_t accum = h11 * 0x10000
                      + (int32_t)(0x10000u - fx) * (h01 - h11)
                      + (int32_t)(0x10000u - fz) * (h10 - h11);
        if (accum < 0)
            accum += 0x1f;
        return accum >> 5;
    }
}

/* HIGH: per-cell texture material id at world (x, z). */
extern uint8_t DAT_8008f020[];   /* material palette */
/* Hex-name alias for Terrain_HeightAt (FUN_80025400). */
int32_t FUN_80025400(uint32_t x, uint32_t z) { return Terrain_HeightAt(x, z); }

uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z)
{
    uint32_t chunkIdx = (x >> 22) * CHUNK_GRID_DIM + (z >> 22);
    if (chunkIdx >= CHUNK_GRID_DIM * CHUNK_GRID_DIM)
        return &DAT_8008f020[0];
    uintptr_t chunkBase = DAT_800911a0[chunkIdx];
    if (chunkBase == 0)
        return &DAT_8008f020[0];
    uint32_t  off = ((z >> 16) & 0x3f) + ((x >> 10) & 0xfc0) + 0x2000;
    return &DAT_8008f020[ *(uint8_t *)(chunkBase + off) * 0x20 ];
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80025400  (from analysis/SLUS_005.10/decomp/80025400.c) --- */
// addr: 0x80025400  name: FUN_80025400

int FUN_80025400(uint param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  
  uVar5 = param_1 & 0xffff;
  uVar8 = param_2 & 0xffff;
  uVar7 = param_1 >> 0x10;
  uVar9 = param_2 >> 0x10;
  if (uVar5 + uVar8 < 0x10000) {
    iVar2 = (uVar9 & 0x3f) * 2;
    iVar3 = (uVar7 & 0x3f) * 0x80;
    uVar1 = *(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (param_2 >> 0x16)] + iVar2 + iVar3) & 0x7ff
    ;
    iVar2 = uVar1 * 0x10000 +
            uVar5 * ((*(ushort *)
                       ((&DAT_800911a0)[(uVar7 + 1 >> 6) * 0x20 + (param_2 >> 0x16)] +
                       iVar2 + (uVar7 + 1 & 0x3f) * 0x80) & 0x7ff) - uVar1) +
            uVar8 * ((*(ushort *)
                       ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (uVar9 + 1 >> 6)] +
                       (uVar9 + 1 & 0x3f) * 2 + iVar3) & 0x7ff) - uVar1);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0x1f;
    }
    return iVar2 >> 5;
  }
  uVar1 = uVar9 + 1 >> 6;
  uVar6 = uVar7 + 1 >> 6;
  iVar2 = (uVar9 + 1 & 0x3f) * 2;
  iVar3 = (uVar7 + 1 & 0x3f) * 0x80;
  uVar4 = *(ushort *)((&DAT_800911a0)[uVar6 * 0x20 + uVar1] + iVar2 + iVar3) & 0x7ff;
  iVar2 = uVar4 * 0x10000 +
          (0x10000 - uVar5) *
          ((*(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + uVar1] + iVar2 + (uVar7 & 0x3f) * 0x80) &
           0x7ff) - uVar4) +
          (0x10000 - uVar8) *
          ((*(ushort *)
             ((&DAT_800911a0)[uVar6 * 0x20 + (param_2 >> 0x16)] + (uVar9 & 0x3f) * 2 + iVar3) &
           0x7ff) - uVar4);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x1f;
  }
  return iVar2 >> 5;
}

/* --- SLUS_005.10 FUN_80029c64  (from analysis/SLUS_005.10/decomp/80029c64.c) --- */
// addr: 0x80029c64  name: FUN_80029c64

undefined * FUN_80029c64(uint param_1,uint param_2)

{
  return &DAT_8008f020 +
         (uint)*(byte *)((&DAT_800911a0)[(param_1 >> 6) * 0x20 + (param_2 >> 6)] +
                         (param_2 & 0x3f) + (param_1 & 0x3f) * 0x40 + 0x2000) * 0x20;
}

#endif  /* GHIDRA REF */

/* ================================================================
 * FUN_80029c64 -- Terrain_MaterialAtCell
 *
 * Cell-space variant of Terrain_MaterialAt: takes coordinates that
 * are already shifted to cell space (world >> 16).
 *   param_1 = cx (cell-space x)
 *   param_2 = cz (cell-space z)
 *
 * Returns pointer to the 32-byte material/texture record in
 * DAT_8008f020 for the terrain cell at (cx, cz).
 *
 * HIGH confidence: direct 1-liner port.
 * ================================================================ */
uint8_t *FUN_80029c64(uint32_t param_1, uint32_t param_2)
{
    uint32_t  chunkIdx = (param_1 >> 6) * CHUNK_GRID_DIM + (param_2 >> 6);
    uintptr_t chunkBase = DAT_800911a0[chunkIdx];
    uint32_t  off = (param_2 & 0x3f) + (param_1 & 0x3f) * 0x40 + 0x2000;
    return &DAT_8008f020[ *(uint8_t *)(chunkBase + off) * 0x20 ];
}
