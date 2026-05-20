/* lever.c -- Hoover Dam Dam_Lever bidirectional swing + spawn child.
 *
 * Source: HOOVRDAM.DLL  FUN_80101580.
 *
 * Each tick:
 *   - If the "swing direction" flag at obj[2] is non-zero, add +0x10
 *     to angle obj[0x11]; once it exceeds 0x200, spawn a child;
 *     otherwise add -0x10 each tick and spawn when angle drops below
 *     -0x200. Either way, on spawn:
 *       - alloc 0x80-byte child of kind 0x25f (water spurt)
 *       - state byte 7, sub-state 0x19, status 0x184
 *       - pos = lever's pos with vy += 0x15000 (up offset)
 *       - tick callback = FUN_80101464 (HD_PowerlineTick variant)
 *       - clear back-buffer flag (FUN_80020778)
 *       - Object_PostUpdate2 + sub-state 0x200
 *       - sfx 0x41 at child pos
 *     - call ImpactSparks_Spawn(parent) for the visual cue
 *   - Toggle the direction bit (obj[2] ^= 1).
 *   - sub-state := 0xf0, register parent for post-update.
 *   - Final: state byte 3, clear flag bit 0x20.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern uint32_t Object_ClearBackBufferFlag(uint32_t *obj);  /* FUN_80020778 */
extern void Object_PostUpdate2(uint32_t obj);
extern void Object_SetSubState(int obj, int sub);
extern uint32_t SfxChannel_Acquire(void);
extern void Audio_PlaySfxAtPos(uint32_t ch, uint32_t bank, int sfxId, int posXyzAddr);
extern void ImpactSparks_Spawn(void *obj);
extern void Object_RegisterPostUpdate(uint32_t *obj);
extern void *FUN_80101464;     /* HD_PowerlineTick */
extern uint32_t _DAT_800658fc;

uint32_t HD_LeverTick(uint32_t *self, int mode)
{
    if (mode == 1) goto epilogue;
    if (mode == 2) goto clearFlag20;
    if (mode != 0 && mode != 3) goto epilogue;

    /* Swing angle. */
    int16_t *ang = (int16_t *)((uint8_t *)self + 0x44);
    int8_t   dir = *(int8_t *)(self + 2);
    int16_t  newAng;
    int      spawn = 0;
    if (dir != 0) {
        newAng = (int16_t)(*ang + 0x10);
        if (newAng > 0x200) spawn = 1;
    } else {
        newAng = (int16_t)(*ang - 0x10);
        if (newAng < -0x200) spawn = 1;
    }
    *ang = newAng;

    if (spawn) {
        uint32_t *p = Object_Pool_AllocFromBank((void *)(uintptr_t)self[0x16], 0x25f, 0x80, 8);
        *((int8_t *)p + 4) = 7;
        *((uint16_t *)p + 6) = 0x19;
        p[0] |= 0x184u;
        p[0x12] = self[0x12];
        p[0x13] = self[0x13] + 0x15000;
        p[0x14] = self[0x14];
        p[0x19] = (uintptr_t)&FUN_80101464;
        Object_ClearBackBufferFlag(self);
        Object_PostUpdate2((uint32_t)(uintptr_t)p);
        Object_SetSubState((int)(uintptr_t)p, 0x200);
        uint32_t ch = SfxChannel_Acquire();
        Audio_PlaySfxAtPos(ch, _DAT_800658fc, 0x41, (int)(intptr_t)&p[0x12]);
    }
    ImpactSparks_Spawn(self);

    *(uint8_t *)(self + 2) = (uint8_t)(*(uint8_t *)(self + 2) ^ 1u);
    self[0] |= 0x20u;
    Object_SetSubState((int)(uintptr_t)self, 0xf0);
    Object_RegisterPostUpdate(self);

epilogue:
    *((int8_t *)self + 4) = 3;
clearFlag20:
    self[0] &= ~0x20u;
    return 0;
}
