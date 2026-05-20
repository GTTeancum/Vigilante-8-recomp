// addr: 0x80042cdc  name: FUN_80042cdc

int * FUN_80042cdc(int *param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int *piVar8;
  int *piVar9;
  int iVar10;
  int *piVar11;
  undefined4 *puVar12;
  int iVar13;
  int iVar14;
  int *piVar15;
  
  iVar14 = 0x7fffffff;
  iVar13 = 0;
  piVar15 = (int *)0x0;
  puVar12 = puRam000008d4;
  if (0 < iRam000008c0) {
    do {
      piVar9 = (int *)*puVar12;
      iVar10 = 0;
      piVar11 = piVar9;
      if (0 < *(short *)((int)piVar9 + 0x12)) {
        do {
          piVar8 = (int *)piVar11[7];
          if (((int *)*piVar8 == piVar9) &&
             ((param_2 == 0xffffffff || (*(ushort *)(piVar8 + 2) == param_2)))) {
            uVar5 = (uint)(*(int *)piVar8[1] < *piVar9);
            iVar1 = *param_1 - *(int *)piVar8[1 - uVar5];
            uVar7 = (uint)(((int *)piVar8[1])[2] < piVar9[2]);
            iVar6 = 0;
            if (0 < iVar1) {
              iVar6 = iVar1;
            }
            iVar2 = *param_1 - *(int *)piVar8[uVar5];
            iVar1 = 0;
            if (iVar2 < 0) {
              iVar1 = iVar2;
            }
            iVar3 = param_1[2] - *(int *)(piVar8[1 - uVar7] + 8);
            iVar2 = 0;
            if (0 < iVar3) {
              iVar2 = iVar3;
            }
            iVar4 = param_1[2] - *(int *)(piVar8[uVar7] + 8);
            iVar3 = 0;
            if (iVar4 < 0) {
              iVar3 = iVar4;
            }
            iVar6 = (iVar6 - iVar1) + (iVar2 - iVar3);
            if (iVar6 < iVar14) {
              iVar14 = iVar6;
              piVar15 = piVar8;
            }
          }
          iVar10 = iVar10 + 1;
          piVar11 = piVar11 + 1;
        } while (iVar10 < *(short *)((int)piVar9 + 0x12));
      }
      iVar13 = iVar13 + 1;
      puVar12 = puVar12 + 1;
    } while (iVar13 < iRam000008c0);
  }
  return piVar15;
}

