/* spawner.c -- Casino City random-direction spawner (palm coconut, etc.).
 *
 * Source: CASNOCTY.DLL  FUN_80100a9c.
 *
 * Decrements an i16 spawn-countdown at obj+0x80. When it reaches -1,
 * spawns a 0xa0-byte child (kind from obj+0x96, bank from obj+0x98)
 * with random aim:
 *   - aimIdx = V8_RandNext() & 0xfff   (uniform 0..4095 cone)
 *   - vx = scaled(obj+0x84 base × sin(aimIdx))
 *   - vz = scaled(obj+0x84 base × cos(aimIdx))
 *   - vy = obj+0x88 ± random(15-bit) × obj+0x88 (gentle vertical jitter)
 *   - status flags |= 0x4b4 (projectile + collidable + animated)
 *   - tick callback at constant absolute address 0x800404c4 (an EXE-
 *     side projectile post-tick we'll resolve in pass 3).
 *   - position copied from parent's effective matrix (parent
 *     Matrix_ComposeParentChain).
 * Then the countdown reloads from obj+0x82.
 *
 * Used by pelicana_1, palm1_1, blimp_1 -- all spawn random-aim
 * projectiles at a fixed cadence.
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern int16_t  DAT_80060db4[];                /* PSY-Q rsin/rcos pair table */
extern uint8_t *Matrix_ComposeParentChain(int obj);   /* FUN_8001d624 */
extern void     Object_RegisterInScene(uint32_t *obj);

static uint32_t scale_4_12_signed(int32_t v, int32_t factor)
{
    int32_t r = v * factor;
    if (r < 0) r += 0xfff;
    return (uint32_t)(r >> 12);
}

uint32_t CC_RandomFire(int obj, int mode)
{
    if (mode != 0) return 0;
    int16_t  *countdown = (int16_t *)(intptr_t)(obj + 0x80);
    int16_t   prev = *countdown;
    *countdown = (int16_t)(prev - 1);
    if (prev != 0) return 0;     /* will fire on the next call when prev was -1; matches Ghidra "==-1" via "==0 before decrement" inversion */

    uint32_t *child = Object_Pool_AllocFromBank(
        (void *)(uintptr_t)*(uint32_t *)(obj + 0x98),
        *(uint16_t *)(obj + 0x96), 0xa0, 8);
    uint32_t aimIdx = V8_RandNext() & 0xfff;
    child[0] |= 0x4b4u;

    int16_t sin = DAT_80060db4[aimIdx * 2 + 0];
    int16_t cos = DAT_80060db4[aimIdx * 2 + 1];
    child[0x22] = scale_4_12_signed(*(int32_t *)(obj + 0x84), sin);
    child[0x24] = scale_4_12_signed(*(int32_t *)(obj + 0x84), cos);

    uint32_t rand1 = V8_RandNext();
    child[0x23] = (uint32_t)(*(int32_t *)(obj + 0x88)
                             + ((int32_t)rand1 * *(int32_t *)(obj + 0x88) >> 15));

    uint8_t *m = Matrix_ComposeParentChain(obj);
    child[9]  = *(uint32_t *)(m + 0x14);
    child[10] = *(uint32_t *)(m + 0x18);
    child[11] = *(uint32_t *)(m + 0x1c);
    child[0x19] = 0x800404c4;       /* main-EXE projectile tick */

    Object_RegisterInScene(child);

    /* Reload countdown. */
    *countdown = *(int16_t *)(obj + 0x82);
    return 0;
}
