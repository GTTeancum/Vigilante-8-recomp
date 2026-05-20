/* rig_emit.c -- Oil Fields rig steam emit + child wiggle.
 *
 * Source: OILFIELD.DLL  FUN_801002b0.
 *
 * Per-tick handler:
 *   1. Decrement countdown at obj+0x80; on -1, spawn a steam puff
 *      (func_0x80040378 allocates with shared bank), positioned at
 *      origin with random Z-velocity jitter. Then reload countdown.
 *   2. Walk each child @+0x38..+0x34 chain: pos += vel each frame,
 *      vy += -0x40 (light gravity), and set their +0x90 ptr to
 *      walk-back-one-slot (`+0x90 - 0x20`).
 *   3. Damage_Apply(self) at the end if a hit landed.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t *Generic_PoolAlloc(uint32_t bank);  /* func_0x80040378 */
extern uint32_t V8_RandNext(void);
extern void Object_BumpSubstate_Or_FX(int obj);
extern void Damage_Apply(void *obj);

uint32_t OF_RigEmit(int self, uint32_t mode, int *impulse)
{
    if (mode != 2 && mode != 3 && mode != 0) goto end;
    if (mode == 3 || mode == 0) {
        int16_t *cd = (int16_t *)(intptr_t)(self + 0x80);
        int16_t prev = *cd;
        *cd = (int16_t)(prev - 1);
        if (prev == 0) {
            uint32_t bank = *(uint32_t *)(self + 0x98);
            uint32_t *puff = Generic_PoolAlloc(bank);
            if (puff != NULL) {
                puff[0x22] = 0;
                puff[0x23] = 0;
                puff[0] |= 0x410u;
                int r = (int)V8_RandNext();
                int32_t vy = *(int32_t *)(self + 0x8c);
                puff[11] = 0; puff[10] = 0; puff[9] = 0;
                puff[0x19] = 0x80040470;
                puff[0x24] = (uint32_t)(vy + ((r * vy) >> 15));
                Object_BumpSubstate_Or_FX(self);
            }
            *cd = *(int16_t *)(self + 0x82);
        }
        /* Walk children. */
        for (int c = *(int *)(self + 0x38); c != 0; c = *(int *)(c + 0x34)) {
            *(int *)(c + 0x24) += *(int *)(c + 0x88);
            *(int *)(c + 0x28) += *(int *)(c + 0x8c);
            *(int *)(c + 0x2c) += *(int *)(c + 0x90);
            *(int *)(c + 0x8c) -= 0x40;
            *(int **)(c + 0x90) = (int *)(*(int *)(c + 0x90) - 0x20);
        }
        Damage_Apply((void *)(intptr_t)self);
    }
end:
    (void)impulse;
    return 0;
}
