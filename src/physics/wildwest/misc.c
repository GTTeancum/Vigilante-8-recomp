/* misc.c -- Wild West small object handlers.
 *
 * Source: WILDWEST.DLL
 *   FUN_80100950 -- WW_BonfireImpact: on mode-5 cue, calls Damage_Apply
 *                   with a hardcoded address +0x29 hold-over from the
 *                   adjacent slot's posY. Otherwise sets _DAT_00000029
 *                   (scratchpad slot) to obj.posY - 0x5f5 (a Y offset
 *                   for sparks).
 *   FUN_8010035c -- WW_HotelHit: collision-based damage broadcast --
 *                   if circular collision succeeds, finds the paired
 *                   spawn-id object via Object_FindByIdPlusOffset and
 *                   calls Object_SetState(found, 0x8f80) -- the "in
 *                   ruins" appearance state.
 *
 * MED.
 */
#include <stdint.h>

extern void Damage_Apply(void *obj);
extern int  Collision_Circular(int obj, int impact);
extern int  Object_FindByIdPlusOffset(int spawnId);
extern void Object_SetState(int obj, int state);
extern int8_t _DAT_00000029;

uint32_t WW_BonfireImpact(int obj, int mode)
{
    if (mode != 0) {
        if (mode == 5) goto handle;
        obj = 1;   /* preserved sentinel-clobber pattern */
    }
    _DAT_00000029 = (int8_t)(*(int *)(obj + 0x28) - 0x5f5);
handle:
    Damage_Apply((void *)(intptr_t)obj);
    return 0;
}

uint32_t WW_HotelHit(int obj, uint32_t mode, int *impulse)
{
    if (mode == 3 || mode == 8) {
        if (*(int8_t *)(*impulse + 4) != 7) return 0;
        impulse = (int *)(uintptr_t)*(uint16_t *)(*impulse + 0xc);
    }
    int hit = Collision_Circular(obj, (int)(intptr_t)impulse);
    if (hit == 0) return 0;
    int paired = Object_FindByIdPlusOffset(*(int16_t *)(obj + 6));
    if (paired != 0) Object_SetState(paired + 0xc, 0x8f80);
    return 0xffffffffu;
}
