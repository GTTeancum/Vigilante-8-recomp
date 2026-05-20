// addr: 0x80025bc0  name: FUN_80025bc0

void FUN_80025bc0(u_long *param_1,undefined4 *param_2,undefined4 param_3,undefined4 *param_4)

{
  uint uVar1;
  int iVar2;
  uint *puVar3;
  uint *puVar4;
  undefined1 *puVar5;
  u_long *puVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  u_long *puVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  int iVar16;
  undefined1 auStack_70 [64];
  int local_30;
  
  puVar14 = (undefined1 *)0x0;
  puVar15 = auStack_70;
  iVar13 = 0x90;
  iVar11 = gte_stSZ1();
  iVar12 = gte_stSZ2();
  iVar10 = gte_stSZ3();
  puVar3 = DAT_1f80001c;
  iVar16 = iVar12;
  iVar2 = iVar10;
  local_30 = iVar11;
  gte_strgb3_gt3(param_1);
  if (iVar11 < iVar13) {
    if (iVar16 < iVar13) {
      if (iVar10 < iVar13) {
        return;
      }
      uVar7 = param_4[1];
      puVar9 = param_1 + 9;
      *(undefined4 *)(puVar15 + 0x30) = *param_4;
      *(undefined4 *)(puVar15 + 0x34) = uVar7;
      *(u_long **)(puVar15 + 0x10) = param_1 + 3;
      *(u_long **)(puVar15 + 0x14) = puVar9;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar11) * 0x1000) / (iVar10 - iVar11);
      FUN_80025b20(puVar15 + 0x20);
      iVar16 = ((iVar13 - iVar16) * 0x1000) / (iVar10 - iVar16);
      uVar7 = *(undefined4 *)(puVar15 + 0x78);
      param_2 = *(undefined4 **)(puVar15 + 0x7c);
      puVar5 = puVar15 + 0x28;
      puVar6 = param_1 + 6;
    }
    else {
      if (0x8f < iVar10) {
        iVar2 = (iVar13 - iVar11) * 0x1000;
        uVar7 = (*(undefined4 **)(puVar15 + 0x7c))[1];
        *(undefined4 *)(puVar15 + 0x38) = **(undefined4 **)(puVar15 + 0x7c);
        *(undefined4 *)(puVar15 + 0x3c) = uVar7;
        *(short *)(puVar3 + 0xc) = (short)param_1[9];
        uVar7 = *(undefined4 *)(puVar15 + 0x7c);
        puVar3[10] = param_1[7];
        *(u_long **)(puVar15 + 0x10) = param_1 + 3;
        *(u_long **)(puVar15 + 0x14) = param_1 + 9;
        *(int *)(puVar15 + 0x18) = iVar2 / (iVar10 - iVar11);
        FUN_80025b20(puVar15 + 0x30,param_2,uVar7,puVar3 + 9);
        *(u_long **)(puVar15 + 0x10) = param_1 + 3;
        *(u_long **)(puVar15 + 0x14) = param_1 + 6;
        *(int *)(puVar15 + 0x18) = iVar2 / (iVar16 - iVar11);
        FUN_80025b20(puVar15 + 0x20,param_2,*(undefined4 *)(puVar15 + 0x78),puVar3 + 3);
        goto LAB_80025fd4;
      }
      puVar8 = *(undefined4 **)(puVar15 + 0x78);
      uVar7 = puVar8[1];
      puVar9 = param_1 + 6;
      *(undefined4 *)(puVar15 + 0x28) = *puVar8;
      *(undefined4 *)(puVar15 + 0x2c) = uVar7;
      *(u_long **)(puVar15 + 0x10) = param_1 + 3;
      *(u_long **)(puVar15 + 0x14) = puVar9;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar11) * 0x1000) / (iVar16 - iVar11);
      FUN_80025b20(puVar15 + 0x20,param_2,puVar8);
      iVar16 = ((iVar13 - iVar10) * 0x1000) / (iVar16 - iVar10);
      uVar7 = *(undefined4 *)(puVar15 + 0x7c);
      param_2 = *(undefined4 **)(puVar15 + 0x78);
