/* vehicle_weapon_tick.c -- Per-frame weapon / targeting / flee AI ticks.
 *
 * Source: SLUS_005.10
 *   FUN_80022d54  -- Vehicle_WeaponTick_Init    (state 1: initial targeting setup)
 *   FUN_8002305c  -- Vehicle_WeaponTick_Player  (state 2: player weapon targeting)
 *   FUN_8002346c  -- Vehicle_WeaponTick_Flee    (state 3: AI escape/flee logic)
 *   FUN_80022e38  -- Vehicle_WeaponTick_Reset   (state 4: reset/re-init)
 *   FUN_80023940  -- Vehicle_WeaponDispatch     (dispatcher: routes to above by kind byte)
 *
 * FUN_80023940 (Vehicle_WeaponDispatch) is the top-level tick.
 * Called from Vehicle_Tick (vehicle_tick.c) whenever the vehicle's control
 * kind byte (param_1[+8]) selects the weapon-targeting branch.
 *
 * State machine:
 *   kind==1 -> FUN_80022d54: If at a valid quadtree cell, call Path_Init
 *              to set up path-following.  Set kind to 3 if successful.
 *   kind==2 -> FUN_8002305c: Player steering + weapon angle tracking.
 *   kind==3 -> FUN_8002346c: AI flee: find an escape waypoint, steer toward it.
 *   kind==4 -> FUN_80022e38: Simple timeout-based re-init (kind -> 1 after delay).
 *
 * Confidence: MED.  Line-by-line translation from Ghidra pseudoC with two
 * known dropped-argument fixes (annotated inline).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- externals ----------------------------------------------------------- */

extern int32_t  iRam0000000c;      /* frame tick counter (gp + 0xc) */

/* Path-follow AI. */
extern void FUN_80042ef0(intptr_t param_1, intptr_t param_2, int *param_3,
                         int param_4, int param_5);  /* Path_Init */
extern int  FUN_80042f98(intptr_t param_1, int16_t *param_2, int param_3);
                                                     /* Path_AngleDelta */

/* Quadtree cell check (stub in panic_stubs.c). */
extern uint32_t FUN_800244c4(int param_1, int param_2);

/* Object-list helpers (object_list_extra.c). */
extern intptr_t FUN_8001ffd4(intptr_t param_1, int param_2);
extern int      FUN_80020120(int *param_1, uint32_t param_2);
extern uint32_t *FUN_80020190(int *param_1, uint32_t param_2, int param_3);

/* Nearest-object finder (vehicle_target.c). */
extern uint32_t *FUN_80023394(int *param_1, uint32_t param_2, intptr_t param_3);

/* Target acquisition + reticle (ai_target.c / vehicle_node.c). */
extern int  FUN_80022e90(uint32_t *param_1);   /* Vehicle_TryAcquireTarget */
extern void FUN_8002ce68(uint32_t *param_1, int lock_flag); /* Reticle_UpdateLock */
extern int  FUN_8002cf90(uint32_t *param_1, int n);         /* Vehicle_CycleNodeSlot */

/* RNG (rng.c). */
extern uint32_t FUN_80017160(void);            /* V8_RandNext */

/* Fatal-error handler (splash.c). */
extern int FUN_80015368(const char *msg);

/* World object-list sentinel. */
extern uint8_t DAT_80065a18[];
extern uint8_t DAT_80065a50[];
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

typedef int (*VehicleWeaponCb)(uint32_t obj, int mode, uint32_t *self);
#define VEHICLE_WEAPON_CB(obj) ((VehicleWeaponCb)Object_CallbackFromPsxSlot((const void *)(uintptr_t)(obj)))
#define VEHICLE_BYTE(obj, off) (*(uint8_t *)((uint8_t *)(obj) + (off)))
#define VEHICLE_HALF(obj, off) (*(int16_t *)((uint8_t *)(obj) + (off)))

static int host_object_ptr_ok(uintptr_t p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + Host_HeapSize();

    return p >= base && p + 0x124 <= end;
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

/* ============================================================
 * FUN_80022d54 -- Vehicle_WeaponTick_Init  (state 1)
 *
 * Called when vehicle kind byte == 1 (initial targeting).
 * If the vehicle's current cell is valid (FUN_800244c4 != 0):
 *   - Calls Path_Init (FUN_80042ef0) to initialise the path-follow block.
 *   - Promotes kind to 3 (flee/escape) and clears flag bit 0x20.
 * Always resets the primary-weapon callback:
 *   - Zeroes the turn-rate and speed counters.
 *   - Calls weapon sub-object (+0x43) with mode 0xB (deactivate) or
 *     0x4 (activate) depending on weapon state byte.
 *
 * HIGH (direct Ghidra ref port).
 * ============================================================ */
void FUN_80022d54(uint *param_1)
{
    short    sVar1;
    int      iVar2;
    undefined4 uVar3;
    uint     uVar4;

    sVar1 = (short)FUN_800244c4(param_1[9], param_1[0xb]);
    if (sVar1 != 0) {
        FUN_80042ef0((intptr_t)(param_1 + 0x30), (intptr_t)(param_1 + 9),
                     (int *)(uintptr_t)(param_1[0x39] + 0x24),
                     0x22740, 0);
        *(undefined1 *)(param_1 + 2) = 3;
        *param_1 = *param_1 & 0xffffffdfU;
    }
    uVar4 = param_1[0x43];
    if (uVar4 == 0)
        return;
    *(undefined2 *)(param_1 + 0x29) = 0;
    VEHICLE_HALF(param_1, 0xa6) = 0x3c;
    uVar3 = 0xb;
    if (*(char *)(uintptr_t)(uVar4 + 8) == '\0') {
        VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar4);
        if (cb == NULL) {
            iVar2 = 0;
        } else {
            iVar2 = cb(uVar4, 0xc, param_1);
        }
        uVar3 = 0xb;
        if (iVar2 == 0) {
            uVar3 = 4;
        }
    }
    {
        VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar4);
        if (cb != NULL)
            cb(uVar4, (int)uVar3, param_1);
    }
}

