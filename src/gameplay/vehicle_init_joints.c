/* vehicle_init_joints.c -- Vehicle sub-object initialisation loop.
 *
 * Source: SLUS_005.10  FUN_8002cce8  (580 B, 93 instr)
 *
 * Called by Vehicle_RollingTick mode 1 (init):
 *     FUN_8002cce8(self, (uint8_t)(*(uint8_t*)(self+3) | 1))
 *
 * The function iterates 7 times (i = 0..6) over the bit-mask argument.
 * For each bit i that is set it:
 *   1. Allocates a sub-object (FUN_8001ac44) from the world object pool.
 *      For i < 6 the kind and callback are read from an 8-byte descriptor
 *      table at DAT_80010534 (entries walked by a pointer s4).
 *      For i == 6 a bone-table path is used (FUN_8001b038 + state-driven
 *      callback from FUN_8003d1e8).
 *   2. If the sub-object has a tick callback, calls it with mode=1 (init).
 *   3. Gets the joint slot via FUN_8003d188(self, sub_obj); if non-null,
 *      links with FUN_8001b2fc(self, joint, sub_obj).
 *      If null, calls FUN_80015368 (error/trap).
 *   4. Stores the sub-object pointer into the vehicle:
 *      - i == 0  -> child slot [4] at *(self + 0x10C)
 *      - i != 0  -> first empty node slot in self+0x114..0x11C
 *                   (*(self + (slot+9)*4 + 0xEC) = sub_obj)
 *
 * Supporting helper: FUN_8003d1e8 (Vehicle_StateToCallback)
 *   Maps a state code (u8, < 0xD) to a function pointer via a 13-entry
 *   jump table at DAT_8005ecb0.  Returns NULL for out-of-range codes.
 *
 * HIGH confidence: line-by-line MIPS port.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

/* ============================================================
 * FUN_8003e76c  Vehicle_RegisterInWorld  (15 instr, 68 B)
 *
 * Acquires a "kind 14" world-registry slot from the global world
 * context at *(0x800737D4), sets bit 3 in that slot's flags, and
 * calls FUN_8003e598 to insert the object into the spatial
 * registry (KDTree / world-object list).
 *
 * Called by Vehicle_RollingTick mode 1 immediately after the joint
 * initialisation below.
 *
 * HIGH confidence: 15 instructions decoded in full.
 * ============================================================ */
extern void   *FUN_8001bda0(void *world_ctx, int kind); /* WorldReg_AllocSlot */
extern void    FUN_8003e598(uint32_t *obj, void *slot);  /* KDTree_Insert */
extern uintptr_t DAT_800737d4;  /* ptr to world context (gp+0x37D4) */

void FUN_8003e76c(uint32_t *self)
{
    /* Allocate a kind-14 slot from the world context. */
    void *slot = FUN_8001bda0((void *)(uintptr_t)DAT_800737d4, 14);

    /* Set bit 3 (0x8) in the slot's u16 flags field. */
    uint16_t flags = *(uint16_t *)slot;
    flags |= 0x8u;
    *(uint16_t *)slot = flags;

    /* Insert self into the world spatial registry. */
    FUN_8003e598(self, slot);
}

/* ============================================================
 * Supporting table for FUN_8002cce8:
 * Sub-object descriptor table in the EXE at 0x80010534:
 *   [kind:u16, pad:u16, callback_fn:u32] * 6.
 * Host stores native callback pointers in this decoded table. */
typedef intptr_t (*VehicleSubObjFn)(intptr_t, int, intptr_t);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
typedef struct {
    uint16_t kind;
    uint16_t pad;
    VehicleSubObjFn callback;
} VehicleJointInitDesc;

extern intptr_t FUN_80031864(intptr_t obj, int event, intptr_t arg);
extern intptr_t LAB_80031fa0(intptr_t obj, int event, intptr_t arg);
extern intptr_t LAB_8003302c(intptr_t obj, int event, intptr_t arg);
extern intptr_t LAB_800336fc(intptr_t obj, intptr_t event, int arg);
extern intptr_t FUN_80034920(uint32_t *obj, uint32_t event, intptr_t arg);
extern intptr_t LAB_8003565c(intptr_t obj, int event, intptr_t arg);

static const VehicleJointInitDesc DAT_80010534_host[6] = {
    { 0x0002, 0, (VehicleSubObjFn)FUN_80031864 },
    { 0x0000, 0, (VehicleSubObjFn)LAB_80031fa0 },
    { 0x0011, 0, (VehicleSubObjFn)LAB_8003302c },
    { 0x0007, 0, (VehicleSubObjFn)LAB_800336fc },
    { 0x000a, 0, (VehicleSubObjFn)FUN_80034920 },
    { 0x000d, 0, (VehicleSubObjFn)LAB_8003565c },
};

/* DAT_800737a0[15] is the common sub-object pool (MIPS: lw a0,0x3c(s5)
 * with s5=&DAT_800737a0, not *(DAT_800737a0[0]+0x3c)). */
extern uintptr_t DAT_800737a0[];

extern void   *FUN_8001ac44(int *bank, uint16_t kind, uint32_t size, uint32_t flags);
extern void   *FUN_8001d470(uint32_t size);
extern intptr_t FUN_8001b038(uint32_t *obj, uint32_t arg);      /* bone table query */
extern void   *FUN_8003d1e8(uint8_t state_code);                /* state->callback */
extern void    FUN_8001b2fc(uint32_t *chassis, const void *joint, uint32_t *wheel);
extern int     FUN_80015368(const char *msg);                   /* error trap */
extern int     FUN_8003c538(uint32_t *obj, intptr_t arg);
extern int32_t FUN_80016aac(const int32_t *a, const int32_t *b);
extern int16_t *FUN_80016cec(int16_t *m, int16_t *angles);
extern void   *FUN_8001d624(int obj);
extern void    FUN_8001fcb4(int payload, uint16_t arg);
extern void    FUN_8004352c(uint32_t *m, int32_t *v, int32_t *out);
extern void    FUN_800435c0(uint32_t *m, int32_t *v, int32_t *out);
extern int     FUN_8004ecd4(int y, int x);
extern void    FUN_8001d708(uint32_t *obj);
extern int32_t iRam00000758;
extern void   *FUN_8003351c(intptr_t self, uint32_t *owner, int16_t kind,
                            uint16_t timer);
extern uint32_t *FUN_80031300(intptr_t parent, intptr_t subBin, uint16_t kind,
                              uint32_t size, intptr_t jointTpl);
extern void    FUN_8002cb7c(intptr_t slot);
extern void    FUN_8001ac08(uint32_t *self);
extern void    FUN_8001d544(uint32_t *parent, uint32_t *child);
extern MATRIX *FUN_8001b07c(MATRIX *out, int obj);
extern MATRIX *CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
extern SVECTOR *ApplyMatrixSV(const MATRIX *m, const SVECTOR *v0, SVECTOR *v1);
extern uint32_t FUN_8002036c(uint32_t *obj);
extern void FUN_80020620(intptr_t obj, uint32_t event);
extern void FUN_800202f4(void *obj);
extern void FUN_800205f8(intptr_t obj);
extern void FUN_800207c4(void *obj);
extern void FUN_80020744(void *obj);
extern void FUN_800207f8(void *obj);
extern uint32_t FUN_80020778(uint32_t *obj);
extern void FUN_80020890(intptr_t obj, int timer);
extern uint32_t FUN_80017160(void);
extern uint32_t *FUN_8003fd24(const int32_t *xyz, int kind);
extern uint32_t *FUN_8003fdcc(const int32_t *xyz, int kind, int arg);
extern int FUN_8001d840(intptr_t obj, int32_t *pos, void *normalOut);
extern void *FUN_8003fea8(int32_t *xyz, uint32_t colour);
extern void FUN_8002c4bc(uint32_t *obj);
extern void FUN_8002c3ac(uint32_t *obj);
extern void FUN_80012050(int slot, uint8_t type);
extern void FUN_80017594(uint32_t *obj, const int32_t *impulse,
                         const int32_t *pos);
extern int FUN_8004410c(void);
extern void FUN_800441c8(int voice);
extern void FUN_8004445c(int voice, uint32_t bank, int sfx);
extern void FUN_8004483c(int voice, uint32_t bank, int sfx, int *pos);
extern void FUN_800447e8(int voice, uint32_t bank, int sfx, const void *pos);
extern uint32_t DAT_800737d8;
extern uint32_t uRam000005f8;
extern int32_t iRam00000758;
extern int FUN_80025400(int x, int z);
extern uint32_t FUN_8003733c(uint32_t *obj, uint32_t event, int *arg);
extern int FUN_8001dc1c(intptr_t obj);
extern void FUN_8001d4f0(uint32_t *parent, uint32_t *child);
extern intptr_t FUN_8001d5a0(intptr_t obj);
extern int FUN_8003fc50(int obj);
extern intptr_t FUN_8001d564(intptr_t self);
extern void FUN_800204dc(int obj);
extern void FUN_8001af48(int obj);
extern void FUN_8001d68c(MATRIX *out, intptr_t parent, intptr_t joint);
extern void FUN_80043358(uint32_t *m, int32_t *v, int32_t *out);
extern void FUN_8002ca94(uint32_t *obj, int slot);
extern void FUN_80016bd8(int16_t *out, int32_t *from, int32_t *to);
extern void FUN_800176f8(intptr_t obj, int32_t *vec, int32_t *pos);
extern int FUN_8001d748(intptr_t obj, int *posXyz, void *normalOut, uintptr_t *materialOut);
extern void *FUN_8001f5a0(intptr_t self, intptr_t query);
extern uint32_t FUN_800449bc(void *srcPos);
extern void FUN_80044574(int voice, uint32_t pan);
extern int FUN_800443c8(int voice, uint32_t bank, int sfxId, int aux);
extern void FUN_80012068(int idx, uint8_t b5, uint8_t b6, uint8_t b7);
extern uint16_t DAT_8005eca8[4];
extern int16_t DAT_800607b4[];
extern int32_t DAT_800657b0[3];
extern int32_t DAT_800657cc[3];
extern uintptr_t DAT_800911a0[];
extern long SquareRoot0(long n);

