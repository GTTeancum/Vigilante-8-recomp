/* terrain_damage.c -- Area explosion terrain-deformation helper.
 *
 * Source: SLUS_005.10
 *   FUN_80033db4  -- TerrainDamage_Apply(x, z, radius, intensity)
 *
 * Computes the grid of cells inside the radius, looks up each cell's
 * height word in the DAT_800911a0 terrain grid, and adds a radially-
 * scaled damage value.  Then registers a delayed FUN_80020890 event.
 *
 * HIGH confidence: direct Ghidra ref port.
 */
#include <stdint.h>
#include "structs.h"

extern void   *FUN_8001d470(uint32_t size);               /* Object_AllocSmall */
extern int     FUN_80029c64(uint32_t cellX, uint32_t cellZ); /* Terrain_CellData */
extern void    FUN_800422d8(int x, int z, int radius);    /* ProximityDispatch */
extern void    FUN_80020890(void *obj, int timer);         /* EventQueue_Deferred */
extern void    Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

extern uintptr_t DAT_800911a0[];    /* terrain chunk-pointer table */

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

/* ================================================================
 * FUN_80033db4  -- TerrainDamage_Apply
 *
 * param_1: world X (fixed-point int)
 * param_2: world Z (fixed-point int)
 * param_3: radius (fixed-point int)
 * param_4: damage intensity (scaled >> 11 in division)
 * ================================================================ */
void FUN_80033db4(int param_1, int param_2, int param_3, int param_4)
{
    int16_t sVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    uint16_t *puVar7;
    uint32_t uVar8;
    uint32_t uVar9;
    uint32_t uVar10;
    int16_t *psVar11;
    uint32_t uVar12;

    /* Compute cell range (RTZ shifts): min = ceil((P - r)/0x10000),
     *                                   max = floor((P + r)/0x10000) */
    iVar2 = mips_addu_i32(mips_subu_i32(param_1, param_3), 0xffff);
    if (iVar2 < 0) {
        iVar2 = mips_addu_i32(mips_subu_i32(param_1, param_3), 0x1fffe);
    }
    iVar6 = mips_addu_i32(param_1, param_3);
    uVar9 = (uint32_t)(iVar2 >> 0x10);
    if (iVar6 < 0) {
        iVar6 = mips_addu_i32(iVar6, 0xffff);
    }

    iVar2 = mips_addu_i32(mips_subu_i32(param_2, param_3), 0xffff);
    if (iVar2 < 0) {
        iVar2 = mips_addu_i32(mips_subu_i32(param_2, param_3), 0x1fffe);
    }
    uVar8 = (uint32_t)(iVar2 >> 0x10);
    iVar2 = mips_addu_i32(param_2, param_3);
    if (iVar2 < 0) {
        iVar2 = mips_addu_i32(iVar2, 0xffff);
    }
    uVar12 = (uint32_t)(iVar2 >> 0x10);

    /* Alloc object: header (0x80 bytes) + 2 bytes per cell */
    iVar3 = (int)(uintptr_t)FUN_8001d470(
        (uint32_t)mips_addu_i32(
            mips_mult_lo_i32(
                mips_mult_lo_i32(
                    mips_addu_i32(mips_subu_i32(iVar6 >> 0x10, (int32_t)uVar9), 1),
                    mips_addu_i32(mips_subu_i32((int32_t)uVar12, (int32_t)uVar8), 1)),
                2),
            0x80));

    *(int32_t *)(uintptr_t)(iVar3 + 0x48) = param_1;
    *(int32_t *)(uintptr_t)(iVar3 + 0x50) = param_2;
    {
        extern void *LAB_80033c74;
        Object_SetCallbackPsxSlot((void *)(uintptr_t)iVar3, (uintptr_t)&LAB_80033c74);
    }

    /* iVar2 = radius_squared (in 256-unit space: (r >> 8)^2) */
    iVar2 = mips_mult_lo_i32(param_3 >> 8, param_3 >> 8);
    psVar11 = (int16_t *)(uintptr_t)(iVar3 + 0x80);
    *(int32_t *)(uintptr_t)(iVar3 + 0x54) = param_3;

    /* RTZ shift param_4 >> 11 */
    if (param_4 < 0) {
        param_4 = mips_addu_i32(param_4, 0x7ff);
    }

    /* Iterate over cells in bounding box */
    for (; uVar9 <= (uint32_t)(iVar6 >> 0x10); uVar9++) {
        if (uVar8 <= uVar12) {
            iVar4 = mips_subu_i32(mips_mult_lo_i32((int32_t)uVar9, 0x10000),
                                  param_1) >> 8;
            uVar10 = uVar8;
            do {
                iVar5 = FUN_80029c64(uVar9, uVar10);
                /* Check cell is passable (height byte == 0) and inside radius */
                if ((*(int16_t *)(uintptr_t)(iVar5 + 0x16) == 0) &&
                    (iVar5 = mips_subu_i32(mips_mult_lo_i32((int32_t)uVar10, 0x10000),
                                           param_2) >> 8,
                     iVar5 = mips_addu_i32(mips_mult_lo_i32(iVar4, iVar4),
                                           mips_mult_lo_i32(iVar5, iVar5)),
                     iVar5 <= iVar2))
                {
                    /* Access the cell's height word in the terrain grid */
                    puVar7 = (uint16_t *)(
                        (uintptr_t)DAT_800911a0[(uVar9 >> 6) * 0x20 + (uVar10 >> 6)]
                        + (uVar9 & 0x3f) * 0x80
                        + (uVar10 & 0x3f) * 2);
                    /* damage = (r_sq - dist_sq) * scale / r_sq, clamp to 11 bits */
                    sVar1 = (int16_t)(mips_mult_lo_i32(mips_subu_i32(iVar2, iVar5),
                                                       param_4 >> 0xb) / iVar2);
                    *puVar7 = (uint16_t)mips_addu_i32(*puVar7, (uint16_t)sVar1) & 0x7ffu;
                    *psVar11 = sVar1;
                } else {
                    *psVar11 = 0;
                }
                uVar10++;
                psVar11++;
            } while (uVar10 <= uVar12);
        }
    }

    FUN_800422d8(param_1, param_2, param_3);
    FUN_80020890((void *)(uintptr_t)iVar3, 0x3c);
}
