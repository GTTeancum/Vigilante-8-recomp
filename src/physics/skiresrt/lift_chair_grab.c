/* lift_chair_grab.c -- Ski Resort lift-chair "grab vehicle" event.
 *
 * Source: SKIRESRT.DLL  FUN_80101464.
 *
 * When a vehicle drives into a ski-lift chair's collision volume
 * (impactor.kind==2, impactor signed-low<0, and chair's signed
 * direction at +0xa2 nonzero), the chair grabs and lifts:
 *   - rebind vehicle tick to FUN_80101050 (controlled-lift tick)
 *   - mark `*v = (~8 | 0x1000020)`
 *   - set velocity to 4 * delta-to-chair-position (overshoot Y by
 *     -0x10000 so the vehicle "lands" into the chair seat)
 *   - stop FX, schedule 0x20-frame damage timer
 *   - if vehicle has a sub-model, alloc 0x30-byte carrier descriptor
 *     with chair coords offset by (-0x32000, -0x64000) for grip
 *   - bind snare FX (Pool_BindSnareToObject)
 *
 * Always forwards to func_0x800223dc (standard impulse propagate).
 *
 * Direction lookup: chair list head at _DAT_800659fc + 0x98 or +0x9c
 * picked by (self+0x42 == 0) XOR (head+0xa0 != 0).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t Pool_AllocSnare(void);                                /* FUN_8004410c */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot, uint32_t *xyz); /* FUN_800447e8 */
extern void SFX_Update(int h, int posVoxel);                          /* FUN_80044574 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);        /* FUN_80020890 */
extern void *Heap_AllocOrRetry(uint32_t n);                           /* FUN_800116f4 */
extern void Spawner_Promote(uint32_t target);                         /* func_0x8003dbb0 */
extern void Damage_ImpulsePropagate(int self, uint32_t mode, uint32_t **arg); /* func_0x800223dc */
extern uint32_t _DAT_800659fc;
extern uint32_t FUN_80101050;

void SK_LiftChairGrab(int self, uint32_t mode, uint32_t **arg)
{
    uint32_t *vic = arg[0];
    int      sub  = (int)arg[3];
    if (*(int16_t *)(sub + 6) == 0 && (char)vic[1] == 2
        && *(int16_t *)((char *)vic + 6) < 0
        && *(int16_t *)(_DAT_800659fc + 0xa2) != 0)
    {
        uint32_t bin = vic[0x38];
        uint32_t side = (*(int16_t *)(self + 0x42) == 0);
        if (*(int16_t *)(_DAT_800659fc + 0xa0) != 0) side ^= 1u;
        uint32_t chair = *(uint32_t *)(_DAT_800659fc + side * 4 + 0x98);
        *(uint32_t **)(chair + 0x80) = vic;
        vic[0x19] = (uint32_t)(uintptr_t)&FUN_80101050;
        vic[0x1e] = chair;
        *vic = (*vic & ~8u) | 0x1000020u;
        vic[0x20] = (*(int *)(chair + 0x24) - vic[9])              * 4;
        vic[0x21] = (*(int *)(chair + 0x28) - (vic[10] - 0x10000)) * 4;
        vic[0x22] = (*(int *)(chair + 0x2c) - vic[0xb])            * 4;
        SFX_Update((int)*((char *)vic + 5), 0);
        Damage_Apply_AgainstSelf(vic, (void *)(intptr_t)0x20);
        if (bin != 0) {
            uint32_t *desc = (uint32_t *)Heap_AllocOrRetry(0x30);
            desc[0] = *(uint32_t *)(bin + 0x48);
            desc[1] = *(uint32_t *)(bin + 0x4c);
            desc[2] = *(uint32_t *)(bin + 0x50);
            desc[3] = 0x78;
            desc[4] = *(uint32_t *)(chair + 0x48);
            desc[5] = *(int *)(chair + 0x4c) - 0x32000;
            desc[6] = *(int *)(chair + 0x50) - 0x64000;
            desc[7] = 0;
            Spawner_Promote(bin);
        }
        uint32_t h = Pool_AllocSnare();
        Pool_BindSnareToObject(h, *(uint32_t *)(*(int *)(self + 0x58) + 8), 5, vic + 9);
    }
    Damage_ImpulsePropagate(self, mode, arg);
}