intptr_t LAB_8003b138(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003b1e0(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003a9dc(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003846c(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_800359c0(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_80036ad8(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_800363e0(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003959c(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_80038cf8(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_80038d18(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_80037b94(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003b8d4(intptr_t obj, int event, intptr_t arg);
intptr_t LAB_8003bde0(intptr_t obj, int event, intptr_t arg);
extern int LAB_8003e7b4(int obj, int event, int param3);
extern int LAB_8003e80c(int obj, int event, int param3);
extern int FUN_8003eab0(int obj, int event, int param3);

static int weapon_trace_enabled(void)
{
    static int checked = 0;
    static int enabled = 0;
    if (!checked) {
        enabled = getenv("V8_TRACE_WEAPONS") != NULL;
        checked = 1;
    }
    return enabled;
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static inline int32_t mips_signext12_i32(int32_t v)
{
    return (int32_t)((uint32_t)v << 20) >> 20;
}

static inline int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static inline int32_t rtz_shift7(int32_t v)
{
    if (v < 0)
        v = mips_addu_i32(v, 0x7f);
    return v >> 7;
}

static inline int32_t rtz_shift4(int32_t v)
{
    if (v < 0)
        v = mips_addu_i32(v, 0x0f);
    return v >> 4;
}

static inline int32_t rtz_shift2(int32_t v)
{
    if (v < 0)
        v = mips_addu_i32(v, 3);
    return v >> 2;
}

static inline int32_t rtz_shift11(int32_t v)
{
    if (v < 0)
        v = mips_addu_i32(v, 0x7ff);
    return v >> 11;
}

static inline int32_t rtz_shift12(int32_t v)
{
    if (v < 0)
        v = mips_addu_i32(v, 0xfff);
    return v >> 12;
}

static inline int32_t state5_projectile_velocity(int32_t ownerVel,
                                                 int16_t axisA,
                                                 int16_t axisB)
{
    int32_t base = rtz_shift7(ownerVel);
    int32_t termA = rtz_shift12(mips_mult_lo_i32((int32_t)axisA, 1750));
    int32_t termB = rtz_shift12(mips_mult_lo_i32((int32_t)axisB, 800));
    return mips_addu_i32(mips_subu_i32(base, termA), termB);
}

static inline int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

intptr_t LAB_8003a9dc(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        *(int32_t *)(self + 0x48) = mips_addu_i32(*(int32_t *)(self + 0x48),
                                                  *(int32_t *)(self + 0x88));
        *(int32_t *)(self + 0x4c) = mips_addu_i32(*(int32_t *)(self + 0x4c),
                                                  *(int32_t *)(self + 0x8c));
        *(int32_t *)(self + 0x50) = mips_addu_i32(*(int32_t *)(self + 0x50),
                                                  *(int32_t *)(self + 0x90));
        *(int32_t *)(self + 0x24) = *(int32_t *)(self + 0x48);
        *(int32_t *)(self + 0x28) = *(int32_t *)(self + 0x4c);
        *(int32_t *)(self + 0x2c) = *(int32_t *)(self + 0x50);
        *(int32_t *)(self + 0x8c) = mips_addu_i32(*(int32_t *)(self + 0x8c), 0x38);
        if (FUN_80025400(*(int32_t *)(self + 0x48), *(int32_t *)(self + 0x50)) <
            *(int32_t *)(self + 0x4c)) {
            uint32_t *fx = FUN_8003fdcc((int32_t *)(self + 0x48), 0x2e, 0x28);
            if (fx != NULL)
                FUN_8001d840((intptr_t)fx, (int32_t *)((uint8_t *)fx + 0x48), fx + 4);
            FUN_80020620(obj, 1);
            return -1;
        }
        return 0;
    }

    if (event == 3 && arg != 0) {
        uint8_t *hit = *(uint8_t **)(uintptr_t)arg;
        FUN_8003fdcc((int32_t *)(self + 0x48), 0x2e, 0);
        FUN_8003fd24((int32_t *)(self + 0x48), 0x0c);
        FUN_8004483c(FUN_8004410c(), uRam000005f8, 0x39, (int *)(self + 0x48));
        if (hit != NULL && hit[4] == 2) {
            if (*(uint16_t *)(hit + 0x11e) == 0) {
                FUN_8003fea8((int32_t *)(hit + 0x24), 0x080000ffu);
                FUN_8002c3ac((uint32_t *)hit);
            }
        }
        FUN_80020620(obj, 0);
        return -1;
    }

    return 0;
}

intptr_t LAB_8003bde0(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;
    (void)arg;

    if (event == 0) {
        int16_t timer = *(int16_t *)(self + 0x96);
        if (timer != 0) {
            timer = (int16_t)(timer - 1);
            *(uint16_t *)(self + 0x96) = (uint16_t)timer;
            if (timer == 0) {
                *(uint32_t *)self &= ~2u;
                FUN_800207c4(self);
            }
        } else {
            *(int32_t *)(self + 0x24) = mips_addu_i32(*(int32_t *)(self + 0x24),
                                                      *(int32_t *)(self + 0x88));
            *(int32_t *)(self + 0x28) = mips_addu_i32(*(int32_t *)(self + 0x28),
                                                      *(int32_t *)(self + 0x8c));
            *(int32_t *)(self + 0x2c) = mips_addu_i32(*(int32_t *)(self + 0x2c),
                                                      *(int32_t *)(self + 0x90));
        }
        return 0;
    }
    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }
    return 0;
}

intptr_t LAB_8003b8d4(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        int16_t timer = (int16_t)(*(uint16_t *)(self + 0x96) - 1u);
        int16_t delta[3] = {0, 0, 0};
        *(uint16_t *)(self + 0x96) = (uint16_t)timer;
        if (timer < 0) {
            if (timer < -0x78) {
                FUN_800205f8(obj);
                iRam00000758 = mips_addu_i32(iRam00000758, -1);
                return 0;
            }
            delta[1] = -0x1000;
        } else {
            uint8_t *target = (uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x84);
            if (target != NULL)
                FUN_80016bd8(delta, (int32_t *)(self + 0x48), (int32_t *)(target + 0x48));
        }

        *(int32_t *)(self + 0x88) =
            clamp_i32(mips_subu_i32(mips_addu_i32(*(int32_t *)(self + 0x88),
                                                  rtz_shift4(delta[0])),
                                    rtz_shift7(*(int32_t *)(self + 0x88))),
                      -0x3b9a, 0x4786);

        {
            int32_t terrain = FUN_8001d748(obj, (int *)(self + 0x48), NULL, NULL);
            int32_t lift = rtz_shift4(mips_subu_i32(terrain, mips_addu_i32(*(int32_t *)(self + 0x4c), 0x2800)));
            if (lift > 0)
                lift = 0;
            *(int32_t *)(self + 0x8c) =
                clamp_i32(mips_addu_i32(mips_subu_i32(mips_addu_i32(*(int32_t *)(self + 0x8c),
                                                                      rtz_shift4(delta[1])),
                                                        rtz_shift7(*(int32_t *)(self + 0x8c))),
                                        lift),
                          -0xbeb, 0x4786);
        }

        *(int32_t *)(self + 0x90) =
            clamp_i32(mips_subu_i32(mips_addu_i32(*(int32_t *)(self + 0x90),
                                                  rtz_shift4(delta[2])),
                                    rtz_shift7(*(int32_t *)(self + 0x90))),
                      -0x3b9a, 0x4786);

        *(int32_t *)(self + 0x48) = mips_addu_i32(*(int32_t *)(self + 0x48),
                                                  *(int32_t *)(self + 0x88));
        *(int32_t *)(self + 0x4c) = mips_addu_i32(*(int32_t *)(self + 0x4c),
                                                  *(int32_t *)(self + 0x8c));
        *(int32_t *)(self + 0x50) = mips_addu_i32(*(int32_t *)(self + 0x50),
                                                  *(int32_t *)(self + 0x90));
        *(uint16_t *)(self + 0x42) = (uint16_t)(*(uint16_t *)(self + 0x42) + 0x88u);
        if (arg != 0) {
            FUN_8001d708((uint32_t *)self);
            FUN_80044574((int8_t)self[5], FUN_800449bc(self + 0x24));
            *(uint32_t *)self &= 0xfffe7fffu;
            if ((*(uint32_t *)self & 0x10000u) != 0)
                *(uint32_t *)self |= 0x8000u;
        }
        return 0;
    }

    if (event == 1) {
        self[5] = (uint8_t)FUN_8004410c();
        FUN_800443c8((int8_t)self[5],
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x58) + 8),
                     0, 0);
        return 0;
    }
    if (event == 4) {
        FUN_800441c8((int8_t)self[5]);
        return 0;
    }
    if (event == 9) {
        if (*(uint32_t *)(self + 0x84) == (uint32_t)(uintptr_t)arg)
            *(uint32_t *)(self + 0x84) = *(uint32_t *)(self + 0x80);
        return 0;
    }
    if (event == 3 && arg != 0) {
        uint8_t *hit = *(uint8_t **)(uintptr_t)arg;
        if (hit != NULL && hit[4] != 3) {
            int32_t dot;
            int32_t impulse[3];
            FUN_8001f5a0(obj, arg);
            dot = rtz_shift11(mips_addu_i32(mips_addu_i32(
                    mips_mult_lo_i32(*(int32_t *)(self + 0x88), *(int16_t *)((uint8_t *)arg + 0x20)),
                    mips_mult_lo_i32(*(int32_t *)(self + 0x8c), *(int16_t *)((uint8_t *)arg + 0x22))),
                    mips_mult_lo_i32(*(int32_t *)(self + 0x90), *(int16_t *)((uint8_t *)arg + 0x24))));
            if (dot < 0) {
                if (hit[4] == 2) {
                    impulse[0] = mips_sll_i32(*(int32_t *)(self + 0x88), 4);
                    impulse[1] = mips_sll_i32(*(int32_t *)(self + 0x8c), 4);
                    impulse[2] = mips_sll_i32(*(int32_t *)(self + 0x90), 4);
                    FUN_800176f8((intptr_t)hit, impulse, (int32_t *)(self + 0x48));
                    if ((*(uint32_t *)self & 0x8000u) == 0) {
                        FUN_8004483c(FUN_8004410c(), uRam000005f8, 5, (int *)(self + 0x48));
                        FUN_8003fd24((int32_t *)(self + 0x48), 0x2d);
                        FUN_8003fea8((int32_t *)(self + 0x48), 0x08404040u);
                        if (*(int16_t *)(hit + 6) < 0)
                            FUN_80012068(~(int)*(int16_t *)(hit + 6), 0xc0, 0, 0x40);
                    }
                    *(uint32_t *)self |= 0x18000u;
                }
                *(int32_t *)(self + 0x88) =
                    mips_subu_i32(*(int32_t *)(self + 0x88),
                                  rtz_shift12(mips_mult_lo_i32(dot, *(int16_t *)((uint8_t *)arg + 0x20))));
                *(int32_t *)(self + 0x8c) =
                    mips_subu_i32(*(int32_t *)(self + 0x8c),
                                  rtz_shift12(mips_mult_lo_i32(dot, *(int16_t *)((uint8_t *)arg + 0x22))));
                *(int32_t *)(self + 0x90) =
                    mips_subu_i32(*(int32_t *)(self + 0x90),
                                  rtz_shift12(mips_mult_lo_i32(dot, *(int16_t *)((uint8_t *)arg + 0x24))));
            }
        }
    }
    return 0;
}

intptr_t LAB_8003b138(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }

    if (event == 3) {
        uint32_t *hit = *(uint32_t **)(uintptr_t)arg;
        if (hit != NULL && *(uint8_t *)((uint8_t *)hit + 4) == 2) {
            FUN_8003fea8((int32_t *)(self + 0x24), 0x08808000u);
            *(uint32_t *)self |= 0x20u;
            FUN_8002c4bc(hit);
            if (*(int16_t *)((uint8_t *)hit + 6) < 0)
                FUN_80012050(~(int)*(int16_t *)((uint8_t *)hit + 6), 0x0a);
        }
    }

    return 0;
}

intptr_t LAB_8003b1e0(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event != 0) {
        if (event == 3)
            goto impact;
        return 0;
    }

    {
        uint16_t life = (uint16_t)(*(uint16_t *)(self + 0x94) + 1u);
        int32_t scale;

        *(uint16_t *)(self + 0x94) = life;
        scale = (int32_t)(int16_t)life * 5;
        if (scale < 0x21)
            scale = 0x21;
        else if (scale > 0x12c)
            scale = 0x12c;
        *(uint16_t *)(self + 0x0c) = (uint16_t)scale;
    }

    *(int32_t *)(self + 0x24) =
        mips_addu_i32(*(int32_t *)(self + 0x24), *(int32_t *)(self + 0x88));
    *(int32_t *)(self + 0x28) =
        mips_addu_i32(*(int32_t *)(self + 0x28), *(int32_t *)(self + 0x8c));
    *(int32_t *)(self + 0x2c) =
        mips_addu_i32(*(int32_t *)(self + 0x2c), *(int32_t *)(self + 0x90));
    *(int32_t *)(self + 0x8c) =
        mips_addu_i32(*(int32_t *)(self + 0x8c), 0x38);

    {
        uint8_t *child = (uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x38);
        if (child != NULL) {
            *(uint16_t *)(child + 0x44) =
                (uint16_t)(*(uint16_t *)(child + 0x44) + 0x22u);
            if (arg != 0)
                FUN_8001d708((uint32_t *)child);
        }
    }

    if (FUN_80025400(*(int32_t *)(self + 0x24),
                    *(int32_t *)(self + 0x2c)) < *(int32_t *)(self + 0x28))
        return 0;

impact:
    {
        uint8_t *burst = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                0x2b, 0x80u, 8u);
        if (burst != NULL) {
            uint8_t *child;

            *(uint8_t *)(burst + 4) = 7;
            *(uint32_t *)burst = 0x514u;
            *(uint16_t *)(burst + 6) = *(uint16_t *)(self + 6);
            *(int32_t *)(burst + 0x48) = *(int32_t *)(self + 0x24);
            *(int32_t *)(burst + 0x4c) = *(int32_t *)(self + 0x28);
            *(int32_t *)(burst + 0x50) = *(int32_t *)(self + 0x2c);
            Object_SetCallbackPsxSlot(burst, (uintptr_t)&LAB_8003b138);

            child = (uint8_t *)(uintptr_t)*(uint32_t *)(burst + 0x38);
            if (child != NULL) {
                Object_SetCallbackPsxSlot(child, (uintptr_t)&LAB_8003e7b4);
                *(uint32_t *)child = 0x410u;
            }
            FUN_8002036c((uint32_t *)burst);
        }

        FUN_8003fd24((const int32_t *)(self + 0x24), 0x1b);
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x41, self + 0x24);
        FUN_80020620(obj, event == 0 ? 1u : 0u);
        iRam00000758 = mips_subu_i32(iRam00000758, 1);
    }

    return 0;
}

intptr_t LAB_8003846c(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    switch (event) {
    case 0:
        *(int32_t *)(self + 0x24) =
            mips_addu_i32(*(int32_t *)(self + 0x24), *(int32_t *)(self + 0x88));
        *(int32_t *)(self + 0x28) =
            mips_addu_i32(*(int32_t *)(self + 0x28), *(int32_t *)(self + 0x8c));
        *(int32_t *)(self + 0x2c) =
            mips_addu_i32(*(int32_t *)(self + 0x2c), *(int32_t *)(self + 0x90));
        *(int32_t *)(self + 0x48) = *(int32_t *)(self + 0x24);
        *(int32_t *)(self + 0x4c) = *(int32_t *)(self + 0x28);
        *(int32_t *)(self + 0x50) = *(int32_t *)(self + 0x2c);
        *(uint16_t *)(self + 0x94) = (uint16_t)(*(uint16_t *)(self + 0x94) + 1u);
        if (*(uint16_t *)(self + 0x94) >= 0x79u) {
            FUN_80020620(obj, 1);
            iRam00000758 = mips_subu_i32(iRam00000758, 1);
        }
        return 0;
    case 2:
    case 5:
        FUN_800205f8(obj);
        return -1;
    case 3: {
        uint8_t *hit = arg ? (uint8_t *)(uintptr_t)*(uint32_t *)(uintptr_t)arg : NULL;
        if (hit != NULL && hit[4] == 2) {
            FUN_8003fd24((const int32_t *)(self + 0x24), 0x17);
            if (*(int16_t *)(hit + 6) < 0)
                FUN_80012050(~(int)*(int16_t *)(hit + 6), 0x0a);
            FUN_80020620(obj, 0);
            iRam00000758 = mips_subu_i32(iRam00000758, 1);
            return -1;
        }
        return 0;
    }
    default:
        return 0;
    }
}

intptr_t LAB_800359c0(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        *(int32_t *)(self + 0x48) =
            mips_addu_i32(*(int32_t *)(self + 0x48), *(int32_t *)(self + 0x88));
        *(int32_t *)(self + 0x4c) =
            mips_addu_i32(*(int32_t *)(self + 0x4c), *(int32_t *)(self + 0x8c));
        *(int32_t *)(self + 0x50) =
            mips_addu_i32(*(int32_t *)(self + 0x50), *(int32_t *)(self + 0x90));
        *(int32_t *)(self + 0x24) = *(int32_t *)(self + 0x48);
        *(int32_t *)(self + 0x28) = *(int32_t *)(self + 0x4c);
        *(int32_t *)(self + 0x2c) = *(int32_t *)(self + 0x50);

        *(uint16_t *)(self + 0x94) = (uint16_t)(*(uint16_t *)(self + 0x94) - 1u);
        if ((int16_t)*(uint16_t *)(self + 0x94) == -1 ||
            FUN_80025400(*(int32_t *)(self + 0x48),
                         *(int32_t *)(self + 0x50)) < *(int32_t *)(self + 0x4c))
        {
            uint32_t *fx = FUN_8003fd24((const int32_t *)(self + 0x48), 0x10);
            if (fx != NULL) {
                *(uint16_t *)((uint8_t *)fx + 0x44) = (uint16_t)FUN_8002036c(fx);
                FUN_8001d708(fx);
            }
            FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x39, self + 0x48);
            FUN_80020620(obj, 1);
        }
        return 0;
    }

    if (event == 3) {
        uint8_t *hit = arg ? (uint8_t *)(uintptr_t)*(uint32_t *)(uintptr_t)arg : NULL;
        uint32_t *fx;
        if (hit != NULL && hit[4] == 3)
            return -1;
        fx = FUN_8003fd24((const int32_t *)(self + 0x48), 0x10);
        if (fx != NULL) {
            *fx |= 0x400u;
            *(uint16_t *)((uint8_t *)fx + 0x44) = (uint16_t)FUN_8002036c(fx);
            FUN_8001d708(fx);
        }
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x39, self + 0x48);
        if (hit != NULL && hit[4] == 2) {
            int32_t impulse[3];
            impulse[0] = *(int32_t *)(self + 0x88) << 4;
            impulse[1] = *(int32_t *)(self + 0x8c) << 4;
            impulse[2] = *(int32_t *)(self + 0x90) << 4;
            FUN_80017594((uint32_t *)hit, impulse, (const int32_t *)(self + 0x24));
            if (*(int16_t *)(hit + 6) < 0)
                FUN_80012050(~(int)*(int16_t *)(hit + 6), 0x0a);
        }
        FUN_80020620(obj, 0);
        return -1;
    }

    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }

    return 0;
}

