/* physics_integrate.c -- Vehicle physics integration tick.
 *
 * Implements one function:
 *   FUN_800173fc  Physics_IntegrateForces   51 instr   HIGH confidence
 *
 * PSX address: 0x800173fc
 *
 * Physics_IntegrateForces accumulates applied force/torque into the vehicle's
 * linear/angular velocity accumulators (at self+0x80 and self+0x90), then
 * integrates velocity into position (self+0x24) and updates the rotation
 * matrix (self+0x10) by applying angular velocity and re-orthonormalising.
 *
 * Vehicle object memory layout relevant here:
 *   self+0x10 : rotation matrix (5 packed uint32 = 9 i16 in PSX MATRIX format)
 *   self+0x24 : position.x  (int32)
 *   self+0x28 : position.y  (int32)
 *   self+0x2c : position.z  (int32)
 *   self+0x80 : linvel_accum.x  (int32, q-something)
 *   self+0x84 : linvel_accum.y  (int32)
 *   self+0x88 : linvel_accum.z  (int32)
 *   self+0x90 : angvel_accum.x  (int32)
 *   self+0x94 : angvel_accum.y  (int32)
 *   self+0x98 : angvel_accum.z  (int32)
 *   self+0x9c : inertia_inv.x   (int16, moment-of-inertia component)
 *   self+0x9e : inertia_inv.y   (int16)
 *   self+0xa0 : inertia_inv.z   (int16)
 *
 * Arguments:
 *   a0 = self  (vehicle object pointer, uint8_t*)
 *   a1 = force   pointer to int32[3] (linear force to apply)
 *   a2 = torque  pointer to int32[3] (angular torque to apply)
 *
 * Note: RTZ right-shift helper macro:
 *   RTZ_SRA(v, n)  -->  if (v < 0) v += (1<<n)-1; v >>= n;
 * This matches the PSX hardware sra behaviour on negative values
 * (arithmetic right shift truncates toward zero for positive, toward
 * more-negative for negative; the bias corrects to truncate toward zero).
 *
 * MIPS source: analysis/SLUS_005.10/mips/800173fc.s
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "gte.h"

/* Forward declarations for dependencies in this translation unit */
void FUN_800439b8(uint32_t *param_1, int param_2, int param_3, int param_4);
void FUN_8004c934(MATRIX *m, MATRIX *n);

/* RTZ arithmetic right shift: truncate toward zero (like C99 division). */
#define RTZ_SRA(v, n) (((v) < 0) ? (((v) + ((1 << (n)) - 1)) >> (n)) : ((v) >> (n)))

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* ==========================================================================
 * FUN_800173fc -- Physics_IntegrateForces
 * PSX address: 0x800173fc   Instruction count: 51   Confidence: HIGH
 *
 * MIPS body summary (800173fc.s):
 *
 * [Linear velocity accumulation]
 *   80017410: lw v0,0x80(s0); lw v1,0x0(a1)
 *   80017420: addu v0,v0,v1; sw v0,0x80(s0)        ; linvel[0] += force[0]
 *   80017424: lw v0,0x4(a0)  [a0=self+0x80]
 *   8001742c: lw a2,0x4(a1)  [a2 saved from a2=torque to a3 earlier]
 *   80017434: addu v0,v0,a2; sw v0,0x4(a0)          ; linvel[1] += force[1]
 *   8001743c: lw v0,0x8(a1)
 *   80017444: addu v1,v1,v0; sw v1,0x8(a0)          ; linvel[2] += force[2]
 *     [a3 = original a2 = torque pointer, a0 = self+0x80]
 *
 * [Angular velocity accumulation with inertia scaling]
 *   8001744c: lh v1,0x9c(s0); lw v0,0x0(a3)
 *   80017458: mult v0,v1      ; angdelta[0] = torque[0] * inertia_inv[0]
 *   8001745c: mflo v1
 *   80017460: bgez v1,+; addiu v1,v1,0x3f   ; RTZ bias for >>6
 *   8001746c: lw v0,0x90(s0); lh a0,0x9e(s0)
 *   80017474: sra v1,v1,0x6  ; angdelta >>= 6
 *   80017478: addu v0,v0,v1; sw v0,0x90(s0) ; angvel[0] += angdelta[0]
 *   [repeat for index 1: torque[1]*inertia[1]>>6 + angvel[1]]
 *   [repeat for index 2: torque[2]*inertia[2]>>6 + angvel[2]]
 *
 * [Integrate angular velocity -> rotation matrix]
 *   800174d0: lw a1,0x90(s0)   ; angvel_x
 *   800174dc: addiu s1,s0,0x10 ; s1 = &self->rot_matrix
 *   800174e0: bgez a1,+; addiu a1,a1,0x7f  ; RTZ bias for >>7
 *   800174e8: sra a1,a1,0x7    ; rx = angvel_x >> 7
 *   800174ec: lw a2,0x94(s0)   ; angvel_y
 *   800174f4: bgez a2,+; addiu a2,a2,0x7f
 *   800174fc: sra a2,a2,0x7    ; ry
 *   80017500: bgez v1,+; addiu v1,v1,0x7f  ; v1 = angvel_z (from sw above)
 *   80017508: sra a3,v1,0x7    ; rz
 *   8001750c: move a0,s1
 *   80017510: jal FUN_800439b8 ; RotMatrix_ApplyAngVel(rot_matrix, rx, ry, rz)
 *
 * [Integrate linear velocity -> position]
 *   80017518: lw v1,0x80(s0)   ; linvel_x
 *   80017520: bgez v1,+; addiu v1,v1,0x7f
 *   80017530: sra v1,v1,0x7    ; dx = linvel_x >> 7
 *   80017534: addu v0,v0,v1; sw v0,0x24(s0) ; pos.x += dx
 *   [a0 = self+0x84 (linvel_y)]
 *   80017538: bgez a0,+; addiu a0,a0,0x7f
 *   80017548: sra v1,a0,0x7    ; dy
 *   80017554: addu v0,v0,v1; sw v0,0x28(s0) ; pos.y += dy
 *   [a2 = self+0x88 (linvel_z)]
 *   80017558: bgez a2,+; addiu a2,a2,0x7f
 *   80017570: sra v0,a2,0x7    ; dz
 *   80017574: addu v1,v1,v0; sw v1,0x2c(s0) ; pos.z += dz
 *   80017578: jal FUN_8004c934 ; MatrixNormal(rot_matrix, rot_matrix)
 * ========================================================================== */
