/* collision_test.c -- Object bounding-volume collision tests.
 *
 * Source: SLUS_005.10
 *   FUN_8001e1c0  AABB_Overlap          (~40 instr)
 *   FUN_8001e408  ShapePlane_SepTest    (~55 instr)
 *   FUN_8001e9a0  CollShape_Dispatch    (~130 instr)
 *   FUN_8001ec48  CollTree_HasContact   (~20 instr)
 *   FUN_8001ecc4  CollTree_Traverse     (~40 instr)
 *   FUN_8001edb4  Object_TestCollision  (~75 instr)
 *
 * Collision shape types (shape record[0]):
 *   0 = free slot (no shape)
 *   1 = AABB bounding box (6 int entries: min.xyz, max.xyz from record+4)
 *   2 = convex-hull / sphere list  (record[1]=sphere_count, followed by
 *       6-byte SVECTOR + radius entries at record+4, stride 6)
 *   3 = binary split node (children at record+8 and record+10 ptrs)
 *
 * Scratchpad (0x1f800000) usage:
 *   On PSX the scratchpad at 0x1f800000 is used to stash collision
 *   result pointers. On the host these are emulated as static globals.
 *   The collision functions return 0x1f800000 (a non-NULL sentinel)
 *   on hit, and 0 on miss.  Callers cast the return to int* and read
 *   [0..4] for the hit shape pointers / object pointers.
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>
#include <stdbool.h>
#include "structs.h"

/* GTE matrix operations */
extern void   FUN_8004366c(uint32_t *m);                   /* GTE_LoadMatrixTransposed */
extern void   FUN_80043248(int *v, int *out);              /* GTE_RotateVec3 */
extern MATRIX *MulRotMatrix0(MATRIX *a, MATRIX *out);
extern void   FUN_80043358(MATRIX *m, int *v, int *out);   /* GTE_RotateMat */
extern void   FUN_800436c8(MATRIX *m);                     /* GTE_LoadMatrix */
extern void   ApplyMatrixSV(MATRIX *m, SVECTOR *in, SVECTOR *out);
extern void   FUN_800434d0(intptr_t obj, SVECTOR *in, int *out); /* GTE_ProjectVec */
extern MATRIX *CompMatrixLV(MATRIX *a, MATRIX *b, MATRIX *out);
extern uint32_t FUN_8001e120(intptr_t param_1, int eventId, intptr_t scratchOut);

/* PSX scratchpad emulation (0x1f800000 region).
 * The query record keeps the original 32-bit field layout because SAT and
 * collision handlers index it by byte offsets.  The pointer to this host
 * buffer is passed through intptr_t so the buffer itself can live above 4GB. */
static uint32_t s_colScratch[16];
uint32_t _DAT_1f80000c; /* object ptr A -- also read by spillway_grab.c */

/* Sentinel return value when a collision is found.
 * On PSX this would be 0x1f800000; on host we return the address of
 * the scratchpad buffer (cast to uintptr_t so callers can index it). */
#define SCRATCH_BASE ((uintptr_t)(void*)s_colScratch)

#define SET_COL_SCRATCH(aShape, bShape, aObj, bObj) do { \
    s_colScratch[0] = 0; \
    s_colScratch[1] = (uint32_t)(uintptr_t)(aShape); \
    s_colScratch[2] = (uint32_t)(uintptr_t)(bShape); \
    s_colScratch[3] = (uint32_t)(uintptr_t)(aObj); \
    s_colScratch[4] = (uint32_t)(uintptr_t)(bObj); \
    _DAT_1f80000c = (uint32_t)(uintptr_t)(aObj); \
} while (0)

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

static inline int32_t mips_div2_rtz_i32(int32_t v)
{
    return mips_addu_i32(v, (int32_t)((uint32_t)v >> 31)) >> 1;
}

static inline int32_t mips_avg2_i32(int32_t a, int32_t b)
{
    return mips_div2_rtz_i32(mips_addu_i32(a, b));
}

static inline int32_t mips_abs_i32(int32_t v)
{
    return (v < 0) ? mips_subu_i32(0, v) : v;
}

