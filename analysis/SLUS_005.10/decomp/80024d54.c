// addr: 0x80024d54  name: FUN_80024d54

int FUN_80024d54(int param_1,int param_2,uint param_3,int param_4)

{
  ushort uVar1;
  bool bVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 *puVar9;
  undefined2 *puVar10;
  int iVar11;
  short *psVar12;
  undefined4 *puVar13;
  undefined4 *puVar14;
  int iVar15;
  undefined4 *puVar16;
  
  iVar4 = FUN_80015010();
  iVar15 = 0x3fe;
  puRam000007ec = &DAT_800738a0;
  puVar6 = &DAT_800738a0;
  do {
    puVar16 = puVar6 + 7;
    *puVar6 = puVar16;
    iVar15 = iVar15 + -1;
    puVar6 = puVar16;
  } while (-1 < iVar15);
  *puVar16 = 0;
  puVar5 = (undefined4 *)FUN_800247dc((int)*(short *)(param_2 + 2),(int)*(short *)(param_2 + 10));
  puVar6 = puVar5;
  if (*(short *)(puVar5[2] + (uint)*(byte *)(puVar5 + 4) * 2 + 2) == 0) {
    puVar6 = (undefined4 *)FUN_80024998(puVar5);
    if (puVar6 == (undefined4 *)0x0) {
      return 0;
    }
    *puVar5 = puRam000007ec;
    puRam000007ec = puVar5;
  }
  puVar7 = (undefined4 *)FUN_800247dc((int)*(short *)(param_1 + 2),(int)*(short *)(param_1 + 10));
  puVar5 = puVar7;
  if (*(short *)(puVar7[2] + (uint)*(byte *)(puVar7 + 4) * 2 + 2) == 0) {
    puVar5 = (undefined4 *)FUN_80024998(puVar7);
    if (puVar5 == (undefined4 *)0x0) {
      return 0;
    }
    *puVar7 = puRam000007ec;
    puRam000007ec = puVar7;
  }
  puVar5[6] = 0;
  uVar8 = FUN_80024cc0(puVar16,(int)*(short *)(puVar6 + 3),(int)*(short *)((int)puVar6 + 0xe));
  puVar5[5] = uVar8;
  *puVar5 = 0;
  puVar5[1] = 0;
  puVar16 = (undefined4 *)0x0;
  while (puVar5 != (undefined4 *)0x0) {
    iVar15 = FUN_80015010();
    bVar2 = param_3 < (uint)(iVar15 - iVar4);
    if ((bVar2) && (param_4 != 0)) break;
    puVar7 = (undefined4 *)*puVar5;
    if ((bVar2) || ((puVar5[2] == puVar6[2] && (*(char *)(puVar5 + 4) == *(char *)(puVar6 + 4))))) {
      iVar4 = puVar5[2] + (uint)*(byte *)(puVar5 + 4) * 2;
      *(ushort *)(iVar4 + 2) = *(ushort *)(iVar4 + 2) & 0x9fff;
      iVar15 = 0;
      for (iVar4 = puVar5[1]; iVar4 != 0; iVar4 = *(int *)(iVar4 + 4)) {
        iVar15 = iVar15 + 1;
      }
      iVar15 = iVar15 - (uint)(iVar15 != 0);
      iVar4 = FUN_800116f4((iVar15 + 2) * 4);
      puVar10 = (undefined2 *)(iVar15 * 4 + iVar4);
      puVar10[3] = 0;
      puVar10[2] = 0;
      *puVar10 = *(undefined2 *)(param_2 + 2);
      iVar15 = iVar15 + -1;
      puVar10[1] = *(undefined2 *)(param_2 + 10);
      if (iVar15 != -1) {
        psVar12 = (short *)(iVar15 * 4 + iVar4);
        do {
          puVar5 = (undefined4 *)puVar5[1];
          *psVar12 = *(short *)(puVar5 + 3) +
                     (short)((1 << (*(byte *)((int)puVar5 + 0x11) & 0x1f)) / 2);
          iVar15 = iVar15 + -1;
          psVar12[1] = *(short *)((int)puVar5 + 0xe) +
                       (short)((1 << (*(byte *)((int)puVar5 + 0x11) & 0x1f)) / 2);
          psVar12 = psVar12 + -2;
        } while (iVar15 != -1);
      }
      FUN_8002479c(puVar7);
      FUN_8002479c(puVar16);
      return iVar4;
    }
    puVar9 = (undefined4 *)FUN_80024998(puVar5);
joined_r0x80025008:
    puVar3 = puVar9;
    if (puVar3 != (undefined4 *)0x0) {
      puVar9 = (undefined4 *)*puVar3;
      puVar3[6] = puVar5[6] +
                  ((uint)*(byte *)(puVar3[2] + (uint)*(byte *)(puVar3 + 4) * 2 + 2) <<
                  (*(byte *)((int)puVar3 + 0x11) & 0x1f));
      iVar15 = puVar3[2] + (uint)*(byte *)(puVar3 + 4) * 2;
      uVar1 = *(ushort *)(iVar15 + 2);
      puVar14 = (undefined4 *)0x0;
      if ((uVar1 & 0x4000) != 0) goto code_r0x80025060;
      *(ushort *)(iVar15 + 2) = uVar1 | 0x4000;
      iVar15 = FUN_80024cc0(puVar3,(int)*(short *)(puVar6 + 3),(int)*(short *)((int)puVar6 + 0xe));
      puVar3[1] = puVar5;
      puVar3[5] = puVar3[6] + iVar15;
      goto LAB_80025178;
    }
    *puVar5 = puVar16;
    iVar15 = puVar5[2] + (uint)*(byte *)(puVar5 + 4) * 2;
    *(ushort *)(iVar15 + 2) = *(ushort *)(iVar15 + 2) | 0x6000;
    puVar16 = puVar5;
    puVar5 = puVar7;
  }
  FUN_8002479c(puVar5);
  FUN_8002479c(puVar16);
  return 0;
code_r0x80025060:
  puVar13 = puVar7;
  if ((uVar1 & 0x2000) != 0) {
    puVar13 = puVar16;
  }
  if ((puVar3[2] != puVar13[2]) || ((uint)*(byte *)(puVar3 + 4) != (uint)*(byte *)(puVar13 + 4))) {
    do {
      do {
        puVar14 = puVar13;
        puVar13 = (undefined4 *)*puVar14;
      } while (puVar3[2] != puVar13[2]);
    } while (*(char *)(puVar3 + 4) != *(char *)(puVar13 + 4));
  }
  *puVar3 = puRam000007ec;
  iVar11 = puVar13[6];
  iVar15 = puVar3[6];
  puRam000007ec = puVar3;
  if (0 < iVar11 - iVar15) {
    if (puVar14 == (undefined4 *)0x0) {
      if ((uVar1 & 0x2000) == 0) {
        puVar7 = (undefined4 *)*puVar13;
      }
      else {
        puVar16 = (undefined4 *)*puVar13;
      }
    }
    else {
      *puVar14 = *puVar13;
    }
    *(ushort *)(puVar13[2] + (uint)*(byte *)(puVar13 + 4) * 2 + 2) = uVar1 & 0xdfff;
    uVar8 = puVar3[6];
    puVar13[1] = puVar5;
    puVar13[6] = uVar8;
    puVar13[5] = puVar13[5] - (iVar11 - iVar15);
LAB_80025178:
    puVar7 = (undefined4 *)FUN_80024748(puVar7);
  }
  goto joined_r0x80025008;
}