/* ============================================================
 * FUN_8002305c -- Vehicle_WeaponTick_Player  (state 2)
 *
 * Per-frame tick for player vehicle weapon targeting.
 *
 *  1. Every 0x80 frames (from iRam0000000c), if path-follow counter
 *     (param_1[0x30]) is idle or forced (flag 0x200000), reinitialise
 *     the path via FUN_80042ef0.
 *  2. Set "active" flag at +0xb2 = 1.
 *  3. Compute angle delta to next waypoint via FUN_80042f98; clamp
 *     to [-0x2aa, +0x2aa] and write to param_1[0x29] (turn rate).
 *  4. Compute throttle contribution from param_1[0x23] (speed) * speed_cap
 *     at +0xaa; clamp 0..+; accumulate into param_1[0x25] (throttle).
 *  5. Update turn-rate accumulator at +0xa6 based on abs(angle) and speed:
 *     - Fast turn (|angle|>=0x156 and speed>=0xbec): decrement toward -cap
 *     - Slow/aligned (speed<0xbec or |angle|<0x156): depends on speed:
 *         speed>=0x1ad3: clamp toward -cap
 *         speed<0x1ad3:  increment toward +cap
 *  6. Targeting: if primary-slot weapon (+0xb3 -> index) is idle (status==0),
 *     call TryAcquireTarget; if a hit via weapon callback (mode 0xC), lock on.
 *  7. Call Reticle_UpdateLock with lock result.
 *  8. Call secondary weapon sub-object (+0x43) with mode 0xB (tracking) or 0x4.
 *  9. If targeting succeeded: maybe cycle node slot (FUN_8002cf90) if RNG low 3
 *     bits == 0.
 *
 * MED (Ghidra ref with undefined2/4 types preserved; bVar1 goto preserved).
 * ============================================================ */
void FUN_8002305c(uint *param_1)
{
    bool       bVar1;
    short      sVar2;
    int        iVar3;
    undefined2 uVar4;
    int        iVar5;
    int        iVar6;
    undefined4 uVar7;
    uint       uVar8;

    /* Step 1: path re-init every 0x80 ticks, when path idle or flag set. */
    if (((mips_subu_i32(iRam0000000c, (int32_t)(uint32_t)VEHICLE_BYTE(param_1, 9)) & 0x7f) == 0) &&
        (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
        FUN_80042ef0((intptr_t)(param_1 + 0x30), (intptr_t)(param_1 + 9),
                     (int *)(uintptr_t)(param_1[0x39] + 0x24),
                     0x22740, 0);
    }

    /* Step 2: set active flag. */
    VEHICLE_BYTE(param_1, 0xb2) = 1;

    /* Step 3: angle delta -> clamped turn rate. */
    sVar2 = (short)FUN_80042f98((intptr_t)param_1, (int16_t *)(param_1 + 0x30),
                                 mips_addu_i32(mips_mult_lo_i32((int32_t)param_1[0x23], 0x20),
                                               0x10000));
    iVar5 = (int)sVar2;
    iVar6 = -0x2aa;
    if ((-0x2ab < iVar5) && (iVar6 = 0x2aa, iVar5 < 0x2ab)) {
        iVar6 = iVar5;
    }
    *(short *)(param_1 + 0x29) = (short)iVar6;

    /* Step 4: throttle. */
    iVar5 = mips_mult_lo_i32((int32_t)param_1[0x23], (int)VEHICLE_HALF(param_1, 0xaa));
    if (iVar5 < 0) {
        iVar5 = mips_addu_i32(iVar5, 0xfff);
    }
    iVar3 = mips_addu_i32((int)(short)param_1[0x2a], iVar5 >> 0xc);
    iVar5 = 0;
    if (0 < iVar3) {
        iVar5 = iVar3;
    }
    iVar6 = mips_mult_lo_i32(iVar6, iVar5);
    if (iVar6 < 0) {
        iVar6 = mips_addu_i32(iVar6, 0xf);
    }
    param_1[0x25] = (uint32_t)mips_addu_i32((int32_t)param_1[0x25], iVar6 >> 4);

    /* Step 5: turn-rate accumulator update. */
    iVar6 = (int)sVar2;
    if (iVar6 < 0) {
        iVar6 = mips_subu_i32(0, iVar6);
    }
    if ((iVar6 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
        if ((int)param_1[0x23] < 0x1ad3) {
            iVar6 = 0;
            if (0 < VEHICLE_HALF(param_1, 0xa6)) {
                iVar6 = (int)VEHICLE_HALF(param_1, 0xa6);
            }
            uVar8 = (uint)(ushort)param_1[0x2b];
            iVar6 = mips_addu_i32(iVar6, 1);
            bVar1 = iVar6 < (int)uVar8;
            goto LAB_80023248;
        }
        iVar5 = mips_addu_i32((int)VEHICLE_HALF(param_1, 0xa6), -1);
        iVar6 = mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]);
        if (mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]) < iVar5) {
            iVar6 = iVar5;
        }
        VEHICLE_HALF(param_1, 0xa6) = (short)iVar6;
    } else {
        iVar6 = 0;
        if (VEHICLE_HALF(param_1, 0xa6) < 0) {
            iVar6 = (int)VEHICLE_HALF(param_1, 0xa6);
        }
        iVar6 = mips_addu_i32(iVar6, -1);
        uVar8 = (uint)mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]);
        bVar1 = (int)uVar8 < iVar6;
