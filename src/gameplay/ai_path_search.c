/* ai_path_search.c -- path-graph search and curve evaluation.
 *
 * Source: SLUS_005.10
 *   FUN_80042cdc  -- Path_NearestWaypoint (was UNKNOWN in unknowns.md
 *                    -- now HIGH per Ghidra port).
 *   FUN_80042724  -- Path_Sample: 8-iteration cubic Bezier subdivision.
 *
 * Road / waypoint graph layout (recovered from Ghidra):
 *
 *   iRam000008c0       int32  -- number of roads
 *   iRam000008d4       int32  -- base of road-pointer array (each
 *                                element is `Road *`).
 *
 *   Road struct (referenced as int *piVar9):
 *     +0x00 ..        int *backref-self
 *     +0x12 (short)   segment count
 *     +0x1c ...       segments (stride = sizeof(Segment))
 *
 *   Segment struct (piVar11):
 *     +0x1c (= piVar11[7])  pointer to waypoint pair (Waypoint *piVar8)
 *
 *   Waypoint struct (piVar8):
 *     +0x00 (int *)   owning-road backref (must equal road for match)
 *     +0x04 (int *)   first node pose (X at +0, Z at +8)
 *     +0x08 (ushort)  segment-type tag (matched against param_2 unless
 *                     param_2 == 0xffffffff = wildcard)
 *     +0x10 (int *)   second node pose
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>

/* ---- globals ---- */
extern int32_t  iRam000008c0;
extern uint32_t iRam000008d4;     /* base of road pointer array */

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

static int32_t mips_div2_rtz_i32(int32_t v)
{
    return mips_addu_i32(v, (int32_t)((uint32_t)v >> 31)) >> 1;
}

static int32_t mips_avg2_i32(int32_t a, int32_t b)
{
    return mips_div2_rtz_i32(mips_addu_i32(a, b));
}

/* ================================================================
 * FUN_80042cdc  -- Path_NearestWaypoint
 *
 * Searches every waypoint of every road, computing an L1 Manhattan
 * "outside-box" distance from `param_1` (a vec3 with x=+0, z=+8)
 * to the waypoint pair's bounding box.  Tracks the smallest distance
 * and returns the matching waypoint pointer (NULL if no match).
 *
 * `param_2`: segment-type filter; 0xffffffff = wildcard.
 * ================================================================ */
int *FUN_80042cdc(int *param_1, uint32_t param_2)
{
    int       iVar1, iVar2, iVar3, iVar4;
    uint32_t  uVar5, uVar7;
    int       iVar6, iVar10, iVar13, iVar14;
    int      *piVar8, *piVar9, *piVar11, *piVar15;
    uint32_t *puVar12;

    iVar14  = 0x7fffffff;
    iVar13  = 0;
    piVar15 = (int *)0;
    puVar12 = &iRam000008d4;

    if (0 < iRam000008c0) {
        do {
            piVar9 = (int *)(uintptr_t)*puVar12;
            iVar10 = 0;
            piVar11 = piVar9;
            if (0 < *(int16_t *)((uint8_t *)piVar9 + 0x12)) {
                do {
                    piVar8 = (int *)(uintptr_t)piVar11[7];
                    if (((int *)(uintptr_t)*piVar8 == piVar9) &&
                        ((param_2 == 0xffffffffu ||
                          (*(uint16_t *)(piVar8 + 2) == (uint16_t)param_2)))) {
                        /* Manhattan L1 outside-of-box on X axis. */
                        uVar5 = (uint32_t)(*(int *)(uintptr_t)piVar8[1] < *piVar9);
                        iVar1 = mips_subu_i32(*param_1,
                                              *(int *)(uintptr_t)piVar8[1 - uVar5]);
                        iVar6 = 0;
                        if (0 < iVar1) iVar6 = iVar1;
                        iVar2 = mips_subu_i32(*param_1,
                                              *(int *)(uintptr_t)piVar8[uVar5]);
                        iVar1 = 0;
                        if (iVar2 < 0) iVar1 = iVar2;

                        /* Same on Z axis. */
                        uVar7 = (uint32_t)(((int *)(uintptr_t)piVar8[1])[2] < piVar9[2]);
                        iVar3 = mips_subu_i32(param_1[2],
                                              *(int *)(uintptr_t)(piVar8[1 - uVar7] + 8));
                        iVar2 = 0;
                        if (0 < iVar3) iVar2 = iVar3;
                        iVar4 = mips_subu_i32(param_1[2],
                                              *(int *)(uintptr_t)(piVar8[uVar7] + 8));
                        iVar3 = 0;
                        if (iVar4 < 0) iVar3 = iVar4;

                        iVar6 = mips_addu_i32(mips_subu_i32(iVar6, iVar1),
                                              mips_subu_i32(iVar2, iVar3));
                        if (iVar6 < iVar14) {
                            iVar14  = iVar6;
                            piVar15 = piVar8;
                        }
                    }
                    iVar10  = mips_addu_i32(iVar10, 1);
                    piVar11 = piVar11 + 1;
                } while (iVar10 < *(int16_t *)((uint8_t *)piVar9 + 0x12));
            }
            iVar13  = mips_addu_i32(iVar13, 1);
            puVar12 = puVar12 + 1;
        } while (iVar13 < iRam000008c0);
    }
    return piVar15;
}

