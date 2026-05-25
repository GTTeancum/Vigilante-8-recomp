/* vehicle_target.c -- Player target retargeting.
 *
 * Source: SLUS_005.10  FUN_8002ea94  (155 instr, 0x9C B)
 *
 * Called from Vehicle_Kill when an enemy kill is registered, so the
 * player's cross-hair can immediately lock onto the next nearest threat.
 *
 * a0 = player (Vehicle *)
 * a1 = flag   (1 = allow clearing target if no candidate found; 0 = no clear)
 *
 * Algorithm:
 *   1. Select a camera MATRIX for GTE context:
 *        if iRam00000010 != 0 (split-screen active) AND player->status == -1
 *            use DAT_8006f6a0 (alternate split-screen matrix)
 *        else use DAT_8006f680 (normal viewport matrix)
 *      Load it into the GTE via SetRotMatrix + SetTransMatrix.
 *
 *   2. Iterate the active-object list anchored at piRam00000714.
 *      Node layout: [+0]=next_node_ptr (u32), [+4]=back_ptr (u32),
 *                   [+8]=object_ptr (u32).
 *      Two-pointer iteration: cur=current node, nxt=lookahead next;
 *      advance both each step; stop when nxt becomes null.
 *      Filter: skip self, kind@+4==3, no flag 0x4000, wrong team.
 *      Accept: status > 0 (enemy), or status <= 0 with cRam00000015 == 3.
 *      For each accepted candidate project world pos (obj+0x48) into
 *      camera space via FUN_800432d0:
 *        dist2d = max(|cam_y >> 10|, |cam_x >> 10|)   (screen offset)
 *        depth  = cam_z >> 10                          (signed depth)
 *
 *      Primary track (screen-centre): if dist2d < depth (on-screen):
 *        Cross-multiply: take current if
 *            best_scr_z * dist2d < depth * best_scr_dist
 *        (= current has smaller angular offset from centre).
 *
 *      Fallback track (3D distance): if dist2d >= depth AND no screen
 *        candidate yet, compute 3D distance via FUN_80016aac and track
 *        the closest.
 *
 *   3. Winner: screen candidate if found, else 3D fallback.
 *      No-op if winner == current target.
 *      No-op if winner == NULL and flag == 0.
 *
 *   4. Write new target into player+0xe4 (currentTarget).
 *      Zero player+0xbc (target timer, i16 at player+0xa4+0x18).
 *      Dispatch mode 0xA to the reticle sub-object selected by
 *      player+0xb3 (damageBits): node = player[0xec + (damageBits+9)*4].
 *
 * HIGH confidence: line-by-line MIPS port.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

/* Camera matrices for GTE setup.  Each is a MATRIX (18 B rot + 2 B pad +
 * 12 B trans = 32 B).  The two structs sit 0x20 bytes apart in the EXE. */
extern const MATRIX DAT_8006f680;   /* normal (single-screen) viewport */
extern const MATRIX DAT_8006f6a0;   /* split-screen / special-status   */

/* Active-object list head pointer (gp+0x714 = 0x80000714).
 * piRam00000714 IS the first node; its [0] field = next node ptr,
 * [2] field (byte +8) = payload object ptr. */
extern uint32_t *piRam00000714;
extern uint8_t DAT_80065a18[];

typedef struct TargetHostNode {
    struct TargetHostNode *next;
    struct TargetHostNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} TargetHostNode;

