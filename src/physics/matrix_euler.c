/* matrix_euler.c -- MATRIX Euler-angle extraction and orthonormal basis builder.
 *
 * Source: SLUS_005.10
 *   FUN_80016cec  Matrix_ToEulerAngles(src, angles)   (~28 instr)
 *   FUN_80016fa8  Matrix_FromNormal(dst9, normal3)     (~35 instr)
 *
 * FUN_80016cec:
 *   Copies a raw short[13] (MATRIX layout: 9 × int16_t rotation elements
 *   + 1 int16_t pad + 3 × int32_t translation words) into a local MATRIX,
 *   then uses RotMatrixY/X and the companion helpers to extract yaw (Y),
 *   pitch (X), and roll (Z) Euler angles.  Stores:
 *     angles[0] = pitch (X rotation, FUN_80016c54)
 *     angles[1] = yaw   (Y rotation, FUN_80016c88)
 *     angles[2] = roll  (Z rotation, FUN_80016cb8)
 *   Returns angles.
 *
 * FUN_80016fa8:
 *   Builds an orthonormal 3×3 rotation matrix from a surface normal
 *   vector param_2 (int16_t[3]).
 *   Constructs two normalised basis vectors:
 *     local_18 = -(param_2.x, param_2.y, param_2.z)         [corrected normal]
 *     local_28 = (-param_2.y, param_2.x, 0)                 [tangent in XZ]
 *   Writes matrix row-vectors:
 *     dst[0,3,6]   = local_28  (tangent)
 *     dst[1,4,7]   = local_18  (corrected normal)
 *     dst[2,5,8]   = GTE OP cross product (binormal via GTE outer-product)
 *
 *   The cross product is computed via the GTE OP instruction:
 *     Load R11=local_28.vx, R22=local_28.vy, R33=local_28.vz
 *     Load IR1=local_18.vx, IR2=local_18.vy, IR3=local_18.vz
 *     Execute OP → MAC1/2/3 = cross(R-diagonal, IR)
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>
#include "gte.h"

/* PSY-Q functions */
extern long    ratan2(int y, int x);
extern MATRIX *RotMatrixY(int ry, MATRIX *m);
extern MATRIX *RotMatrixX(int rx, MATRIX *m);
extern long    VectorNormal(VECTOR *v0, VECTOR *v1);

/* FUN_80016c54 / FUN_80016c88 / FUN_80016cb8 -- angle extractors */
extern int16_t FUN_80016c54(int param_1);   /* pitch: ratan2(m+10, m+0x10) */
extern int16_t FUN_80016c88(int param_1);   /* yaw:   ratan2(m+4,  m+0x10) */
extern int16_t FUN_80016cb8(int16_t *p);    /* roll:  ratan2(p[3], p[0])   */

/* GTE register-load operations used by FUN_80016fa8.
 * gte_ldR11R12(v): pack v[15:0]=R11, v[31:16]=R12 into GTE control
 * gte_ldR22R23(v): R22, R23
 * gte_ldR33(v):    R33
 * gte_ldIR1/2/3(v): load IR1/2/3 from the integer value v
 * gte_op12(): execute GTE outer-product (OP) instruction; result in MAC1/2/3 */
extern void    gte_ldR11R12(uint32_t v);
extern void    gte_ldR22R23(uint32_t v);
extern void    gte_ldR33   (uint32_t v);
extern void    gte_ldIR1   (uint32_t v);
extern void    gte_ldIR2   (uint32_t v);
extern void    gte_ldIR3   (uint32_t v);
extern void    gte_op12    (void);
extern int32_t gte_stMAC1  (void);
extern int32_t gte_stMAC2  (void);
extern int32_t gte_stMAC3  (void);

/* ================================================================
 * FUN_80016cec -- Matrix_ToEulerAngles
 *
 * Extracts yaw, pitch, roll Euler angles from the matrix encoded in
 * param_1 (a short array with PSX MATRIX layout) and stores them
 * into param_2[0..2].  Returns param_2.
 * ================================================================ */
int16_t *FUN_80016cec(int16_t *param_1, int16_t *param_2)
{
    int16_t sVar1;
    MATRIX  local_30;

    /* Copy from short array into local MATRIX */
    local_30.m[0][0] = param_1[0];
    local_30.m[0][1] = param_1[1];
    local_30.m[0][2] = param_1[2];
    local_30.m[1][0] = param_1[3];
    local_30.m[1][1] = param_1[4];
    local_30.m[1][2] = param_1[5];
    local_30.m[2][0] = param_1[6];
    local_30.m[2][1] = param_1[7];
    local_30.m[2][2] = param_1[8];
    /* pad at param_1[9] not used */
    local_30.t[0] = *(int32_t *)(param_1 + 10);
    local_30.t[1] = *(int32_t *)(param_1 + 12);
    local_30.t[2] = *(int32_t *)(param_1 + 14);

    /* Yaw (Y axis) -- read from matrix, rotate Y out */
    sVar1 = FUN_80016c88((int)(uintptr_t)&local_30);
    param_2[1] = sVar1;
    RotMatrixY(-(int)sVar1, &local_30);

    /* Pitch (X axis) -- read from de-yawed matrix, rotate X out */
    sVar1 = FUN_80016c54((int)(uintptr_t)&local_30);
    param_2[0] = sVar1;
    RotMatrixX(-(int)sVar1, &local_30);

    /* Roll (Z axis) -- read from de-yawed de-pitched matrix */
    sVar1 = FUN_80016cb8(local_30.m[0]);
    param_2[2] = sVar1;

    return param_2;
}

