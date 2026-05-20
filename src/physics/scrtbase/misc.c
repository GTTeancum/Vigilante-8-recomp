/* misc.c -- Secret Base small collision handlers.
 *
 * Source: SCRTBASE.DLL
 *   FUN_801006f4 -- SB_FenceImpact: terrain + circular collision tests.
 *                   Always returns -1 (destroyed/handled).
 *   FUN_801025ec -- SB_TurretHit: on collision with mode-7 impulse,
 *                   broadcast event id 9 to the linked spawn id.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

extern int  Collision_AgainstTerrain(int obj, int impact);    /* func_0x8002239c */
extern int  Collision_Circular     (int obj, int impact);    /* func_0x80022320 */
extern void Object_BroadcastEvent  (int eventId, int spawnId); /* func_0x8002185c */

uint32_t SB_FenceImpact(int obj, uint32_t mode, void *impact)
{
    if (mode == 3 || mode == 8) Collision_AgainstTerrain(obj, (int)(intptr_t)impact);
    Collision_Circular(obj, (int)(intptr_t)impact);
    return 0xffffffffu;
}

uint32_t SB_TurretHit(int obj, uint32_t mode, int *impulse)
{
    if (mode == 3 || mode == 8) {
        if (*(int8_t *)(*impulse + 4) != 7) return 0;
        impulse = (int *)(uintptr_t)*(uint16_t *)(*impulse + 0xc);
    }
    int hit = Collision_Circular(obj, (int)(intptr_t)impulse);
    if (hit != 0) Object_BroadcastEvent(9, *(int16_t *)(obj + 6));
    return 0;
}
