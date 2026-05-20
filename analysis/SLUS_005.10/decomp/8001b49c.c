// addr: 0x8001b49c  name: FUN_8001b49c

ushort * FUN_8001b49c(int *param_1,uint param_2)

{
  byte bVar1;
  bool bVar2;
  ushort *puVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined *puVar7;
  short sVar8;
  ushort uVar9;
  uint uVar10;
  uint uVar11;
  ushort *puVar12;
  char *pcVar13;
  int iVar14;
  uint *puVar15;
  uint *puVar16;
  int iVar17;
  undefined4 *puVar18;
  int local_34;
  
  puVar18 = *(undefined4 **)((param_2 & 0xffff) * 4 + *(int *)(*param_1 + 4));
  puVar3 = (ushort *)FUN_800116f4((uint)*(byte *)((int)puVar18 + 0x19) * 4 + 0x2c);
  iVar17 = 0;
  local_34 = 0;
  bVar2 = false;
  puVar12 = puVar3;
  if (*(char *)((int)puVar18 + 0x19) != '\0') {
    do {
      uVar4 = FUN_8001b36c(param_1,0);
      *(undefined4 *)(puVar12 + 0x16) = uVar4;
      local_34 = local_34 + 1;
      puVar12 = puVar12 + 2;
    } while (local_34 < (int)(uint)*(byte *)((int)puVar18 + 0x19));
  }
  local_34 = 0;
  iVar14 = puVar18[5];
  if (*(ushort *)(puVar18 + 4) != 0) {
    do {
      uVar11 = *(byte *)(iVar14 + 3) >> 2 & 0xf;
      if (uVar11 == 10) {
        iVar17 = iVar17 + (uint)*(ushort *)(iVar14 + 10) * 0x28;
        iVar14 = iVar14 + (uint)*(ushort *)(iVar14 + 10) * 4;
      }
      iVar5 = uVar11 * 4;
      local_34 = local_34 + 1;
      iVar17 = iVar17 + (uint)*(ushort *)(&DAT_800568fe + iVar5);
      iVar14 = iVar14 + (uint)*(ushort *)(&DAT_800568fc + iVar5);
    } while (local_34 < (int)(uint)*(ushort *)(puVar18 + 4));
  }
  *puVar3 = 0;
  puVar3[1] = (ushort)iVar17;
  *(undefined4 *)(puVar3 + 2) = *puVar18;
  *(undefined4 *)(puVar3 + 4) = puVar18[1];
  *(undefined4 *)(puVar3 + 6) = puVar18[2];
  *(undefined4 *)(puVar3 + 8) = puVar18[3];
  *(uint *)(puVar3 + 10) = (uint)*(ushort *)(puVar18 + 4);
  *(undefined4 *)(puVar3 + 0xc) = puVar18[5];
  puVar6 = (undefined4 *)FUN_800116f4(iVar17);
  *(undefined4 **)(puVar3 + 0xe) = puVar6;
  puVar3[0x10] = 0;
  puVar3[0x11] = 0;
  bVar1 = *(byte *)(puVar18 + 6);
  puVar3[0x14] = 0;
  puVar3[0x13] = (ushort)bVar1;
  puVar3[0x15] = *(ushort *)((int)puVar18 + 0x12);
  puVar3[0x12] = *(ushort *)((int)puVar18 + 0x1a);
  local_34 = 0;
  puVar15 = (uint *)puVar18[5];
  if (*(short *)(puVar18 + 4) != 0) {
    do {
      if ((*puVar15 & 0x80000000) != 0) {
        *(undefined1 *)puVar15 = uRam00000854;
        *(undefined1 *)((int)puVar15 + 1) = uRam00000855;
        *(undefined1 *)((int)puVar15 + 2) = uRam00000856;
      }
      puVar16 = puVar15;
      switch(*(byte *)((int)puVar15 + 3) >> 2 & 0xf) {
      case 0:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x4000000;
        uVar11 = uVar11 & 0xffffff;
        uVar10 = bVar1 & 3 | 0x20;
        goto LAB_8001bb68;
      case 1:
        iVar17 = FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) +
                                      (*(ushort *)((int)puVar15 + 0x12) & 0x3fff));
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x9000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x34) << 0x18;
        puVar6[4] = puVar15[5];
        puVar6[7] = puVar15[6];
        *(undefined2 *)((int)puVar6 + 0xe) = *(undefined2 *)(iVar17 + 10);
        *(ushort *)((int)puVar6 + 0x1a) =
             *(ushort *)(iVar17 + 8) | (ushort)((*(ushort *)((int)puVar15 + 0x12) & 0xc000) >> 9);
        *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)(iVar17 + 6);
        *(short *)(puVar6 + 6) = *(short *)((int)puVar15 + 0xe) + *(short *)(iVar17 + 6);
        sVar8 = (short)puVar15[4] + *(short *)(iVar17 + 6);
        goto LAB_8001ba20;
      case 2:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x6000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x30) << 0x18;
        puVar6[3] = puVar15[3];
        puVar6[5] = puVar15[4];
        break;
      case 3:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x9000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x34) << 0x18;
        puVar6[4] = puVar15[5];
        puVar6[7] = puVar15[6];
        break;
      case 4:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        bVar2 = true;
        *puVar6 = 0x4000000;
        uVar11 = uVar11 & 0xffffff;
        uVar10 = bVar1 & 3 | 0x20;
        goto LAB_8001bb68;
      case 5:
        iVar17 = FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) +
                                      (*(ushort *)((int)puVar15 + 0x12) & 0x3fff));
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x7000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x24) << 0x18;
        *(undefined2 *)((int)puVar6 + 0xe) = *(undefined2 *)(iVar17 + 10);
        *(ushort *)((int)puVar6 + 0x16) =
             *(ushort *)(iVar17 + 8) | (ushort)((*(ushort *)((int)puVar15 + 0x12) & 0xc000) >> 9);
        *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)(iVar17 + 6);
        *(short *)(puVar6 + 5) = *(short *)((int)puVar15 + 0xe) + *(short *)(iVar17 + 6);
        bVar2 = true;
        *(short *)(puVar6 + 7) = (short)puVar15[4] + *(short *)(iVar17 + 6);
        break;
      case 6:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x3000000;
        uVar11 = uVar11 & 0xffffff;
        uVar10 = bVar1 & 3 | 0x40;
        goto LAB_8001bb68;
      case 7:
        uVar10 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        bVar2 = true;
        *puVar6 = 0x7000000;
        uVar11 = bVar1 & 3 | 0x24;
        goto LAB_8001bcfc;
      case 8:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        bVar2 = true;
        *puVar6 = 0x6000000;
        uVar11 = uVar11 & 0xffffff;
        uVar10 = bVar1 & 3 | 0x30;
        goto LAB_8001bb68;
      case 9:
        iVar17 = FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) +
                                      (*(ushort *)((int)puVar15 + 0x16) & 0x3fff));
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x9000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x34) << 0x18;
        *(undefined2 *)((int)puVar6 + 0xe) = *(undefined2 *)(iVar17 + 10);
        *(ushort *)((int)puVar6 + 0x1a) =
             *(ushort *)(iVar17 + 8) | (ushort)((*(ushort *)((int)puVar15 + 0x16) & 0xc000) >> 9);
        *(short *)(puVar6 + 3) = (short)puVar15[4] + *(short *)(iVar17 + 6);
        *(short *)(puVar6 + 6) = *(short *)((int)puVar15 + 0x12) + *(short *)(iVar17 + 6);
        bVar2 = true;
        sVar8 = (short)puVar15[5] + *(short *)(iVar17 + 6);
