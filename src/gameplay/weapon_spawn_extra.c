/* weapon_spawn_extra.c -- additional weapon/effect spawn helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8003fdcc  -- WeaponSpawn_TrailObject (LAB_8003efc8 tick, sub-tick LAB_8003e7b4).
 *   FUN_80040234  -- WeaponSpawn_FireBurst   (LAB_8004007c tick).
 *
 * Each constructs a new object from the world bone-bank
 * (DAT_800737d8) and registers it into the active scene.
 *
 * HIGH confidence (direct Ghidra port).  The LAB_ tick callbacks are
 * implemented in the gameplay/effect modules and installed through the
 * source callback-slot bridge.
 */
#include <stdint.h>

extern void    *FUN_8001d470(uint32_t size);
extern void    *FUN_8001ac44(int *bank, uint16_t idx, uint32_t flags, uint32_t mode);
extern void     FUN_8002036c(int obj);                         /* Object_PostUpdate2 */
extern void     FUN_800202f4(uint32_t *obj);                   /* Object_RegisterInScene */
extern int      FUN_8001db54(int *pos, int radius);            /* GTE frustum visibility check */

extern uintptr_t DAT_800737d8;
extern int32_t  iRam00000010;

extern int  LAB_8003e7b4(int obj, int event, int param3); /* effect_death_ticks.c */
extern void LAB_8003efc8(void);
extern void LAB_8003ee88(void);
extern int  LAB_8004007c(int obj, int event, int param3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* ================================================================
 * FUN_8003fdcc -- WeaponSpawn_TrailObject
 *
 *   param_1: source pos (int[3])
 *   param_2: bone index (ushort)
 *   param_3: trail-length seed (int; 0 = no-trail variant)
 *
 * Allocates a bone-tree object via FUN_8001ac44, sets type byte +0x04
 * = 7, flags = 0x124 (with trail) or 0x104 (without), copies position
 * into +0x48..0x50, installs LAB_8003efc8 as main tick and
 * LAB_8003e7b4 as sub-element tick at +0x64 of the first child, then
 * post-updates.
 * ================================================================ */
uint32_t *FUN_8003fdcc(uint32_t *param_1, uint16_t param_2, int param_3)
{
    uint32_t *puVar1;
    uint32_t  uVar2, uVar3;

    puVar1 = (uint32_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8, param_2, 0x80u, 8);
    *(uint8_t *)(puVar1 + 1) = 7;
    *puVar1 = (param_3 == 0) ? 0x124u : 0x104u;

    /* Mirror the +0x10 matrix translation slot into the position +0x54 cache. */
    puVar1[0x15] = *(uint32_t *)(uintptr_t)(puVar1[0x17] + 0x10);

    uVar2 = param_1[1];
    uVar3 = param_1[2];
    puVar1[0x12] = param_1[0];
    puVar1[0x13] = uVar2;
    puVar1[0x14] = uVar3;
    Object_SetCallbackPsxSlot(puVar1, (uintptr_t)&LAB_8003efc8);

    *(int16_t *)(puVar1 + 3) = (int16_t)(param_3 / 0xc);

    /* Install sub-element tick on the first child object (+0x38 of self). */
    Object_SetCallbackPsxSlot((void *)(uintptr_t)puVar1[0xe], (uintptr_t)&LAB_8003e7b4);

    FUN_8002036c((int)(intptr_t)puVar1);
    return puVar1;
}

/* ================================================================
 * FUN_80040234 -- WeaponSpawn_FireBurst
 *
 *   param_1: world position (int[3]).
 *
 * Allocates a bone-tree object (bone index 0x16, flags 0x80, mode 8),
 * sets type 4 with state 0xb4, copies position, installs LAB_8004007c
 * as tick, post-updates.
 * ================================================================ */
uint32_t *FUN_80040234(uint32_t *param_1)
{
    uint32_t *puVar1;
    uint32_t  uVar2, uVar3;

    puVar1 = (uint32_t *)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8, 0x16u, 0x80u, 8);
    *(uint8_t *)(puVar1 + 1) = 4;
    *puVar1 = 0xb4u;

    uVar2 = param_1[1];
    uVar3 = param_1[2];
    puVar1[0x12] = param_1[0];
    puVar1[0x13] = uVar2;
    puVar1[0x14] = uVar3;
    Object_SetCallbackPsxSlot(puVar1, (uintptr_t)&LAB_8004007c);

    FUN_8002036c((int)(intptr_t)puVar1);
    return puVar1;
}
