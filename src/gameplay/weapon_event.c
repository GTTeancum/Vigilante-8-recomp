/* weapon_event.c -- Weapon object event dispatch callbacks.
 *
 * Source: SLUS_005.10
 *   FUN_80031454  -- WeaponHit_Apply(self, hitTarget, kind, sfxId)
 *   FUN_80031864  -- WeaponObj_Dispatch(self, event, owner)
 *   FUN_80034920  -- WeaponSpray_Dispatch(self, event, owner)
 *   FUN_8003733c  -- MissileTracker_Dispatch(self, event, vehicle)
 *   FUN_8003ce24  -- WeaponTarget_PickRandom(playerFlags)
 *
 * HIGH confidence: direct Ghidra ref ports with field-offset verification.
 */
#include <stdint.h>
#include <stddef.h>
#include "structs.h"

extern void    FUN_800176f8(intptr_t obj, int32_t *vec, int32_t *pos);  /* Object_ApplyImpulseFrom */
extern int     FUN_80012068(int spawnId, int color, int type,
                             int flags);                           /* HudFlash_SetEntry */
extern int     FUN_8004410c(void);                                 /* Audio_AllocVoice */
extern void    FUN_4483c(int voice, uint32_t bank, int sfxId,
                          const void *pos3d);                      /* Audio_PlaySfx3D */
extern void    FUN_8004483c(int voice, uint32_t bank, int sfxId,
                             const void *pos3d);                   /* Audio_PlaySfx3D */
extern void    FUN_8003fd24(const int32_t *xyz, int arg);          /* Debris_AttachTrack */
extern void    FUN_8003c538(uint32_t *proj, uint32_t dist);        /* WeaponProjectile_Dispatch */
extern void    FUN_800205f8(void *obj);                            /* Object_RetireDeferred */
extern void   *FUN_8001ac44(void *pool, int kind, int flags,
                             int align);                           /* BoneBank_AllocByKind */
extern void   *FUN_80031300(int parent, int subBin, int kind,
                             uint32_t kindB, int parentSlot);      /* Object_SpawnAttached */
extern void    FUN_800202f4(void *obj);                            /* Object_RegisterInScene */
extern void    FUN_800207c4(void *obj);                            /* Object_BindFinalize */
extern void    FUN_800447e8(int voice, uint32_t bank,
                             int sfxId, const void *pos3d);        /* Audio_Play3D */
extern void    FUN_800435c0(uint32_t *mat, int32_t *src,
                             int32_t *dst);                        /* Util_TransposeMatRotate */
extern int     FUN_80016aac(const int32_t *a, const int32_t *b);  /* Vec3_Distance */
extern void    FUN_8001d564(void *obj);                            /* Object_DetachFromParent */
extern void    FUN_8001d708(void *obj);                            /* Object_InitBoneMatrix */
extern uint32_t FUN_800446dc(const int32_t *pos3d);                /* SfxPan_For3DPos */
extern void    FUN_80044574(int voice, uint32_t pan);              /* SfxPan_Simple */
extern void    FUN_800441c8(int voice);                            /* Audio_FreeVoice */
extern void    FUN_8001d4f0(void *parent);                         /* Object_AppendChild */
extern void    FUN_80012050(int spawnId, int type);                 /* HudSlot_SetTypeByte */
extern int     FUN_80017160(void);                                 /* V8_RandNext */
extern void    FUN_8002036c(void *obj);                            /* Object_PostUpdate2 */
extern long    ratan2(int y, int x);
extern void    FUN_80043408(const uint32_t *m, const int32_t *v,
                             int32_t *out);                        /* GTE_RotateLongMatTrans */
extern void    FUN_8001f5a0(intptr_t self, intptr_t node);         /* SAT_AxisSetup */
extern void   *FUN_8001ac44_v(void *pool, int kind, int flags,
                               int align);                         /* BoneBank_AllocByKind returning ptr */
extern int     FUN_80025400(int x, int z);

