/* misc.c -- Casino City small object handlers.
 *
 * Source: CASNOCTY.DLL
 *   FUN_80101ca8 -- CC_GenericCleanup: same shape as HD_TransformerCleanup
 *                   (mode 7 stashes to FX pool, then Heap_Free).
 *   FUN_80100168 -- CC_BurgerSignReset: mode 5 plays an audio cue via
 *                   FUN_80044ac8; reuses obj's draw-prim slot via
 *                   FUN_8001add0 with size 0x24, sets state flag 0x34,
 *                   installs FUN_800223dc as the new per-tick callback.
 *
 * MED confidence.
 */
#include <stdint.h>

extern void *Object_Pool_Alloc(uint32_t size);
extern void  Heap_Free(void *p);
extern void  Audio_PlaySfxRelative(uint32_t bank, int sfxId, void *posXyz);  /* func_0x80044ac8 */
extern void  Object_FreeAndUnhook(void *p);
extern void  Object_RebindPrim(uint32_t *obj, uint32_t bank, int size, int flags);  /* FUN_8001add0 */

extern int8_t *_DAT_800659fc;
extern uint32_t _DAT_800658fc;
extern uint32_t _DAT_800737d8;
extern uint32_t FUN_800223dc;

uint32_t CC_GenericCleanup(void *obj, int mode)
{
    if (mode != 1) {
        if (mode != 7) return 0;
        *(void **)(_DAT_800659fc + 0x58) = obj;
        Object_Pool_Alloc(0x80);
    }
    Heap_Free(obj);
    return 0xffffffffu;
}

uint32_t CC_BurgerSignReset(uint32_t *obj, int mode)
{
    if (mode != 1) {
        if (mode != 5) return 0;
        Audio_PlaySfxRelative(_DAT_800658fc, 0x3b, obj + 0x12);
    }
    uint32_t status = 0x80020000;
    if (((int8_t *)obj)[8] != 0) {
        Object_FreeAndUnhook((void *)(uintptr_t)obj[0xc]);
        obj[0xc] = 0;
        Object_RebindPrim(obj, _DAT_800737d8, 0x24, 8);
        status = obj[0] | 0x34;
        obj[0] = status;
    }
    obj[0x19] = status + 0x23dc;
    return 0;
}
