/* misc.c -- Hoover Dam tiny collision/destruction handlers.
 *
 * Source: HOOVRDAM.DLL
 *   FUN_8010036c -- HD_TransformerCleanup(obj, mode)
 *                   On mode 7, stash the obj pointer into a global pool
 *                   slot (+0x58 of _DAT_800659fc) and queue a 0x80-byte
 *                   FX prim via FUN_8001d470. Always Heap_Free's the
 *                   passed object pointer.
 *   FUN_801013e0 -- HD_TransformerDestroy(obj, mode, impact)
 *                   On collision (mode 3 or 8) checks terrain, then
 *                   broadcasts the destruction by signalling the
 *                   paired object via FUN_8001fd9c with offset 1000.
 *   FUN_8010135c -- HD_PipeOutDestroy(obj, mode, impact)
 *                   Same shape; on hit invokes Damage_Apply on the
 *                   paired object.
 *   FUN_80101118 -- HD_ArchTransfImpact(obj, mode, impact)
 *                   On heavy collision spawns an audio cue via
 *                   FUN_8004483c (Audio_PlaySfxAtPos: param=6).
 *
 * HIGH-MED confidence across all four.
 */
#include <stdint.h>

extern void  Heap_Free(void *p);
extern void *Object_Pool_Alloc(uint32_t size);   /* FUN_8001d470 */
extern void  Damage_Apply(void *obj);
extern int   Collision_AgainstTerrain(int obj, int impact);    /* func_0x8002239c */
extern int   Collision_Circular     (int obj, int impact);    /* func_0x80022320 */
extern int   Object_FindByIdPlusOffset(int spawnId);            /* func_0x8001fd9c */
extern int   Object_FindByIdPlusOffset2(int spawnId);           /* func_0x80021888 */
extern void  Object_SetState(int obj, int state);               /* func_0x80024718 */
extern uint32_t SfxChannel_Acquire(void);                       /* FUN_8004410c */
extern void  Audio_PlaySfxAtPos(uint32_t ch, uint32_t bank, int sfxId, int posXyzAddr);  /* FUN_8004483c */

extern int8_t *_DAT_800659fc;

uint32_t HD_TransformerCleanup(void *obj, int mode)
{
    if (mode != 1) {
        if (mode != 7) return 0;
        *(void **)(_DAT_800659fc + 0x58) = obj;
        Object_Pool_Alloc(0x80);
        /* Ghidra noted obj is reloaded from $v1; we preserve the alias. */
    }
    Heap_Free(obj);
    return 0xffffffffu;
}

uint32_t HD_TransformerDestroy(int obj, uint32_t mode, void *impactCtx)
{
    if (mode == 3 || mode == 8) {
        Collision_AgainstTerrain(obj, (int)(intptr_t)impactCtx);
    }
    int hit = Collision_Circular(obj, (int)(intptr_t)impactCtx);
    if (hit == 0) return 0;
    int paired = Object_FindByIdPlusOffset(*(int16_t *)(obj + 6) + 1000);
    if (paired != 0) Object_SetState(paired + 0xc, 0);
    return 0xffffffffu;
}

uint32_t HD_PipeOutDestroy(int obj, uint32_t mode, void *impactCtx)
{
    if (mode == 3 || mode == 8) {
        Collision_AgainstTerrain(obj, (int)(intptr_t)impactCtx);
    }
    int hit = Collision_Circular(obj, (int)(intptr_t)impactCtx);
    if (hit == 0) return 0;
    int paired = Object_FindByIdPlusOffset2(*(int16_t *)(obj + 6) + 1000);
    if (paired == 0) return 0;
    Damage_Apply((void *)(intptr_t)paired);
    return 0;
}

uint32_t HD_ArchTransfImpact(int obj, uint32_t mode, int *impulse)
{
    if (mode == 3 || mode == 8) {
        if (*(int8_t *)(*impulse + 4) != 7) return 0;
        impulse = (int *)(uintptr_t)*(uint16_t *)(*impulse + 0xc);
    }
    int hit = Collision_Circular(obj, (int)(intptr_t)impulse);
    if (hit != 0) {
        uint32_t ch = SfxChannel_Acquire();
        Audio_PlaySfxAtPos(ch,
                           *(uint32_t *)(*(int *)(obj + 0x58) + 8),
                           6,
                           obj + 0x24);
    }
    return 0;
}