LAB_80025e4c:
      puVar5 = puVar15 + 0x30;
      puVar6 = param_1 + 9;
    }
    *(u_long **)(puVar15 + 0x10) = puVar6;
    *(u_long **)(puVar15 + 0x14) = puVar9;
    *(int *)(puVar15 + 0x18) = iVar16;
    FUN_80025b20(puVar5,uVar7,param_2);
    gte_ldVXY0(puVar15 + 0x20);
    gte_ldVZ0(puVar15 + 0x24);
    gte_ldVXY1(puVar15 + 0x28);
    gte_ldVZ1(puVar15 + 0x2c);
    gte_ldVXY2(puVar15 + 0x30);
    gte_ldVZ2(puVar15 + 0x34);
    gte_rtpt();
    uVar1 = gte_stFLAG();
    if ((uVar1 & 0x7f85e000) == 0) {
      gte_nclip_b();
      uVar1 = gte_stMAC0();
      if (0 < (int)(uVar1 ^ *(uint *)(puVar15 + 0x80))) {
        iVar2 = gte_stSZ0();
        iVar16 = gte_stSZ1();
        if (iVar16 < iVar2) {
          iVar16 = iVar2;
        }
        iVar2 = gte_stSZ2();
        if (iVar2 < iVar16) {
          iVar2 = iVar16;
        }
        gte_stsxy3_gt3(param_1);
        *(undefined2 *)((int)param_1 + 0xe) = *(undefined2 *)(puVar14 + 0x800);
        if (iVar2 < 0) {
          iVar2 = iVar2 + 7;
        }
        puVar3 = (uint *)((iVar2 >> 3) * 4 + DAT_1f800000);
        uVar1 = *puVar3;
        *puVar3 = (uint)param_1 & 0xffffff;
        *param_1 = uVar1 | 0x9000000;
      }
    }
  }
  else {
    if (iVar16 < 0x90) {
      if (iVar10 < 0x90) {
        uVar7 = param_2[1];
        puVar9 = param_1 + 3;
        *(undefined4 *)(puVar15 + 0x20) = *param_2;
        *(undefined4 *)(puVar15 + 0x24) = uVar7;
        *(u_long **)(puVar15 + 0x10) = param_1 + 6;
        *(u_long **)(puVar15 + 0x14) = puVar9;
        *(int *)(puVar15 + 0x18) = ((iVar13 - iVar16) * 0x1000) / (iVar11 - iVar16);
        FUN_80025b20(puVar15 + 0x28,*(undefined4 *)(puVar15 + 0x78),param_2);
        iVar16 = ((iVar13 - iVar10) * 0x1000) / (iVar11 - iVar10);
        uVar7 = *(undefined4 *)(puVar15 + 0x7c);
        goto LAB_80025e4c;
      }
      *(u_long **)(puVar15 + 0x10) = param_1 + 9;
      *(u_long **)(puVar15 + 0x14) = param_1 + 6;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar10) * 0x1000) / (iVar16 - iVar10);
      FUN_80025b20(puVar15 + 0x38,*(undefined4 *)(puVar15 + 0x7c),*(undefined4 *)(puVar15 + 0x78),
                   puVar3 + 0xc);
      *(u_long **)(puVar15 + 0x10) = param_1 + 6;
      *(u_long **)(puVar15 + 0x14) = param_1 + 3;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar16) * 0x1000) / (iVar11 - iVar16);
      FUN_80025b20(puVar15 + 0x28,*(undefined4 *)(puVar15 + 0x78),param_2,puVar3 + 6);
      uVar7 = param_2[1];
      *(undefined4 *)(puVar15 + 0x20) = *param_2;
      *(undefined4 *)(puVar15 + 0x24) = uVar7;
      *(short *)(puVar3 + 3) = (short)param_1[3];
      puVar3[1] = param_1[1];
      uVar7 = (*(undefined4 **)(puVar15 + 0x7c))[1];
      *(undefined4 *)(puVar15 + 0x30) = **(undefined4 **)(puVar15 + 0x7c);
      *(undefined4 *)(puVar15 + 0x34) = uVar7;
      *(short *)(puVar3 + 9) = (short)param_1[9];
      puVar3[7] = param_1[7];
    }
    else {
      *(u_long **)(puVar15 + 0x10) = param_1 + 6;
      *(u_long **)(puVar15 + 0x14) = param_1 + 9;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar12) * 0x1000) / (iVar2 - iVar12);
      FUN_80025b20(puVar15 + 0x38,*(undefined4 *)(puVar15 + 0x78),*(undefined4 *)(puVar15 + 0x7c),
                   puVar3 + 0xc);
      *(u_long **)(puVar15 + 0x10) = param_1 + 9;
      *(u_long **)(puVar15 + 0x14) = param_1 + 3;
      *(int *)(puVar15 + 0x18) = ((iVar13 - iVar2) * 0x1000) / (*(int *)(puVar15 + 0x40) - iVar2);
      FUN_80025b20(puVar15 + 0x30,*(undefined4 *)(puVar15 + 0x7c),param_2,puVar3 + 9);
      uVar7 = param_2[1];
      *(undefined4 *)(puVar15 + 0x20) = *param_2;
      *(undefined4 *)(puVar15 + 0x24) = uVar7;
      *(short *)(puVar3 + 3) = (short)param_1[3];
      puVar3[1] = param_1[1];
