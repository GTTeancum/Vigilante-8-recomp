/* pipe_end.c -- Oil Fields pipe_end / OilPump child spawner.
 *
 * Source: OILFIELD.DLL  FUN_80100870.
 *
 * Allocates a 0x9c-byte projectile, copies the parent's rotation
 * matrix (parent[4..7]) and pos (parent[8..0xb]) into the child,
 * then snaps the child's posY to the terrain height under it.
 * Configures: tick=FUN_80100668, LOD radius 0x40000, type 7,
 * sub-state 10, mesh=DAT_801000a8, texture=DAT_80101148, lifetime
 * counter 4.
 *
 * Used by `pipe_end_1` / `OilPump_1` for the secondary oil
 * spray effect after primary destruction.
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_Alloc(uint32_t size);
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern void *FUN_80100668;
extern uint8_t DAT_801000a8[];
extern uint8_t DAT_80101148[];

void OF_PipeEndSpawn(uint32_t *parent)
{
    uint32_t *c = Object_Pool_Alloc(0x9c);
    c[4] = parent[4];
    c[5] = parent[5];
    c[6] = parent[6];
    c[7] = parent[7];
    c[8]  = parent[8];
    c[9]  = parent[9];
    c[10] = parent[10];
    c[11] = parent[11];
    /* Snap to terrain. */
    c[10] = (uint32_t)Terrain_HeightAt(c[9], c[11]);

    c[0x19] = (uintptr_t)&FUN_80100668;
    c[0x15] = 0x40000;
    *((int8_t *)c + 4) = 7;
    *((uint16_t *)c + 6) = 10;
    c[0x17] = (uintptr_t)DAT_801000a8;
    c[0x26] = (uintptr_t)DAT_80101148;
    *((uint16_t *)c + 0x41) = 4;     /* +0x82 i16 */
}