static TargetHostNode *target_list_first(void *listHead)
{
    TargetHostNode *sentinel = (TargetHostNode *)listHead;
    if (sentinel == NULL || sentinel->prev == NULL)
        return NULL;
    return sentinel->next;
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static uint32_t mips_addu_u32(uint32_t a, uint32_t b)
{
    return a + b;
}

static uint32_t mips_sll_u32(uint32_t v, unsigned sh)
{
    return v << sh;
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int32_t mips_abs_i32(int32_t v)
{
    return v < 0 ? mips_subu_i32(0, v) : v;
}

/* Split-screen mode flag (0=off, 1=horizontal, 2=vertical). */
extern int32_t iRam00000010;

/* Game mode / active-player-count byte. */
extern int8_t cRam00000015;
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* FUN_800432d0 -- apply current GTE rot+trans to a world i32[3] position
 * and write the resulting camera-space i32[3] into *out_xyz.
 * The three output components are stored at out+0, out+4, out+8.
 * Returns out_xyz. */
extern void *FUN_800432d0(const int32_t *world_xyz, int32_t *out_xyz);

/* FUN_80016aac -- unsigned 3D Euclidean distance between two i32[3] vecs. */
extern uint32_t FUN_80016aac(const int32_t *a, const int32_t *b);

void FUN_8002ea94(uint32_t *player, int flag)
{
    uint8_t *s = (uint8_t *)player;

    /* ------------------------------------------------------------------
     * 1. Load camera MATRIX into the GTE.
     * ------------------------------------------------------------------ */
    const MATRIX *mat;
    if (iRam00000010 != 0 && *(int16_t *)(s + 6) == (int16_t)-1)
        mat = &DAT_8006f6a0;
    else
        mat = &DAT_8006f680;

    gte_set_rot_matrix(mat);
    gte_set_translation((const VECTOR *)mat->t);   /* = SetTransMatrix(mat) */

    /* ------------------------------------------------------------------
     * 2. Iterate active-object list.
     * ------------------------------------------------------------------ */
    uint32_t *best_scr_obj  = NULL;           /* s2 */
    uint32_t  best_scr_dist = 0;              /* s7 */
    int32_t   best_scr_z    = 0;              /* s6 */
    uint32_t *best_3d_obj   = NULL;           /* s5 */
    uint32_t  best_3d_dist  = (uint32_t)-1;   /* s4 = 0xFFFFFFFF (infinity) */

    TargetHostNode *node = target_list_first(piRam00000714 ? (void *)piRam00000714 : (void *)DAT_80065a18);
    if (!node)
        goto select;

    for (; node != NULL; node = node->next) {
        /* Current node's payload object. */
        uint32_t *obj = (uint32_t *)node->payload;
        if (!obj)
            goto advance;

        /* -------- Filters -------- */
        if (obj == player)
            goto advance;

        /* kind byte at +4 -- skip if == 3 */
        if (*(uint8_t *)((uint8_t *)obj + 4) == 3)
            goto advance;

        /* Must have flag 0x4000 (active/targetable). */
        if (!(obj[0] & 0x4000u))
            goto advance;

        {
            int16_t st = *(int16_t *)((uint8_t *)obj + 6);
            if (st <= 0 && cRam00000015 != 3)
                goto advance;
        }

        /* -------- GTE world→camera projection -------- */
        {
            int32_t cam[3];
            FUN_800432d0((const int32_t *)((uint8_t *)obj + 0x48), cam);

            /* abs(cam_y >> 10) → then abs(cam_x >> 10); dist2d = max */
            int32_t ay = mips_abs_i32(cam[1] >> 10);
            int32_t ax = mips_abs_i32(cam[0] >> 10);
            uint32_t dist2d = (uint32_t)((ay > ax) ? ay : ax);  /* a0 */
            int32_t  depth  = cam[2] >> 10;                      /* v1 */

            if ((int32_t)dist2d < depth) {
                /* --- On-screen candidate: minimise angular offset --- */
                if (!best_scr_obj) {
                    /* First on-screen hit: take unconditionally. */
                    best_scr_obj  = obj;
                    best_scr_dist = dist2d;
                    best_scr_z    = depth;
                } else {
                    /* Cross-multiply comparison.
                     * Current is better if: best_z * dist2d < depth * best_dist
                     * i.e., smaller dist2d/depth ratio (closer to crosshair). */
                    int32_t lhs = mips_mult_lo_i32(best_scr_z, (int32_t)dist2d);  /* t0 */
                    int32_t rhs = mips_mult_lo_i32(depth, (int32_t)best_scr_dist); /* v0 */
                    if (lhs < rhs) {
                        best_scr_obj  = obj;
                        best_scr_dist = dist2d;
                        best_scr_z    = depth;
                    }
                    /* Not better: fall through to advance (skips 3D path). */
                }
            } else {
                /* --- Off-screen: 3D world-distance fallback --- */
                /* Only try if no on-screen candidate has been found. */
                if (best_scr_obj)
                    goto advance;
                {
                    uint32_t d = FUN_80016aac(
                        (const int32_t *)(s + 0x24),
                        (const int32_t *)((uint8_t *)obj + 0x48));
                    if (d < best_3d_dist) {
                        best_3d_dist = d;
                        best_3d_obj  = obj;
                    }
                }
            }
        }

    advance:
        ;
    }

select:
    /* ------------------------------------------------------------------
     * 3. Choose winner: on-screen first, 3D fallback if none.
     * ------------------------------------------------------------------ */
    if (!best_scr_obj)
        best_scr_obj = best_3d_obj;
    uint32_t *new_tgt = best_scr_obj;

    /* No change -- bail out. */
    uint32_t *old_tgt = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xe4);
    if (new_tgt == old_tgt)
        return;

    /* Only clear existing target when flag permits. */
    if (!new_tgt && !flag)
        return;

    /* ------------------------------------------------------------------
     * 4. Commit new target and notify the reticle sub-object.
     * ------------------------------------------------------------------ */
    *(uint32_t *)(s + 0xe4) = (uint32_t)(uintptr_t)new_tgt;

    /* player+0xa4 block: zero the target timer at [+0x18] = player+0xbc. */
    uint8_t *blk = s + 0xa4;
    *(int16_t *)(blk + 0x18) = 0;                      /* player->0xbc = 0 */

    /* Select reticle sub-object by damageBits (+0xb3) node index:
     *   node_ptr = player[ 0xec + (damageBits + 9) * 4 ]             */
    uint8_t  idx     = *(uint8_t *)(blk + 0x0f);       /* player->0xb3 */
    uint32_t off     = mips_sll_u32(mips_addu_u32((uint32_t)idx, 9u), 2);
    uint32_t *reticle = (uint32_t *)(uintptr_t)*(uint32_t *)(s + off + 0xec);
    if (!reticle)
        return;

    typedef int (*TickFn)(uint32_t *, int, intptr_t);
    TickFn cb = (TickFn)Object_CallbackFromPsxSlot(reticle);
    if (!cb)
        return;
    cb(reticle, 0xa, 0);    /* mode 0xA = target-change event */
}

/* ============================================================
 * FUN_8002ed34  FindNextTarget  (89 instr, ~356 B)
 *
 * Source: SLUS_005.10  FUN_8002ed34
 *
 * a0 = self      (Vehicle *)
 * a1 = hint      (Vehicle * or NULL)
 *
 * Finds the best retarget candidate from the active-object list at
 * piRam00000714, applying the same filters as FUN_8002ea94 (not self,
 * kind != 3, flag 0x4000 set, status > 0 or cRam00000015 == 3).
 *
 * Distance metric: FUN_80016aac (3D world distance) from self+0x24
 * (player world pos) to each candidate at obj+0x48.
 *
 * Two tracking slots:
 *   best_far  (s5/s2): nearest object whose dist > hint_dist.
 *             Represents "next farther target" for cycling.
 *   best_near (s6/s3): absolute nearest object overall (fallback).
 *             Also updated when a far-object doesn't improve best_far.
 *
 * If hint is NULL, hint_dist = 0 so ALL objects are "far".
 *
 * Returns: best_far if non-null, else best_near.
 *
 * HIGH confidence: line-by-line MIPS port.
 * ============================================================ */
uint32_t *FUN_8002ed34(uint32_t *self, uint32_t *hint)
{
    uint8_t  *s = (uint8_t *)self;

    uint32_t *best_far  = NULL;              /* s5 */
    uint32_t  far_dist  = (uint32_t)-1;      /* s2 */
    uint32_t *best_near = NULL;              /* s6 */
    uint32_t  near_dist = (uint32_t)-1;      /* s3 */

    /* hint_dist: distance from self to hint, or 0 (s4). */
    uint32_t hint_dist = 0;
    if (hint)
        hint_dist = FUN_80016aac((const int32_t *)(s + 0x24),
                                 (const int32_t *)((uint8_t *)hint + 0x48));

    /* Walk the active-object list. */
    TargetHostNode *node = target_list_first(piRam00000714 ? (void *)piRam00000714 : (void *)DAT_80065a18);
    if (!node)
        goto select;

    for (; node != NULL; node = node->next) {
        uint32_t *obj = (uint32_t *)node->payload;
        if (!obj)
            goto advance;

        /* Filters */
        if (obj == self)                                    goto advance;
        if (*(uint8_t *)((uint8_t *)obj + 4) == 3)         goto advance;
        if (!(obj[0] & 0x4000u))                            goto advance;
        {
            int16_t st = *(int16_t *)((uint8_t *)obj + 6);
            if (st <= 0 && cRam00000015 != 3)               goto advance;
        }

        /* Compute 3D world distance from player to this object. */
        {
            uint32_t dist = FUN_80016aac(
                (const int32_t *)(s + 0x24),
                (const int32_t *)((uint8_t *)obj + 0x48));

            if (hint_dist < dist) {
                /* Object is farther from player than hint (s4 < dist):
                 * update best_far if this is closer than current best_far. */
                if (dist < far_dist) {
                    best_far = obj;
                    far_dist = dist;
                    /* Jump to advance -- do NOT update near tracker. */
                    goto advance;
                }
                /* dist >= far_dist: fall through to near check. */
            }
            /* Either dist <= hint_dist, or dist >= far_dist:
             * update absolute-nearest tracker. */
            if (dist < near_dist) {
                best_near = obj;
                near_dist = dist;
            }
        }

    advance:
        ;
    }

select:
    return best_far ? best_far : best_near;
}

/* ================================================================
 * FUN_80023394 -- ObjList_FindNearestWithFlags
 *
 * Walks a linked object list and returns the payload pointer of the
 * first entry with:
 *   - collision layer  > 0x1f  (*(short*)(obj+6) > 0x1f)
 *   - flag 0x4000 set
 *   - flag mask param_2 intersects obj flags
 *   - 3D distance from param_3 is smallest so far
 *
 * param_1: list sentinel (param_1[0]=first node ptr)
 * param_2: flag mask
 * param_3: reference position (int32_t[3] at +0x48 of object)
 *
 * Returns nearest matching object pointer or NULL.
 *
 * HIGH confidence.
 * ================================================================ */
uint32_t *FUN_80023394(int *param_1, uint32_t param_2, intptr_t param_3)
{
    uint32_t  uVar3;
    uint32_t *puVar4;
    uint32_t  uVar5;
    uint32_t *puVar6;
    TargetHostNode *node;

    uVar5 = 0xffffffff;
    puVar6 = NULL;
    for (node = target_list_first(param_1); node != NULL; node = node->next) {
        puVar4 = (uint32_t *)node->payload;
        if (puVar4 == NULL)
            continue;
        if ((((0x1f < *(int16_t *)((uintptr_t)puVar4 + 6)) &&
              ((*puVar4 & 0x4000) != 0)) &&
             ((*puVar4 & param_2) != 0)) &&
            (uVar3 = (uint32_t)FUN_80016aac((const int32_t *)(uintptr_t)param_3,
                                             (const int32_t *)(puVar4 + 0x12)),
             uVar3 < uVar5)) {
            uVar5 = uVar3;
            puVar6 = puVar4;
        }
    }
    return puVar6;
}
