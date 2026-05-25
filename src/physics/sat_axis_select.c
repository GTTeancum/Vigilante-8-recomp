/* sat_axis_select.c -- Separating-axis selection for OBB-vs-OBB or
 * OBB-vs-poly collision.
 *
 * Source: SLUS_005.10  FUN_8001f5a0  (980 B).
 *
 * Given a collision-test descriptor `query` with two shape pointers
 * (the "host" obj's AABB and the "candidate" shape -- AABB or convex
 * polyhedron), iterates the candidate's face normals, calls
 * SAT_ProjectAxis (FUN_8001e6dc) for each, and tracks the axis with
 * the GREATEST overlap value (= least separation).  Per SAT, the
 * "least-separating" axis is the one collision response uses, since
 * if any axis truly separates we can early-out; if all overlap, the
 * smallest overlap is the shallowest penetration.
 *
 * Query record layout (output fields filled in-place on return):
 *
 *   +0x00  ???                          (header / state)
 *   +0x04  ptr -> shape #1  (host)      kind|count + AABB i32[6]  (kind 1)
 *   +0x08  ptr -> shape #2  (candidate) kind|count + AABB i32[6]  (kind 1)
 *                                       OR kind|nFaces + face_records  (kind 2)
 *                                       where each face_record is 12 bytes:
 *                                         i16[3] face_normal + pad + i32 half_extent
 *   +0x10  parent-chain ptr             (for ComposeParentChain on candidate)
 *   +0x14  i32  contact_X    (out: shape#1 vertex coord on best axis)
 *   +0x18  i32  contact_Y    (out)
 *   +0x1c  i32  contact_Z    (out)
 *   +0x20  i16[4] world_axis (out: world-space SAT separation axis)
 *   +0x28  i16[4] local_axis (out: vehicle-local-frame SAT axis)
 *   +0x30  i32  overlap      (out: best overlap value, INT_MIN if no
 *                             axis tested -- positive = separation,
 *                             negative = penetration depth)
 *
 * Returns the `query` pointer (for chaining), per MIPS `move v0, s5`.
 *
 * HIGH confidence: every branch + offset matches MIPS line-for-line.
 */
#include <stdint.h>
#include "structs.h"
#include "gte.h"

extern void *Matrix_ComposeParentChain(uint32_t parent_handle);   /* FUN_8001d624 */
extern void ApplyMatrixSV(const MATRIX *m, const SVECTOR *v0, SVECTOR *v1);
extern void GTE_RotateShortMatTranspose(uint32_t *m, const void *sv_in, void *sv_out);  /* FUN_800434f8 */
extern int  SAT_ProjectAxis(int32_t *obstacle_aabb, MATRIX *vehicle_mat,
                            const int16_t *axis_query, MATRIX *candidate_mat);

#define INT_MIN_I32  ((int32_t)0x80000000)

/* Build the i16[3] face-normal block for the s0'th canonical face of an
 * AABB: s0 in [0,3,1,4,2,5] -> (-X, +X, -Y, +Y, -Z, +Z).
 * MIPS uses a clever `xori s0, 0xN; sltiu v0, v0, 1; conditional shift`
 * pattern that produces +0x1000 / 0 / -0x1000 per s0.  We replicate the
 * value table directly. */
static inline void build_aabb_face_normal(int s0, int16_t out[3])
{
    out[0] = (s0 == 0) ? -0x1000 : (s0 == 3) ? +0x1000 : 0;
    out[1] = (s0 == 1) ? -0x1000 : (s0 == 4) ? +0x1000 : 0;
    out[2] = (s0 == 2) ? -0x1000 : (s0 == 5) ? +0x1000 : 0;
}

/* Write the contact-point fields and the local-frame axis fields of
 * the query record from the host AABB and current local-frame axis. */
static void write_result_contact_and_axes(uint8_t *query, int32_t *host_aabb)
{
    int16_t *local_axis = (int16_t *)(query + 0x28);
    int32_t *contact    = (int32_t *)(query + 0x14);

    /* Per axis component: < 0 -> pick MAX face vertex; >= 0 -> MIN face. */
    contact[0] = (local_axis[0] < 0) ? host_aabb[3] : host_aabb[0];
    contact[1] = (local_axis[1] < 0) ? host_aabb[4] : host_aabb[1];
    contact[2] = (local_axis[2] < 0) ? host_aabb[5] : host_aabb[2];
}

/* HIGH: SAT selection over all candidate face normals.  Returns the
 * query record pointer (caller pattern). */
