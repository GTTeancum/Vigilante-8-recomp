// addr: 0x8004dcbc  name: RCpolyF3A

undefined4 RCpolyF3A(undefined4 param_1,int *param_2,int param_3,SVECTOR *param_4)

{
  short sVar1;
  short sVar2;
  short sVar3;
  SVECTOR *pSVar4;
  short *psVar5;
  short *psVar6;
  short *psVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  undefined4 uVar11;
  
  psVar5 = *(short **)(param_4 + 9);
  psVar6 = *(short **)&param_4[9].vz;
  psVar7 = *(short **)(param_4 + 10);
  iVar9 = gte_stH();
  uVar8 = iVar9 >> 1;
  if (((*(uint *)(psVar5 + 10) < uVar8) && (*(uint *)(psVar6 + 10) < uVar8)) &&
     (*(uint *)(psVar7 + 10) < uVar8)) {
    return param_1;
  }
  iVar10 = gte_stOFX();
  iVar9 = (iVar10 >> 0x10) + ((uint)param_2[1] >> 1);
  if (((iVar9 < psVar5[8]) && (iVar9 < psVar6[8])) && (iVar9 < psVar7[8])) {
    return param_1;
  }
  iVar9 = (iVar10 >> 0x10) - ((uint)param_2[1] >> 1);
  if (((psVar5[8] < iVar9) && (psVar6[8] < iVar9)) && (psVar7[8] < iVar9)) {
    return param_1;
  }
  iVar10 = gte_stOFY();
  iVar9 = (iVar10 >> 0x10) + ((uint)param_2[2] >> 1);
  if (((iVar9 < psVar5[9]) && (iVar9 < psVar6[9])) && (iVar9 < psVar7[9])) {
    return param_1;
  }
  iVar9 = (iVar10 >> 0x10) - ((uint)param_2[2] >> 1);
  if (((psVar5[9] < iVar9) && (psVar6[9] < iVar9)) && (psVar7[9] < iVar9)) {
    return param_1;
  }
  sVar1 = *psVar5;
  sVar2 = *psVar6;
  sVar3 = *psVar7;
  param_4->vx = (short)((int)sVar1 + (int)sVar2 >> 1);
  param_4[3].vx = (short)((int)sVar2 + (int)sVar3 >> 1);
  param_4[6].vx = (short)((int)sVar3 + (int)sVar1 >> 1);
  sVar1 = psVar5[1];
  sVar2 = psVar6[1];
  sVar3 = psVar7[1];
  param_4->vy = (short)((int)sVar1 + (int)sVar2 >> 1);
  param_4[3].vy = (short)((int)sVar2 + (int)sVar3 >> 1);
  param_4[6].vy = (short)((int)sVar3 + (int)sVar1 >> 1);
  sVar1 = psVar5[2];
  sVar2 = psVar6[2];
  sVar3 = psVar7[2];
  param_4->vz = (short)((int)sVar1 + (int)sVar2 >> 1);
  param_4[3].vz = (short)((int)sVar2 + (int)sVar3 >> 1);
  param_4[6].vz = (short)((int)sVar3 + (int)sVar1 >> 1);
  uVar11 = 0x8004de84;
  gte_ldv0(param_4);
  gte_ldVXY1(param_4 + 3);
  gte_ldVZ1(&param_4[3].vz);
  gte_ldVXY2(param_4 + 6);
  gte_ldVZ2(&param_4[6].vz);
  gte_rtpt_b();
  if (*param_2 == param_3 + 1) {
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
  }
  else {
    pSVar4 = param_4 + 0xb;
    param_4[0x15].vz = (short)uVar11;
    param_4[0x15].pad = (short)((uint)uVar11 >> 0x10);
    sVar1 = param_4[9].vy;
    param_4[0x14].vx = param_4[9].vx;
    param_4[0x14].vy = sVar1;
    *(SVECTOR **)&param_4[0x14].vz = param_4;
    *(SVECTOR **)(param_4 + 0x15) = param_4 + 6;
    gte_stSZ1();
    gte_stSZ2();
    gte_stSZ3();
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    RCpolyF3A();
    sVar1 = pSVar4[-0xffffffff00000002].pad;
    pSVar4[9].vx = pSVar4[-0xffffffff00000002].vz;
    pSVar4[9].vy = sVar1;
    *(SVECTOR **)&pSVar4[9].vz = pSVar4 + -8;
    *(SVECTOR **)(pSVar4 + 10) = pSVar4 + -0xb;
    RCpolyF3A();
    sVar1 = pSVar4[-0xffffffff00000001].vy;
    pSVar4[9].vx = pSVar4[-0xffffffff00000001].vx;
    pSVar4[9].vy = sVar1;
    *(SVECTOR **)&pSVar4[9].vz = pSVar4 + -5;
    *(SVECTOR **)(pSVar4 + 10) = pSVar4 + -8;
    RCpolyF3A();
    *(SVECTOR **)(pSVar4 + 9) = pSVar4 + -0xb;
    *(SVECTOR **)&pSVar4[9].vz = pSVar4 + -8;
    *(SVECTOR **)(pSVar4 + 10) = pSVar4 + -5;
    RCpolyF3A();
  }
  return param_1;
}