/* ================================================================
 * FUN_80042724  -- Path_Sample
 *
 * Bisects a cubic Bezier path defined by `param_1`'s control state
 * (two endpoints plus offsets) toward `param_2` (the target point).
 * Runs 8 De-Casteljau halvings; at each step picks the half whose
 * AABB-Manhattan distance to the target is smaller, and writes the
 * 4 sub-segment control points to `param_3`.  Returns the midpoint
 * of the final t-interval as a fixed-point 1.12 fraction in [0, 0x1000).
 *
 * Each control "endpoint" has x at +0 and z at +8 in its referenced
 * struct.  param_1 layout:
 *   [0] -> first endpoint pose ptr
 *   [1] -> second endpoint pose ptr
 *   [4..5] -- xz offset for the first endpoint's tangent
 *   [6..7] -- xz offset for the second endpoint's tangent
 *
 * HIGH confidence.
 * ================================================================ */
int FUN_80042724(int *param_1, int *param_2, int *param_3)
{
    int iVar1, iVar2, iVar3, iVar4, iVar5, iVar6, iVar7, iVar8, iVar9, iVar10;
    int iVar11, iVar12, iVar13;
    int local_88, local_84, local_80, local_7c;
    int local_78, local_74, local_70, local_6c;
    int local_68[20];

    local_88 = *(int *)(uintptr_t)*param_1;
    local_84 = *(int *)(uintptr_t)(*param_1 + 8);
    local_80 = mips_addu_i32(*(int *)(uintptr_t)*param_1, param_1[4]);
    local_7c = mips_addu_i32(*(int *)(uintptr_t)(*param_1 + 8), param_1[5]);
    local_78 = mips_addu_i32(*(int *)(uintptr_t)param_1[1], param_1[6]);
    local_74 = mips_addu_i32(*(int *)(uintptr_t)(param_1[1] + 8), param_1[7]);
    local_70 = *(int *)(uintptr_t)param_1[1];
    iVar12   = 0;
    local_6c = *(int *)(uintptr_t)(param_1[1] + 8);
    iVar11   = 0x1000;
    iVar13   = 0;

    do {
        local_68[0x10] = mips_avg2_i32(local_80, local_78);
        iVar3 = mips_avg2_i32(local_7c, local_74);
        local_68[0] = local_88;
        local_68[1] = local_84;
        local_68[0xe] = local_70;
        local_68[0xf] = local_6c;
        local_68[2] = mips_avg2_i32(local_88, local_80);
        local_68[3] = mips_avg2_i32(local_84, local_7c);
        local_68[4] = mips_avg2_i32(local_68[2], local_68[0x10]);
        local_68[5] = mips_avg2_i32(local_68[3], iVar3);
        local_78 = mips_avg2_i32(local_78, local_70);
        local_74 = mips_avg2_i32(local_74, local_6c);
        local_68[0xc] = local_78;
        local_68[0xd] = local_74;
        local_80 = mips_avg2_i32(local_78, local_68[0x10]);
        local_7c = mips_avg2_i32(local_74, iVar3);
        local_68[10] = local_80;
        local_68[0xb] = local_7c;
        local_68[8]  = mips_avg2_i32(local_68[4], local_80);
        local_68[9]  = mips_avg2_i32(local_68[5], local_7c);
        local_68[6]  = local_68[8];
        local_68[7]  = local_68[9];

        iVar3 = 0;
        if (local_68[8] < local_88) iVar3 = 3;
        iVar6 = 0;
        if (local_68[9] < local_84) iVar6 = 3;

        iVar9 = *param_2;
        iVar1 = 0;
        if (0 < mips_subu_i32(iVar9, local_68[mips_sll_i32(mips_subu_i32(3, iVar3), 1)]))
            iVar1 = mips_subu_i32(iVar9, local_68[mips_sll_i32(mips_subu_i32(3, iVar3), 1)]);
        iVar4 = 0;
        if (mips_subu_i32(iVar9, local_68[mips_sll_i32(iVar3, 1)]) < 0)
            iVar4 = mips_subu_i32(iVar9, local_68[mips_sll_i32(iVar3, 1)]);

        iVar10 = param_2[2];
        iVar3 = 0;
        if (0 < mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar6), 1), 1)]))
            iVar3 = mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar6), 1), 1)]);
        iVar7 = 0;
        if (mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(iVar6, 1), 1)]) < 0)
            iVar7 = mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(iVar6, 1), 1)]);

        iVar6 = 0;
        if (local_70 < local_68[8]) iVar6 = 3;
        iVar8 = 0;
        if (local_6c < local_68[9]) iVar8 = 3;

        iVar2 = 0;
        if (0 < mips_subu_i32(iVar9, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar6), 1), 8)]))
            iVar2 = mips_subu_i32(iVar9, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar6), 1), 8)]);
        iVar5 = 0;
        if (mips_subu_i32(iVar9, local_68[mips_addu_i32(mips_sll_i32(iVar6, 1), 8)]) < 0)
            iVar5 = mips_subu_i32(iVar9, local_68[mips_addu_i32(mips_sll_i32(iVar6, 1), 8)]);

        iVar6 = 0;
        if (0 < mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar8), 1), 9)]))
            iVar6 = mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(mips_subu_i32(3, iVar8), 1), 9)]);
        iVar9 = 0;
        if (mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(iVar8, 1), 9)]) < 0)
            iVar9 = mips_subu_i32(iVar10, local_68[mips_addu_i32(mips_sll_i32(iVar8, 1), 9)]);

        if (mips_addu_i32(mips_subu_i32(iVar1, iVar4), mips_subu_i32(iVar3, iVar7)) <
            mips_addu_i32(mips_subu_i32(iVar2, iVar5), mips_subu_i32(iVar6, iVar9))) {
            iVar11 = mips_avg2_i32(iVar12, iVar11);
            local_80 = local_68[2];
            local_7c = local_68[3];
            local_78 = local_68[4];
            local_74 = local_68[5];
            local_70 = local_68[8];
            local_6c = local_68[9];
        } else {
            iVar12 = mips_avg2_i32(iVar12, iVar11);
            local_88 = local_68[8];
            local_84 = local_68[9];
        }
        iVar13 = mips_addu_i32(iVar13, 1);
    } while (iVar13 < 8);

    param_3[0] = local_88;
    param_3[1] = local_84;
    param_3[2] = local_80;
    param_3[3] = local_7c;
    param_3[4] = local_78;
    param_3[5] = local_74;
    param_3[6] = local_70;
    param_3[7] = local_6c;
    return mips_avg2_i32(iVar12, iVar11);
}

/* Public aliases for semantic-name callers. */
int *Path_NearestWaypoint(int *pos, uint32_t typeMask)
    { return FUN_80042cdc(pos, typeMask); }

int  Path_Sample(int *seg, int *target, int *outCtrlPts)
    { return FUN_80042724(seg, target, outCtrlPts); }

/* ================================================================
 * FUN_80042f5c -- Path_FreeAttachedWaypoints
 *
 * If the path follow-state at `param_1+4` is non-NULL, heap-free it
 * and clear the slot.  Used during AI-path teardown.
 *
 * HIGH confidence.
 * ================================================================ */
extern void Heap_Free(void *p);     /* FUN_80045088 */

void FUN_80042f5c(int param_1)
{
    void *p = (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 4);
    if (p != (void *)0) {
        Heap_Free(p);
        *(uint32_t *)(uintptr_t)(param_1 + 4) = 0;
    }
}
