/* obstacle_probe.c -- per-object obstacle override for terrain-probe.
 *
 * Source: SLUS_005.10
 *   FUN_8001f51c  Object_FindObstacleAt  (dispatcher, 124 B)
 *   FUN_8001f3ac  ObstacleChain_Walk     (recursive walker, 368 B)
 *   FUN_8001ef74  ObstacleLeaf_Test      (leaf interpreter, 1080 B)
 *
 * Each dynamic object can carry a hierarchical "obstacle tree" hung at
 * +0x38 (a linked list of children).  Each child may itself carry a
 * sub-tree at +0x5c (the kd-tree leaf root).  Terrain_HeightAndProbe
 * calls Object_FindObstacleAt to ask "is there a static obstacle
 * (door, ramp, drum, ...) at this XYZ that should override the
 * heightmap's Y?".
 *
 * The leaf tree is an interpreted instruction stream:
 *   *(i16 *)leaf == 0   -> terminator (end of stream)
 *   *(i16 *)leaf == 1   -> AABB trigger.  If the world XYZ -- after
 *                          transforming back into the leaf's local
 *                          frame via the parent matrix^T -- falls
 *                          inside the i32 box at leaf+4, return the
 *                          override Y stored in the box.
 *   *(i16 *)leaf == 2   -> convex polyhedron patch.  Iterate the
 *                          patch's faces; for each, project the world
 *                          XYZ along the face normal via GTE_RotateShort,
 *                          dot it with the face delta, divide by IR2
 *                          (the face-normal Y component) to get the
 *                          parametric t along the probe ray, track the
 *                          maximum (deepest) intersection.  If the
 *                          patch is hit and the resulting Y is within
 *                          the probe's vertical band, return it.
 *
 * Every operation is a line-by-line MIPS port; the GTE math (RTV0
 * and SetRotMatrix) preserves the engine's saturation behaviour.
 *
 * HIGH on structure; the kind=2 leaf math should get a MIPS-vs-cleaned
 * diff audit before any change to its constants (the 0x2800 vertical
 * band and the 0x800 dot-product threshold are PSX-tunables).
 */
#include <stdint.h>
#include <stddef.h>
#include "structs.h"
#include "gte.h"

extern long CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2);  /* PSY-Q / FUN_8004cf04 */
extern void SetRotMatrix(const MATRIX *m);
extern void gte_ldv0(const SVECTOR *sv);
extern void gte_rtv0(void);
extern int32_t gte_stIR1(void);
extern int32_t gte_stIR2(void);
extern int32_t gte_stIR3(void);
extern void GTE_RotateLongMatTranspose(uint32_t *m, const int32_t *v, int32_t *out);  /* FUN_8004352c */
extern void GTE_RotateLongMat(const uint32_t *m, const int32_t *v, int32_t *out);     /* FUN_80043358 */
extern uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z);                            /* FUN_800255f4 ish */
extern int32_t  Terrain_HeightAt(uint32_t x, uint32_t z);                              /* unused here */

/* Forward decl. */
int ObstacleLeaf_Test(int16_t *leaf, MATRIX *parent_mat,
                      int terrain_y, int *posXyz, int16_t *normalOut);

/* HIGH: walk the obstacle chain rooted at parent[+0x38], recursing
 * through children that have a kd-tree (+0x5c) or that have flag 0x800.
 * Returns the first non-zero override Y encountered, or 0 if none. */
int ObstacleChain_Walk(int *parent_obj, MATRIX *parent_mat,
                       int terrain_y, int *posXyz, int16_t *normalOut)
{
    int32_t *child = *(int32_t **)((uint8_t *)parent_obj + 0x38);

    while (child != NULL) {
        int32_t *kdroot = (int32_t *)*(uint32_t *)((uint8_t *)child + 0x5c);
        MATRIX   composed;

        if (kdroot != NULL) {
            CompMatrixLV(parent_mat, (MATRIX *)((uint8_t *)child + 0x10), &composed);

            /* Visibility test: the leaf is reachable if the child's
             * world-Y axis (composed M's Y row) points "up" OR if its
             * projection along the parent's (i16-at-0x12, -at-0x18,
             * -at-0x1e) reach vector exceeds 0x800.
             *
             * The constants at +0x12/0x18/0x1e are i16 components of
             * the parent object's collision sphere extent. */
            int reach_dot =
                  (int)composed.m[0][1] * (int)*(int16_t *)((uint8_t *)parent_obj + 0x12)
                + (int)composed.m[1][1] * (int)*(int16_t *)((uint8_t *)parent_obj + 0x18)
                + (int)composed.m[2][1] * (int)*(int16_t *)((uint8_t *)parent_obj + 0x1e);

            if ((composed.m[1][1] > 0) || (reach_dot > 0x800)) {
                int hit = ObstacleLeaf_Test((int16_t *)kdroot, &composed,
                                            terrain_y, posXyz, normalOut);
                if (hit != 0) return hit;
            }
        }

        /* If the child itself is marked collidable (flag 0x800), recurse. */
        if ((*(uint32_t *)child & 0x800u) != 0) {
            if (kdroot == NULL) {
                CompMatrixLV(parent_mat,
                             (MATRIX *)((uint8_t *)child + 0x10),
                             &composed);
            }
            int hit = ObstacleChain_Walk(child, &composed, terrain_y,
                                         posXyz, normalOut);
            if (hit != 0) return hit;
        }

        child = (int32_t *)*(uint32_t *)((uint8_t *)child + 0x34);
    }
    return 0;
}

