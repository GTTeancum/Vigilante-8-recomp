/* object_integrate.c -- core "advance object one frame" routines.
 *
 * Source: SLUS_005.10
 *   FUN_80017324  -- Object_IntegrateAndOrient(obj)
 *   FUN_800439b8  -- Object_ApplyAngularVelocity(matrix, pitchRate, yawRate, rollRate)
 *
 * These are the heart of V8's per-frame physics integration for ALL
 * movable objects (vehicles, projectiles, debris). Each tick:
 *
 *   1. Apply angular velocity to the rotation matrix in-place via GTE
 *      (Object_ApplyAngularVelocity, the second function).
 *   2. Accumulate linear velocity into the world position (1/128 scale).
 *   3. Re-orthogonalise the rotation matrix via MatrixNormal so the
 *      basis stays a proper rotation despite accumulated rounding.
 *
 * Per-object struct offsets used:
 *   +0x10 ..   MATRIX  rotation (3x3 i16 + 3x i32 pad/translation)
 *   +0x24      i32     posX                    +0x80   i32 vx
 *   +0x28      i32     posY                    +0x84   i32 vy
 *   +0x2c      i32     posZ                    +0x88   i32 vz
 *   +0x90      i32     pitchRate
 *   +0x94      i32     yawRate
 *   +0x98      i32     rollRate
 *
 * Bit-exact: every velocity component runs through the same
 * +0x7f-then-arith-shift rounding (round-toward-zero for negatives).
 * The /128 quantisation defines a "physics speed unit" of about
 * 128 ticks per world-unit-per-second.
 *
 * Object_ApplyAngularVelocity uses GTE RTIR with a "small angle"
 * approximation: it loads a virtual rotation matrix R = [1, dRoll,
 * -dYaw; ...; ...] and applies it to the current basis -- the
 * GTE saturates the output to keep the matrix in i16 range. Pass 2
 * preserves the exact saturation by going through the GTE rather
 * than computing the matmul in C.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

extern void gte_ldR11R12(uint32_t v);
extern void gte_ldR13R21(uint32_t v);
extern void gte_ldR22R23(uint32_t v);
extern void gte_ldR31R32(uint32_t v);
extern void gte_ldR33   (uint32_t v);
extern void gte_ldIR1   (int v);
extern void gte_ldIR2   (int v);
extern void gte_ldIR3   (int v);
extern void gte_rtir_b  (void);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);
extern void MatrixNormal(MATRIX *m, MATRIX *out);   /* PSY-Q libgte */

#define POS_X_OFF      0x24
#define POS_Y_OFF      0x28
#define POS_Z_OFF      0x2c
#define VEL_X_OFF      0x80
#define VEL_Y_OFF      0x84
#define VEL_Z_OFF      0x88
#define ANGVEL_X_OFF   0x90
#define ANGVEL_Y_OFF   0x94
#define ANGVEL_Z_OFF   0x98
#define ROT_MATRIX_OFF 0x10

/* HIGH: rotate `m` by (pitch, yaw, roll) angular-velocity components.
 *
 * Audit 2026-05-19: previous cleanup stopped after column 1 with note
 * "pass 3 will inline the rest". Pass 3 never ran. Completed here by
 * porting the full Ghidra ref body of FUN_800439b8 (visible in the
 * `#if 0` audit block below).
 *
 * Algorithm: R_new = R_old * R_small, where R_small is the skew-
 * symmetric small-angle rotation matrix:
 *
 *   R_small = | 1       -roll     yaw   |
 *             | roll     1        -pitch|
 *             | -yaw     pitch    1     |
 *
 * Each new column of R_new is computed by GTE matrix-vector multiply
 * with the corresponding column of R_small as the input. Results go
 * back in-place at the byte offsets that match PSY-Q MATRIX layout.
 */