LAB_8001ba20:
        *(short *)(puVar6 + 9) = sVar8;
        break;
      case 10:
        iVar17 = 0;
        if (*(short *)((int)puVar15 + 10) != 0) {
          pcVar13 = (char *)((int)puVar6 + 0x25);
          do {
            iVar14 = FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) +
                                          (*(ushort *)((int)puVar16 + 0xe) & 0x3fff));
            pcVar13[-0x1e] = '/';
            *(ushort *)(pcVar13 + -0xf) = *(ushort *)(iVar14 + 8) | 0x20;
            *(undefined2 *)(pcVar13 + -0x17) = *(undefined2 *)(iVar14 + 10);
            pcVar13[-0x19] = *(char *)(iVar14 + 6);
            pcVar13[-0x18] = (char)((ushort)*(undefined2 *)(iVar14 + 6) >> 8);
            pcVar13[-0x11] = *(char *)(iVar14 + 2) + *(char *)(iVar14 + 6) + -1;
            pcVar13[-0x10] = (char)((ushort)*(undefined2 *)(iVar14 + 6) >> 8);
            pcVar13[-9] = *(char *)(iVar14 + 6);
            pcVar13[-8] = *(char *)(iVar14 + 4) +
                          (char)((ushort)*(undefined2 *)(iVar14 + 6) >> 8) + -1;
            pcVar13[-1] = *(char *)(iVar14 + 2) + *(char *)(iVar14 + 6) + -1;
            *pcVar13 = *(char *)(iVar14 + 4) + (char)((ushort)*(undefined2 *)(iVar14 + 6) >> 8) + -1
            ;
            puVar16 = puVar16 + 1;
            iVar17 = iVar17 + 1;
            puVar6 = puVar6 + 10;
            pcVar13 = pcVar13 + 0x28;
          } while (iVar17 < (int)(uint)*(ushort *)((int)puVar15 + 10));
        }
        puVar16 = puVar15 + *(ushort *)((int)puVar15 + 10);
        break;
      case 0xb:
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        bVar2 = true;
        *puVar6 = 0x9000000;
        uVar11 = uVar11 & 0xffffff;
        uVar10 = bVar1 & 3 | 0x34;
