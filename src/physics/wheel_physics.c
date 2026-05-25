/* wheel_physics.c -- PSX wheel-physics body.
 *
 * Source:  SLUS_005.10  FUN_8002f9bc  (1069 MIPS instructions)
 * Address: 0x8002f9bc
 *
 * Main tick callback installed on the vehicle wheel-body object (type 0x26
 * sub-type 2).  Called once per frame with the vehicle object pointer.
 *
 * Two execution paths, selected by (short)param_1[6]:
 *   < 1  (grounded):  iterate 8 corner contact points of the vehicle AABB,
 *                     probe terrain height for each, accumulate spring/damper
 *                     forces and torque, update per-wheel spin animation.
 *   >= 1 (tumbling):  per-wheel terrain probe via scratchpad, compute
 *                     longitudinal + lateral friction forces using sin/cos of
 *                     wheel angle, spin each wheel proportionally to ground
 *                     speed, accumulate global force + torque.
 *
 * Both paths converge: apply aerodynamic drag, add gravity to Y force, call
 * Physics_IntegrateForces, decay angular velocity, decay hit-flash counters,
 * interpolate display position toward physics position.
 *
 * Confidence: HIGH (direct translation of Ghidra pseudoC; all arithmetic
 * matches PSX MIPS including RTZ-bias right-shifts and 64-bit dot product).
 *
 * Dependencies:
 *   FUN_80016a20  -- Vec3_Length (src/physics/vec_math64.c)
 *   FUN_80043408  -- RotLong_PointTranslate (src/physics/matrix_ops.c)
 *   FUN_8001d748  -- Terrain_HeightAndProbe (src/physics/terrain_probe.c)
 *   FUN_8004352c  -- RotLong_VecTranspose (src/physics/matrix_ops.c)
 *   FUN_800435c0  -- GTE_RotateWorldDelta (src/physics/gte_long_vec.c)
 *   FUN_800434d0  -- GTE_RotateNormalTranspose (src/physics/gte_long_vec.c)
 *   FUN_8001d708  -- Object_InitBoneMatrix (src/gameplay/object_bone.c)
 *   FUN_80043358  -- GTE_RotateLongMat (src/physics/gte_long_vec.c)
 *   FUN_800173fc  -- Physics_IntegrateForces (src/physics/physics_integrate.c)
 *   pcRam00000730 -- surface-type callback (platform/panic_stubs.c)
 *   iRam00000030  -- gravity constant (platform/panic_stubs.c)
 *   DAT_800607b4  -- sin/cos table (platform/panic_stubs.c)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "gte.h"

/* ---- External functions ---- */
extern int   FUN_80016a20(int32_t *v);
extern void  FUN_80043408(uint32_t *mat, uint32_t *v_in, int32_t *v_out);
extern int   Terrain_HeightAndProbe(intptr_t obj, int32_t *pos,
                                    SVECTOR *normal_out, uintptr_t *mat_out);
extern void  FUN_8004352c(uint32_t *mat, uint32_t *v_in, int32_t *v_out);
extern void  FUN_800435c0(uint32_t *mat, int32_t *v_in, int32_t *v_out);
extern void  FUN_800434d0(uint32_t *mat, int32_t *v_in, int32_t *v_out);
extern void  FUN_8001d708(uint32_t *bone);
extern void *FUN_80043358(uint32_t *mat, int32_t *v_in, int32_t *v_out);
extern void  FUN_800173fc(uint8_t *self, int32_t *force, int32_t *torque);
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

/* ---- GTE ops ---- */
extern void    gte_ldR11R12(uint32_t v);
extern void    gte_ldR22R23(uint32_t v);
extern void    gte_ldR33   (uint32_t v);
extern void    gte_ldsv_(int x, int y, int z);
extern void    gte_op12(void);
extern int32_t gte_stMAC1(void);
extern int32_t gte_stMAC2(void);
extern int32_t gte_stMAC3(void);

/* ---- RAM/data references ---- */
extern int32_t iRam00000030;   /* gravity acceleration, Q12 downward per tick */
extern int16_t DAT_800607b4[]; /* sin/cos table: [i*2]=sin, [i*2+1]=cos (Q12) */
extern void (*pcRam00000730)(void *, int, void *, unsigned int);

static int32_t psx_div_i32(int32_t numer, int32_t denom)
{
    if (denom == 0)
        return (numer < 0) ? 1 : -1;
    if ((numer == INT32_MIN) && (denom == -1))
        return INT32_MIN;
    return numer / denom;
}

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

static inline int32_t mips_mult_hi_i32(int32_t a, int32_t b)
{
    return (int32_t)(((int64_t)a * (int64_t)b) >> 32);
}

/* ---- Scratchpad replacement ----
 * PSX original uses hardware scratchpad at 0x1f800000 for 4-wheel temp data.
 * Each wheel occupies 0x18 bytes:
 *   +0x00  world_x   (from FUN_80043408 transform)
 *   +0x04  terrain_y (overwritten with Terrain_HeightAndProbe result)
 *   +0x08  world_z   (from FUN_80043408 transform)
 *   +0x0c  mat_ptr   (terrain material pointer, from probe; native-width
 *                     in the host scratch replacement)
 *   +0x10  normal    (SVECTOR terrain normal: {nx,ny,nz,pad} × int16)
 */
typedef struct {
    int32_t world_x;
    int32_t terrain_y;
    int32_t world_z;
    uintptr_t mat_ptr;
    SVECTOR normal;   /* vx=nx, vy=ny, vz=nz, pad */
} WheelScratch;

/* Host note: the PSX scratchpad record is 0x18 bytes because pointers are
 * 32-bit.  The host replacement keeps native material pointers to avoid
 * truncating module-data addresses before dereference. */

static WheelScratch g_wscr[4];