intptr_t LAB_80036ad8(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        int32_t *pos = (int32_t *)(self + 0x48);
        pos[0] = mips_addu_i32(pos[0], *(int32_t *)(self + 0x88));
        pos[1] = mips_addu_i32(pos[1], *(int32_t *)(self + 0x8c));
        pos[2] = mips_addu_i32(pos[2], *(int32_t *)(self + 0x90));
        *(int32_t *)(self + 0x24) = pos[0];
        *(int32_t *)(self + 0x28) = pos[1];
        *(int32_t *)(self + 0x2c) = pos[2];
        *(uint16_t *)(self + 0x94) = (uint16_t)(*(uint16_t *)(self + 0x94) - 1u);
        if ((int16_t)*(uint16_t *)(self + 0x94) == -1 ||
            FUN_80025400(pos[0], pos[2]) < pos[1])
        {
            uint32_t *fx = FUN_8003fd24(pos, 0x10);
            if (fx != NULL) {
                *(uint16_t *)((uint8_t *)fx + 0x44) = (uint16_t)FUN_80017160();
                FUN_8001d708(fx);
            }
            FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x39, pos);
            FUN_80020620(obj, 1);
        }
        return 0;
    }

    if (event == 3) {
        uint8_t *hit = arg ? (uint8_t *)(uintptr_t)*(uint32_t *)(uintptr_t)arg : NULL;
        uint32_t *fx;
        int32_t *pos = (int32_t *)(self + 0x48);

        if (hit != NULL && hit[4] == 3)
            return -1;

        fx = FUN_8003fd24(pos, 0x10);
        if (fx != NULL) {
            *fx |= 0x400u;
            *(uint16_t *)((uint8_t *)fx + 0x44) = (uint16_t)FUN_80017160();
            FUN_8001d708(fx);
        }
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x39, pos);

        if (hit != NULL && hit[4] == 2) {
            int32_t impulse[3];
            impulse[0] = *(int32_t *)(self + 0x88) << 4;
            impulse[1] = *(int32_t *)(self + 0x8c) << 4;
            impulse[2] = *(int32_t *)(self + 0x90) << 4;
            FUN_80017594((uint32_t *)hit, impulse, (const int32_t *)(self + 0x24));
            if (*(int16_t *)(hit + 6) < 0)
                FUN_80012050(~(int)*(int16_t *)(hit + 6), 0x0a);
            if ((FUN_80017160() & 7u) == 0) {
                uint32_t idx = ((uint32_t)FUN_80017160() * 3u) >> 15;
                uint8_t *part = (uint8_t *)(uintptr_t)*(uint32_t *)(hit + 0x110 + (idx * 4));
                if (part != NULL) {
                    uint32_t *m = (uint32_t *)FUN_8001d624((int)(uintptr_t)part);
                    FUN_8003fd24((const int32_t *)((uint8_t *)m + 0x14), 0x2d);
                    FUN_8002ca94((uint32_t *)hit, (int)idx);
                }
            }
        }

        FUN_80020620(obj, 0);
        return -1;
    }

    return 0;
}

intptr_t LAB_800363e0(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;
    uint8_t *owner;

    if (event == 1) {
        int rowBase = 0x10;
        for (int x = -0x10; x < 0x10; x++, rowBase += 0x20) {
            int xSq = mips_mult_lo_i32(x, x);
            for (int z = -0x10; z < 0x10; z++) {
                int distSq = mips_addu_i32(xSq, mips_mult_lo_i32(z, z));
                long d = SquareRoot0((long)mips_sll_i32(distSq, 6));
                *(uint8_t *)(self + 0x1088 + rowBase + z) = (uint8_t)d;
            }
        }
        return 0;
    }
    if (event != 0)
        return 0;

    owner = (uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x80);
    if (owner == NULL)
        return 0;

    if (*(int32_t *)(self + 0x84) < 0x3c)
        *(uint16_t *)(owner + 0xa6) =
            (uint16_t)(mips_subu_i32(0, *(uint16_t *)(owner + 0xac)) << 1);

    if (*(int32_t *)(self + 0x84) == 0) {
        uint8_t *pulse = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(self + 0x58),
                                                0x33, 0x98u, 8u);
        if (pulse != NULL) {
            uint8_t *child;
            *(uint8_t *)(pulse + 4) = 7;
            *(uint32_t *)pulse = 0x01800004u;
            *(uint16_t *)(pulse + 6) = *(uint16_t *)(self + 6);
            if (*(uint32_t *)(pulse + 0x5c) != 0)
                *(uint32_t *)(pulse + 0x54) =
                    *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(pulse + 0x5c) + 0x10);
            *(uint32_t *)(pulse + 0x48) = *(uint32_t *)(self + 0x48);
            *(uint32_t *)(pulse + 0x4c) = *(uint32_t *)(self + 0x4c);
            *(uint32_t *)(pulse + 0x50) = *(uint32_t *)(self + 0x50);
            Object_SetCallbackPsxSlot(pulse, (uintptr_t)&LAB_80031fa0);
            *(uint16_t *)(pulse + 0x0c) = 0x0f;
            child = (uint8_t *)(uintptr_t)*(uint32_t *)(pulse + 0x38);
            if (child != NULL)
                Object_SetCallbackPsxSlot(child, (uintptr_t)&LAB_8003e7b4);
            *(uint32_t *)(pulse + 0x80) = (uint32_t)(uintptr_t)owner;
            FUN_8002036c((uint32_t *)pulse);
            FUN_8004483c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x58) + 8),
                         1, (int *)(self + 0x48));
        }
    }

    if (arg != 0 && *(int32_t *)(self + 0x84) >= 0x3c) {
        int32_t x = *(int32_t *)(self + 0x48);
        int32_t z = *(int32_t *)(self + 0x50);
        int32_t counter = *(int32_t *)(self + 0x84);
        int32_t baseX;
        int32_t baseZ;
        int32_t half;
        int32_t phaseCenter;

        if (x < 0)
            x = mips_addu_i32(x, 0xffff);
        if (z < 0)
            z = mips_addu_i32(z, 0xffff);
        baseX = (x >> 16) - 0x10;
        baseZ = (z >> 16) - 0x10;
        half = mips_addu_i32(mips_subu_i32(counter, 0x3c),
                             (int32_t)((uint32_t)mips_subu_i32(counter, 0x3c) >> 31)) >> 1;
        phaseCenter = mips_addu_i32(half, 0x30);

        for (int dx = 0; dx < 0x20; dx++) {
            uint32_t cellX = (uint32_t)mips_addu_i32(baseX, dx);
            for (int dz = 0; dz < 0x20; dz++) {
                uint8_t *cell = self + (dx * 2) + (dz * 0x40);
                uint8_t dist = *(uint8_t *)(self + 0x1088 + dx + (dz * 0x20));
                int32_t ring = mips_subu_i32(0x80, dist);
                int32_t profile = 0;
                int32_t phase;
                int32_t absPhase;

                ring = mips_mult_lo_i32(ring, ring);
                phase = mips_sll_i32(mips_subu_i32(dist, phaseCenter), 7);
                absPhase = phase < 0 ? mips_subu_i32(0, phase) : phase;
                if (absPhase < 0x1000) {
                    uint32_t lut = ((uint32_t)phase & 0xfffu) * 2u;
                    int32_t wave = mips_mult_lo_i32(DAT_800607b4[lut], ring);
                    int32_t edge;
                    if (wave < 0)
                        wave = mips_addu_i32(wave, 0xfffff);
                    wave >>= 20;
                    edge = DAT_800607b4[lut + 1];
                    if (edge < 0)
                        edge = mips_addu_i32(edge, 0x1ff);
                    edge = mips_sll_i32(mips_subu_i32(0, edge >> 9), 11);
                    profile = wave | edge;
                }

                {
                    int16_t oldProfile = *(int16_t *)(cell + 0x888);
                    int16_t newProfile = (int16_t)profile;
                    int16_t delta = (int16_t)mips_subu_i32(newProfile, oldProfile);
                    *(int16_t *)(cell + 0x88) = newProfile;
                    if (delta != 0) {
                        uint32_t cellZ = (uint32_t)mips_addu_i32(baseZ, dz);
                        uintptr_t chunk = DAT_800911a0[(cellX >> 6) * 0x20u + (cellZ >> 6)];
                        uint16_t *height = (uint16_t *)(chunk +
                            ((cellX & 0x3fu) << 7) + ((cellZ & 0x3fu) << 1));
                        *height = (uint16_t)mips_addu_i32(*height, (uint16_t)delta);
                    }
                }
            }
        }

        {
            uint32_t *src = (uint32_t *)(self + 0x88);
            uint32_t *dst = (uint32_t *)(self + 0x888);
            while (src != (uint32_t *)(self + 0x888)) {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                src += 4;
                dst += 4;
            }
        }
    }

    *(int32_t *)(self + 0x84) = mips_addu_i32(*(int32_t *)(self + 0x84), 1);
    if (*(int32_t *)(self + 0x84) == 0x13d) {
        int32_t x = *(int32_t *)(self + 0x48);
        int32_t z = *(int32_t *)(self + 0x50);
        int32_t baseX;
        int32_t baseZ;

        if (x < 0)
            x = mips_addu_i32(x, 0xffff);
        if (z < 0)
            z = mips_addu_i32(z, 0xffff);
        baseX = (x >> 16) - 0x10;
        baseZ = (z >> 16) - 0x10;

        for (int dx = 0; dx < 0x20; dx++) {
            uint32_t cellX = (uint32_t)mips_addu_i32(baseX, dx);
            for (int dz = 0; dz < 0x20; dz++) {
                int16_t profile = *(int16_t *)(self + 0x888 + (dx * 2) + (dz * 0x40));
                if (profile != 0) {
                    uint32_t cellZ = (uint32_t)mips_addu_i32(baseZ, dz);
                    uintptr_t chunk = DAT_800911a0[(cellX >> 6) * 0x20u + (cellZ >> 6)];
                    uint16_t *height = (uint16_t *)(chunk +
                        ((cellX & 0x3fu) << 7) + ((cellZ & 0x3fu) << 1));
                    *height = (uint16_t)mips_subu_i32(*height, (uint16_t)profile);
                }
            }
        }
        FUN_800205f8((intptr_t)self);
        iRam00000758 = mips_subu_i32(iRam00000758, 1);
    }

    return 0;
}

intptr_t LAB_80038cf8(intptr_t obj, int event, intptr_t arg)
{
    (void)arg;
    if (event == 5) {
        *(uint32_t *)((uint8_t *)(uintptr_t)obj + 0x60) = 0;
        return -1;
    }
    return 0;
}

