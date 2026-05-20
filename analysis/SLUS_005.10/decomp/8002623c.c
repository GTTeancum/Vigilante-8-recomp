// addr: 0x8002623c  name: FUN_8002623c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8002623c(uint param_1,uint param_2,uint param_3,int param_4)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  uint uVar5;
  undefined4 uVar6;
  uint uVar7;
  int iVar8;
  uint *puVar9;
  uint *puVar10;
  uint uVar11;
  short sVar12;
  int iVar13;
  uint uVar14;
  uint uVar15;
  int iVar16;
  uint uVar17;
  uint uVar18;
  uint uVar19;
  u_long *puVar20;
  uint *puVar21;
  undefined1 *puVar22;
  undefined1 *puVar23;
  undefined1 *puVar24;
  undefined1 *puVar25;
  undefined1 *puVar26;
  undefined1 *puVar27;
  undefined1 *puVar28;
  undefined1 *puVar29;
  undefined1 *puVar30;
  undefined1 *puVar31;
  undefined1 *puVar32;
  undefined1 *puVar33;
  undefined1 *puVar34;
  undefined1 *puVar35;
  undefined1 auStack_70 [16];
  undefined4 local_60;
  short local_58;
  short local_56;
  short local_54 [2];
  short local_50;
  short local_4e;
  short local_4c [2];
  short local_48;
  short local_46;
  short local_44 [2];
  short local_40;
  short local_3e;
  short local_3c [2];
  short local_38;
  short local_36;
  short local_34 [2];
  short local_30;
  short local_2e;
  short local_2c;
  
  puVar10 = DAT_1f800018;
  puVar22 = (undefined1 *)0x0;
  puVar34 = auStack_70;
  puVar33 = auStack_70;
  puVar30 = auStack_70;
  puVar32 = auStack_70;
  puVar31 = auStack_70;
  puVar29 = auStack_70;
  puVar28 = auStack_70;
  puVar27 = auStack_70;
  puVar26 = auStack_70;
  puVar24 = auStack_70;
  puVar25 = auStack_70;
  puVar23 = auStack_70;
  puVar35 = auStack_70;
  uVar19 = 1 << (param_1 & 0x1f);
  uVar7 = param_3 >> 6;
  uVar11 = param_2 >> 6;
  uVar17 = param_2 + uVar19 >> 6;
  iVar8 = (param_3 & 0x3f) * 2;
  iVar16 = (param_2 & 0x3f) * 0x80;
  uVar1 = *(ushort *)((&DAT_800911a0)[uVar11 * 0x20 + uVar7] + iVar8 + iVar16);
  uVar15 = (uint)uVar1;
  iVar13 = (param_2 + uVar19 & 0x3f) * 0x80;
  uVar2 = *(ushort *)((&DAT_800911a0)[uVar17 * 0x20 + uVar7] + iVar8 + iVar13);
  uVar14 = (uint)uVar2;
  uVar5 = param_3 + uVar19 >> 6;
  iVar8 = (param_3 + uVar19 & 0x3f) * 2;
  uVar3 = *(ushort *)((&DAT_800911a0)[uVar17 * 0x20 + uVar5] + iVar8 + iVar13);
  uVar17 = (uint)uVar3;
  uVar18 = uVar19 * 0x100;
  local_58 = (short)param_2 * 0x100 + (short)DAT_1f800004;
  uVar4 = *(ushort *)((&DAT_800911a0)[uVar11 * 0x20 + uVar5] + iVar8 + iVar16);
  uVar5 = (uint)uVar4;
  local_54[0] = (short)param_3 * 0x100 + (short)_DAT_1f80000c;
  sVar12 = (short)uVar18;
  local_50 = local_58 + sVar12;
  local_44[0] = local_54[0] + sVar12;
  local_2e = (short)DAT_1f800008;
  local_56 = local_2e + (uVar1 & 0x7ff) * 8;
  local_4e = local_2e + (uVar2 & 0x7ff) * 8;
  local_46 = local_2e + (uVar4 & 0x7ff) * 8;
  local_3e = local_2e + (uVar3 & 0x7ff) * 8;
  local_4c[0] = local_54[0];
  local_48 = local_58;
  local_40 = local_50;
  local_3c[0] = local_44[0];
  if (param_1 == 0) {
    iVar8 = (uint)*(byte *)((&DAT_800911a0)[uVar11 * 0x20 + uVar7] +
                            (param_3 & 0x3f) + (param_2 & 0x3f) * 0x40 + 0x2000) * 0x20;
    puVar21 = (uint *)(&DAT_8008f020 + iVar8);
    if (*(short *)(&DAT_8008f022 + iVar8) == 0) {
      return;
    }
    gte_ldRGB(0x34808080);
    if (*(short *)(&DAT_8008f03e + iVar8) == 0) {
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_ldVXY1(&local_50);
      gte_ldVZ1(local_4c);
      gte_ldVXY2(&local_48);
      gte_ldVZ2(local_44);
      gte_rtpt();
      puVar9 = DAT_1f800018 + 3;
      DAT_1f800018 = DAT_1f800018 + 0x14;
      *(short *)puVar9 = (short)*puVar21;
      puVar10[6] = *(uint *)(&DAT_8008f024 + iVar8);
      *(undefined2 *)(puVar10 + 9) = *(undefined2 *)(&DAT_8008f028 + iVar8);
      iVar13 = gte_stFLAG();
      if (-1 < iVar13) {
        gte_nclip_b();
        iVar13 = gte_stMAC0();
        if (iVar13 < 0) {
          uVar11 = gte_stSZ0();
          uVar7 = gte_stSZ1();
          if ((int)uVar7 < (int)uVar11) {
            uVar7 = uVar11;
          }
          uVar11 = gte_stSZ2();
          if ((int)uVar11 < (int)uVar7) {
            uVar11 = uVar7;
          }
          uVar6 = gte_stIR0();
          sVar12 = (short)((uint)uVar6 >> 8);
          gte_stsxy3_gt3(puVar10);
          gte_ldVXY0((uVar15 >> 0xb) << 7);
          gte_ldVXY1((uVar14 >> 0xb) << 7);
          gte_ldVXY2((uVar5 >> 0xb) << 7);
          gte_ncct();
          sVar12 = sVar12 * 0x40;
          puVar9 = (uint *)((uVar11 >> 3) * 4 + DAT_1f800000);
          *(short *)((int)puVar10 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12;
          uVar5 = *puVar9;
          uVar7 = 0x9000000;
          *puVar9 = (uint)puVar10 & 0xffffff;
          *puVar10 = uVar5 | 0x9000000;
          gte_strgb3_gt3(puVar10);
          gte_ldVXY0(puVar24 + 0x30);
          gte_ldVZ0(puVar24 + 0x34);
          gte_rtps();
          puVar20 = puVar10 + 10;
          *(short *)(puVar10 + 0xd) = (short)puVar21[1];
          puVar10[0x10] = puVar21[2];
          *(short *)(puVar10 + 0x13) = (short)puVar21[3];
          iVar8 = gte_stFLAG();
          if (-1 < iVar8) {
            gte_nclip_b();
            iVar8 = gte_stMAC0();
            if (0 < iVar8) {
              uVar11 = gte_stSZ0();
              uVar5 = gte_stSZ1();
              if ((int)uVar5 < (int)uVar11) {
                uVar5 = uVar11;
              }
              uVar11 = gte_stSZ2();
              if ((int)uVar11 < (int)uVar5) {
                uVar11 = uVar5;
              }
              gte_stsxy3_gt3(puVar20);
              gte_ldVXY0((uVar17 >> 0xb) << 7);
              gte_nccs();
              puVar10 = (uint *)((uVar11 >> 3) * 4 + DAT_1f800000);
              *(short *)((int)puVar20 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12;
              uVar5 = *puVar10;
              *puVar10 = (uint)puVar20 & 0xffffff;
              *puVar20 = uVar5 | uVar7;
              gte_strgb3_gt3(puVar20);
              return;
            }
          }
          uVar5 = gte_stFLAG();
          if ((uVar5 & 0x20000) == 0) {
            return;
          }
          gte_ldVXY0((uVar17 >> 0xb) << 7);
          gte_nccs();
          *(undefined4 *)(puVar24 + 0x10) = 0;
          puVar25 = puVar24;
          goto LAB_80026cf8;
        }
      }
      uVar5 = gte_stFLAG();
      if ((uVar5 & 0x20000) == 0) {
        gte_ldVXY0(&local_40);
        gte_ldVZ0(local_3c);
        gte_rtps();
      }
      else {
        gte_ldVXY0((uint)(uVar1 >> 0xb) << 7);
        gte_ldVXY1((uint)(uVar2 >> 0xb) << 7);
        gte_ldVXY2((uint)(uVar4 >> 0xb) << 7);
        gte_ncct();
        local_60 = 0xffffffff;
        FUN_80025bc0(puVar10,&local_58,&local_50,&local_48);
        gte_ldVXY0(&local_50);
        gte_ldVZ0(local_4c);
        gte_ldVXY1(&local_48);
        gte_ldVZ1(local_44);
        gte_ldVXY2(&local_40);
        gte_ldVZ2(local_3c);
        gte_rtpt();
      }
      puVar20 = puVar10 + 10;
      *(undefined2 *)(puVar10 + 0xd) = *(undefined2 *)(&DAT_8008f024 + iVar8);
      puVar10[0x10] = *(uint *)(&DAT_8008f028 + iVar8);
      *(undefined2 *)(puVar10 + 0x13) = *(undefined2 *)(&DAT_8008f02c + iVar8);
      gte_ldVXY0((uint)(uVar2 >> 0xb) << 7);
      gte_ldVXY1((uint)(uVar4 >> 0xb) << 7);
      gte_ldVXY2((uint)(uVar3 >> 0xb) << 7);
      iVar8 = gte_stFLAG();
      if (-1 < iVar8) {
        gte_nclip_b();
        iVar8 = gte_stMAC0();
        if (0 < iVar8) {
          uVar7 = gte_stSZ0();
          uVar5 = gte_stSZ1();
          if ((int)uVar5 < (int)uVar7) {
            uVar5 = uVar7;
          }
          uVar7 = gte_stSZ2();
          if ((int)uVar7 < (int)uVar5) {
            uVar7 = uVar5;
          }
          uVar6 = gte_stIR0();
          sVar12 = (short)((uint)uVar6 >> 8);
          gte_stsxy3_gt3(puVar20);
          gte_ncct();
          puVar10 = (uint *)((uVar7 >> 3) * 4 + DAT_1f800000);
          *(short *)((int)puVar20 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12 * 0x40;
          uVar5 = *puVar10;
          *puVar10 = (uint)puVar20 & 0xffffff;
          *puVar20 = uVar5 | 0x9000000;
          gte_strgb3_gt3(puVar20);
          return;
        }
      }
      uVar5 = gte_stFLAG();
      if ((uVar5 & 0x20000) == 0) {
        return;
      }
      gte_ncct();
      local_60 = 0;
LAB_80026cf8:
      FUN_80025bc0(puVar20,puVar25 + 0x20,puVar25 + 0x28,puVar25 + 0x30);
      return;
    }
    gte_ldVXY0(&local_50);
    gte_ldVZ0(local_4c);
    gte_ldVXY1(&local_40);
    gte_ldVZ1(local_3c);
    gte_ldVXY2(&local_58);
    gte_ldVZ2(local_54);
    gte_rtpt();
    puVar9 = DAT_1f800018 + 3;
    DAT_1f800018 = DAT_1f800018 + 0x14;
    *(undefined2 *)puVar9 = *(undefined2 *)(&DAT_8008f024 + iVar8);
    puVar10[6] = *(uint *)(&DAT_8008f02c + iVar8);
    *(short *)(puVar10 + 9) = (short)*puVar21;
    iVar13 = gte_stFLAG();
    if (-1 < iVar13) {
      gte_nclip_b();
      iVar13 = gte_stMAC0();
      if (iVar13 < 0) {
        uVar11 = gte_stSZ0();
        uVar7 = gte_stSZ1();
        if ((int)uVar7 < (int)uVar11) {
          uVar7 = uVar11;
        }
        uVar11 = gte_stSZ2();
        if ((int)uVar11 < (int)uVar7) {
          uVar11 = uVar7;
        }
        uVar6 = gte_stIR0();
        sVar12 = (short)((uint)uVar6 >> 8);
        gte_stsxy3_gt3(puVar10);
        gte_ldVXY0((uVar14 >> 0xb) << 7);
        gte_ldVXY1((uVar17 >> 0xb) << 7);
        gte_ldVXY2((uVar15 >> 0xb) << 7);
        gte_ncct();
        sVar12 = sVar12 * 0x40;
        puVar9 = (uint *)((uVar11 >> 3) * 4 + DAT_1f800000);
        *(short *)((int)puVar10 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12;
        uVar7 = *puVar9;
        uVar17 = 0x9000000;
        *puVar9 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar7 | 0x9000000;
        gte_strgb3_gt3(puVar10);
        gte_ldVXY0(puVar23 + 0x28);
        gte_ldVZ0(puVar23 + 0x2c);
        gte_rtps();
        puVar20 = puVar10 + 10;
        *(short *)(puVar10 + 0xd) = (short)puVar21[3];
        puVar10[0x10] = *puVar21;
        *(short *)(puVar10 + 0x13) = (short)puVar21[2];
        iVar8 = gte_stFLAG();
        if (-1 < iVar8) {
          gte_nclip_b();
          iVar8 = gte_stMAC0();
          if (0 < iVar8) {
            uVar11 = gte_stSZ0();
            uVar7 = gte_stSZ1();
            if ((int)uVar7 < (int)uVar11) {
              uVar7 = uVar11;
            }
            uVar11 = gte_stSZ2();
            if ((int)uVar11 < (int)uVar7) {
              uVar11 = uVar7;
            }
            gte_stsxy3_gt3(puVar20);
            gte_ldVXY0((uVar5 >> 0xb) << 7);
            gte_nccs();
            puVar10 = (uint *)((uVar11 >> 3) * 4 + DAT_1f800000);
            *(short *)((int)puVar20 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12;
            uVar5 = *puVar10;
            *puVar10 = (uint)puVar20 & 0xffffff;
            *puVar20 = uVar5 | uVar17;
            gte_strgb3_gt3(puVar20);
            return;
          }
        }
        uVar7 = gte_stFLAG();
        if ((uVar7 & 0x20000) == 0) {
          return;
        }
        gte_ldVXY0((uVar5 >> 0xb) << 7);
        gte_nccs();
        *(undefined4 *)(puVar23 + 0x10) = 0;
        puVar35 = puVar23;
        goto LAB_8002687c;
      }
    }
    uVar5 = gte_stFLAG();
    if ((uVar5 & 0x20000) == 0) {
      gte_ldVXY0(&local_48);
      gte_ldVZ0(local_44);
      gte_rtps();
    }
    else {
      gte_ldVXY0((uint)(uVar2 >> 0xb) << 7);
      gte_ldVXY1((uint)(uVar3 >> 0xb) << 7);
      gte_ldVXY2((uint)(uVar1 >> 0xb) << 7);
      gte_ncct();
      local_60 = 0xffffffff;
      FUN_80025bc0(puVar10,&local_50,&local_40,&local_58);
      gte_ldVXY0(&local_40);
      gte_ldVZ0(local_3c);
      gte_ldVXY1(&local_58);
      gte_ldVZ1(local_54);
      gte_ldVXY2(&local_48);
      gte_ldVZ2(local_44);
      gte_rtpt();
    }
    puVar20 = puVar10 + 10;
    *(undefined2 *)(puVar10 + 0xd) = *(undefined2 *)(&DAT_8008f02c + iVar8);
    puVar10[0x10] = *puVar21;
    *(undefined2 *)(puVar10 + 0x13) = *(undefined2 *)(&DAT_8008f028 + iVar8);
    gte_ldVXY0((uint)(uVar3 >> 0xb) << 7);
    gte_ldVXY1((uint)(uVar1 >> 0xb) << 7);
    gte_ldVXY2((uint)(uVar4 >> 0xb) << 7);
    iVar8 = gte_stFLAG();
    if (-1 < iVar8) {
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        uVar7 = gte_stSZ0();
        uVar5 = gte_stSZ1();
        if ((int)uVar5 < (int)uVar7) {
          uVar5 = uVar7;
        }
        uVar7 = gte_stSZ2();
        if ((int)uVar7 < (int)uVar5) {
          uVar7 = uVar5;
        }
        uVar6 = gte_stIR0();
        sVar12 = (short)((uint)uVar6 >> 8);
        gte_stsxy3_gt3(puVar20);
        gte_ncct();
        puVar10 = (uint *)((uVar7 >> 3) * 4 + DAT_1f800000);
        *(short *)((int)puVar20 + 0xe) = *(short *)(puVar22 + 0x800) + sVar12 * 0x40;
        uVar5 = *puVar10;
        *puVar10 = (uint)puVar20 & 0xffffff;
        *puVar20 = uVar5 | 0x9000000;
        gte_strgb3_gt3(puVar20);
        return;
      }
    }
    uVar5 = gte_stFLAG();
    if ((uVar5 & 0x20000) == 0) {
      return;
    }
    gte_ncct();
    local_60 = 0;
LAB_8002687c:
    FUN_80025bc0(puVar20,puVar35 + 0x30,puVar35 + 0x18,puVar35 + 0x28);
    return;
  }
  gte_ldRGB(0x828);
  if (param_4 == 0) {
    gte_ldVXY0(&local_48);
    gte_ldVZ0(local_44);
    gte_rtps();
    gte_stSXY2();
    gte_ldIR1((uint)(uVar4 >> 0xb) << 7);
    gte_cdp();
    gte_ldVXY0(&local_58);
    gte_ldVZ0(local_54);
    gte_rtps();
    gte_stSXY2();
    gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
    gte_cdp();
    gte_ldVXY0(&local_40);
    gte_ldVZ0(local_3c);
    gte_rtps();
    gte_stSXY2();
    gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
    gte_cdp();
    gte_nclip();
    iVar8 = gte_stMAC0();
    puVar10 = DAT_1f800014;
    puVar35 = auStack_70;
    if (iVar8 < 0) {
      gte_strgb3_g3(DAT_1f800014);
      gte_avsz3();
      uVar5 = gte_stOTZ();
      puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
      uVar5 = *puVar21;
      *puVar21 = (uint)puVar10 & 0xffffff;
      *puVar10 = uVar5 | 0x6000000;
      puVar10 = puVar10 + 7;
      puVar35 = puVar26;
    }
    gte_ldVXY0(puVar35 + 0x20);
    gte_ldVZ0(puVar35 + 0x24);
    gte_rtps();
    gte_nclip();
    iVar8 = gte_stMAC0();
    if (iVar8 < 1) {
      DAT_1f800014 = puVar10;
      return;
    }
    gte_ldIR1((uVar14 >> 0xb) << 7);
    gte_cdp();
    gte_stsxy3_g3(puVar10);
    gte_strgb3_g3(puVar10);
    gte_avsz3();
    uVar5 = gte_stOTZ();
    puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
    uVar5 = *puVar21;
    *puVar21 = (uint)puVar10 & 0xffffff;
    *puVar10 = uVar5 | 0x6000000;
LAB_800288c4:
    puVar10 = puVar10 + 7;
  }
  else {
    puVar10 = DAT_1f800014;
    local_38 = local_58;
    local_2c = local_54[0];
    switch(param_4) {
    case 1:
      param_3 = param_3 + (uVar19 >> 1);
      uVar3 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 >> 6)] +
               (param_3 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      local_34[0] = local_54[0] + (short)(uVar18 >> 1);
      local_36 = local_2e + (uVar3 & 0x7ff) * 8;
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_50);
      gte_ldVZ0(local_4c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      puVar35 = auStack_70;
      if (iVar8 < 0) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar21 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar7 | 0x6000000;
        puVar10 = puVar10 + 7;
        puVar35 = puVar27;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar17 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar21 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar7 | 0x6000000;
        puVar10 = puVar10 + 7;
      }
      gte_ldVXY0(puVar35 + 0x28);
      gte_ldVZ0(puVar35 + 0x2c);
      gte_rtps();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_avsz3();
        uVar7 = gte_stOTZ();
        gte_ldIR1((uVar5 >> 0xb) << 7);
        gte_cdp();
        puVar21 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        puVar10 = puVar10 + 7;
      }
      break;
    case 2:
      param_3 = param_3 + (uVar19 >> 1);
      uVar3 = *(ushort *)
               ((&DAT_800911a0)[(param_2 + uVar19 >> 6) * 0x20 + (param_3 >> 6)] +
               (param_3 & 0x3f) * 2 + (param_2 + uVar19 & 0x3f) * 0x80);
      local_38 = local_58 + sVar12;
      local_34[0] = local_54[0] + (short)(uVar18 >> 1);
      local_36 = local_2e + (uVar3 & 0x7ff) * 8;
      gte_ldVXY0(&local_50);
      gte_ldVZ0(local_4c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      puVar35 = auStack_70;
      if (0 < iVar8) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar21 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar7 | 0x6000000;
        puVar10 = puVar10 + 7;
        puVar35 = puVar28;
      }
      gte_ldVXY0(puVar35 + 0x28);
      gte_ldVZ0(puVar35 + 0x2c);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar5 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        puVar10 = puVar10 + 7;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_avsz3();
        uVar5 = gte_stOTZ();
        gte_ldIR1((uVar17 >> 0xb) << 7);
        gte_cdp();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        puVar10 = puVar10 + 7;
      }
      break;
    case 4:
      param_2 = param_2 + (uVar19 >> 1);
      uVar2 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 + uVar19 >> 6)] +
               (param_3 + uVar19 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      local_38 = local_58 + (short)(uVar18 >> 1);
      local_34[0] = local_54[0] + sVar12;
      local_36 = local_2e + (uVar2 & 0x7ff) * 8;
      gte_ldVXY0(&local_48);
      gte_ldVZ0(local_44);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar4 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      puVar35 = auStack_70;
      if (iVar8 < 0) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        puVar10 = puVar10 + 7;
        puVar35 = puVar29;
      }
      gte_ldVXY0(puVar35 + 0x20);
      gte_ldVZ0(puVar35 + 0x24);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar14 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        puVar10 = puVar10 + 7;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_avsz3();
        uVar5 = gte_stOTZ();
        gte_ldIR1((uVar17 >> 0xb) << 7);
        gte_cdp();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        puVar10 = puVar10 + 7;
      }
      break;
    case 5:
      uVar7 = param_3 + (uVar19 >> 1);
      uVar3 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (uVar7 >> 6)] +
               (uVar7 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      local_30 = (short)(uVar18 >> 1);
      local_34[0] = local_54[0] + local_30;
      local_36 = local_2e + (uVar3 & 0x7ff) * 8;
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_50);
      gte_ldVZ0(local_4c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      param_2 = param_2 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 + uVar19 >> 6)] +
               (param_3 + uVar19 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      uVar7 = (uint)uVar1;
      local_30 = local_58 + local_30;
      local_2c = local_54[0] + sVar12;
      local_2e = local_2e + (uVar1 & 0x7ff) * 8;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      puVar21 = DAT_1f800014;
      puVar35 = auStack_70;
      if (0 < iVar8) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar11 = gte_stOTZ();
        puVar10 = (uint *)((uVar11 >> 1) * 4 + DAT_1f800000);
        uVar11 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar11 | 0x6000000;
        puVar21 = puVar21 + 7;
        puVar35 = puVar31;
      }
      gte_ldVXY0(puVar35 + 0x40);
      gte_ldVZ0(puVar35 + 0x44);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar7 >> 0xb) << 7);
      gte_cdp();
      gte_stRGB0();
      gte_stSXY0();
      gte_stRGB2();
      gte_stSXY2();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar10 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar7 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar17 >> 0xb) << 7);
      gte_cdp();
      puVar21 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar10 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar7 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x28);
      gte_ldVZ0(puVar35 + 0x2c);
      gte_rtps();
      gte_stSXY2();
      gte_ldSXY0(puVar21 + 9);
      gte_ldSXY1(puVar21 + 0xb);
      puVar10 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_avsz3();
        uVar7 = gte_stOTZ();
        gte_ldIR1((uVar5 >> 0xb) << 7);
        gte_cdp();
        puVar9 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar9;
        *puVar9 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stRGB2();
        puVar10 = puVar21 + 0xe;
      }
      break;
    case 6:
      uVar7 = param_3 + (uVar19 >> 1);
      uVar3 = *(ushort *)
               ((&DAT_800911a0)[(param_2 + uVar19 >> 6) * 0x20 + (uVar7 >> 6)] +
               (uVar7 & 0x3f) * 2 + (param_2 + uVar19 & 0x3f) * 0x80);
      local_38 = local_58 + sVar12;
      local_30 = (short)(uVar18 >> 1);
      local_34[0] = local_54[0] + local_30;
      local_36 = local_2e + (uVar3 & 0x7ff) * 8;
      gte_ldVXY0(&local_50);
      gte_ldVZ0(local_4c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      param_2 = param_2 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 + uVar19 >> 6)] +
               (param_3 + uVar19 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      uVar7 = (uint)uVar1;
      local_30 = local_58 + local_30;
      local_2c = local_54[0] + sVar12;
      local_2e = local_2e + (uVar1 & 0x7ff) * 8;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      puVar21 = DAT_1f800014;
      puVar35 = auStack_70;
      if (iVar8 < 0) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar11 = gte_stOTZ();
        puVar10 = (uint *)((uVar11 >> 1) * 4 + DAT_1f800000);
        uVar11 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar11 | 0x6000000;
        puVar21 = puVar21 + 7;
        puVar35 = puVar32;
      }
      gte_ldVXY0(puVar35 + 0x40);
      gte_ldVZ0(puVar35 + 0x44);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar7 >> 0xb) << 7);
      gte_cdp();
      gte_stRGB0();
      gte_stSXY0();
      gte_stRGB2();
      gte_stSXY2();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar10 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar7 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x28);
      gte_ldVZ0(puVar35 + 0x2c);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar5 >> 0xb) << 7);
      gte_cdp();
      puVar21 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar10 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar5 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_stSXY2();
      gte_ldSXY0(puVar21 + 9);
      gte_ldSXY1(puVar21 + 0xb);
      puVar10 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_avsz3();
        uVar5 = gte_stOTZ();
        gte_ldIR1((uVar17 >> 0xb) << 7);
        gte_cdp();
        puVar9 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar9;
        *puVar9 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stRGB2();
        puVar10 = puVar21 + 0xe;
      }
      break;
    case 8:
      param_2 = param_2 + (uVar19 >> 1);
      uVar2 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 >> 6)] +
               (param_3 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      local_38 = local_58 + (short)(uVar18 >> 1);
      local_36 = local_2e + (uVar2 & 0x7ff) * 8;
      gte_ldVXY0(&local_58);
      gte_ldVZ0(local_54);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_48);
      gte_ldVZ0(local_44);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar4 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar2 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      puVar35 = auStack_70;
      local_34[0] = local_54[0];
      if (0 < iVar8) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        puVar10 = puVar10 + 7;
        puVar35 = puVar30;
      }
      gte_ldVXY0(puVar35 + 0x30);
      gte_ldVZ0(puVar35 + 0x34);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar17 >> 0xb) << 7);
      gte_cdp();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        puVar10 = puVar10 + 7;
      }
      gte_ldVXY0(puVar35 + 0x20);
      gte_ldVZ0(puVar35 + 0x24);
      gte_rtps();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_avsz3();
        uVar5 = gte_stOTZ();
        gte_ldIR1((uVar14 >> 0xb) << 7);
        gte_cdp();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        puVar10 = puVar10 + 7;
      }
      break;
    case 9:
      uVar5 = param_3 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (uVar5 >> 6)] +
               (uVar5 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      local_30 = (short)(uVar18 >> 1);
      local_34[0] = local_54[0] + local_30;
      local_36 = local_2e + (uVar1 & 0x7ff) * 8;
      gte_ldVXY0(&local_48);
      gte_ldVZ0(local_44);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar4 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_40);
      gte_ldVZ0(local_3c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      param_2 = param_2 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 >> 6)] +
               (param_3 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      uVar5 = (uint)uVar1;
      local_30 = local_58 + local_30;
      local_2e = local_2e + (uVar1 & 0x7ff) * 8;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      puVar21 = DAT_1f800014;
      puVar35 = auStack_70;
      if (iVar8 < 0) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar10 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar7 | 0x6000000;
        puVar21 = puVar21 + 7;
        puVar35 = puVar33;
      }
      gte_ldVXY0(puVar35 + 0x40);
      gte_ldVZ0(puVar35 + 0x44);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar5 >> 0xb) << 7);
      gte_cdp();
      gte_stRGB0();
      gte_stSXY0();
      gte_stRGB2();
      gte_stSXY2();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar10 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar5 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x20);
      gte_ldVZ0(puVar35 + 0x24);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar14 >> 0xb) << 7);
      gte_cdp();
      puVar21 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar21);
        gte_strgb3_g3(puVar21);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar10 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar10;
        *puVar10 = (uint)puVar21 & 0xffffff;
        *puVar21 = uVar5 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x18);
      gte_ldVZ0(puVar35 + 0x1c);
      gte_rtps();
      gte_stSXY2();
      gte_ldSXY0(puVar21 + 9);
      gte_ldSXY1(puVar21 + 0xb);
      puVar10 = puVar21 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_avsz3();
        uVar5 = gte_stOTZ();
        gte_ldIR1((uVar15 >> 0xb) << 7);
        gte_cdp();
        puVar9 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar9;
        *puVar9 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
        gte_stRGB2();
        puVar10 = puVar21 + 0xe;
      }
      break;
    case 10:
      uVar5 = param_3 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 + uVar19 >> 6) * 0x20 + (uVar5 >> 6)] +
               (uVar5 & 0x3f) * 2 + (param_2 + uVar19 & 0x3f) * 0x80);
      local_38 = local_58 + sVar12;
      local_30 = (short)(uVar18 >> 1);
      local_34[0] = local_54[0] + local_30;
      local_36 = local_2e + (uVar1 & 0x7ff) * 8;
      gte_ldVXY0(&local_40);
      gte_ldVZ0(local_3c);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar3 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_38);
      gte_ldVZ0(local_34);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar1 >> 0xb) << 7);
      gte_cdp();
      gte_ldVXY0(&local_48);
      gte_ldVZ0(local_44);
      gte_rtps();
      gte_stSXY2();
      gte_ldIR1((uint)(uVar4 >> 0xb) << 7);
      gte_cdp();
      param_2 = param_2 + (uVar19 >> 1);
      uVar1 = *(ushort *)
               ((&DAT_800911a0)[(param_2 >> 6) * 0x20 + (param_3 >> 6)] +
               (param_3 & 0x3f) * 2 + (param_2 & 0x3f) * 0x80);
      uVar5 = (uint)uVar1;
      local_30 = local_58 + local_30;
      local_2e = local_2e + (uVar1 & 0x7ff) * 8;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      puVar35 = auStack_70;
      if (0 < iVar8) {
        gte_strgb3_g3(DAT_1f800014);
        gte_avsz3();
        uVar7 = gte_stOTZ();
        puVar21 = (uint *)((uVar7 >> 1) * 4 + DAT_1f800000);
        uVar7 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar7 | 0x6000000;
        puVar10 = puVar10 + 7;
        puVar35 = puVar34;
      }
      gte_ldVXY0(puVar35 + 0x40);
      gte_ldVZ0(puVar35 + 0x44);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar5 >> 0xb) << 7);
      gte_cdp();
      gte_stRGB0();
      gte_stSXY0();
      gte_stRGB2();
      gte_stSXY2();
      gte_nclip();
      iVar8 = gte_stMAC0();
      if (iVar8 < 0) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x18);
      gte_ldVZ0(puVar35 + 0x1c);
      gte_rtps();
      gte_stOTZ();
      gte_ldIR1((uVar15 >> 0xb) << 7);
      gte_cdp();
      puVar10 = puVar10 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (0 < iVar8) {
        gte_stsxy3_g3(puVar10);
        gte_strgb3_g3(puVar10);
        gte_avsz3();
        uVar5 = gte_stOTZ();
        puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
        uVar5 = *puVar21;
        *puVar21 = (uint)puVar10 & 0xffffff;
        *puVar10 = uVar5 | 0x6000000;
      }
      gte_ldVXY0(puVar35 + 0x20);
      gte_ldVZ0(puVar35 + 0x24);
      gte_rtps();
      gte_stSXY2();
      gte_ldSXY0(puVar10 + 9);
      gte_ldSXY1(puVar10 + 0xb);
      puVar10 = puVar10 + 7;
      gte_nclip_b();
      iVar8 = gte_stMAC0();
      if (-1 < iVar8) {
        DAT_1f800014 = puVar10;
        return;
      }
      gte_avsz3();
      uVar5 = gte_stOTZ();
      gte_ldIR1((uVar14 >> 0xb) << 7);
      gte_cdp();
      puVar21 = (uint *)((uVar5 >> 1) * 4 + DAT_1f800000);
      uVar5 = *puVar21;
      *puVar21 = (uint)puVar10 & 0xffffff;
      *puVar10 = uVar5 | 0x6000000;
      gte_stRGB2();
      goto LAB_800288c4;
    }
  }
  DAT_1f800014 = puVar10;
  return;
}

