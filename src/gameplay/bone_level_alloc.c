/* bone_level_alloc.c -- Bone-level GPU-packet builder.
 *
 * Source: SLUS_005.10
 *   FUN_8001b49c  Bone_AllocLevel(bank, key)   (~80 instr binary, ~200 lines pseudoC)
 *   FUN_8001b36c  BoneAnim_InitSlot(chain, idx)  (~15 instr)
 *
 * Bone_AllocLevel:
 *   Given a bone-bank `bank` and entry-index `key`, allocates and fills
 *   an in-memory bone-level object.  The level struct (puVar3) has:
 *     +0x00  flags / refcount word
 *     +0x02  total primitive data size (u16)
 *     +0x04..+0x1b  copy of bone entry header (fields [0..5])
 *     +0x1c  primitive count (u16, = *(u16*)(entry+8))
 *     +0x1e  pointer to primitive data buffer
 *     ... etc (full struct defined in include/structs.h)
 *
 *   A 14-case switch builds PSX GPU primitive packets into the alloc'd
 *   buffer from the bone entry's inline primitive list.
 *
 *   DAT_800568fc / DAT_800568fe: u16 stride tables indexed by (primType*4).
 *   Each entry gives src-stride and dst-stride for the corresponding prim type.
 *
 * BoneAnim_InitSlot (FUN_8001b36c):
 *   Like BoneAnim_TouchSlot (FUN_8001b3d4) but non-incrementing: if the
 *   slot ref-count is 0, calls FUN_800187e4 to initialise it.
 *
 * HIGH confidence: direct port from Ghidra pseudoC.
 */
#include <stdint.h>
#include <stdbool.h>

/* ---- dependencies ---- */
extern void    *FUN_800116f4(uint32_t size);     /* Heap_AllocOrRetry */
extern int     *FUN_8001b3d4(int *param_1, uint param_2); /* BoneAnim_TouchSlot */
extern void     FUN_800187e4(uint32_t src, void *dst);    /* Font_DecodeFNT */

/* Stride tables for GPU primitive packing (one entry per primType, step 4 bytes):
 *   DAT_800568fc[i*2] = source stride (how many bytes to advance src ptr)
 *   DAT_800568fe[i*2] = dest   stride (how many bytes to advance dst ptr)
 * Actual base is: &DAT_800568fc for src, &DAT_800568fe for dst.
 * Both referenced as u16 arrays offset by the computed index. */
extern uint16_t DAT_800568fc[];
extern uint16_t DAT_800568fe[];

extern uint8_t  DAT_80065a28[];  /* default/null animation slot */
extern uint8_t  uRam00000854;    /* ambient R */
extern uint8_t  uRam00000855;    /* ambient G */
extern uint8_t  uRam00000856;    /* ambient B */

/* ================================================================
 * FUN_8001b36c -- BoneAnim_InitSlot
 * ================================================================ */
int *FUN_8001b36c(int *param_1, uint param_2)
{
    int *piVar1;
    piVar1 = param_1 + (param_2 & 0xffffu) * 3 + 3;
    if ((short)*piVar1 == 0) {
        uintptr_t tmpl = (uintptr_t)*(uint32_t *)param_1;
        FUN_800187e4(
            *(uint32_t *)((uintptr_t)*(uint32_t *)(tmpl + 0x14)
                          + (param_2 & 0xffffu) * 4),
            piVar1);
    }
    return piVar1;
}

/* ================================================================
 * FUN_8001b49c -- Bone_AllocLevel
 * ================================================================ */
