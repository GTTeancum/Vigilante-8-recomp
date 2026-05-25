/* weapon_spawn.c -- Weapon projectile and effect spawn helpers.
 *
 * Source: SLUS_005.10
 *   FUN_80031dfc  -- WeaponShot_SpawnBullet(subBin, owner, kind, pos_ofs)
 *   FUN_80032e48  -- WeaponShot_SpawnShard(subBin, owner, kind)
 *   FUN_8003351c  -- WeaponShot_SpawnMissile(self, owner, kind, flags)
 *   FUN_800346cc  -- WeaponShot_SpawnGuided(self, owner, kind, type, speed)
 *   FUN_800354e0  -- WeaponShot_SpawnGeneric(subBin, owner, kind, tickFn)
 *   FUN_8003e254  -- Weapon_AllocTracker(flags, x, z)
 *   FUN_8003e730  -- WeaponSlot_SpawnEffect(self)
 *
 * HIGH confidence: direct Ghidra ref ports, verified field offsets match
 * weapon-slot struct layout.
 */
#include <stdint.h>
#include "structs.h"

extern void   *FUN_80031300(int parent, int subBin, int kind, uint32_t kindB,
                             int parentSlot);    /* Object_SpawnAttached */
extern uint32_t *FUN_8003fd24(const int32_t *xyz, int arg); /* Debris_AttachTrack (returns alloc'd particle) */
extern void    FUN_80017594(uint32_t *obj, const int32_t *impulse,
                             const int32_t *pos);            /* Object_ApplyImpulse */
extern void    FUN_800202f4(void *obj);                      /* Object_RegisterInScene */
extern void   *FUN_8001ac44(void *pool, int kind, int flags,
                             int align);                     /* BoneBank_AllocByKind */
extern void   *FUN_8001d470(uint32_t size);                  /* Object_AllocSmall */
extern void    FUN_8001dc1c(void *obj);                      /* Object_RandomizeRotation */
extern void    FUN_800207c4(void *obj);                      /* Object_BindFinalize */
extern int     FUN_80016aac(const int32_t *a, const int32_t *b); /* Vec3_Distance */
extern void    FUN_80016da8(int32_t *v);                     /* Vec3_Normalize (variant) */
extern void    FUN_80043358(const void *matIn, const int32_t *src,
                             int32_t *dst);                  /* Matrix_TransformVec */
extern int     FUN_8004410c(void);                           /* Audio_AllocVoice */
extern void    FUN_800447e8(int voice, uint32_t bank,
                             int sfxId, const void *pos3d); /* Audio_Play3D */
extern void    FUN_8002cb7c(int slot);                       /* WeaponSlot_OnChildDestroy */
extern void   *FUN_8001bda0(int *pool, uint32_t kind);       /* WorldReg_AllocSlot */
extern void    FUN_800116f4(uint32_t size);                  /* Heap_AllocOrRetry */

extern uint32_t DAT_800737d8;   /* world-object pool handle  */
extern uint32_t uRam000005f8;   /* global audio bank         */
extern int32_t  DAT_8006576c[3];  /* impulse vector (FUN_80031dfc) */
extern int32_t  DAT_80065778[4];  /* matrix vector (FUN_8003351c)  */
extern int32_t  DAT_80065788[4];  /* matrix vector (FUN_800346cc)  */
extern int32_t  DAT_800657a4[4];  /* impulse vector (FUN_800354e0) */
extern uint32_t DAT_800737d4;     /* world bone-bank           */
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
 * FUN_80031dfc  -- WeaponShot_SpawnBullet
 *
 * Spawns a bullet projectile attached to a weapon's owner.
 * Copies the owner's full bone-matrix (rotation + position),
 * clears flag bit 4 in the projectile's flags word, sets position,
 * applies impulse, plays SFX 0x2f.
 * ================================================================ */
