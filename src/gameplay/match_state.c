/* match_state.c -- Match_End (FUN_8002accc) and Match_ResetState (FUN_8002a598).
 *
 * Source: SLUS_005.10
 *   FUN_8002accc  -- Match_End: free per-match HUD resources.
 *   FUN_8002a598  -- Match_ResetState: load HUD.TBL, set up GPU primitives,
 *                    allocate HUD bone banks, init font, load Arms.exp.
 *
 * HIGH confidence on Match_End (12 lines, direct port).
 * MED confidence on Match_ResetState (direct Ghidra port; many GPU primitive
 * structs are renderer-internal and their field names are unknown).
 */
#include <stdint.h>
#include "structs.h"

/* ---- external functions ---- */

extern int        FUN_80015f80(const char *path);        /* Asset_LoadFileOrDie */
extern void       FUN_80045088(int ptr);                 /* Heap_Free */
extern void       FUN_8001bddc(int bank);                /* BoneBank_Free */
extern uint32_t   FUN_8001bda0(uint32_t pool, int kind); /* BoneBank_AllocByIndex */
extern void       FUN_800190d8(uint32_t font);           /* Font_Free */
extern int        FUN_80019034(int fileHandle, int arg); /* Font_Init */
extern void       FUN_800187e4(int src, void *dst);      /* GfxObj_CopyFromFile */
extern void       FUN_80018c3c(void *dst, void *src);    /* GfxObj_Init */
extern void       FUN_80018d64(void *obj, int count);    /* GfxObj_SetupSlots */
extern void       FUN_80018bd0(void *listHead);          /* GfxList_Init */
extern void       FUN_8001910c(void);                    /* ??? (no-arg call) */
extern void       FUN_800159b4(const char *path);        /* Arms_LoadExp */
extern int        FUN_800225d4(uint32_t *outTag, int *ioHandle); /* Archive_ReadNext */
extern int        FUN_8002263c(int handle, int arg);     /* Archive_GetBoneBank */
extern void       FUN_80015bf0(int handle, int arg);     /* Archive_Skip */
extern void       FUN_80015a00(void);                    /* Arms_FinishLoad */
extern void       MargePrim(void *a, void *b);           /* PSY-Q GPU prim link */

/* ---- globals referenced by Match_End ---- */
extern uint32_t  uRam000008b0;          /* active bone-bank handle */
extern uint32_t  DAT_80065b70;          /* HUD bone-slot array [4] (undefined4 *) */
extern uint32_t  uRam0000087c;          /* font handle */

