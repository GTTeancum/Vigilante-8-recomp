/* weapon_projectile_bounce.c -- Bouncing projectile physics tick.
 *
 * Source: SLUS_005.10
 *   FUN_80034b5c  -- WeaponBounce_Tick(self)
 *
 * Physics update for a ballistic/bouncing projectile:
 *   pos += vel; vel *= 15/16 (drag); display_pos = pos;
 *   vel_y += gravity (0x38);
 *   probe terrain; if inside: bounce or retire.
 *
 * HIGH confidence: direct Ghidra ref port.
 */
#include <stdint.h>
#include "structs.h"

extern int      FUN_8001d748(intptr_t obj, int *pos, void *normal_out,
                             uintptr_t *material_out); /* Terrain_HeightProbe */
extern uint32_t FUN_800446dc(const int32_t *pos3d);    /* SfxPan_For3DPos */
extern void     FUN_800443c8(int voice, uint32_t bank, int sfxId,
                               uint32_t pan_and_vol);  /* Audio_PlaySfx_inner */
extern void     FUN_80016fa8(void *mat, const int32_t *normal); /* Matrix_FromNormal */
extern int      FUN_8004410c(void);                    /* Audio_AllocVoice */

extern uint32_t uRam000005f8;   /* audio bank */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* ================================================================
 * FUN_80034b5c  -- WeaponBounce_Tick
 *
 * Returns 0 normally; 1 when the projectile should be destroyed
 * (terrain penetration depth < 0x2fa and still moving).
 * ================================================================ */
uint32_t FUN_80034b5c(int param_1)
{
    int iVar1;
    uint32_t uVar2;
    uint32_t uVar3;
    int32_t auStack_18[2];   /* terrain probe normal out (xy) */

    /* pos += vel */
    *(int32_t *)(uintptr_t)(param_1 + 0x48) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x48),
                      *(int32_t *)(uintptr_t)(param_1 + 0x88));
    *(int32_t *)(uintptr_t)(param_1 + 0x4c) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x4c),
                      *(int32_t *)(uintptr_t)(param_1 + 0x8c));
    *(int32_t *)(uintptr_t)(param_1 + 0x50) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x50),
                      *(int32_t *)(uintptr_t)(param_1 + 0x90));

    /* vel *= 15/16 (drag) */
    *(int32_t *)(uintptr_t)(param_1 + 0x88) =
        mips_subu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x88),
                      *(int32_t *)(uintptr_t)(param_1 + 0x88) >> 4);
    *(int32_t *)(uintptr_t)(param_1 + 0x8c) =
        mips_subu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x8c),
                      *(int32_t *)(uintptr_t)(param_1 + 0x8c) >> 4);
    *(int32_t *)(uintptr_t)(param_1 + 0x90) =
        mips_subu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x90),
                      *(int32_t *)(uintptr_t)(param_1 + 0x90) >> 4);

    /* copy pos to display_pos */
    *(uint32_t *)(uintptr_t)(param_1 + 0x24) = *(uint32_t *)(uintptr_t)(param_1 + 0x48);
    *(uint32_t *)(uintptr_t)(param_1 + 0x28) = *(uint32_t *)(uintptr_t)(param_1 + 0x4c);
    *(uint32_t *)(uintptr_t)(param_1 + 0x2c) = *(uint32_t *)(uintptr_t)(param_1 + 0x50);

    /* apply gravity */
    *(int32_t *)(uintptr_t)(param_1 + 0x8c) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x8c), 0x38);

    /* probe terrain height */
    iVar1 = FUN_8001d748((intptr_t)param_1,
                          (int *)(uintptr_t)(param_1 + 0x48),
                          auStack_18, 0);

    uVar2 = 0;
    if (iVar1 < *(int32_t *)(uintptr_t)(param_1 + 0x4c)) {
        /* below terrain -- bounce */
        uVar3 = FUN_800446dc((const int32_t *)(uintptr_t)(param_1 + 0x48));
        *(int32_t *)(uintptr_t)(param_1 + 0x4c) = iVar1;
        uVar2 = FUN_8004410c();
        FUN_800443c8(uVar2, uRam000005f8, 0x32,
                     (uVar3 >> ((int)*(int16_t *)(uintptr_t)(param_1 + 0x94) & 0x1fu))
                     & ((uint32_t)(0x40004000u >> ((int)*(int16_t *)(uintptr_t)(param_1 + 0x94) & 0x1fu))
                        - 0x10001u));
        *(uint16_t *)(uintptr_t)(param_1 + 0x94) =
            (uint16_t)(*(uint16_t *)(uintptr_t)(param_1 + 0x94) + 1u);
        if (*(int32_t *)(uintptr_t)(param_1 + 0x8c) < 0x2fa) {
            uVar2 = 1;
        } else {
            FUN_80016fa8((void *)(uintptr_t)(param_1 + 0x10), auStack_18);
            *(int32_t *)(uintptr_t)(param_1 + 0x8c) =
                mips_subu_i32(0, *(int32_t *)(uintptr_t)(param_1 + 0x8c)) / 2;
            *(int32_t *)(uintptr_t)(param_1 + 0x88) =
                *(int32_t *)(uintptr_t)(param_1 + 0x88) >> 2;
            *(int32_t *)(uintptr_t)(param_1 + 0x90) =
                *(int32_t *)(uintptr_t)(param_1 + 0x90) >> 2;
            uVar2 = 0;
        }
    }
    return uVar2;
}
