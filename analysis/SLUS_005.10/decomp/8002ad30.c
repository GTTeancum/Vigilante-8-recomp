// addr: 0x8002ad30  name: FUN_8002ad30

void FUN_8002ad30(uint *param_1,void *param_2,short param_3,short param_4)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  long lVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined4 uVar9;
  uint *puVar10;
  
  piVar3 = (int *)*piRam00000714;
  piVar2 = piRam00000714;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    puVar10 = (uint *)piVar2[2];
    if (((puVar10 != param_1) && ((char)puVar10[1] != '\x03')) && ((*puVar10 & 0x4000) != 0)) {
      iVar7 = puVar10[0x12] - param_1[9];
      uRam000003bc = uRam000003bc + 1 & 0x3f;
      iVar4 = uRam000003bc * 0x10;
      if (iVar7 < 0) {
        iVar7 = iVar7 + 0x1ffff;
      }
      iVar6 = puVar10[0x14] - param_1[0xb];
      iVar7 = iVar7 >> 0x11;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0x1ffff;
      }
      iVar6 = iVar6 >> 0x11;
      iVar8 = iVar7 * iVar7 + iVar6 * iVar6;
      if (0x2d9 < iVar8) {
        lVar5 = SquareRoot0(iVar8);
        iVar7 = iVar7 * (0x1b000 / lVar5);
        iVar6 = iVar6 * (0x1b000 / lVar5);
        if (iVar7 < 0) {
          iVar7 = iVar7 + 0xfff;
        }
        iVar7 = iVar7 >> 0xc;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = iVar6 >> 0xc;
      }
      if ((uint *)param_1[0x39] == puVar10) {
        uVar9 = 0x6000ff00;
      }
      else {
        uVar9 = 0x600000ff;
        if ((*(short *)((int)puVar10 + 6) < 0) && (cRam00000015 == '\x04')) {
          uVar9 = 0x60808080;
        }
      }
      *(undefined4 *)(&DAT_800a2bbc + iVar4) = uVar9;
      iVar8 = (short)param_1[4] * iVar7 + (short)param_1[7] * iVar6;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      *(short *)(&DAT_800a2bc0 + iVar4) = param_3 + (short)(iVar8 >> 0xc);
      iVar7 = (short)param_1[5] * iVar7 + (short)param_1[8] * iVar6;
      if (iVar7 < 0) {
        iVar7 = iVar7 + 0xfff;
      }
      *(short *)(&UNK_800a2bc2 + iVar4) = param_4 - (short)(iVar7 >> 0xc);
      AddPrim(param_2,&UNK_800a2bb8 + iVar4);
    }
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  return;
}

