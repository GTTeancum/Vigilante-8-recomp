/* misc.c -- Valley Farms small handlers.
 *
 * Source: VALLYFRM.DLL
 *   FUN_80100e70 -- VF_DetachAndStopSfx: unlink from parent and stop
 *                   the per-object SFX. Returns -1 (destroyed). Same
 *                   3-line teardown idiom as elsewhere.
 *
 * HIGH.
 */
#include <stdint.h>

extern void Object_DetachFromParent(int self);   /* FUN_8001d564 */
extern void Audio_StopAllSfx(void *bank);        /* FUN_8001af48 */

uint32_t VF_DetachAndStopSfx(int self)
{
    Object_DetachFromParent(self);
    Audio_StopAllSfx((void *)(intptr_t)self);
    return 0xffffffffu;
}