extern uint32_t  DAT_800737d8;    /* world-object pool */
extern uint32_t  uRam000005f8;    /* audio bank        */
extern uint32_t  DAT_80010588;    /* debris particle data table */
extern int32_t   iRam00000758;    /* missile tracker active count */
extern int8_t    cRam00000016;    /* difficulty byte   */

extern void (*pcRam00000730)(void *obj, int eventId, int arg);   /* world event callback */

extern int16_t DAT_8005ec84[];    /* vehicle info table (12 entries × int16) */
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, int shift)
{
    return (int32_t)((uint32_t)v << shift);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* ================================================================
 * FUN_80031454  -- WeaponHit_Apply
 *
 * Called when a weapon projectile collides with a target object.
 * param_1: projectile object ptr (int)
 * param_2: pointer to target object ptr (int*)
 * param_3: effect kind (uint2)
 * param_4: SFX id (positive = play, -1 = suppress)
 *
 * Returns 0 if target is kind==3 (player), -1 otherwise.
 * ================================================================ */
int FUN_80031454(int param_1, int *param_2, uint16_t param_3, int param_4)
{
    int iVar1;
    uint32_t uVar2;
    int iVar3;
    uint32_t uVar4;
    int32_t local_28, local_24, local_20;

    if (param_2 != NULL) {
        iVar3 = *param_2;
        if (*(int8_t *)(uintptr_t)(iVar3 + 4) == 3) {
            return 0;
        }
        iVar1 = param_4 << 0x10;
        if (*(int8_t *)(uintptr_t)(iVar3 + 4) == 2) {
            /* rolling vehicle: scale impulse by ammo/mass ratio */
            uVar4 = ((uint32_t)*(uint16_t *)(uintptr_t)(param_1 + 0xc) << 0xb)
                    / (uint32_t)*(uint16_t *)(uintptr_t)(iVar3 + 0xa2);
            iVar1 = mips_mult_lo_i32(*(int32_t *)(uintptr_t)(param_1 + 0x88), (int32_t)uVar4);
            if (iVar1 < -0x80000)       local_28 = -0x80000;
            else if (iVar1 < 0x80001)   local_28 = iVar1;
            else                         local_28 = 0x80000;

            iVar1 = mips_mult_lo_i32(*(int32_t *)(uintptr_t)(param_1 + 0x8c), (int32_t)uVar4);
            if (iVar1 < -0x80000)       local_24 = -0x80000;
            else if (iVar1 < 0x80001)   local_24 = iVar1;
            else                         local_24 = 0x80000;

            iVar1 = mips_mult_lo_i32(*(int32_t *)(uintptr_t)(param_1 + 0x90), (int32_t)uVar4);
            if (iVar1 < -0x80000)       local_20 = -0x80000;
            else if (iVar1 < 0x80001)   local_20 = iVar1;
            else                         local_20 = 0x80000;

            FUN_800176f8(iVar3, &local_28,
                         (int32_t *)(uintptr_t)(param_1 + 0x24));
            if (*(int16_t *)(uintptr_t)(iVar3 + 6) < 0) {
                FUN_80012068(~(int)*(int16_t *)(uintptr_t)(iVar3 + 6),
                             0xff, 2, 0x80);
            }
            iVar1 = param_4 << 0x10;
            if (*(int16_t *)(uintptr_t)(iVar3 + 0x11e) == 0) {
                param_4 = -1;
            }
        }
        iVar1 = param_4 << 0x10;
    }
    /* LAB_800315c4: */
    if ((iVar1 >> 0x10) >= 0) {
        uVar2 = FUN_8004410c();
        FUN_8004483c(uVar2, uRam000005f8, iVar1 >> 0x10,
                     (void *)(uintptr_t)(param_1 + 0x24));
    }
    FUN_8003fd24((int32_t *)(uintptr_t)(param_1 + 0x24), (int)param_3);
    (*pcRam00000730)((void *)(uintptr_t)param_1, 0x11, 0);
    FUN_800205f8((void *)(uintptr_t)param_1);
    return -1;
}

/* ================================================================
 * FUN_80031864  -- WeaponObj_Dispatch
 *
 * Event dispatch callback for standard weapon objects.
 * param_1: weapon self (int)
 * param_2: event id
 * param_3: owner vehicle (uint*)
 *
 * case 0  = fire (→ WeaponProjectile_Dispatch)
 * case 1  = init: reset speed to 0x500
 * case 4  = tick: decay speed by 0x40, floor 0x500; clear anim
 * case 0xb = charge: spawn explosion burst sub-object
 * case 0xc = aim check: angle to target < 0x70 && in range
 * case 0xe = return HUD flags 0x8010
 * ================================================================ */
uint32_t FUN_80031864(int param_1, uint32_t param_2, uint32_t *param_3)
{
    int8_t cVar1;
    uint32_t *puVar2;
    int32_t local_vec[3];
    int iVar4;
    uint16_t uVar5;
    uint32_t uVar6;
    int iVar7;
    long lVar3;

    switch ((int)param_2) {
    case 0:
        FUN_8003c538((uint32_t *)(uintptr_t)param_1, (uint32_t)(uintptr_t)param_3);
        return 0;
    case 1:
        *(uint16_t *)(uintptr_t)(param_1 + 0xc) = 0x500;
        break;
    case 4:
        iVar7 = mips_subu_i32((int32_t)(uint32_t)*(uint16_t *)(uintptr_t)(param_1 + 0xc), 0x40);
        iVar4 = 0x500;
        if (0x500 < iVar7) iVar4 = iVar7;
        *(int16_t *)(uintptr_t)(param_1 + 0xc) = (int16_t)iVar4;
        *(uint8_t *)(uintptr_t)(param_1 + 5) = 0;
        *(uint8_t *)(uintptr_t)(param_1 + 8) = 0;
        break;
    case 0xb:
        {
            int8_t cVar1b;
            cVar1b = (int8_t)mips_addu_i32(*(int8_t *)(uintptr_t)(param_1 + 8), -1);
            *(int8_t *)(uintptr_t)(param_1 + 8) = cVar1b;
            if (cVar1b != -1) return 0;
        }
        iVar4 = (int)(uintptr_t)FUN_8001ac44((void *)(uintptr_t)DAT_800737d8, 3, 0x80, 8);
        puVar2 = (uint32_t *)FUN_80031300((int)(uintptr_t)param_3, param_1, 4, 0x98, iVar4);
        puVar2[0] = 0x280;
        uVar5 = 7;
        if ((int16_t)param_3[0x47] != 0) uVar5 = 0xe;
        *(uint16_t *)((uint32_t *)puVar2 + 3) = uVar5;
        {
            extern void *LAB_80031634;
            Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_80031634);
        }
        uVar6 = param_3[0x20];
        if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
        puVar2[0x22] = mips_addu_i32((int32_t)uVar6 >> 7,
                                     mips_sll_i32(*(int16_t *)((uint32_t *)puVar2 + 5), 2));

        uVar6 = param_3[0x21];
        if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
        puVar2[0x23] = mips_addu_i32((int32_t)uVar6 >> 7,
                                     mips_sll_i32(*(int16_t *)((uintptr_t)puVar2 + 0x1a), 2));

        uVar6 = param_3[0x22];
        if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
        puVar2[0x24] = mips_addu_i32((int32_t)uVar6 >> 7,
                                     mips_sll_i32(*(int16_t *)((uint32_t *)puVar2 + 8), 2));

        *(uint16_t *)((uintptr_t)puVar2 + 0x94) = 0x2d;
        FUN_800202f4(puVar2);
        {
            extern int LAB_8003e80c(int,int,int); /* effect_death_ticks.c */
            Object_SetCallbackPsxSlot((void *)(uintptr_t)iVar4, (uintptr_t)&LAB_8003e80c);
        }
        if ((*param_3 & 4u) == 0) {
            FUN_800207c4((void *)(uintptr_t)iVar4);
        }

        iVar4 = (int)*(int8_t *)(uintptr_t)(param_1 + 5);
        if (iVar4 == 0) {
            cVar1 = (int8_t)FUN_8004410c();
            *(int8_t *)(uintptr_t)(param_1 + 5) = cVar1;
            iVar4 = (int)cVar1;
        }
        FUN_800447e8(iVar4, uRam000005f8, 0x24,
                     (void *)((uintptr_t)puVar2 + 0x48));
        *(int8_t *)(uintptr_t)(param_1 + 8) =
            (int8_t)((uint16_t)*(uint16_t *)(uintptr_t)(param_1 + 0xc) >> 8);
        *(int16_t *)(uintptr_t)(param_1 + 0xc) =
            (int16_t)mips_addu_i32(*(int16_t *)(uintptr_t)(param_1 + 0xc), 0x20);
        break;
    case 0xc:
        FUN_800435c0(param_3 + 4,
                     (int32_t *)(uintptr_t)((uintptr_t)param_3[0x39] + 0x24),
                     local_vec);
        lVar3 = ratan2(local_vec[0], local_vec[2]);
        iVar4 = (int)((lVar3 << 0x14) >> 0x14);
        if (iVar4 < 0) iVar4 = mips_subu_i32(0, iVar4);
        if (0x70 < iVar4) return 0;
        return (uint32_t)(0x7cfff < local_vec[2]) ^ 1u;
    case 0xe:
        return 0x8010;
    default:
        break;
    }
    return 0;
}