/* ---- globals referenced by Match_ResetState ---- */
extern uint32_t  _DAT_800a2858;         /* packed 4-byte word (CONCAT access) */
extern uint8_t   DAT_800a2858;
extern uint8_t   DAT_800a2859;
extern uint8_t   DAT_800a285a;
extern uint8_t   DAT_800a285b;
extern uint32_t  DAT_800a28a4;          /* HUD gfx object 1 */
extern uint32_t  DAT_800a28ac;          /* HUD gfx object 1 flag word */
extern uint32_t  DAT_800a2914;          /* HUD gfx object copy dst */
extern uint32_t  UNK_800a2910;          /* sentinel / tag */
extern uint32_t  _UNK_800a2910;         /* packed read of UNK_800a2910 */
extern uint32_t  DAT_800a2fb8;          /* HUD primitive block */
extern uint32_t  DAT_800a2fbc;
extern uint32_t  DAT_800a2fc0;
extern uint32_t  DAT_800a2fc4;
extern uint32_t  DAT_800a2fc8;
extern uint32_t  DAT_800a2fcc;
extern uint32_t  DAT_800a2fd0;
extern uint32_t  DAT_800a2fd4;
extern uint32_t  DAT_800a2fd8;
extern uint32_t  DAT_800a2fdc;
extern uint32_t  DAT_800a2fe0;
extern uint32_t  DAT_800a2fe4;
extern uint32_t  DAT_800a2fe8;
extern uint32_t  DAT_800a2fec;
extern uint32_t  uRam000008ac;
extern uint32_t  DAT_800a2988;          /* HUD gfx object 2 */
extern uint32_t  DAT_800a2990;
extern uint32_t  uRam0000089c;
extern uint32_t  UNK_800a2bb8;          /* radar primitive */
extern uint32_t  DAT_800a2bc4;
extern uint32_t  DAT_800a2a30;          /* HUD gfx objects 3/4 */
extern uint32_t  DAT_800a2a38;
extern uint32_t  DAT_800a2ad8;
extern uint32_t  DAT_800a2ae0;
extern uint32_t  DAT_800a2b80;          /* HUD gfx object 5 */
extern uint32_t  DAT_800a2b84;
extern uint32_t  DAT_800a2b88;
extern uint32_t  DAT_800a2b8c;
extern uint32_t  DAT_800a2b90;
extern uint32_t  DAT_800a2b94;
extern uint32_t  DAT_800a2b98;
extern uint32_t  DAT_800a2b9c;
extern uint32_t  DAT_800a2ba0;
extern uint32_t  DAT_800a2ba4;
extern uint32_t  DAT_800a2ba8;
extern uint32_t  DAT_800a2bac;
extern uint32_t  DAT_800a2bb0;
extern uint32_t  DAT_800a2bb4;
extern uint32_t  DAT_800a2828;          /* GPU prim list head */
extern uint32_t  DAT_800a2844;          /* GPU prim structs */
extern uint32_t  DAT_800a2848;
extern uint32_t  DAT_800a284c;
extern uint32_t  DAT_800a2850;
extern uint32_t  DAT_800a2854;
extern uint32_t  DAT_800a2857;
extern uint32_t  DAT_800a2862;
extern uint32_t  DAT_800a2864;          /* GPU prim copy dst */
extern uint32_t  DAT_800a2834;
extern uint8_t   uRam0000085f;
extern uint32_t  uRam00000860;
extern uint8_t   uRam00000867;
extern uint32_t  uRam00000868;
extern uint8_t   uRam00000880;          /* byte from stack: font slot */
extern uint32_t  DAT_8005ea5c;          /* bone kind table [4] */
extern uint32_t  DAT_800737e0;          /* player 1 bone bank ptr */
extern uint32_t  DAT_800737e4;          /* player 2 bone bank ptr */
extern uint32_t  DAT_800737d4;          /* world bone pool */
extern uint8_t   DAT_80065674;          /* player slot index table[0] */
extern uint8_t   DAT_80065675;          /* player slot index [1] */
extern int8_t    cRam00000015;          /* match mode */

/* ================================================================
 * FUN_8002accc  -- Match_End
 *
 * Free all HUD bone banks and the font handle allocated by
 * Match_ResetState.  Called from the main loop at match conclusion.
 * ================================================================ */
void Match_End(void)
{
    uint32_t uVar1;
    uint32_t *puVar2;
    uint32_t uVar3;

    puVar2 = &DAT_80065b70;
    uVar3 = 0;
    FUN_8001bddc((int)uRam000008b0);
    uRam000008b0 = 0;
    do {
        uVar1 = *puVar2;
        puVar2 = puVar2 + 1;
        uVar3 = uVar3 + 1;
        FUN_8001bddc((int)uVar1);
    } while (uVar3 < 4);
    FUN_800190d8(uRam0000087c);
}

/* ================================================================
 * FUN_8002a598  -- Match_ResetState
 *
 * Load "HUD\HUD.TBL", initialise all HUD display-list GPU primitives,
 * allocate 4 HUD bone banks, load "HUD\Font.FNT", then iterate
 * "HUD\Arms.exp" to find and assign character bone banks.
 *
 * MED confidence -- direct Ghidra port; field accesses on DAT_800a2xxx
 * match the GPU primitive layout; names are tentative.
 *
 * PHASE 2 STUB: this function does PSX GPU display-list setup that
 * writes to PSX-RAM-layout globals (DAT_800a28xx..DAT_800a3xxx) which
 * are not contiguous in the host build. The scattered writes cause a
 * segfault. HUD/GPU init is renderer-adjacent (out-of-scope for Phase 2).
 * Stub until Phase 9 when HUD primitives are needed.
 * ================================================================ */