LAB_80023248:
        uVar4 = (undefined2)uVar8;
        if (bVar1) {
            uVar4 = (undefined2)iVar6;
        }
        VEHICLE_HALF(param_1, 0xa6) = uVar4;
    }

    /* Step 6: targeting. */
    uVar8 = param_1[VEHICLE_BYTE(param_1, 0xb3) + 0x44];
    iVar6 = 0;
    bVar1 = false;
    if (uVar8 != 0) {
        bVar1 = *(short *)(uintptr_t)(uVar8 + 6) == 0;
    }
    if ((bVar1) && (iVar5 = FUN_80022e90(param_1), iVar5 == 0)) {
        VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar8);
        if (cb == NULL) {
            iVar5 = 0;
        } else {
            iVar5 = cb(uVar8, 0xc, param_1);
        }
        if (iVar5 != 0) {
            iVar6 = 1;
        }
    }

    /* Step 7: Reticle_UpdateLock. */
    FUN_8002ce68(param_1, iVar6);

    /* Step 8: secondary weapon dispatch. */
    uVar8 = param_1[0x43];
    if (*(char *)(uintptr_t)(uVar8 + 8) == '\0') {
        if (iVar6 != 0) {
            uVar7 = 4;
            goto LAB_80023334;
        }
        VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar8);
        if (cb == NULL) {
            iVar5 = 0;
        } else {
            iVar5 = cb(uVar8, 0xc, param_1);
        }
        uVar7 = 4;
        if (iVar5 == 0) goto LAB_80023334;
    }
    uVar7 = 0xb;
LAB_80023334:
    {
        VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar8);
        if (cb != NULL)
            cb(uVar8, (int)uVar7, param_1);
    }

    /* Step 9: maybe cycle node slot. */
    if ((bVar1) && ((iVar6 == 0 ||
                     (uVar8 = FUN_80017160(), (uVar8 & 7) == 0)))) {
        FUN_8002cf90(param_1, 1);
    }
}

/* ============================================================
 * FUN_8002346c -- Vehicle_WeaponTick_Flee  (state 3)
 *
 * AI vehicle escape/flee targeting tick.
 *
 *  1. Every 0x80 ticks, choose an escape waypoint:
 *       a. If no stored path (param_1[0x3a]==0) or lookup fails:
 *            - If weapons near depletion: try FUN_80023394 flag=0x100000
 *              (find nearest enemy), fall back to flag=0x400000 or 0x7f000000.
 *            - If params[0x45] (some config): try flag=0x7f000000/0x7f780000
 *              or pick a random object from the world list by RNG.
 *       b. Store found waypoint in param_1[0x3a] (as spawn-id from +6).
 *       c. Call Path_Init toward the waypoint's position (+0x48).
 *  2-9: Same steering/throttle/weapon-targeting logic as FUN_8002305c,
 *       but with a different proximity check before targeting:
 *         - if vehicle is within 0x12c000 world units of its target in
 *           each axis, only then run targeting logic.
 *
 * MED: two Ghidra dropped-arg fixes annotated inline.
 * ============================================================ */
