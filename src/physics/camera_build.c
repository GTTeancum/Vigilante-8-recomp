/* camera_build.c -- per-frame terrain-aligned tilt matrix builder.
 *
 * Source: SLUS_005.10  FUN_8003e2fc.
 *
 * Called once per visible object that needs a terrain-aligned local
 * transform (camera follow target, ground vehicles, etc.).
 *
 * Steps:
 *   1. Sample terrain height at (obj.x, obj.z) -> Y.
 *   2. Walk the two obstacle chains at obj+0x74 / obj+0x78 via
 *      Object_FindObstacleAt; if a stack hits, it returns an override
 *      Y plus a surface normal (local_70/6e/6c) and we use it.
 *   3. Otherwise fall back to Terrain_NormalAt to fetch the ground
 *      normal at (x, z).
 *   4. If status bit 0x8 is clear (vehicle is "on world"), build a
 *      tilt matrix from the normal slopes -- (nx/-ny) and (nz/-ny)
 *      converted to 4.12-fixed -- compose with the parent rotation
 *      via MulMatrix0, and write to the camera matrix slot at
 *      (puVar3+1).
 *   5. Else write identity matrix + slope-into-M[1][0..2] directly.
 *
 * The +0x70 slot is a pointer to an output cell {flags, MATRIX,
 * groundX, groundY, groundZ, parentEulerY?, parentEulerZ?, ...}.
 * The 4.12 division `(nx * -0x1000) / ny` is the cot(slope) projection.
 *
 * MED. Names of puVar3 sub-fields are mechanical from offsets.
 */
#include <stdint.h>

#include "structs.h"   /* MATRIX comes from here via gte.h */

extern uint32_t Terrain_HeightAt(int32_t x, int32_t z);                    /* FUN_80025400 */
extern int      Object_FindObstacleAt(int *chain, uint32_t terrainY,
                                      int *posXyz, int16_t *normalOut);    /* FUN_8001f51c */
extern void     Terrain_NormalAt(int32_t x, int32_t z, int16_t *out);      /* FUN_80025800 */
extern void     Util_EulerToMatrix(int *eulerXyz, int *order, MATRIX *out);/* FUN_80043754 */
extern void     MulMatrix0(MATRIX *a, MATRIX *b, MATRIX *out);

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int16_t slope_q12(int16_t n, int16_t ny, int allowZeroFallback)
{
    int32_t neg = mips_subu_i32(0, (int32_t)n);
    if (ny == 0 && allowZeroFallback)
        return (int16_t)mips_sll_i32(neg, 4);
    return (int16_t)(mips_sll_i32(neg, 12) / (int32_t)ny);
}

void Camera_BuildMatrix(intptr_t self)
{
    uint32_t *out = (uint32_t *)(uintptr_t)*(uint32_t *)(self + 0x70);
    uint32_t terrainY = Terrain_HeightAt(*(int32_t *)(self + 0x24),
                                        *(int32_t *)(self + 0x2c));

    out[6] = *(uint32_t *)(self + 0x24);
    out[8] = *(uint32_t *)(self + 0x2c);

    int16_t normal[3];
    int     usedObstacle = 0;

    if (*(uint32_t *)(self + 0x74) != 0) {
        int yOverride = Object_FindObstacleAt((int *)(uintptr_t)*(uint32_t *)(self + 0x74),
                                              terrainY,
                                              (int *)(self + 0x24), normal);
        if (yOverride == 0 && *(uint32_t *)(self + 0x78) != 0) {
            yOverride = Object_FindObstacleAt((int *)(uintptr_t)*(uint32_t *)(self + 0x78),
                                              terrainY,
                                              (int *)(self + 0x24), normal);
        }
        if (yOverride != 0) {
            out[7] = (uint32_t)yOverride;
            usedObstacle = 1;
        }
    }
    if (!usedObstacle) {
        out[7] = terrainY;
        Terrain_NormalAt(*(int32_t *)(self + 0x24),
                         *(int32_t *)(self + 0x2c), normal);
    }

    int16_t *outMat = (int16_t *)(out + 1);

    uint32_t flagsPtr = out[0];
    uint16_t flags = flagsPtr ? *(uint16_t *)(uintptr_t)flagsPtr : 0;

    if ((flags & 0x8) == 0) {
        MATRIX tilt, parentRot;
        tilt.m[0][0] = 0x1000;
        tilt.m[0][1] = 0;
        tilt.m[0][2] = 0;
        tilt.m[1][1] = 0;
        tilt.m[2][0] = 0;
        tilt.m[2][1] = 0;
        tilt.m[2][2] = 0x1000;
        int16_t l70 = normal[0];
        int16_t l6e = normal[1];
        int16_t l6c = normal[2];
        tilt.m[1][0] = slope_q12(l70, l6e, 1);
        tilt.m[1][2] = slope_q12(l6c, l6e, 1);

        int euler[3];
        euler[0] = (int32_t)out[9];
        if (*(int16_t *)(self + 0x18) < 1)
            euler[0] = mips_subu_i32(0, euler[0]);
        euler[1] = 0;
        euler[2] = (int32_t)out[10];

        Util_EulerToMatrix((int *)(self + 0x10), euler, &parentRot);
        MulMatrix0(&tilt, &parentRot, (MATRIX *)outMat);
    } else {
        outMat[0] = 0x1000;        /* m[0][0] */
        outMat[1] = 0;             /* m[0][1] */
        outMat[2] = 0;             /* m[0][2] */
        outMat[3] = 0;             /* m[1][0] -- set below */
        outMat[4] = 0;             /* m[1][1] */
        outMat[5] = 0;             /* m[1][2] -- set below */
        outMat[6] = 0;             /* m[2][0] */
        outMat[7] = 0;             /* m[2][1] (one offset, but original sets) */
        outMat[8] = 0x1000;        /* m[2][2] */
        int16_t l70 = normal[0];
        int16_t l6e = normal[1];
        int16_t l6c = normal[2];
        outMat[3] = slope_q12(l70, l6e, 0);
        outMat[5] = slope_q12(l6c, l6e, 0);
    }
}

