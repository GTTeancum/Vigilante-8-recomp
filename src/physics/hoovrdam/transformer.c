/* transformer.c -- Hoover Dam transformer aim + collapse.
 *
 * Source: HOOVRDAM.DLL  FUN_801003c4.
 *
 * Aims a transformer at the camera (the player view). Each tick:
 *   1. Compute camera-space delta: (obj.pos - camera_pos) / N
 *      where N is a divisor from param_3, with offsets +0x5f5
 *      and +0x1dcd along x/y to compensate for the dam's tilt.
 *   2. Project that vector through the world->camera matrix at
 *      DAT_8006f680 to get screen-space direction.
 *   3. Build a YXZ rotation matrix pointing at the camera.
 *   4. Walk the obj.children chain (+0xe -> next +0x34) and write
 *      that rotation into each child's +0x10 matrix slot, plus
 *      adjust their +0x2c (Z) to wrap modulo 0x40000 (= +0x20000
 *      half-range).
 *   5. After visiting all children, alloc an effect prim (0x80 bytes)
 *      tied to obj at +0x58, slot at +0xa = param_3 ("event index").
 *   6. If difficulty > 2, Heap_Free obj (= "transformer destroyed at
 *      hard difficulty" sub-clause -- the entire transformer goes
 *      away rather than just toppling).
 *   7. Sets flag 0xa0 on obj.
 *   8. Spawn 0x40 (64) random debris particles, each with random
 *      direction (V8_RandNext, << 17 >> 13 = scale into world units).
 *
 * MED.
 */
#include <stdint.h>

extern int32_t  GTE_RotateLongMat(uint32_t mAddr, int32_t *src, int32_t *dst);
extern long ratan2(int y, int x);
extern void RotMatrixYXZ_gte(void *euler, void *m);
extern uint32_t *Object_Pool_Alloc(uint32_t size);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int size, int flags);
extern uint32_t V8_RandNext(void);
extern void ImpactSparks_Spawn(void *obj);
extern void Object_BumpSubstate_Or_FX(void *parent, void *child);   /* FUN_8001d4f0 */
extern void Heap_Free(void *p);
extern int32_t DAT_8006f6f4, DAT_8006f6f8, DAT_8006f6fc;
extern int32_t DAT_8006f6e4, DAT_8006f6ea, DAT_8006f6f0;
extern int8_t  DAT_80065319;     /* g_difficulty */

uint32_t HD_TransformerAim(uint32_t *self, int mode, int divisor)
{
    if (mode == 1) goto spawn;
    if (mode != 0 && mode != 7) goto spawn;
    if (divisor == 0) return 0;

    uint32_t cx = (uint32_t)(DAT_8006f6f4 + DAT_8006f6e4 * 0x40);
    uint32_t cy = (uint32_t)(DAT_8006f6f8 + DAT_8006f6ea * 0x40);
    uint32_t cz = (uint32_t)(DAT_8006f6fc + DAT_8006f6f0 * 0x40);

    int32_t local[4];
    local[0] = ((int32_t)(self[9]  - cx) + divisor * 0x5f5) / divisor;
    local[1] = ((int32_t)(self[10] - cy) + divisor * 0x1dcd) / divisor;
    local[2] = ((int32_t)(self[11] - cz)) / divisor;

    GTE_RotateLongMat(0x8006f680u, local, local);
    self[9]  = cx;
    self[10] = cy;
    self[11] = cz;

    int16_t euler[3];
    euler[0] = (int16_t)ratan2(local[2], local[1]);
    euler[1] = 0;
    euler[2] = (int16_t)-ratan2(local[0], local[1]);

    uint32_t m[8];
    RotMatrixYXZ_gte(euler, m);

    /* Walk children -- write rotation, wrap Z. */
    uint32_t child = self[0xe];
    if (child != 0) {
        do {
            *(uint32_t *)(child + 0x10) = m[0];
            *(uint32_t *)(child + 0x14) = m[1];
            *(uint32_t *)(child + 0x18) = m[2];
            *(uint32_t *)(child + 0x1c) = m[3];
            *(uint16_t *)(child + 0x20) = (uint16_t)m[4];
            *(uint32_t *)(child + 0x2c) =
                ((*(uint32_t *)(child + 0x2c) + (uint32_t)local[2] + 0x20000) & 0x3ffff) - 0x20000;
            child = *(uint32_t *)(child + 0x34);
        } while (child != 0);
    }

    /* Spawn effect prim. */
    uint32_t *fx = Object_Pool_Alloc(0x80);
    *(uint32_t **)((uintptr_t)fx + 0x58) = self;
    *(int16_t *)((uintptr_t)fx + 0xa) = (int16_t)divisor;

spawn:
    if (DAT_80065319 > 2) {
        Heap_Free(self);
        return 0;
    }
    self[0] |= 0xa0u;

    for (int i = 0; i < 0x40; i++) {
        uint32_t *p = Object_Pool_AllocFromBank((void *)(uintptr_t)self[0x16],
            *(uint16_t *)((uintptr_t)self + 0xa), 0x80, 0);
        p[0] |= 0x410u;
        int r0 = (int)V8_RandNext(); p[0x12] = (uint32_t)((r0 << 17) >> 13);
        int r1 = (int)V8_RandNext(); p[0x13] = (uint32_t)((r1 << 17) >> 13);
        int r2 = (int)V8_RandNext(); p[0x14] = (uint32_t)((r2 << 17) >> 13);
        *(uint16_t *)(p[0xc] + 0x28) = 0x40;
        ImpactSparks_Spawn(p);
        Object_BumpSubstate_Or_FX(self, p);
    }
    return 0;
}