void Match_ResetState(void)
{
#if 0  /* Phase 2 stub -- full body below in #if 0 / Ghidra ref block */
    /* real body follows */
#else
    /* no-op for Phase 2 */
    return;
#endif
}
#if 0  /* PHASE 2 STUB BODY -- real implementation, kept for Phase 9 */
void Match_ResetState_impl(void)
{
    int     iVar3;
    int     iVar11;
    uint32_t *puVar6;
    uint32_t *puVar10;
    int     iVar13;
    uint32_t *puVar14;
    uint32_t *puVar15;
    uint16_t *puVar5;
    uint8_t  *puVar4;
    uint32_t  uVar7, uVar8, uVar9;
    uint32_t  uVar12;
    int      local_2c;
    uint32_t local_30;
    int      local_28[2];
    /* local stack scratch (7 bytes each), used as GfxObj containers */
    uint32_t auStack_50[4];  /* 7 undefined1 bytes aliased as 4 words */
    uint32_t auStack_40[4];
    uint16_t local_48;
    uint16_t local_38;
    uint8_t  uStack_49;

    _DAT_800a2858 = (uint32_t)DAT_800a285b << 24 |
                    (uint32_t)DAT_800a285a << 16 |
                    (uint32_t)DAT_800a2859 <<  8 |
                    (uint32_t)DAT_800a2858;

    iVar3  = FUN_80015f80("HUD\\HUD.TBL");
    iVar11 = 0;
    puVar6  = &DAT_800a28a4;
    puVar10 = &DAT_800a28ac;
    iVar13  = iVar3;
    do {
        FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar13 + 4), auStack_50);
        FUN_80018c3c(puVar6, auStack_50);
        iVar13 = iVar13 + 4;
        iVar11 = iVar11 + 1;
        puVar6  = puVar6 + 7;
        *(uint8_t *)((uint8_t *)puVar10 + 7) |= 2;
        puVar10 = puVar10 + 7;
    } while (iVar11 < 4);

    puVar6  = &DAT_800a28a4 - 1;   /* &DAT_800a28a0 */
    puVar10 = &DAT_800a2914;
    do {
        uVar7 = puVar6[1];
        uVar8 = puVar6[2];
        uVar9 = puVar6[3];
        *puVar10 = *puVar6;
        puVar10[1] = uVar7;
        puVar10[2] = uVar8;
        puVar10[3] = uVar9;
        puVar6  = puVar6 + 4;
        puVar10 = puVar10 + 4;
    } while (puVar6 != (uint32_t *)&UNK_800a2910);
    *puVar10 = _UNK_800a2910;

    FUN_80018d64(&DAT_800a28a4 - 1 /* &DAT_800a28a0 */, 4);
    FUN_80018d64(&DAT_800a2914, 4);
    FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar3 + 0x34), &DAT_800a2fb8);
    uRam000008ac |= 0x20;
    FUN_80018c3c(&DAT_800a2fb8, &DAT_800a2fb8); /* src==dst: self-init? */
    /* WARNING: Read-only address writes (see Ghidra warnings) */
    DAT_800a2fd0 = (uint32_t)(0x10 << 16) | (uint16_t)DAT_800a2fd0;
    DAT_800a2fc4 = DAT_800a2fc4 | 0x2000000;
    DAT_800a2fcc = (DAT_800a2fcc & 0xffffff00u) | uRam000008ac; /* CONCAT31 */
    DAT_800a2fd4 = DAT_800a2fb8;
    DAT_800a2fd8 = DAT_800a2fbc;
    DAT_800a2fdc = DAT_800a2fc0;
    DAT_800a2fe0 = DAT_800a2fc4;
    DAT_800a2fe4 = DAT_800a2fc8;
    DAT_800a2fe8 = DAT_800a2fcc;
    DAT_800a2fec = DAT_800a2fd0;

    FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar3 + 0x38), &DAT_800a2988);
    iVar13  = 0;
    puVar6  = &DAT_800a2988;
    puVar10 = &DAT_800a2990;
    uRam0000089c |= 0x20;
    do {
        FUN_80018c3c(puVar6, &DAT_800a2988);
        *(uint8_t *)((uint8_t *)puVar10 + 7) |= 2;
        iVar13 = iVar13 + 1;
        puVar10 = puVar10 + 0xe;
        *(uint16_t *)((uint8_t *)puVar6 + 0x1a) = 0x10;
        *(int8_t  *)((uint8_t *)puVar6 + 0x15) += (int8_t)(iVar13 - 1) * (int8_t)0x10;
        puVar6[7]  = *puVar6;
        puVar6[8]  = puVar6[1];
        puVar6[9]  = puVar6[2];
        puVar6[10] = puVar6[3];
        puVar6[0xb] = puVar6[4];
        puVar6[0xc] = puVar6[5];
        puVar6[0xd] = puVar6[6];
        puVar6 = puVar6 + 0xe;
    } while (iVar13 < 3);

    /* Radar scratch init (64 entries × 16 bytes) */
    uVar12 = 0;
    puVar4 = (uint8_t *)&UNK_800a2bb8;
    puVar6 = &DAT_800a2bc4;
    do {
        puVar4[3] = 3;
        puVar4[7] = 0x60;
        *puVar6 = 0x20002;
        puVar6 = puVar6 + 4;
        uVar12 = uVar12 + 1;
        puVar4 = puVar4 + 0x10;
    } while (uVar12 < 0x40);

    FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar3 + 0x1c), auStack_50);
    FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar3 + 0x18), auStack_40);
    iVar13  = 0;
    puVar6  = &DAT_800a2a30;
    puVar10 = &DAT_800a2ad8;
    puVar15 = &DAT_800a2a38;
    puVar14 = &DAT_800a2ae0;
    local_48 |= 0x20;
    local_38 |= 0x20;
    do {
        FUN_80018c3c(puVar10, auStack_50);
        FUN_80018c3c(puVar6,  auStack_40);
        *(uint8_t *)((uint8_t *)puVar14 + 7) |= 2;
        iVar13 = iVar13 + 1;
        puVar14 = puVar14 + 0xe;
        *(uint8_t *)((uint8_t *)puVar15 + 7) |= 2;
        puVar10[7]  = *puVar10;
        puVar10[8]  = puVar10[1];
        puVar10[9]  = puVar10[2];
        puVar10[10] = puVar10[3];
        puVar10[0xb] = puVar10[4];
        puVar10[0xc] = puVar10[5];
        puVar10[0xd] = puVar10[6];
        puVar6[7]  = *puVar6;
        puVar6[8]  = puVar6[1];
        puVar6[9]  = puVar6[2];
        puVar6[10] = puVar6[3];
        puVar6[0xb] = puVar6[4];
        puVar6[0xc] = puVar6[5];
        puVar6[0xd] = puVar6[6];
        puVar6  = puVar6 + 0xe;
        puVar10 = puVar10 + 0xe;
        puVar15 = puVar15 + 0xe;
    } while (iVar13 < 3);

    FUN_800187e4(iVar3 + *(int *)((uint8_t *)iVar3 + 0x20), auStack_50);
    local_48 |= 0x20;
    FUN_80018c3c(&DAT_800a2b80, auStack_50);
    DAT_800a2b8c = DAT_800a2b8c | 0x2000000;
    DAT_800a2b9c = DAT_800a2b80;
    DAT_800a2ba0 = DAT_800a2b84;
    DAT_800a2ba4 = DAT_800a2b88;
    DAT_800a2ba8 = DAT_800a2b8c;
    DAT_800a2bac = DAT_800a2b90;
    DAT_800a2bb0 = DAT_800a2b94;
    DAT_800a2bb4 = DAT_800a2b98;
    uRam00000880 = uStack_49;

    FUN_80045088(iVar3);
    FUN_80018bd0(&DAT_800a2828);
    /* GPU primitive packet setup (see Ghidra warnings: writing "read-only" RAM) */
    ((uint8_t *)&DAT_800a2844)[3] = 3;
    ((uint8_t *)&DAT_800a2848)[3] = 0x60;
    DAT_800a2857 = 3;
    ((uint8_t *)&DAT_800a2848)[0] = 0xff;
    ((uint8_t *)&DAT_800a2848)[1] = 0;
    ((uint8_t *)&DAT_800a2848)[2] = 0;
    _DAT_800a2858 = 0x6000ff00;
    DAT_800a2862 = 2;
    ((uint16_t *)&DAT_800a2850)[1] = 2;
    ((uint8_t *)&DAT_800a2834)[3] |= 2;
    MargePrim(&DAT_800a2844, &DAT_800a2854);
    MargePrim(&DAT_800a2828, &DAT_800a2844);

    /* Copy the primitive chain to the shadow buffer */
    puVar6  = &DAT_800a2864;
    puVar10 = &DAT_800a2828;
    do {
        puVar15 = puVar10;
        puVar14 = puVar6;
        uVar7 = puVar15[1];
        uVar8 = puVar15[2];
        uVar9 = puVar15[3];
        *puVar14 = *puVar15;
        puVar14[1] = uVar7;
        puVar14[2] = uVar8;
        puVar14[3] = uVar9;
        puVar6  = puVar14 + 4;
        puVar10 = puVar15 + 4;
    } while (puVar15 + 4 != (uint32_t *)&DAT_800a2858);
    uVar12 = 0;
    uVar7 = puVar15[5];
    uVar8 = puVar15[6];
    puVar14[4] = _DAT_800a2858;
    puVar14[5] = uVar7;
    puVar14[6] = uVar8;

    uRam0000085f = 1;
    uRam00000860 = 0xe1000400;
    uRam00000867 = 1;
    uRam00000868 = 0xe1000400;

    /* Allocate 4 HUD bone banks */
    do {
        puVar5 = (uint16_t *)FUN_8001bda0(DAT_800737d4, (&DAT_8005ea5c)[uVar12]);
        (&DAT_80065b70)[uVar12] = (uint32_t)(uintptr_t)puVar5;
        uVar12 = uVar12 + 1;
        *puVar5 = *puVar5 | 2;
    } while (uVar12 < 4);

    /* Load font */
    uVar7 = (uint32_t)FUN_80015f80("HUD\\Font.FNT");
    uRam0000087c = (uint32_t)FUN_80019034((int)uVar7, 0x22);
    iVar13 = -1;
    FUN_8001910c();
    FUN_800159b4("HUD\\Arms.exp");

    /* Iterate Arms.exp entries to find character bone banks */
    FUN_800225d4(&local_30, local_28);
    local_28[0] = local_2c;

    for (;;) {
        if (local_28[0] == 0) {
            FUN_80015a00();
            return;
        }
        iVar3 = FUN_800225d4(&local_30, local_28);
        if (iVar3 != 0) {
            FUN_80045088(iVar3);
            continue;
        }
        /* Byte-swap tag to check for 'FOXB' (0x584f4246 little-endian) */
        if ((local_30 >> 0x18 |
             (local_30 >> 8 & 0xff00) |
             ((local_30 & 0xff00) << 8) |
             (local_30 << 0x18)) == 0x584f4246u) {
            iVar13 = iVar13 + 1;
            if (iVar13 == (int)(int8_t)DAT_80065674) {
                puVar6 = (uint32_t *)FUN_8002263c(local_2c, 1);
                DAT_800737e0 = (uint32_t)(uintptr_t)puVar6;
                if (cRam00000015 < (int8_t)3) {
                    uRam000008b0 = FUN_8001bda0((uint32_t)(uintptr_t)puVar6, 0);
                    FUN_80015a00();
                    return;
                }
                FUN_80045088((int)*puVar6);
                *puVar6 = 0;
                if (cRam00000015 > (int8_t)2 && iVar13 == (int)(int8_t)DAT_80065675) {
                    DAT_800737e4 = (uint32_t)(uintptr_t)puVar6;
                }
                continue;
            }
            if (cRam00000015 > (int8_t)2 && iVar13 == (int)(int8_t)DAT_80065675) {
                puVar6 = (uint32_t *)FUN_8002263c(local_2c, 1);
                DAT_800737e4 = (uint32_t)(uintptr_t)puVar6;
                FUN_80045088((int)*puVar6);
                *puVar6 = 0;
                continue;
            }
        }
        FUN_80015bf0(local_2c, 1);
    }
}
#endif  /* PHASE 2 STUB BODY */
