// addr: 0x8004e424  name: RCpolyG3

undefined4 RCpolyG3(undefined4 param_1,int *param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  int iVar7;
  SVECTOR *r0;
  SVECTOR *pSVar8;
  short *psVar9;
  short *psVar10;
  short *psVar11;
  uint uVar12;
  int iVar13;
  int iVar14;
  undefined4 uVar15;
  
  r0 = (SVECTOR *)(param_2 + 0x18);
  iVar7 = 0;
  psVar9 = (short *)param_2[0x2a];
  psVar10 = (short *)param_2[0x2b];
  psVar11 = (short *)param_2[0x2c];
  iVar13 = gte_stH();
  uVar12 = iVar13 >> 1;
  if (((*(uint *)(psVar9 + 10) < uVar12) && (*(uint *)(psVar10 + 10) < uVar12)) &&
     (*(uint *)(psVar11 + 10) < uVar12)) {
    return param_1;
  }
  iVar14 = gte_stOFX();
  iVar13 = (iVar14 >> 0x10) + ((uint)param_2[1] >> 1);
  if (((iVar13 < psVar9[8]) && (iVar13 < psVar10[8])) && (iVar13 < psVar11[8])) {
    return param_1;
  }
  iVar13 = (iVar14 >> 0x10) - ((uint)param_2[1] >> 1);
  if (((psVar9[8] < iVar13) && (psVar10[8] < iVar13)) && (psVar11[8] < iVar13)) {
    return param_1;
  }
  iVar14 = gte_stOFY();
  iVar13 = (iVar14 >> 0x10) + ((uint)param_2[2] >> 1);
  if (((iVar13 < psVar9[9]) && (iVar13 < psVar10[9])) && (iVar13 < psVar11[9])) {
    return param_1;
  }
  iVar13 = (iVar14 >> 0x10) - ((uint)param_2[2] >> 1);
  if (((psVar9[9] < iVar13) && (psVar10[9] < iVar13)) && (psVar11[9] < iVar13)) {
    return param_1;
  }
  sVar4 = *psVar9;
  sVar5 = *psVar10;
  sVar6 = *psVar11;
  r0->vx = (short)((int)sVar4 + (int)sVar5 >> 1);
  *(short *)(param_2 + 0x1e) = (short)((int)sVar5 + (int)sVar6 >> 1);
  *(short *)(param_2 + 0x24) = (short)((int)sVar6 + (int)sVar4 >> 1);
  sVar4 = psVar9[1];
  sVar5 = psVar10[1];
  sVar6 = psVar11[1];
  *(short *)((int)param_2 + 0x62) = (short)((int)sVar4 + (int)sVar5 >> 1);
  *(short *)((int)param_2 + 0x7a) = (short)((int)sVar5 + (int)sVar6 >> 1);
  *(short *)((int)param_2 + 0x92) = (short)((int)sVar6 + (int)sVar4 >> 1);
  sVar4 = psVar9[2];
  sVar5 = psVar10[2];
  sVar6 = psVar11[2];
  *(short *)(param_2 + 0x19) = (short)((int)sVar4 + (int)sVar5 >> 1);
  *(short *)(param_2 + 0x1f) = (short)((int)sVar5 + (int)sVar6 >> 1);
  *(short *)(param_2 + 0x25) = (short)((int)sVar6 + (int)sVar4 >> 1);
  uVar15 = 0x8004e5f4;
  gte_ldv0(r0);
  gte_ldVXY1(r0 + 3);
  gte_ldVZ1(&r0[3].vz);
  gte_ldVXY2(r0 + 6);
  gte_ldVZ2(&r0[6].vz);
  bVar1 = *(byte *)(psVar9 + 6);
  bVar2 = *(byte *)(psVar10 + 6);
  bVar3 = *(byte *)(psVar11 + 6);
  gte_rtpt_b();
  *(char *)&r0[1].vz = (char)((uint)bVar1 + (uint)bVar2 >> 1);
  *(char *)&r0[4].vz = (char)((uint)bVar2 + (uint)bVar3 >> 1);
  *(char *)&r0[7].vz = (char)((uint)bVar3 + (uint)bVar1 >> 1);
  bVar1 = *(byte *)((int)psVar9 + 0xd);
  bVar2 = *(byte *)((int)psVar10 + 0xd);
  bVar3 = *(byte *)((int)psVar11 + 0xd);
  *(char *)((int)&r0[1].vz + 1) = (char)((uint)bVar1 + (uint)bVar2 >> 1);
  *(char *)((int)&r0[4].vz + 1) = (char)((uint)bVar2 + (uint)bVar3 >> 1);
  *(char *)((int)&r0[7].vz + 1) = (char)((uint)bVar3 + (uint)bVar1 >> 1);
  bVar1 = *(byte *)(psVar9 + 7);
  bVar2 = *(byte *)(psVar10 + 7);
  bVar3 = *(byte *)(psVar11 + 7);
  *(char *)&r0[1].pad = (char)((uint)bVar1 + (uint)bVar2 >> 1);
  *(char *)&r0[4].pad = (char)((uint)bVar2 + (uint)bVar3 >> 1);
  *(char *)&r0[7].pad = (char)((uint)bVar3 + (uint)bVar1 >> 1);
  if (*param_2 == iVar7 + 1) {
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    DIVG3A_OBJ_3AC();
    DIVG3A_OBJ_3AC();
    DIVG3A_OBJ_3AC();
    DIVG3A_OBJ_3AC();
  }
  else {
    gte_stSZ1();
    gte_stSZ2();
    gte_stSZ3();
    gte_stSXY0();
    gte_stSXY1();
    gte_stSXY2();
    pSVar8 = r0 + 0xb;
    r0[0x15].vz = (short)uVar15;
    r0[0x15].pad = (short)((uint)uVar15 >> 0x10);
    sVar4 = r0[9].vy;
    r0[0x14].vx = r0[9].vx;
    r0[0x14].vy = sVar4;
    *(SVECTOR **)&r0[0x14].vz = r0;
    *(SVECTOR **)(r0 + 0x15) = r0 + 6;
    RCpolyG3A();
    sVar4 = pSVar8[-0xffffffff00000002].pad;
    pSVar8[9].vx = pSVar8[-0xffffffff00000002].vz;
    pSVar8[9].vy = sVar4;
    *(SVECTOR **)&pSVar8[9].vz = pSVar8 + -8;
    *(SVECTOR **)(pSVar8 + 10) = pSVar8 + -0xb;
    RCpolyG3A();
    sVar4 = pSVar8[-0xffffffff00000001].vy;
    pSVar8[9].vx = pSVar8[-0xffffffff00000001].vx;
    pSVar8[9].vy = sVar4;
    *(SVECTOR **)&pSVar8[9].vz = pSVar8 + -5;
    *(SVECTOR **)(pSVar8 + 10) = pSVar8 + -8;
    RCpolyG3A();
    *(SVECTOR **)(pSVar8 + 9) = pSVar8 + -0xb;
    *(SVECTOR **)&pSVar8[9].vz = pSVar8 + -8;
    *(SVECTOR **)(pSVar8 + 10) = pSVar8 + -5;
    RCpolyG3A();
  }
  return param_1;
}