/* ================================================================
 * FUN_80034920  -- WeaponSpray_Dispatch
 *
 * Event dispatch for spray-type weapons (like machine gun / spray).
 * case 0  = fire → WeaponProjectile_Dispatch
 * case 1  = init: set life 10, flags |= 0x4000, anim byte = 4
 * case 9  = fire burst (0x222=single shot, 0x224=burst-5): call
 *           WeaponShot_SpawnGuided, decrement ammo
 * case 0xb = single shot spawn, decrement ammo
 * case 0xc = range check (dist <= 0x1f3fff)
 * case 0xd = return 4
 * case 0xe = return HUD flags 0x8014
 * ================================================================ */
uint32_t FUN_80034920(uint32_t *param_1, uint32_t param_2, uint32_t param_3)
{
    uint16_t uVar1;
    uint32_t uVar2;
    uint32_t *puVar4;
    int iVar5;
    uint32_t uVar6;
    uint32_t uVar3;

    extern void *FUN_800346cc(int, uint32_t *, int16_t, uint16_t, uint16_t);
    extern void  FUN_8002cb7c(int);
    extern int   FUN_8001d5a0(int);
    extern int   FUN_80016aac(const int32_t *, const int32_t *);

    switch ((int)param_2) {
    case 0:
        FUN_8003c538(param_1, param_3);
        uVar6 = 0;
        break;
    case 1:
        *(uint16_t *)((uint32_t *)param_1 + 3) = 10;
        *(uint8_t *)((uint32_t *)param_1 + 2) = 4;
        *param_1 = *param_1 | 0x4000u;
        /* fall through */
    default:
        uVar6 = 0;
        break;
    case 9:
        if ((param_3 & 0xfff) == 0x222) {
            if ((uint16_t)param_1[3] < 2) return 0xffffffff;
            uVar2 = (uint32_t)FUN_8001d5a0((int)(uintptr_t)param_1);
            puVar4 = (uint32_t *)FUN_800346cc((int)(uintptr_t)param_1,
                                               (uint32_t *)(uintptr_t)uVar2,
                                               0x24, 0x1c, 0x25);
            *(uint8_t *)((uint32_t *)puVar4 + 2) = 1;
            *puVar4 = *puVar4 | 0x1000000u;
            uVar1 = (uint16_t)mips_addu_i32((int32_t)(uint32_t)(uint16_t)param_1[3], -2);
            *(uint16_t *)((uint32_t *)param_1 + 3) = uVar1;
            uVar3 = (uint32_t)uVar1;
        } else {
            if ((param_3 & 0xfff) != 0x224) return 0;
            if ((uint16_t)param_1[3] < 2) return 0xffffffff;
            uVar2 = (uint32_t)FUN_8001d5a0((int)(uintptr_t)param_1);
            uVar6 = 5;
            if ((uint16_t)param_1[3] < 5) uVar6 = (uint32_t)(uint16_t)param_1[3];
            puVar4 = (uint32_t *)FUN_800346cc((int)(uintptr_t)param_1,
                                               (uint32_t *)(uintptr_t)uVar2,
                                               0x25, 0x23, (uint16_t)mips_mult_lo_i32((int32_t)uVar6, 0x4b));
            *(uint8_t *)((uint32_t *)puVar4 + 2) = 2;
            *puVar4 = *puVar4 | 0x1000020u;
            uVar6 = (uint32_t)mips_addu_i32((int32_t)(uint32_t)(uint16_t)param_1[3], -5);
            uVar3 = 0;
            if ((int)uVar6 > 0) uVar3 = uVar6;
            *(int16_t *)((uint32_t *)param_1 + 3) = (int16_t)uVar3;
            uVar3 = uVar3 & 0xffff;
        }
        goto LAB_80034adc;
    case 0xb:
        {
            uint32_t uVar2b = (*(int16_t *)(uintptr_t)((uintptr_t)param_3 + 0x11c) == 0)
                              ? 0x4b : 0x96;
            FUN_800346cc((int)(uintptr_t)param_1,
                          (uint32_t *)(uintptr_t)param_3,
                          0x10, 8, (uint16_t)uVar2b);
        }
        uVar1 = (uint16_t)mips_addu_i32((int32_t)(uint32_t)(uint16_t)param_1[3], -1);
        *(uint16_t *)((uint32_t *)param_1 + 3) = uVar1;
        uVar3 = (uint32_t)uVar1;
    LAB_80034adc:
        uVar6 = 0x78;
        if (uVar3 == 0) {
            FUN_8002cb7c((int)(uintptr_t)param_1);
            uVar6 = 0x78;
        }
        break;
    case 0xc:
        iVar5 = FUN_80016aac((int32_t *)((uintptr_t)param_3 + 0x48),
                              (int32_t *)(uintptr_t)(*(int32_t *)(uintptr_t)(param_3 + 0xe4) + 0x48));
        uVar6 = (uint32_t)(0x1f3ffe < (uint32_t)mips_subu_i32(iVar5, 0x1f4001)) ^ 1u;
        break;
    case 0xd:
        uVar6 = 4;
        break;
    case 0xe:
        uVar6 = 0x8014;
    }
    return uVar6;
}