void *FUN_80031dfc(int param_1, int param_2, int16_t param_3)
{
    void *puVar1;
    uint32_t *puVar2;
    uint32_t uVar3, uVar4, uVar5, uVar6, uVar7, uVar8;
    int iVar5;

    puVar1 = FUN_80031300(param_2, param_1, (int)param_3, 0x98, 0);
    puVar2 = (uint32_t *)FUN_8003fd24((const int32_t *)((uintptr_t)puVar1 + 0x48), 2);
    *puVar2 = *puVar2 & 0xffffffefu;

    /* copy bone matrix rotation rows (uint32[8] at +0x10) */
    uVar6 = ((uint32_t *)puVar1)[5];
    uVar7 = ((uint32_t *)puVar1)[6];
    uVar8 = ((uint32_t *)puVar1)[7];
    puVar2[4] = ((uint32_t *)puVar1)[4];
    puVar2[5] = uVar6;
    puVar2[6] = uVar7;
    puVar2[7] = uVar8;
    uVar6 = ((uint32_t *)puVar1)[9];
    uVar7 = ((uint32_t *)puVar1)[10];
    uVar8 = ((uint32_t *)puVar1)[0xb];
    puVar2[8] = ((uint32_t *)puVar1)[8];
    puVar2[9] = uVar6;
    puVar2[10] = uVar7;
    puVar2[0xb] = uVar8;

    ((uint32_t *)puVar1)[0] = 0x800084;
    /* timer: 0x50 normal, 0xa0 if self+0x11c (hard mode flag) set */
    {
        uint16_t uVar4x = 0x50;
        if (*(int16_t *)((uintptr_t)param_2 + 0x11c) != 0) {
            uVar4x = 0xa0;
        }
        *(uint16_t *)((uintptr_t)puVar1 + 0xc) = uVar4x;
    }
    {
        extern void *LAB_80031bbc;
        Object_SetCallbackPsxSlot(puVar1, (uintptr_t)&LAB_80031bbc);
    }
    FUN_800202f4(puVar1);

    /* set initial position from owner's velocity (RTZ >>7) */
    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x80);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    ((uint32_t *)puVar1)[0x22] =
        mips_addu_i32(iVar5 >> 7, mips_sll_i32(*(int16_t *)((uintptr_t)puVar1 + 0x12), 2));

    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x84);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    ((uint32_t *)puVar1)[0x23] =
        mips_addu_i32(iVar5 >> 7, mips_sll_i32(*(int16_t *)((uintptr_t)puVar1 + 0x1a), 2));

    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x88);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    ((uint32_t *)puVar1)[0x24] =
        mips_addu_i32(iVar5 >> 7, mips_sll_i32(*(int16_t *)((uintptr_t)puVar1 + 0x1e), 2));

    *(uint16_t *)((uintptr_t)puVar1 + 0x94) = 0xf0;

    {
        uint32_t uVoice = FUN_8004410c();
        FUN_800447e8(uVoice, uRam000005f8, 0x2f, (uint32_t *)puVar1 + 0x12);
    }
    FUN_80017594((uint32_t *)(uintptr_t)param_2, DAT_8006576c,
                 (int32_t *)((uintptr_t)param_1 + 0x48));
    return puVar1;
}

/* ================================================================
 * FUN_80032e48  -- WeaponShot_SpawnShard
 *
 * Spawns a shard/shrapnel projectile. Sets flags 0x800084, tick
 * LAB_80032aa4. Computes initial position as owner-velocity >>7
 * minus bounding-box half-extents scaled by 0x6d6/4096 (RTZ >>12).
 * Decrements self's ammo counter; destroys slot on zero.
 * ================================================================ */
void *FUN_80032e48(int param_1, int param_2, int16_t param_3)
{
    int16_t sVar1;
    void *puVar2;
    int iVar3, iVar4;
    int32_t local_28, local_24, local_20;

    puVar2 = FUN_80031300(param_2, param_1, (int)param_3, 0x98, 0);
    local_28 = mips_sll_i32(*(int16_t *)((uintptr_t)puVar2 + 0x12), 5);
    local_24 = mips_sll_i32(*(int16_t *)((uint32_t *)puVar2 + 6), 5);
    local_20 = mips_sll_i32(*(int16_t *)((uintptr_t)puVar2 + 0x1e), 5);

    ((uint32_t *)puVar2)[0] = 0x800084;
    {
        extern void *LAB_80032aa4;
        Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_80032aa4);
    }
    FUN_800202f4(puVar2);

    iVar4 = *(int32_t *)(uintptr_t)(param_2 + 0x80);
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x7f);
    iVar3 = mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar2 + 0x12), 0x6d6);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0xfff);
    ((uint32_t *)puVar2)[0x22] = mips_subu_i32(iVar4 >> 7, iVar3 >> 0xc);

    iVar4 = *(int32_t *)(uintptr_t)(param_2 + 0x84);
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x7f);
    iVar3 = mips_mult_lo_i32(*(int16_t *)((uint32_t *)puVar2 + 6), 0x6d6);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0xfff);
    ((uint32_t *)puVar2)[0x23] = mips_subu_i32(iVar4 >> 7, iVar3 >> 0xc);

    iVar4 = *(int32_t *)(uintptr_t)(param_2 + 0x88);
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0x7f);
    iVar3 = mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar2 + 0x1e), 0x6d6);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0xfff);
    ((uint32_t *)puVar2)[0x24] = mips_subu_i32(iVar4 >> 7, iVar3 >> 0xc);

    /* target: owner's lock-on target if set, else owner */
    {
        int iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0xe4);
        if (iVar5 == 0) iVar5 = param_2;
        ((uint32_t *)puVar2)[0x21] = iVar5;
    }

    FUN_80017594((uint32_t *)(uintptr_t)param_2, &local_28,
                 (int32_t *)((uintptr_t)puVar2 + 0x48));

    sVar1 = (int16_t)mips_addu_i32(*(int16_t *)(uintptr_t)(param_1 + 0xc), -1);
    *(int16_t *)(uintptr_t)(param_1 + 0xc) = sVar1;
    if (sVar1 == 0) {
        FUN_8002cb7c(param_1);
    }
    return puVar2;
}