/* ================================================================
 * FUN_8001e1c0 -- AABB_Overlap
 *
 * Tests whether a world-space AABB (param_1[0..5], 3 × min + 3 × max)
 * overlaps a local-space AABB (param_3[0..5]) after transforming
 * param_3 through the rigid body matrix param_4.
 *
 * param_2: object handle (used to read +0x14/+0x18/+0x1c translation).
 *
 * Returns true if overlap detected.
 *
 * HIGH confidence.
 * ================================================================ */
bool FUN_8001e1c0(int *param_1, intptr_t param_2, int *param_3, MATRIX *param_4)
{
    bool  bVar1;
    MATRIX MStack_68;
    int   local_48[3];  /* sp+0x30 displacement vector */
    int   local_38[3];  /* sp+0x40 transformed centre vector */
    int   local_28[3];  /* sp+0x50 half-extents vector */

    /* World-space displacement from param_4 translation to param_2 centre */
    local_48[0] = mips_subu_i32(param_4->t[0], *(int *)(uintptr_t)(param_2 + 0x14));
    local_48[1] = mips_subu_i32(param_4->t[1], *(int *)(uintptr_t)(param_2 + 0x18));
    local_48[2] = mips_subu_i32(param_4->t[2], *(int *)(uintptr_t)(param_2 + 0x1c));

    /* Centre of param_3 AABB */
    local_38[0] = mips_avg2_i32(*param_3, param_3[3]);
    local_38[1] = mips_avg2_i32(param_3[1], param_3[4]);
    local_38[2] = mips_avg2_i32(param_3[2], param_3[5]);

    /* Half-extents of param_3 AABB */
    local_28[0] = mips_div2_rtz_i32(mips_subu_i32(param_3[3], *param_3));
    local_28[1] = mips_div2_rtz_i32(mips_subu_i32(param_3[4], param_3[1]));
    local_28[2] = mips_div2_rtz_i32(mips_subu_i32(param_3[5], param_3[2]));

    /* MIPS leaves a0 = param_2 here; load param_2's transposed matrix. */
    FUN_8004366c((uint32_t *)(uintptr_t)param_2);
    /* Rotate displacement into param_4 local space */
    FUN_80043248(local_48, local_48);
    /* Compose rotation: param_4 R^T  ×  param_4 R = identity aligned */
    MulRotMatrix0(param_4, &MStack_68);
    /* Rotate centre into composed space */
    FUN_80043358(&MStack_68, local_38, local_38);

    local_38[0] = mips_addu_i32(local_38[0], local_48[0]);
    local_38[1] = mips_addu_i32(local_38[1], local_48[1]);
    local_38[2] = mips_addu_i32(local_38[2], local_48[2]);

    /* Load param_4 matrix for half-extent rotation */
    FUN_800436c8(&MStack_68);
    FUN_80043248(local_28, local_28);

    /* Separating axis test on all three axes */
    bVar1 = false;
    if (((mips_subu_i32(local_38[0], local_28[0]) <= param_1[3]) &&
         (*param_1 <= mips_addu_i32(local_38[0], local_28[0]))) &&
         (mips_subu_i32(local_38[1], local_28[1]) <= param_1[4])) {
        if ((param_1[1] <= mips_addu_i32(local_38[1], local_28[1])) &&
            (mips_subu_i32(local_38[2], local_28[2]) <= param_1[5])) {
            bVar1 = param_1[2] <= mips_addu_i32(local_38[2], local_28[2]);
        }
    }
    return bVar1;
}

/* ================================================================
 * FUN_8001e408 -- ShapePlane_SepTest
 *
 * Projects a sphere (param_3: SVECTOR with vw = radius) of shape B
 * into shape A's local space (param_4) and tests whether it is
 * separated from shape A's AABB (param_1[0..5]).
 *
 * param_2: object B handle (for +0x14/+0x18/+0x1c translation).
 *
 * Returns 1 if separated (no collision), 0 if overlapping.
 *
 * HIGH confidence: 64-bit dot-product math preserved exactly.
 * ================================================================ */
