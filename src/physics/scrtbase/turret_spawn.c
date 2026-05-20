/* turret_spawn.c -- SCRTBASE turret child-projectile spawner.
 *
 * Source: SCRTBASE.DLL  FUN_8010183c.
 *
 * Spawns a 0x98-byte projectile of kind 0x1b7 attached to a bone slot
 * on the parent. Sets initial pos = bone pos, state = 0x84, sub-state
 * = 7, tick callback = FUN_801010f4 (level-local projectile physics).
 *
 * The target slot at puVar2[0x21] is set to the attacker's currentTarget
 * (+0xe4) if non-NULL, else the attacker itself -- i.e., the missile
 * tracks whomever the turret was already aiming at, falling back to
 * the attacker that hit us.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern int Bone_AllocSlot(int parent, uint16_t slotKey);
extern void Object_CopyMatrixFromBone(void *outMatrix, int parent, int bonePtr);
extern void Object_PostUpdate2(uint32_t obj);
extern void *FUN_801010f4;

uint32_t *SB_TurretSpawn(int self, int attacker)
{
    uint32_t *p = Object_Pool_AllocFromBank(
        (void *)(uintptr_t)*(uint32_t *)(self + 0x58), 0x1b7, 0x98, 8);
    int slot = Bone_AllocSlot(self, 0x8000);
    Object_CopyMatrixFromBone(p + 4, self, slot);

    p[0x12] = p[9];
    p[0x13] = p[10];
    p[0x14] = p[11];
    p[0]    = 0x84;
    *((int8_t *)p + 4) = 7;
    *((uint16_t *)p + 6) = *(uint16_t *)(self + 6);
    *(uint16_t *)(p + 3) = 0;
    p[0x19] = (uintptr_t)&FUN_801010f4;

    int target = *(int *)(attacker + 0xe4);
    if (target == 0) target = attacker;
    p[0x21] = (uint32_t)target;

    Object_PostUpdate2((uint32_t)(uintptr_t)p);
    return p;
}
