// addr: 0x8002f9bc  name: FUN_8002f9bc

void FUN_8002f9bc(uint *param_1)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  uint uVar14;
  uint uVar15;
  uint uVar16;
  int iVar17;
  int iVar18;
  undefined2 *puVar19;
  int iVar20;
  int iVar21;
  uint *puVar22;
  int iVar23;
  int local_108;
  int local_104;
  int local_100;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_e8;
  int local_e4;
  int local_e0;
  undefined4 local_d8;
  int local_d4;
  undefined4 local_d0;
  int local_c8;
  int local_c4;
  int local_c0;
  uint local_b8;
  int local_b4;
  uint local_b0;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  int local_98;
  uint local_94;
  int local_90;
  int local_88;
  int local_84;
  int local_80;
  int local_78;
  undefined4 local_74;
  int local_70;
  int local_6c;
  int local_68;
  int local_60;
  uint *local_5c;
  int local_58;
  int *local_54;
  int *local_50;
  uint *local_4c;
  int local_48;
  undefined8 local_40;
  longlong local_38;
  int local_30;
  
  *param_1 = *param_1 & 0xff8fffff;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_f8 = 0;
  local_f4 = 0;
  local_f0 = 0;
  uVar14 = param_1[0x3f];
  uVar6 = param_1[0x29];
  local_5c = param_1 + 0x29;
  *(short *)(param_1[0x40] + 0x42) = (short)uVar6;
  *(short *)(uVar14 + 0x42) = (short)uVar6;
  iVar4 = FUN_80016a20(param_1 + 0x20);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x7f;
  }
  param_1[0x23] = iVar4 >> 7;
  if ((short)param_1[6] < 1) {
    uVar6 = param_1[0x17];
    uVar8 = 0;
    uVar14 = 0;
    do {
      if (uVar14 == 0) {
        local_88 = *(int *)(uVar6 + 0x10);
      }
      else {
        local_88 = *(int *)(uVar6 + 4);
      }
      if ((uVar8 & 2) == 0) {
        local_84 = *(int *)(uVar6 + 0x14);
      }
      else {
        local_84 = *(int *)(uVar6 + 8);
      }
      if ((uVar8 & 4) == 0) {
        local_80 = *(int *)(uVar6 + 0x18);
      }
      else {
        local_80 = *(int *)(uVar6 + 0xc);
      }
      FUN_80043408(param_1 + 4,&local_88,&local_88);
      iVar4 = FUN_8001d748(param_1,&local_88,0,&local_60);
      if (0 < local_84 - iVar4) {
        iVar20 = -param_1[0x20];
        if (iVar20 < 0) {
          iVar20 = iVar20 + 3;
        }
        uVar14 = iVar20 >> 2;
        if ((int)uVar14 < -0xb40) {
          local_b8 = 0xfffff4c0;
        }
        else {
          local_b8 = 0xb40;
          if ((int)uVar14 < 0xb41) {
            local_b8 = uVar14;
          }
        }
        iVar20 = -param_1[0x22];
        if (iVar20 < 0) {
          iVar20 = iVar20 + 3;
        }
        uVar14 = iVar20 >> 2;
        if ((int)uVar14 < -0xb40) {
          local_b0 = 0xfffff4c0;
        }
        else {
          local_b0 = 0xb40;
          if ((int)uVar14 < 0xb41) {
            local_b0 = uVar14;
          }
        }
        local_b4 = -(local_84 - iVar4);
        if (0 < (int)param_1[0x21]) {
          local_b4 = local_b4 - ((int)param_1[0x21] >> 2);
        }
        gte_ldR11R12((int)(local_88 - param_1[0x12]) >> 3);
        gte_ldR22R23((int)(local_84 - param_1[0x13]) >> 3);
        gte_ldR33((int)(local_80 - param_1[0x14]) >> 3);
        gte_ldsv_((int)local_b8 >> 3,local_b4 >> 3,(int)local_b0 >> 3);
        gte_op12();
        local_f8 = local_f8 + local_b8;
        local_f4 = local_f4 + local_b4;
        local_f0 = local_f0 + local_b0;
        iVar4 = gte_stMAC1();
        local_108 = local_108 + iVar4;
        iVar4 = gte_stMAC2();
        local_104 = local_104 + iVar4;
        iVar4 = gte_stMAC3();
        local_100 = local_100 + iVar4;
        if (((local_60 != 0) && (*(short *)(local_60 + 0x16) != 0)) &&
           (*(short *)(local_60 + 0x16) != 7)) {
          (*pcRam00000730)(param_1,9,&local_88,param_1[0x14]);
        }
        if (0x4c00 < (int)param_1[0x21]) {
          *param_1 = *param_1 | 0x400000;
        }
      }
      uVar8 = uVar8 + 1;
      uVar14 = uVar8 & 1;
    } while ((int)uVar8 < 8);
    puVar22 = param_1 + 4;
    FUN_8004352c(puVar22,&local_108,&local_108);
    iVar4 = 4;
    do {
      uVar6 = puVar22[0x3b];
      iVar21 = *(int *)(uVar6 + 0x98);
      *(undefined4 *)(uVar6 + 0x4c) = *(undefined4 *)(uVar6 + 0x84);
      iVar20 = iVar21;
      if (iVar21 < 0) {
        iVar20 = iVar21 + 0x3f;
      }
      iVar21 = iVar21 - (iVar20 >> 6);
      *(int *)(uVar6 + 0x98) = iVar21;
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0xfff;
      }
      iVar20 = (iVar21 >> 0xc) * *(int *)(uVar6 + 0x94);
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0x7ffff;
      }
      puVar22 = puVar22 + 1;
      iVar4 = iVar4 + 1;
      *(short *)(uVar6 + 0x40) = *(short *)(uVar6 + 0x40) - (short)(iVar20 >> 0x13);
      FUN_8001d708();
    } while (iVar4 < 8);
  }
  else {
    FUN_8004352c(param_1 + 4,param_1 + 0x20,&local_e8);
    iVar23 = 0;
    puVar19 = &DAT_1f80000c;
    iVar4 = 0x1f800000;
    iVar21 = 0;
    iVar20 = 0x10;
    do {
      iVar11 = *(int *)((int)param_1 + iVar20 + 0xec);
      local_d8 = *(undefined4 *)(iVar11 + 0x48);
      local_d4 = *(int *)(iVar11 + 0x4c) + *(int *)(iVar11 + 0x90);
      local_d0 = *(undefined4 *)(iVar11 + 0x50);
      iVar20 = iVar20 + 4;
      iVar23 = iVar23 + 1;
      FUN_80043408(param_1 + 4,&local_d8,iVar4);
      uVar5 = FUN_8001d748(param_1,iVar4,(int)&DAT_1f800010 + iVar21,puVar19);
      puVar19 = puVar19 + 0xc;
      *(undefined4 *)(iVar4 + 4) = uVar5;
      iVar4 = iVar4 + 0x18;
      iVar21 = iVar21 + 0x18;
    } while (iVar23 < 4);
    local_54 = &local_88;
    local_50 = &local_78;
    local_4c = param_1 + 0x20;
    iVar20 = 4;
    iVar4 = 0;
    do {
      puVar22 = (uint *)param_1[iVar20 + 0x3b];
      uVar6 = *(ushort *)((int)puVar22 + 0x42) & 0xfff;
      iVar21 = (int)(short)(&DAT_800607b4)[uVar6 * 2];
      local_58 = (int)(short)(&DAT_800607b6)[uVar6 * 2];
      local_b8 = puVar22[0x12];
      local_b4 = puVar22[0x13] + puVar22[0x24];
      local_b0 = puVar22[0x14];
      iVar20 = param_1[0x25] * puVar22[0x14];
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0xfff;
      }
      local_88 = local_e8 + (iVar20 >> 0xc);
      local_54[1] = local_e4;
      iVar20 = param_1[0x25] * puVar22[0x12];
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0xfff;
      }
      local_54[2] = local_e0 - (iVar20 >> 0xc);
      local_a8 = *(undefined4 *)(iVar4 * 0x18 + 0x1f800000);
      local_a4 = (&DAT_1f800004)[iVar4 * 6];
      local_a0 = (&DAT_1f800008)[iVar4 * 6];
      local_78 = (&DAT_1f800010)[iVar4 * 6];
      local_74 = (&DAT_1f800014)[iVar4 * 6];
      local_48 = *(int *)(&DAT_1f80000c + iVar4 * 0xc);
      FUN_800435c0(param_1 + 4,&local_a8,&local_98);
      local_94 = local_94 - puVar22[0x24];
      if ((int)local_94 < (int)puVar22[0x21]) {
        uVar6 = 0x100000;
        if (local_48 != 0) {
          uVar6 = 0x300000;
        }
        *param_1 = *param_1 | uVar6;
        uVar8 = param_1[0x20];
        uVar14 = (local_78 << 0x10) >> 0x10;
        lVar1 = (ulonglong)uVar14 * (ulonglong)uVar8;
        uVar9 = local_4c[1];
        iVar20 = (uint)*(ushort *)((int)local_50 + 2) << 0x10;
        uVar16 = iVar20 >> 0x10;
        lVar2 = (ulonglong)uVar16 * (ulonglong)uVar9;
        uVar15 = local_4c[2];
        uVar12 = (int)((uint)*(ushort *)(local_50 + 1) << 0x10) >> 0x10;
        lVar3 = (ulonglong)uVar12 * (ulonglong)uVar15;
        uVar6 = (uint)lVar3;
        local_30 = uVar14 * ((int)uVar8 >> 0x1f);
        local_40._4_4_ = (int)((ulonglong)lVar2 >> 0x20);
        uVar14 = (uint)lVar2;
        iVar20 = local_40._4_4_ + uVar16 * ((int)uVar9 >> 0x1f) + uVar9 * (iVar20 >> 0x1f);
        local_40 = CONCAT44(iVar20,uVar14);
        uVar9 = (int)lVar1 + uVar14;
        uVar16 = uVar9 + uVar6;
        uVar6 = uVar16 >> 0xf |
                ((int)((ulonglong)lVar1 >> 0x20) + local_30 + uVar8 * ((local_78 << 0x10) >> 0x1f) +
                 iVar20 + (uint)(uVar9 < uVar14) +
                 (int)((ulonglong)lVar3 >> 0x20) + uVar12 * ((int)uVar15 >> 0x1f) +
                 uVar15 * ((int)((uint)*(ushort *)(local_50 + 1) << 0x10) >> 0x1f) +
                (uint)(uVar16 < uVar6)) * 0x20000;
        FUN_800434d0(param_1 + 4,local_50,&local_70);
        iVar20 = -local_70 * uVar6;
        if (iVar20 < 0) {
          iVar20 = iVar20 + 0xfff;
        }
        iVar23 = 0;
        if ((int)(puVar22[0x12] - local_98) < 0) {
          iVar23 = puVar22[0x12] - local_98;
        }
        iVar11 = -local_68 * uVar6;
        if (iVar11 < 0) {
          iVar11 = iVar11 + 0xfff;
        }
        iVar10 = 0;
        if ((int)(puVar22[0x14] - local_90) < 0) {
          iVar10 = puVar22[0x14] - local_90;
        }
        uVar6 = puVar22[0x20];
        if ((int)puVar22[0x20] < (int)local_94) {
          uVar6 = local_94;
        }
        uVar8 = puVar22[0x21];
        uVar14 = puVar22[0x23];
        if (((int)puVar22[0x20] < (int)local_94) || ((int)puVar22[0x13] < (int)local_94)) {
          local_c4 = (local_94 - puVar22[0x13]) * (int)*(short *)((int)puVar22 + 0x8e);
          if (local_c4 < 0) {
            local_c4 = local_c4 + 0x1f;
          }
          local_c4 = local_c4 >> 5;
        }
        else {
          local_c4 = (local_94 - puVar22[0x13]) * 0x10;
          *param_1 = *param_1 | 0x400000;
        }
        local_c4 = (int)((uVar8 - uVar6) * (int)(short)uVar14 * 0x80) / local_6c + local_c4;
        puVar22[0x13] = local_94;
        if ((local_48 == 0) || (*(short *)(local_48 + 0x10) == 0)) {
          iVar17 = local_c4 * -2;
        }
        else {
          iVar17 = -local_c4 * (0x100 - *(short *)(local_48 + 0x10)) >> 7;
        }
        if ((*puVar22 & 0x20000) == 0) {
          uVar6 = local_88 >> 5;
          uVar14 = local_e0 >> 2;
        }
        else {
          local_38 = (longlong)local_e0 * (longlong)local_58;
          lVar1 = (longlong)local_88 * (longlong)local_58 - (longlong)local_80 * (longlong)iVar21;
          uVar6 = (uint)lVar1 >> 0x11 | (int)((ulonglong)lVar1 >> 0x20) * 0x8000;
          lVar1 = (longlong)local_e8 * (longlong)iVar21 + local_38;
          uVar14 = (uint)lVar1 >> 0xe | (int)((ulonglong)lVar1 >> 0x20) * 0x40000;
        }
        iVar13 = (int)*(short *)((int)local_5c + 2);
        iVar18 = iVar13;
        if (iVar13 < 0) {
          iVar18 = -iVar13;
        }
        iVar7 = iVar17;
        if (iVar18 << 6 < iVar17) {
          iVar7 = iVar18 << 6;
        }
        if (iVar13 < 0) {
          iVar18 = -uVar14;
          if ((int)uVar14 < 1) {
            iVar13 = iVar18;
            if (iVar7 < iVar18) {
              iVar13 = iVar7;
            }
          }
          else {
            iVar13 = -iVar7;
            if (iVar13 <= iVar18) goto LAB_80030138;
          }
          iVar18 = iVar13;
        }
        else if ((*puVar22 & 0x10000) == 0) {
          iVar18 = 0;
        }
        else if (*(char *)((int)local_5c + 0xe) < '\x01') {
          iVar18 = -iVar7;
        }
        else {
          iVar18 = iVar7;
          if (iVar7 < (int)-uVar14 >> 2) {
            iVar18 = (int)-uVar14 >> 2;
          }
        }
LAB_80030138:
        if (local_48 != 0) {
          iVar13 = (int)uVar14 >> 8;
          if (*(short *)(local_48 + 0x12) != 0) {
            iVar7 = iVar13;
            if (iVar13 < 0) {
              iVar7 = -iVar13;
            }
            iVar18 = iVar18 - (iVar13 * iVar7 * (int)*(short *)(local_48 + 0x12) >> 0xc);
          }
        }
        iVar13 = -uVar6;
        if ((int)uVar6 < 1) {
          if (iVar17 < iVar13) goto LAB_800301b4;
        }
        else {
          iVar17 = -iVar17;
          if (iVar13 < iVar17) {
LAB_800301b4:
            iVar13 = iVar17;
          }
        }
        local_c8 = ((iVar20 >> 0xc) - iVar23) + (iVar21 * iVar18 + local_58 * iVar13 >> 0xc);
        local_c0 = ((iVar11 >> 0xc) - iVar10) + (local_58 * iVar18 - iVar21 * iVar13 >> 0xc);
        gte_ldR11R12((int)local_b8 >> 3);
        gte_ldR22R23(local_b4 >> 3);
        gte_ldR33((int)local_b0 >> 3);
        iVar20 = 0x7fff;
        if (local_c8 >> 3 < 0x7fff) {
          iVar20 = local_c8 >> 3;
        }
        iVar23 = -0x8000;
        if (-0x8000 < iVar20) {
          iVar23 = iVar20;
        }
        iVar20 = 0x7fff;
        if (local_c4 >> 3 < 0x7fff) {
          iVar20 = local_c4 >> 3;
        }
        iVar11 = -0x8000;
        if (-0x8000 < iVar20) {
          iVar11 = iVar20;
        }
        iVar20 = 0x7fff;
        if (local_c0 >> 3 < 0x7fff) {
          iVar20 = local_c0 >> 3;
        }
        iVar10 = -0x8000;
        if (-0x8000 < iVar20) {
          iVar10 = iVar20;
        }
        gte_ldsv_(iVar23,iVar11,iVar10);
        gte_op12();
        local_f8 = local_f8 + local_c8;
        local_f4 = local_f4 + local_c4;
        local_f0 = local_f0 + local_c0;
        iVar20 = gte_stMAC1();
        local_108 = local_108 + iVar20;
        iVar20 = gte_stMAC2();
        local_104 = local_104 + iVar20;
        iVar20 = gte_stMAC3();
        local_100 = local_100 + iVar20;
        if (((local_48 != 0) && (*(short *)(local_48 + 0x16) != 0)) &&
           (*(short *)(local_48 + 0x16) != 7)) {
          (*pcRam00000730)(puVar22,9,&local_a8);
        }
      }
      else {
        puVar22[0x13] = puVar22[0x21];
      }
      iVar20 = iVar21 * local_88 + local_58 * local_80;
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0xfff;
      }
      iVar21 = (iVar20 >> 0xc) * puVar22[0x25];
      puVar22[0x26] = iVar20 >> 0xc;
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0x7ffff;
      }
      iVar23 = iVar4 + 1;
      *(short *)(puVar22 + 0x10) = (short)puVar22[0x10] - (short)(iVar21 >> 0x13);
      iVar20 = iVar4 + 5;
      iVar4 = iVar23;
    } while (iVar23 < 4);
    iVar20 = 0;
    iVar4 = 0x10;
    do {
      iVar21 = iVar4 + 0xec;
      iVar4 = iVar4 + 4;
      iVar20 = iVar20 + 1;
      FUN_8001d708(*(undefined4 *)((int)param_1 + iVar21));
    } while (iVar20 < 4);
    FUN_80043358(param_1 + 4,&local_f8,&local_f8);
  }
  iVar4 = param_1[0x23] * param_1[0x37];
  local_f8 = local_f8 - (int)((ulonglong)((longlong)(int)param_1[0x20] * (longlong)iVar4) >> 0x20);
  local_f4 = (local_f4 + iRam00000030) -
             (int)((ulonglong)((longlong)(int)param_1[0x21] * (longlong)iVar4) >> 0x20);
  local_38 = (longlong)(int)param_1[0x22] * (longlong)iVar4;
  local_f0 = local_f0 - (int)((ulonglong)local_38 >> 0x20);
  FUN_800173fc(param_1,&local_f8,&local_108);
  uVar14 = param_1[0x24];
  uVar6 = uVar14;
  if ((int)uVar14 < 0) {
    uVar6 = uVar14 + 0x1f;
  }
  uVar8 = param_1[0x25];
  param_1[0x24] = uVar14 - ((int)uVar6 >> 5);
  uVar6 = uVar8;
  if ((int)uVar8 < 0) {
    uVar6 = uVar8 + 0x1f;
  }
  uVar14 = param_1[0x26];
  param_1[0x25] = uVar8 - ((int)uVar6 >> 5);
  uVar6 = uVar14;
  if ((int)uVar14 < 0) {
    uVar6 = uVar14 + 0x1f;
  }
  param_1[0x26] = uVar14 - ((int)uVar6 >> 5);
  iVar4 = 0;
  puVar22 = param_1;
  do {
    uVar6 = puVar22[0x44];
    if ((uVar6 != 0) && (*(short *)(uVar6 + 6) != 0)) {
      *(short *)(uVar6 + 6) = *(short *)(uVar6 + 6) + -1;
    }
    iVar4 = iVar4 + 1;
    puVar22 = puVar22 + 1;
  } while (iVar4 < 3);
  iVar4 = 0;
  puVar22 = param_1;
  do {
    if ((short)puVar22[0x47] != 0) {
      *(short *)(puVar22 + 0x47) = (short)puVar22[0x47] + -1;
    }
    iVar4 = iVar4 + 1;
    puVar22 = (uint *)((int)puVar22 + 2);
  } while (iVar4 < 3);
  if ((*param_1 & 0x800000) == 0) {
    if ((short)param_1[0x48] == 0) {
      if ((*(short *)((int)param_1 + 6) < 0) || ((*param_1 & 0x40000) != 0)) {
        iVar20 = 0x100 - (uint)(byte)local_5c[4];
        iVar4 = (param_1[9] - param_1[0x12]) * iVar20;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xff;
        }
        iVar21 = (param_1[10] - param_1[0x13]) * iVar20;
        param_1[0x12] = param_1[0x12] + (iVar4 >> 8);
        if (iVar21 < 0) {
          iVar21 = iVar21 + 0xff;
        }
        iVar20 = (param_1[0xb] - param_1[0x14]) * iVar20;
        param_1[0x13] = param_1[0x13] + (iVar21 >> 8);
        if (iVar20 < 0) {
          iVar20 = iVar20 + 0xff;
        }
        param_1[0x14] = param_1[0x14] + (iVar20 >> 8);
      }
      else {
        param_1[0x12] = param_1[9];
        param_1[0x13] = param_1[10];
        param_1[0x14] = param_1[0xb];
      }
    }
    else {
      iVar4 = param_1[9] - param_1[0x12];
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0x1f;
      }
      param_1[0x12] = param_1[0x12] + (iVar4 >> 5);
      iVar4 = param_1[10] - param_1[0x13];
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0x1f;
      }
      param_1[0x13] = param_1[0x13] + (iVar4 >> 5);
      iVar4 = param_1[0xb] - param_1[0x14];
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0x1f;
      }
      param_1[0x14] = param_1[0x14] + (iVar4 >> 5);
    }
  }
  return;
}