ushort *FUN_8001b49c(int *param_1, uint param_2)
{
    byte       bVar1;
    bool       bVar2;
    ushort    *puVar3;
    uint32_t   uVar4;
    int        iVar5;
    uint32_t  *puVar6;
    uint8_t   *puVar7;
    short      sVar8;
    ushort     uVar9;
    uint       uVar10;
    uint       uVar11;
    ushort    *puVar12;
    char      *pcVar13;
    uintptr_t  iVar14;
    uint      *puVar15;
    uint      *puVar16;
    int        iVar17;
    uint32_t  *puVar18;
    int        local_34;

    uintptr_t tmpl = (uintptr_t)*(uint32_t *)param_1;
    uintptr_t group_table = (uintptr_t)*(uint32_t *)(tmpl + 4);
    puVar18 = (uint32_t *)(uintptr_t)*(uint32_t *)(group_table + (param_2 & 0xffffu) * 4);
    puVar3  = (ushort *)FUN_800116f4((uint)*(byte *)((uintptr_t)puVar18 + 0x19) * 4 + 0x2c);
    iVar17  = 0;
    local_34 = 0;
    bVar2   = false;
    puVar12 = puVar3;

    if (*(char *)((uintptr_t)puVar18 + 0x19) != '\0') {
        do {
            uVar4 = (uint32_t)(uintptr_t)FUN_8001b36c(param_1, 0);
            *(uint32_t *)((uint8_t *)puVar12 + 0x2c) = uVar4;
            local_34++;
            puVar12 += 2;
        } while (local_34 < (int)(uint)*(byte *)((uintptr_t)puVar18 + 0x19));
    }

    local_34 = 0;
    iVar14   = (uintptr_t)puVar18[5];

    if (*(ushort *)((uint8_t *)puVar18 + 8) != 0) {
        do {
            uVar11 = *(byte *)((uint8_t *)(uintptr_t)iVar14 + 3) >> 2 & 0xfu;
            if (uVar11 == 10) {
                iVar17  += (uint)*(ushort *)((uint8_t *)(uintptr_t)iVar14 + 10) * 0x28;
                iVar14  += (uint)*(ushort *)((uint8_t *)(uintptr_t)iVar14 + 10) * 4;
            }
            iVar5     = (int)(uVar11 * 4);
            local_34++;
            iVar17   += (uint)DAT_800568fe[iVar5 / 2];
            iVar14   += (uint)DAT_800568fc[iVar5 / 2];
        } while (local_34 < (int)(uint)*(ushort *)((uint8_t *)puVar18 + 8));
    }

    *puVar3     = 0;
    puVar3[1]   = (ushort)iVar17;
    *(uint32_t *)(puVar3 + 2) = puVar18[0];
    *(uint32_t *)(puVar3 + 4) = puVar18[1];
    *(uint32_t *)(puVar3 + 6) = puVar18[2];
    *(uint32_t *)(puVar3 + 8) = puVar18[3];
    *(uint  *)(puVar3 + 10)   = (uint)*(ushort *)((uint8_t *)puVar18 + 8);
    *(uint32_t *)(puVar3 + 0xc) = puVar18[5];
    puVar6 = (uint32_t *)FUN_800116f4((uint32_t)iVar17);
    *(uint32_t *)(puVar3 + 0xe) = (uint32_t)(uintptr_t)puVar6;
    puVar3[0x10] = 0;
    puVar3[0x11] = 0;
    bVar1        = *(byte *)((uint8_t *)puVar18 + 0x18);
    puVar3[0x14] = 0;
    puVar3[0x13] = (ushort)bVar1;
    puVar3[0x15] = *(ushort *)((uint8_t *)puVar18 + 0x12);
    puVar3[0x12] = *(ushort *)((uint8_t *)puVar18 + 0x1a);
    local_34 = 0;
    puVar15 = (uint *)(uintptr_t)puVar18[5];

    if (*(short *)((uint8_t *)puVar18 + 8) != 0) {
        do {
            if ((*puVar15 & 0x80000000u) != 0) {
                *(uint8_t *)puVar15       = uRam00000854;
                *(uint8_t *)((uint8_t *)puVar15 + 1) = uRam00000855;
                *(uint8_t *)((uint8_t *)puVar15 + 2) = uRam00000856;
            }
            puVar16 = puVar15;
            switch (*(uint8_t *)((uint8_t *)puVar15 + 3) >> 2 & 0xfu) {
            case 0:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x4000000u;
                uVar11  = uVar11 & 0xffffffu;
                uVar10  = (bVar1 & 3u) | 0x20u;
                goto LAB_8001bb68;
            case 1:
                iVar17 = (int)(uintptr_t)FUN_8001b3d4(param_1,
                            (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) +
                                   (int)(*(ushort *)((uint8_t *)puVar15 + 0x12) & 0x3fffu)));
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x9000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x34u) << 0x18;
                puVar6[4] = puVar15[5];
                puVar6[7] = puVar15[6];
                *(uint16_t *)((uint8_t *)puVar6 + 0xe) = *(uint16_t *)((uint8_t *)(uintptr_t)iVar17 + 10);
                *(ushort *)((uint8_t *)puVar6 + 0x1a) =
                    *(ushort *)((uint8_t *)(uintptr_t)iVar17 + 8) |
                    (ushort)((*(ushort *)((uint8_t *)puVar15 + 0x12) & 0xc000u) >> 9);
                *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                *(short *)(puVar6 + 6) = *(short *)((uint8_t *)puVar15 + 0xe) + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                sVar8 = (short)puVar15[4] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                goto LAB_8001ba20;
            case 2:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x6000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x30u) << 0x18;
                puVar6[3] = puVar15[3];
                puVar6[5] = puVar15[4];
                break;
            case 3:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x9000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x34u) << 0x18;
                puVar6[4] = puVar15[5];
                puVar6[7] = puVar15[6];
                break;
            case 4:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                bVar2  = true;
                *puVar6 = 0x4000000u;
                uVar11  = uVar11 & 0xffffffu;
                uVar10  = (bVar1 & 3u) | 0x20u;
                goto LAB_8001bb68;
            case 5:
                iVar17 = (int)(uintptr_t)FUN_8001b3d4(param_1,
                            (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) +
                                   (int)(*(ushort *)((uint8_t *)puVar15 + 0x12) & 0x3fffu)));
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x7000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x24u) << 0x18;
                *(uint16_t *)((uint8_t *)puVar6 + 0xe) = *(uint16_t *)((uint8_t *)(uintptr_t)iVar17 + 10);
                *(ushort *)((uint8_t *)puVar6 + 0x16) =
                    *(ushort *)((uint8_t *)(uintptr_t)iVar17 + 8) |
                    (ushort)((*(ushort *)((uint8_t *)puVar15 + 0x12) & 0xc000u) >> 9);
                *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                *(short *)(puVar6 + 5) = *(short *)((uint8_t *)puVar15 + 0xe) + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                bVar2 = true;
                *(short *)(puVar6 + 7) = (short)puVar15[4] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                break;
            case 6:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x3000000u;
                uVar11  = uVar11 & 0xffffffu;
                uVar10  = (bVar1 & 3u) | 0x40u;
                goto LAB_8001bb68;
            case 7:
                uVar10 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                bVar2  = true;
                *puVar6 = 0x7000000u;
                uVar11  = (uint)((bVar1 & 3u) | 0x24u);
                goto LAB_8001bcfc;
            case 8:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                bVar2  = true;
                *puVar6 = 0x6000000u;
                uVar11  = uVar11 & 0xffffffu;
                uVar10  = (bVar1 & 3u) | 0x30u;
                goto LAB_8001bb68;
            case 9:
                iVar17 = (int)(uintptr_t)FUN_8001b3d4(param_1,
                            (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) +
                                   (int)(*(ushort *)((uint8_t *)puVar15 + 0x16) & 0x3fffu)));
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x9000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x34u) << 0x18;
                *(uint16_t *)((uint8_t *)puVar6 + 0xe) = *(uint16_t *)((uint8_t *)(uintptr_t)iVar17 + 10);
                *(ushort *)((uint8_t *)puVar6 + 0x1a) =
                    *(ushort *)((uint8_t *)(uintptr_t)iVar17 + 8) |
                    (ushort)((*(ushort *)((uint8_t *)puVar15 + 0x16) & 0xc000u) >> 9);
                *(short *)(puVar6 + 3) = (short)puVar15[4] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                *(short *)(puVar6 + 6) = *(short *)((uint8_t *)puVar15 + 0x12) + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
                bVar2 = true;
                sVar8 = (short)puVar15[5] + *(short *)((uint8_t *)(uintptr_t)iVar17 + 6);
            LAB_8001ba20:
                *(short *)(puVar6 + 9) = sVar8;
                break;
            case 10:
                iVar17 = 0;
                if (*(short *)((uint8_t *)puVar15 + 10) != 0) {
                    pcVar13 = (char *)((uint8_t *)puVar6 + 0x25);
                    do {
                        iVar14 = (int)(uintptr_t)FUN_8001b3d4(param_1,
                                    (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) +
                                           (int)(*(ushort *)((uint8_t *)puVar16 + 0xe) & 0x3fffu)));
                        pcVar13[-0x1e] = '/';
                        *(ushort *)(pcVar13 - 0xf) = *(ushort *)((uint8_t *)(uintptr_t)iVar14 + 8) | 0x20u;
                        *(uint16_t *)(pcVar13 - 0x17) = *(uint16_t *)((uint8_t *)(uintptr_t)iVar14 + 10);
                        pcVar13[-0x19] = *(char *)((uint8_t *)(uintptr_t)iVar14 + 6);
                        pcVar13[-0x18] = (char)((ushort)*(uint16_t *)((uint8_t *)(uintptr_t)iVar14 + 6) >> 8);
                        pcVar13[-0x11] = *(char *)((uint8_t *)(uintptr_t)iVar14 + 2) + *(char *)((uint8_t *)(uintptr_t)iVar14 + 6) - 1;
                        pcVar13[-0x10] = (char)((ushort)*(uint16_t *)((uint8_t *)(uintptr_t)iVar14 + 6) >> 8);
                        pcVar13[-9]    = *(char *)((uint8_t *)(uintptr_t)iVar14 + 6);
                        pcVar13[-8]    = *(char *)((uint8_t *)(uintptr_t)iVar14 + 4) +
                                         (char)((ushort)*(uint16_t *)((uint8_t *)(uintptr_t)iVar14 + 6) >> 8) - 1;
                        pcVar13[-1]    = *(char *)((uint8_t *)(uintptr_t)iVar14 + 2) + *(char *)((uint8_t *)(uintptr_t)iVar14 + 6) - 1;
                        *pcVar13       = *(char *)((uint8_t *)(uintptr_t)iVar14 + 4) +
                                         (char)((ushort)*(uint16_t *)((uint8_t *)(uintptr_t)iVar14 + 6) >> 8) - 1;
                        puVar16++;
                        iVar17++;
                        puVar6   += 10;
                        pcVar13 += 0x28;
                    } while (iVar17 < (int)(uint)*(ushort *)((uint8_t *)puVar15 + 10));
                }
                puVar16 = puVar15 + *(ushort *)((uint8_t *)puVar15 + 10);
                break;
            case 0xb:
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                bVar2  = true;
                *puVar6 = 0x9000000u;
                uVar11  = uVar11 & 0xffffffu;
                uVar10  = (bVar1 & 3u) | 0x34u;
            LAB_8001bb68:
                puVar6[1] = uVar11 | (uVar10 << 0x18);
                break;
            case 0xc:
                uVar9 = *(ushort *)((uint8_t *)puVar15 + 0x10) & 0x3fffu;
                if (uVar9 == 0x3fffu) {
                    puVar7 = DAT_80065a28;
                    *(uint8_t *)((uint8_t *)puVar15 + 0x13) = 0;
                } else {
                    puVar7 = (uint8_t *)(uintptr_t)FUN_8001b3d4(param_1,
                                (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) + (int)uVar9));
                }
                uVar11 = *puVar15;
                *(char *)((uint8_t *)puVar15 + 0x12) = (char)((ushort)*(uint16_t *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 6) >> 8);
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x7000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x24u) << 0x18;
                *(uint16_t *)((uint8_t *)puVar6 + 0xe) = *(uint16_t *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 10);
                *(ushort *)((uint8_t *)puVar6 + 0x16) =
                    *(ushort *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 8) |
                    (ushort)((*(ushort *)((uint8_t *)puVar15 + 0x10) & 0xc000u) >> 9);
                break;
            case 0xd:
                if (*(ushort *)((uint8_t *)puVar15 + 0x12) == 0xffffu) {
                    puVar7 = DAT_80065a28;
                } else {
                    puVar7 = (uint8_t *)(uintptr_t)FUN_8001b3d4(param_1,
                                (uint)((short)*(ushort *)((uint8_t *)puVar18 + 0x12) +
                                       (int)(*(ushort *)((uint8_t *)puVar15 + 0x12) & 0x3fffu)));
                }
                uVar11 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x7000000u;
                puVar6[1] = (uVar11 & 0xffffffu) | (uint)((bVar1 & 3u) | 0x25u) << 0x18;
                *(uint16_t *)((uint8_t *)puVar6 + 0xe) = *(uint16_t *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 10);
                *(ushort *)((uint8_t *)puVar6 + 0x16) =
                    *(ushort *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 8) |
                    (ushort)((*(ushort *)((uint8_t *)puVar15 + 0x12) & 0xc000u) >> 9);
                *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 6);
                *(short *)(puVar6 + 5) = *(short *)((uint8_t *)puVar15 + 0xe) + *(short *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 6);
                *(short *)(puVar6 + 7) = (short)puVar15[4] + *(short *)((uint8_t *)(uintptr_t)(uintptr_t)puVar7 + 6);
                break;
            case 0xf:
                uVar10 = *puVar15;
                bVar1  = *(uint8_t *)((uint8_t *)puVar15 + 3);
                *puVar6 = 0x7000000u;
                uVar11  = (uint)((bVar1 & 3u) | 0x25u);
            LAB_8001bcfc:
                puVar6[1] = (uVar10 & 0xffffffu) | (uVar11 << 0x18);
            }   /* end switch */

            uVar11  = (uint)(*(uint8_t *)((uint8_t *)puVar15 + 3) & 0x3cu);
            local_34++;
            puVar6  = (uint32_t *)((uint8_t *)puVar6  + (uint)DAT_800568fe[uVar11 / 2]);
            puVar15 = (uint   *)((uint8_t *)puVar16 + (uint)DAT_800568fc[uVar11 / 2]);
        } while (local_34 < (int)(uint)*(ushort *)((uint8_t *)puVar18 + 8));
    }

    if (bVar2) {
        *puVar3 = *puVar3 | 1u;
    }
    return puVar3;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001b36c (from analysis/SLUS_005.10/decomp/8001b36c.c) --- */
// addr: 0x8001b36c  name: FUN_8001b36c
// (see bone_anim.c for FUN_8001b3d4 which increments the slot;
//  FUN_8001b36c initialises without incrementing.)

/* --- SLUS_005.10 FUN_8001b49c (from analysis/SLUS_005.10/decomp/8001b49c.c) --- */
// addr: 0x8001b49c  name: FUN_8001b49c
// Full Ghidra export: see analysis/SLUS_005.10/decomp/8001b49c.c

#endif  /* GHIDRA REF */