/* HIGH: top-level obstacle dispatcher.  Routes via the chain walker
 * with the parent object's own matrix; falls into a direct test on
 * (child = parent[+0x5c]) when the parent itself is the leaf holder. */
int Object_FindObstacleAt(int *parent_obj, int terrain_y, int *posXyz, int16_t *normalOut)
{
    /* If parent has flag 0x800, walk its chain first. */
    if ((*(uint32_t *)parent_obj & 0x800u) != 0) {
        int hit = ObstacleChain_Walk(parent_obj,
                                     (MATRIX *)((uint8_t *)parent_obj + 0x10),
                                     terrain_y, posXyz, normalOut);
        if (hit != 0) return hit;
    }
    /* Always try the parent's own kd-tree at +0x5c (parent_obj[0x17]). */
    int16_t *kdroot = *(int16_t **)((uint8_t *)parent_obj + 0x5c);
    return ObstacleLeaf_Test(kdroot,
                             (MATRIX *)((uint8_t *)parent_obj + 0x10),
                             terrain_y, posXyz, normalOut);
}

/* HIGH on dispatch / structure; MED on the kind=2 GTE math constants
 * (the 0x2800 vertical band and per-face dot threshold).  Direct port
 * from the Ghidra pseudo-C; matches the MIPS at analysis/. */