LAB_80025fd4:
      uVar7 = (*(undefined4 **)(puVar15 + 0x78))[1];
      *(undefined4 *)(puVar15 + 0x28) = **(undefined4 **)(puVar15 + 0x78);
      *(undefined4 *)(puVar15 + 0x2c) = uVar7;
      *(short *)(puVar3 + 6) = (short)param_1[6];
      puVar3[4] = param_1[4];
    }
    gte_ldVXY0(puVar15 + 0x20);
    gte_ldVZ0(puVar15 + 0x24);
    gte_ldVXY1(puVar15 + 0x28);
    gte_ldVZ1(puVar15 + 0x2c);
    gte_ldVXY2(puVar15 + 0x30);
    gte_ldVZ2(puVar15 + 0x34);
    gte_rtpt();
                    /* Probable PsyQ macro: setPolyGT4() */
    *(undefined1 *)((int)puVar3 + 7) = 0x3c;
    *(undefined2 *)((int)puVar3 + 0x1a) = *(undefined2 *)((int)param_1 + 0x1a);
    uVar1 = gte_stFLAG();
    if ((uVar1 & 0x7f85e000) == 0) {
      gte_nclip_b();
      uVar1 = gte_stMAC0();
      if (0 < (int)(uVar1 ^ *(uint *)(puVar15 + 0x80))) {
        iVar2 = gte_stSZ0();
        iVar16 = gte_stSZ1();
        if (iVar16 < iVar2) {
          iVar16 = iVar2;
        }
        iVar2 = gte_stSZ2();
        if (iVar2 < iVar16) {
          iVar2 = iVar16;
        }
        gte_stsxy3_gt3(puVar3);
        gte_ldVXY0(puVar15 + 0x38);
        gte_ldVZ0(puVar15 + 0x3c);
        gte_rtps();
        *(undefined2 *)((int)puVar3 + 0xe) = *(undefined2 *)(puVar14 + 0x800);
        gte_stSXY2();
        if (iVar2 < 0) {
          iVar2 = iVar2 + 7;
        }
        puVar4 = (uint *)((iVar2 >> 3) * 4 + DAT_1f800000);
        uVar1 = *puVar4;
        *puVar4 = (uint)puVar3 & 0xffffff;
        *puVar3 = uVar1 | 0xc000000;
        DAT_1f80001c = DAT_1f80001c + 0xd;
      }
    }
  }
  return;
}