/* ================================================================
 * FUN_8003351c  -- WeaponShot_SpawnMissile
 *
 * Spawns a missile-type projectile. Allocates a bone-bank slot
 * (type 4), attaches via FUN_80031300, sets flags 0x800094 + timer,
 * installs LAB_80033290 as tick. Applies matrix-transformed impulse
 * from DAT_80065778. Decrements ammo; destroys slot on zero.
 * ================================================================ */
void *FUN_8003351c(int param_1, uint32_t *param_2, int16_t param_3,
                   uint16_t param_4)
{
    int16_t sVar1;
    int iVar2;
    void *puVar3;
    uint32_t uVar4, uVar5;
    int32_t auStack_30[4];
    int iVar6;

    iVar6 = *(int32_t *)(uintptr_t)(param_1 + 0x38);
    iVar2 = (int)(uintptr_t)FUN_8001ac44((void *)(uintptr_t)DAT_800737d8, 4, 0x80, 8);
    puVar3 = FUN_80031300((int)(uintptr_t)param_2, iVar6, (int)param_3, 0x98, iVar2);
    ((uint32_t *)puVar3)[0] = 0x800094;
    *(uint16_t *)((uintptr_t)puVar3 + 0xc) = param_4;
    {
        extern void *LAB_80033290;
        Object_SetCallbackPsxSlot(puVar3, (uintptr_t)&LAB_80033290);
    }
    FUN_800202f4(puVar3);
    *(uint16_t *)((uintptr_t)puVar3 + 0x94) = 0x3c;

    uVar5 = param_2[0x20];
    if ((int)uVar5 < 0) uVar5 = (uint32_t)mips_addu_i32((int32_t)uVar5, 0x7f);
    ((uint32_t *)puVar3)[0x22] =
        mips_addu_i32((int32_t)uVar5 >> 7,
                      mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar3 + 0x14), 6));

    uVar5 = param_2[0x21];
    if ((int)uVar5 < 0) uVar5 = (uint32_t)mips_addu_i32((int32_t)uVar5, 0x7f);
    ((uint32_t *)puVar3)[0x23] =
        mips_addu_i32((int32_t)uVar5 >> 7,
                      mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar3 + 0x1a), 6));

    uVar5 = param_2[0x22];
    if ((int)uVar5 < 0) uVar5 = (uint32_t)mips_addu_i32((int32_t)uVar5, 0x7f);
    ((uint32_t *)puVar3)[0x24] =
        mips_addu_i32((int32_t)uVar5 >> 7,
                      mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar3 + 0x1e), 6));

    {
        extern int LAB_8003e80c(int,int,int); /* effect_death_ticks.c */
        Object_SetCallbackPsxSlot((void *)(uintptr_t)iVar2, (uintptr_t)&LAB_8003e80c);
    }
    if ((*param_2 & 4u) == 0) {
        FUN_800207c4((void *)(uintptr_t)iVar2);
    }

    FUN_80043358((void *)(uintptr_t)(iVar6 + 0x10), DAT_80065778, auStack_30);
    FUN_80017594(param_2, auStack_30,
                 (int32_t *)((uintptr_t)param_1 + 0x48));

    uVar4 = FUN_8004410c();
    FUN_800447e8(uVar4, uRam000005f8, 0x30, (uint32_t *)puVar3 + 0x12);

    sVar1 = (int16_t)mips_addu_i32(*(int16_t *)(uintptr_t)(param_1 + 0xc), -1);
    *(int16_t *)(uintptr_t)(param_1 + 0xc) = sVar1;
    if (sVar1 == 0) {
        FUN_8002cb7c(param_1);
    }
    return puVar3;
}