LAB_8001bb68:
        puVar6[1] = uVar11 | uVar10 << 0x18;
        break;
      case 0xc:
        uVar9 = (ushort)puVar15[4] & 0x3fff;
        if (uVar9 == 0x3fff) {
          puVar7 = &DAT_80065a28;
          *(undefined1 *)((int)puVar15 + 0x13) = 0;
        }
        else {
          puVar7 = (undefined *)FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) + uVar9);
        }
        uVar11 = *puVar15;
        *(char *)((int)puVar15 + 0x12) = (char)((ushort)*(undefined2 *)(puVar7 + 6) >> 8);
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x7000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x24) << 0x18;
        *(undefined2 *)((int)puVar6 + 0xe) = *(undefined2 *)(puVar7 + 10);
        *(ushort *)((int)puVar6 + 0x16) =
             *(ushort *)(puVar7 + 8) | (ushort)(((ushort)puVar15[4] & 0xc000) >> 9);
        break;
      case 0xd:
        if (*(ushort *)((int)puVar15 + 0x12) == 0xffff) {
          puVar7 = &DAT_80065a28;
        }
        else {
          puVar7 = (undefined *)
                   FUN_8001b3d4(param_1,*(short *)((int)puVar18 + 0x12) +
                                        (*(ushort *)((int)puVar15 + 0x12) & 0x3fff));
        }
        uVar11 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x7000000;
        puVar6[1] = uVar11 & 0xffffff | (bVar1 & 3 | 0x25) << 0x18;
        *(undefined2 *)((int)puVar6 + 0xe) = *(undefined2 *)(puVar7 + 10);
        *(ushort *)((int)puVar6 + 0x16) =
             *(ushort *)(puVar7 + 8) | (ushort)((*(ushort *)((int)puVar15 + 0x12) & 0xc000) >> 9);
        *(short *)(puVar6 + 3) = (short)puVar15[3] + *(short *)(puVar7 + 6);
        *(short *)(puVar6 + 5) = *(short *)((int)puVar15 + 0xe) + *(short *)(puVar7 + 6);
        *(short *)(puVar6 + 7) = (short)puVar15[4] + *(short *)(puVar7 + 6);
        break;
      case 0xf:
        uVar10 = *puVar15;
        bVar1 = *(byte *)((int)puVar15 + 3);
        *puVar6 = 0x7000000;
        uVar11 = bVar1 & 3 | 0x25;
LAB_8001bcfc:
        puVar6[1] = uVar10 & 0xffffff | uVar11 << 0x18;
      }
      uVar11 = *(byte *)((int)puVar15 + 3) & 0x3c;
      local_34 = local_34 + 1;
      puVar6 = (undefined4 *)((int)puVar6 + (uint)*(ushort *)(&DAT_800568fe + uVar11));
      puVar15 = (uint *)((int)puVar16 + (uint)*(ushort *)(&DAT_800568fc + uVar11));
    } while (local_34 < (int)(uint)*(ushort *)(puVar18 + 4));
  }
  if (bVar2) {
    *puVar3 = *puVar3 | 1;
  }
  return puVar3;
}

