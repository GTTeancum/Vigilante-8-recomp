/* weapon_script.c -- Weapon script opcode interpreter.
 *
 * Source: SLUS_005.10
 *   FUN_8003f0b4  -- WeaponScript_ExecEntry(scriptBase, entryIdx, owner)
 *
 * Reads a 16-bit opcode word from the script table entry, dispatches
 * on the high nibble (bits 15:12):
 *   0x9xxx  = play SFX (sfxId from LUT)
 *   0x8xxx  = spawn weapon object (subtype from low bits)
 *   0xexxx  = create sub-bone object from pool
 * After dispatching, follows the "next" link (entry[0xd]) recursively.
 *
 * HIGH confidence: direct Ghidra ref port.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "structs.h"

extern void   *FUN_8001ac44(void *pool, int kind, int flags,
                              int align);                 /* BoneBank_AllocByKind */
extern void    FUN_800202f4(void *obj);                   /* Object_RegisterInScene */
extern void    FUN_8001dc1c(void *obj);                   /* Object_RandomizeRotation */
extern void    FUN_8003fd24(const int32_t *xyz, int arg); /* Debris_AttachTrack */
extern void   *FUN_8003fdcc(int32_t *xyz, int kind, int life); /* Debris_PoolAlloc */
extern void   *FUN_8003fea8(int32_t *xyz, uint32_t color); /* Object_SpawnEffect */
extern void    FUN_8003f4a0(int *scriptBase, uint16_t entryIdx,
                              MATRIX *mat, int spawnFlag); /* WeaponScript_ExecEntry_r */
extern uint32_t *FUN_80040c40(uint32_t pool, uint16_t kind, MATRIX *mat,
                               int32_t *anchor, int depth); /* SpawnOrbitObject */
extern int     FUN_8004410c(void);                         /* Audio_AllocVoice */
extern void    FUN_8004483c(int voice, uint32_t bank,
                              int sfxId, const void *pos3d); /* Audio_PlaySfx3D */
extern void    Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

extern uintptr_t DAT_800737d8;    /* world-object pool      */
extern uint32_t  uRam000005f8;    /* audio bank             */
extern int32_t   DAT_80065864[];  /* anchor point (int[3])  */
extern int16_t   DAT_8005ec84[];  /* vehicle info table     */

/* PTR_s_6789__8005ed14: pointer table indexed by (*opcode >> 6 & 0x3c).
 * Each entry points to an SFX-id byte table. */
extern uintptr_t PTR_s_6789__8005ed14[];

/* DAT_8005ece4: SFX opcode → sfxId map for 0x8400..0x8409 range.
 * Indexed as (uint16_t)((*opcode - 0x8400) * 2). */
extern uint16_t  DAT_8005ece4[];

/* DAT_8005ecf8: effect-type table for 0x8800..0x8807.
 * 4-byte entries indexed by (*opcode - 0x8800). */
extern uint32_t  DAT_8005ecf8[];

