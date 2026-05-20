/* matrix_chain.c -- compose parent-chain matrices for hierarchical objects.
 *
 * Source: SLUS_005.10  FUN_8001d624.
 *
 * Walks an object's parent chain (linked via FUN_8001d5a0 which returns
 * obj->parent or NULL). For each parent, composes the parent's
 * matrix with the running result via PSY-Q's CompMatrixLV, accumulating
 * into the scratch slot at DAT_8006f640. Returns a pointer to the
 * final accumulated matrix (or the original object's matrix if it has
 * no parents).
 *
 * Used by hierarchical objects (e.g. crane base + crane arm + crane
 * hook, or a level door + hinge transform). Each part's draw position
 * is the product of all parent transforms.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern int   Object_Parent(int obj);                 /* FUN_8001d5a0 */
extern MATRIX *CompMatrixLV(MATRIX *a, MATRIX *b, MATRIX *out);
extern MATRIX DAT_8006f640;

MATRIX *Matrix_ComposeParentChain(int obj)
{
    MATRIX *m = (MATRIX *)(intptr_t)(obj + 0x10);
    for (;;) {
        obj = Object_Parent(obj);
        if (obj == 0) break;
        CompMatrixLV((MATRIX *)(intptr_t)(obj + 0x10), m, &DAT_8006f640);
        m = &DAT_8006f640;
    }
    return m;
}