/* ================================================================
 * FUN_80016fa8 -- Matrix_FromNormal
 *
 * Builds an orthonormal rotation matrix from a surface normal vector
 * param_2 (3 × int16_t).  Writes the 3×3 result to param_1 as
 * 9 × int16_t (row-major).
 *
 * local_18 = negated normal   (second row of output matrix)
 * local_28 = tangent in XZ    (first row; vz = 0 forces it horizontal)
 * binormal  = cross(local_28, local_18) via GTE OP (third row)
 * ================================================================ */
void FUN_80016fa8(int16_t *param_1, int16_t *param_2)
{
    VECTOR local_28;
    VECTOR local_18;

    /* Corrected normal (negated input) */
    local_18.vx = -(int)param_2[0];
    local_18.vy = -(int)param_2[1];
    local_18.vz = -(int)param_2[2];

    /* Tangent vector in the XZ plane perpendicular to the normal */
    local_28.vx = -(int)param_2[1];
    local_28.vy =  (int)param_2[0];
    local_28.vz =  0;

    VectorNormal(&local_28, &local_28);
    VectorNormal(&local_18, &local_18);

    /* Load the diagonal of the "R matrix" from local_28 */
    gte_ldR11R12((uint32_t)(int32_t)local_28.vx);
    gte_ldR22R23((uint32_t)(int32_t)local_28.vy);
    gte_ldR33   ((uint32_t)(int32_t)local_28.vz);

    /* Load IR vector from local_18 */
    gte_ldIR3((uint32_t)(int32_t)local_18.vz);
    gte_ldIR1((uint32_t)(int32_t)local_18.vx);
    gte_ldIR2((uint32_t)(int32_t)local_18.vy);

    /* Outer product → binormal in MAC1/2/3 */
    gte_op12();

    /* Write row 0: tangent (local_28) */
    param_1[0] = (int16_t)local_28.vx;
    param_1[3] = (int16_t)local_28.vy;
    param_1[6] = (int16_t)local_28.vz;

    /* Write row 1: corrected normal (local_18) */
    param_1[1] = (int16_t)local_18.vx;
    param_1[4] = (int16_t)local_18.vy;
    param_1[7] = (int16_t)local_18.vz;

    /* Write row 2: binormal from GTE cross product */
    param_1[2] = (int16_t)gte_stMAC1();
    param_1[5] = (int16_t)gte_stMAC2();
    param_1[8] = (int16_t)gte_stMAC3();
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80016cec  (from analysis/SLUS_005.10/decomp/80016cec.c) --- */
// addr: 0x80016cec  name: FUN_80016cec

short * FUN_80016cec(short *param_1,short *param_2)

{
  short sVar1;
  MATRIX local_30;

  local_30.m[0]._0_4_ = *(undefined4 *)param_1;
  local_30.m._4_4_ = *(undefined4 *)(param_1 + 2);
  local_30.m[1]._2_4_ = *(undefined4 *)(param_1 + 4);
  local_30.m[2]._0_4_ = *(undefined4 *)(param_1 + 6);
  local_30.m[2][2] = param_1[8];
  local_30._18_2_ = param_1[9];
  local_30.t[0] = *(long *)(param_1 + 10);
  local_30.t[1] = *(long *)(param_1 + 0xc);
  local_30.t[2] = *(long *)(param_1 + 0xe);
  sVar1 = FUN_80016c88(&local_30);
  param_2[1] = sVar1;
  RotMatrixY(-(int)sVar1,&local_30);
  sVar1 = FUN_80016c54(&local_30);
  *param_2 = sVar1;
  RotMatrixX(-(int)sVar1,&local_30);
  sVar1 = FUN_80016cb8(&local_30);
  param_2[2] = sVar1;
  return param_2;
}

/* --- SLUS_005.10 FUN_80016fa8  (from analysis/SLUS_005.10/decomp/80016fa8.c) --- */
// addr: 0x80016fa8  name: FUN_80016fa8

void FUN_80016fa8(undefined2 *param_1,short *param_2)

{
  undefined4 uVar1;
  VECTOR local_28;
  VECTOR local_18;

  local_18.vx = -(int)*param_2;
  local_18.vy = -(int)param_2[1];
  local_18.vz = -(int)param_2[2];
  local_28.vx = -(int)param_2[1];
  local_28.vy = (long)*param_2;
  local_28.vz = 0;
  VectorNormal(&local_28,&local_28);
  VectorNormal(&local_18,&local_18);
  gte_ldR11R12(local_28.vx);
  gte_ldR22R23(local_28.vy);
  gte_ldR33(local_28.vz);
  gte_ldIR3(&local_18.vz);
  gte_ldIR1(&local_18);
  gte_ldIR2(&local_18.vy);
  gte_op12();
  *param_1 = (undefined2)local_28.vx;
  param_1[3] = (undefined2)local_28.vy;
  param_1[6] = (undefined2)local_28.vz;
  param_1[1] = (undefined2)local_18.vx;
  param_1[4] = (undefined2)local_18.vy;
  param_1[7] = (undefined2)local_18.vz;
  uVar1 = gte_stMAC1();
  param_1[2] = (short)uVar1;
  uVar1 = gte_stMAC2();
  param_1[5] = (short)uVar1;
  uVar1 = gte_stMAC3();
  param_1[8] = (short)uVar1;
  return;
}

#endif  /* GHIDRA REF */
