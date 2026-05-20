/* callback_dispatch.c -- safe invocation of an object's per-tick callback.
 *
 * Source: SLUS_005.10  FUN_8001e120.
 *
 * Thin wrapper: if `obj->callback @ +0x64` is non-NULL, save the BIOS
 * scratchpad fields used for stack unwinding (DAT_1f8003f8, +4) and
 * invoke the callback. Returns whatever it returns, or 0 if no callback.
 *
 * Used as the canonical "tick this object" call site -- replaces ~20
 * places where the same pattern appears.
 *
 * Bit-exact: the scratchpad save (DAT_1f8003f8) is a PSX BIOS trick to
 * facilitate the optional restart from an exception in the callee.
 * Don't simplify.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern uintptr_t DAT_1f8003f8;
extern uint8_t  *DAT_1f8003fc;

uint32_t Object_TickCallback(int obj)
{
    typedef uint32_t (*TickFn)(void);
    TickFn fn = *(TickFn *)(intptr_t)(obj + 100);
    if (fn == NULL) return 0;
    /* Stack-save aliases for re-entry. */
    DAT_1f8003f8 = (uintptr_t)__builtin_return_address(0);
    DAT_1f8003fc = (uint8_t *)__builtin_frame_address(0);
    return fn();
}
