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

typedef struct { int16_t m[3][3]; int32_t t[3]; } MATRIX;

extern uint32_t Terrain_HeightAt(int32_t x, int32_t z);                    /* FUN_80025400 */
extern int      Object_FindObstacleAt(int chain, uint32_t terrainY,
                                      int *posXyz, int16_t *normalOut);    /* FUN_8001f51c */
extern void     Terrain_NormalAt(int32_t x, int32_t z, int16_t *out);      /* FUN_80025800 */
extern void     Util_EulerToMatrix(int *eulerXyz, int *order, MATRIX *out);/* FUN_80043754 */
extern void     MulMatrix0(MATRIX *a, MATRIX *b, MATRIX *out);

void Camera_BuildMatrix(int self)
{
    uint32_t *out = *(uint32_t **)(self + 0x70);
    uint32_t terrainY = Terrain_HeightAt(*(int32_t *)(self + 0x24),
                                        *(int32_t *)(self + 0x2c));

    out[6] = *(uint32_t *)(self + 0x24);
    out[8] = *(uint32_t *)(self + 0x2c);

    int16_t nx, ny, nz;
    int     usedObstacle = 0;

    if (*(int *)(self + 0x74) != 0) {
        int yOverride = Object_FindObstacleAt(*(int *)(self + 0x74),
                                              terrainY,
                                              (int *)(self + 0x24), &nx);
        if (yOverride == 0 && *(int *)(self + 0x78) != 0) {
            yOverride = Object_FindObstacleAt(*(int *)(self + 0x78),
                                              terrainY,
                                              (int *)(self + 0x24), &nx);
        }
        if (yOverride != 0) {
            out[7] = (uint32_t)yOverride;
            usedObstacle = 1;
        }
    }
    if (!usedObstacle) {
        out[7] = terrainY;
        Terrain_NormalAt(*(int32_t *)(self + 0x24),
                         *(int32_t *)(self + 0x2c), &nx);
    }
    (void)ny; (void)nz;  /* loaded from stack slots adjacent to nx */

    int16_t *outMat = (int16_t *)(out + 1);

    if ((*(uint16_t *)out & 0x8) == 0) {
        MATRIX tilt, parentRot;
        tilt.m[0][0] = 0x1000;
        tilt.m[0][1] = 0;
        tilt.m[0][2] = 0;
        tilt.m[1][1] = 0;
        tilt.m[2][0] = 0;
        tilt.m[2][1] = 0;
        tilt.m[2][2] = 0x1000;
        /* nx and nz read at &nx + {1,2}; identical to local_6e/local_6c */
        int16_t l70 = *(&nx + 0);
        int16_t l6e = *(&nx + 1);
        int16_t l6c = *(&nx + 2);
        tilt.m[1][0] = (l6e == 0) ? (int16_t)(l70 * -0x10)
                                  : (int16_t)((l70 * -0x1000) / l6e);
        tilt.m[1][2] = (l6e == 0) ? (int16_t)(l6c * -0x10)
                                  : (int16_t)((l6c * -0x1000) / l6e);

        int euler[3];
        euler[0] = (int32_t)out[9];
        if (*(int16_t *)(self + 0x18) < 1) euler[0] = -euler[0];
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
        int16_t l70 = *(&nx + 0);
        int16_t l6e = *(&nx + 1);
        int16_t l6c = *(&nx + 2);
        outMat[3] = (int16_t)((l70 * -0x1000) / l6e);
        outMat[5] = (int16_t)((l6c * -0x1000) / l6e);
    }
}