intptr_t LAB_80038d18(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event != 0)
        return 0;

    *(int32_t *)(self + 0x4c) =
        mips_addu_i32(*(int32_t *)(self + 0x4c), *(int32_t *)(self + 0x8c));
    *(uint16_t *)(self + 0x40) = (uint16_t)(*(uint16_t *)(self + 0x40) + 0x5bu);
    *(uint16_t *)(self + 0x42) = (uint16_t)(*(uint16_t *)(self + 0x42) + 0x44u);
    if (arg != 0)
        FUN_8001d708((uint32_t *)self);

    if (*(int32_t *)(self + 0x8c) < 0)
        *(int32_t *)(self + 0x8c) = mips_addu_i32(*(int32_t *)(self + 0x8c), 0x38);
    else
        *(int32_t *)(self + 0x8c) = 0;

    *(uint16_t *)(self + 0x94) = (uint16_t)(*(uint16_t *)(self + 0x94) + 1u);
    if ((*(uint16_t *)(self + 0x94) & 0x0fu) == 0) {
        uint32_t r0 = FUN_80017160();
        uint32_t idx = (uint32_t)((uint64_t)r0 * 3u >> 15) & 3u;
        uint8_t *child = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(self + 0x58),
                                                 DAT_8005eca8[idx], 0x98u, 8u);
        uint32_t r1 = FUN_80017160();
        uint32_t r2 = FUN_80017160();
        if (child != NULL) {
            int32_t radius = *(int32_t *)(self + 0x54);
            uint32_t a = r2 & 0xfffu;
            uint32_t b = r1 & 0xfffu;
            int32_t v;

            *(uint8_t *)(child + 4) = 7;
            *(uint16_t *)(child + 0x0a) = 0x30;
            *(uint8_t *)(child + 8) = (uint8_t)idx;
            *(uint16_t *)(child + 6) = *(uint16_t *)(self + 6);

            v = mips_mult_lo_i32(DAT_800607b4[a * 2 + 1], DAT_800607b4[b * 2]);
            if (v < 0)
                v = mips_addu_i32(v, 0xfff);
            v >>= 12;
            v = mips_mult_lo_i32(v, radius);
            if (v < 0)
                v = mips_addu_i32(v, 0xfff);
            *(int32_t *)(child + 0x48) = v >> 12;

            v = mips_mult_lo_i32(DAT_800607b4[a * 2], radius);
            if (v < 0)
                v = mips_addu_i32(v, 0xfff);
            *(int32_t *)(child + 0x4c) = v >> 12;

            v = mips_mult_lo_i32(DAT_800607b4[a * 2 + 1], DAT_800607b4[b * 2 + 1]);
            if (v < 0)
                v = mips_addu_i32(v, 0xfff);
            v >>= 12;
            v = mips_mult_lo_i32(v, radius);
            if (v < 0)
                v = mips_addu_i32(v, 0xfff);
            *(int32_t *)(child + 0x48) = v >> 12;

            *(uint32_t *)child = 0x01800410u;
            Object_SetCallbackPsxSlot(child, (uintptr_t)&FUN_8003eab0);
            *(uint16_t *)(child + 0x0c) = *(uint16_t *)(self + 0x0c);
            *(uint32_t *)(child + 0x84) = *(uint32_t *)(self + 0x84);
            *(uint32_t *)(child + 0x80) = *(uint32_t *)(self + 0x80);
            FUN_8001d708((uint32_t *)child);
            FUN_8001d4f0((uint32_t *)self, (uint32_t *)child);
            FUN_80020890((intptr_t)child, 0x78);
        }
    }

    if (*(int16_t *)(self + 0x94) == 0xf0) {
        uint8_t *slot = (uint8_t *)(uintptr_t)FUN_8001d5a0((intptr_t)self);
        if (slot != NULL) {
            if (*(uint16_t *)(slot + 0x0c) == 0) {
                FUN_8002cb7c((intptr_t)slot);
            } else {
                *(uint32_t *)slot &= 0xfff7ffffu;
            }
        }
        FUN_8003fc50((int)(uintptr_t)self);
        FUN_800204dc((int)FUN_8001d564((intptr_t)self));
        iRam00000758 = mips_subu_i32(iRam00000758, 1);
    }

    return 0;
}

intptr_t LAB_8003959c(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;

    if (event == 9) {
        if (*(uint32_t *)(self + 0x84) == (uint32_t)(uintptr_t)arg)
            *(uint32_t *)(self + 0x84) = *(uint32_t *)(self + 0x80);
        return 0;
    }
    if (event != 0)
        return 0;

    if (*(int16_t *)(self + 0x96) != 0) {
        *(uint16_t *)(self + 0x96) = (uint16_t)(*(uint16_t *)(self + 0x96) - 1u);
        return 0;
    }

    *(int32_t *)(self + 0x48) =
        mips_addu_i32(*(int32_t *)(self + 0x48), *(int32_t *)(self + 0x88));
    *(int32_t *)(self + 0x4c) =
        mips_addu_i32(*(int32_t *)(self + 0x4c), *(int32_t *)(self + 0x8c));
    *(int32_t *)(self + 0x50) =
        mips_addu_i32(*(int32_t *)(self + 0x50), *(int32_t *)(self + 0x90));
    *(int32_t *)(self + 0x24) = *(int32_t *)(self + 0x48);
    *(int32_t *)(self + 0x28) = *(int32_t *)(self + 0x4c);
    *(int32_t *)(self + 0x2c) = *(int32_t *)(self + 0x50);

    *(uint16_t *)(self + 0x94) = (uint16_t)(*(uint16_t *)(self + 0x94) + 1u);
    if ((*(uint16_t *)(self + 0x94) & 3u) == 0) {
        uint8_t *spark = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                 0x13, 0x98u, 8u);
        if (spark != NULL) {
            *(uint8_t *)(spark + 4) = 4;
            *(uint32_t *)spark = 0x4b4u;
            Object_SetCallbackPsxSlot(spark, (uintptr_t)&FUN_8003eab0);
            *(uint32_t *)(spark + 0x48) = *(uint32_t *)(self + 0x48);
            *(uint32_t *)(spark + 0x4c) = *(uint32_t *)(self + 0x4c);
            *(uint32_t *)(spark + 0x50) = *(uint32_t *)(self + 0x50);
            *(uint16_t *)(spark + 0x44) = (uint16_t)((int16_t)*(uint16_t *)(self + 0x94) * 0x60);
            *(int32_t *)(spark + 0x88) = mips_subu_i32(0, *(int16_t *)(self + 0x14));
            *(int32_t *)(spark + 0x8c) = mips_subu_i32(0, *(int16_t *)(self + 0x1a));
            *(int32_t *)(spark + 0x90) = mips_subu_i32(0, *(int16_t *)(self + 0x20));
            FUN_8002036c((uint32_t *)spark);
        }
    }

    if (*(int16_t *)(self + 0x94) >= 9) {
        int32_t vy = mips_addu_i32(*(int32_t *)(self + 0x8c), 0x38);
        *(int32_t *)(self + 0x8c) = (vy < 0) ? vy : 0;
    }

    if (*(int16_t *)(self + 0x94) >= 0x259) {
        for (int i = 0; i < 5; i++) {
            uint8_t *piece = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(self + 0x58),
                                                     0x39, 0x98u, 0u);
            if (piece != NULL) {
                *(uint8_t *)(piece + 4) = 7;
                *(uint32_t *)piece = 0x01800080u;
                *(uint32_t *)(piece + 0x48) = *(uint32_t *)(self + 0x48);
                *(uint32_t *)(piece + 0x4c) = *(uint32_t *)(self + 0x4c);
                *(uint32_t *)(piece + 0x50) = *(uint32_t *)(self + 0x50);
                Object_SetCallbackPsxSlot(piece, (uintptr_t)&FUN_8003eab0);
                *(uint16_t *)(piece + 0x0c) = *(uint16_t *)(self + 0x0c);
                *(int32_t *)(piece + 0x88) =
                    (((int32_t)FUN_80017160() * 0x989u) >> 15) - 0x4c4;
                *(int32_t *)(piece + 0x8c) =
                    (((int32_t)FUN_80017160() * 0x17d7u) >> 15) + 0xbeb;
                *(int32_t *)(piece + 0x90) =
                    (((int32_t)FUN_80017160() * 0x989u) >> 15) - 0x4c4;
                *(uint32_t *)(piece + 0x80) = *(uint32_t *)(self + 0x80);
                FUN_8002036c((uint32_t *)piece);
            }
        }
        FUN_8003fd24((const int32_t *)(self + 0x48), 0x0c);
        FUN_800205f8(obj);
        iRam00000758 = mips_subu_i32(iRam00000758, 1);
    }

    return 0;
}

intptr_t LAB_80037b94(intptr_t obj, int event, intptr_t arg)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;
    uint8_t *hit = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        *(int32_t *)(self + 0x48) =
            mips_addu_i32(*(int32_t *)(self + 0x48), *(int32_t *)(self + 0x88));
        *(int32_t *)(self + 0x4c) =
            mips_addu_i32(*(int32_t *)(self + 0x4c), *(int32_t *)(self + 0x8c));
        *(int32_t *)(self + 0x50) =
            mips_addu_i32(*(int32_t *)(self + 0x50), *(int32_t *)(self + 0x90));
        *(int32_t *)(self + 0x88) = mips_mult_lo_i32(*(int32_t *)(self + 0x88), 0xf80) >> 12;
        *(int32_t *)(self + 0x8c) = mips_mult_lo_i32(*(int32_t *)(self + 0x8c), 0xf80) >> 12;
        *(int32_t *)(self + 0x90) = mips_mult_lo_i32(*(int32_t *)(self + 0x90), 0xf80) >> 12;
        *(uint16_t *)(self + 0x44) =
            (uint16_t)(*(uint16_t *)(self + 0x44) + *(uint16_t *)(self + 0x96));
        if (arg != 0)
            FUN_8001d708((uint32_t *)self);
        return 0;
    }

    if (event == 3) {
        if ((*(uint32_t *)self & 0x10000u) == 0 && hit != NULL) {
            uint8_t *hitObj = (uint8_t *)(uintptr_t)*(uint32_t *)hit;
            if (hitObj != NULL && *(uint8_t *)(hitObj + 4) == 2) {
                *(uint32_t *)self |= 0x10000u;
                FUN_8004483c(FUN_8004410c(), uRam000005f8, 0x20, (int *)(self + 0x48));
                if (((int32_t)(FUN_80017160() * 5u) >> 15) == 0)
                    FUN_8002c4bc((uint32_t *)hitObj);
            }
        }
        return 0;
    }

    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }

    return 0;
}

static int Vehicle_FireBlobLauncher(uint32_t *obj, uint8_t *owner,
                                    uint16_t effectKind, uint16_t shotKind,
                                    uint16_t shotLife, uint16_t shotAux,
                                    int sfxId, int retHuman, int retAi)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                              effectKind, 0x80u, 8u);
    uint8_t *shot = NULL;

    if (effect != NULL) {
        shot = (uint8_t *)FUN_80031300((intptr_t)owner,
                                       (intptr_t)*(uint32_t *)(s + 0x38),
                                       shotKind, 0x98u, (intptr_t)effect);
    }
    if (shot != NULL) {
        *(uint32_t *)shot = 0x01800084u;
        *(uint16_t *)(shot + 0x0c) = shotLife;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_800359c0);
        *(uint16_t *)(shot + 0x96) = 2;
        *(uint16_t *)(shot + 0x94) = shotAux;
        *(int32_t *)(shot + 0x88) = *(int32_t *)(owner + 0x80) << 5 >> 12;
        *(int32_t *)(shot + 0x8c) = *(int32_t *)(owner + 0x84) << 5 >> 12;
        *(int32_t *)(shot + 0x90) = *(int32_t *)(owner + 0x88) << 5 >> 12;
        FUN_800202f4(shot);
    }

    if (effect != NULL) {
        *(uint32_t *)effect = 0x10u;
        Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
        if ((*(uint32_t *)owner & 4u) == 0)
            FUN_800207c4(effect);
    }

    if (shot != NULL)
        FUN_800447e8(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     sfxId, shot + 0x24);
    FUN_80020890((intptr_t)obj, 0x19);
    *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
    if (*(uint16_t *)(s + 0x0c) == 0)
        FUN_8002cb7c((intptr_t)obj);
    return (*(int16_t *)(owner + 6) >= 0) ? retHuman : retAi;
}

static int Vehicle_AimSubObjectAtOwner(uint32_t *obj, intptr_t arg,
                                       int minPitch, int maxPitch)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *target = (uint8_t *)(uintptr_t)arg;
    uint8_t *owner;
    uint8_t *pivot;
    uint32_t *mat;
    int32_t delta[3];
    int32_t pitch;

    if (target == NULL)
        return 0;
    if (!FUN_8003c538(obj, (intptr_t)target))
        return 0;
    owner = (uint8_t *)(uintptr_t)*(uint32_t *)(target + 0xe4);
    if (owner == NULL)
        return 0;
    pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
    if (pivot == NULL)
        return 0;

    mat = (uint32_t *)FUN_8001d624((int)(uintptr_t)obj);
    delta[0] = mips_subu_i32(*(int32_t *)(owner + 0x48), *(int32_t *)((uint8_t *)mat + 0x14));
    delta[1] = mips_subu_i32(*(int32_t *)(owner + 0x4c), *(int32_t *)((uint8_t *)mat + 0x18));
    delta[2] = mips_subu_i32(*(int32_t *)(owner + 0x50), *(int32_t *)((uint8_t *)mat + 0x1c));
    FUN_8004352c(mat, delta, delta);

    *(int16_t *)(pivot + 0x42) = (int16_t)FUN_8004ecd4(delta[0], delta[2]);
    pitch = -(int16_t)FUN_8004ecd4(delta[1], delta[2]);
    if (pitch < minPitch)
        pitch = minPitch;
    if (pitch > maxPitch)
        pitch = maxPitch;
    *(int16_t *)(pivot + 0x40) = (int16_t)pitch;
    FUN_8001d708((uint32_t *)pivot);
    return 0;
}

