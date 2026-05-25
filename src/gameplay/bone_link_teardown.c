/* bone_link_teardown.c -- recursive bone-link teardown/spawn helpers.
 *
 * Sources:
 *   SLUS_005.10 LAB_8003e8a0  -- bouncing debris callback
 *   SLUS_005.10 LAB_8003f45c  -- deferred weapon-script callback
 *   SLUS_005.10 FUN_8003f4a0  -- BoneLink_TearDown
 *
 * MED: direct line port of the MIPS/Ghidra flow with host callback-slot
 * handling centralized through Object_SetCallbackPsxSlot.
 */
#include <stdint.h>
#include "psyq/psyq_stubs.h"

extern MATRIX *FUN_8001b07c(MATRIX *out, int obj);
extern void   *FUN_8001aaa8(void *bank, uint16_t *entry, uint32_t size, uint32_t flags);
extern void   *FUN_8001ac44(void *bank, uint16_t slot, uint32_t size, uint32_t flags);
extern void   *FUN_8001d470(uint32_t size);
extern void    FUN_8001dc1c(void *obj);
extern void   *FUN_8001f5a0(intptr_t self, intptr_t query);
extern void    FUN_800202f4(void *obj);
extern void    FUN_800205f8(void *obj);
extern void    FUN_80020744(void *obj);
extern void    FUN_800207c4(void *obj);
extern void    FUN_80020890(void *obj, int mode);
extern uint32_t FUN_80017160(void);
extern int32_t  FUN_80025400(int32_t x, int32_t z);
extern uint32_t *FUN_80040a80(uint32_t ctx, uint16_t kind, uint32_t *pos);
extern void     FUN_8003f0b4(void *scriptBase, uint32_t entryIdx, MATRIX *owner);
extern void     FUN_80043358(uint32_t *mat, int32_t *src, int32_t *dst);
extern void     FUN_80045088(int ptr);
extern void     Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

extern uintptr_t DAT_800737d8;
extern int       LAB_8003e868(int obj, int event, int param3);
extern uint32_t  FUN_8003eab0(uint8_t *obj, int mode, intptr_t arg);

static void copy_matrix_words(void *dst, const MATRIX *src)
{
    uint32_t *d = (uint32_t *)((uint8_t *)dst + 0x10);
    const uint32_t *s = (const uint32_t *)src;

    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = s[6];
    d[7] = s[7];
}

