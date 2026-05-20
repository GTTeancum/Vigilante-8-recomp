/* manhole_spawn.c -- Casino City manhole_1 / Burger_sign_1 spawn helper.
 *
 * Source: CASNOCTY.DLL  FUN_801004e8.
 *
 * On mode 2 (re-trigger from a state machine), spawns a child object
 * (size 0x9c) copying the parent's rotation matrix and pos, snapping
 * to the terrain. The child:
 *   - LOD radius 0x40000 (large draw distance)
 *   - state byte 7
 *   - mesh from DAT_80100094 (manhole cover texture)
 *   - lifetime counter 4 (animation frames)
 *   - sub-state 0x200
 *   - status flags 0x184 (active + collidable + animated)
 *   - vx = -0x800, vz = 0 (sliding sideways)
 *   - tick callback = FUN_801002bc (manhole physics in this same DLL)
 *
 * Then Object_RegisterInScene + sub-state set to 0x3c + parent
 * "cleanup spawn" (FUN_8001ac08).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_Alloc(uint32_t size);
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern void Object_RegisterInScene(uint32_t *obj);
extern void Object_SetSubState(int obj, int sub);
extern void Object_CleanupSpawn(uint32_t *parent);   /* FUN_8001ac08 */
extern uint32_t FUN_801002bc;
extern uint8_t  DAT_80100094[];

uint32_t CC_ManholeSpawn(uint32_t *parent, uint32_t mode, int *impulse)
{
    if (mode != 2) return 0;

    uint32_t *p = Object_Pool_Alloc(0x9c);
    p[4] = parent[4];
    p[5] = parent[5];
    p[6] = parent[6];
    p[7] = parent[7];
    p[8]  = parent[8];
    p[9]  = parent[9];
    p[10] = parent[10];
    p[11] = parent[11];
    p[10] = (uint32_t)Terrain_HeightAt(p[9], p[11]);

    p[0x19] = (uintptr_t)&FUN_801002bc;
    p[0x15] = 0x40000;
    *((int8_t *)p + 4) = 7;
    p[0x17] = (uintptr_t)DAT_80100094;
    *((uint16_t *)p + 0x41) = 4;
    p[0x21] = 0x200;
    *(int16_t *)(p + 3) = (int16_t)mode;
    p[0] |= 0x184u;
    p[0x22] = 0xfffff800u;   /* vx = -0x800 */
    p[0x23] = 0;

    Object_RegisterInScene(p);
    Object_SetSubState((int)(uintptr_t)p, 0x3c);
    Object_CleanupSpawn(parent);
    (void)impulse;
    return 0;
}