static int Vehicle_StateSubObjectCommon(uint32_t *obj, int event, intptr_t arg)
{
    (void)arg;

    switch (event) {
    case 1:
        *(uint16_t *)((uint8_t *)obj + 0x0c) = 5;
        *(uint8_t  *)((uint8_t *)obj + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    case 13:
        return 6;
    case 14:
        return 0x801f;
    default:
        return 0;
    }
}

static int Vehicle_StateTrackTarget12(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint32_t *target = (uint32_t *)(uintptr_t)arg;

    switch (event) {
    case 0: {
        if (target == 0)
            return 0;
        if (!FUN_8003c538(obj, (intptr_t)target))
            return 0;
        uint32_t target_owner = *(uint32_t *)((uint8_t *)target + 0xe4);
        if (target_owner == 0)
            return 0;

        uint32_t *pivot = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        if (pivot == 0)
            return 0;

        uint32_t *m = (uint32_t *)FUN_8001d624((int)(uintptr_t)obj);
        uint8_t *owner = (uint8_t *)(uintptr_t)target_owner;
        int32_t delta[3];
        delta[0] = mips_subu_i32(*(int32_t *)(owner + 0x48),
                                  *(int32_t *)((uint8_t *)m + 0x14));
        delta[1] = mips_subu_i32(*(int32_t *)(owner + 0x4c),
                                  *(int32_t *)((uint8_t *)m + 0x18));
        delta[2] = mips_subu_i32(*(int32_t *)(owner + 0x50),
                                  *(int32_t *)((uint8_t *)m + 0x1c));
        FUN_8004352c(m, delta, delta);

        *(int16_t *)((uint8_t *)pivot + 0x42) =
            (int16_t)((int16_t)FUN_8004ecd4(delta[0], delta[2]));
        *(int16_t *)((uint8_t *)pivot + 0x40) =
            (int16_t)(-(int16_t)FUN_8004ecd4(delta[1], delta[2]));
        FUN_8001d708(pivot);
        return 0;
    }
    case 1:
        *(uint16_t *)(s + 0x0c) = 12;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    case 2:
        if ((obj[0] & 0x20000u) == 0) {
            obj[0] |= 0x20000u;
            FUN_8004445c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                         0);
        }
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        return 0;
    case 10:
        obj[0] &= ~0x20000u;
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x2b,
                     (uint8_t *)FUN_8001d624((int)(uintptr_t)obj) + 0x14);
        return 0;
    case 12:
        if (target == 0 || iRam00000758 != 0)
            return 0;
        if (*(int32_t *)((uint8_t *)target + 0x8c) >= 0x11e1)
            return 0;
        if (*(uint32_t *)((uint8_t *)target + 0xe4) == 0)
            return 0;
        return FUN_80016aac((const int32_t *)((uint8_t *)target + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)((uint8_t *)target + 0xe4) + 0x48))
               <= 0x12bfff;
    case 13:
        return 6;
    case 14:
        return 0x801f;
    default:
        return 0;
    }
}

