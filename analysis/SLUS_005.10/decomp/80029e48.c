// addr: 0x80029e48  name: FUN_80029e48

void FUN_80029e48(int param_1,uint *param_2)

{
  short sVar1;
  short sVar2;
  long lVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  uint *puVar11;
  int iVar12;
  int iVar13;
  uint *puVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int iVar19;
  
  puVar14 = *(uint **)(param_1 + 0x10);
  iVar18 = gte_stOFX();
  iVar17 = gte_stOFY();
  iVar16 = (int)*(short *)(param_1 + 8) - (iVar18 >> 0x10);
  iVar15 = (int)*(short *)(param_1 + 10) - (iVar17 >> 0x10);
  lVar3 = SquareRoot0(iVar16 * iVar16 + iVar15 * iVar15);
  if ((lVar3 < 0x40) && (puVar11 = *(uint **)(param_1 + 0xc), (*puVar11 & 0xffffff) != 0)) {
    iVar10 = 0x40 - lVar3;
    uVar4 = iVar10 * (uint)*(byte *)((int)puVar11 + 2) >> 6;
    uVar6 = 0xff;
    if (uVar4 < 0xff) {
      uVar6 = uVar4;
    }
    uVar5 = iVar10 * (uint)*(byte *)((int)puVar11 + 1) >> 6;
    uVar4 = 0xff;
    if (uVar5 < 0xff) {
      uVar4 = uVar5;
    }
    uVar4 = uVar6 << 0x10 | uVar4 << 8;
    uVar6 = (uRam00000680 & 0xff) + (iVar10 * (uint)(byte)*puVar11 >> 6);
    uRam00000680 = uVar4 | 0xff;
    if (uVar6 < 0xff) {
      uRam00000680 = uVar4 | uVar6;
    }
  }
  if (lVar3 == 0) {
    iVar19 = 0x1000;
    iVar10 = 0;
  }
  else {
    iVar19 = (iVar16 * 0x1000 + lVar3 / 2) / lVar3;
    iVar10 = (iVar15 * 0x1000 + lVar3 / 2) / lVar3;
  }
  iVar7 = *(int *)(param_1 + 0xc);
  iVar13 = 0;
  puVar11 = puVar14;
  if (*(short *)(iVar7 + 10) != 0) {
    do {
      iVar8 = (int)*(short *)(iVar7 + iVar13 * 4 + 0xc);
      iVar7 = iVar16 * iVar8;
      iVar8 = iVar15 * iVar8;
      if (iVar7 < 0) {
        iVar7 = iVar7 + 0xfff;
      }
      sVar1 = (short)((uint)iVar18 >> 0x10) + (short)(iVar7 >> 0xc);
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      iVar12 = (int)((uint)(byte)puVar11[5] - (uint)(byte)puVar11[3]) / 2;
      iVar9 = (int)((uint)*(byte *)((int)puVar11 + 0x1d) - (uint)*(byte *)((int)puVar11 + 0xd)) / 2;
      iVar7 = iVar10 * iVar9;
      puVar14 = puVar11 + 10;
      sVar2 = (short)((uint)iVar17 >> 0x10) + (short)(iVar8 >> 0xc);
      iVar8 = -iVar19 * iVar12 + iVar7;
      *puVar11 = (uint)puVar14 & 0xffffff | 0x9000000;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      iVar9 = iVar19 * iVar9;
      *(short *)(puVar11 + 2) = sVar1 + (short)(iVar8 >> 0xc);
      iVar8 = -iVar10 * iVar12 - iVar9;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)((int)puVar11 + 10) = sVar2 + (short)(iVar8 >> 0xc);
      iVar8 = iVar19 * iVar12 + iVar7;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)(puVar11 + 4) = sVar1 + (short)(iVar8 >> 0xc);
      iVar8 = iVar10 * iVar12 - iVar9;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)((int)puVar11 + 0x12) = sVar2 + (short)(iVar8 >> 0xc);
      iVar8 = -iVar19 * iVar12 - iVar7;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)(puVar11 + 6) = sVar1 + (short)(iVar8 >> 0xc);
      iVar8 = -iVar10 * iVar12 + iVar9;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)((int)puVar11 + 0x1a) = sVar2 + (short)(iVar8 >> 0xc);
      iVar7 = iVar19 * iVar12 - iVar7;
      if (iVar7 < 0) {
        iVar7 = iVar7 + 0xfff;
      }
      *(short *)(puVar11 + 8) = sVar1 + (short)(iVar7 >> 0xc);
      iVar9 = iVar10 * iVar12 + iVar9;
      if (iVar9 < 0) {
        iVar9 = iVar9 + 0xfff;
      }
      *(short *)((int)puVar11 + 0x22) = sVar2 + (short)(iVar9 >> 0xc);
      iVar7 = *(int *)(param_1 + 0xc);
      iVar13 = iVar13 + 1;
      puVar11 = puVar14;
    } while (iVar13 < (int)(uint)*(ushort *)(iVar7 + 10));
  }
  uVar6 = *param_2;
  *param_2 = *(uint *)(param_1 + 0x10) & 0xffffff;
  puVar14[-10] = uVar6 | 0x9000000;
  return;
}

