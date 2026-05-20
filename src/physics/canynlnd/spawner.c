/* spawner.c -- Canyonlands "boulder/projectile launcher" entry point.
 *
 * Source: CANYNLND.DLL  FUN_80100940.
 *
 * On mode 9 (= "fire-now"), spawns a child object (size 0x98, type
 * given by the parent's animation slot index at +0xa). Configuration:
 *
 *   - LOD/scale: parent[0x15] (max-LOD radius?) is multiplied by 0x93c
 *     and divided by 0x1000 (4.12-fixed shrink) so the child is
 *     ~36% the parent's size.
 *   - spawnId: 1000 (= "level-spawned projectile").
 *   - randomised offsets: per-call random rotation, random pad byte at
 *     +9 (sound variation index).
 *   - status flags: |= 0x488 (active + projectile + collidable).
 *   - position: copied from parent[0x12..0x14] (parent's posXYZ).
 *   - tick callback: FUN_80100244 (the per-frame physics loop for
 *     these projectiles, also in CANYNLND.DLL).
 *   - initial vy = 0xfffff415 (= -0xbeb, upward).
 *   - initial vx, vz: looked up in a 4096-entry sin/cos trig table
 *     at address 0x80060db4 (in the main EXE's read-only data),
 *     keyed by `(parent_heading + ((rand<<8)>>15) - 0x80) & 0xfff`,
 *     each entry being two i16 (sin, cos) scaled by 0xbeb / 0x1000.
 *
 * MED confidence. Pass 3 should:
 *   - identify the DAT_80060db4 trig table as the standard PSY-Q
 *     rsin/rcos lookup
 *   - rename FUN_8001dc1c -> Object_RandomizeRotation
 *   - finalize the cone-aim formula
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);  /* FUN_8001ac44 */
extern void  Object_RandomizeRotation(uint32_t *obj);   /* FUN_8001dc1c */
extern uint32_t FUN_80100244;                            /* per-tick callback */
extern int16_t DAT_80060db4[];                            /* PSY-Q rsin/rcos table */

static uint32_t scale_4_12(int32_t v, int32_t factor)
{
    int32_t r = v * factor;
    if (r < 0) r += 0xfff;
    return (uint32_t)(r >> 12);
}

uint32_t CL_Launcher(uint32_t *parent, uint32_t mode, uint32_t *impulse)
{
    if (mode == 7)  goto sharedTail;
    if (mode == 1)  goto initTail;
    if (mode != 9)  return 0;
    if ((parent[0] & 1u) != 0) return 0;     /* already active -- don't double-fire */

    /* mode 9: fire */
    uint16_t kind = *(uint16_t *)((uint8_t *)parent + 0xa);
    uint32_t *child = Object_Pool_AllocFromBank((void *)(uintptr_t)parent[0x16], kind, 0x98, 0);

    uint32_t rand0 = V8_RandNext();
    int16_t  parentHeading = *(int16_t *)((uint8_t *)parent + 0x42);

    Object_RandomizeRotation(child);
    child[0x15] = scale_4_12((int32_t)child[0x15], 0x93c);
    *(uint16_t *)((uint8_t *)child + 6) = 1000;        /* spawnId */
    ((uint8_t *)child)[9] = (uint8_t)V8_RandNext();    /* sound variant */
    child[0] |= 0x488u;

    uint16_t kindLo = *(uint16_t *)((uint8_t *)parent + 0xe);
    *(uint16_t *)((uint8_t *)child + 0xe) = kindLo;
    *(uint16_t *)(child + 3) = kindLo;

    child[0x12] = parent[0x12];   /* posX */
    child[0x13] = parent[0x13];   /* posY */
    child[0x14] = parent[0x14];   /* posZ */
    child[0x19] = (uintptr_t)&FUN_80100244;
    child[0x21] = 0xfffff415u;    /* vy = -0xbeb (upward) */

    /* Aim cone: angle = parentHeading + random[-128..127], wrap 0..0xfff. */
    int aimIdx = (((int)parentHeading + (((int)rand0 << 8) >> 15)) - 0x80) & 0xfff;
    int16_t sin = DAT_80060db4[aimIdx * 2 + 0];
    int16_t cos = DAT_80060db4[aimIdx * 2 + 1];
    child[0x20] = scale_4_12(sin, 0xbeb);
    child[0x22] = scale_4_12(cos, 0xbeb);

    impulse = child;
sharedTail:
initTail:
    /* mode-7 cleanup + mode-1 init tail registers the new child with
     * the world list via Object_BindLifecycle/Object_BindFinalize
     * (same idiom as wildwest/dynamite_keg.c case 1). */
    (void)impulse;
    return 0;
}
