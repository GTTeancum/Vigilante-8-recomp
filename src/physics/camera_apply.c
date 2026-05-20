/* camera_apply.c -- write current frame's camera basis into render globals.
 *
 * Source: SLUS_005.10  FUN_8001d9c0  and the tiny wrapper FUN_8001db24.
 *
 * Called once per frame after the camera matrix is built. Copies the
 * 5-u32 packed rotation + 3-i32 translation into TWO sets of globals:
 *   DAT_8006f6e0..fc -- "current frame" camera basis (used by physics
 *                       culling -- see src/physics/culling.c).
 *   DAT_8006f740..5c -- "previous frame" camera basis (consumed by
 *                       interpolation / motion-blur, renderer-side).
 *
 * Then it caches the inverse rigid-body matrix at DAT_8006f680 (the
 * "world->camera" transform) via Matrix_InverseRigid, and stores the
 * passed near-clip Z at uRam000006d4.
 *
 * The companion FUN_8001db24 composes the parent chain (Matrix_ComposeParentChain)
 * and immediately calls Camera_ApplyFrame -- used at vehicle camera attach.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern void Matrix_InverseRigid(const MATRIX *src, MATRIX *dst);   /* FUN_80016dfc */
extern MATRIX *Matrix_ComposeParentChain(int obj);                  /* FUN_8001d624 */

extern uint32_t DAT_8006f6e0, DAT_8006f6e4, DAT_8006f6e8, DAT_8006f6ec, DAT_8006f6f0;
extern int32_t  DAT_8006f6f4, DAT_8006f6f8, DAT_8006f6fc;
extern uint32_t DAT_8006f740, DAT_8006f744, DAT_8006f748, DAT_8006f74c, DAT_8006f750;
extern int32_t  DAT_8006f754, DAT_8006f758, DAT_8006f75c;
extern MATRIX   DAT_8006f680;
extern int32_t  uRam000006d4;

/* HIGH: write the camera matrix into the renderer / culling globals. */
void Camera_ApplyFrame(MATRIX *m, int32_t nearZ)
{
    DAT_8006f6e0 = *(uint32_t *)m->m[0];
    DAT_8006f6e4 = *(uint32_t *)(m->m[0] + 2);
    DAT_8006f6e8 = *(uint32_t *)(m->m[1] + 1);
    DAT_8006f6ec = *(uint32_t *)m->m[2];
    DAT_8006f6f0 = *(uint32_t *)(m->m[2] + 2);
    DAT_8006f6f4 = m->t[0];
    DAT_8006f6f8 = m->t[1];
    DAT_8006f6fc = m->t[2];

    DAT_8006f740 = DAT_8006f6e0;
    DAT_8006f744 = DAT_8006f6e4;
    DAT_8006f748 = DAT_8006f6e8;
    DAT_8006f74c = DAT_8006f6ec;
    DAT_8006f750 = DAT_8006f6f0;
    DAT_8006f754 = DAT_8006f6f4;
    DAT_8006f758 = DAT_8006f6f8;
    DAT_8006f75c = DAT_8006f6fc;

    Matrix_InverseRigid(m, &DAT_8006f680);
    uRam000006d4 = nearZ;
}

/* HIGH: convenience for "use this object's effective world matrix as
 * the camera and apply with the given near-Z." */
void Camera_AttachToObject(int obj, int32_t nearZ)
{
    MATRIX *m = Matrix_ComposeParentChain(obj);
    Camera_ApplyFrame(m, nearZ);
}
