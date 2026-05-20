/* blimp.c -- Casino City blimp_1 slow-spin + RNG tilt.
 *
 * Source: CASNOCTY.DLL  FUN_8010020c.
 *
 * The casino's hovering blimp child object (obj[0xe]) rotates each
 * tick by 0x11 (same as Secret Base's radar dish) AND randomises its
 * roll axis using V8_RandNext (truncated to i16). The 0x80 bit of the
 * top-level state word is set on every tick to keep the blimp in the
 * "always visible" render list.
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern void  ImpactSparks_Spawn(void *obj, int mode);     /* FUN_8001d708 */
extern void  Object_DispatchToParent(void *obj, int mode, int *impulse); /* func_0x800223dc */

void CC_BlimpTick(uint32_t *obj, int mode, int *impulse)
{
    uint32_t *target = obj;
    if (mode == 0 || mode != 1) {
        uint8_t *child = (uint8_t *)(uintptr_t)obj[0xe];
        *(int16_t *)(child + 0x42) = (int16_t)(*(int16_t *)(child + 0x42) + 0x11);
        if (impulse != NULL) ImpactSparks_Spawn(child, mode);
        target = (uint32_t *)1;
    }
    *obj |= 0x80;                                  /* keep visible */
    uint8_t *child = (uint8_t *)(uintptr_t)obj[0xe];
    *(int16_t *)(child + 0x42) = (int16_t)V8_RandNext();
    Object_DispatchToParent(obj, mode, impulse);
    (void)target;
}
