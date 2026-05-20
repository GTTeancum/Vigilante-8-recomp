/* debris_throw.c -- OilField destructible debris-throw handler.
 *
 * Source: OILFIELD.DLL  FUN_80100e78.
 *
 * On collision (mode 3, 8) tests via Damage_FromImpulse / circular.
 * Mode 1/6/non-matched falls through to spawning debris from a
 * pre-recorded list.
 *
 * Spawn loop:
 *   1. Pick kind from obj+0x80 (which holds the next debris-kind index).
 *   2. Allocate 0x98-byte child from parent's bank.
 *   3. Copy parent matrix + scale by 0xc00 (4.12 fixed = ~0.75x).
 *   4. Randomize rotation; set spawnId 1000; install tick callback
 *      FUN_80100a30 (the per-debris physics).
 *   5. Initial vy = 0xfffff415 (= -0xbeb -- upward toss).
 *   6. Inverse-distance scale at +0x4a from radius via 0x1000000 / r.
 *   7. Object_RegisterDebris (FUN_8003e76c) chains it onto the world.
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern void Object_CopyMatrixFromBone(void *outMatrix, int parent, int bonePtr);   /* FUN_8001d68c */
extern void Object_RandomizeRotation(uint32_t *obj);   /* FUN_8001dc1c */
extern void Object_RegisterDebris(uint32_t *obj);      /* FUN_8003e76c */

static uint32_t scale_4_12(int32_t v, int32_t factor)
{
    int32_t r = v * factor;
    if (r < 0) r += 0xfff;
    return (uint32_t)(r >> 12);
}

void OF_DebrisThrow(int self, uint32_t mode, void *impactCtx)
{
    int size = 0;
    int hit = 0;

    if (mode == 3 || (mode != 8 && (mode > 3 || mode == 1 || mode == 6))) {
        if (mode == 3 || mode == 8) {
            if (mode == 3) Damage_FromImpulse((uint32_t *)(intptr_t)self, (int *)impactCtx);
            hit = Damage_AccumulateOrFire((uint32_t *)(intptr_t)self, (uint16_t)(uintptr_t)impactCtx);
            size = 0x98;
            if (hit) goto spawn;
        }
        *(uint32_t *)(self + 0x80) =
            (uint32_t)*(uint16_t *)(*(int *)(self + 0x38) + 10);   /* first kind */
    }

spawn:
    {
        uint32_t *child = Object_Pool_AllocFromBank(
            (void *)(uintptr_t)*(uint32_t *)(self + 0x58),
            *(uint16_t *)(self + 0x80), size, 8);
        Object_CopyMatrixFromBone(child + 4, self,
            **(int **)(self + 0x58) + *(int *)(self + 0x80) * 0x1c + 0x1c);
        Object_RandomizeRotation(child);
        child[0x15] = scale_4_12((int32_t)child[0x15], 0xc00);
        *(uint16_t *)((uint8_t *)child + 6) = 1000;
        child[0x19] = (uintptr_t)0;     /* tick = FUN_80100a30 (level local) */
        child[0x21] = 0xfffff415u;       /* vy = -0xbeb (upward) */

        int32_t r = (int32_t)child[0x15] * 0x3243;
        if (r < 0) r += 0xfff;
        r >>= 12;
        if (r != 0) {
            *(int16_t *)(child + 0x25) = (int16_t)(0x1000000 / r);
        }

        uint16_t parentAttr = *(uint16_t *)(self + 0xe);
        *(uint16_t *)(child + 3) = parentAttr;
        child[0] |= 0x88u;

        Object_RegisterDebris(child);
    }
}