void FUN_8002346c(uint *param_1)
{
    bool       bVar1;
    short      sVar2;
    int        iVar3;
    void       *pcVar4;
    undefined2 uVar5;
    int        iVar6;
    uint       uVar7;
    undefined4 uVar8;
    int        iVar9;
    intptr_t   iVar10;
    uint       uVar11;
    uint       uVar12;

    /* Step 1: path selection every 0x80 ticks. */
    if (((mips_subu_i32(iRam0000000c, (int32_t)(uint32_t)VEHICLE_BYTE(param_1, 9)) & 0x7f) == 0) &&
        (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
        iVar10 = 0;
        /* FIX: Ghidra dropped 2nd arg.  param_1[0x3a] is the stored path-id;
         * look it up in the world-object list (DAT_80065a18). */
        if (((short)param_1[0x3a] == 0) ||
            (iVar10 = FUN_8001ffd4((intptr_t)DAT_80065a18, (short)param_1[0x3a]),
             iVar10 == 0)) {
            /* No stored path or it expired: find a new escape target. */
            if ((uint32_t)mips_addu_i32(
                    mips_addu_i32((int32_t)(uint32_t)*(ushort *)(uintptr_t)(param_1[0x3b] + 0xc),
                                  (int32_t)(uint32_t)*(ushort *)(uintptr_t)(param_1[0x3c] + 0xc)),
                    (int32_t)(uint32_t)*(ushort *)(uintptr_t)(param_1[0x3d] + 0xc)) <
                (uint)(ushort)param_1[3]) {
                /* Weapons below ammo threshold: flee toward enemy. */
                iVar10 = (intptr_t)FUN_80023394(
                    (int *)DAT_80065a18, 0x100000, (intptr_t)(param_1 + 9));
                uVar8 = 0x400000;
                if (iVar10 == 0) {
LAB_80023570:
                    iVar10 = (intptr_t)FUN_80023394(
                        (int *)DAT_80065a18, (uint)uVar8, (intptr_t)(param_1 + 9));
                    goto LAB_8002357c;
                }
LAB_800235a0:
                if (!host_object_ptr_ok((uintptr_t)iVar10))
                    return;
                *(undefined2 *)(param_1 + 0x3a) = *(undefined2 *)(uintptr_t)(iVar10 + 6);
            } else {
                /* Ammo above threshold: flee from player. */
                uVar8 = 0x7f000000;
                if (param_1[0x45] == 0) goto LAB_80023570;
LAB_8002357c:
                if ((iVar10 != 0) ||
                    (iVar10 = (intptr_t)FUN_80023394(
                         (int *)DAT_80065a18, 0x7f780000, (intptr_t)(param_1 + 9)),
                     iVar10 != 0))
                    goto LAB_800235a0;
                /* No object found: pick a random one by RNG. */
                iVar10 = (int)FUN_80017160();
                iVar6 = FUN_80020120((int *)DAT_80065a50, 0x7f780000);
                iVar10 = (intptr_t)FUN_80020190(
                    (int *)DAT_80065a50, 0x7f780000,
                    mips_mult_lo_i32((int32_t)iVar10, iVar6) >> 0xf);
            }
            if (iVar10 == 0) {
                iVar10 = (intptr_t)(uintptr_t)param_1[0x39];
                if (iVar10 == 0 || !host_object_ptr_ok((uintptr_t)iVar10))
                    FUN_80015368("Nowhere to run!");
            }
        }
        if (!host_object_ptr_ok((uintptr_t)iVar10))
            return;
        FUN_80042ef0((intptr_t)(param_1 + 0x30), (intptr_t)(param_1 + 9),
                     (int *)(uintptr_t)(iVar10 + 0x48),
                     0x22740, 0);
    }

    /* Steps 2-5: steering + throttle (identical to FUN_8002305c). */
    VEHICLE_BYTE(param_1, 0xb2) = 1;
    sVar2 = (short)FUN_80042f98((intptr_t)param_1, (int16_t *)(param_1 + 0x30),
                                 mips_addu_i32(mips_mult_lo_i32((int32_t)param_1[0x23], 0x20),
                                               0x10000));
    iVar10 = (int)sVar2;
    iVar6 = -0x2aa;
    if (-0x2aa < iVar10) {
        iVar6 = iVar10;
    }
    iVar9 = 0x2aa;
    if (iVar6 < 0x2aa) {
        iVar9 = iVar6;
    }
    *(short *)(param_1 + 0x29) = (short)iVar9;
    iVar6 = mips_mult_lo_i32((int32_t)param_1[0x23], (int)VEHICLE_HALF(param_1, 0xaa));
    if (iVar6 < 0) {
        iVar6 = mips_addu_i32(iVar6, 0xfff);
    }
    iVar3 = mips_addu_i32((int)(short)param_1[0x2a], iVar6 >> 0xc);
    iVar6 = 0;
    if (0 < iVar3) {
        iVar6 = iVar3;
    }
    iVar6 = mips_mult_lo_i32((int16_t)iVar9, iVar6);
    if (iVar6 < 0) {
        iVar6 = mips_addu_i32(iVar6, 0xf);
    }
    param_1[0x25] = (uint32_t)mips_addu_i32((int32_t)param_1[0x25], iVar6 >> 4);
    if (iVar10 < 0) {
        iVar10 = mips_subu_i32(0, (int32_t)iVar10);
    }
    if ((iVar10 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
        if ((int)param_1[0x23] < 0x1ad3) {
            iVar10 = 0;
            if (0 < VEHICLE_HALF(param_1, 0xa6)) {
                iVar10 = (int)VEHICLE_HALF(param_1, 0xa6);
            }
            uVar11 = (uint)(ushort)param_1[0x2b];
            iVar10 = iVar10 + 1;
            bVar1 = iVar10 < (int)uVar11;
            goto LAB_80023778;
        }
        iVar6 = mips_addu_i32((int)VEHICLE_HALF(param_1, 0xa6), -1);
        iVar10 = mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]);
        if (mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]) < iVar6) {
            iVar10 = iVar6;
        }
        VEHICLE_HALF(param_1, 0xa6) = (short)iVar10;
    } else {
        iVar10 = 0;
        if (VEHICLE_HALF(param_1, 0xa6) < 0) {
            iVar10 = (int)VEHICLE_HALF(param_1, 0xa6);
        }
        iVar10 = mips_addu_i32((int32_t)iVar10, -1);
        uVar11 = (uint)mips_subu_i32(0, (int32_t)(uint32_t)(ushort)param_1[0x2b]);
        bVar1 = (int)uVar11 < iVar10;
LAB_80023778:
        uVar5 = (undefined2)uVar11;
        if (bVar1) {
            uVar5 = (undefined2)iVar10;
        }
        VEHICLE_HALF(param_1, 0xa6) = uVar5;
    }

    /* Steps 6-9: targeting (proximity-gated). */
    uVar11 = param_1[VEHICLE_BYTE(param_1, 0xb3) + 0x44];
    uVar12 = param_1[0x43];
    if (uVar12 == 0)
        return;
    if (((uVar11 != 0) && (*(short *)(uintptr_t)(uVar11 + 6) == 0)) &&
        (iVar10 = FUN_80022e90(param_1), iVar10 == 0)) {
        uVar7 = param_1[0x39];
        if (uVar7 == 0 || !host_object_ptr_ok((uintptr_t)uVar7))
            goto default_secondary_dispatch;
        iVar10 = mips_subu_i32((int32_t)param_1[9], *(int32_t *)(uintptr_t)(uVar7 + 0x24));
        if (iVar10 < 0) iVar10 = mips_subu_i32(0, iVar10);
        if (iVar10 < 0x12c000) {
            iVar10 = mips_subu_i32((int32_t)param_1[10], *(int32_t *)(uintptr_t)(uVar7 + 0x28));
            if (iVar10 < 0) iVar10 = mips_subu_i32(0, iVar10);
            if (iVar10 < 0x12c000) {
                iVar10 = mips_subu_i32((int32_t)param_1[0xb], *(int32_t *)(uintptr_t)(uVar7 + 0x2c));
                if (iVar10 < 0) iVar10 = mips_subu_i32(0, iVar10);
                if (iVar10 < 0x12c000) {
                    VehicleWeaponCb cb = VEHICLE_WEAPON_CB(uVar11);
                    if (cb == NULL) {
                        iVar10 = 0;
                    } else {
                        iVar10 = cb(uVar11, 0xc, param_1);
                    }
                    FUN_8002ce68(param_1, iVar10);
                    if ((iVar10 == 0) ||
                        (uVar11 = FUN_80017160(), (uVar11 & 7) == 0)) {
                        FUN_8002cf90(param_1, 1);
                    }
                    if (*(char *)(uintptr_t)(uVar12 + 8) == '\0') {
                        if (iVar10 == 0) {
                            VehicleWeaponCb cb2 = VEHICLE_WEAPON_CB(uVar12);
                            if (cb2 == NULL) {
                                iVar10 = 0;
                            } else {
                                iVar10 = cb2(uVar12, 0xc, param_1);
                            }
                            uVar8 = 4;
                            if (iVar10 != 0) goto LAB_800238e4;
                        } else {
                            uVar8 = 4;
                        }
                    } else {
LAB_800238e4:
                        uVar8 = 0xb;
                    }
                    pcVar4 = (void *)VEHICLE_WEAPON_CB(uVar12);
                    if (pcVar4 == NULL) {
                        return;
                    }
                    goto LAB_8002391c;
                }
            }
        }
    }
