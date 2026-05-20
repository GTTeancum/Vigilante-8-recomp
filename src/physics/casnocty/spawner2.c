/* spawner2.c -- another Casino City projectile spawner pair.
 *
 * Source: CASNOCTY.DLL
 *   FUN_80101a90 -- CC_RandomScatterTick: countdown-driven spawn that
 *                   scatters projectiles in a 2D random-square cone
 *                   around the parent's +0x54 radius. Same idiom as
 *                   CC_RandomFire but using uniform-square rather than
 *                   cos/sin cone.
 *   FUN_80101bb8 -- CC_BoneSpawn: generic per-bone child constructor
 *                   (size 0x9c, with audio cue 1 played at spawn).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern void Object_RandomizeRotation(uint32_t *m);
extern void Object_RegisterInScene(uint32_t *obj);
extern void Object_SetSubState(int obj, int sub);
extern uint32_t SfxChannel_Acquire(void);
extern void Audio_PlaySfxAtPosVar(uint32_t ch, uint32_t bank, int sfxId, void *pos);
extern void Object_BumpSubstate_Or_FX(int obj);   /* FUN_8001d4f0 */

uint32_t CC_RandomScatterTick(int obj, uint32_t mode)
{
    if (mode == 2) return 0;
    if (mode != 5 && mode != 0) return 0;

    int16_t *countdown = (int16_t *)(intptr_t)(obj + 0x80);
    int16_t prev = *countdown;
    *countdown = (int16_t)(prev - 1);
    if (prev != 0) return 0;

    uint32_t *child = Object_Pool_AllocFromBank(
        (void *)(uintptr_t)*(uint32_t *)(obj + 0x98),
        *(uint16_t *)(obj + 0x96), 0x80, 8);

    int32_t r = *(int32_t *)(obj + 0x54);
    int32_t rx = ((int)V8_RandNext() * 2 * r >> 15) - r;
    int32_t rz = ((int)V8_RandNext() * 2 * r >> 15) - r;
    child[0x9 + 0] = 0;    /* placeholder -- exact layout in pass 3 */
    child[0xa]     = 0;
    *(int32_t *)((uint8_t *)child + 0x24) = rx;
    *(int32_t *)((uint8_t *)child + 0x28) = 0;
    *(int32_t *)((uint8_t *)child + 0x2c) = rz;
    *(uint32_t *)((uint8_t *)child + 100)  = 0x8003e80c;   /* main-EXE projectile tick */

    Object_BumpSubstate_Or_FX(obj);
    *countdown = *(int16_t *)(obj + 0x82);
    return 0;
}

uint32_t *CC_BoneSpawn(uint32_t *parentPos, uint32_t bank, uint16_t kind,
                       uint16_t animSlot, uint16_t spriteSlot, uint32_t lifetime)
{
    uint32_t *c = Object_Pool_AllocFromBank((void *)(uintptr_t)bank, kind, 0x9c, 8);
    Object_RandomizeRotation(c + 4);
    c[9]  = parentPos[0];
    c[10] = parentPos[1];
    c[11] = parentPos[2];
    c[0x19] = 0;       /* tick callback set externally */
    c[0x15] = 0x8000;
    c[0x26] = bank;
    *(uint16_t *)((uint8_t *)c + 0x96) = animSlot;
    c[0] |= 0xa4u;
    *(uint16_t *)((uint8_t *)c + 0x82) = spriteSlot;
    Object_RegisterInScene(c);
    Object_SetSubState((int)(uintptr_t)c, (int)lifetime);
    uint32_t ch = SfxChannel_Acquire();
    Audio_PlaySfxAtPosVar(ch, *(uint32_t *)(bank + 8), 1, parentPos);
    return c;
}