static int32_t div_rtz_4096(int32_t value)
{
    if (value < 0)
        value = (int32_t)((uint32_t)value + 0xfffu);
    return value >> 12;
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

int LAB_8003e8a0(uint32_t *obj, int event, int32_t *arg)
{
    uint8_t *s = (uint8_t *)obj;

    if (event == 1) {
        uint32_t height = obj[0x15];
        uint8_t bounceClass = 3;

        if (height >= 0x2800 && height < 0x7800)
            bounceClass = 2;
        *(uint8_t *)(s + 0x87) = bounceClass;
        return 0;
    }

    if (event == 0) {
        obj[0x09] = (uint32_t)mips_addu_i32((int32_t)obj[0x09], (int32_t)obj[0x22]);
        obj[0x0a] = (uint32_t)mips_addu_i32((int32_t)obj[0x0a], (int32_t)obj[0x23]);
        obj[0x0b] = (uint32_t)mips_addu_i32((int32_t)obj[0x0b], (int32_t)obj[0x24]);
        obj[0x23] = (uint32_t)mips_addu_i32((int32_t)obj[0x23], 0x5a);

        int32_t terrainY = FUN_80025400((int32_t)obj[0x09], (int32_t)obj[0x0b]);
        if (terrainY < (int32_t)obj[0x0a]) {
            obj[0x0a] = (uint32_t)terrainY;
            int32_t negVy = mips_subu_i32(0, (int32_t)obj[0x23]);
            obj[0x23] = (uint32_t)(mips_addu_i32(negVy, (int32_t)((uint32_t)negVy >> 31)) >> 1);
            uint8_t count = (uint8_t)mips_subu_i32(*(uint8_t *)(s + 0x87), 1);
            *(uint8_t *)(s + 0x87) = count;
            if (count == 0) {
                FUN_800205f8(obj);
                return -1;
            }
        }
        return 0;
    }

    if (event == 3) {
        FUN_8001f5a0((intptr_t)obj, (intptr_t)arg);
        int32_t dot = mips_addu_i32(
            mips_addu_i32(
                mips_mult_lo_i32((int32_t)obj[0x22], *(int16_t *)((uint8_t *)arg + 0x20)),
                mips_mult_lo_i32((int32_t)obj[0x23], *(int16_t *)((uint8_t *)arg + 0x22))),
            mips_mult_lo_i32((int32_t)obj[0x24], *(int16_t *)((uint8_t *)arg + 0x24)));
        int32_t push = div_rtz_4096(mips_sll_i32(dot, 1));

        if (push < 0) {
            obj[0x22] = (uint32_t)mips_subu_i32((int32_t)obj[0x22],
                div_rtz_4096(mips_mult_lo_i32(push, *(int16_t *)((uint8_t *)arg + 0x20))));
            obj[0x23] = (uint32_t)mips_subu_i32((int32_t)obj[0x23],
                div_rtz_4096(mips_mult_lo_i32(push, *(int16_t *)((uint8_t *)arg + 0x22))));
            obj[0x24] = (uint32_t)mips_subu_i32((int32_t)obj[0x24],
                div_rtz_4096(mips_mult_lo_i32(push, *(int16_t *)((uint8_t *)arg + 0x24))));
        }

        uint32_t height = obj[0x15];
        uint8_t bounceClass = 3;
        if (height >= 0x2800 && height < 0x7800)
            bounceClass = 2;
        *(uint8_t *)(s + 0x87) = bounceClass;
        return 0;
    }

    return 0;
}

int LAB_8003f45c(uint32_t *obj, int event, int param3)
{
    (void)param3;

    if (event == 2) {
        FUN_8003f0b4((void *)(uintptr_t)obj[0x16],
                     *(uint16_t *)((uint8_t *)obj + 0x0a),
                     (MATRIX *)(obj + 4));
        FUN_80045088((int)(uintptr_t)obj);
    }
    return 0;
}

uint32_t *FUN_8003f4a0(int *bank, uint16_t entryIdx, MATRIX *parentMat, int spawnFlag)
{
    uint32_t *lastObj = 0;

    for (;;) {
        if (entryIdx == 0xffff)
            return lastObj;

        uint16_t *entry = (uint16_t *)((uintptr_t)*bank + (uint32_t)entryIdx * 0x1c + 0x1c);
        MATRIX localMat;
        MATRIX *entryMat = FUN_8001b07c(&localMat, (int)(uintptr_t)entry);
        CompMatrixLV(parentMat, entryMat, &localMat);

        uint32_t *nextLast = lastObj;
        uint16_t opcode = entry[0];

        if (opcode == 0x8502) {
            if (spawnFlag != 0) {
                nextLast = (uint32_t *)FUN_80040a80((uint32_t)DAT_800737d8, 0x12, (uint32_t *)parentMat->t);
                nextLast[0] |= 0x20000u;
                FUN_80020744(nextLast);
                FUN_800207c4(nextLast);
                FUN_80020890(nextLast, 0x1e0);
                if (lastObj != 0) {
                    nextLast[0x0d] = (uint32_t)(uintptr_t)lastObj;
                    lastObj[0x0f] = (uint32_t)(uintptr_t)nextLast;
                }
            }
            entryIdx = entry[0x0c];
            lastObj = nextLast;
            continue;
        }

        if (opcode == 0xffff && spawnFlag != 0) {
spawn_small:
            nextLast = (uint32_t *)FUN_8001ac44(bank, entryIdx, 0x80, 8);
            Object_SetCallbackPsxSlot(nextLast, (uintptr_t)&LAB_8003e868);
            if (lastObj != 0) {
                nextLast[0x0d] = (uint32_t)(uintptr_t)lastObj;
                lastObj[0x0f] = (uint32_t)(uintptr_t)nextLast;
            }
            entryIdx = entry[0x0c];
            lastObj = nextLast;
            continue;
        }

        switch (opcode >> 12) {
        case 0:
            if (spawnFlag != 0 && entry[0x0b] != 0xaaaa)
                goto spawn_small;

            lastObj = (uint32_t *)FUN_8001aaa8(bank, entry, 0x94, 0);
            *(uint16_t *)((uint8_t *)lastObj + 0x80) = (uint16_t)FUN_80017160() & 0xff;
            *(uint16_t *)((uint8_t *)lastObj + 0x82) = (uint16_t)FUN_80017160() & 0xff;
            *(uint16_t *)((uint8_t *)lastObj + 0x84) = (uint16_t)FUN_80017160() & 0xff;
            goto setup_bouncy_obj;

        case 8:
        case 9:
        case 0xe:
            if (entry[0x0b] == 0xaaaa || entry[0x0b] == 0) {
                FUN_8003f0b4(bank, entryIdx, &localMat);
            } else {
                uint32_t *delayObj = (uint32_t *)FUN_8001d470(0x80);
                copy_matrix_words(delayObj, &localMat);
                delayObj[0x16] = (uint32_t)(uintptr_t)bank;
                *(uint16_t *)((uint8_t *)delayObj + 0x0a) = entryIdx;
                Object_SetCallbackPsxSlot(delayObj, (uintptr_t)&LAB_8003f45c);
                *(uint16_t *)((uint8_t *)delayObj + 0x0c) = entry[0x0b];
                FUN_80020890(delayObj, entry[0x0b]);
            }
            break;

        case 0xd:
            entry[0] = opcode & 0x0fff;
            lastObj = (uint32_t *)FUN_8001aaa8(bank, entry, 0x94, 0);
            entry[0] |= 0xd000;
            if (entry[0x0b] != 0) {
                *(uint16_t *)((uint8_t *)lastObj + 0x80) = (uint16_t)FUN_80017160() & 0xff;
                *(uint16_t *)((uint8_t *)lastObj + 0x82) = (uint16_t)FUN_80017160() & 0xff;
                *(uint16_t *)((uint8_t *)lastObj + 0x84) = (uint16_t)FUN_80017160() & 0xff;
            } else {
                *(uint16_t *)((uint8_t *)lastObj + 0x80) = 0;
                *(uint16_t *)((uint8_t *)lastObj + 0x82) = 0;
                *(uint16_t *)((uint8_t *)lastObj + 0x84) = 0;
            }

setup_bouncy_obj:
            *(uint8_t *)((uint8_t *)lastObj + 0x04) = 1;
            lastObj[0] |= 0x180u;
            if (lastObj[0x17] == 0)
                lastObj[0] |= 0x1a0u;

            if ((lastObj[0] & 0x10) == 0)
                Object_SetCallbackPsxSlot(lastObj, (uintptr_t)&FUN_8003eab0);
            else
                Object_SetCallbackPsxSlot(lastObj, (uintptr_t)&LAB_8003e8a0);

            lastObj[0x22] = (uint32_t)((int32_t)((uint32_t)*(uint32_t *)(entry + 2) << 8) >> 12);
            lastObj[0x23] = (uint32_t)((int32_t)((uint32_t)*(uint32_t *)(entry + 4) << 8) >> 12);
            lastObj[0x24] = (uint32_t)((int32_t)((uint32_t)*(uint32_t *)(entry + 6) << 8) >> 12);
            FUN_80043358((uint32_t *)parentMat, (int32_t *)(lastObj + 0x22), (int32_t *)(lastObj + 0x22));

            copy_matrix_words(lastObj, &localMat);
            FUN_8001dc1c(lastObj);

            uint8_t bounceClass = 3;
            if ((int32_t)lastObj[0x15] >= 0x2800 && (int32_t)lastObj[0x15] < 0x7800)
                bounceClass = 2;
            *(uint8_t *)((uint8_t *)lastObj + 0x87) = bounceClass;
            FUN_800202f4(lastObj);
            break;
        }

        entryIdx = entry[0x0c];
    }
}