int LAB_80034920(int obj, int event, int param3)
{
    return (int)FUN_80034920((uint32_t *)(uintptr_t)(uint32_t)obj,
                             (uint32_t)event, (uint32_t)param3);
}

/* ================================================================
 * FUN_8003733c  -- MissileTracker_Dispatch
 *
 * Event dispatch for the missile-tracker "hub" object. Manages
 * 16 debris particles (case 1), tracks/follows a target (case 0),
 * handles impact (case 3), and owns iRam00000758 counter.
 * ================================================================ */
uint32_t FUN_8003733c(uint32_t *param_1, uint32_t param_2, int *param_3)
{
    int16_t sVar1;
    int8_t cVar2;
    int iVar3;
    uint32_t uVar4;
    uint32_t *puVar5;
    uint32_t uVar6;
    int iVar7;
    uint32_t uVar8;
    uint32_t *puVar9;
    int32_t auStack_28[4];

    switch ((int)param_2) {
    case 0:
        /* tick: move toward target, update velocity, update audio pan */
        param_1[0x12] = (uint32_t)mips_addu_i32((int32_t)param_1[0x12], (int32_t)param_1[0x22]);
        param_1[0x13] = (uint32_t)mips_addu_i32((int32_t)param_1[0x13], (int32_t)param_1[0x23]);
        param_1[0x14] = (uint32_t)mips_addu_i32((int32_t)param_1[0x14], (int32_t)param_1[0x24]);

        iVar7 = FUN_80025400(param_1[0x12], param_1[0x14]);
        if (mips_subu_i32(iVar7, (int32_t)param_1[0x15]) < (int)param_1[0x13]) {
            param_1[0x13] = (uint32_t)mips_subu_i32(iVar7, (int32_t)param_1[0x15]);
        }

        *(int16_t *)((uintptr_t)param_1 + 0x42) =
            (int16_t)mips_addu_i32(*(int16_t *)((uintptr_t)param_1 + 0x42), 0x88);
        *(int16_t *)((uint32_t *)param_1 + 0x10) =
            (int16_t)mips_addu_i32((int16_t)param_1[0x10], 0x5b);

        if (param_3 != NULL) {
            FUN_8001d708(param_1);
            uVar4 = FUN_800446dc((int32_t *)(param_1 + 9));
            FUN_80044574((int)*(int8_t *)((uintptr_t)param_1 + 5), uVar4);
        }

        iVar7 = mips_subu_i32(*(int32_t *)(uintptr_t)((uintptr_t)param_1[0x21] + 0x48),
                              (int32_t)param_1[0x12]);
        if (iVar7 < 0) iVar7 = mips_addu_i32(iVar7, 0xff);
        uVar6 = (uint32_t)mips_addu_i32((int32_t)param_1[0x22], iVar7 >> 8);
        uVar8 = 0xffffec55u;
        if (-0x13ac < (int)uVar6 && (uVar8 = 0x13abu, (int)uVar6 < 0x13ac)) uVar8 = uVar6;
        param_1[0x22] = uVar8;

        iVar7 = mips_subu_i32(*(int32_t *)(uintptr_t)((uintptr_t)param_1[0x21] + 0x4c),
                              (int32_t)param_1[0x13]);
        if (iVar7 < 0) iVar7 = mips_addu_i32(iVar7, 0xff);
        uVar6 = (uint32_t)mips_addu_i32((int32_t)param_1[0x23], iVar7 >> 8);
        uVar8 = 0xffffec55u;
        if (-0x13ac < (int)uVar6 && (uVar8 = 0x13abu, (int)uVar6 < 0x13ac)) uVar8 = uVar6;
        param_1[0x23] = uVar8;

        iVar7 = mips_subu_i32(*(int32_t *)(uintptr_t)((uintptr_t)param_1[0x21] + 0x50),
                              (int32_t)param_1[0x14]);
        if (iVar7 < 0) iVar7 = mips_addu_i32(iVar7, 0xff);
        uVar6 = (uint32_t)mips_addu_i32((int32_t)param_1[0x24], iVar7 >> 8);
        uVar8 = 0xffffec55u;
        if (-0x13ac < (int)uVar6 && (uVar8 = 0x13abu, (int)uVar6 < 0x13ac)) uVar8 = uVar6;

        sVar1 = *(int16_t *)((uintptr_t)param_1 + 0x96);
        param_1[0x24] = uVar8;
        *(int16_t *)((uintptr_t)param_1 + 0x96) = (int16_t)mips_addu_i32(sVar1, -1);
        if (sVar1 != 1) return 0;

        /* timer expired: detach parent or retire */
        puVar9 = (uint32_t *)(uintptr_t)param_1[0xe];
        if (puVar9 == NULL) {
            FUN_800441c8((int)*(int8_t *)((uintptr_t)param_1 + 5));
            FUN_800205f8(param_1);
            return 0xffffffff;
        }
        FUN_8001d564(puVar9);
        puVar9[0x12] = (uint32_t)mips_addu_i32((int32_t)puVar9[0x12], (int32_t)param_1[0x12]);
        puVar9[0x13] = (uint32_t)mips_addu_i32((int32_t)puVar9[0x13], (int32_t)param_1[0x13]);
        puVar9[0x14] = (uint32_t)mips_addu_i32((int32_t)puVar9[0x14], (int32_t)param_1[0x14]);
        {
            extern int LAB_800372b0(int,int,int); /* effect_death_ticks.c */
            Object_SetCallbackPsxSlot(puVar9, (uintptr_t)&LAB_800372b0);
        }
        *puVar9 = *puVar9 | 0x84u;

        iVar7 = FUN_80017160();
        puVar9[0x22] = (uint32_t)mips_subu_i32(mips_sll_i32(iVar7, 10) >> 0xf, 0x200);
        iVar7 = FUN_80017160();
        puVar9[0x23] = (uint32_t)mips_subu_i32(0, mips_addu_i32(mips_sll_i32(iVar7, 10) >> 0xf, 0x400));
        iVar7 = FUN_80017160();
        puVar9[0x24] = (uint32_t)mips_subu_i32(mips_sll_i32(iVar7, 10) >> 0xf, 0x200);
        *(uint8_t *)((uintptr_t)puVar9 + 0x86) = 0xf0;
        FUN_8002036c(param_1);
        *(uint16_t *)((uintptr_t)param_1 + 0x96) = 0xf;
        break;

    case 1:
        /* spawn 16 debris sub-particles */
        iVar7 = 0;
        do {
            puVar5 = (uint32_t *)FUN_8001ac44((void *)(uintptr_t)param_1[0x16],
                                               0x3a, 0x94, 8);
            iVar3 = FUN_80017160();
            puVar5[0x12] = (uint32_t)mips_addu_i32(mips_sll_i32(iVar3, 0x10) >> 0xf, -0x8000);
            iVar3 = FUN_80017160();
            puVar5[0x13] = (uint32_t)mips_addu_i32(mips_sll_i32(iVar3, 0x10) >> 0xf, -0x8000);
            iVar3 = FUN_80017160();
            iVar7 = mips_addu_i32(iVar7, 1);
            puVar5[0x14] = (uint32_t)mips_addu_i32(mips_sll_i32(iVar3, 0x10) >> 0xf, -0x8000);
            puVar5[9]  = puVar5[0x12];
            puVar5[10] = puVar5[0x13];
            puVar5[0xb] = puVar5[0x14];
            *puVar5 = 0x410;
            iVar3 = FUN_80017160();
            *(int8_t *)((uintptr_t)puVar5 + 0x86) =
                (int8_t)mips_addu_i32(mips_sll_i32(iVar3, 3) >> 0xf, 0x18);
            FUN_8001d4f0(param_1);
        } while (iVar7 < 0x10);

        param_1[0x17] = (uint32_t)(uintptr_t)&DAT_80010588;
        param_1[0x15] = 0x8000;
        cVar2 = (int8_t)FUN_8004410c();
        *(int8_t *)((uintptr_t)param_1 + 5) = cVar2;
        FUN_800447e8((int)cVar2,
                     *(uint32_t *)(uintptr_t)(param_1[0x16] + 8),
                     0, param_1 + 0x12);
        iRam00000758 = mips_addu_i32(iRam00000758, 1);
        break;

    case 3:
        /* impact: apply impulse, terrain probe, set velocity for sub-object */
        FUN_8001f5a0((intptr_t)param_1, (intptr_t)param_3);
        FUN_80043408(param_1 + 4, param_3 + 5, auStack_28);
        FUN_8003fd24(auStack_28, 0x17);

        {
            int nx = (int)((uint32_t)*(uint16_t *)((uint32_t *)param_3 + 8) << 0x10);
            param_1[0x22] = (uint32_t)mips_addu_i32((int32_t)param_1[0x22],
                mips_subu_i32(nx >> 0x10, nx >> 0x1f) >> 1);
        }
        {
            int iVar7b = (int)((uint32_t)*(uint16_t *)((uintptr_t)param_3 + 0x22) << 0x10);
            param_1[0x23] = (uint32_t)mips_addu_i32((int32_t)param_1[0x23],
                mips_subu_i32(iVar7b >> 0x10, iVar7b >> 0x1f) >> 1);
        }
        {
            int nz = (int)((uint32_t)*(uint16_t *)((uint32_t *)param_3 + 9) << 0x10);
            param_1[0x24] = (uint32_t)mips_addu_i32((int32_t)param_1[0x24],
                mips_subu_i32(nz >> 0x10, nz >> 0x1f) >> 1);
        }

        iVar7 = *param_3;
        if (*(int8_t *)(uintptr_t)(iVar7 + 4) != 2) return 0;

        *(int32_t *)(uintptr_t)(iVar7 + 0x80) = *(int32_t *)(uintptr_t)(iVar7 + 0x80) >> 2;
        {
            extern int FUN_8001d748(intptr_t obj, int *pos, void *normal_out,
                                    uintptr_t *material_out);
            iVar3 = FUN_8001d748((intptr_t)iVar7, (int *)(uintptr_t)(iVar7 + 0x48), NULL, NULL);
        }
        if (mips_addu_i32(iVar3, -0xf000) < *(int32_t *)(uintptr_t)(iVar7 + 0x28)) {
            *(int32_t *)(uintptr_t)(iVar7 + 0x84) = -0x60f580;  /* 0xfffa0a80 as int */
        }
        *(int32_t *)(uintptr_t)(iVar7 + 0x88) = *(int32_t *)(uintptr_t)(iVar7 + 0x88) >> 2;

        {
            uint32_t uVoice = FUN_8004410c();
            FUN_800447e8(uVoice, uRam000005f8, 0x3e,
                         (void *)(uintptr_t)(param_1[0x12]));
        }
        if (*(int16_t *)(uintptr_t)(iVar7 + 6) < 0) {
            FUN_80012050(~(int)*(int16_t *)(uintptr_t)(iVar7 + 6), 8);
        }
        if ((*param_1 & 0x10000u) != 0) return 0;
        *param_1 = *param_1 | 0x10000u;
        *(uint16_t *)((uintptr_t)param_1 + 0x96) = 0xf0;
        break;

    case 4:
        iRam00000758 = mips_addu_i32(iRam00000758, -1);
        break;

    case 9:
        if ((int *)(uintptr_t)param_1[0x21] != param_3) return 0;
        param_1[0x21] = param_1[0x20];
        break;
    }
    return 0;
}