static int Vehicle_StateSubObject0(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint32_t *target = (uint32_t *)(uintptr_t)arg;

    if (event == 11) {
        uint32_t *owner = (uint32_t *)(uintptr_t)arg;
        intptr_t bone = FUN_8001b038(obj, 0x8000u);
        uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                  2, 0x80u, 8u);
        MATRIX localMat;
        MATRIX composed;
        MATRIX *selfMat;
        int outer;
        int baseX = -0x800;

        selfMat = (MATRIX *)FUN_8001d624((int)(uintptr_t)obj);
        FUN_8001b07c(&localMat, (int)(uintptr_t)bone);
        CompMatrixLV(selfMat, &localMat, &composed);

        if (effect != NULL) {
            *(uint32_t *)effect = 0x24u;
            memcpy(effect + 0x10, &composed, sizeof(MATRIX));
            Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
            FUN_800202f4(effect);
        }

        for (outer = 0; outer < 3; outer++, baseX += 0x800) {
            int inner;
            for (inner = 0; inner < 3; inner++) {
                SVECTOR srcVec;
                SVECTOR outVec;
                uint8_t *child;
                uint8_t *childBone;

                child = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                0x35, 0x98u, 8u);
                if (child == NULL)
                    continue;

                srcVec.vx = (int16_t)baseX;
                srcVec.vy = (int16_t)mips_subu_i32(0, inner << 11);
                srcVec.vz = 0x2000;
                srcVec.pad = 0;
                ApplyMatrixSV(&composed, &srcVec, &outVec);

                *(uint32_t *)child = 0x01800084u;
                childBone = (uint8_t *)(uintptr_t)*(uint32_t *)(child + 0x38);
                if (childBone != NULL)
                    *(uint32_t *)childBone = 0x410u;
                *(uint8_t *)(child + 4) = 7;
                *(uint16_t *)(child + 6) = *(uint16_t *)((uint8_t *)owner + 6);
                *(int32_t *)(child + 0x24) = composed.t[0];
                *(int32_t *)(child + 0x28) = composed.t[1];
                *(int32_t *)(child + 0x2c) = composed.t[2];
                *(uint16_t *)(child + 0x0c) = 0;
                Object_SetCallbackPsxSlot(child, (uintptr_t)&LAB_8003b1e0);
                *(int32_t *)(child + 0x88) =
                    mips_addu_i32(rtz_shift7(*(int32_t *)((uint8_t *)owner + 0x80)),
                                  (int16_t)outVec.vx);
                *(int32_t *)(child + 0x8c) =
                    mips_addu_i32(rtz_shift7(*(int32_t *)((uint8_t *)owner + 0x84)),
                                  (int16_t)outVec.vy);
                *(int32_t *)(child + 0x90) =
                    mips_addu_i32(rtz_shift7(*(int32_t *)((uint8_t *)owner + 0x88)),
                                  (int16_t)outVec.vz);
                *(uint32_t *)(child + 0x80) = (uint32_t)(uintptr_t)owner;
                FUN_800202f4(child);
                iRam00000758 = mips_addu_i32(iRam00000758, 1);
            }
        }

        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x2f, &composed.t[0]);
        {
            uint32_t bank = 0;
            uint8_t *objBank = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58);
            if (objBank != NULL)
                bank = *(uint32_t *)(objBank + 8);
            FUN_8004445c(FUN_8004410c(), bank, 0);
        }

        {
            int32_t impulse[3];
            impulse[0] = mips_subu_i32(0, (int16_t)localMat.m[0][2]) << 3;
            impulse[1] = mips_subu_i32(0, (int16_t)localMat.m[2][0]) << 3;
            impulse[2] = mips_subu_i32(0, (int16_t)localMat.pad) << 3;
            FUN_80017594(owner, impulse, &localMat.t[0]);
        }

        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);

        if (weapon_trace_enabled()) {
            fprintf(stderr,
                    "v8: state0 burst weapon=%p owner=%p ammo=%u particles=%d\n",
                    (void *)obj, (void *)(uintptr_t)arg,
                    (unsigned)*(uint16_t *)(s + 0x0c), (int)iRam00000758);
        }
        return (*(int16_t *)((uint8_t *)owner + 6) >= 0) ? 0xb4 : 0x5a;
    }

    if (event == 0) {
        if (target == 0)
            return 0;
        if (!FUN_8003c538(obj, (intptr_t)target))
            return 0;
        uint32_t target_owner = *(uint32_t *)((uint8_t *)target + 0xe4);
        if (target_owner == 0)
            return 0;

        uint32_t *m = (uint32_t *)FUN_8001d624((int)(uintptr_t)obj);
        uint8_t *owner = (uint8_t *)(uintptr_t)target_owner;
        int32_t delta[3];
        delta[0] = mips_subu_i32(*(int32_t *)(owner + 0x48),
                                  *(int32_t *)((uint8_t *)m + 0x14));
        delta[1] = mips_subu_i32(*(int32_t *)(owner + 0x4c),
                                  *(int32_t *)((uint8_t *)m + 0x18));
        delta[2] = mips_subu_i32(*(int32_t *)(owner + 0x50),
                                  *(int32_t *)((uint8_t *)m + 0x1c));
        FUN_8004352c(m, delta, delta);

        int32_t yaw = mips_signext12_i32(mips_addu_i32(*(uint16_t *)(s + 0x42),
                                                       FUN_8004ecd4(delta[0], delta[2])));
        if (yaw < -0x200)
            yaw = -0x200;
        else if (yaw > 0x200)
            yaw = 0x200;

        int32_t pitch = mips_signext12_i32(mips_subu_i32(*(uint16_t *)(s + 0x40),
                                                         FUN_8004ecd4(delta[1], delta[2])));
        if (pitch < -0xe3)
            pitch = -0xe3;
        else if (pitch > 0xe3)
            pitch = 0xe3;

        *(int16_t *)(s + 0x42) = (int16_t)yaw;
        *(int16_t *)(s + 0x40) = (int16_t)pitch;
        FUN_8001d708(obj);
        return 0;
    }

    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject1(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint32_t *target = (uint32_t *)(uintptr_t)arg;

    switch (event) {
    case 0:
        if (target != 0)
            (void)FUN_8003c538(obj, (intptr_t)target);
        return 0;
    case 1:
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        FUN_8001fcb4((int)(uintptr_t)obj, *(uint16_t *)(s + 0x46));
        if (*(uint32_t *)(s + 0x38) != 0) {
            uint8_t *child = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
            if (*(uint32_t *)(child + 0x38) != 0)
                *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(child + 0x38) + 0x60) = 0;
        }
        return 0;
    case 12:
        if (target != 0 && *(uint32_t *)((uint8_t *)target + 0xe4) != 0) {
            uint8_t *owner = (uint8_t *)(uintptr_t)*(uint32_t *)((uint8_t *)target + 0xe4);
            int32_t dist = FUN_80016aac((const int32_t *)((uint8_t *)target + 0x48),
                                        (const int32_t *)(owner + 0x48));
            return (dist > 0x64000 && dist <= 0x1f3fff) ? 1 : 0;
        }
        return 0;
    case 11: {
        uint8_t *owner = (uint8_t *)(uintptr_t)arg;
        uint8_t *shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                0x3f, 0x98u, 8u);
        if (shot == NULL)
            return 0;

        if (*(uint32_t *)(s + 0x38) != 0) {
            uint8_t *joint = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
            if (*(uint32_t *)(joint + 0x38) != 0)
                FUN_8001ac08((uint32_t *)(uintptr_t)*(uint32_t *)(joint + 0x38));
        }

        *(uint32_t *)shot = 0x10u;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003846c);
        *(int32_t *)(shot + 0x48) = 0;
        *(int32_t *)(shot + 0x4c) = 0;
        *(int32_t *)(shot + 0x50) = 0;
        *(uint16_t *)(shot + 0x0c) = 0x12c;
        if (*(uint32_t *)(owner + 0xe4) != 0)
            *(uint32_t *)(shot + 0x84) = *(uint32_t *)(owner + 0xe4);
        else
            *(uint32_t *)(shot + 0x84) = (uint32_t)(uintptr_t)owner;
        *(uint16_t *)(shot + 0x94) = 0;
        *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
        if (*(uint32_t *)(s + 0x38) != 0)
            FUN_8001d544((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38),
                         (uint32_t *)shot);

        FUN_800447e8(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     0, owner + 0x24);
        FUN_8004445c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     1);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        iRam00000758 = mips_addu_i32(iRam00000758, 1);
        return (*(int16_t *)(owner + 6) >= 0) ? 0xf0 : 0x78;
    }
    case 13:
        return 6;
    case 14:
        return 0x801f;
    default:
        return 0;
    }

    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject2(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0)
        return Vehicle_AimSubObjectAtOwner(obj, arg, -0x80, 0x100);
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 0x32;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 10) {
        obj[0] &= ~0x20000u;
        *(uint32_t *)(s + 0x40) = 0;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                  0x0d, 0x80u, 8u);
        uint8_t *shot = (uint8_t *)FUN_80031300((intptr_t)owner, (intptr_t)pivot,
                                                0x37, 0x98u, (intptr_t)effect);
        uint8_t *drop = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                0x36, 0x94u, 0u);
        uint32_t shotFlags;

        if (shot == NULL || drop == NULL)
            return 0;

        shotFlags = (*(int16_t *)(s + 0x40) != 0) ? 0x280u : 0x01800280u;
        *(uint16_t *)(shot + 0x0c) = 0x14;
        *(uint32_t *)shot = shotFlags;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_80036ad8);
        FUN_800202f4(shot);
        *(int32_t *)(shot + 0x88) =
            rtz_shift7(*(int32_t *)(owner + 0x80)) + ((int32_t)*(int16_t *)(shot + 0x14) * 6);
        *(int32_t *)(shot + 0x8c) =
            rtz_shift7(*(int32_t *)(owner + 0x84)) + ((int32_t)*(int16_t *)(shot + 0x1a) * 6);
        *(int32_t *)(shot + 0x90) =
            rtz_shift7(*(int32_t *)(owner + 0x88)) + ((int32_t)*(int16_t *)(shot + 0x20) * 6);
        *(uint16_t *)(shot + 0x94) = 0xb4;

        if (effect != NULL) {
            Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
            if ((*(uint32_t *)owner & 4u) == 0)
                FUN_800207c4(effect);
        }

        if (pivot != NULL) {
            intptr_t bone = FUN_8001b038((uint32_t *)pivot, 0x8001u);
            FUN_8001d68c((MATRIX *)(drop + 0x10), (intptr_t)pivot, bone);
            *(uint8_t *)(drop + 4) = 4;
            *(uint32_t *)drop = 0xa0u;
            Object_SetCallbackPsxSlot(drop, (uintptr_t)&FUN_8003eab0);
            *(int32_t *)(drop + 0x88) = (int32_t)*(int16_t *)(drop + 0x14) >> 2;
            *(int32_t *)(drop + 0x8c) = (int32_t)*(int16_t *)(drop + 0x1a) >> 2;
            *(int32_t *)(drop + 0x90) = (int32_t)*(int16_t *)(drop + 0x20) >> 2;
            *(uint8_t *)(drop + 0x87) = 2;
            FUN_800202f4(drop);
        }

        {
            int32_t recoil[3];
            if (pivot != NULL)
                FUN_80043358((uint32_t *)(pivot + 0x10), DAT_800657b0, recoil);
            else {
                recoil[0] = DAT_800657b0[0];
                recoil[1] = DAT_800657b0[1];
                recoil[2] = DAT_800657b0[2];
            }
            FUN_80017594((uint32_t *)owner, recoil, (const int32_t *)(s + 0x48));
        }

        if ((obj[0] & 0x20000u) == 0) {
            obj[0] |= 0x20000u;
            FUN_8004445c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                         1);
        }
        FUN_8004483c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     0, (int *)(shot + 0x48));

        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);

        if (*(int16_t *)(owner + 6) < 0) {
            obj[0] |= 0x40000u;
            return 0x0c;
        }
        if ((int16_t)*(uint16_t *)(s + 0x40) < 8)
            *(uint16_t *)(s + 0x40) = (uint16_t)(*(uint16_t *)(s + 0x40) + 1u);
        else
            obj[0] &= ~0x40000u;
        return 0xb4;
    }
    if (event == 12 && owner != NULL) {
        if ((int16_t)*(uint16_t *)(s + 0x40) >= 8)
            return 0;
        if (*(int16_t *)(s + 0x40) == 0) {
            int32_t dist;
            if (iRam00000758 != 0 || *(uint32_t *)(owner + 0xe4) == 0)
                return 0;
            dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                                (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
            if (dist > 0x31fff)
                return 0;
            if (*(int32_t *)(owner + 0x8c) >= 0xbeb)
                return 0;
        }
        return 1;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject3(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0)
            (void)FUN_8003c538(obj, arg);
        return 0;
    }
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        if ((*(uint32_t *)owner & 0x200000u) != 0) {
            uint8_t *effect = (uint8_t *)FUN_8001d470(0x1488u);
            if (effect != NULL) {
                *(uint32_t *)(effect + 0x80) = (uint32_t)(uintptr_t)owner;
                *(uint16_t *)(effect + 6) = *(uint16_t *)(owner + 6);
                Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_800363e0);
                *(uint32_t *)(effect + 0x58) = *(uint32_t *)(s + 0x58);
                *(uint32_t *)(effect + 0x48) = *(uint32_t *)(owner + 0x48);
                *(uint32_t *)(effect + 0x4c) = *(uint32_t *)(owner + 0x4c);
                *(uint32_t *)(effect + 0x50) = *(uint32_t *)(owner + 0x50);
                *(uint32_t *)effect = 0xa0u;
                FUN_8002036c((uint32_t *)effect);
                FUN_8004445c(FUN_8004410c(),
                             *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                             0);
                iRam00000758 = mips_addu_i32(iRam00000758, 1);
            }
            *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
            if (*(uint16_t *)(s + 0x0c) == 0)
                FUN_8002cb7c((intptr_t)obj);
            return 0xb4;
        }
        FUN_8004445c(FUN_8004410c(), uRam000005f8, 0x15);
        return 0xb4;
    }
    if (event == 10)
        return 0;
    if (event == 12 && owner != NULL) {
        int32_t dist;
        if (iRam00000758 != 0)
            return 0;
        if (*(int32_t *)(owner + 0x8c) >= 0x5f5)
            return 0;
        if ((*(uint32_t *)owner & 0x200000u) == 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist <= 0xc7fff) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject4(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;

    if (event == 0) {
        if (arg != 0 && FUN_8003c538(obj, arg)) {
            uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
            if (pivot != NULL) {
                *(uint16_t *)(pivot + 0x42) =
                    (uint16_t)(*(uint16_t *)(pivot + 0x42) + 0x22u);
                FUN_8001d708((uint32_t *)pivot);
            }
        }
        return 0;
    }
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        return 0;
    }
    if (event == 11) {
        uint8_t *owner = (uint8_t *)(uintptr_t)arg;
        uint8_t *target;
        intptr_t bone;
        MATRIX muzzle;
        int childKind;
        int i;
        int timer;

        if (owner == NULL)
            return 0;
        target = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4);
        if (target == NULL)
            target = owner;

        bone = FUN_8001b038((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38), 0x8000u);
        childKind = (*(int16_t *)(target + 6) < 0) ? 7 : 4;
        FUN_8001d68c(&muzzle, (intptr_t)(uintptr_t)*(uint32_t *)(s + 0x38), bone);

        for (i = 0, timer = 1; i < 4; i++, timer += 8) {
            uint8_t *burst = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                     0x3d, 0x98u, 8u);
            if (burst == NULL)
                continue;
            burst[4] = 4;
            *(uint16_t *)(burst + 6) = (uint16_t)i;
            *(uint32_t *)burst = 0xa2u;
            memcpy(burst + 0x10, &muzzle, 0x20);
            Object_SetCallbackPsxSlot(burst, (uintptr_t)&LAB_8003bde0);
            if (*(uint32_t *)(burst + 0x38) != 0)
                Object_SetCallbackPsxSlot((void *)(uintptr_t)*(uint32_t *)(burst + 0x38),
                                          (uintptr_t)&LAB_8003e7b4);
            *(uint16_t *)(burst + 0x96) = (uint16_t)timer;
            *(int32_t *)(burst + 0x88) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x80)),
                                                       *(int16_t *)(burst + 0x14));
            *(int32_t *)(burst + 0x8c) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x84)),
                                                       *(int16_t *)(burst + 0x1a));
            *(int32_t *)(burst + 0x90) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x88)),
                                                       *(int16_t *)(burst + 0x20));
            FUN_8001dc1c((intptr_t)burst);
            FUN_800202f4(burst);
        }

        for (i = 0, timer = 0x168; i < 3; i++, timer += 0x3c) {
            uint8_t *shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                    0x3c, 0x98u, 8u);
            int angle = ((i * 0x1000) / 3) & 0xfff;
            if (shot == NULL)
                continue;
            *(uint32_t *)shot = 0x01800080u;
            shot[4] = 7;
            *(int32_t *)(shot + 0x48) = *(int32_t *)(target + 0x48);
            *(int32_t *)(shot + 0x4c) = mips_addu_i32(*(int32_t *)(target + 0x4c), -0xc0000);
            *(int32_t *)(shot + 0x50) = *(int32_t *)(target + 0x50);
            Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003b8d4);
            *(uint16_t *)(shot + 0x0c) = (uint16_t)childKind;
            *(int32_t *)(shot + 0x88) = DAT_800607b4[angle * 2 + 1];
            *(int32_t *)(shot + 0x8c) = 0;
            *(int32_t *)(shot + 0x90) = DAT_800607b4[angle * 2 + 0];
            iRam00000758 = mips_addu_i32(iRam00000758, 1);
            *(uint16_t *)(shot + 0x96) = (uint16_t)timer;
            *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
            *(uint32_t *)(shot + 0x84) = (uint32_t)(uintptr_t)target;
            FUN_8002036c((uint32_t *)shot);
        }

        FUN_8004445c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     2);
        FUN_800447e8(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     1, (uint8_t *)&muzzle.t[0]);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        return 0x2d0;
    }
    if (event == 12 && arg != 0) {
        uint8_t *owner = (uint8_t *)(uintptr_t)arg;
        int32_t dist;
        if (iRam00000758 != 0 || (obj[0] & 1u) != 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist > 0x64000) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject5(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0)
            (void)FUN_8003c538(obj, arg);
        return 0;
    }
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        uint8_t *source = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        uint8_t *shot;
        uint8_t *target;
        uint8_t *m;
        int want = 3 - (int)*(uint16_t *)(s + 0x0c);
        if (want < 0)
            want = 0;
        while (source != NULL && *(int16_t *)(source + 6) != want)
            source = (uint8_t *)(uintptr_t)*(uint32_t *)(source + 0x34);
        if (source == NULL)
            return 0;

        shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(source + 0x58),
                                       *(uint16_t *)(source + 0x0a),
                                       0xa8u, 0u);
        if (shot == NULL)
            return 0;
        *(uint16_t *)(shot + 6) = *(uint16_t *)(owner + 6);
        m = (uint8_t *)FUN_8001d624((int)(uintptr_t)source);
        memcpy(shot + 0x10, m, 0x20);
        FUN_80016cec((int16_t *)(shot + 0x10), (int16_t *)(shot + 0x40));
        *(uint32_t *)shot = 0x01800080u;
        *(uint8_t *)(shot + 4) = 7;
        *(uint16_t *)(shot + 0x0c) = 0;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003a9dc);
        *(int32_t *)(shot + 0x48) = *(int32_t *)(shot + 0x24);
        *(int32_t *)(shot + 0x4c) = *(int32_t *)(shot + 0x28);
        *(int32_t *)(shot + 0x50) = *(int32_t *)(shot + 0x2c);
        *(int32_t *)(shot + 0x8c) =
            state5_projectile_velocity(*(int32_t *)(owner + 0x80),
                                       *(int16_t *)(shot + 0x12),
                                       *(int16_t *)(shot + 0x10));
        *(int32_t *)(shot + 0x90) =
            state5_projectile_velocity(*(int32_t *)(owner + 0x84),
                                       *(int16_t *)(shot + 0x18),
                                       *(int16_t *)(shot + 0x16));
        *(int32_t *)(shot + 0x94) =
            state5_projectile_velocity(*(int32_t *)(owner + 0x88),
                                       *(int16_t *)(shot + 0x1e),
                                       *(int16_t *)(shot + 0x1c));
        *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
        target = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4);
        if (target == NULL)
            target = owner;
        *(uint32_t *)(shot + 0x84) = (uint32_t)(uintptr_t)target;
        *(int32_t *)(shot + 0x88) = *(int16_t *)(target + 6);
        FUN_8002036c((uint32_t *)shot);
        FUN_80020890((intptr_t)shot, 0x708);
        FUN_80017594((uint32_t *)owner, DAT_800657cc, (const int32_t *)(source + 0x48));
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x14, shot + 0x48);
        FUN_8004445c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     1);
        if (*(uint16_t *)(s + 0x0c) < 4) {
            intptr_t slot = FUN_8001d564((intptr_t)source);
            FUN_8001af48((int)slot);
        }
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        return (*(int16_t *)(owner + 6) >= 0) ? 0xb4 : 0xf0;
    }
    if (event == 12 && owner != NULL) {
        int32_t dist;
        if (iRam00000758 != 0)
            return 0;
        if (*(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist > 0xfa000 && dist <= 0x3e7fff) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    if (event == 15 && owner != NULL) {
        uint8_t *ownerPivot = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0x38);
        intptr_t slot;
        int count;
        if (ownerPivot == NULL)
            return 0;
        slot = FUN_8001d564((intptr_t)ownerPivot);
        if (slot == 0)
            return 0;
        count = 2 - (int)*(uint16_t *)(s + 0x0c);
        if (count < 0)
            count = 0;
        *(uint16_t *)((uint8_t *)(uintptr_t)slot + 0x0c) = (uint16_t)count;
        FUN_8001d544(obj, (uint32_t *)(uintptr_t)slot);
        return 0;
    }
    return 0;
}
static int Vehicle_StateSubObject6(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0 && FUN_8003c538(obj, arg)) {
            uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
            uint8_t phase = *(uint8_t *)(s + 9);
            if (pivot != NULL) {
                if (phase < 0x1e) {
                    *(uint16_t *)(pivot + 0x40) =
                        (uint16_t)(*(uint16_t *)(pivot + 0x40) - 0x11u);
                } else if (phase >= 0x3d) {
                    *(uint16_t *)(pivot + 0x40) =
                        (uint16_t)(*(uint16_t *)(pivot + 0x40) + 0x11u);
                }
                FUN_8001d708((uint32_t *)pivot);
            }
            *(uint8_t *)(s + 9) = (uint8_t)(phase + 1u);
            if (*(uint8_t *)(s + 9) == 0x5b)
                FUN_80020778(obj);
        }
        return 0;
    }
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 0x28;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        if ((obj[0] & 0x80u) != 0) {
            FUN_8004445c(FUN_8004410c(), uRam000005f8, 0x15);
        } else {
            uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
            uint8_t *source = NULL;
            uint8_t *shot = NULL;

            if (pivot != NULL)
                source = (uint8_t *)(uintptr_t)*(uint32_t *)(pivot + 0x38);
            if (source != NULL) {
                shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(source + 0x58),
                                               *(uint16_t *)(source + 0x0a),
                                               0x98u, 0u);
            }
            if (shot != NULL) {
                memcpy(shot + 0x10, source + 0x10, 0x20);
                *(uint8_t *)(shot + 4) = 7;
                *(uint16_t *)(shot + 6) = 3;
                *(uint16_t *)(shot + 0x0c) = 0x64;
                Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003959c);
                *(uint16_t *)(shot + 0x96) = 0x1e;
                *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
                *(uint32_t *)(shot + 0x84) = (uint32_t)(uintptr_t)owner;
                *(int32_t *)(shot + 0x88) = rtz_shift7(*(int32_t *)(owner + 0x80));
                *(int32_t *)(shot + 0x8c) = rtz_shift7(*(int32_t *)(owner + 0x84));
                *(int32_t *)(shot + 0x90) = rtz_shift7(*(int32_t *)(owner + 0x88));
                FUN_80020744(shot);
                if (*(uint16_t *)(s + 0x0c) < 4) {
                    intptr_t slot = FUN_8001d564((intptr_t)source);
                    FUN_8001af48((int)slot);
                }
                if (pivot != NULL)
                    FUN_8001d4f0((uint32_t *)pivot, (uint32_t *)shot);
                FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x2e, owner + 0x24);
                FUN_8004445c(FUN_8004410c(),
                             *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                             0);
                iRam00000758 = mips_addu_i32(iRam00000758, 1);
                *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
                FUN_80020744(obj);
                *(uint8_t *)(s + 9) = 0;
            }
        }
        return (*(int16_t *)(owner + 6) < 0) ? 0x12c : 0x168;
    }
    if (event == 12 && owner != NULL) {
        int32_t dist;
        if (iRam00000758 != 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist > 0x96000 && dist <= 0x3e7fff) ? 1 : 0;
    }
    if (event == 15 && owner != NULL) {
        uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        if (pivot != NULL && *(uint32_t *)(pivot + 0x38) != 0 &&
            *(uint32_t *)(owner + 0x38) != 0) {
            uint8_t *child = (uint8_t *)(uintptr_t)*(uint32_t *)(pivot + 0x38);
            int want = (int)*(int16_t *)(child + 6) - 1;
            uint8_t *scan = (uint8_t *)(uintptr_t)*(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0x38) + 0x38);
            while (scan != NULL && *(int16_t *)(scan + 6) != want)
                scan = (uint8_t *)(uintptr_t)*(uint32_t *)(scan + 0x34);
            if (scan != NULL)
                FUN_8001d544((uint32_t *)pivot, (uint32_t *)FUN_8001d564((intptr_t)scan));
        }
        return 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return 0;
}
static int Vehicle_StateSubObject7(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    switch (event) {
    case 0: {
        uint8_t *targetOwner;
        uint8_t *pivot;
        uint32_t *m;
        int32_t delta[3];
        int32_t pitch;

        if (owner == NULL)
            return 0;
        if (!FUN_8003c538(obj, arg))
            return 0;
        targetOwner = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4);
        if (targetOwner == NULL)
            return 0;
        pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        if (pivot == NULL)
            return 0;

        m = (uint32_t *)FUN_8001d624((int)(uintptr_t)obj);
        delta[0] = mips_subu_i32(*(int32_t *)(targetOwner + 0x48),
                                  *(int32_t *)((uint8_t *)m + 0x14));
        delta[1] = mips_subu_i32(*(int32_t *)(targetOwner + 0x4c),
                                  *(int32_t *)((uint8_t *)m + 0x18));
        delta[2] = mips_subu_i32(*(int32_t *)(targetOwner + 0x50),
                                  *(int32_t *)((uint8_t *)m + 0x1c));
        FUN_8004352c(m, delta, delta);

        *(int16_t *)(pivot + 0x42) =
            (int16_t)mips_signext12_i32(FUN_8004ecd4(delta[0], delta[2]));
        pitch = mips_signext12_i32(-FUN_8004ecd4(delta[1], delta[2]));
        if (pitch < -0x80)
            pitch = -0x80;
        else if (pitch > 0x100)
            pitch = 0x100;
        *(int16_t *)(pivot + 0x40) = (int16_t)pitch;
        FUN_8001d708((uint32_t *)pivot);
        return 0;
    }
    case 1:
        *(uint16_t *)(s + 0x0c) = 12;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    case 2:
        if ((obj[0] & 0x20000u) == 0) {
            obj[0] |= 0x20000u;
            FUN_8004445c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                         1);
        }
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        return 0;
    case 10:
        obj[0] &= ~0x20000u;
        FUN_8004483c(FUN_8004410c(), uRam000005f8, 0x2b,
                     (int *)((uint8_t *)FUN_8001d624((int)(uintptr_t)obj) + 0x14));
        return 0;
    case 11: {
        uint8_t *effect;
        uint8_t *shot;

        if (owner == NULL)
            return 0;
        effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                         0x3c, 0x80u, 8u);
        shot = NULL;
        if (effect != NULL) {
            shot = (uint8_t *)FUN_80031300((intptr_t)owner,
                                           (intptr_t)*(uint32_t *)(s + 0x38),
                                           0x3a, 0x98u, (intptr_t)effect);
        }
        if (shot == NULL)
            return 0;

        *(uint16_t *)(shot + 0x0c) = 0x32;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_800359c0);
        *(uint16_t *)(shot + 0x96) = 2;
        *(uint16_t *)(shot + 0x94) = 8;
        *(uint32_t *)shot = 0x01800084u;
        *(int32_t *)(shot + 0x88) = mips_sll_i32(*(int32_t *)(owner + 0x80), 5) >> 12;
        *(int32_t *)(shot + 0x8c) = mips_sll_i32(*(int32_t *)(owner + 0x84), 5) >> 12;
        *(int32_t *)(shot + 0x90) = mips_sll_i32(*(int32_t *)(owner + 0x88), 5) >> 12;
        FUN_800202f4(shot);

        *(uint32_t *)effect = 0x10u;
        Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
        if ((*(uint32_t *)owner & 4u) == 0)
            FUN_800207c4(effect);

        FUN_8004483c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     0, (int *)(shot + 0x24));
        FUN_80020890((intptr_t)obj, 0x19);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        return (*(int16_t *)(owner + 6) >= 0) ? 0xb4 : 0x1e;
    }
    case 12:
        if (owner == NULL || iRam00000758 != 0)
            return 0;
        if (*(int32_t *)(owner + 0x8c) >= 0x11e1)
            return 0;
        if (*(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        return FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48))
               <= 0x12bfff;
    case 13:
        return 6;
    case 14:
        return 0x801f;
    default:
        return 0;
    }
}
static int Vehicle_StateSubObject8(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0)
            (void)FUN_8003c538(obj, arg);
        return 0;
    }
    if (event == 1) {
        if (*(uint32_t *)(s + 0x38) != 0)
            Object_SetCallbackPsxSlot((void *)(uintptr_t)*(uint32_t *)(s + 0x38),
                                      (uintptr_t)&LAB_8003e7b4);
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        intptr_t bone = FUN_8001b038(obj, 0x8000u);
        uint8_t *shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)*(uint32_t *)(s + 0x58),
                                                0x31, 0x98u, 8u);
        uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                  2, 0x80u, 8u);
        uint8_t *shotBone;

        if (shot == NULL || effect == NULL)
            return 0;

        *(uint16_t *)(effect + 6) = *(uint16_t *)(owner + 6);
        *(uint32_t *)(effect + 0x48) = *(uint32_t *)(shot + 4);
        *(uint32_t *)(effect + 0x4c) = *(uint32_t *)(shot + 8);
        *(uint32_t *)(effect + 0x50) = *(uint32_t *)(shot + 0x0c);
        *(uint32_t *)effect = 0x80u;
        Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_80038d18);
        *(uint16_t *)(effect + 0x0c) = 0x32;
        *(int32_t *)(effect + 0x8c) = mips_addu_i32(*(int32_t *)(effect + 0x8c), -0x800);
        *(uint32_t *)(effect + 0x80) = (uint32_t)(uintptr_t)owner;
        *(uint32_t *)(effect + 0x84) = *(uint32_t *)(owner + 0xe4);

        shotBone = (uint8_t *)(uintptr_t)*(uint32_t *)(effect + 0x38);
        while (shotBone != NULL) {
            uint32_t flags = *(uint32_t *)shotBone;
            Object_SetCallbackPsxSlot(shotBone, (uintptr_t)&LAB_80038cf8);
            if ((flags & 0x10u) != 0)
                *(uint32_t *)shotBone = flags | 0x400u;
            shotBone = (uint8_t *)(uintptr_t)*(uint32_t *)(shotBone + 0x34);
        }

        FUN_8001dc1c((intptr_t)effect);
        FUN_80020744(effect);
        FUN_8001d4f0(obj, (uint32_t *)effect);

        *(uint32_t *)shot |= 0x10u;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003e80c);
        FUN_8001b2fc((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38),
                     (const void *)(uintptr_t)bone, (uint32_t *)shot);
        if (*(uint32_t *)(s + 0x38) != 0)
            FUN_8001ac08((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38));

        if ((*(uint32_t *)owner & 4u) == 0)
            FUN_800207c4(obj);

        FUN_8004445c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     1);
        iRam00000758 = mips_addu_i32(iRam00000758, 1);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        obj[0] |= 0x800000u;
        return 0x168;
    }
    if (event == 12 && owner != NULL) {
        int32_t dist;
        if (iRam00000758 != 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist <= 0x12bfff && dist > 0x32000) ? 1 : 0;
    }
    if (event == 5) {
        if (*(uint32_t *)(s + 0x38) != 0)
            *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38) + 0x60) = 0;
        FUN_800207f8(obj);
        return -1;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject9(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0)
            (void)FUN_8003c538(obj, arg);
        return 0;
    }
    if (event == 1) {
        if (*(uint32_t *)(s + 0x38) != 0)
            Object_SetCallbackPsxSlot((void *)(uintptr_t)*(uint32_t *)(s + 0x38),
                                      (uintptr_t)&LAB_8003e7b4);
        *(uint16_t *)(s + 0x0c) = 3;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                  2, 0x80u, 8u);
        uint8_t *target = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4);
        uint8_t *shot = (uint8_t *)FUN_80031300((intptr_t)owner, (intptr_t)obj,
                                                (uint16_t)-1, 0x98u, (intptr_t)effect);
        if (shot == NULL || effect == NULL)
            return 0;

        *(uint32_t *)shot = 0x01800084u;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&FUN_8003733c);
        *(uint16_t *)(shot + 0x0c) = 5;
        *(uint32_t *)(shot + 0x58) = *(uint32_t *)(s + 0x58);
        *(int32_t *)(shot + 0x88) = mips_sll_i32(*(int32_t *)(owner + 0x80), 5) >> 12;
        *(int32_t *)(shot + 0x8c) = mips_sll_i32(*(int32_t *)(owner + 0x84), 5) >> 12;
        *(int32_t *)(shot + 0x90) = mips_sll_i32(*(int32_t *)(owner + 0x88), 5) >> 12;
        *(int32_t *)(shot + 0x8c) = mips_addu_i32(*(int32_t *)(shot + 0x8c), -0x80);
        *(uint32_t *)(shot + 0x84) = (uint32_t)(uintptr_t)(target != NULL ? target : owner);
        *(uint16_t *)(shot + 0x96) = 0x258;
        FUN_800202f4(shot);
        FUN_8003733c((uint32_t *)shot, 1, NULL);

        *(uint32_t *)effect |= 0x10u;
        Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
        FUN_8003fea8((int32_t *)(shot + 0x48), 0x08008080u);
        FUN_8004445c(FUN_8004410c(),
                     *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                     1);
        FUN_8004483c(FUN_8004410c(), uRam000005f8, 0x14, (int *)(shot + 0x48));
        if (*(uint32_t *)(s + 0x38) != 0)
            FUN_8001ac08((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38));
        if ((*(uint32_t *)owner & 4u) == 0)
            FUN_800207c4(obj);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        return 0x258;
    }
    if (event == 5) {
        if (*(uint32_t *)(s + 0x38) != 0)
            *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38) + 0x60) = 0;
        FUN_800207f8(obj);
        return 0;
    }
    if (event == 12 && owner != NULL) {
        int32_t dist;
        if (iRam00000758 != 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        dist = FUN_80016aac((const int32_t *)(owner + 0x48),
                            (const int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x48));
        return (dist > 0x96000 && dist <= 0x1f3fff) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject10(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0) {
        if (arg != 0)
            (void)FUN_8003c538(obj, arg);
        return 0;
    }
    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 0x28;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        intptr_t bone = FUN_8001b038(obj, 0x8000u);
        uint8_t *shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                0x0f, 0x98u, 8u);
        if (shot == NULL)
            return 0;
        *(uint32_t *)shot = 0x01800594u;
        *(uint16_t *)(shot + 6) = *(uint16_t *)(owner + 6);
        *(uint8_t *)(shot + 4) = 7;
        *(uint16_t *)(shot + 0x0c) = (uint16_t)(((FUN_80017160() & 1u) + 1u));
        *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
        if (bone != 0)
            FUN_8001d68c((MATRIX *)(shot + 0x10), (intptr_t)obj, bone);
        else {
            uint8_t *m = (uint8_t *)FUN_8001d624((int)(uintptr_t)obj);
            memcpy(shot + 0x10, m, 0x20);
        }
        *(uint32_t *)(shot + 0x48) = *(uint32_t *)(shot + 0x24);
        *(uint32_t *)(shot + 0x4c) = *(uint32_t *)(shot + 0x28);
        *(uint32_t *)(shot + 0x50) = *(uint32_t *)(shot + 0x2c);
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_80037b94);
        *(int32_t *)(shot + 0x88) = rtz_shift7(*(int32_t *)(owner + 0x80)) - ((int16_t)*(uint16_t *)(shot + 0x14) / 2);
        *(int32_t *)(shot + 0x8c) = rtz_shift7(*(int32_t *)(owner + 0x84)) - ((int16_t)*(uint16_t *)(shot + 0x1a) / 2);
        *(int32_t *)(shot + 0x90) = rtz_shift7(*(int32_t *)(owner + 0x88)) - ((int16_t)*(uint16_t *)(shot + 0x20) / 2);
        *(uint16_t *)(shot + 0x96) =
            (uint16_t)mips_addu_i32(mips_mult_lo_i32((int32_t)FUN_80017160(), 0x44) >> 15, -0x22);
        FUN_800202f4(shot);

        *(uint16_t *)(s + 0x40) = (uint16_t)(*(uint16_t *)(s + 0x40) + 1u);
        if (*(int16_t *)(s + 0x40) == 1)
            FUN_8004445c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                         0);
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        if (*(int16_t *)(owner + 6) < 0 || *(int16_t *)(s + 0x40) < 0x14) {
            obj[0] |= 0x40000u;
            return 0x0a;
        }
        obj[0] &= 0xfffbffffu;
        return 0xb4;
    }
    if (event == 10) {
        *(uint16_t *)(s + 0x40) = 0;
        return 0;
    }
    if (event == 12 && owner != NULL) {
        int32_t local[3];
        int32_t yaw;
        if (*(int16_t *)(s + 0x40) != 0 || iRam00000758 != 0 || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        FUN_800435c0((uint32_t *)(owner + 0x10),
                     (int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x24),
                     local);
        if (local[2] >= 0 || local[2] <= -0x4b000)
            return 0;
        yaw = FUN_8004ecd4(local[0], local[2]);
        if (yaw < 0)
            yaw = -yaw;
        return (yaw >= 0x556) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return 0;
}
static int Vehicle_StateSubObject11(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 0 && owner != NULL) {
        uint8_t *target = (uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4);
        uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        int32_t worldDelta[3];
        int32_t localDelta[3];
        int32_t planar;
        int32_t scaledY;
        int32_t yaw;
        int32_t pitch;
        int32_t d;

        if (target == NULL || pivot == NULL)
            return 0;
        if (!FUN_8003c538(obj, arg))
            return 0;

        worldDelta[0] = mips_subu_i32(*(int32_t *)(target + 0x48), *(int32_t *)(s + 0x24));
        worldDelta[1] = mips_subu_i32(*(int32_t *)(target + 0x4c), *(int32_t *)(s + 0x28));
        worldDelta[2] = mips_subu_i32(*(int32_t *)(target + 0x50), *(int32_t *)(s + 0x2c));
        FUN_8004352c((uint32_t *)s, worldDelta, localDelta);
        yaw = FUN_8004ecd4(localDelta[0], localDelta[2]);
        yaw = (int16_t)yaw;
        if (yaw > 0x400)
            yaw = 0x400;
        if (yaw < -0x400)
            yaw = -0x400;

        planar = SquareRoot0((long)((uint64_t)(uint32_t)mips_mult_lo_i32(localDelta[0], localDelta[0]) +
                                    (uint64_t)(uint32_t)mips_mult_lo_i32(localDelta[2], localDelta[2])));
        scaledY = (int32_t)((int64_t)localDelta[1] * 0x3000);
        if (planar != 0)
            scaledY = (int32_t)(scaledY / planar);
        pitch = -FUN_8004ecd4(scaledY - (int32_t)(((int64_t)((planar << 3) - planar) * 0x2aaaaaabLL) >> 44),
                              0x3000);
        pitch = (int16_t)pitch;
        if (pitch > 0x155)
            pitch = 0x155;
        if (pitch < -0x71)
            pitch = -0x71;

        d = mips_subu_i32((int16_t)yaw, *(int16_t *)(pivot + 0x42));
        *(uint16_t *)(pivot + 0x42) =
            (uint16_t)(*(uint16_t *)(pivot + 0x42) + (uint16_t)rtz_shift2(d));
        d = mips_subu_i32((int16_t)pitch, *(int16_t *)(pivot + 0x40));
        *(uint16_t *)(pivot + 0x40) =
            (uint16_t)(*(uint16_t *)(pivot + 0x40) + (uint16_t)rtz_shift2(d));
        FUN_8001d708((uint32_t *)pivot);
        return 0;
    }

    if (event == 1) {
        *(uint16_t *)(s + 0x0c) = 0x10;
        *(uint8_t *)(s + 0x08) = 6;
        obj[0] |= 0x4000u;
        return 0;
    }
    if (event == 10) {
        obj[0] &= ~0x20000u;
        *(uint32_t *)(s + 0x40) = 0;
        return 0;
    }
    if (event == 11 && owner != NULL) {
        static int32_t const sourceImpulse[3] = {0, 0, -0x18000};
        uint8_t *pivot = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        uint8_t *effect = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                  4, 0x80u, 8u);
        intptr_t bone = FUN_8001b038((uint32_t *)pivot,
                                     (uint32_t)((*(uint16_t *)(s + 0x0c) & 1u) | 0x8000u));
        uint8_t *shot = (uint8_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                0x18, 0x98u, 8u);
        int32_t impulse[3];

        if (effect == NULL || shot == NULL || pivot == NULL)
            return 0;
        FUN_8001d68c((MATRIX *)(shot + 0x10), (intptr_t)pivot, bone);
        *(uint32_t *)shot = 0x01800094u;
        shot[4] = 7;
        *(uint16_t *)(shot + 6) = *(uint16_t *)(owner + 6);
        *(int32_t *)(shot + 0x48) = *(int32_t *)(shot + 0x24);
        *(int32_t *)(shot + 0x4c) = *(int32_t *)(shot + 0x28);
        *(int32_t *)(shot + 0x50) = *(int32_t *)(shot + 0x2c);
        *(uint16_t *)(shot + 0x0c) = 0x19;
        Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_8003a9dc);
        *(uint16_t *)(shot + 0x94) = 0x3c;
        *(uint32_t *)(shot + 0x80) = (uint32_t)(uintptr_t)owner;
        *(int32_t *)(shot + 0x88) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x80)),
                                                  3 * (int16_t)*(uint16_t *)(shot + 0x14));
        *(int32_t *)(shot + 0x8c) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x84)),
                                                  3 * (int16_t)*(uint16_t *)(shot + 0x1a));
        *(int32_t *)(shot + 0x90) = mips_addu_i32(rtz_shift7(*(int32_t *)(owner + 0x88)),
                                                  3 * (int16_t)*(uint16_t *)(shot + 0x20));
        FUN_800202f4(shot);

        FUN_8001b2fc((uint32_t *)pivot, (const void *)bone, (uint32_t *)effect);
        Object_SetCallbackPsxSlot(effect, (uintptr_t)&LAB_8003e80c);
        if ((*(uint32_t *)owner & 4u) == 0)
            FUN_800207c4(effect);

        FUN_80043358((uint32_t *)pivot, (int32_t *)sourceImpulse, impulse);
        FUN_80017594((uint32_t *)owner, impulse, (int32_t *)(s + 0x48));
        FUN_800447e8(FUN_8004410c(), uRam000005f8, 0x30, (int *)(shot + 0x48));
        if ((obj[0] & 0x20000u) == 0) {
            obj[0] |= 0x20000u;
            FUN_8004445c(FUN_8004410c(),
                         *(uint32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x58) + 8),
                         0);
        }
        *(uint16_t *)(s + 0x0c) = (uint16_t)(*(uint16_t *)(s + 0x0c) - 1u);
        if (*(uint16_t *)(s + 0x0c) == 0)
            FUN_8002cb7c((intptr_t)obj);
        if (*(int16_t *)(owner + 6) < 0 || *(int16_t *)(s + 0x40) < 6) {
            *(uint16_t *)(s + 0x40) = (uint16_t)(*(uint16_t *)(s + 0x40) + 1u);
            obj[0] |= 0x40000u;
            return 0x28;
        }
        obj[0] &= 0xfffbffffu;
        return 0xb4;
    }
    if (event == 12 && owner != NULL) {
        int32_t local[3];
        int32_t yaw;
        if (*(int16_t *)(s + 0x40) != 0 || iRam00000758 != 0 ||
            *(int32_t *)(owner + 0x8c) >= 0xbec || *(uint32_t *)(owner + 0xe4) == 0)
            return 0;
        FUN_800435c0((uint32_t *)(owner + 0x10),
                     (int32_t *)((uint8_t *)(uintptr_t)*(uint32_t *)(owner + 0xe4) + 0x24),
                     local);
        if ((uint32_t)mips_addu_i32(local[2], -0x96000) > 0x95ffeu)
            return 0;
        yaw = FUN_8004ecd4(local[0], local[2]);
        yaw = (int16_t)yaw;
        if (yaw < 0)
            yaw = -yaw;
        return (yaw < 0x2aa) ? 1 : 0;
    }
    if (event == 13)
        return 6;
    if (event == 14)
        return 0x801f;
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject12(uint32_t *obj, int event, intptr_t arg)
{
    uint8_t *s = (uint8_t *)obj;
    uint8_t *owner = (uint8_t *)(uintptr_t)arg;

    if (event == 11 && owner != NULL)
        return Vehicle_FireBlobLauncher(obj, owner, 0x2a, 0x28, 0x64, 8, 2, 0xb4, 0x1e);
    return Vehicle_StateTrackTarget12(obj, event, arg);
}