void *SAT_SelectAxis(uint8_t *host_obj, uint8_t *query)
{
    /* Compose candidate's world matrix via its parent chain. */
    MATRIX *cand_mat = (MATRIX *)Matrix_ComposeParentChain(
        *(uint32_t *)(query + 0x10));

    /* Shape #1 (host's AABB descriptor). */
    int16_t *shape1 = (int16_t *)(uintptr_t)*(uint32_t *)(query + 0x4);
    if ((uint16_t)shape1[0] != 1) {
        /* Host shape is not kind=1 (AABB); we don't handle other kinds. */
        return query;
    }
    int32_t *host_aabb = (int32_t *)(shape1 + 2);     /* skip kind+count header */

    /* Shape #2 (candidate's shape: kind 1 = AABB, kind 2 = poly). */
    int16_t *shape2 = (int16_t *)(uintptr_t)*(uint32_t *)(query + 0x8);
    uint16_t cand_kind = (uint16_t)shape2[0];

    /* host_mat = host_obj + 0x10. */
    MATRIX *host_mat = (MATRIX *)(host_obj + 0x10);

    int32_t best_overlap = INT_MIN_I32;
    int     best_axis_idx = 0;

    if (cand_kind == 1) {
        /* AABB-vs-AABB: 6 candidate axes (±X, ±Y, ±Z). */
        int32_t *cand_aabb = (int32_t *)(shape2 + 2);

        for (int s0 = 0; s0 < 6; s0++) {
            /* Build the axis_query: i16[3] face_normal + pad + i32 half_extent. */
            int16_t axis_query[6] = {0,0,0,0, 0,0};  /* 12 bytes */
            build_aabb_face_normal(s0, &axis_query[0]);

            int32_t half_extent_val = cand_aabb[s0];
            if (s0 < 3) half_extent_val = -half_extent_val;
            *(int32_t *)((uint8_t *)axis_query + 8) = half_extent_val;

            int32_t r = SAT_ProjectAxis(host_aabb, host_mat,
                                        &axis_query[0], cand_mat);
            if (r > best_overlap) {
                best_overlap = r;
                best_axis_idx = s0;
            }
        }
    } else if (cand_kind == 2) {
        /* AABB-vs-poly: iterate the polyhedron's face records.
         * Each face record is 12 bytes at shape2 + 4 + i*12:
         *    i16[3] face_normal + pad + i32 half_extent. */
        uint16_t n_faces = (uint16_t)shape2[1];
        for (int s0 = 0; s0 < (int)n_faces; s0++) {
            int16_t *face_rec = (int16_t *)((uint8_t *)shape2 + 4 + s0 * 12);
            int32_t r = SAT_ProjectAxis(host_aabb, host_mat,
                                        face_rec, cand_mat);
            if (r > best_overlap) {
                best_overlap = r;
                best_axis_idx = s0;
            }
        }
    } else {
        /* Unknown candidate kind -- leave overlap = INT_MIN to signal
         * "no test performed". */
        return query;
    }

    /* Re-build axis_query for the chosen face and write the result
     * fields:
     *   +0x20  world_axis = candidate_mat * best_face_normal
     *   +0x28  local_axis = host_mat^T * best_face_normal
     *   +0x14  contact = host_aabb support vertex along local_axis
     *   +0x30  overlap = best_overlap
     */
    int16_t best_normal[4] = {0,0,0,0};
    if (cand_kind == 1) {
        build_aabb_face_normal(best_axis_idx, &best_normal[0]);
    } else {
        int16_t *face_rec = (int16_t *)((uint8_t *)shape2 + 4 + best_axis_idx * 12);
        best_normal[0] = face_rec[0];
        best_normal[1] = face_rec[1];
        best_normal[2] = face_rec[2];
    }

    /* world_axis = cand_mat * best_normal */
    ApplyMatrixSV(cand_mat, (SVECTOR *)best_normal,
                  (SVECTOR *)(query + 0x20));
    /* local_axis = host_mat^T * (world_axis) */
    GTE_RotateShortMatTranspose((uint32_t *)host_mat,
                                query + 0x20,
                                query + 0x28);

    write_result_contact_and_axes(query, host_aabb);
    *(int32_t *)(query + 0x30) = best_overlap;
    return query;
}

/* Legacy FUN_ alias. */
void *FUN_8001f5a0(intptr_t host_obj_int, intptr_t query_int)
{
    return SAT_SelectAxis((uint8_t *)(intptr_t)host_obj_int,
                          (uint8_t *)(intptr_t)query_int);
}
