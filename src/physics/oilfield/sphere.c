/* sphere.c -- Oil Fields gas-sphere collision handler.
 *
 * Source: OILFIELD.DLL  FUN_80100540.
 *
 * Handles the destructible gas-storage sphere (sphere_1). State
 * transitions:
 *
 *   mode 2 (init):  set sub-state 0x3c on a placeholder, then 0xf0
 *                   on the parent, queue an explosion sound (sfx 0x3b
 *                   via Audio_PlaySfxAtPos), and allocate the bone
 *                   slot pair for the child gas-cloud effect.
 *   mode 1:         allocate the bone slot (without the sound).
 *   mode 3 / 8:     collision tests.
 *   default:        same as mode 2 (init / re-init).
 *
 * On any successful circular collision (after either path), if the
 * gas-cloud child still exists at obj+0x74, set its sub-state to 0
 * (active explosion frame), promote the parent to sub-state 0xf0,
 * and clear obj+0x74 so the gas-cloud isn't re-triggered.
 *
 * MED confidence.
 */
#include <stdint.h>

extern void Object_SetSubState(int obj, int sub);    /* FUN_80020890 */
extern uint32_t SfxChannel_Acquire(void);             /* FUN_8004410c */
extern void Audio_PlaySfxAtPos(uint32_t ch, uint32_t bank, int sfxId, int posXyzAddr);  /* FUN_8004483c */
extern int  Bone_AllocSlot(int parent, uint16_t slotKey);    /* FUN_8001b038 */
extern int  Collision_AgainstTerrain(int obj, int impact);
extern int  Collision_Circular(int obj, int impact);
extern uint32_t _DAT_800658fc;

uint32_t OF_SphereTick(int obj, uint32_t mode, void *impact)
{
    int gasCloudObj = 2;

    if (mode == 2 || mode == 1 || mode == 3 || mode == 8) {
        if (mode == 2) goto initOrSpawn;
        if (mode == 1) goto bindSlot;
        if (mode == 8) goto checkCircular;
        Collision_AgainstTerrain(obj, (int)(intptr_t)impact);
        goto checkCircular;
    }

initOrSpawn:
    Object_SetSubState(gasCloudObj, 0x3c);
    Object_SetSubState(obj, 0xf0);
    {
        uint32_t ch = SfxChannel_Acquire();
        Audio_PlaySfxAtPos(ch, _DAT_800658fc, 0x3b, obj + 0x24);
    }
bindSlot:
    {
        int slot = Bone_AllocSlot(obj, 0x8000);
        *(int *)(obj + 0x70) = slot;
        *(int *)(obj + 0x74) = slot;
    }
checkCircular:
    {
        int hit = Collision_Circular(obj, (int)(intptr_t)impact);
        if (hit != 0 && *(int *)(obj + 0x74) != 0) {
            Object_SetSubState(*(int *)(obj + 0x74), 0);
            Object_SetSubState(obj, 0xf0);
            *(int *)(obj + 0x74) = 0;
        }
    }
    return 0;
}