static VehicleSubObjFn const DAT_8005ecb0_host[13] = {
    (VehicleSubObjFn)Vehicle_StateSubObject0,
    (VehicleSubObjFn)Vehicle_StateSubObject1,
    (VehicleSubObjFn)Vehicle_StateSubObject2,
    (VehicleSubObjFn)Vehicle_StateSubObject3,
    (VehicleSubObjFn)Vehicle_StateSubObject4,
    (VehicleSubObjFn)Vehicle_StateSubObject5,
    (VehicleSubObjFn)Vehicle_StateSubObject6,
    (VehicleSubObjFn)Vehicle_StateSubObject7,
    (VehicleSubObjFn)Vehicle_StateSubObject8,
    (VehicleSubObjFn)Vehicle_StateSubObject9,
    (VehicleSubObjFn)Vehicle_StateSubObject10,
    (VehicleSubObjFn)Vehicle_StateSubObject11,
    (VehicleSubObjFn)Vehicle_StateSubObject12,
};

/* ============================================================
 * FUN_8003d1e8  Vehicle_StateToCallback  (13 instr)
 *
 * Returns the tick callback for a given state code by indexing into
 * a 13-entry function-pointer table at DAT_8005ecb0.
 * Returns NULL for state >= 0xD.
 * ============================================================ */