uint32_t FUN_8001e408(int *param_1, intptr_t param_2, SVECTOR *param_3, MATRIX *param_4)
{
    uint32_t  uVar1;
    long long lVar2, lVar3, lVar4;
    uint32_t  uVar5, uVar6, uVar7, uVar8, uVar9, uVar10, uVar11;
    uint32_t  uVar12, uVar13, uVar14;
    int       iVar15;
    SVECTOR   local_50;
    int       local_48 = 0, local_44 = 0, local_40 = 0;
    SVECTOR  *local_38;
    int       local_30;

    local_38 = &local_50;
    local_30  = param_2;

    /* Rotate param_3 (sphere axis) into param_4's local space */
    ApplyMatrixSV(param_4, param_3, local_38);

    uVar11 = (int)((uint32_t)(unsigned short)local_50.vx << 16) >> 16;
    uVar6  = (uint32_t)mips_subu_i32(
              mips_addu_i32(*(int *)(uintptr_t)(local_30 + 0x14),
                             mips_avg2_i32(*param_1, param_1[3])),
              param_4->t[0]);
    lVar2  = (unsigned long long)uVar11 * (unsigned long long)uVar6;

    uVar13 = (int)((uint32_t)(unsigned short)local_50.vy << 16) >> 16;
    uVar9  = (uint32_t)mips_subu_i32(
              mips_addu_i32(*(int *)(uintptr_t)(local_30 + 0x18),
                             mips_avg2_i32(param_1[1], param_1[4])),
              param_4->t[1]);
    lVar3  = (unsigned long long)uVar13 * (unsigned long long)uVar9;
    uVar5  = (uint32_t)lVar3;

    uVar10 = (int)((uint32_t)(unsigned short)local_50.vz << 16) >> 16;
    uVar14 = (uint32_t)mips_subu_i32(
              mips_addu_i32(*(int *)(uintptr_t)(local_30 + 0x1c),
                             mips_avg2_i32(param_1[2], param_1[5])),
              param_4->t[2]);
    lVar4  = (unsigned long long)uVar10 * (unsigned long long)uVar14;
    uVar1  = (uint32_t)lVar4;

    uVar7 = (uint32_t)mips_addu_i32((int32_t)lVar2, (int32_t)uVar5);
    uVar8 = (uint32_t)mips_addu_i32((int32_t)uVar7, (int32_t)uVar1);

    iVar15 = (uVar8 >> 12 |
              ((int)((unsigned long long)lVar2 >> 32) + uVar11 * ((int)uVar6 >> 31) +
               uVar6 * ((int)((uint32_t)(unsigned short)local_50.vx << 16) >> 31) +
               (int)((unsigned long long)lVar3 >> 32) + uVar13 * ((int)uVar9 >> 31) +
               uVar9 * ((int)((uint32_t)(unsigned short)local_50.vy << 16) >> 31) +
               (uint32_t)(uVar7 < uVar5) +
               (int)((unsigned long long)lVar4 >> 32) + uVar10 * ((int)uVar14 >> 31) +
               uVar14 * ((int)((uint32_t)(unsigned short)local_50.vz << 16) >> 31) +
               (uint32_t)(uVar8 < uVar1)) * 0x100000)
             - *(int *)(param_3 + 1);   /* param_3->vw = radius */

    if (iVar15 < 0) {
        uVar5 = 1;
    } else {
        /* Project the AABB half-extents along the sphere axis */
        FUN_800434d0(local_30, local_38, &local_48);
        local_48 = mips_mult_lo_i32(local_48, mips_subu_i32(param_1[3], *param_1));
        local_44 = mips_mult_lo_i32(local_44, mips_subu_i32(param_1[4], param_1[1]));
        local_40 = mips_mult_lo_i32(local_40, mips_subu_i32(param_1[5], param_1[2]));
        local_48 = mips_abs_i32(local_48);
        local_44 = mips_abs_i32(local_44);
        local_40 = mips_abs_i32(local_40);
        local_40 = mips_addu_i32(mips_addu_i32(local_48, local_44), local_40);
        if (local_40 < 0) local_40 = mips_addu_i32(local_40, 0x1fff);
        uVar5 = (uint32_t)mips_subu_i32(iVar15, local_40 >> 13) >> 31;
    }
    return uVar5;
}

