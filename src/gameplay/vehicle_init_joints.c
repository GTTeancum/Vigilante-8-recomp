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
typedef int (*VehicleSubObjFn)(void *, int, intptr_t);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
typedef struct {
    uint16_t kind;
    uint16_t pad;
    VehicleSubObjFn callback;
} VehicleJointInitDesc;

extern int FUN_80031864(void *obj, int event, intptr_t arg);
extern int LAB_80031fa0(void *obj, int event, intptr_t arg);
extern int LAB_8003302c(void *obj, int event, intptr_t arg);
extern int LAB_800336fc(void *obj, int event, intptr_t arg);
extern int FUN_80034920(void *obj, int event, intptr_t arg);
extern int LAB_8003565c(void *obj, int event, intptr_t arg);

static const VehicleJointInitDesc DAT_80010534_host[6] = {
    { 0x0002, 0, FUN_80031864 },
    { 0x0000, 0, LAB_80031fa0 },
    { 0x0011, 0, LAB_8003302c },
    { 0x0007, 0, LAB_800336fc },
    { 0x000a, 0, FUN_80034920 },
    { 0x000d, 0, LAB_8003565c },
};

/* DAT_800737a0[15] is the common sub-object pool (MIPS: lw a0,0x3c(s5)
 * with s5=&DAT_800737a0, not *(DAT_800737a0[0]+0x3c)). */
extern uintptr_t DAT_800737a0[];

extern void   *FUN_8001ac44(int *bank, uint16_t kind, uint32_t size, uint32_t flags);
extern intptr_t FUN_8001b038(uint32_t *obj, uint32_t arg);      /* bone table query */
extern void   *FUN_8003d1e8(uint8_t state_code);                /* state->callback */
extern void    FUN_8001b2fc(uint32_t *chassis, const void *joint, uint32_t *wheel);
extern int     FUN_80015368(const char *msg);                   /* error trap */
extern int     FUN_8003c538(uint32_t *obj, uint32_t arg);
extern int32_t FUN_80016aac(const int32_t *a, const int32_t *b);
extern void   *FUN_8001d624(int obj);
extern void    FUN_8001fcb4(int payload, uint16_t arg);
extern void    FUN_8004352c(uint32_t *m, int32_t *v, int32_t *out);
extern int     FUN_8004ecd4(int y, int x);
extern void    FUN_8001d708(uint32_t *obj);
extern int32_t iRam00000758;

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
        if (!FUN_8003c538(obj, (uint32_t)(uintptr_t)target))
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
        delta[0] = *(int32_t *)(owner + 0x48) - *(int32_t *)((uint8_t *)m + 0x14);
        delta[1] = *(int32_t *)(owner + 0x4c) - *(int32_t *)((uint8_t *)m + 0x18);
        delta[2] = *(int32_t *)(owner + 0x50) - *(int32_t *)((uint8_t *)m + 0x1c);
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
    case 10:
        obj[0] &= 0xfffdffffu;
        if (*(uint32_t *)(s + 0x38) != 0)
            FUN_8001d708((uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38));
        return 0;
    case 12:
        if (target == 0 || iRam00000758 != 0)
            return 0;
        if (*(int32_t *)((uint8_t *)target + 0x8c) >= 0x11e1)
            return 0;
        if (*(uint32_t *)((uint8_t *)target + 0xe4) == 0)
            return 0;
        {
            uint8_t *owner = (uint8_t *)(uintptr_t)*(uint32_t *)((uint8_t *)target + 0xe4);
            int dx = *(int32_t *)((uint8_t *)target + 0x48) - *(int32_t *)(owner + 0x48);
            int dy = *(int32_t *)((uint8_t *)target + 0x4c) - *(int32_t *)(owner + 0x4c);
            int dz = *(int32_t *)((uint8_t *)target + 0x50) - *(int32_t *)(owner + 0x50);
            int64_t dist2 = (int64_t)dx * dx + (int64_t)dy * dy + (int64_t)dz * dz;
            return dist2 <= (int64_t)0x12bfff * (int64_t)0x12bfff;
        }
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

    if (event == 0) {
        if (target == 0)
            return 0;
        if (!FUN_8003c538(obj, (uint32_t)(uintptr_t)target))
            return 0;
        uint32_t target_owner = *(uint32_t *)((uint8_t *)target + 0xe4);
        if (target_owner == 0)
            return 0;

        uint32_t *m = (uint32_t *)FUN_8001d624((int)(uintptr_t)obj);
        uint8_t *owner = (uint8_t *)(uintptr_t)target_owner;
        int32_t delta[3];
        delta[0] = *(int32_t *)(owner + 0x48) - *(int32_t *)((uint8_t *)m + 0x14);
        delta[1] = *(int32_t *)(owner + 0x4c) - *(int32_t *)((uint8_t *)m + 0x18);
        delta[2] = *(int32_t *)(owner + 0x50) - *(int32_t *)((uint8_t *)m + 0x1c);
        FUN_8004352c(m, delta, delta);

        int32_t yaw = (int32_t)*(uint16_t *)(s + 0x42) +
                      (int32_t)(int16_t)FUN_8004ecd4(delta[0], delta[2]);
        yaw = (yaw << 20) >> 20;
        if (yaw < -0x200)
            yaw = -0x200;
        else if (yaw > 0x200)
            yaw = 0x200;

        int32_t pitch = (int32_t)*(uint16_t *)(s + 0x40) -
                        (int32_t)(int16_t)FUN_8004ecd4(delta[1], delta[2]);
        pitch = (pitch << 20) >> 20;
        if (pitch < -0xe3)
            pitch = -0xe3;
        else if (pitch > 0xe3)
            pitch = 0xe3;

        uint32_t *pivot = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x38);
        if (pivot != 0) {
            *(int16_t *)((uint8_t *)pivot + 0x42) = (int16_t)yaw;
            *(int16_t *)((uint8_t *)pivot + 0x40) = (int16_t)pitch;
            FUN_8001d708(pivot);
        }
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
            (void)FUN_8003c538(obj, (uint32_t)(uintptr_t)target);
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
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject3(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject4(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject5(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject6(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject7(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject8(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject9(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject10(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject11(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateSubObjectCommon(obj, event, arg);
}
static int Vehicle_StateSubObject12(uint32_t *obj, int event, intptr_t arg)
{
    return Vehicle_StateTrackTarget12(obj, event, arg);
}

static VehicleSubObjFn const DAT_8005ecb0_host[13] = {
    Vehicle_StateSubObject0,
    Vehicle_StateSubObject1,
    Vehicle_StateSubObject2,
    Vehicle_StateSubObject3,
    Vehicle_StateSubObject4,
    Vehicle_StateSubObject5,
    Vehicle_StateSubObject6,
    Vehicle_StateSubObject7,
    Vehicle_StateSubObject8,
    Vehicle_StateSubObject9,
    Vehicle_StateSubObject10,
    Vehicle_StateSubObject11,
    Vehicle_StateSubObject12,
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
