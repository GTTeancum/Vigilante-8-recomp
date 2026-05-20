// addr: 0x8004e02c  name: RCpolyFT3A

undefined4 RCpolyFT3A(undefined4 param_1,int *param_2,int param_3,SVECTOR *param_4)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  SVECTOR *pSVar7;
  short *psVar8;
  short *psVar9;
  short *psVar10;
  uint uVar11;
  int iVar12;
  int iVar13;
  undefined4 uVar14;
  
  psVar8 = *(short **)(param_4 + 9);
  psVar9 = *(short **)&param_4[9].vz;
  psVar10 = *(short **)(param_4 + 10);
  iVar12 = gte_stH();
  uVar11 = iVar12 >> 1;
  if (((*(uint *)(psVar8 + 10) < uVar11) && (*(uint *)(psVar9 + 10) < uVar11)) &&
     (*(uint *)(psVar10 + 10) < uVar11)) {
    return param_1;
  }
  iVar13 = gte_stOFX();
  iVar12 = (iVar13 >> 0x10) + ((uint)param_2[1] >> 1);
  if (((iVar12 < psVar8[8]) && (iVar12 < psVar9[8])) && (iVar12 < psVar10[8])) {
    return param_1;
  }
  iVar12 = (iVar13 >> 0x10) - ((uint)param_2[1] >> 1);
  if (((psVar8[8] < iVar12) && (psVar9[8] < iVar12)) && (psVar10[8] < iVar12)) {
    return param_1;
  }
  iVar13 = gte_stOFY();
  iVar12 = (iVar13 >> 0x10) + ((uint)param_2[2] >> 1);
  if (((iVar12 < psVar8[9]) && (iVar12 < psVar9[9])) && (iVar12 < psVar10[9])) {
    return param_1;
  }
  iVar12 = (iVar13 >> 0x10) - ((uint)param_2[2] >> 1);
  if (((psVar8[9] < iVar12) && (psVar9[9] < iVar12)) && (psVar10[9] < iVar12)) {
    return param_1;
  }
  sVar4 = *psVar8;
  sVar5 = *psVar9;
  sVar6 = *psVar10;
  param_4->vx = (short)((int)sVar4 + (int)sVar5 >> 1);
  param_4[3].vx = (short)((int)sVar5 + (int)sVar6 >> 1);
  param_4[6].vx = (short)((int)sVar6 + (int)sVar4 >> 1);
  sVar4 = psVar8[1];
  sVar5 = psVar9[1];
  sVar6 = psVar10[1];
  param_4->vy = (short)((int)sVar4 + (int)sVar5 >> 1);
  param_4[3].vy = (short)((int)sVar5 + (int)sVar6 >> 1);
  param_4[6].vy = (short)((int)sVar6 + (int)sVar4 >> 1);
  sVar4 = psVar8[2];
  sVar5 = psVar9[2];
  sVar6 = psVar10[2];
  param_4->vz = (short)((int)sVar4 + (int)sVar5 >> 1);
  param_4[3].vz = (short)((int)sVar5 + (int)sVar6 >> 1);
  param_4[6].vz = (short)((int)sVar6 + (int)sVar4 >> 1);
  uVar14 = 0x8004e1f4;
  gte_ldv0(param_4);
  gte_ldVXY1(param_4 + 3);
  gte_ldVZ1(&param_4[3].vz);
  gte_ldVXY2(param_4 + 6);
  gte_ldVZ2(&param_4[6].vz);
  bVar1 = *(byte *)(psVar8 + 4);
  gte_rtpt_b();
  bVar2 = *(byte *)(psVar9 + 4);
  bVar3 = *(byte *)(psVar10 + 4);
  *(char *)&param_4[1].vx = (char)((uint)bVar1 + (uint)bVar2 >> 1);
  *(char *)&param_4[4].vx = (char)((uint)bVar2 + (uint)bVar3 >> 1);
  *(char *)&param_4[7].vx = (char)((uint)bVar3 + (uint)bVar1 >> 1);
  bVar1 = *(byte *)((int)psVar8 + 9);
  bVar2 = *(byte *)((int)psVar9 + 9);
  bVar3 = *(byte *)((int)psVar10 + 9);
  *(char *)((int)&param_4[1].vx + 1) = (char)((uint)bVar1 + (uint)bVar2 >> 1);
  *(char *)((int)&param_4[4].vx + 1) = (char)((uint)bVar2 + (uint)bVar3 >> 1);
  *(char *)((int)&param_4[7].vx + 1) = (char)((uint)bVar3 + (uint)bVar1 >> 1);
  if (*param_2 == param_3 + 1) {
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    DIVFT3A_OBJ_37C();
    DIVFT3A_OBJ_37C();
    DIVFT3A_OBJ_37C();
    DIVFT3A_OBJ_37C();
  }
  else {
    gte_stSZ1();
    gte_stSZ2();
    gte_stSZ3();
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    pSVar7 = param_4 + 0xb;
    param_4[0x15].vz = (short)uVar14;
    param_4[0x15].pad = (short)((uint)uVar14 >> 0x10);
    sVar4 = param_4[9].vy;
    param_4[0x14].vx = param_4[9].vx;
    param_4[0x14].vy = sVar4;
    *(SVECTOR **)&param_4[0x14].vz = param_4;
    *(SVECTOR **)(param_4 + 0x15) = param_4 + 6;
    RCpolyFT3A();
    sVar4 = pSVar7[-0xffffffff00000002].pad;
    pSVar7[9].vx = pSVar7[-0xffffffff00000002].vz;
    pSVar7[9].vy = sVar4;
    *(SVECTOR **)&pSVar7[9].vz = pSVar7 + -8;
    *(SVECTOR **)(pSVar7 + 10) = pSVar7 + -0xb;
    RCpolyFT3A();
    sVar4 = pSVar7[-0xffffffff00000001].vy;
    pSVar7[9].vx = pSVar7[-0xffffffff00000001].vx;
    pSVar7[9].vy = sVar4;
    *(SVECTOR **)&pSVar7[9].vz = pSVar7 + -5;
    *(SVECTOR **)(pSVar7 + 10) = pSVar7 + -8;
    RCpolyFT3A();
    *(SVECTOR **)(pSVar7 + 9) = pSVar7 + -0xb;
    *(SVECTOR **)&pSVar7[9].vz = pSVar7 + -8;
    *(SVECTOR **)(pSVar7 + 10) = pSVar7 + -5;
    RCpolyFT3A();
  }
  return param_1;
}