/* ================================================================
 * FUN_8001e9a0 -- CollShape_Dispatch
 *
 * Tests object-A (param_1) against object-B (param_2) using their
 * attached collision-shape lists (each linked via +0x5c).
 *
 * Shape type 1 = AABB: uses AABB_Overlap (FUN_8001e1c0).
 * Shape type 2 = sphere list: uses ShapePlane_SepTest (FUN_8001e408).
 *
 * On hit: stores result pointers to the scratchpad, returns
 * SCRATCH_BASE (non-zero).  On no hit: returns 0.
 *
 * HIGH confidence: direct port.
 * ================================================================ */
uintptr_t FUN_8001e9a0(intptr_t param_1, intptr_t param_2, uintptr_t param_3, uintptr_t param_4)
{
    int16_t   sVar1;
    int       iVar2, iVar3, iVar5;
    int16_t  *psVar4, *psVar6;

    psVar6 = (int16_t *)(uintptr_t)*(uint32_t *)(param_1 + 0x5c);
    if (psVar6 != NULL) {
        if (*(int *)(uintptr_t)(param_2 + 0x5c) == 0) {
            return 0;
        }
        sVar1 = *psVar6;
        while (sVar1 != 0) {
            sVar1 = *psVar6;
            psVar4 = (int16_t *)(uintptr_t)*(uint32_t *)(param_2 + 0x5c);
            if (sVar1 == 1) {
                if (*psVar4 != 0) {
                    do {
                        sVar1 = *psVar4;
                        if (sVar1 == 1) {
                            iVar3 = FUN_8001e1c0((int *)psVar6 + 1, (intptr_t)param_3,
                                                 (int *)psVar4 + 1, (MATRIX *)(uintptr_t)param_4);
                            if ((iVar3 != 0) &&
                                (iVar3 = FUN_8001e1c0((int *)psVar4 + 1, (intptr_t)param_4,
                                                      (int *)psVar6 + 1, (MATRIX *)(uintptr_t)param_3),
                                 iVar3 != 0)) {
                                SET_COL_SCRATCH(psVar6, psVar4, param_1, param_2);
                                return SCRATCH_BASE;
                            }
                            psVar4 = psVar4 + 0xe;
                            sVar1  = *psVar4;
                        } else if (sVar1 == 2) {
                            iVar3 = 0;
                            if (psVar4[1] == 0) {
                                SET_COL_SCRATCH(psVar6, psVar4, param_1, param_2);
                                return SCRATCH_BASE;
                            }
                            iVar5 = 4;
                            while (true) {
                                iVar2 = (int)FUN_8001e408((int *)psVar6 + 1, (intptr_t)param_3,
                                                          (SVECTOR *)((uintptr_t)psVar4 + iVar5),
                                                          (MATRIX *)(uintptr_t)param_4);
                                iVar3 = iVar3 + 1;
                                if (iVar2 == 0) break;
                                iVar5 = iVar5 + 0xc;
                                if ((int)(uint32_t)(unsigned short)psVar4[1] <= iVar3) {
                                    SET_COL_SCRATCH(psVar6, psVar4, param_1, param_2);
                                    return SCRATCH_BASE;
                                }
                            }
                            psVar4 = psVar4 + (uint32_t)(unsigned short)psVar4[1] * 6 + 2;
                            sVar1  = *psVar4;
                        }
                    } while (sVar1 != 0);
                }
                psVar6 = psVar6 + 0xe;
                sVar1  = *psVar6;
            } else if (sVar1 == 2) {
                /* type-2 shape in A, iterate B shapes */
LAB_8001ebac:
                do {
                    sVar1 = *psVar4;
                    while (true) {
                        if (sVar1 == 0) {
                            psVar6 = psVar6 + (uint32_t)(unsigned short)psVar6[1] * 6 + 2;
                            sVar1  = *psVar6;
                            goto next_outer;
                        }
                        sVar1 = *psVar4;
                        if (sVar1 == 1) break;
                        if (sVar1 == 2) goto LAB_8001eb90;
                    }
                    iVar3 = 0;
                    if (psVar6[1] == 0) {
                        SET_COL_SCRATCH(psVar6, psVar4, param_1, param_2);
                        return SCRATCH_BASE;
                    }
                    iVar5 = 4;
                    while (true) {
                        iVar2 = (int)FUN_8001e408((int *)psVar4 + 1, (intptr_t)param_4,
                                                  (SVECTOR *)((uintptr_t)psVar6 + iVar5),
                                                  (MATRIX *)(uintptr_t)param_3);
                        iVar3 = iVar3 + 1;
                        if (iVar2 == 0) break;
                        iVar5 = iVar5 + 0xc;
                        if ((int)(uint32_t)(unsigned short)psVar6[1] <= iVar3) {
                            SET_COL_SCRATCH(psVar6, psVar4, param_1, param_2);
                            return SCRATCH_BASE;
                        }
                    }
                    psVar4 = psVar4 + 0xe;
                    continue;
LAB_8001eb90:
                    psVar4 = psVar4 + (uint32_t)(unsigned short)psVar4[1] * 6 + 2;
                } while (true);
            }
next_outer:;
        }
    }
    return 0;

    (void)param_3; /* suppress potential unused-parameter warning in paths */
    goto LAB_8001ebac;
}

