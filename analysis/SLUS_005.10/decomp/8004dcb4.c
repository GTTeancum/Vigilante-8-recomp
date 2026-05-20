// addr: 0x8004dcb4  name: RCpolyF3

undefined4 RCpolyF3(undefined4 param_1,int *param_2)

{
  short sVar1;
  short sVar2;
  short sVar3;
  int iVar4;
  SVECTOR *r0;
  SVECTOR *pSVar5;
  short *psVar6;
  short *psVar7;
  short *psVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  undefined4 uVar12;
  
  r0 = (SVECTOR *)(param_2 + 0x18);
  iVar4 = 0;
  psVar6 = (short *)param_2[0x2a];
  psVar7 = (short *)param_2[0x2b];
  psVar8 = (short *)param_2[0x2c];
  iVar10 = gte_stH();
  uVar9 = iVar10 >> 1;
  if (((*(uint *)(psVar6 + 10) < uVar9) && (*(uint *)(psVar7 + 10) < uVar9)) &&
     (*(uint *)(psVar8 + 10) < uVar9)) {
    return param_1;
  }
  iVar11 = gte_stOFX();
  iVar10 = (iVar11 >> 0x10) + ((uint)param_2[1] >> 1);
  if (((iVar10 < psVar6[8]) && (iVar10 < psVar7[8])) && (iVar10 < psVar8[8])) {
    return param_1;
  }
  iVar10 = (iVar11 >> 0x10) - ((uint)param_2[1] >> 1);
  if (((psVar6[8] < iVar10) && (psVar7[8] < iVar10)) && (psVar8[8] < iVar10)) {
    return param_1;
  }
  iVar11 = gte_stOFY();
  iVar10 = (iVar11 >> 0x10) + ((uint)param_2[2] >> 1);
  if (((iVar10 < psVar6[9]) && (iVar10 < psVar7[9])) && (iVar10 < psVar8[9])) {
    return param_1;
  }
  iVar10 = (iVar11 >> 0x10) - ((uint)param_2[2] >> 1);
  if (((psVar6[9] < iVar10) && (psVar7[9] < iVar10)) && (psVar8[9] < iVar10)) {
    return param_1;
  }
  sVar1 = *psVar6;
  sVar2 = *psVar7;
  sVar3 = *psVar8;
  r0->vx = (short)((int)sVar1 + (int)sVar2 >> 1);
  *(short *)(param_2 + 0x1e) = (short)((int)sVar2 + (int)sVar3 >> 1);
  *(short *)(param_2 + 0x24) = (short)((int)sVar3 + (int)sVar1 >> 1);
  sVar1 = psVar6[1];
  sVar2 = psVar7[1];
  sVar3 = psVar8[1];
  *(short *)((int)param_2 + 0x62) = (short)((int)sVar1 + (int)sVar2 >> 1);
  *(short *)((int)param_2 + 0x7a) = (short)((int)sVar2 + (int)sVar3 >> 1);
  *(short *)((int)param_2 + 0x92) = (short)((int)sVar3 + (int)sVar1 >> 1);
  sVar1 = psVar6[2];
  sVar2 = psVar7[2];
  sVar3 = psVar8[2];
  *(short *)(param_2 + 0x19) = (short)((int)sVar1 + (int)sVar2 >> 1);
  *(short *)(param_2 + 0x1f) = (short)((int)sVar2 + (int)sVar3 >> 1);
  *(short *)(param_2 + 0x25) = (short)((int)sVar3 + (int)sVar1 >> 1);
  uVar12 = 0x8004de84;
  gte_ldv0(r0);
  gte_ldVXY1(r0 + 3);
  gte_ldVZ1(&r0[3].vz);
  gte_ldVXY2(r0 + 6);
  gte_ldVZ2(&r0[6].vz);
  gte_rtpt_b();
  if (*param_2 == iVar4 + 1) {
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
    DIVF3A_OBJ_320();
  }
  else {
    pSVar5 = r0 + 0xb;
    r0[0x15].vz = (short)uVar12;
    r0[0x15].pad = (short)((uint)uVar12 >> 0x10);
    sVar1 = r0[9].vy;
    r0[0x14].vx = r0[9].vx;
    r0[0x14].vy = sVar1;
    *(SVECTOR **)&r0[0x14].vz = r0;
    *(SVECTOR **)(r0 + 0x15) = r0 + 6;
    gte_stSZ1();
    gte_stSZ2();
    gte_stSZ3();
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    RCpolyF3A();
    sVar1 = pSVar5[-0xffffffff00000002].pad;
    pSVar5[9].vx = pSVar5[-0xffffffff00000002].vz;
    pSVar5[9].vy = sVar1;
    *(SVECTOR **)&pSVar5[9].vz = pSVar5 + -8;
    *(SVECTOR **)(pSVar5 + 10) = pSVar5 + -0xb;
    RCpolyF3A();
    sVar1 = pSVar5[-0xffffffff00000001].vy;
    pSVar5[9].vx = pSVar5[-0xffffffff00000001].vx;
    pSVar5[9].vy = sVar1;
    *(SVECTOR **)&pSVar5[9].vz = pSVar5 + -5;
    *(SVECTOR **)(pSVar5 + 10) = pSVar5 + -8;
    RCpolyF3A();
    *(SVECTOR **)(pSVar5 + 9) = pSVar5 + -0xb;
    *(SVECTOR **)&pSVar5[9].vz = pSVar5 + -8;
    *(SVECTOR **)(pSVar5 + 10) = pSVar5 + -5;
    RCpolyF3A();
  }
  return param_1;
}

