/* hierarchy_transform.c -- compose object hierarchy + GTE load.
 *
 * Source: LOAD.DLL  FUN_80101574.
 *
 * Walks the sibling chain at obj+0x34. For each node:
 *   1. CompMatrixLV(parent_world_matrix, node->local_matrix @ +0x10,
 *                   scratch_matrix)
 *   2. GTE_LoadTransform(scratch_matrix)
 *   3. If node->geomPair (+0x30) is non-NULL and its flag bit 0 is
 *      set, swap that flag: clear bit 0, set bit 2. Same for the
 *      texPair at +0x68. This signals "geometry is live -- consume it
 *      now, mark as taken".
 *
 * The traversal moves to the next sibling via +0x34. Used during
 * per-frame draw to push every leaf's transform onto the GTE for
 * rendering.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

extern void CompMatrixLV(void *a, void *b, void *out);
extern void GTE_LoadTransform(const uint32_t *m);

void Hierarchy_TransformChain(int obj, void *parentMatrix)
{
    uint32_t scratch[8];
    while (obj != 0) {
        CompMatrixLV(parentMatrix, (void *)(intptr_t)(obj + 0x10), scratch);
        GTE_LoadTransform(scratch);
        uint16_t *geom = *(uint16_t **)(intptr_t)(obj + 0x30);
        if (geom != NULL && (geom[0] & 1u) != 0) {
            geom[0] = (uint16_t)((geom[0] & 0xfffeu) | 4u);
        }
        uint16_t *tex = *(uint16_t **)(intptr_t)(obj + 0x68);
        if (tex != NULL && (tex[0] & 1u) != 0) {
            tex[0] = (uint16_t)((tex[0] & 0xfffeu) | 4u);
        }
        obj = *(int *)(intptr_t)(obj + 0x34);
    }
}