/* ================================================================
 * FUN_800346cc  -- WeaponShot_SpawnGuided
 *
 * Spawns a guided/homing projectile. Allocates bone-bank type 10,
 * attaches, sets flags 0x800490 + timers, installs LAB_8003403c.
 * target = owner->0x39 (lock-on) if non-NULL else owner.
 * Scales initial velocity by distance-clamped factor [0x1000, 0x2000].
 * Applies matrix-transformed impulse from DAT_80065788.
 * ================================================================ */
void *FUN_800346cc(int param_1, uint32_t *param_2, int16_t param_3,
                   uint16_t param_4, uint16_t param_5)
{
    char cVar1;
    uint32_t *puVar2;
    void *puVar3;
    int iVar4, iVar5;
    uint32_t uVar6;
    uint32_t *puVar7;
    int32_t auStack_30[4];
    int iVar8;

    iVar8 = *(int32_t *)(uintptr_t)(param_1 + 0x38);
    puVar2 = (uint32_t *)FUN_8001ac44((void *)(uintptr_t)DAT_800737d8, 10, 0x80, 8);
    puVar3 = FUN_80031300((int)(uintptr_t)param_2, iVar8, (int)param_3, 0x98,
                          (int)(uintptr_t)puVar2);
    *(uint16_t *)((uintptr_t)puVar3 + 10) = param_4;
    ((uint32_t *)puVar3)[0] = 0x800490;
    *(uint16_t *)((uintptr_t)puVar3 + 0xc) = param_5;
    {
        extern void *LAB_8003403c;
        Object_SetCallbackPsxSlot(puVar3, (uintptr_t)&LAB_8003403c);
    }
    FUN_800202f4(puVar3);
    ((uint32_t *)puVar3)[0x25] = 0;

    puVar7 = (uint32_t *)(uintptr_t)param_2[0x39];
    if (puVar7 == NULL) puVar7 = param_2;
    ((uint32_t *)puVar3)[0x21] = (uint32_t)(uintptr_t)puVar7;

    iVar4 = FUN_80016aac((int32_t *)((uint32_t *)puVar3 + 9),
                          (int32_t *)(puVar7 + 9));
    iVar4 = iVar4 >> 9;
    iVar5 = 0x1000;
    if (iVar4 > 0xfff && iVar4 < 0x2001) iVar5 = iVar4;
    else if (iVar4 >= 0x2001) iVar5 = 0x2000;

    uVar6 = param_2[0x20];
    if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
    iVar4 = mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar3 + 0x12), iVar5);
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0xfff);
    ((uint32_t *)puVar3)[0x22] = mips_subu_i32((int32_t)uVar6 >> 7, iVar4 >> 0xc);

    uVar6 = param_2[0x21];
    if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
    iVar4 = mips_mult_lo_i32(*(int16_t *)((uint32_t *)puVar3 + 6), iVar5);
    if (iVar4 < 0) iVar4 = mips_addu_i32(iVar4, 0xfff);
    ((uint32_t *)puVar3)[0x23] = mips_subu_i32((int32_t)uVar6 >> 7, iVar4 >> 0xc);

    uVar6 = param_2[0x22];
    if ((int)uVar6 < 0) uVar6 = (uint32_t)mips_addu_i32((int32_t)uVar6, 0x7f);
    iVar5 = mips_mult_lo_i32(*(int16_t *)((uintptr_t)puVar3 + 0x1e), iVar5);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0xfff);
    ((uint32_t *)puVar3)[0x24] = mips_subu_i32((int32_t)uVar6 >> 7, iVar5 >> 0xc);

    FUN_80016da8((int32_t *)((uint32_t *)puVar3 + 4));

    {
        extern int LAB_8003e80c(int,int,int); /* effect_death_ticks.c */
        Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_8003e80c);
    }
    *puVar2 = *puVar2 | 0x410u;
    if ((*param_2 & 4u) == 0) {
        FUN_800207c4(puVar2);
    }

    FUN_80043358((void *)(uintptr_t)(iVar8 + 0x10), DAT_80065788, auStack_30);
    FUN_80017594(param_2, auStack_30,
                 (int32_t *)((uintptr_t)param_1 + 0x48));

    cVar1 = (char)FUN_8004410c();
    *(char *)((uintptr_t)puVar3 + 5) = cVar1;
    FUN_800447e8((int)cVar1, uRam000005f8, 0x34, (uint32_t *)puVar3 + 0x12);
    return puVar3;
}

