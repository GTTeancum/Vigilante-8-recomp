// addr: 0x80100ca0  name: FUN_80100ca0

undefined4 FUN_80100ca0(int param_1,int param_2,int param_3)

{
  short sVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  short *psVar7;
  undefined4 *puVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  int iVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  
  if ((param_2 == 0) || (param_2 != 1)) {
    if (param_3 != 0) {
      iVar5 = *(int *)(param_1 + 0x48);
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xffff;
      }
      iVar9 = *(int *)(param_1 + 0x50);
      if (iVar9 < 0) {
        iVar9 = iVar9 + 0xffff;
      }
      iVar12 = 0;
      do {
        iVar11 = 0;
        uVar6 = (iVar5 >> 0x10) + -0x10 + iVar12;
        do {
          iVar3 = param_1 + iVar11 * 2 + iVar12 * 0x40;
          *(undefined2 *)(iVar3 + 0x84) = 0;
          iVar3 = -(int)*(short *)(iVar3 + 0x884);
          uVar10 = (iVar9 >> 0x10) + -0x10 + iVar11;
          if (iVar3 != 0) {
            psVar7 = (short *)(*(int *)((uVar10 >> 6) * 4 + (uVar6 >> 6) * 0x80 + -0x7ff6ee60) +
                               (uVar6 & 0x3f) * 0x80 + (uVar10 & 0x3f) * 2);
            *psVar7 = *psVar7 + (short)iVar3;
          }
          iVar11 = iVar11 + 1;
        } while (iVar11 < 0x20);
        iVar12 = iVar12 + 1;
      } while (iVar12 < 0x20);
      puVar4 = (undefined4 *)(param_1 + 0x84);
      puVar8 = (undefined4 *)(param_1 + 0x884);
      do {
        uVar13 = puVar4[1];
        uVar14 = puVar4[2];
        uVar15 = puVar4[3];
        *puVar8 = *puVar4;
        puVar8[1] = uVar13;
        puVar8[2] = uVar14;
        puVar8[3] = uVar15;
        puVar4 = puVar4 + 4;
        puVar8 = puVar8 + 4;
      } while (puVar4 != (undefined4 *)(param_1 + 0x884));
    }
    iVar5 = *(int *)(param_1 + 0x80) + 1;
    *(int *)(param_1 + 0x80) = iVar5;
    if (iVar5 != 0x101) {
      return 0;
    }
    iVar5 = *(int *)(param_1 + 0x48);
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xffff;
    }
    iVar9 = *(int *)(param_1 + 0x50);
    if (iVar9 < 0) {
      iVar9 = iVar9 + 0xffff;
    }
    iVar12 = 0;
    do {
      iVar3 = 0;
      uVar6 = (iVar5 >> 0x10) + -0x10 + iVar12;
      iVar11 = iVar12 << 6;
      do {
        sVar1 = *(short *)(param_1 + iVar11 + 0x884);
        uVar10 = (iVar9 >> 0x10) + -0x10 + iVar3;
        if (sVar1 != 0) {
          psVar7 = (short *)(*(int *)((uVar10 >> 6) * 4 + (uVar6 >> 6) * 0x80 + -0x7ff6ee60) +
                             (uVar6 & 0x3f) * 0x80 + (uVar10 & 0x3f) * 2);
          *psVar7 = *psVar7 - sVar1;
        }
        iVar3 = iVar3 + 1;
        iVar11 = iVar11 + 2;
      } while (iVar3 < 0x20);
      iVar12 = iVar12 + 1;
    } while (iVar12 < 0x20);
    FUN_800205f8/*0x800205f8*/(param_1);
  }
  iVar5 = -0x10;
  iVar9 = 0x10;
  do {
    iVar11 = -0x10;
    iVar12 = 0x100;
    do {
      uVar2 = SquareRoot0/*0x8004c6e4*/((iVar5 * iVar5 + iVar12) * 0x40);
      *(undefined1 *)(param_1 + iVar11 + iVar9 + 0x1084) = uVar2;
      iVar11 = iVar11 + 1;
      iVar12 = iVar11 * iVar11;
    } while (iVar11 < 0x10);
    iVar5 = iVar5 + 1;
    iVar9 = iVar9 + 0x20;
  } while (iVar5 < 0x10);
  return 0;
}

