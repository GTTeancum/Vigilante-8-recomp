/* silo_cleanup.c -- Valley Farms generic cleanup hook.
 *
 * Source: VALLYFRM.DLL  FUN_801002c4.
 *
 * Same idiom as HD_TransformerCleanup / CC_GenericCleanup -- mode 7
 * stashes the obj into the FX pool slot at _DAT_800659fc+0x58 and
 * allocates a 0x80-byte effect prim. All other modes (except 1)
 * Heap_Free the obj.
 *
 * Confirms this is THE shared "generic cleanup" pattern used by every
 * destructible level object across overlays. Pass 3 should hoist into
 * a shared helper in src/gameplay/object_cleanup.c.
 *
 * HIGH.
 */
#include <stdint.h>

extern void *Object_Pool_Alloc(uint32_t size);
extern void  Heap_Free(void *p);
extern int8_t *_DAT_800659fc;

uint32_t VF_GenericCleanup(void *obj, int mode)
{
    if (mode != 1) {
        if (mode != 7) return 0;
        *(void **)(_DAT_800659fc + 0x58) = obj;
        Object_Pool_Alloc(0x80);
    }
    Heap_Free(obj);
    return 0xffffffffu;
}