default_secondary_dispatch:
    pcVar4 = (void *)VEHICLE_WEAPON_CB(uVar12);
    if (pcVar4 == NULL) {
        return;
    }
    uVar8 = 4;
LAB_8002391c:
    (*(int (*)(uint, undefined4, uint *))(uintptr_t)pcVar4)(uVar12, uVar8, param_1);
}

/* ============================================================
 * FUN_80022e38 -- Vehicle_WeaponTick_Reset  (state 4)
 *
 * Simple reset tick: if the frame counter is past 0x3c, promote the
 * vehicle's kind to 1 (initial targeting).  Resets the speed/angle
 * state variables and fires the primary weapon callback with mode 0xB.
 *
 * HIGH (direct Ghidra ref port).
 * ============================================================ */
void FUN_80022e38(intptr_t param_1)
{
    VehicleWeaponCb pcVar1;

    if (0x3c < iRam0000000c) {
        *(undefined1 *)(uintptr_t)(param_1 + 8) = 1;
    }
    *(undefined2 *)(uintptr_t)(param_1 + 0xa4) = 0;
    *(undefined2 *)(uintptr_t)(param_1 + 0xa6) = 0x3c;
    pcVar1 = VEHICLE_WEAPON_CB(*(uint32_t *)(uintptr_t)(param_1 + 0x10c));
    if (pcVar1 != NULL) {
        pcVar1(*(uint32_t *)(uintptr_t)(param_1 + 0x10c), 0xb, (uint32_t *)(uintptr_t)param_1);
    }
}

