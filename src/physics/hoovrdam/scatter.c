/* scatter.c -- Hoover Dam parking_meter/transformer scatter (same idiom
 * as CC_RandomScatterTick).
 *
 * Source: HOOVRDAM.DLL  FUN_80100a18.
 *
 * Identical body to CC_RandomScatterTick (CASNOCTY.DLL spawner2.c):
 * decrement obj+0x80 countdown; on -1 spawn random-position child
 * in obj+0x54 cube; reload countdown. Plus extra guard: returns
 * early if obj has children at +0x38, else cascades to
 * Damage_Apply(self) and clears the child-bone slot at +0x60.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t V8_RandNext(void);
extern uint32_t *Object_Pool_AllocFromBank(void *bank, uint16_t kind, int u, int flags);
extern void Object_BumpSubstate_Or_FX(int obj);     /* FUN_8001d4f0 */
extern void Damage_Apply(void *obj);

uint32_t HD_ScatterTick(int obj, uint32_t mode)
{
    if (mode == 2) goto applyTermFlag;
    if (mode != 5 && mode != 0) goto clearChildSlot;

    int16_t *countdown = (int16_t *)(intptr_t)(obj + 0x80);
    int16_t prev = *countdown;
    *countdown = (int16_t)(prev - 1);
    if (prev == 0) {
        uint32_t *child = Object_Pool_AllocFromBank(
            (void *)(uintptr_t)*(uint32_t *)(obj + 0x98),
            *(uint16_t *)(obj + 0x96), 0x80, 8);
        int32_t r = *(int32_t *)(obj + 0x54);
        int32_t rx = ((int)V8_RandNext() * 2 * r >> 15) - r;
        int32_t rz = ((int)V8_RandNext() * 2 * r >> 15) - r;
        *(int32_t *)((uint8_t *)child + 0x28) = 0;
        *(int32_t *)((uint8_t *)child + 0x24) = rx;
        *(int32_t *)((uint8_t *)child + 0x2c) = rz;
        *(uint32_t *)((uint8_t *)child + 100) = 0x8003e80c;
        Object_BumpSubstate_Or_FX(obj);
        *countdown = *(int16_t *)(obj + 0x82);
    }
    if (*(int *)(obj + 0x38) != 0) return 0;
    Damage_Apply((void *)(intptr_t)obj);

clearChildSlot:
    *(int *)(obj + 0x60) = 0;
applyTermFlag:
    *(uint16_t *)(obj + 0x80) = 0xffff;
    return 0;
}