static int weapon_script_trace_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_WEAPON_SCRIPT");
        cached = (env != NULL && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

/* ================================================================
 * FUN_8003f0b4  -- WeaponScript_ExecEntry
 *
 * param_1: script base (int*; *param_1 = ptr to entry table)
 * param_2: entry index
 * param_3: owner vehicle / weapon object (uint*)
 * ================================================================ */
void FUN_8003f0b4(int *param_1, uint32_t param_2, uint32_t *param_3)
{
    uint16_t uVar1;
    uint16_t uVar2;
    uint32_t *puVar3;
    uint32_t uVar4, uVar5, uVar6, uVar7;
    uint16_t *puVar8;

    /* Entry struct starts at scriptTable + idx*0x1c + 0x1c (skip header) */
    puVar8 = (uint16_t *)((uintptr_t)*param_1 + (param_2 & 0xffff) * 0x1c + 0x1c);
    uVar1  = *puVar8;
    uVar2  = (uint16_t)(uVar1 >> 0xc);

    if (weapon_script_trace_enabled()) {
        fprintf(stderr,
                "v8: weapon_script base=%p table=%p entry=%u opcode=0x%04x owner=%p\n",
                (void *)param_1, (void *)(uintptr_t)*param_1,
                (unsigned)(param_2 & 0xffff), (unsigned)uVar1, (void *)param_3);
    }

    if (uVar2 == 9) {
        /* SFX play: look up sfxId from pointer table */
        {
            int voice = FUN_8004410c();
            uint32_t tableByteOff = ((*puVar8 >> 6) & 0x3cu);
            uint8_t sfxId = *(uint8_t *)(PTR_s_6789__8005ed14[tableByteOff >> 2]
                                         + (*puVar8 & 0xffu));
            FUN_8004483c(voice, uRam000005f8, sfxId, param_3 + 5);
        }
        goto LAB_8003f420;
    }

    if (uVar2 < 10) {
        if (uVar2 != 8) goto LAB_8003f420;

        /* 0x8xxx opcodes */
        if ((uint16_t)(uVar1 + 0x7c00u) < 0xb) {
            /* 0x8400..0x840a: play mapped SFX */
            FUN_8003fd24(param_3 + 5,
                         (int)*(uint16_t *)((uintptr_t)DAT_8005ece4
                                            + (*puVar8 - 0x8400) * 2));
            goto LAB_8003f420;
        }

        if (uVar1 == 0x8500) {
            puVar3 = (uint32_t *)FUN_80040c40(DAT_800737d8, 7, (MATRIX *)(uintptr_t)param_3,
                                               DAT_80065864, 8);
        } else if (uVar1 == 0x8501) {
            puVar3 = (uint32_t *)FUN_80040c40(DAT_800737d8, 0xb, (MATRIX *)(uintptr_t)param_3,
                                               DAT_80065864, 8);
        } else if (uVar1 == 0x8600) {
            FUN_8003fd24(param_3 + 5, 0x24);
            goto LAB_8003f420;
        } else if (uVar1 == 0x8601) {
            FUN_8003fd24(param_3 + 5, 0x25);
            goto LAB_8003f420;
        } else if (uVar1 == 0x8602) {
            FUN_8003fd24(param_3 + 5, 0x26);
            goto LAB_8003f420;
        } else if (uVar1 == 0x8700) {
            puVar3 = (uint32_t *)FUN_8003fdcc(param_3 + 5, 0x27, 100);
            *puVar3 = *puVar3 | 0x10u;
            FUN_8003fd24(param_3 + 5, 0);
            goto LAB_8003f420;
        } else if (uVar1 == 0x8701) {
            FUN_8003fdcc(param_3 + 5, 0x2e, 100);
            FUN_8003fd24(param_3 + 5, 0xc);
            goto LAB_8003f420;
        } else if (uVar1 == 0x8702) {
            puVar3 = (uint32_t *)FUN_8003fdcc(param_3 + 5, 0x29, 200);
            *puVar3 = *puVar3 | 0x10u;
            goto LAB_8003f420;
        } else if (uVar1 == 0x8703) {
            FUN_8003fdcc(param_3 + 5, 0x30, 200);
            goto LAB_8003f420;
        } else if ((uint16_t)(uVar1 + 0x7800u) < 8) {
            /* 0x8800..0x8807: spawn effect from table */
            FUN_8003fea8(param_3 + 5,
                         DAT_8005ecf8[*puVar8 - 0x8800]);
            goto LAB_8003f420;
        } else {
            goto LAB_8003f420;
        }
    } else {
        if (uVar2 != 0xe) goto LAB_8003f420;

        /* 0xexxx: allocate sub-bone object from pool */
        *puVar8 = uVar1 & 0xfffu;
        puVar3 = (uint32_t *)FUN_8001ac44(param_1, param_2 & 0xffff, 0x80, 0);
        *puVar8 = *puVar8 | 0xe000u;
        *(uint8_t *)(puVar3 + 1) = 1;
        {
            extern int LAB_8003ed38(int obj, int event, int param3);
            Object_SetCallbackPsxSlot(puVar3, (uintptr_t)&LAB_8003ed38);
        }
        *puVar3 = *puVar3 | 0x10080u;

        /* copy matrix rows */
        uVar4 = param_3[1]; uVar5 = param_3[2]; uVar6 = param_3[3];
        puVar3[4] = *param_3;
        puVar3[5] = uVar4; puVar3[6] = uVar5; puVar3[7] = uVar6;
        uVar4 = param_3[5]; uVar5 = param_3[6]; uVar6 = param_3[7];
        puVar3[8] = param_3[4];
        puVar3[9] = uVar4; puVar3[10] = uVar5; puVar3[0xb] = uVar6;

        *(uint16_t *)((uintptr_t)puVar3 + 0x42) = 0xc;
        FUN_8001dc1c(puVar3);
    }

    FUN_800202f4(puVar3);

LAB_8003f420:
    /* follow "next" link if present */
    if (puVar8[0xd] != 0xffff) {
        FUN_8003f4a0(param_1, puVar8[0xd], (MATRIX *)(uintptr_t)param_3, 0);
    }
}