int ObstacleLeaf_Test(int16_t *leaf, MATRIX *parent_mat,
                      int terrain_y, int *posXyz, int16_t *normalOut)
{
    if (leaf == NULL) return 0;

    /* Local frame: pos - parent_mat.t (used by kind=2 patch math). */
    int32_t local_pos[3];
    local_pos[0] = posXyz[0] - parent_mat->t[0];
    local_pos[1] = posXyz[1] - parent_mat->t[1];
    local_pos[2] = posXyz[2] - parent_mat->t[2];

    while (1) {
        int16_t kind = leaf[0];
        if (kind == 0) return 0;

        if (kind == 1) {
            /* AABB trigger: transform local_pos back into the leaf's
             * own local frame, then in-box test. */
            int32_t transformed[3];
            GTE_RotateLongMatTranspose((uint32_t *)parent_mat, local_pos, transformed);

            /* Box is at leaf+4 (i32[6] arranged as min/max with a non-
             * obvious interleave -- match the Ghidra pseudo-C verbatim).
             * Field offsets are short-array indexed in the pseudo-C:
             *   psVar12+2 = byte 4   -> minX
             *   psVar12+6 = byte 12  -> minZ
             *   psVar12+8 = byte 16  -> maxX
             *   psVar12+0xa = byte 20 -> maxY
             *   psVar12+0xc = byte 24 -> maxZ
             *
             * The pseudo-C tests:
             *   if (max[i] <= scratch[i]  ||  scratch[i] <= min[i])  miss
             */
            int8_t *base = (int8_t *)leaf;
            int32_t minX = *(int32_t *)(base + 4);
            int32_t minZ = *(int32_t *)(base + 12);
            int32_t maxX = *(int32_t *)(base + 16);
            int32_t maxY = *(int32_t *)(base + 20);
            int32_t maxZ = *(int32_t *)(base + 24);
            int32_t override_y = *(int32_t *)(base + 8);  /* minY field stores the override */

            if (maxX <= transformed[0] || transformed[0] <= minX ||
                maxZ <= transformed[2] || transformed[2] <= minZ ||
                maxY <= transformed[1]) {
                /* Miss this box; advance leaf past its record (14 bytes
                 * = 7 shorts, matches the MIPS `psVar12 + 0xe`). */
                leaf = leaf + 14;
                continue;
            }
            /* In-box X/Z; final Y test against the candidate y_override. */
            int32_t y_check = transformed[1];

            /* The "near-band" check: if the probe is well above the
             * override Y (probe_y - override_y > 0x10000), AND the
             * box's stored normal Y is significant, fall through to
             * the override.  Otherwise, accept the box.
             *
             * Match Ghidra: tests `(parent_mat.t[1] + override_y < terrain_y) || (probe_y + 0x10000 < terrain_y)`. */
            if (parent_mat->t[1] + override_y >= terrain_y &&
                terrain_y + 0x10000 >= posXyz[1]) {
                /* Surface-material check via terrain attr: only override
                 * when not over an "ignore" material. */
                uint8_t *mat = Terrain_MaterialAt((uint32_t)posXyz[0],
                                                  (uint32_t)posXyz[2]);
                if (*(int16_t *)(mat + 2) != 0) {
                    leaf = leaf + 14;
                    continue;
                }
            }

            if (normalOut != NULL) {
                normalOut[0] = (int16_t)(-parent_mat->m[0][1]);
                normalOut[1] = (int16_t)(-parent_mat->m[1][1]);
                normalOut[2] = (int16_t)(-parent_mat->m[2][1]);
            }
            return parent_mat->t[1] + override_y;
        }

        if (kind == 2) {
            /* Convex-polyhedron patch.  Each face is described by an
             * SVECTOR face_normal + extra fields (12 bytes total per
             * face).  Per-face math:
             *   1. RotateShort(face_normal) by parent_mat -> world_n
             *   2. dot   = (i64) world_n . (probe_world_pos - face_offset)
             *   3. t     = (dot >> 32 sign-correct) / IR2 (world_n.y)
             *   4. Track per-face min/max bounds.
             * Returns parent_mat.t[1] + t when the probe lands inside
             * the patch (sign tests on IR2 control direction). */
            uint16_t n_faces = (uint16_t)leaf[1];
            int32_t  best_t  = 0x7fff0000;
            int32_t  best_lo = 0;
            int16_t  best_n[3] = {0, 0, 0};

            SetRotMatrix(parent_mat);

            uint8_t *face_p = (uint8_t *)leaf + 4;
            for (uint16_t face_idx = 0; face_idx < n_faces; face_idx++) {
                SVECTOR *fn = (SVECTOR *)face_p;
                gte_ldv0(fn);
                gte_rtv0();

                int32_t ir1 = gte_stIR1();
                int32_t ir2 = gte_stIR2();
                int32_t ir3 = gte_stIR3();

                int16_t fn_extra_x = *(int16_t *)(face_p + 8);
                int16_t fn_extra_y = *(int16_t *)(face_p + 10);  /* unused in MIPS body proper */
                (void)fn_extra_y;

                /* world_normal . (local_pos - face_offset).  The face's
                 * 3 offsets live at face_p+0/2/4 (the SVECTOR's xyz)
                 * scaled by 0x1000 (q12); the local_pos is i32 q15.16.
                 * MIPS does the full 64-bit multiply with carry. */
                int64_t lv1 = (int64_t)ir1 * (int64_t)local_pos[0];
                int64_t lv2 = (int64_t)ir3 * (int64_t)local_pos[2];
                int64_t sum = (int64_t)((int32_t)fn_extra_x) * 0x1000
                            - lv1 - lv2;

                if (ir2 < 0) {
                    /* Plane normal points "down"; only consider faces
                     * whose ray-param yields a finite negative t. */
                    int32_t t = (int32_t)(sum / (int64_t)ir2);
                    if (t < best_t) {
                        best_t   = t;
                        best_n[0] = (int16_t)ir1;
                        best_n[1] = (int16_t)ir2;
                        best_n[2] = (int16_t)ir3;
                    }
                } else if (ir2 > 0) {
                    int32_t t = (int32_t)(sum / (int64_t)ir2);
                    if (t < *(int32_t *)(face_p + 4)) {
                        /* Below the face's lower bound -- short-circuit
                         * "miss". */
                        goto miss_face;
                    }
                    if (t < best_t) best_t = t;
                } else {
                    /* ir2 == 0: ray parallel to face plane along Y.
                     * If `sum` is negative the probe is on the "outside"
                     * of this face -- early miss. */
                    if (sum < 0) goto miss_face;
                }
                /* fall-through: keep iterating */
                face_p += 12;
                continue;

            miss_face:
                /* Skip remaining faces of this patch. */
                leaf = (int16_t *)((uint8_t *)leaf + (size_t)n_faces * 12 + 4);
                goto next_leaf;
            }

            /* All faces visited; if best_t resolves to a real
             * intersection inside the band, return it. */
            if (best_t < parent_mat->t[1] + 0 /* sentinel */) {
                int32_t y_world = parent_mat->t[1] + best_t;
                if (y_world < terrain_y && (posXyz[1] - 0x2800) < y_world && best_n[1] < -0x800) {
                    if (normalOut != NULL) {
                        normalOut[0] = best_n[0];
                        normalOut[1] = best_n[1];
                        normalOut[2] = best_n[2];
                    }
                    return y_world;
                }
            }
            leaf = (int16_t *)((uint8_t *)leaf + (size_t)n_faces * 12 + 4);
            continue;

        next_leaf:
            continue;
        }

        /* Unknown kind -- per MIPS, this is an infinite loop on the
         * same instruction.  In practice never reached in valid data. */
        return 0;
    }
}

/* Legacy FUN_ aliases. */
int FUN_8001f51c(int *parent_obj, int terrain_y, int *posXyz, int16_t *normalOut)
{
    return Object_FindObstacleAt(parent_obj, terrain_y, posXyz, normalOut);
}
int FUN_8001f3ac(int parent_obj_int, MATRIX *parent_mat, int terrain_y,
                 int *posXyz, int16_t *normalOut)
{
    return ObstacleChain_Walk((int *)(intptr_t)parent_obj_int, parent_mat,
                              terrain_y, posXyz, normalOut);
}
int FUN_8001ef74(int16_t *leaf, MATRIX *parent_mat, int terrain_y,
                 int *posXyz, int16_t *normalOut)
{
    return ObstacleLeaf_Test(leaf, parent_mat, terrain_y, posXyz, normalOut);
}
