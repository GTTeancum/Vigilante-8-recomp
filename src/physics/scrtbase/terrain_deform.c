/* terrain_deform.c -- SCRTBASE MSilo destruction terrain deformation.
 *
 * Source: SCRTBASE.DLL  FUN_80100ca0.
 *
 * When the Secret Base's missile silo (MSilo) is destroyed, the
 * terrain around it deforms (crater). This function:
 *
 *   1. Reads the object's center (x, z) at obj+0x48 / +0x50, applies
 *      the standard +0xffff round-down for negative coords.
 *   2. Walks a 32x32 sample grid (-16..+15 around the center, +0x10
 *      cell stride) -- 0x400 (1024) cells total.
 *   3. For each cell: computes the current crater profile into obj+0x84,
 *      diffs it against the previous profile at obj+0x884, and applies
 *      the delta into the shared heightmap.
 *   4. Copies the current profile to obj+0x884, advances the 0x80
 *      frame counter, and on frame 0x101 removes the profile from the
 *      heightmap and retires the object.
 *   5. Rebuilds the radial distance profile at obj+0x1084.
 *
 * Bit-exact: the heightmap deformation happens directly in
 * DAT_800911a0's chunked grid -- the same memory Terrain_HeightAt
 * reads. Once written, the crater persists across the rest of the
 * match.
 *
 * HIGH (mechanism); MED (the exact 0x884/0x84 stride needs confirmation).
 */
#include <stdint.h>

extern uintptr_t DAT_800911a0[];
extern int16_t DAT_800607b4[];
extern long SquareRoot0(long n);
extern void FUN_800205f8(int obj);

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int16_t crater_profile_sample(uint8_t dist, int32_t counter)
{
    int32_t half = mips_addu_i32(counter, (int32_t)((uint32_t)counter >> 31)) >> 1;
    int32_t phaseCenter = mips_addu_i32(half, 0x30);
    int32_t ring = mips_subu_i32(0x80, dist);
    ring = mips_mult_lo_i32(ring, ring);

    int32_t phase = mips_sll_i32(mips_subu_i32(dist, phaseCenter), 7);
    int32_t absPhase = phase < 0 ? mips_subu_i32(0, phase) : phase;
    if (absPhase >= 0x1000)
        return 0;

    uint32_t lut = ((uint32_t)phase & 0xfffu) * 2u;
    int32_t wave = mips_mult_lo_i32(DAT_800607b4[lut], ring);
    if (wave < 0) wave = mips_addu_i32(wave, 0xfffff);
    wave >>= 20;

    int32_t edge = DAT_800607b4[lut + 1];
    if (edge < 0) edge = mips_addu_i32(edge, 0x1ff);
    edge = mips_sll_i32(mips_subu_i32(0, edge >> 9), 11);
    return (int16_t)(wave | edge);
}

static int16_t *terrain_height_cell(uint32_t cellX, uint32_t cellZ)
{
    uintptr_t chunkBase = DAT_800911a0[(cellX >> 6) * 32 + (cellZ >> 6)];
    return (int16_t *)(chunkBase + (cellX & 0x3f) * 0x80 + (cellZ & 0x3f) * 2);
}

uint32_t FUN_80100ca0(uint32_t *self, int mode, int arg)
{
    if (mode != 1) {
        if (arg != 0) {
            int32_t cx = (int32_t)self[0x12];
            int32_t cz = (int32_t)self[0x14];
            if (cx < 0) cx = mips_addu_i32(cx, 0xffff);
            if (cz < 0) cz = mips_addu_i32(cz, 0xffff);
            int32_t baseX = (cx >> 16) - 0x10;
            int32_t baseZ = (cz >> 16) - 0x10;

            for (int dx = 0; dx < 0x20; dx++) {
                uint32_t cellX = (uint32_t)mips_addu_i32(baseX, dx);
                for (int dz = 0; dz < 0x20; dz++) {
                    uint8_t *cell = (uint8_t *)self + dz * 2 + dx * 0x40;
                    int16_t oldProfile = *(int16_t *)(cell + 0x884);
                    int16_t newProfile = crater_profile_sample(*(uint8_t *)(cell + 0x1084),
                                                               (int32_t)self[0x20]);
                    *(int16_t *)(cell + 0x84) = newProfile;
                    int16_t delta = (int16_t)mips_subu_i32(newProfile, oldProfile);
                    if (delta != 0) {
                        uint32_t cellZ = (uint32_t)mips_addu_i32(baseZ, dz);
                        int16_t *hp = terrain_height_cell(cellX, cellZ);
                        *hp = (int16_t)mips_addu_i32(*hp, delta);
                    }
                }
            }

            uint32_t *src = (uint32_t *)((uint8_t *)self + 0x84);
            uint32_t *dst = (uint32_t *)((uint8_t *)self + 0x884);
            while (src != (uint32_t *)((uint8_t *)self + 0x884)) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                src += 4;
                dst += 4;
            }
        }

        self[0x20] = (uint32_t)mips_addu_i32((int32_t)self[0x20], 1);
        if (self[0x20] != 0x101)
            return 0;

        int32_t cx = (int32_t)self[0x12];
        int32_t cz = (int32_t)self[0x14];
        if (cx < 0) cx = mips_addu_i32(cx, 0xffff);
        if (cz < 0) cz = mips_addu_i32(cz, 0xffff);
        int32_t baseX = (cx >> 16) - 0x10;
        int32_t baseZ = (cz >> 16) - 0x10;

        for (int dx = 0; dx < 0x20; dx++) {
            uint32_t cellX = (uint32_t)mips_addu_i32(baseX, dx);
            for (int dz = 0; dz < 0x20; dz++) {
                int16_t profile = *(int16_t *)((uint8_t *)self + dx * 0x40 + dz * 2 + 0x884);
                if (profile != 0) {
                    uint32_t cellZ = (uint32_t)mips_addu_i32(baseZ, dz);
                    int16_t *hp = terrain_height_cell(cellX, cellZ);
                    *hp = (int16_t)mips_subu_i32(*hp, profile);
                }
            }
        }
        FUN_800205f8((int)(uintptr_t)self);
    }

    for (int x = -0x10, row = 0x10; x < 0x10; x++, row += 0x20) {
        for (int z = -0x10; z < 0x10; z++) {
            long d = SquareRoot0((long)mips_sll_i32(mips_addu_i32(
                mips_mult_lo_i32(x, x), mips_mult_lo_i32(z, z)), 6));
            *((uint8_t *)self + z + row + 0x1084) = (uint8_t)d;
        }
    }
    return 0;
}

