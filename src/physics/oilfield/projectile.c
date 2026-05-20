/* projectile.c -- Oil Fields rig/pipe projectile spawner.
 *
 * Source: OILFIELD.DLL  FUN_8010047c.
 *
 * Allocates a fresh 0x9c-byte projectile (via FUN_8001d470 -- the
 * generic object pool allocator) and configures it for the Oil Fields
 * level: composes the parent's world matrix into the projectile's
 * draw matrix, sets type 7 / state 5, points at the per-level mesh
 * pool DAT_80100064, hands it the local model atlas DAT_80101148.
 * Initial velocity: (0, -0x200, 0xc00) in 16.16 -- a fast forward
 * shot with a slight upward component (PSX +y = down so -0x200 = up).
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_Alloc(uint32_t size);     /* FUN_8001d470 */
extern uint32_t  Composite_Matrix_From(uint8_t *outTmp, uint32_t parentMtxIdx);  /* FUN_8001b07c */
extern void      CompMatrixLV(void *, void *, void *);  /* PSY-Q */
extern void      Object_RegisterInScene(uint32_t *obj); /* FUN_800202f4 */

extern uint32_t FUN_801002b0(void);              /* Oil Fields per-tick callback */
extern uint8_t  DAT_80100064[];                  /* per-level mesh palette */
extern uint8_t  DAT_80101148[];                  /* per-level texture atlas */

uint32_t *OF_ProjectileSpawn(int parentObj)
{
    uint32_t *p = Object_Pool_Alloc(0x9c);
    uint8_t  tmp[0x20];
    uint32_t mtx = Composite_Matrix_From(tmp, *(uint32_t *)(parentObj + 0x70));
    CompMatrixLV((void *)(intptr_t)(parentObj + 0x10), (void *)(uintptr_t)mtx, p + 4);

    p[0x19] = (uint32_t)(uintptr_t)FUN_801002b0;
    p[0x15] = 0x40000;
    *((uint8_t *)p + 4) = 7;                /* state */
    *((uint16_t *)p + 6) = 5;               /* sub-state */
    p[0x17] = (uint32_t)(uintptr_t)DAT_80100064;
    p[0x26] = (uint32_t)(uintptr_t)DAT_80101148;
    *((uint16_t *)p + 0x41) = 3;            /* obj+0x82 */
    p[0x21] = 0;
    p[0] |= 0x184;                          /* status flags: spawned + active + visible */
    p[0x22] = 0xfffffe00u;                  /* vx = -0x200 (upward) */
    p[0x23] = 0xc00u;                       /* vz = +0xc00 (forward) */
    Object_RegisterInScene(p);
    return p;
}