/* ================================================================
 * FUN_8003ce24  -- WeaponTarget_PickRandom
 *
 * Picks a random valid vehicle target from the active-vehicle table.
 * param_1: bitmask of allowed vehicles (bit i set = vehicle i eligible)
 * cRam00000016: difficulty byte (0=hard, max 2 = easy retries)
 *
 * Returns vehicle index 0..0xb, or 0xb (last) on max-retries.
 *
 * Algorithm:
 *   iVar2 = ~cRam00000016 + 3 = (3 - difficulty) retries
 *   Each iteration: pick random index in_v1 = (rnd*12) >> 15
 *   Accept if:
 *     1. DAT_8005ec84[in_v1] >= 0 (vehicle slot active)
 *     2. param_1 has bit (in_v1 + 0x13) & 0x1f set
 *   If accepted and in_v1 == 0xb: return 0xb immediately.
 *   Else decrement retry and if param_1 & 0x40000000 nonzero: retry.
 * ================================================================ */
uint32_t FUN_8003ce24(uint32_t param_1)
{
    int iVar1;
    uint32_t in_v1;
    int iVar2;

    iVar2 = mips_addu_i32(~(int)cRam00000016, 3);
    while (1) {
        if (iVar2 == -1) return in_v1;
        do {
            do {
                iVar1 = FUN_80017160();
                in_v1 = (uint32_t)(mips_mult_lo_i32(iVar1, 0xc) >> 0xf);
            } while (*(int16_t *)((uintptr_t)DAT_8005ec84 + in_v1 * 2) < 0);
        } while ((param_1 & (1u << ((in_v1 + 0x13) & 0x1f))) == 0);
        if (in_v1 == 0xb) break;
        iVar2 = mips_addu_i32(iVar2, -1);
        if ((param_1 & 0x40000000u) == 0) return in_v1;
    }
    return 0xb;
}
