/* palm_spawn.c -- Casino City pelican / palm-tree projectile spawner.
 *
 * Source: CASNOCTY.DLL  FUN_80100bd0.
 *
 * Allocates a 0x9c-byte projectile, configures it for the Casino City
 * level's pelicana_1 / palm1_1 throwables:
 *   - state byte +0x96 = 0x12 (kind 18, palm coconut?)
 *   - lifetime +0x82 = 4 ticks
 *   - sub-state +0xc = 2
 *   - lifespan counter +0x21 = 0x200 (frames)
 *   - draw bank @+0x26 = vehicle template bank
 *   - initial velocity vx = -0x600, vz = 0 (lateral throw)
 *   - state flags |= 0xa4 (active + animated + collidable)
 * Attaches as a bone child of the spawner via FUN_8001b038 + 8001b2fc,
 * then registers in the post-update queue via FUN_80020744.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_Alloc(uint32_t size);
extern int  Bone_AllocSlot(uint32_t parent, uint16_t slotKey);
extern void Bone_AttachChild(uint32_t parent, int slot, uint32_t *child);
extern void Object_RegisterPostUpdate(uint32_t *obj);   /* FUN_80020744 */
extern void *FUN_80100a9c;                              /* per-tick callback */
extern uint32_t _DAT_800737d8;

void CC_PalmSpawn(uint32_t parent, uint16_t slotKey)
{
    uint32_t *p = Object_Pool_Alloc(0x9c);
    p[0x19] = (uintptr_t)&FUN_80100a9c;
    p[0]  |= 0xa4u;
    *(uint16_t *)((uint8_t *)p + 0x96) = 0x12;
    *(uint16_t *)((uint8_t *)p + 0x82) = 4;
    *(uint16_t *)(p + 3) = 2;
    p[0x21] = 0x200;
    p[0x26] = _DAT_800737d8;
    p[0x22] = 0xfffffa00u;   /* vx = -0x600 (lateral throw) */
    p[0x23] = 0;             /* vz = 0 */

    int slot = Bone_AllocSlot(parent, slotKey);
    Bone_AttachChild(parent, slot, p);
    Object_RegisterPostUpdate(p);
}