void FUN_8003e2fc(intptr_t self)
{
    Camera_BuildMatrix(self);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8003e2fc  (from analysis/SLUS_005.10/decomp/8003e2fc.c) --- */
// addr: 0x8003e2fc  name: FUN_8003e2fc

void FUN_8003e2fc(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  short local_70;
  short local_6e;
  short local_6c;
  int local_68 [4];
  MATRIX local_58;
  MATRIX MStack_38;
  
  puVar3 = *(undefined4 **)(param_1 + 0x70);
  uVar1 = FUN_80025400(*(undefined4 *)(param_1 + 0x24),*(undefined4 *)(param_1 + 0x2c));
  puVar3[6] = *(undefined4 *)(param_1 + 0x24);
  puVar3[8] = *(undefined4 *)(param_1 + 0x2c);
  if (*(int *)(param_1 + 0x74) != 0) {
    iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x74),uVar1,param_1 + 0x24,&local_70);
    if ((iVar2 != 0) ||
       ((*(int *)(param_1 + 0x78) != 0 &&
        (iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x78),uVar1,param_1 + 0x24,&local_70), iVar2 != 0))
       )) {
      puVar3[7] = iVar2;
      goto LAB_8003e3b0;
    }
  }
  puVar3[7] = uVar1;
  FUN_80025800(*(undefined4 *)(param_1 + 0x24),*(undefined4 *)(param_1 + 0x2c),&local_70);
LAB_8003e3b0:
  if ((*(ushort *)*puVar3 & 8) == 0) {
    local_58.m[0][0] = 0x1000;
    local_58.m[2][2] = 0x1000;
    local_58.m[2][1] = 0;
    local_58.m[2][0] = 0;
    local_58.m[1][1] = 0;
    local_58.m[0][2] = 0;
    local_58.m[0][1] = 0;
    if (local_6e == 0) {
      local_58.m[1][0] = local_70 * -0x10;
    }
    else {
      local_58.m[1][0] = (short)((local_70 * -0x1000) / (int)local_6e);
    }
    if (local_6e == 0) {
      local_58.m[1][2] = local_6c * -0x10;
    }
    else {
      local_58.m[1][2] = (short)((local_6c * -0x1000) / (int)local_6e);
    }
    local_68[0] = puVar3[9];
    if (*(short *)(param_1 + 0x18) < 1) {
      local_68[0] = -local_68[0];
    }
    local_68[1] = 0;
    local_68[2] = puVar3[10];
    FUN_80043754(param_1 + 0x10,local_68,&MStack_38);
    MulMatrix0(&local_58,&MStack_38,(MATRIX *)(puVar3 + 1));
  }
  else {
    *(undefined2 *)(puVar3 + 1) = 0x1000;
    *(undefined2 *)(puVar3 + 5) = 0x1000;
    *(undefined2 *)((int)puVar3 + 0x12) = 0;
    *(undefined2 *)(puVar3 + 4) = 0;
    *(undefined2 *)(puVar3 + 3) = 0;
    *(undefined2 *)(puVar3 + 2) = 0;
    *(undefined2 *)((int)puVar3 + 6) = 0;
    *(short *)((int)puVar3 + 10) = (short)((local_70 * -0x1000) / (int)local_6e);
    *(short *)((int)puVar3 + 0xe) = (short)((local_6c * -0x1000) / (int)local_6e);
  }
  return;
}

#endif  /* GHIDRA REF */