static int wheel_trace_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_WHEEL");
        cached = (env != 0 && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

static int host_heap_contains_ptr(uintptr_t p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    return p >= base && p < base + Host_HeapSize();
}

/* ================================================================
 * FUN_8002f9bc -- WheelBody_Tick
 * ================================================================ */
void FUN_8002f9bc(uint32_t *param_1)
{
    static int dbg_count = 0;
    static int trace_count = 0;
    /* Force and torque accumulators (world space) */
    int32_t local_torque[3] = {0, 0, 0};
    int32_t local_force[3] = {0, 0, 0};
#define local_108 local_torque[0]
#define local_104 local_torque[1]
#define local_100 local_torque[2]
#define local_f8 local_force[0]
#define local_f4 local_force[1]
#define local_f0 local_force[2]

    /* Per-contact / per-wheel locals */
    int32_t local_e_vec[3] = {0, 0, 0};     /* local-space velocity (tumbling) */
    int32_t local_d_vec[3] = {0, 0, 0};     /* wheel world position (temp) */
    int32_t local_c_vec[3] = {0, 0, 0};     /* per-wheel force x,y,z */
    int32_t local_b_vec[3] = {0, 0, 0};     /* grounded contact force */
    int32_t local_a_vec[3] = {0, 0, 0};     /* per-wheel world pos (from scratch) */
    int32_t local_9_vec[3] = {0, 0, 0};     /* FUN_800435c0 output (local wheel pos) */
    int32_t local_8_vec[3] = {0, 0, 0};     /* target position / contact point */
    int32_t local_probe[2] = {0, 0};        /* terrain normal packed words */
    int32_t local_7_vec[3] = {0, 0, 0};     /* FUN_800434d0 output (local normal) */
#define local_e8 local_e_vec[0]
#define local_e4 local_e_vec[1]
#define local_e0 local_e_vec[2]
#define local_d8 local_d_vec[0]
#define local_d4 local_d_vec[1]
#define local_d0 local_d_vec[2]
#define local_c8 local_c_vec[0]
#define local_c4 local_c_vec[1]
#define local_c0 local_c_vec[2]
#define local_b8 local_b_vec[0]
#define local_b4 local_b_vec[1]
#define local_b0 local_b_vec[2]
#define local_a8 local_a_vec[0]
#define local_a4 local_a_vec[1]
#define local_a0 local_a_vec[2]
#define local_98 local_9_vec[0]
#define local_94 local_9_vec[1]
#define local_90 local_9_vec[2]
#define local_88 local_8_vec[0]
#define local_84 local_8_vec[1]
#define local_80 local_8_vec[2]
#define local_78 local_probe[0]
#define local_74 local_probe[1]
#define local_70 local_7_vec[0]
#define local_6c local_7_vec[1]
#define local_68 local_7_vec[2]
    uintptr_t local_60;                    /* terrain material ptr (grounded path) */
    int32_t local_58;                      /* cos of wheel angle */
    uintptr_t local_48;                    /* terrain material ptr (tumbling path) */

    /* Misc loop / computation temporaries (matching Ghidra variable names) */
    uint32_t uVar6, uVar8, uVar9, uVar12, uVar14, uVar15, uVar16;
    int32_t  iVar4, iVar5, iVar7, iVar10, iVar11, iVar13, iVar17, iVar18,
             iVar20, iVar21, iVar23;
    int64_t  lVar1, lVar2, lVar3;
    int32_t  local_30;

    /* local_5c = pointer to param_1[0x29] (wheel parameter block) */
    uint32_t *local_5c = param_1 + 0x29;
    /* local_54 → &local_88, local_50 → &local_78, local_4c → vel (param_1+0x20) */

    /* ---- Clear status bits [22:20] ---- */
    *param_1 = *param_1 & 0xff8fffffu;

    /* ---- Update wheel-angle display references ---- */
    uintptr_t display0 = (uintptr_t)param_1[0x3f];          /* display obj 0 ptr */
    uintptr_t display1 = (uintptr_t)param_1[0x40];          /* display obj 1 ptr */
    if (0 && dbg_count < 4) {
        uintptr_t contactBox_dbg = (uintptr_t)param_1[0x17];
        fprintf(stderr,
                "v8: f9bc enter obj=%p m=(0x%x,0x%x,0x%x,0x%x,0x%x) t=(0x%x,0x%x,0x%x) display=(0x%zx,0x%zx) contact=0x%zx contact_words=(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)\n",
                (void *)param_1,
                param_1[4], param_1[5], param_1[6], param_1[7], param_1[8],
                param_1[9], param_1[10], param_1[11],
                display0, display1, contactBox_dbg,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x04) : 0,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x08) : 0,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x0c) : 0,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x10) : 0,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x14) : 0,
                contactBox_dbg ? *(uint32_t *)(contactBox_dbg + 0x18) : 0);
        dbg_count++;
    }
    uVar6  = param_1[0x29];          /* current wheel angle word */
    *(int16_t *)(display1 + 0x42) = (int16_t)uVar6;
    *(int16_t *)(display0 + 0x42) = (int16_t)uVar6;
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc display angle updated angle=0x%x\n", uVar6);

    /* ---- Compute normalised speed ---- */
    iVar4 = FUN_80016a20((int32_t *)(param_1 + 0x20));
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x7f);
    param_1[0x23] = (uint32_t)(iVar4 >> 7);
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc speed=0x%x branch_word6=0x%x branch_i16=%d\n",
                (unsigned)iVar4, (unsigned)param_1[6], (int)(int16_t)param_1[6]);

    /* ================================================================
     * GROUNDED PATH  (status < 1)
     * ================================================================ */
    if ((int16_t)param_1[6] < 1) {

        uintptr_t contactBox = (uintptr_t)param_1[0x17];  /* contact-point bounding box base pointer */
        uVar8  = 0;
        uVar14 = 0;

        /* Loop 8 corners of the AABB (uVar8 bits 0/1/2 → X/Y/Z vertex) */
        do {
            if (0 && dbg_count <= 4)
                fprintf(stderr, "v8: f9bc grounded corner=%u\n", (unsigned)uVar8);
            /* Select X coordinate */
            if (uVar14 == 0)
                local_88 = *(int32_t *)(contactBox + 0x10);
            else
                local_88 = *(int32_t *)(contactBox + 0x04);

            /* Select Y coordinate */
            if ((uVar8 & 2) == 0)
                local_84 = *(int32_t *)(contactBox + 0x14);
            else
                local_84 = *(int32_t *)(contactBox + 0x08);

            /* Select Z coordinate */
            if ((uVar8 & 4) == 0)
                local_80 = *(int32_t *)(contactBox + 0x18);
            else
                local_80 = *(int32_t *)(contactBox + 0x0c);

            /* Transform contact point to world space */
            FUN_80043408(param_1 + 4, (uint32_t *)&local_88, &local_88);
            if (0 && dbg_count <= 4)
                fprintf(stderr, "v8: f9bc corner world=(0x%x,0x%x,0x%x)\n",
                        (unsigned)local_88, (unsigned)local_84, (unsigned)local_80);

            /* Terrain height probe (no normal, store material in local_60) */
                iVar4 = Terrain_HeightAndProbe((intptr_t)param_1,
                                           &local_88, NULL,
                                           &local_60);
            if (0 && dbg_count <= 4)
                fprintf(stderr, "v8: f9bc terrain y=0x%x mat=0x%zx\n",
                        (unsigned)iVar4, (size_t)local_60);

                if (0 < mips_subu_i32(local_84, iVar4)) {
                /* Contact: compute spring + damper forces */

                /* X velocity component, RTZ>>2, clamped to [-0xb40, 0xb40] */
                iVar20 = mips_subu_i32(0, (int32_t)param_1[0x20]);
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 3);
                uVar14 = (uint32_t)(iVar20 >> 2);
                if      ((int32_t)uVar14 < -0xb40) local_b8 = (uint32_t)(-0xb40);
                else if ((int32_t)uVar14 <  0xb41) local_b8 = uVar14;
                else                               local_b8 = 0xb40u;

                /* Z velocity component, same */
                iVar20 = mips_subu_i32(0, (int32_t)param_1[0x22]);
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 3);
                uVar14 = (uint32_t)(iVar20 >> 2);
                if      ((int32_t)uVar14 < -0xb40) local_b0 = (uint32_t)(-0xb40);
                else if ((int32_t)uVar14 <  0xb41) local_b0 = uVar14;
                else                               local_b0 = 0xb40u;

                /* Spring force Y = -compression, minus damping */
                local_b4 = mips_subu_i32(0, mips_subu_i32(local_84, iVar4));
                if (0 < (int32_t)param_1[0x21])
                    local_b4 = mips_subu_i32(local_b4, (int32_t)param_1[0x21] >> 2);

                /* Load contact-position offset into GTE R diagonal for OP12 */
                gte_ldR11R12((uint32_t)(mips_subu_i32(local_88, (int32_t)param_1[0x12]) >> 3));
                gte_ldR22R23((uint32_t)(mips_subu_i32(local_84, (int32_t)param_1[0x13]) >> 3));
                gte_ldR33   ((uint32_t)(mips_subu_i32(local_80, (int32_t)param_1[0x14]) >> 3));

                /* OP12: cross(diag_r, force) → torque contribution */
                gte_ldsv_((int32_t)local_b8 >> 3,
                           local_b4 >> 3,
                          (int32_t)local_b0 >> 3);
                gte_op12();

                /* Accumulate force */
                local_f8  = mips_addu_i32(local_f8, (int32_t)local_b8);
                local_f4  = mips_addu_i32(local_f4, local_b4);
                local_f0  = mips_addu_i32(local_f0, (int32_t)local_b0);

                /* Accumulate torque */
                local_108 = mips_addu_i32(local_108, gte_stMAC1());
                local_104 = mips_addu_i32(local_104, gte_stMAC2());
                local_100 = mips_addu_i32(local_100, gte_stMAC3());

                /* Surface-type callback (non-default material) */
                if (local_60 != 0 &&
                    *(int16_t *)(local_60 + 0x16) != 0 &&
                    *(int16_t *)(local_60 + 0x16) != 7) {
                    if (pcRam00000730)
                        (*pcRam00000730)(param_1, 9, &local_88,
                                         (unsigned int)param_1[0x14]);
                }

                /* Flag airborne if strong upward velocity */
                if (0x4c00 < (int32_t)param_1[0x21])
                    *param_1 |= 0x400000u;
            }

            uVar8 = (uint32_t)mips_addu_i32((int32_t)uVar8, 1);
            uVar14 = uVar8 & 1;
        } while ((int32_t)uVar8 < 8);
        if (0 && dbg_count <= 4)
            fprintf(stderr, "v8: f9bc grounded after corners force=(0x%x,0x%x,0x%x) torque=(0x%x,0x%x,0x%x)\n",
                    (unsigned)local_f8, (unsigned)local_f4, (unsigned)local_f0,
                    (unsigned)local_108, (unsigned)local_104, (unsigned)local_100);

        /* Rotate accumulated torque from body to world space */
        FUN_8004352c(param_1 + 4,
                     (uint32_t *)&local_108,
                     &local_108);
        if (0 && dbg_count <= 4)
            fprintf(stderr, "v8: f9bc grounded after torque rotate torque=(0x%x,0x%x,0x%x)\n",
                    (unsigned)local_108, (unsigned)local_104, (unsigned)local_100);

        /* ---- Per-wheel spin/animation update (wheels 0-3) ---- */
        {
            uint32_t *puVar22 = param_1 + 4;
            iVar4 = 4;
            do {
                uintptr_t wheelObj = (uintptr_t)puVar22[0x3b];           /* wheel object ptr */
                if (0 && dbg_count <= 4)
                    fprintf(stderr, "v8: f9bc grounded wheel update idx=%d ptr=0x%zx\n",
                            iVar4, wheelObj);
                iVar21 = *(int32_t *)(wheelObj + 0x98);
                /* Copy field: [0x4c] ← [0x84] */
                *(uint32_t *)(wheelObj + 0x4c) = *(uint32_t *)(wheelObj + 0x84);

                /* RTZ >> 6 decay of angular velocity */
                iVar20 = iVar21;
                if (iVar21 < 0) iVar20 = mips_addu_i32(iVar21, 0x3f);
                iVar21 = mips_subu_i32(iVar21, iVar20 >> 6);
                *(int32_t *)(wheelObj + 0x98) = iVar21;

                /* Update wheel display angle */
                if (iVar21 < 0) iVar21 = mips_addu_i32(iVar21, 0xfff);
                iVar20 = mips_mult_lo_i32(iVar21 >> 12, *(int32_t *)(wheelObj + 0x94));
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0x7ffff);
                *(int16_t *)(wheelObj + 0x40) =
                    (int16_t)mips_subu_i32(
                        (int32_t)(uint32_t)(uint16_t)*(int16_t *)(wheelObj + 0x40),
                        (int32_t)(uint32_t)(uint16_t)(int16_t)(iVar20 >> 19));

                FUN_8001d708((uint32_t *)wheelObj);

                puVar22++;
                iVar4 = mips_addu_i32(iVar4, 1);
            } while (iVar4 < 8);
        }

    } else {
        /* ================================================================
         * TUMBLING PATH  (status >= 1)
         * ================================================================ */
        if (0 && dbg_count <= 4)
            fprintf(stderr, "v8: f9bc tumbling path vel=(0x%x,0x%x,0x%x)\n",
                    (unsigned)param_1[0x20], (unsigned)param_1[0x21],
                    (unsigned)param_1[0x22]);

        /* Transform vehicle velocity to local/body space */
        FUN_8004352c(param_1 + 4,
                     (uint32_t *)(param_1 + 0x20),
                     &local_e8);
        if (0 && dbg_count <= 4)
            fprintf(stderr, "v8: f9bc local vel=(0x%x,0x%x,0x%x)\n",
                    (unsigned)local_e8, (unsigned)local_e4, (unsigned)local_e0);
        /* local_e8/e4/e0 = velocity in local space */

        /* ---- Write scratchpad: probe terrain for each of 4 wheels ---- */
        {
            int32_t iVar20_off = 0x10; /* byte offset from param_1 to wheel ptrs */
            for (iVar23 = 0; iVar23 < 4; iVar23++) {
                /* Wheel object pointer at param_1[0x3f + iVar23] */
                uintptr_t wptr = *(uint32_t *)((uint8_t *)param_1 + iVar20_off + 0xec);
                if (0 && dbg_count <= 4)
                    fprintf(stderr,
                            "v8: f9bc scratch wheel%d wptr=0x%zx local48=(0x%x,0x%x,0x%x) ride=(0x%x,0x%x,0x%x)\n",
                            iVar23, wptr,
                            wptr ? *(uint32_t *)(wptr + 0x48) : 0,
                            wptr ? *(uint32_t *)(wptr + 0x4c) : 0,
                            wptr ? *(uint32_t *)(wptr + 0x50) : 0,
                            wptr ? *(uint32_t *)(wptr + 0x84) : 0,
                            wptr ? *(uint32_t *)(wptr + 0x90) : 0,
                            wptr ? *(uint32_t *)(wptr + 0x94) : 0);
                iVar20_off = mips_addu_i32(iVar20_off, 4);

                local_d8 = *(int32_t *)(wptr + 0x48);
                local_d4 = mips_addu_i32(*(int32_t *)(wptr + 0x4c),
                                          *(int32_t *)(wptr + 0x90));
                local_d0 = *(int32_t *)(wptr + 0x50);

                /* Transform world wheel position into scratchpad (world_x/y/z) */
                FUN_80043408(param_1 + 4,
                             (uint32_t *)&local_d8,
                             (int32_t *)&g_wscr[iVar23]);
                if (0 && dbg_count <= 4)
                    fprintf(stderr,
                            "v8: f9bc scratch wheel%d after transform=(0x%x,0x%x,0x%x)\n",
                            iVar23,
                            (unsigned)g_wscr[iVar23].world_x,
                            (unsigned)g_wscr[iVar23].terrain_y,
                            (unsigned)g_wscr[iVar23].world_z);

                /* Probe terrain; result overwrites terrain_y, fills normal + mat */
                g_wscr[iVar23].terrain_y = Terrain_HeightAndProbe(
                    (intptr_t)param_1,
                    (int32_t *)&g_wscr[iVar23],
                    &g_wscr[iVar23].normal,
                    &g_wscr[iVar23].mat_ptr);
                if (0 && dbg_count <= 4)
                    fprintf(stderr,
                            "v8: f9bc scratch wheel%d world=(0x%x,0x%x,0x%x) ty=0x%x n=(%d,%d,%d) mat=0x%zx\n",
                            iVar23,
                            (unsigned)g_wscr[iVar23].world_x,
                            (unsigned)g_wscr[iVar23].terrain_y,
                            (unsigned)g_wscr[iVar23].world_z,
                            (unsigned)g_wscr[iVar23].terrain_y,
                            (int)g_wscr[iVar23].normal.vx,
                            (int)g_wscr[iVar23].normal.vy,
                            (int)g_wscr[iVar23].normal.vz,
                            g_wscr[iVar23].mat_ptr);
            }
        }

        /* ---- Main 4-wheel loop ---- */
        {
            uint32_t *local_4c  = param_1 + 0x20; /* velocity pointer */
            int32_t  *local_54  = &local_88;       /* target pos {x,y,z} */
            int32_t  *local_50  = &local_78;       /* probe {probe0, probe1} */

            iVar4  = 0; /* sequential wheel index 0..3 */
            iVar20 = 4; /* wheel ptr offset (4..7 → param_1[0x3f..0x42]) */

            do {
                uint32_t *puVar22 = (uint32_t *)(uintptr_t)param_1[iVar20 + 0x3b];

                /* Wheel angle → sin / cos */
                uVar6  = *(uint16_t *)((uint8_t *)puVar22 + 0x42) & 0xfffu;
                iVar21 = ((int16_t *)DAT_800607b4)[uVar6 * 2];     /* sin */
                local_58 = ((int16_t *)DAT_800607b4)[uVar6 * 2 + 1]; /* cos */

                /* Wheel body position */
                local_b8 = puVar22[0x12]; /* world pos x */
                local_b4 = mips_addu_i32((int32_t)puVar22[0x13],
                                          (int32_t)puVar22[0x24]); /* y + ride height */
                local_b0 = puVar22[0x14]; /* world pos z */

                /* Compute local-space target position using vehicle speed */
                iVar20 = mips_mult_lo_i32((int32_t)param_1[0x25],
                                          (int32_t)puVar22[0x14]); /* speed * wz */
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0xfff);
                local_88 = mips_addu_i32(local_e8, iVar20 >> 12);
                local_54[1] = local_e4;                       /* local_84 = e4 */
                iVar20 = mips_mult_lo_i32((int32_t)param_1[0x25],
                                          (int32_t)puVar22[0x12]); /* speed * wx */
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0xfff);
                local_54[2] = mips_subu_i32(local_e0, iVar20 >> 12);     /* local_80 */

                /* Read this wheel's scratchpad data */
                local_a8 = g_wscr[iVar4].world_x;
                local_a4 = g_wscr[iVar4].terrain_y;
                local_a0 = g_wscr[iVar4].world_z;
                /* probe0 = {ny_lo, ny_hi} packed int32, probe1 = {nz, pad} */
                local_78 = *(int32_t *)&g_wscr[iVar4].normal;
                local_74 = *((int32_t *)&g_wscr[iVar4].normal + 1);
                local_48 = g_wscr[iVar4].mat_ptr;

                /* Transform wheel world pos to local space → local_98/94/90 */
                FUN_800435c0(param_1 + 4, &local_a8, &local_98);
                local_94 = mips_subu_i32(local_94, (int32_t)puVar22[0x24]); /* subtract rest height */

                if ((int32_t)local_94 < (int32_t)puVar22[0x21]) {
                    /* ---- Wheel is in contact with terrain ---- */

                    /* Set contact-type status flags */
                    uVar6 = (local_48 == 0) ? 0x100000u : 0x300000u;
                    *param_1 |= uVar6;

                    /* -- 64-bit dot product: n · v (terrain normal × velocity) --
                     * normal components packed in local_78 and local_74:
                     *   nx = (int16_t)(local_78 & 0xffff)
                     *   ny = (int16_t)(local_78 >> 16)
                     *   nz = (int16_t)(local_74 & 0xffff)
                     * velocity from local_4c = param_1 + 0x20 */
                    {
                        int16_t nx = (int16_t)(local_78 & 0xffff);
                        int16_t ny = (int16_t)((uint32_t)local_78 >> 16);
                        int16_t nz = (int16_t)(local_74 & 0xffff);
                        int32_t vx = (int32_t)local_4c[0];
                        int32_t vy = (int32_t)local_4c[1];
                        int32_t vz = (int32_t)local_4c[2];
                        int64_t dot = (int64_t)nx * vx
                                    + (int64_t)ny * vy
                                    + (int64_t)nz * vz;
                        uVar6 = (uint32_t)(int32_t)(dot >> 15);
                    }

                    /* Transform terrain normal to local space → local_70/6c/68 */
                    FUN_800434d0(param_1 + 4, local_50, &local_70);

                    /* Normal-based friction force components (pre-shift by 12) */
                    iVar20 = mips_mult_lo_i32(mips_subu_i32(0, local_70), (int32_t)uVar6); /* -nx_local * dot */
                    if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0xfff);

                    /* X penetration depth clamped to 0 */
                    iVar23 = 0;
                    if (mips_subu_i32((int32_t)puVar22[0x12], local_98) < 0)
                        iVar23 = mips_subu_i32((int32_t)puVar22[0x12], local_98);

                    iVar11 = mips_mult_lo_i32(mips_subu_i32(0, local_68), (int32_t)uVar6); /* -nz_local * dot */
                    if (iVar11 < 0) iVar11 = mips_addu_i32(iVar11, 0xfff);

                    /* Z penetration depth clamped to 0 */
                    iVar10 = 0;
                    if (mips_subu_i32((int32_t)puVar22[0x14], local_90) < 0)
                        iVar10 = mips_subu_i32((int32_t)puVar22[0x14], local_90);

                    /* Spring force vertical component */
                    uVar6 = puVar22[0x20];
                    if ((int32_t)puVar22[0x20] < (int32_t)local_94)
                        uVar6 = (uint32_t)local_94;

                    uVar8  = puVar22[0x21]; /* rest height */
                    uVar14 = puVar22[0x23]; /* spring stiffness (int16) */

                    if (((int32_t)puVar22[0x20] < (int32_t)local_94) ||
                        ((int32_t)puVar22[0x13] < (int32_t)local_94)) {
                        /* Moving through surface: damped spring */
                        local_c4 = mips_mult_lo_i32(
                            mips_subu_i32(local_94, (int32_t)puVar22[0x13]),
                            (int32_t)*(int16_t *)((uint8_t *)puVar22 + 0x8e));
                        if (local_c4 < 0) local_c4 = mips_addu_i32(local_c4, 0x1f);
                        local_c4 >>= 5;
                    } else {
                        /* Bouncing: stiff spring, set airborne flag */
                        local_c4 = mips_mult_lo_i32(
                            mips_subu_i32(local_94, (int32_t)puVar22[0x13]), 0x10);
                        *param_1 |= 0x400000u;
                    }

                    /* PSX DIV does not trap on zero; host C does. */
                    {
                        int32_t spring_prod = mips_mult_lo_i32(
                            mips_subu_i32((int32_t)uVar8, (int32_t)uVar6),
                            (int32_t)(int16_t)uVar14);
                        int32_t spring_num = (int32_t)((uint32_t)spring_prod << 7);
                        local_c4 = mips_addu_i32(local_c4,
                                                 psx_div_i32(spring_num, (int32_t)local_6c));
                    }
                    /* Clamp spring travel */
                    puVar22[0x13] = (uint32_t)local_94;

                    /* Damping force: proportional to spring force */
                    if ((local_48 == 0) || (*(int16_t *)(local_48 + 0x10) == 0))
                        iVar17 = mips_mult_lo_i32(mips_subu_i32(0, local_c4), 2);
                    else
                        iVar17 = mips_mult_lo_i32(
                                      mips_subu_i32(0, local_c4),
                                      mips_subu_i32(0x100,
                                          (int32_t)*(int16_t *)(local_48 + 0x10))) >> 7;

                    /* -- Longitudinal (uVar6) and lateral (uVar14) velocity -- */
                    if ((*puVar22 & 0x20000u) == 0) {
                        /* Slow mode */
                        uVar6  = (uint32_t)((int32_t)local_88 >> 5);
                        uVar14 = (uint32_t)((int32_t)local_e0 >> 2);
                    } else {
                        /* Fast mode: decompose velocity into wheel frame */
                        int64_t l38 = (int64_t)local_e0 * (int64_t)local_58;
                        lVar1 = (int64_t)local_88 * (int64_t)local_58
                              - (int64_t)local_80 * (int64_t)iVar21;
                        uVar6  = (uint32_t)(int32_t)(lVar1 >> 17); /* long vel */
                        lVar1  = (int64_t)local_e8 * (int64_t)iVar21 + l38;
                        uVar14 = (uint32_t)(int32_t)(lVar1 >> 14); /* lat vel */
                    }

                    /* -- Steering / drive / brake lateral-force selection -- */
                    iVar13 = (int32_t)*(int16_t *)((uint8_t *)local_5c + 2); /* steer */
                    iVar18 = (iVar13 < 0) ? mips_subu_i32(0, iVar13) : iVar13; /* |steer| */
                    iVar7  = iVar17;
                    if (mips_mult_lo_i32(iVar18, 0x40) < iVar17)
                        iVar7 = mips_mult_lo_i32(iVar18, 0x40);

                    if (iVar13 < 0) {
                        /* Negative steering */
                        iVar18 = mips_subu_i32(0, (int32_t)uVar14);
                        if ((int32_t)uVar14 < 1) {
                            iVar13 = iVar18;
                            if (iVar7 < iVar18) iVar13 = iVar7;
                        } else {
                            iVar13 = mips_subu_i32(0, iVar7);
                            if (iVar13 <= iVar18) goto LAB_80030138;
                        }
                        iVar18 = iVar13;
                    } else if ((*puVar22 & 0x10000u) == 0) {
                        iVar18 = 0;
                    } else if (*(int8_t *)((uint8_t *)local_5c + 0xe) < 1) {
                        iVar18 = mips_subu_i32(0, iVar7);
                    } else {
                        iVar18 = iVar7;
                        if (iVar7 < (mips_subu_i32(0, (int32_t)uVar14) >> 2))
                            iVar18 = mips_subu_i32(0, (int32_t)uVar14) >> 2;
                    }

LAB_80030138:
                    /* Surface friction correction on lateral force */
                    if (local_48 != 0) {
                        iVar13 = (int32_t)uVar14 >> 8;
                        if (*(int16_t *)(local_48 + 0x12) != 0) {
                            iVar7 = (iVar13 < 0) ? mips_subu_i32(0, iVar13) : iVar13;
                            int32_t slip = mips_mult_lo_i32(iVar13, iVar7);
                            slip = mips_mult_lo_i32(slip,
                                (int32_t)*(int16_t *)(local_48 + 0x12));
                            iVar18 = mips_subu_i32(iVar18, slip >> 12);
                        }
                    }

                    /* Longitudinal force clamping */
                    iVar13 = mips_subu_i32(0, (int32_t)uVar6);
                    if ((int32_t)uVar6 < 1) {
                        if (iVar17 < iVar13) goto LAB_800301b4;
                    } else {
                        iVar17 = mips_subu_i32(0, iVar17);
                        if (iVar13 < iVar17) {
LAB_800301b4:
                            iVar13 = iVar17;
                        }
                    }

                    /* Final per-wheel forces in local frame (sin/cos rotate from wheel frame) */
                    local_c8 = mips_addu_i32(
                        mips_subu_i32(iVar20 >> 12, iVar23),
                        mips_addu_i32(mips_mult_lo_i32(iVar21, iVar18),
                                      mips_mult_lo_i32(local_58, iVar13)) >> 12);
                    local_c0 = mips_addu_i32(
                        mips_subu_i32(iVar11 >> 12, iVar10),
                        mips_subu_i32(mips_mult_lo_i32(local_58, iVar18),
                                      mips_mult_lo_i32(iVar21, iVar13)) >> 12);

                    /* Load wheel-centre offset into GTE R diagonal for OP12 torque */
                    gte_ldR11R12((uint32_t)((int32_t)local_b8 >> 3));
                    gte_ldR22R23((uint32_t)(local_b4 >> 3));
                    gte_ldR33   ((uint32_t)((int32_t)local_b0 >> 3));

                    /* Clamp forces to int16 range before OP12 */
                    {
                        int32_t sv_x = local_c8 >> 3;
                        int32_t sv_y = local_c4 >> 3;
                        int32_t sv_z = local_c0 >> 3;
                        if (sv_x >  0x7fff) sv_x =  0x7fff;
                        if (sv_x < -0x8000) sv_x = -0x8000;
                        if (sv_y >  0x7fff) sv_y =  0x7fff;
                        if (sv_y < -0x8000) sv_y = -0x8000;
                        if (sv_z >  0x7fff) sv_z =  0x7fff;
                        if (sv_z < -0x8000) sv_z = -0x8000;
                        gte_ldsv_(sv_x, sv_y, sv_z);
                    }
                    gte_op12();

                    /* Accumulate force + torque */
                    local_f8  = mips_addu_i32(local_f8, local_c8);
                    local_f4  = mips_addu_i32(local_f4, local_c4);
                    local_f0  = mips_addu_i32(local_f0, local_c0);
                    local_108 = mips_addu_i32(local_108, gte_stMAC1());
                    local_104 = mips_addu_i32(local_104, gte_stMAC2());
                    local_100 = mips_addu_i32(local_100, gte_stMAC3());

                    /* Surface callback */
                    if (local_48 != 0 &&
                        *(int16_t *)(local_48 + 0x16) != 0 &&
                        *(int16_t *)(local_48 + 0x16) != 7) {
                        if (pcRam00000730)
                            (*pcRam00000730)(puVar22, 9, &local_a8, 0u);
                    }

                } else {
                    /* Not in contact: snap wheel height to rest */
                    puVar22[0x13] = puVar22[0x21];
                }

                /* ---- Update wheel spin angle ---- */
                iVar20 = mips_addu_i32(mips_mult_lo_i32(iVar21, (int32_t)local_88),
                                       mips_mult_lo_i32(local_58, (int32_t)local_80));
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0xfff);
                iVar21 = mips_mult_lo_i32(iVar20 >> 12, (int32_t)puVar22[0x25]);
                puVar22[0x26] = (uint32_t)(iVar20 >> 12);
                if (iVar21 < 0) iVar21 = mips_addu_i32(iVar21, 0x7ffff);
                *(int16_t *)(puVar22 + 0x10) =
                    (int16_t)mips_subu_i32(
                        (int32_t)(uint32_t)*(uint16_t *)(puVar22 + 0x10),
                        (int32_t)(uint32_t)(uint16_t)(int16_t)(iVar21 >> 19));

                /* Advance loop */
                iVar23 = mips_addu_i32(iVar4, 1);
                iVar20 = mips_addu_i32(iVar4, 5);  /* next wheel ptr offset: 5,6,7 */
                iVar4  = iVar23;
            } while (iVar23 < 4);
        }

        /* Update 4 bone matrices */
        {
            int32_t off = 0x10;
            for (int w = 0; w < 4; w++) {
                int32_t slot = off + 0xec;
                uintptr_t wheelObj = *(uint32_t *)((uint8_t *)param_1 + slot);
                off = mips_addu_i32(off, 4);
                FUN_8001d708((uint32_t *)wheelObj);
            }
        }

        /* Rotate accumulated force from local to world space */
        FUN_80043358(param_1 + 4, &local_f8, &local_f8);
    }

    /* ================================================================
     * COMMON TAIL (both paths)
     * ================================================================ */

    /* Aerodynamic drag: proportional to speed × drag_coeff */
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc common before drag force=(0x%x,0x%x,0x%x) torque=(0x%x,0x%x,0x%x)\n",
                (unsigned)local_f8, (unsigned)local_f4, (unsigned)local_f0,
                (unsigned)local_108, (unsigned)local_104, (unsigned)local_100);
    iVar4 = mips_mult_lo_i32((int32_t)param_1[0x23], (int32_t)param_1[0x37]); /* speed * drag */
    local_f8 = mips_subu_i32(local_f8, mips_mult_hi_i32((int32_t)param_1[0x20], iVar4));
    local_f4 = mips_subu_i32(mips_addu_i32(local_f4, iRam00000030),
                             mips_mult_hi_i32((int32_t)param_1[0x21], iVar4));
    local_f0 = mips_subu_i32(local_f0, mips_mult_hi_i32((int32_t)param_1[0x22], iVar4));

    /* Apply forces + torques to physics state */
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc common before integrate force=(0x%x,0x%x,0x%x) torque=(0x%x,0x%x,0x%x)\n",
                (unsigned)local_f8, (unsigned)local_f4, (unsigned)local_f0,
                (unsigned)local_108, (unsigned)local_104, (unsigned)local_100);
    FUN_800173fc((uint8_t *)param_1, &local_f8, &local_108);
    if (wheel_trace_enabled() && trace_count < 160) {
        fprintf(stderr,
                "v8: wheel_trace obj=%p status=%d flags=0x%x sub=(steer=%d input=%d max=%u dir=%d) "
                "force=(%d,%d,%d) torque=(%d,%d,%d) pos=(0x%x,0x%x,0x%x) vel=(%d,%d,%d) "
                "mat=[%d,%d,%d;%d,%d,%d;%d,%d,%d]\n",
                (void *)param_1,
                (int)(int16_t)param_1[6],
                (unsigned)param_1[0],
                (int)*(int16_t *)((uint8_t *)local_5c + 0),
                (int)*(int16_t *)((uint8_t *)local_5c + 2),
                (unsigned)*(uint16_t *)((uint8_t *)local_5c + 8),
                (int)*(int8_t *)((uint8_t *)local_5c + 0xe),
                (int)local_f8, (int)local_f4, (int)local_f0,
                (int)local_108, (int)local_104, (int)local_100,
                param_1[9], param_1[10], param_1[11],
                (int32_t)param_1[0x20], (int32_t)param_1[0x21], (int32_t)param_1[0x22],
                (int)*(int16_t *)((uint8_t *)param_1 + 0x10),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x12),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x14),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x16),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x18),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x1a),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x1c),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x1e),
                (int)*(int16_t *)((uint8_t *)param_1 + 0x20));
        trace_count++;
    }
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc common after integrate m=(0x%x,0x%x,0x%x,0x%x,0x%x) t=(0x%x,0x%x,0x%x) vel=(0x%x,0x%x,0x%x)\n",
                param_1[4], param_1[5], param_1[6], param_1[7], param_1[8],
                param_1[9], param_1[10], param_1[11],
                param_1[0x20], param_1[0x21], param_1[0x22]);

    /* Decay angular velocity components (param_1[0x24..0x26]) RTZ >> 5 */
    uVar14 = param_1[0x24];
    uVar6  = uVar14;
    if ((int32_t)uVar14 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar14, 0x1f);
    uVar8 = param_1[0x25];
    param_1[0x24] = (uint32_t)mips_subu_i32((int32_t)uVar14, (int32_t)uVar6 >> 5);

    uVar6 = uVar8;
    if ((int32_t)uVar8 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar8, 0x1f);
    uVar14 = param_1[0x26];
    param_1[0x25] = (uint32_t)mips_subu_i32((int32_t)uVar8, (int32_t)uVar6 >> 5);

    uVar6 = uVar14;
    if ((int32_t)uVar14 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar14, 0x1f);
    param_1[0x26] = (uint32_t)mips_subu_i32((int32_t)uVar14, (int32_t)uVar6 >> 5);

    /* Decay per-slot hit counters at param_1[0x44..0x46] */
    {
        uint32_t *puVar22 = param_1;
        for (iVar4 = 0; iVar4 < 3; iVar4++) {
            uintptr_t hitObj = puVar22[0x44];
            if (host_heap_contains_ptr(hitObj) && *(int16_t *)(hitObj + 6) != 0)
                *(uint16_t *)(hitObj + 6) =
                    (uint16_t)mips_subu_i32(
                        (int32_t)(uint32_t)*(uint16_t *)(hitObj + 6), 1);
            puVar22++;
        }
    }
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc common after hit counters\n");

    /* Decay short counters at param_1[0x47..] (stride 2 bytes) */
    {
        uint32_t *puVar22 = param_1;
        for (iVar4 = 0; iVar4 < 3; iVar4++) {
            if ((int16_t)puVar22[0x47] != 0)
                *(uint16_t *)((uint8_t *)puVar22 + 0x47 * 4) =
                    (uint16_t)mips_subu_i32(
                        (int32_t)(uint32_t)*(uint16_t *)((uint8_t *)puVar22 + 0x47 * 4), 1);
            puVar22 = (uint32_t *)((uint8_t *)puVar22 + 2);
        }
    }

    /* Interpolate display position toward physics position */
    if ((*param_1 & 0x800000u) == 0) {
        if ((int16_t)param_1[0x48] == 0) {
            if ((int16_t)param_1[6] < 0 || (*param_1 & 0x40000u) != 0) {
                /* Smooth interpolation (blend factor from wheel param block) */
                iVar20 = mips_subu_i32(0x100, (int32_t)(uint32_t)(uint8_t)local_5c[4]);
                iVar4  = mips_mult_lo_i32(
                    mips_subu_i32((int32_t)param_1[9], (int32_t)param_1[0x12]), iVar20);
                if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0xff);
                iVar21 = mips_mult_lo_i32(
                    mips_subu_i32((int32_t)param_1[10], (int32_t)param_1[0x13]), iVar20);
                param_1[0x12] = (uint32_t)mips_addu_i32((int32_t)param_1[0x12], iVar4 >> 8);
                if (iVar21 < 0) iVar21 = mips_addu_i32(iVar21, 0xff);
                iVar20 = mips_mult_lo_i32(
                    mips_subu_i32((int32_t)param_1[0xb], (int32_t)param_1[0x14]), iVar20);
                param_1[0x13] = (uint32_t)mips_addu_i32((int32_t)param_1[0x13], iVar21 >> 8);
                if (iVar20 < 0) iVar20 = mips_addu_i32(iVar20, 0xff);
                param_1[0x14] = (uint32_t)mips_addu_i32((int32_t)param_1[0x14], iVar20 >> 8);
            } else {
                /* Snap directly to target */
                param_1[0x12] = param_1[9];
                param_1[0x13] = param_1[10];
                param_1[0x14] = param_1[0xb];
            }
        } else {
            /* RTZ >> 5 interpolation toward target */
            iVar4 = mips_subu_i32((int32_t)param_1[9], (int32_t)param_1[0x12]);
            if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x1f);
            param_1[0x12] = (uint32_t)mips_addu_i32((int32_t)param_1[0x12], iVar4 >> 5);

            iVar4 = mips_subu_i32((int32_t)param_1[10], (int32_t)param_1[0x13]);
            if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x1f);
            param_1[0x13] = (uint32_t)mips_addu_i32((int32_t)param_1[0x13], iVar4 >> 5);

            iVar4 = mips_subu_i32((int32_t)param_1[0xb], (int32_t)param_1[0x14]);
            if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x1f);
            param_1[0x14] = (uint32_t)mips_addu_i32((int32_t)param_1[0x14], iVar4 >> 5);
        }
    }
    if (0 && dbg_count <= 4)
        fprintf(stderr, "v8: f9bc common exit\n");
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10 FUN_8002f9bc (from analysis/SLUS_005.10/decomp/8002f9bc.c) --- */
/* (see analysis/SLUS_005.10/decomp/8002f9bc.c for full 553-line Ghidra listing) */
#endif /* GHIDRA REF */
