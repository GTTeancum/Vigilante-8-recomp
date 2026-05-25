/* object_register_child.c -- allocate/register child world object.
 *
 * Source: SLUS_005.10  FUN_80040a80.
 *
 * HIGH confidence: line-for-line port of the allocator/setup routine used by
 * wheel teardown and similar child object registration paths.
 */
#include <stdint.h>

extern void *FUN_8001d470(uint32_t size);
extern void *FUN_8001ac44(uint32_t pool, uint16_t kind, uint32_t size, uint32_t align);
extern void *FUN_8001d5a0(uint32_t *obj);
extern void *FUN_8001d624(void *mat);
extern void FUN_8001d4f0(uint32_t *parent, uint32_t *child);
extern void FUN_8001d708(uint32_t *obj);
extern void FUN_800205f8(uint32_t *obj);
extern void TransposeMatrix(void *src, void *dst);
extern uint32_t FUN_80017160(void);
extern int LAB_8004042c(int obj, int event, int param3);
extern int16_t DAT_800607b4[];
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

int LAB_80040894(uint32_t *obj, int mode, int arg2, int arg3)
{
    (void)arg2;
    (void)arg3;
    uint8_t *s = (uint8_t *)obj;

    if (mode == 2) {
        *(uint16_t *)(s + 0x80) = 0xffff;
        return 0;
    }
    if (mode != 0)
        return 0;

    if (obj[0] & 0x00020000u) {
        void *m = FUN_8001d5a0(obj);
        m = FUN_8001d624(m);
        TransposeMatrix(m, s + 0x10);
    }

    uint16_t timer = (uint16_t)(*(uint16_t *)(s + 0x80) - 1);
    *(uint16_t *)(s + 0x80) = timer;
    if (timer == 0xffff) {
        uint32_t rnd = FUN_80017160();
        uint32_t *child = (uint32_t *)FUN_8001ac44(obj[0x26],
                                                    *(uint16_t *)(s + 0x96),
                                                    0xa0, 8);
        uint8_t *c = (uint8_t *)child;
        child[0] |= 0x410;
        uint32_t idx = (rnd & 0xfffu) * 2u;
        int32_t vx = obj[0x21] * DAT_800607b4[idx];
        if (vx < 0)
            vx += 0xfff;
        *(int32_t *)(c + 0x88) = vx >> 12;
        int32_t vz = obj[0x21] * DAT_800607b4[idx + 1];
        if (vz < 0)
            vz += 0xfff;
        *(int32_t *)(c + 0x90) = vz >> 12;
        rnd = FUN_80017160();
        *(int32_t *)(c + 0x24) = 0;
        *(int32_t *)(c + 0x28) = 0;
        *(int32_t *)(c + 0x2c) = 0;
        Object_SetCallbackPsxSlot(c, (uintptr_t)&LAB_8004042c);
        *(int32_t *)(c + 0x8c) = obj[0x22] + (int32_t)((int64_t)(int32_t)rnd * (int32_t)obj[0x22] >> 15);
        FUN_8001d4f0(obj, child);
        *(uint16_t *)(s + 0x80) = *(uint16_t *)(s + 0x82);
    }

    uint32_t *child = (uint32_t *)(uintptr_t)obj[0x0e];
    if (child == NULL) {
        FUN_800205f8(obj);
        return -1;
    }

    while (child != NULL) {
        child[0x09] += child[0x22];
        child[0x0a] += child[0x23];
        child[0x0b] += child[0x24];
        child = (uint32_t *)(uintptr_t)child[0x0d];
    }
    return 0;
}

uint32_t *FUN_80040a80(uint32_t ctx, uint16_t kind, uint32_t *pos)
{
    uint32_t *obj = (uint32_t *)FUN_8001d470(0x9c);
    uint8_t *s = (uint8_t *)obj;

    obj[0] |= 0xa4;
    obj[0x12] = pos[0];
    obj[0x13] = pos[1];
    obj[0x14] = pos[2];
    Object_SetCallbackPsxSlot(s, (uintptr_t)&LAB_80040894);
    obj[0x15] = 0x10000;
    FUN_8001d708(obj);
    *(uint16_t *)(s + 0x82) = 4;
    obj[0x21] = 0x200;
    obj[0x26] = ctx;
    *(uint16_t *)(s + 0x96) = kind;
    obj[0x22] = 0xfffffa00u;
    obj[0x23] = 0;
    return obj;
}