/* ================================================================
 * FUN_8001ec48 -- CollTree_HasContact
 *
 * Recursively tests whether any node in the object sub-tree starting
 * at param_1 has a non-zero contact list (param_1[0x17] != 0).
 * Propagates the 0x800 flag up to mark intermediate nodes that have
 * active contacts somewhere in their subtree.
 *
 * param_1[0xe]  = first child pointer (siblings linked via child[0x34])
 * param_1[0x17] = active contact count
 *
 * Returns non-zero if any descendant has a contact.
 *
 * HIGH confidence.
 * ================================================================ */
uint32_t FUN_8001ec48(uint32_t *param_1)
{
    uint32_t uVar1, uVar2, uVar3;

    uVar3 = 0;
    for (uVar2 = param_1[0xe]; uVar2 != 0; uVar2 = *(uint32_t *)(uintptr_t)(uVar2 + 0x34)) {
        uVar1 = FUN_8001ec48((uint32_t *)(uintptr_t)uVar2);
        uVar3 = uVar3 | uVar1;
    }
    if (uVar3 != 0) {
        *param_1 = *param_1 | 0x800;
    }
    return uVar3 | (param_1[0x17] != 0);
}

/* ================================================================
 * FUN_8001ecc4 -- CollTree_Traverse
 *
 * Walks object B's child tree (each child at +0x38 linked via
 * child[0x0d]) looking for a collision with object A (param_1)
 * using its composed matrix param_3.
 *
 * For each child: if flag 0x20 or contactCount==0: skip unless
 * flag 0x800 (has sub-children with contacts), in which case recurse.
 * Otherwise: run CollShape_Dispatch; if hit return the scratchpad ptr.
 *
 * HIGH confidence: direct port.
 * ================================================================ */
uintptr_t FUN_8001ecc4(intptr_t param_1, intptr_t param_2, MATRIX *param_3)
{
    uintptr_t iVar1;
    uint32_t *puVar2;
    MATRIX MStack_30;

    puVar2 = (uint32_t *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_2 + 0x38);
    do {
        if (puVar2 == NULL) {
            return 0;
        }
        if ((puVar2[0x17] == 0) || ((*puVar2 & 0x20) != 0)) {
            if ((*puVar2 & 0x800) != 0) {
                CompMatrixLV(param_3, (MATRIX *)(puVar2 + 4), &MStack_30);
                goto LAB_8001ed78;
            }
        } else {
            CompMatrixLV(param_3, (MATRIX *)(puVar2 + 4), &MStack_30);
            iVar1 = FUN_8001e9a0(param_1, (intptr_t)puVar2,
                                  (uintptr_t)(param_1 + 0x10),
                                  (uintptr_t)&MStack_30);
            if (iVar1 != 0) {
                return iVar1;
            }
            if ((*puVar2 & 0x800) != 0) {
LAB_8001ed78:
                iVar1 = FUN_8001ecc4(param_1, (intptr_t)puVar2, &MStack_30);
                if (iVar1 != 0) {
                    return iVar1;
                }
            }
        }
        puVar2 = (uint32_t *)(uintptr_t)puVar2[0x0d];
    } while (true);
}