void FUN_800173fc(uint8_t *self, int32_t *force, int32_t *torque)
{
    int32_t v0, v1;
    int32_t a0_lv, a1_lv, a2_lv, a3_lv;

    /* s0 = self (preserved throughout) */
    /* addiu a0,s0,0x80 -- pointer to linvel accumulators */
    int32_t *linvel = (int32_t *)(self + 0x80);
    int32_t *angvel = (int32_t *)(self + 0x90);

    /* --- Linear velocity accumulation (force += linvel) ---
     * 80017410: lw v0,0x80(s0); lw v1,0x0(a1)
     * 8001741c: addu v0,v0,v1; sw v0,0x80(s0) */
    v0 = *(int32_t *)(self + 0x80);
    v1 = force[0];
    v0 = mips_addu_i32(v0, v1);
    *(int32_t *)(self + 0x80) = v0;

    /* 80017424: lw v0,0x4(a0)  [a0 = self+0x80, so self+0x84]
     * 8001742c: lw a2,0x4(a1) -- note: a2=torque is saved to a3, a2 now = force[1]
     * 80017434: addu v0,v0,a2; sw v0,0x4(a0) */
    /* In the MIPS: 'move a3,a2' at 8001742c saves torque ptr to a3.
     * But looking more carefully: a2 is the original 3rd argument (torque ptr).
     * The code does: move a3,a2 (save torque to a3), then lw a2,0x4(a1)
     * Actually at 80017428: move a3,a2 -- so a3 = torque (original a2)
     * Then 8001742c: lw a2,0x4(a1) -- a2 = force[1] */
    v0 = *(int32_t *)(self + 0x84);
    a2_lv = force[1];
    v0 = mips_addu_i32(v0, a2_lv);
    *(int32_t *)(self + 0x84) = v0;

    /* 8001743c: lw v0,0x8(a1); addu v1,v1,v0; sw v1,0x8(a0)
     * Wait: v1 was overwritten above as force[0]. After sw 0x80(s0),
     * the MIPS reuses v1. Let's re-read:
     * 80017430: lw v1,0x8(a0)  [self+0x88 = linvel[2]]
     * 8001743c: lw v0,0x8(a1)  [force[2]]
     * 80017444: addu v1,v1,v0; sw v1,0x8(a0) */
    v1 = *(int32_t *)(self + 0x88);
    v0 = force[2];
    v1 = mips_addu_i32(v1, v0);
    *(int32_t *)(self + 0x88) = v1;

    /* --- Angular velocity accumulation (torque * inertia_inv >> 6) ---
     * a3 holds the torque pointer from here on.
     * index 0: torque[0] * *(int16)(self+0x9c) >> 6 -> angvel[0] */
    {
        int16_t inertia0 = *(int16_t *)(self + 0x9c);
        int32_t torque0  = torque[0];
        int32_t delta;
        /* mult v0,v1; mflo v1 */
        delta = mips_mult_lo_i32(torque0, (int32_t)inertia0);  /* mflo */
        /* bgez v1,+; addiu v1,v1,0x3f -- RTZ right-shift by 6 */
        if (delta < 0) delta = mips_addu_i32(delta, 0x3f);
        delta >>= 6;
        /* addu v0,v0,v1; sw v0,0x90(s0) */
        v0 = *(int32_t *)(self + 0x90);
        v0 = mips_addu_i32(v0, delta);
        *(int32_t *)(self + 0x90) = v0;
    }

    /* index 1: torque[1] * *(int16)(self+0x9e) >> 6 -> angvel[1] */
    {
        int16_t inertia1 = *(int16_t *)(self + 0x9e);
        int32_t torque1  = torque[1];
        int32_t delta;
        delta = mips_mult_lo_i32(torque1, (int32_t)inertia1);
        if (delta < 0) delta = mips_addu_i32(delta, 0x3f);
        delta >>= 6;
        v0 = *(int32_t *)(self + 0x94);
        v0 = mips_addu_i32(v0, delta);
        *(int32_t *)(self + 0x94) = v0;
    }

    /* index 2: torque[2] * *(int16)(self+0xa0) >> 6 -> angvel[2]
     * 800174b0: lw v0,0x8(a3); lh a0,0xa0(s0)
     * 800174b8: mult v0,a0; mflo v0
     * 800174c0: bgez v0,+; addiu v0,v0,0x3f
     * 800174cc: lw v1,0x98(s0); sra v0,v0,0x6
     * 800174d8: addu v1,v1,v0; sw v1,0x98(s0) */
    {
        int16_t inertia2 = *(int16_t *)(self + 0xa0);
        int32_t torque2  = torque[2];
        int32_t delta;
        v0 = mips_mult_lo_i32(torque2, (int32_t)inertia2);
        if (v0 < 0) v0 = mips_addu_i32(v0, 0x3f);
        v0 >>= 6;
        v1 = *(int32_t *)(self + 0x98);
        v1 = mips_addu_i32(v1, v0);
        *(int32_t *)(self + 0x98) = v1;
        /* Note: v1 = angvel[2] (updated), used below for RTZ shift */
    }

    /* --- Integrate angular velocity into rotation matrix ---
     * a1 = angvel[0] = self+0x90
     * 800174d0: lw a1,0x90(s0)
     * 800174dc: addiu s1,s0,0x10  ; s1 = &rotation_matrix
     * 800174e0: bgez a1,+; addiu a1,a1,0x7f -- RTZ >>7
     * 800174e4: _sw v1,0x98(s0)   [delay slot of sw above]
     * 800174e8: sra a1,a1,0x7    ; rx */
    a1_lv = *(int32_t *)(self + 0x90);
    uint32_t *rot_mat = (uint32_t *)(self + 0x10);

    if (a1_lv < 0) a1_lv = mips_addu_i32(a1_lv, 0x7f);
    a1_lv >>= 7;   /* rx */

    /* 800174ec: lw a2,0x94(s0)
     * 800174f4: bgez a2,+; addiu a2,a2,0x7f -- RTZ >>7
     * 800174fc: sra a2,a2,0x7    ; ry */
    a2_lv = *(int32_t *)(self + 0x94);
    if (a2_lv < 0) a2_lv = mips_addu_i32(a2_lv, 0x7f);
    a2_lv >>= 7;   /* ry */

    /* 80017500: bgez v1,+; addiu v1,v1,0x7f -- RTZ >>7 on angvel[2]
     * v1 = updated angvel[2] from store above
     * 80017508: sra a3,v1,0x7    ; rz */
    v1 = *(int32_t *)(self + 0x98);   /* re-read (was just stored) */
    if (v1 < 0) v1 = mips_addu_i32(v1, 0x7f);
    a3_lv = v1 >> 7;   /* rz */

    int trace_matrix = 0;
    static int trace_matrix_count = 0;
    if (trace_matrix_count < 24 && getenv("V8_TRACE_INTEGRATE_MATRIX") != NULL) {
        const int16_t *m = (const int16_t *)rot_mat;
        trace_matrix = 1;
        fprintf(stderr,
                "v8: integrate_matrix before obj=%p ang=(%d,%d,%d) "
                "mat=[%d,%d,%d;%d,%d,%d;%d,%d,%d]\n",
                (void *)self, a1_lv, a2_lv, a3_lv,
                m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
    }

    /* 8001750c: move a0,s1
     * 80017510: jal FUN_800439b8  (delay slot: sra a3,v1,0x7 -> a3=rz) */
    FUN_800439b8(rot_mat, a1_lv, a2_lv, a3_lv);

    if (trace_matrix) {
        const int16_t *m = (const int16_t *)rot_mat;
        fprintf(stderr,
                "v8: integrate_matrix after_apply obj=%p "
                "mat=[%d,%d,%d;%d,%d,%d;%d,%d,%d]\n",
                (void *)self,
                m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
    }

    /* --- Integrate linear velocity into position ---
     * 80017518: lw v1,0x80(s0)   ; linvel[0]
     * 80017520: bgez v1,+; addiu v1,v1,0x7f -- RTZ >>7
     * 80017530: sra v1,v1,0x7    ; dx
     * 80017534: addu v0,v0,v1; sw v0,0x24(s0)
     *   [v0 was pos.x read? No -- v0 = *(self+0x24) ]
     *   Actually 8001752c: lw v0,0x24(s0) in the delay slot of bgez above
     *   Then 80017534: sra v1,v1,0x7 is AFTER the branch */
    v1 = *(int32_t *)(self + 0x80);
    v0 = *(int32_t *)(self + 0x24);
    if (v1 < 0) v1 = mips_addu_i32(v1, 0x7f);
    v1 >>= 7;    /* dx */
    v0 = mips_addu_i32(v0, v1);
    *(int32_t *)(self + 0x24) = v0;

    /* 80017530-80017540: lw a0,0x84(s0) (in delay slot after bgez)
     * 80017538: bgez a0,+; addiu a0,a0,0x7f
     * 80017548: lw v0,0x28(s0); sra v1,a0,0x7  ; dy
     * 80017554: addu v0,v0,v1; sw v0,0x28(s0) */
    a0_lv = *(int32_t *)(self + 0x84);
    v0 = *(int32_t *)(self + 0x28);
    if (a0_lv < 0) a0_lv = mips_addu_i32(a0_lv, 0x7f);
    v1 = a0_lv >> 7;   /* dy */
    v0 = mips_addu_i32(v0, v1);
    *(int32_t *)(self + 0x28) = v0;

    /* 80017558: bgez a2,+; addiu a2,a2,0x7f
     *   a2 = *(self+0x88) linvel[2] -- loaded in delay slot of bgez above
     * 80017564: lw v1,0x2c(s0); sra v0,a2,0x7  ; dz
     * 80017574: addu v1,v1,v0; sw v1,0x2c(s0) */
    a2_lv = *(int32_t *)(self + 0x88);
    v1 = *(int32_t *)(self + 0x2c);
    if (a2_lv < 0) a2_lv = mips_addu_i32(a2_lv, 0x7f);
    v0 = a2_lv >> 7;   /* dz */
    v1 = mips_addu_i32(v1, v0);
    *(int32_t *)(self + 0x2c) = v1;

    /* 80017568: move a0,s1  [s1 = self+0x10 = rot_mat pointer]
     * 8001756c: move a1,a0  [a1 = a0 = same pointer -> in-place]
     * 80017578: jal FUN_8004c934  ; MatrixNormal(rot_mat, rot_mat)
     * delay slot: _sw v1,0x2c(s0) -- already done above */
    FUN_8004c934((MATRIX *)(self + 0x10), (MATRIX *)(self + 0x10));
    if (trace_matrix) {
        const int16_t *m = (const int16_t *)rot_mat;
        fprintf(stderr,
                "v8: integrate_matrix after_normal obj=%p "
                "mat=[%d,%d,%d;%d,%d,%d;%d,%d,%d]\n",
                (void *)self,
                m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
        trace_matrix_count++;
    }
}

/* ==========================================================================
 * Ghidra reference: no separate decomp file for FUN_800173fc; analysis from
 * raw MIPS at analysis/SLUS_005.10/mips/800173fc.s.
 * ========================================================================== */
