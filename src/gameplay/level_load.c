/* level_load.c -- load level via Shell\Load.dll trampoline.
 *
 * Source: SLUS_005.10  FUN_80022ba8.
 *
 * Three-arg variant: pass through to Load.dll's entry function, then
 * release the DLL image, then tick the world-update callback once
 * with mode=1 (init). If the world-list head has the 0x80 bit set,
 * call ObjList_FastClear on the back-buffer pending list.
 *
 * The Load.dll's entry function (image+4 after relocation) decides
 * the level via the args. After it returns, the DLL is no longer
 * needed in memory -- its only job was to bootstrap.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void *Overlay_LoadAndRelocate_Named(const char *path);   /* FUN_80011adc */
extern void  Heap_Free(void *p);
extern void  ObjList_FastClear(void *listSentinel);             /* FUN_8001fe50 */
extern void (*pcRam00000730)(void *obj, int mode, int unused);   /* world update callback */
extern uint32_t *puRam000006f8;                                  /* world object root */
extern uint8_t   DAT_80065a60[];

void Level_LoadViaShellLoadDll(uint32_t a0, uint32_t a1, uint32_t a2)
{
    void *image = Overlay_LoadAndRelocate_Named("Shell\\Load.dll");
    typedef void (*EntryFn)(uint32_t, uint32_t, uint32_t);
    EntryFn entry = *(EntryFn *)((uint8_t *)image + 4);
    entry(a0, a1, a2);
    Heap_Free(image);

    pcRam00000730(puRam000006f8, 1, 0);
    if ((*puRam000006f8 & 0x80u) != 0) {
        ObjList_FastClear(DAT_80065a60);
    }
}