/* ================================================================
 * FUN_8001edb4 -- Object_TestCollision
 *
 * Primary object-vs-object collision test.  Checks whether two
 * objects (param_1 and param_2) have overlapping bounding shapes.
 *
 * Fast sphere-sphere reject: if |pos_A - pos_B| >= reach_A + reach_B
 * the objects cannot touch → return 0.
 *
 * Flag meanings:
 *   param_2[0] & 0x40  = register contact pointers into param_1's slot
 *   param_2[0] & 0x800 = object has sub-children → use CollTree_Traverse
 *
 * Contact dispatch via FUN_8001e120 (event id 3).
 *
 * Returns 0 if no collision, non-zero otherwise.
 *
 * HIGH confidence: direct port.
 * ================================================================ */
uint32_t FUN_8001edb4(intptr_t param_1, uint32_t *param_2)
{
    bool  bVar1;
    int   iVar2;
    uint32_t uVar3, uVar5;
    uint32_t *piVar4;
    int   iVar6;

    bVar1 = false;

    /* Quick-reject: same layer (short at +6 matches) → no collision */
    if (*(int16_t *)(uintptr_t)(param_1 + 6) == *(int16_t *)((uintptr_t)param_2 + 6)) {
        uVar3 = 0;
    } else {
        /* Sphere-sphere broad phase */
        iVar6 = mips_addu_i32(*(int *)(uintptr_t)(param_1 + 0x54),
                              (int32_t)param_2[0x15]);

        iVar2 = mips_subu_i32(*(int *)(uintptr_t)(param_1 + 0x24), (int32_t)param_2[9]);
        if (iVar2 < 0) iVar2 = mips_subu_i32(0, iVar2);
        if (iVar2 < iVar6) {
            iVar2 = mips_subu_i32(*(int *)(uintptr_t)(param_1 + 0x28), (int32_t)param_2[10]);
            if (iVar2 < 0) iVar2 = mips_subu_i32(0, iVar2);
            if (iVar2 < iVar6) {
                iVar2 = mips_subu_i32(*(int *)(uintptr_t)(param_1 + 0x2c),
                                      (int32_t)param_2[0x0b]);
                if (iVar2 < 0) iVar2 = mips_subu_i32(0, iVar2);
                bVar1 = iVar2 < iVar6;
            }
        }

        uVar3 = 0;
        if (bVar1) {
            /* Register contact slot pointers if B has the 0x40 flag */
            if ((*param_2 & 0x40) != 0) {
                if (*(int *)(uintptr_t)(param_1 + 0x74) == 0) {
                    *(uint32_t *)(uintptr_t)(param_1 + 0x74) = (uint32_t)(uintptr_t)param_2;
                } else {
                    *(uint32_t *)(uintptr_t)(param_1 + 0x78) = (uint32_t)(uintptr_t)param_2;
                }
            }

            /* Narrow phase: sub-tree traverse or direct shape test */
            if (((*param_2 & 0x800) == 0) ||
                (piVar4 = (uint32_t *)FUN_8001ecc4(param_1, (intptr_t)param_2, (MATRIX *)(param_2 + 4)),
                 piVar4 == NULL)) {
                piVar4 = (uint32_t *)FUN_8001e9a0(param_1, (intptr_t)param_2,
                                                   (uintptr_t)(param_1 + 0x10),
                                                   (uintptr_t)(param_2 + 4));
                if (piVar4 == NULL) {
                    return 0;
                }
            }

            /* piVar4 points into scratchpad; rewrite obj A pointer */
            *piVar4 = (uint32_t)(uintptr_t)param_2;

            uVar5 = FUN_8001e120(param_1, 3, (intptr_t)piVar4);
            if ((uVar5 == 0) || (uVar3 = uVar5 >> 31, uVar5 == 0xffffffff)) {
                /* Swap shape pointers so A is the contacting object */
                uint32_t oldShapeB = piVar4[1];
                uint32_t oldObjB = piVar4[3];
                uVar3 = uVar5 >> 31;
                *piVar4     = (uint32_t)(uintptr_t)param_1;
                piVar4[1]   = piVar4[2];
                piVar4[2]   = oldShapeB;
                piVar4[3]   = piVar4[4];
                piVar4[4]   = oldObjB;
                iVar2 = (int)FUN_8001e120((intptr_t)param_2, 3, (intptr_t)piVar4);
                if (iVar2 < 0) {
                    uVar3 = uVar3 | 2;
                }
            }
        }
    }
    return uVar3;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001e1c0  (from analysis/SLUS_005.10/decomp/8001e1c0.c) --- */
// addr: 0x8001e1c0  name: FUN_8001e1c0

bool FUN_8001e1c0(int *param_1,int param_2,int *param_3,MATRIX *param_4)
{
  bool bVar1;
  MATRIX MStack_68;
  int local_48; int local_44; int local_40;
  int local_38; int local_34; int local_30;
  int local_28; int local_24; int local_20;

  local_48 = param_4->t[0] - *(int *)(param_2 + 0x14);
  local_44 = param_4->t[1] - *(int *)(param_2 + 0x18);
  local_40 = param_4->t[2] - *(int *)(param_2 + 0x1c);
  local_38 = (*param_3 + param_3[3]) / 2;
  local_34 = (param_3[1] + param_3[4]) / 2;
  local_30 = (param_3[2] + param_3[5]) / 2;
  local_28 = (param_3[3] - *param_3) / 2;
  local_24 = (param_3[4] - param_3[1]) / 2;
  local_20 = (param_3[5] - param_3[2]) / 2;
  FUN_8004366c();
  FUN_80043248(&local_48,&local_48);
  MulRotMatrix0(param_4,&MStack_68);
  FUN_80043358(&MStack_68,&local_38,&local_38);
  local_38 = local_38 + local_48;
  local_34 = local_34 + local_44;
  local_30 = local_30 + local_40;
  FUN_800436c8(&MStack_68);
  FUN_80043248(&local_28,&local_28);
  bVar1 = false;
  if ((((local_38 - local_28 <= param_1[3]) && (*param_1 <= local_38 + local_28)) &&
      (local_34 - local_24 <= param_1[4])) &&
     ((param_1[1] <= local_34 + local_24 && (local_30 - local_20 <= param_1[5])))) {
    bVar1 = param_1[2] <= local_30 + local_20;
  }
  return bVar1;
}

/* --- SLUS_005.10 FUN_8001ec48  (from analysis/SLUS_005.10/decomp/8001ec48.c) --- */
// addr: 0x8001ec48  name: FUN_8001ec48

uint FUN_8001ec48(uint *param_1)
{
  uint uVar1; uint uVar2; uint uVar3;
  uVar3 = 0;
  for (uVar2 = param_1[0xe]; uVar2 != 0; uVar2 = *(uint *)(uVar2 + 0x34)) {
    uVar1 = FUN_8001ec48(uVar2); uVar3 = uVar3 | uVar1;
  }
  if (uVar3 != 0) { *param_1 = *param_1 | 0x800; }
  return uVar3 | param_1[0x17] != 0;
}

/* --- SLUS_005.10 FUN_8001ecc4  (from analysis/SLUS_005.10/decomp/8001ecc4.c) --- */
// addr: 0x8001ecc4  name: FUN_8001ecc4

int FUN_8001ecc4(int param_1,int param_2,MATRIX *param_3)
{
  int iVar1; uint *puVar2; MATRIX MStack_30;
  puVar2 = *(uint **)(param_2 + 0x38);
  do {
    if (puVar2 == (uint *)0x0) { return 0; }
    if ((puVar2[0x17] == 0) || ((*puVar2 & 0x20) != 0)) {
      if ((*puVar2 & 0x800) != 0) {
        CompMatrixLV(param_3,(MATRIX *)(puVar2 + 4),&MStack_30);
        goto LAB_8001ed78;
      }
    } else {
      CompMatrixLV(param_3,(MATRIX *)(puVar2 + 4),&MStack_30);
      iVar1 = FUN_8001e9a0(param_1,puVar2,param_1 + 0x10,&MStack_30);
      if (iVar1 != 0) { return iVar1; }
      if ((*puVar2 & 0x800) != 0) {
LAB_8001ed78:
        iVar1 = FUN_8001ecc4(param_1,puVar2,&MStack_30);
        if (iVar1 != 0) { return iVar1; }
      }
    }
    puVar2 = (uint *)puVar2[0xd];
  } while (true);
}

/* --- SLUS_005.10 FUN_8001edb4  (from analysis/SLUS_005.10/decomp/8001edb4.c) --- */
// addr: 0x8001edb4  name: FUN_8001edb4
/* (full body as shown above in Ghidra pseudoC) */

#endif  /* GHIDRA REF */