void SB_TerrainDeform_FromCrater(int obj, int onComplete)
{
    (void)FUN_80100ca0((uint32_t *)(uintptr_t)obj, 0, onComplete);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SCRTBASE.DLL FUN_80100ca0  (from analysis/dll/SCRTBASE/decomp/80100ca0.c) --- */
// addr: 0x80100ca0  name: FUN_80100ca0

undefined4 FUN_80100ca0(int param_1,int param_2,int param_3)

{
  short sVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  short *psVar7;
  undefined4 *puVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  int iVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  
  if ((param_2 == 0) || (param_2 != 1)) {
    if (param_3 != 0) {
      iVar5 = *(int *)(param_1 + 0x48);
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xffff;
      }
      iVar9 = *(int *)(param_1 + 0x50);
      if (iVar9 < 0) {
        iVar9 = iVar9 + 0xffff;
      }
      iVar12 = 0;
      do {
        iVar11 = 0;
        uVar6 = (iVar5 >> 0x10) + -0x10 + iVar12;
        do {
          iVar3 = param_1 + iVar11 * 2 + iVar12 * 0x40;
          *(undefined2 *)(iVar3 + 0x84) = 0;
          iVar3 = -(int)*(short *)(iVar3 + 0x884);
          uVar10 = (iVar9 >> 0x10) + -0x10 + iVar11;
          if (iVar3 != 0) {
            psVar7 = (short *)(*(int *)((uVar10 >> 6) * 4 + (uVar6 >> 6) * 0x80 + -0x7ff6ee60) +
                               (uVar6 & 0x3f) * 0x80 + (uVar10 & 0x3f) * 2);
            *psVar7 = *psVar7 + (short)iVar3;
          }
          iVar11 = iVar11 + 1;
        } while (iVar11 < 0x20);
        iVar12 = iVar12 + 1;
      } while (iVar12 < 0x20);
      puVar4 = (undefined4 *)(param_1 + 0x84);
      puVar8 = (undefined4 *)(param_1 + 0x884);
      do {
        uVar13 = puVar4[1];
        uVar14 = puVar4[2];
        uVar15 = puVar4[3];
        *puVar8 = *puVar4;
        puVar8[1] = uVar13;
        puVar8[2] = uVar14;
        puVar8[3] = uVar15;
        puVar4 = puVar4 + 4;
        puVar8 = puVar8 + 4;
      } while (puVar4 != (undefined4 *)(param_1 + 0x884));
    }
    iVar5 = *(int *)(param_1 + 0x80) + 1;
    *(int *)(param_1 + 0x80) = iVar5;
    if (iVar5 != 0x101) {
      return 0;
    }
    iVar5 = *(int *)(param_1 + 0x48);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xffff;
    }
    iVar9 = *(int *)(param_1 + 0x50);
    if (iVar9 < 0) {
      iVar9 = iVar9 + 0xffff;
    }
    iVar12 = 0;
    do {
      iVar3 = 0;
      uVar6 = (iVar5 >> 0x10) + -0x10 + iVar12;
      iVar11 = iVar12 << 6;
      do {
        sVar1 = *(short *)(param_1 + iVar11 + 0x884);
        uVar10 = (iVar9 >> 0x10) + -0x10 + iVar3;
        if (sVar1 != 0) {
          psVar7 = (short *)(*(int *)((uVar10 >> 6) * 4 + (uVar6 >> 6) * 0x80 + -0x7ff6ee60) +
                             (uVar6 & 0x3f) * 0x80 + (uVar10 & 0x3f) * 2);
          *psVar7 = *psVar7 - sVar1;
        }
        iVar3 = iVar3 + 1;
        iVar11 = iVar11 + 2;
      } while (iVar3 < 0x20);
      iVar12 = iVar12 + 1;
    } while (iVar12 < 0x20);
    FUN_800205f8/*0x800205f8*/(param_1);
  }
  iVar5 = -0x10;
  iVar9 = 0x10;
  do {
    iVar11 = -0x10;
    iVar12 = 0x100;
    do {
      uVar2 = SquareRoot0/*0x8004c6e4*/((iVar5 * iVar5 + iVar12) * 0x40);
      *(undefined1 *)(param_1 + iVar11 + iVar9 + 0x1084) = uVar2;
      iVar11 = iVar11 + 1;
      iVar12 = iVar11 * iVar11;
    } while (iVar11 < 0x10);
    iVar5 = iVar5 + 1;
    iVar9 = iVar9 + 0x20;
  } while (iVar5 < 0x10);
  return 0;
}

#endif  /* GHIDRA REF */