void *FUN_8003d1e8(uint8_t state_code)
{
    if (state_code >= 0xdu)
        return (void *)0;
    return (void *)DAT_8005ecb0_host[state_code];
}

void *FUN_8003d188(uint32_t *obj, void *sub_obj)
{
    extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
    uintptr_t callback = Object_CallbackFromPsxSlot(sub_obj);
    uint32_t joint_key = 0;

    if (callback != 0) {
        typedef uint32_t (*EventFn)(void *, int, intptr_t);
        joint_key = ((EventFn)callback)(sub_obj, 0xe, 0);
    }
    if (joint_key == 0)
        return (void *)0;
    return (void *)FUN_8001b038(obj, joint_key & 0xffffu);
}

/* ============================================================
 * FUN_8002cce8  Vehicle_InitJoints  (93 instr, 580 B)
 *
 * a0 = self (vehicle chassis object)
 * a1 = cap_mask (bit i set = allocate sub-object for slot i)
 * ============================================================ */
void FUN_8002cce8(uint32_t *self, uint8_t cap_mask)
{
    uint8_t *s = (uint8_t *)self;

    /* s4 strides through the 8-byte descriptor table one entry per
     * iteration.  s5 holds a ptr to the world pool registry. */
    int            s2;

    for (s2 = 0; s2 < 7; s2++) {
        /* Check bit s2 of cap_mask. */
        if (!((cap_mask >> s2) & 1))
            continue;

        void *new_obj;

        if (s2 < 6) {
            /* Fast path: allocate 0x80-byte sub-object from the pool.
             * kind = *(u16*)s4,  callback = *(u32*)(s4+4) */
            uint16_t kind     = DAT_80010534_host[s2].kind;
            uintptr_t callback = (uintptr_t)DAT_80010534_host[s2].callback;
            uint32_t *bank    = (uint32_t *)(uintptr_t)DAT_800737a0[15];
            new_obj = FUN_8001ac44(bank, kind, 0x80u, 0x8u);
            Object_SetCallbackPsxSlot(new_obj, callback);
        } else {
            /* Bone-table path for slot 6. */
            uint16_t bone_key = 0x801fu;
            FUN_8001b038(self, bone_key);      /* sets up bone selection */

            uint32_t *bone_tbl = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x58);
            uint16_t  bone_idx16;
            /* Allocate from bone table params (size=0x80, flags=8). */
            new_obj = FUN_8001ac44((int *)bone_tbl,
                                   (uint16_t)*(uint16_t *)(s + 0x1a),
                                   0x80u, 0x8u);

            /* Install state-driven callback. */
            void *cb = FUN_8003d1e8(*(uint8_t *)(s + 0xd0));
            Object_SetCallbackPsxSlot(new_obj, (uintptr_t)cb);
        }

        /* Fire mode-1 (init) if the new object has a tick callback. */
        typedef void (*TickFn)(void *obj, int mode, int arg);
        *(int16_t *)((uint8_t *)new_obj + 6) = 0;
        uintptr_t cb_raw = Object_CallbackFromPsxSlot(new_obj);
        if (cb_raw) {
            ((TickFn)(uintptr_t)cb_raw)(new_obj, 1, 0);
        }

        /* Joint query: FUN_8003d188 returns the joint slot, or NULL. */
        void *joint = FUN_8003d188(self, new_obj);
        int   s0    = -1;   /* set here; used for slot index below */

        if (joint) {
            FUN_8001b2fc(self, joint, (uint32_t *)new_obj);
        } else {
            /* Null joint is unexpected -- call error trap. */
            static const char errmsg[] = "Vehicle_InitJoints missing joint";
            FUN_80015368(errmsg);
        }

        /* Store new_obj into the vehicle's child/node slot array.
         *
         * s2 == 0: write to child slot [4] at offset 0x10C
         *   v0 = (s0+9)*4 = (-1+9)*4 = 8*4 = 32, offset = 0x20
         *   *(self + 0x20 + 0xEC) = *(self + 0x10C) = new_obj
         *
         * s2 != 0: scan node slots self+0x114..0x11C for first empty,
         *   then *(self + (slot_idx+9)*4 + 0xEC) = new_obj
         */
        if (s2 == 0) {
            /* Direct store at (s0+9)*4+0xEC = (-1+9)*4+0xEC = 0x10C. */
            uint32_t target_off = (uint32_t)((s0 + 9) * 4);
            *(uint32_t *)(s + target_off + 0xec) = (uint32_t)(uintptr_t)new_obj;
        } else {
            /* Scan for first empty node slot starting from node[1..3].
             * If node[0] (self+0x110) is zero, store at node[0] (s0=0). */
            int scan = 0;
            if (*(uint32_t *)(s + 0x110) != 0) {
                /* Node[0] occupied: walk node[1..3] looking for empty. */
                uint32_t v1 = 0x24u;
                scan = 0;
                while (scan < 4) {
                    v1 += 4u;
                    if (*(uint32_t *)(s + v1 + 0xec) == 0)
                        break;
                    scan++;
                }
            }
            /* Store: *(self + (scan+9)*4 + 0xEC) = new_obj */
            uint32_t target_off = (uint32_t)((scan + 9) * 4);
            *(uint32_t *)(s + target_off + 0xec) = (uint32_t)(uintptr_t)new_obj;
        }
    }
}
