/* rig_destroy.c -- Oil Field rig_1 destruction broadcast.
 *
 * Source: OILFIELD.DLL  FUN_80101028.
 *
 * On damage collision: takes hit, computes 1x1 cell at obj's world
 * (x, z), broadcasts state 0x8f80 (= "ruins") via Object_SetState,
 * then walks the global obstacle chain (DAT_80065a50) and clears
 * flag bit 2 on every entry whose spawnId matches our spawnId --
 * essentially "find all my siblings and mark them as no-longer-active".
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern void Object_SetState(int obj, int state);
extern int32_t **_DAT_80065a50;

uint32_t OF_RigDestroy(int obj, uint32_t mode, void *impact)
{
    int hit = 0;
    if ((mode == 3 || mode != 8) && (hit = Damage_FromImpulse((uint32_t *)(intptr_t)obj, (int *)impact)) != 0)
        goto fire;
    if ((hit = Damage_AccumulateOrFire((uint32_t *)(intptr_t)obj, (uint16_t)(uintptr_t)impact)) == 0)
        return 0;

fire: {
    int32_t px = *(int32_t *)(obj + 0x48);
    int32_t pz = *(int32_t *)(obj + 0x50);
    if (px < 0) px += 0xffff;
    if (pz < 0) pz += 0xffff;
    int16_t rect[4] = {
        (int16_t)((uint32_t)px >> 16),
        (int16_t)((uint32_t)pz >> 16),
        1, 1,
    };
    Object_SetState((int)(uintptr_t)rect, 0x8f80);
}

    /* Walk obstacle chain, clear "active" bit on all matching spawnIds. */
    int32_t *node = (int32_t *)_DAT_80065a50[0];
    int32_t **prev = _DAT_80065a50;
    while (node != NULL) {
        uint32_t *payload = (uint32_t *)(uintptr_t)prev[2];
        if (*(int16_t *)((uintptr_t)payload + 6) == *(int16_t *)(obj + 6)) {
            payload[0] &= ~2u;
        }
        prev = (int32_t **)node;
        node = (int32_t *)*node;
    }
    return 0;
}
