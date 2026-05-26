/* terrain_probe.c -- combined terrain + obstacle height probe.
 *
 * Source: SLUS_005.10  FUN_8001d748, FUN_8001d840.
 *
 * Given an object and a world (x, z) point, returns the highest
 * surface y the object would stand on, considering:
 *
 *   1. The terrain heightmap (Terrain_HeightAt).
 *   2. The object's two attached obstacle chains (+0x74 and +0x78) --
 *      each is a kd-tree of trigger volumes that Object_FindObstacleAt
 *      (FUN_8001f51c) tests against and returns the override y for.
 *
 * If the obstacle test misses, optionally fills the normal (param_3)
 * via Terrain_NormalAt and the material id (param_4) via
 * Terrain_MaterialAt. If the obstacle test hits, the override y
 * displaces the terrain y entirely.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "structs.h"

extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z);
extern void  Terrain_NormalAt(uint32_t x, uint32_t z, SVECTOR *out);  /* FUN_80025648 */
extern long  VectorNormalSS(SVECTOR *a, SVECTOR *out);
extern int   Object_FindObstacleAt(int *parentObj, int terrainY, int *posXyz, int16_t *normalOut);  /* FUN_8001f51c */
extern int   TerrainMesh_ObstacleProbeAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                         int32_t terrain_y, int16_t *normalOut,
                                         int32_t *out_y);
extern void  FUN_80016fa8(int16_t *param_1, int16_t *param_2); /* Matrix_FromNormal */

/* Forward declaration (defined later in this file) */
int Terrain_HeightAndProbe(intptr_t obj, int *posXyz, SVECTOR *normalOut, uintptr_t *materialOut);

int Terrain_HeightAndProbe(intptr_t obj, int *posXyz, SVECTOR *normalOut, uintptr_t *materialOut)
{
    static int trace_cached = -1;
    static int trace_count = 0;
    if (trace_cached < 0) {
        const char *env = getenv("V8_TRACE_TERRAIN_PROBE");
        trace_cached = (env != 0 && env[0] != 0 && env[0] != '0');
    }

    int terrainY = Terrain_HeightAt((uint32_t)posXyz[0], (uint32_t)posXyz[2]);

    int yResult;
    int *chainA = (int *)(uintptr_t)*(uint32_t *)(obj + 0x74);
    if (chainA != 0
        && (yResult = Object_FindObstacleAt(chainA, terrainY, posXyz, (int16_t *)normalOut)) != 0) {
        if (materialOut != NULL) {
            *materialOut = 0;
        }
        if (trace_cached && trace_count < 240) {
            fprintf(stderr,
                    "v8: terrain_probe obj=%p pos=(0x%x,0x%x,0x%x) terrain=0x%x hitA=0x%x normal=(%d,%d,%d)\n",
                    (void *)obj, (unsigned)posXyz[0], (unsigned)posXyz[1],
                    (unsigned)posXyz[2], (unsigned)terrainY, (unsigned)yResult,
                    normalOut ? normalOut->vx : 0,
                    normalOut ? normalOut->vy : 0,
                    normalOut ? normalOut->vz : 0);
            trace_count++;
        }
        return yResult;
    }
    int *chainB = (int *)(uintptr_t)*(uint32_t *)(obj + 0x78);
    if (chainB != 0
        && (yResult = Object_FindObstacleAt(chainB, terrainY, posXyz, (int16_t *)normalOut)) != 0) {
        if (materialOut != NULL) {
            *materialOut = 0;
        }
        if (trace_cached && trace_count < 240) {
            fprintf(stderr,
                    "v8: terrain_probe obj=%p pos=(0x%x,0x%x,0x%x) terrain=0x%x hitB=0x%x normal=(%d,%d,%d)\n",
                    (void *)obj, (unsigned)posXyz[0], (unsigned)posXyz[1],
                    (unsigned)posXyz[2], (unsigned)terrainY, (unsigned)yResult,
                    normalOut ? normalOut->vx : 0,
                    normalOut ? normalOut->vy : 0,
                    normalOut ? normalOut->vz : 0);
            trace_count++;
        }
        return yResult;
    }

    if (TerrainMesh_ObstacleProbeAt(posXyz[0], posXyz[1], posXyz[2],
                                    terrainY, (int16_t *)normalOut, &yResult)) {
        if (materialOut != NULL) {
            *materialOut = 0;
        }
        if (trace_cached && trace_count < 240) {
            fprintf(stderr,
                    "v8: terrain_probe obj=%p pos=(0x%x,0x%x,0x%x) terrain=0x%x hitXOBF=0x%x normal=(%d,%d,%d)\n",
                    (void *)obj, (unsigned)posXyz[0], (unsigned)posXyz[1],
                    (unsigned)posXyz[2], (unsigned)terrainY, (unsigned)yResult,
                    normalOut ? normalOut->vx : 0,
                    normalOut ? normalOut->vy : 0,
                    normalOut ? normalOut->vz : 0);
            trace_count++;
        }
        return yResult;
    }

    /* No obstacle hit -- use terrain. Fill normal + material if requested. */
    if (normalOut != NULL) {
        Terrain_NormalAt((uint32_t)posXyz[0], (uint32_t)posXyz[2], normalOut);
        VectorNormalSS(normalOut, normalOut);
    }
    if (materialOut != NULL) {
        *materialOut = (uintptr_t)Terrain_MaterialAt(
            (uint32_t)posXyz[0], (uint32_t)posXyz[2]);
    }
    if (trace_cached && trace_count < 240) {
        uintptr_t mat = materialOut ? *materialOut : 0;
        fprintf(stderr,
                "v8: terrain_probe obj=%p pos=(0x%x,0x%x,0x%x) terrain=0x%x normal=(%d,%d,%d) mat=%p attr=(%d,%d,%d)\n",
                (void *)obj, (unsigned)posXyz[0], (unsigned)posXyz[1],
                (unsigned)posXyz[2], (unsigned)terrainY,
                normalOut ? normalOut->vx : 0,
                normalOut ? normalOut->vy : 0,
                normalOut ? normalOut->vz : 0,
                (void *)mat,
                mat ? *(int16_t *)(mat + 0x10) : 0,
                mat ? *(int16_t *)(mat + 0x12) : 0,
                mat ? *(int16_t *)(mat + 0x16) : 0);
        trace_count++;
    }
    return terrainY;
}