void Object_ApplyAngularVelocity(uint32_t *m, int pitchRate, int yawRate, int rollRate)
{
    /* Load current rotation matrix into GTE B register. */
    gte_ldR11R12(m[0]);
    gte_ldR13R21(m[1]);
    gte_ldR22R23(m[2]);
    gte_ldR31R32(m[3]);
    gte_ldR33   (m[4]);

    int16_t *bytes = (int16_t *)m;

    /* Column 1: input vector (1, roll, -yaw). Result is new (R11, R21, R31). */
    gte_ldIR1(0x1000);
    gte_ldIR2(rollRate);
    gte_ldIR3(-yawRate);
    gte_rtir_b();
    int32_t c1_R11 = gte_stIR1();
    int32_t c1_R21 = gte_stIR2();
    int32_t c1_R31 = gte_stIR3();

    /* Column 2: input vector (-roll, 1, pitch). Result is new (R12, R22, R32). */
    gte_ldsv_(-rollRate, 0x1000, pitchRate);
    gte_rtir_b();
    /* The Ghidra ref writes col1 results into the matrix AFTER computing
     * col2's multiply but BEFORE reading col2's IR. We follow the same
     * order so any saturation side-effects line up. */
    bytes[0]  = (int16_t)c1_R11;     /* R11 @ byte 0  */
    bytes[3]  = (int16_t)c1_R21;     /* R21 @ byte 6  */
    bytes[6]  = (int16_t)c1_R31;     /* R31 @ byte 12 */
    int32_t c2_R12 = gte_stIR1();
    int32_t c2_R22 = gte_stIR2();
    int32_t c2_R32 = gte_stIR3();

    /* Column 3: input vector (yaw, -pitch, 1). Result is new (R13, R23, R33). */
    gte_ldsv_(yawRate, -pitchRate, 0x1000);
    gte_rtir_b();
    bytes[1]  = (int16_t)c2_R12;     /* R12 @ byte 2  */
    bytes[4]  = (int16_t)c2_R22;     /* R22 @ byte 8  */
    bytes[7]  = (int16_t)c2_R32;     /* R32 @ byte 14 */
    int32_t c3_R13 = gte_stIR1();
    int32_t c3_R23 = gte_stIR2();
    int32_t c3_R33 = gte_stIR3();
    bytes[2]  = (int16_t)c3_R13;     /* R13 @ byte 4  */
    bytes[5]  = (int16_t)c3_R23;     /* R23 @ byte 10 */
    bytes[8]  = (int16_t)c3_R33;     /* R33 @ byte 16 */
}

/* HIGH: full per-object frame step. */
void Object_IntegrateAndOrient(uint8_t *obj)
{
    MATRIX *m = (MATRIX *)(obj + ROT_MATRIX_OFF);

    int32_t pr = *(int32_t *)(obj + ANGVEL_X_OFF);
    int32_t yr = *(int32_t *)(obj + ANGVEL_Y_OFF);
    int32_t rr = *(int32_t *)(obj + ANGVEL_Z_OFF);
    if (pr < 0) pr += 0x7f;
    if (yr < 0) yr += 0x7f;
    if (rr < 0) rr += 0x7f;
    Object_ApplyAngularVelocity((uint32_t *)m, pr >> 7, yr >> 7, rr >> 7);

    int32_t vx = *(int32_t *)(obj + VEL_X_OFF);
    int32_t vy = *(int32_t *)(obj + VEL_Y_OFF);
    int32_t vz = *(int32_t *)(obj + VEL_Z_OFF);
    if (vx < 0) vx += 0x7f;
    if (vy < 0) vy += 0x7f;
    if (vz < 0) vz += 0x7f;
    *(int32_t *)(obj + POS_X_OFF) += (vx >> 7);
    *(int32_t *)(obj + POS_Y_OFF) += (vy >> 7);
    *(int32_t *)(obj + POS_Z_OFF) += (vz >> 7);

    MatrixNormal(m, m);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80017324  (from analysis/SLUS_005.10/decomp/80017324.c) --- */
// addr: 0x80017324  name: FUN_80017324

void FUN_80017324(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  MATRIX *m;
  
  iVar1 = *(int *)(param_1 + 0x90);
  m = (MATRIX *)(param_1 + 0x10);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  iVar2 = *(int *)(param_1 + 0x94);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  iVar3 = *(int *)(param_1 + 0x98);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0x7f;
  }
  FUN_800439b8(m,iVar1 >> 7,iVar2 >> 7,iVar3 >> 7);
  iVar1 = *(int *)(param_1 + 0x80);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  iVar2 = *(int *)(param_1 + 0x84);
  *(int *)(param_1 + 0x24) = *(int *)(param_1 + 0x24) + (iVar1 >> 7);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  iVar1 = *(int *)(param_1 + 0x88);
  *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + (iVar2 >> 7);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x7f;
  }
  *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + (iVar1 >> 7);
  MatrixNormal(m,m);
  return;
}

/* --- SLUS_005.10 FUN_800439b8  (from analysis/SLUS_005.10/decomp/800439b8.c) --- */
// addr: 0x800439b8  name: FUN_800439b8

void FUN_800439b8(undefined4 *param_1,int param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldR11R12(*param_1);
  gte_ldR13R21(param_1[1]);
  gte_ldR22R23(param_1[2]);
  gte_ldR31R32(param_1[3]);
  gte_ldR33(param_1[4]);
  gte_ldIR1(0x1000);
  gte_ldIR2(param_4);
  gte_ldIR3(-param_3);
  gte_rtir_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(-param_4,0x1000,param_2);
  gte_rtir_b();
  *(short *)param_1 = (short)uVar1;
  *(short *)((int)param_1 + 6) = (short)uVar2;
  *(short *)(param_1 + 3) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  gte_ldsv_(param_3,-param_2,0x1000);
  gte_rtir_b();
  *(short *)((int)param_1 + 2) = (short)uVar1;
  *(short *)(param_1 + 2) = (short)uVar2;
  *(short *)((int)param_1 + 0xe) = (short)uVar3;
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *(short *)(param_1 + 1) = (short)uVar1;
  *(short *)((int)param_1 + 10) = (short)uVar2;
  *(short *)(param_1 + 4) = (short)uVar3;
  return;
}

#endif  /* GHIDRA REF */
