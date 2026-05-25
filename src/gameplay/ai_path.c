/* ai_path.c -- AI path-follow waypoint state machine.
 *
 * Source: SLUS_005.10
 *   FUN_80042e78  -- Path_SetWaypointList: store waypoint pointer in the
 *                    path-state block, free any previous list, and prime
 *                    the current-target (.x12.16) and current-target-z
 *                    (.x14.16) fields from the first waypoint.
 *   FUN_80024d30  -- QuadTree_AStarPath_Trampoline: PSX scratchpad save
 *                    of return-addr/sp then tail-call FUN_80024d54 with
 *                    args passed through registers. On host the scratchpad
 *                    is plain memory and we just forward the arguments.
 *   FUN_80042ef0  -- Path_Init: compute a fresh A* path between two cells,
 *                    install the resulting waypoint list via Path_SetWaypointList.
 *   FUN_80042f98  -- Path_AngleDelta: advance to next waypoint when within
 *                    `range` of the current one; return signed angle delta
 *                    from the vehicle's heading to the bearing of the current
 *                    waypoint in body-space.
 *
 * Path-state block layout (uint16_t *blk):
 *   blk[0]    : u16  active flag (0 = no path)
 *   blk[1]    : u16  current waypoint index
 *   blk[2..3] : ptr  waypoint list ((short *)blk[2])
 *   blk[4..5] : u32  current target X (.16 fixed = (*wp << 16))
 *   blk[6..7] : u32  current target Z (.16 fixed = (wp[1] << 16))
 *
 * Waypoint list layout (short *wp):
 *   wp[0]    : i16  count (negative = end-of-list sentinel)
 *   wp[1]    : u16  next index (wraps via list table)
 *   wp[2..3] : ptr  packed XZ table base
 *   wp[4..5] : u32  current xy (cached)
 *   wp[6..7] : u32  current xz (cached)
 *
 * HIGH confidence (direct Ghidra port; minor Ghidra ambiguity on
 * dropped FUN_80045088 argument noted inline).
 */
#include <stdint.h>

extern void  Heap_Free(void *p);                                  /* FUN_80045088 */
extern int   FUN_80024d54(intptr_t param_1, intptr_t param_2,
                          uint32_t param_3, int param_4);          /* QuadTree_AStarPath */
extern int  *FUN_8004352c(uint32_t *m, int32_t *v, int32_t *out); /* GTE matrix transform */
extern long  ratan2(long y, long x);

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

/* ================================================================
 * FUN_80042e78 -- Path_SetWaypointList
 * ================================================================ */
int FUN_80042e78(uint16_t *param_1, int16_t *param_2)
{
    /* Free any previous waypoint list. Ghidra shows FUN_80045088() with
     * no arg; the MIPS preserves $a0 from the prior load, freeing the
     * old waypoint pointer. Preserve verbatim semantics. */
    if (*(uint32_t *)(uintptr_t)(param_1 + 2) != 0) {
        Heap_Free((void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 2));
    }
    *(uint32_t *)(uintptr_t)(param_1 + 2) = (uint32_t)(uintptr_t)param_2;
    param_1[1] = 0;
    *param_1   = (uint16_t)(param_2 != (int16_t *)0);
    if (param_2 != (int16_t *)0) {
        *(int32_t *)(uintptr_t)(param_1 + 4) = (int32_t)*param_2     << 16;
        *(int32_t *)(uintptr_t)(param_1 + 6) = (int32_t)param_2[1]   << 16;
    }
    return (int)(int16_t)*param_1;
}

/* ================================================================
 * FUN_80024d30 -- QuadTree_AStarPath_Trampoline
 *
 * On PSX, this saves $ra/$sp to the scratchpad then tail-calls
 * FUN_80024d54 with the original 4 args still in $a0-$a3. On host
 * we forward directly.
 * ================================================================ */
uint32_t FUN_80024d30(intptr_t start_cell, intptr_t target_cell,
                      uint32_t arg3, int arg4)
{
    return (uint32_t)FUN_80024d54(start_cell, target_cell, arg3, arg4);
}

/* ================================================================
 * FUN_80042ef0 -- Path_Init
 *
 * Compute an A* path from the supplied start/target cells; if no
 * path exists, snap the path-state to the literal target position.
 * ================================================================ */
void FUN_80042ef0(intptr_t param_1, intptr_t param_2, int32_t *param_3,
                  int param_4, int param_5)
{
    uint32_t uVar1;
    int      iVar2;

    uVar1 = FUN_80024d30(param_2, (intptr_t)param_3, (uint32_t)param_4, param_5);
    iVar2 = FUN_80042e78((uint16_t *)(uintptr_t)param_1, (int16_t *)(uintptr_t)uVar1);
    if (iVar2 == 0) {
        *(int32_t *)(uintptr_t)(param_1 + 8) = *param_3;
        *(int32_t *)(uintptr_t)(param_1 + 0xc) = param_3[2];
    }
}

/* ================================================================
 * FUN_80042f98 -- Path_AngleDelta
 *
 * If the vehicle is within `param_3` of the current waypoint in both
 * X and Z, advance to the next waypoint (or sentinel -1 if the list
 * is exhausted). Then transform the world-space delta to the current
 * waypoint into the vehicle's body frame (FUN_8004352c) and return
 * the bearing as an int16 angle.
 * ================================================================ */
int FUN_80042f98(intptr_t param_1, int16_t *param_2, int param_3)
{
    int16_t  sVar1 = 0;
    int      iVar2;
    long     lVar3;
    uint32_t uVar4;
    int32_t  local_18[4];

    iVar2 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 4),
                          *(int32_t *)(uintptr_t)(param_1 + 0x24));
    if (iVar2 < 0) iVar2 = mips_subu_i32(0, iVar2);
    if (iVar2 < param_3) {
        iVar2 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 6),
                              *(int32_t *)(uintptr_t)(param_1 + 0x2c));
        if (iVar2 < 0) iVar2 = mips_subu_i32(0, iVar2);
        if (iVar2 < param_3) {
            if (0 < *param_2) {
                sVar1 = param_2[1];
                param_2[1] = (int16_t)mips_addu_i32(sVar1, 1);
                uVar4 = *(uint32_t *)(uintptr_t)(
                          (((int32_t)((uint32_t)(uint16_t)mips_addu_i32(sVar1, 1) << 16) >> 14)) +
                          *(int32_t *)(uintptr_t)(param_2 + 2));
                if (uVar4 != 0) {
                    *(uint32_t *)(uintptr_t)(param_2 + 4) = (uint32_t)mips_sll_i32((int32_t)uVar4, 16);
                    *(uint32_t *)(uintptr_t)(param_2 + 6) = uVar4 & 0xffff0000u;
                    goto LAB_80043048;
                }
            }
            *param_2 = -1;
        }
    }
LAB_80043048:
    local_18[1] = 0;
    local_18[0] = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 4),
                                *(int32_t *)(uintptr_t)(param_1 + 0x24));
    local_18[2] = mips_subu_i32(*(int32_t *)(uintptr_t)(param_2 + 6),
                                *(int32_t *)(uintptr_t)(param_1 + 0x2c));
    FUN_8004352c((uint32_t *)(uintptr_t)(param_1 + 0x10), local_18, local_18);
    lVar3 = ratan2(local_18[0], local_18[2]);
    (void)sVar1;
    /* Sign-extend lVar3 from 12 bits: (x << 20) >> 20. */
    return (int)((lVar3 << 0x14) >> 0x14);
}