/* ============================================================
 * FUN_80023940 -- Vehicle_WeaponDispatch
 *
 * Dispatches to the appropriate weapon/targeting tick based on the
 * vehicle's kind byte (param_1[+8]):
 *   1 -> FUN_80022d54  (initial targeting)
 *   2 -> FUN_8002305c  (player weapon tick)
 *   3 -> FUN_8002346c  (AI flee tick)
 *   4 -> FUN_80022e38  (reset)
 *
 * Called from Vehicle_Tick (vehicle_tick.c) for vehicles whose main
 * tick selects the weapon-state branch.
 *
 * Param type: uint32_t * (matching vehicle_tick.c extern declaration).
 * Ghidra shows int param_1; on PSX both are 32-bit.
 *
 * HIGH.
 * ============================================================ */
void FUN_80023940(uint32_t *param_1)
{
    switch (*(uint8_t *)((uint8_t *)param_1 + 8)) {
    case 1:
        FUN_80022d54((uint *)param_1);
        break;
    case 2:
        FUN_8002305c((uint *)param_1);
        break;
    case 3:
        FUN_8002346c((uint *)param_1);
        break;
    case 4:
        FUN_80022e38((intptr_t)param_1);
        break;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80022d54  (from analysis/SLUS_005.10/decomp/80022d54.c) --- */
// addr: 0x80022d54  name: FUN_80022d54

void FUN_80022d54(uint *param_1)
{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  sVar1 = FUN_800244c4(param_1[9],param_1[0xb]);
  if (sVar1 != 0) {
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,param_1[0x39] + 0x24,0x22740,0);
    *(undefined1 *)(param_1 + 2) = 3;
    *param_1 = *param_1 & 0xffffffdf;
  }
  uVar4 = param_1[0x43];
  *(undefined2 *)(param_1 + 0x29) = 0;
  *(undefined2 *)((int)param_1 + 0xa6) = 0x3c;
  uVar3 = 0xb;
  if (*(char *)(uVar4 + 8) == '\0') {
    if (*(code **)(uVar4 + 100) == (code *)0x0) { iVar2 = 0; }
    else { iVar2 = (**(code **)(uVar4 + 100))(uVar4,0xc,param_1); }
    uVar3 = 0xb;
    if (iVar2 == 0) { uVar3 = 4; }
  }
  if (*(code **)(uVar4 + 100) != (code *)0x0) {
    (**(code **)(uVar4 + 100))(uVar4,uVar3,param_1);
  }
  return;
}

/* --- SLUS_005.10 FUN_8002305c  (from analysis/SLUS_005.10/decomp/8002305c.c) --- */
// addr: 0x8002305c  name: FUN_8002305c

void FUN_8002305c(uint *param_1)
{
  bool bVar1;
  short sVar2;
  int iVar3;
  undefined2 uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  if (((iRam0000000c - (uint)*(byte *)((int)param_1 + 9) & 0x7f) == 0) &&
     (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,param_1[0x39] + 0x24,0x22740,0);
  }
  *(undefined1 *)((int)param_1 + 0xb2) = 1;
  sVar2 = FUN_80042f98(param_1,param_1 + 0x30,param_1[0x23] * 0x20 + 0x10000);
  iVar5 = (int)sVar2;
  iVar6 = -0x2aa;
  if ((-0x2ab < iVar5) && (iVar6 = 0x2aa, iVar5 < 0x2ab)) {
    iVar6 = iVar5;
  }
  *(short *)(param_1 + 0x29) = (short)iVar6;
  iVar5 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
  if (iVar5 < 0) { iVar5 = iVar5 + 0xfff; }
  iVar3 = (int)(short)param_1[0x2a] + (iVar5 >> 0xc);
  iVar5 = 0;
  if (0 < iVar3) { iVar5 = iVar3; }
  iVar6 = iVar6 * iVar5;
  if (iVar6 < 0) { iVar6 = iVar6 + 0xf; }
  param_1[0x25] = param_1[0x25] + (iVar6 >> 4);
  iVar6 = (int)sVar2;
  if (iVar6 < 0) { iVar6 = -iVar6; }
  if ((iVar6 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
    if ((int)param_1[0x23] < 0x1ad3) {
      iVar6 = 0;
      if (0 < *(short *)((int)param_1 + 0xa6)) { iVar6 = (int)*(short *)((int)param_1 + 0xa6); }
      uVar8 = (uint)(ushort)param_1[0x2b];
      iVar6 = iVar6 + 1;
      bVar1 = iVar6 < (int)uVar8;
      goto LAB_80023248;
    }
    iVar5 = *(short *)((int)param_1 + 0xa6) + -1;
    iVar6 = -(uint)(ushort)param_1[0x2b];
    if ((int)-(uint)(ushort)param_1[0x2b] < iVar5) { iVar6 = iVar5; }
    *(short *)((int)param_1 + 0xa6) = (short)iVar6;
  }
  else {
    iVar6 = 0;
    if (*(short *)((int)param_1 + 0xa6) < 0) { iVar6 = (int)*(short *)((int)param_1 + 0xa6); }
    iVar6 = iVar6 + -1;
    uVar8 = -(uint)(ushort)param_1[0x2b];
    bVar1 = (int)uVar8 < iVar6;
LAB_80023248:
    uVar4 = (undefined2)uVar8;
    if (bVar1) { uVar4 = (undefined2)iVar6; }
    *(undefined2 *)((int)param_1 + 0xa6) = uVar4;
  }
  uVar8 = param_1[*(byte *)((int)param_1 + 0xb3) + 0x44];
  iVar6 = 0;
  bVar1 = false;
  if (uVar8 != 0) { bVar1 = *(short *)(uVar8 + 6) == 0; }
  if ((bVar1) && (iVar5 = FUN_80022e90(param_1), iVar5 == 0)) {
    if (*(code **)(uVar8 + 100) == (code *)0x0) { iVar5 = 0; }
    else { iVar5 = (**(code **)(uVar8 + 100))(uVar8,0xc,param_1); }
    if (iVar5 != 0) { iVar6 = 1; }
  }
  FUN_8002ce68(param_1,iVar6);
  uVar8 = param_1[0x43];
  if (*(char *)(uVar8 + 8) == '\0') {
    if (iVar6 != 0) { uVar7 = 4; goto LAB_80023334; }
    if (*(code **)(uVar8 + 100) == (code *)0x0) { iVar5 = 0; }
    else { iVar5 = (**(code **)(uVar8 + 100))(uVar8,0xc,param_1); }
    uVar7 = 4;
    if (iVar5 == 0) goto LAB_80023334;
  }
  uVar7 = 0xb;
LAB_80023334:
  if (*(code **)(uVar8 + 100) != (code *)0x0) {
    (**(code **)(uVar8 + 100))(uVar8,uVar7,param_1);
  }
  if ((bVar1) && ((iVar6 == 0 || (uVar8 = FUN_80017160(param_1), (uVar8 & 7) == 0)))) {
    FUN_8002cf90(param_1,1);
  }
  return;
}

/* --- SLUS_005.10 FUN_8002346c  (from analysis/SLUS_005.10/decomp/8002346c.c) --- */
// addr: 0x8002346c  name: FUN_8002346c

void FUN_8002346c(uint *param_1)
{
  bool bVar1;
  short sVar2;
  int iVar3;
  code *pcVar4;
  undefined2 uVar5;
  int iVar6;
  uint uVar7;
  undefined4 uVar8;
  int iVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  if (((iRam0000000c - (uint)*(byte *)((int)param_1 + 9) & 0x7f) == 0) &&
     (((short)param_1[0x30] < 1 || ((*param_1 & 0x200000) != 0)))) {
    iVar10 = 0;
    if (((short)param_1[0x3a] == 0) || (iVar10 = FUN_8001ffd4(&DAT_80065a18), iVar10 == 0)) {
      if ((uint)*(ushort *)(param_1[0x3b] + 0xc) + (uint)*(ushort *)(param_1[0x3c] + 0xc) +
          (uint)*(ushort *)(param_1[0x3d] + 0xc) < (uint)(ushort)param_1[3]) {
        iVar10 = FUN_80023394(&DAT_80065a18,0x100000,param_1 + 9);
        uVar8 = 0x400000;
        if (iVar10 == 0) {
LAB_80023570:
          iVar10 = FUN_80023394(&DAT_80065a18,uVar8,param_1 + 9);
          goto LAB_8002357c;
        }
LAB_800235a0:
        *(undefined2 *)(param_1 + 0x3a) = *(undefined2 *)(iVar10 + 6);
      }
      else {
        uVar8 = 0x7f000000;
        if (param_1[0x45] == 0) goto LAB_80023570;
LAB_8002357c:
        if ((iVar10 != 0) ||
           (iVar10 = FUN_80023394(&DAT_80065a18,0x7f780000,param_1 + 9), iVar10 != 0))
        goto LAB_800235a0;
        iVar10 = FUN_80017160();
        iVar6 = FUN_80020120(&DAT_80065a50,0x7f780000);
        iVar10 = FUN_80020190(&DAT_80065a50,0x7f780000,iVar10 * iVar6 >> 0xf);
      }
      if (iVar10 == 0) {
        FUN_80015368("Nowhere to run!");
      }
    }
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,iVar10 + 0x48,0x22740,0);
  }
  *(undefined1 *)((int)param_1 + 0xb2) = 1;
  sVar2 = FUN_80042f98(param_1,param_1 + 0x30,param_1[0x23] * 0x20 + 0x10000);
  iVar10 = (int)sVar2;
  iVar6 = -0x2aa;
  if (-0x2aa < iVar10) { iVar6 = iVar10; }
  iVar9 = 0x2aa;
  if (iVar6 < 0x2aa) { iVar9 = iVar6; }
  *(short *)(param_1 + 0x29) = (short)iVar9;
  iVar6 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
  if (iVar6 < 0) { iVar6 = iVar6 + 0xfff; }
  iVar3 = (int)(short)param_1[0x2a] + (iVar6 >> 0xc);
  iVar6 = 0;
  if (0 < iVar3) { iVar6 = iVar3; }
  iVar6 = (short)iVar9 * iVar6;
  if (iVar6 < 0) { iVar6 = iVar6 + 0xf; }
  param_1[0x25] = param_1[0x25] + (iVar6 >> 4);
  if (iVar10 < 0) { iVar10 = -iVar10; }
  if ((iVar10 < 0x156) || ((int)param_1[0x23] < 0xbec)) {
    if ((int)param_1[0x23] < 0x1ad3) {
      iVar10 = 0;
      if (0 < *(short *)((int)param_1 + 0xa6)) { iVar10 = (int)*(short *)((int)param_1 + 0xa6); }
      uVar11 = (uint)(ushort)param_1[0x2b];
      iVar10 = iVar10 + 1;
      bVar1 = iVar10 < (int)uVar11;
      goto LAB_80023778;
    }
    iVar6 = *(short *)((int)param_1 + 0xa6) + -1;
    iVar10 = -(uint)(ushort)param_1[0x2b];
    if ((int)-(uint)(ushort)param_1[0x2b] < iVar6) { iVar10 = iVar6; }
    *(short *)((int)param_1 + 0xa6) = (short)iVar10;
  }
  else {
    iVar10 = 0;
    if (*(short *)((int)param_1 + 0xa6) < 0) { iVar10 = (int)*(short *)((int)param_1 + 0xa6); }
    iVar10 = iVar10 + -1;
    uVar11 = -(uint)(ushort)param_1[0x2b];
    bVar1 = (int)uVar11 < iVar10;
LAB_80023778:
    uVar5 = (undefined2)uVar11;
    if (bVar1) { uVar5 = (undefined2)iVar10; }
    *(undefined2 *)((int)param_1 + 0xa6) = uVar5;
  }
  uVar11 = param_1[*(byte *)((int)param_1 + 0xb3) + 0x44];
  uVar12 = param_1[0x43];
  if (((uVar11 != 0) && (*(short *)(uVar11 + 6) == 0)) &&
     (iVar10 = FUN_80022e90(param_1), iVar10 == 0)) {
    uVar7 = param_1[0x39];
    iVar10 = param_1[9] - *(int *)(uVar7 + 0x24);
    if (iVar10 < 0) { iVar10 = -iVar10; }
    if (iVar10 < 0x12c000) {
      iVar10 = param_1[10] - *(int *)(uVar7 + 0x28);
      if (iVar10 < 0) { iVar10 = -iVar10; }
      if (iVar10 < 0x12c000) {
        iVar10 = param_1[0xb] - *(int *)(uVar7 + 0x2c);
        if (iVar10 < 0) { iVar10 = -iVar10; }
        if (iVar10 < 0x12c000) {
          if (*(code **)(uVar11 + 100) == (code *)0x0) { iVar10 = 0; }
          else { iVar10 = (**(code **)(uVar11 + 100))(uVar11,0xc,param_1); }
          FUN_8002ce68(param_1,iVar10);
          if ((iVar10 == 0) || (uVar11 = FUN_80017160(param_1), (uVar11 & 7) == 0)) {
            FUN_8002cf90(param_1,1);
          }
          if (*(char *)(uVar12 + 8) == '\0') {
            if (iVar10 == 0) {
              if (*(code **)(uVar12 + 100) == (code *)0x0) { iVar10 = 0; }
              else { iVar10 = (**(code **)(uVar12 + 100))(uVar12,0xc,param_1); }
              uVar8 = 4;
              if (iVar10 != 0) goto LAB_800238e4;
            }
            else { uVar8 = 4; }
          }
          else {
LAB_800238e4:
            uVar8 = 0xb;
          }
          pcVar4 = *(code **)(uVar12 + 100);
          if (pcVar4 == (code *)0x0) { return; }
          goto LAB_8002391c;
        }
      }
    }
  }
  pcVar4 = *(code **)(uVar12 + 100);
  if (pcVar4 == (code *)0x0) { return; }
  uVar8 = 4;
LAB_8002391c:
  (*pcVar4)(uVar12,uVar8,param_1);
  return;
}

/* --- SLUS_005.10 FUN_80022e38  (from analysis/SLUS_005.10/decomp/80022e38.c) --- */
// addr: 0x80022e38  name: FUN_80022e38

void FUN_80022e38(int param_1)
{
  code *pcVar1;
  if (0x3c < iRam0000000c) {
    *(undefined1 *)(param_1 + 8) = 1;
  }
  *(undefined2 *)(param_1 + 0xa4) = 0;
  *(undefined2 *)(param_1 + 0xa6) = 0x3c;
  pcVar1 = *(code **)(*(int *)(param_1 + 0x10c) + 100);
  if (pcVar1 != (code *)0x0) {
    (*pcVar1)(*(int *)(param_1 + 0x10c),0xb);
  }
  return;
}

/* --- SLUS_005.10 FUN_80023940  (from analysis/SLUS_005.10/decomp/80023940.c) --- */
// addr: 0x80023940  name: FUN_80023940

void FUN_80023940(int param_1)
{
  switch(*(undefined1 *)(param_1 + 8)) {
  case 1:
    FUN_80022d54();
    break;
  case 2:
    FUN_8002305c();
    break;
  case 3:
    FUN_8002346c();
    break;
  case 4:
    FUN_80022e38();
  }
  return;
}

#endif  /* GHIDRA REF */
