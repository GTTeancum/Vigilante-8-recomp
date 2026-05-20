// addr: 0x80029750  name: FUN_80029750

/* WARNING: Removing unreachable block (ram,0x8002986c) */

void FUN_80029750(void)

{
  undefined2 uVar1;
  undefined2 uVar2;
  short sVar3;
  uint uVar4;
  int iVar5;
  undefined *puVar6;
  short sVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  uint *puVar11;
  uint *puVar12;
  undefined1 *puVar13;
  undefined1 *puVar14;
  undefined1 auStack_68 [16];
  MATRIX local_58;
  undefined1 local_38 [32];
  
  puVar13 = (undefined1 *)0x0;
  puVar14 = auStack_68;
  local_58.m[0]._0_4_ = DAT_8006f740;
  local_58.m._4_4_ = DAT_8006f744;
  local_58.m[1]._2_4_ = DAT_8006f748;
  local_58.m[2]._0_4_ = DAT_8006f74c;
  local_58._16_4_ = DAT_8006f750;
  local_58.t[0] = DAT_8006f754;
  local_58.t[1] = DAT_8006f758;
  local_58.t[2] = DAT_8006f75c;
  puVar11 = (uint *)(&DAT_800910c0 + iRam00000004 * 0x30);
  puVar12 = (uint *)(&DAT_80091020 + iRam00000004 * 0x50);
  uVar4 = FUN_80016c88(&local_58);
  RotMatrixY(-(int)(short)uVar4,&local_58);
  sVar3 = FUN_80016c54(&local_58);
  iVar5 = (int)sVar3;
  if (iVar5 < 0) {
    iVar5 = -iVar5;
  }
  if (0x400 < iVar5) {
    sVar3 = 0x800 - sVar3;
  }
  RotMatrixX(-(int)sVar3,&local_58);
  uVar4 = (uVar4 & 0xfff) * 5;
  local_58.t[0] = uVar4 - (uVar4 & 0xf000);
  local_58.t[1] = sVar3 * -5;
  local_58.t[2] = -0xd00;
  FUN_80016dfc(&local_58,local_38);
  FUN_80043974(local_38);
  puVar6 = &DAT_8005e9b4;
  gte_ldv3c((SVECTOR *)&DAT_8005e9b4);
  gte_rtpt();
  uVar4 = *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc);
  *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc) = (uint)puVar12 & 0xffffff;
  *puVar12 = (uint)*(byte *)((int)puVar12 + 3) << 0x18 | uVar4;
  gte_stSXY0();
  gte_stSXY1();
  gte_stSXY2();
  gte_stSXY1();
  gte_ldVXY0(puVar6 + 0x18);
  gte_ldVZ0(puVar6 + 0x1c);
  gte_ldVXY1(puVar6 + 0x20);
  gte_ldVZ1(puVar6 + 0x24);
  gte_ldVXY2(puVar6 + 0x28);
  gte_ldVZ2(puVar6 + 0x2c);
  gte_rtpt();
  uVar4 = *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc);
  *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc) = (uint)(puVar12 + 10) & 0xffffff;
  puVar12[10] = (uint)*(byte *)((int)puVar12 + 0x2b) << 0x18 | uVar4;
  gte_stSXY0();
  gte_stSXY1();
  gte_stSXY2();
  gte_stSXY1();
  iVar8 = (int)*(short *)(puVar14 + 0x30);
  iVar9 = ((int)*(short *)((int)puVar12 + 0x12) + (int)*(short *)((int)puVar12 + 0x22)) / 2;
  iVar5 = (int)*(short *)(puVar14 + 0x36);
  iVar10 = ((int)(short)puVar12[4] + (int)(short)puVar12[8]) / 2;
  sVar3 = (short)iVar9;
  if (iVar8 < 0xb51) {
    if (iVar8 < -0xb50) {
      iVar9 = *(int *)(puVar13 + 0x6d8);
      uVar1 = *(undefined2 *)(puVar13 + 0x6dc);
      uVar2 = *(undefined2 *)(puVar13 + 0x6d8);
      *(undefined2 *)((int)puVar11 + 0x16) = uVar1;
      *(undefined2 *)((int)puVar11 + 0x12) = uVar1;
      *(undefined2 *)((int)puVar11 + 0x2e) = 0;
      *(undefined2 *)((int)puVar11 + 0x2a) = 0;
      *(undefined2 *)(puVar11 + 4) = 0;
      *(undefined2 *)(puVar11 + 2) = 0;
      *(undefined2 *)(puVar11 + 10) = 0;
      *(undefined2 *)(puVar11 + 8) = 0;
      *(undefined2 *)(puVar11 + 5) = uVar2;
      *(undefined2 *)(puVar11 + 3) = uVar2;
      *(undefined2 *)(puVar11 + 0xb) = uVar2;
      *(undefined2 *)(puVar11 + 9) = uVar2;
      sVar7 = sVar3 + (short)((-iVar10 * iVar5) / iVar8);
      *(short *)((int)puVar11 + 0x22) = sVar7;
      *(short *)((int)puVar11 + 10) = sVar7;
      sVar3 = sVar3 + (short)(((iVar9 - iVar10) * iVar5) / iVar8);
      *(short *)((int)puVar11 + 0x26) = sVar3;
      *(short *)((int)puVar11 + 0xe) = sVar3;
    }
    else {
      sVar3 = (short)iVar10;
      if (iVar5 < 1) {
        iVar10 = *(int *)(puVar13 + 0x6dc);
        uVar1 = *(undefined2 *)(puVar13 + 0x6d8);
        uVar2 = *(undefined2 *)(puVar13 + 0x6dc);
        *(undefined2 *)(puVar11 + 5) = 0;
        *(undefined2 *)(puVar11 + 4) = 0;
        *(undefined2 *)(puVar11 + 0xb) = uVar1;
        *(undefined2 *)(puVar11 + 10) = uVar1;
        *(undefined2 *)((int)puVar11 + 0x12) = 0;
        *(undefined2 *)((int)puVar11 + 10) = 0;
        *(undefined2 *)((int)puVar11 + 0x2a) = 0;
        *(undefined2 *)((int)puVar11 + 0x22) = 0;
        *(undefined2 *)((int)puVar11 + 0x16) = uVar2;
        *(undefined2 *)((int)puVar11 + 0xe) = uVar2;
        *(undefined2 *)((int)puVar11 + 0x2e) = uVar2;
        *(undefined2 *)((int)puVar11 + 0x26) = uVar2;
        sVar7 = sVar3 + (short)((-iVar9 * iVar8) / iVar5);
        *(short *)(puVar11 + 8) = sVar7;
        *(short *)(puVar11 + 2) = sVar7;
        sVar3 = sVar3 + (short)(((iVar10 - iVar9) * iVar8) / iVar5);
        *(short *)(puVar11 + 9) = sVar3;
        *(short *)(puVar11 + 3) = sVar3;
      }
      else {
        iVar10 = *(int *)(puVar13 + 0x6dc);
        uVar1 = *(undefined2 *)(puVar13 + 0x6d8);
        uVar2 = *(undefined2 *)(puVar13 + 0x6dc);
        *(undefined2 *)(puVar11 + 5) = uVar1;
        *(undefined2 *)(puVar11 + 4) = uVar1;
        *(undefined2 *)(puVar11 + 0xb) = 0;
        *(undefined2 *)(puVar11 + 10) = 0;
        *(undefined2 *)((int)puVar11 + 0x12) = 0;
        *(undefined2 *)((int)puVar11 + 10) = 0;
        *(undefined2 *)((int)puVar11 + 0x2a) = 0;
        *(undefined2 *)((int)puVar11 + 0x22) = 0;
        *(undefined2 *)((int)puVar11 + 0x16) = uVar2;
        *(undefined2 *)((int)puVar11 + 0xe) = uVar2;
        *(undefined2 *)((int)puVar11 + 0x2e) = uVar2;
        *(undefined2 *)((int)puVar11 + 0x26) = uVar2;
        sVar7 = sVar3 + (short)((-iVar9 * iVar8) / iVar5);
        *(short *)(puVar11 + 8) = sVar7;
        *(short *)(puVar11 + 2) = sVar7;
        sVar3 = sVar3 + (short)(((iVar10 - iVar9) * iVar8) / iVar5);
        *(short *)(puVar11 + 9) = sVar3;
        *(short *)(puVar11 + 3) = sVar3;
      }
    }
  }
  else {
    iVar9 = *(int *)(puVar13 + 0x6d8);
    uVar1 = *(undefined2 *)(puVar13 + 0x6dc);
    uVar2 = *(undefined2 *)(puVar13 + 0x6d8);
    *(undefined2 *)((int)puVar11 + 0x16) = 0;
    *(undefined2 *)((int)puVar11 + 0x12) = 0;
    *(undefined2 *)((int)puVar11 + 0x2e) = uVar1;
    *(undefined2 *)((int)puVar11 + 0x2a) = uVar1;
    *(undefined2 *)(puVar11 + 4) = 0;
    *(undefined2 *)(puVar11 + 2) = 0;
    *(undefined2 *)(puVar11 + 10) = 0;
    *(undefined2 *)(puVar11 + 8) = 0;
    *(undefined2 *)(puVar11 + 5) = uVar2;
    *(undefined2 *)(puVar11 + 3) = uVar2;
    *(undefined2 *)(puVar11 + 0xb) = uVar2;
    *(undefined2 *)(puVar11 + 9) = uVar2;
    sVar7 = sVar3 + (short)((-iVar10 * iVar5) / iVar8);
    *(short *)((int)puVar11 + 0x22) = sVar7;
    *(short *)((int)puVar11 + 10) = sVar7;
    sVar3 = sVar3 + (short)(((iVar9 - iVar10) * iVar5) / iVar8);
    *(short *)((int)puVar11 + 0x26) = sVar3;
    *(short *)((int)puVar11 + 0xe) = sVar3;
  }
  uVar4 = *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc);
  *(uint *)(*(int *)(puVar13 + 0x60c) + 0x3ffc) = (uint)puVar11 & 0xffffff;
  *puVar11 = (uint)*(byte *)((int)puVar11 + 3) << 0x18 | uVar4;
  return;
}