/* ================================================================
 * FUN_800354e0  -- WeaponShot_SpawnGeneric
 *
 * Generic weapon object spawn helper. Sets flags from puVar1[0x18]
 * (0x800084 if has sub-object, else 0x800080). Timer: 0x96 normal,
 * 300 if hard-mode. Tick from param_4. Calls FUN_8001dc1c
 * (Object_RandomizeRotation) after registering. Plays SFX 0x31.
 * ================================================================ */
void *FUN_800354e0(int param_1, int param_2, int16_t param_3,
                   void *param_4)
{
    void *puVar1;
    int iVar2;
    uint32_t uVar3;
    int iVar5;

    puVar1 = FUN_80031300(param_2, param_1, (int)param_3, 0x98, 0);
    uVar3 = (((uint32_t *)puVar1)[0x18] != 0) ? 0x800084u : 0x800080u;
    ((uint32_t *)puVar1)[0] = uVar3;

    {
        uint16_t uVar3x = 0x96;
        if (*(int16_t *)(uintptr_t)(param_2 + 0x11c) != 0) uVar3x = 300;
        *(uint16_t *)((uintptr_t)puVar1 + 0xc) = uVar3x;
    }
    Object_SetCallbackPsxSlot(puVar1, (uintptr_t)param_4);
    FUN_800202f4(puVar1);
    FUN_8001dc1c(puVar1);
    ((uint32_t *)puVar1)[0x25] = 0;

    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x80);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    iVar2 = (int)*(int16_t *)((uintptr_t)puVar1 + 0x12);
    if (iVar2 < 0) iVar2 = mips_addu_i32(iVar2, 3);
    ((uint32_t *)puVar1)[0x22] = mips_subu_i32(iVar5 >> 7, iVar2 >> 2);

    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x84);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    iVar2 = (int)*(int16_t *)((uint32_t *)puVar1 + 6);
    if (iVar2 < 0) iVar2 = mips_addu_i32(iVar2, 3);
    ((uint32_t *)puVar1)[0x23] = mips_subu_i32(iVar5 >> 7, iVar2 >> 2);

    iVar5 = *(int32_t *)(uintptr_t)(param_2 + 0x88);
    if (iVar5 < 0) iVar5 = mips_addu_i32(iVar5, 0x7f);
    iVar2 = (int)*(int16_t *)((uintptr_t)puVar1 + 0x1e);
    if (iVar2 < 0) iVar2 = mips_addu_i32(iVar2, 3);
    ((uint32_t *)puVar1)[0x24] = mips_subu_i32(iVar5 >> 7, iVar2 >> 2);

    FUN_80017594((uint32_t *)(uintptr_t)param_2, DAT_800657a4,
                 (int32_t *)((uintptr_t)param_1 + 0x48));
    uVar3 = FUN_8004410c();
    FUN_800447e8(uVar3, uRam000005f8, 0x31, (uint32_t *)puVar1 + 0x12);
    return puVar1;
}

/* ================================================================
 * FUN_8003e254  -- Weapon_AllocTracker
 *
 * Allocates 0x2c bytes, stores param_1 at [0], RTZ-shifts param_2
 * >> 4 into [9], RTZ-shifts param_3 >> 4 into [10].
 * Returns the allocated block.
 * ================================================================ */
void *FUN_8003e254(uint32_t param_1, int param_2, int param_3)
{
    void *puVar1 = FUN_8001d470(0x2c);
    ((uint32_t *)puVar1)[0] = param_1;
    if (param_2 < 0) param_2 = mips_addu_i32(param_2, 0xf);
    ((uint32_t *)puVar1)[9] = param_2 >> 4;
    if (param_3 < 0) param_3 = mips_addu_i32(param_3, 0xf);
    ((uint32_t *)puVar1)[10] = param_3 >> 4;
    return puVar1;
}

/* ================================================================
 * FUN_8003e730  -- WeaponSlot_SpawnEffect
 *
 * Allocates a bone slot (kind 0xd) from the world bank (DAT_800737d4)
 * and passes it to FUN_8003e598 along with param_1.
 * ================================================================ */
extern void FUN_8003e598(int param_1, uint32_t param_2);

void FUN_8003e730(uint32_t param_1)
{
    uint32_t uVar1 = (uint32_t)(uintptr_t)FUN_8001bda0((int *)(uintptr_t)DAT_800737d4, 0xd);
    FUN_8003e598((int)param_1, uVar1);
}