/* HIGH: original symbol alias for decompiled callers that have not yet been
 * renamed to Terrain_HeightAndProbe. */
int FUN_8001d748(intptr_t obj, int *posXyz, SVECTOR *normalOut, uintptr_t *materialOut)
{
    return Terrain_HeightAndProbe(obj, posXyz, normalOut, materialOut);
}

/* MED: convenience name used by several cleaned level-DLL physics ports for
 * the original FUN_8001d748 terrain/normal query. `flag` is a decompiler-era
 * placeholder; the source function's fourth argument is materialOut and those
 * callers pass 0. */
int Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *normalOut, int flag)
{
    (void)flag;
    return Terrain_HeightAndProbe((intptr_t)self, (int *)pos,
                                  (SVECTOR *)normalOut, NULL);
}

/* ================================================================
 * FUN_8001d840 -- Object_AlignToTerrainNormal
 *
 * Probes the terrain normal at the object's current position,
 * writes the terrain material and some secondary params back into
 * the destination object record (param_3), then builds a 3×3
 * orientation matrix from that normal via FUN_80016fa8.
 *
 *   param_3+0x14 = *param_2          (base param copy)
 *   param_3+0x18 = probe return val  (terrain Y / obstacle material)
 *   param_3+0x1c = param_2[2]        (secondary param copy)
 *
 * HIGH confidence: direct port.
 * ================================================================ */
void FUN_8001d840(uint32_t param_1, uint32_t *param_2, int param_3)
{
    int16_t  auStack_18[4]; /* holds the SVECTOR normal (3 × i16 + pad) */
    uint32_t uVar1 = (uint32_t)Terrain_HeightAndProbe((intptr_t)param_1, (int *)param_2,
                                                      (SVECTOR *)auStack_18, NULL);
    uint32_t uVar2 = *param_2;
    *(uint32_t *)(uintptr_t)(param_3 + 0x18) = uVar1;
    *(uint32_t *)(uintptr_t)(param_3 + 0x14) = uVar2;
    *(uint32_t *)(uintptr_t)(param_3 + 0x1c) = param_2[2];
    FUN_80016fa8((int16_t *)(uintptr_t)param_3, auStack_18);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001d748  (from analysis/SLUS_005.10/decomp/8001d748.c) --- */
// addr: 0x8001d748  name: FUN_8001d748

int FUN_8001d748(int param_1,undefined4 *param_2,SVECTOR *param_3,undefined4 *param_4)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = FUN_80025400(*param_2,param_2[2]);
  if ((*(int *)(param_1 + 0x74) == 0) ||
     ((iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x74),iVar1,param_2,param_3), iVar2 == 0 &&
      ((*(int *)(param_1 + 0x78) == 0 ||
       (iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x78),iVar1,param_2,param_3), iVar2 == 0)))))) {
    if (param_3 != (SVECTOR *)0x0) {
      FUN_80025648(*param_2,param_2[2],param_3);
      VectorNormalSS(param_3,param_3);
    }
    if (param_4 != (undefined4 *)0x0) {
      uVar3 = FUN_800255f4(*param_2,param_2[2]);
      *param_4 = uVar3;
    }
  }
  else {
    iVar1 = iVar2;
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = 0;
    }
  }
  return iVar1;
}

/* --- SLUS_005.10 FUN_8001d840  (from analysis/SLUS_005.10/decomp/8001d840.c) --- */
// addr: 0x8001d840  name: FUN_8001d840

void FUN_8001d840(undefined4 param_1,undefined4 *param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined1 auStack_18 [8];

  uVar1 = FUN_8001d748(param_1,param_2,auStack_18,0);
  uVar2 = *param_2;
  *(undefined4 *)(param_3 + 0x18) = uVar1;
  *(undefined4 *)(param_3 + 0x14) = uVar2;
  *(undefined4 *)(param_3 + 0x1c) = param_2[2];
  FUN_80016fa8(param_3,auStack_18);
  return;
}

#endif  /* GHIDRA REF */
