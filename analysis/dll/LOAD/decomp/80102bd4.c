// addr: 0x80102bd4  name: FUN_80102bd4

int FUN_80102bd4(int *param_1,int param_2,uint param_3)

{
  int iVar1;
  int iVar2;
  longlong lVar3;
  undefined2 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  undefined4 uVar15;
  int iVar16;
  uint uVar17;
  int iVar18;
  int iVar19;
  int iVar20;
  int iVar21;
  int iVar22;
  int iVar23;
  int iVar24;
  int iVar25;
  int local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  int *local_44;
  int local_40;
  int local_3c;
  int local_38;
  longlong local_30;
  
  iVar5 = (param_1[3] * 3 - *param_1) + param_1[6] * -3 + param_1[9];
  if (iVar5 < 0) {
    iVar5 = iVar5 + 0xf;
  }
  iVar6 = (param_1[5] * 3 - param_1[2]) + param_1[8] * -3 + param_1[0xb];
  if (iVar6 < 0) {
    iVar6 = iVar6 + 0xf;
  }
  iVar7 = *param_1 * 3 + param_1[3] * -6 + param_1[6] * 3;
  if (iVar7 < 0) {
    iVar7 = iVar7 + 0xf;
  }
  iVar8 = param_1[2] * 3 + param_1[5] * -6 + param_1[8] * 3;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar9 = param_1[3] * 3 + *param_1 * -3;
  if (iVar9 < 0) {
    iVar9 = iVar9 + 0xf;
  }
  iVar9 = iVar9 >> 4;
  iVar10 = param_1[5] * 3 + param_1[2] * -3;
  if (iVar10 < 0) {
    iVar10 = iVar10 + 0xf;
  }
  iVar10 = iVar10 >> 4;
  iVar11 = *param_1;
  iVar12 = param_1[2];
  iVar1 = (iVar5 >> 4) * 3;
  iVar2 = (iVar6 >> 4) * 3;
  iVar13 = (iVar7 >> 4) * 2;
  iVar18 = (iVar8 >> 4) * 2;
  local_70 = iVar11;
  local_6c = iVar12;
  local_68 = iVar1;
  local_64 = iVar2;
  local_60 = iVar13;
  local_5c = iVar18;
  local_58 = iVar9;
  local_54 = iVar10;
  if ((param_3 & 1) != 0) {
    Terrain_NormalAt/*0x80025800*/(*param_1,param_1[1],&local_70);
    VectorNormalSS/*0x8004c844*/(&local_70,&local_70);
  }
  iVar23 = 0;
  local_50 = 0;
  do {
    iVar19 = iVar23 * iVar23;
    if (iVar19 < 0) {
      iVar19 = iVar19 + 0xfff;
    }
    iVar14 = iVar1 * (iVar19 >> 0xc) + iVar13 * iVar23;
    if (iVar14 < 0) {
      iVar14 = iVar14 + 0xfff;
    }
    iVar14 = (iVar14 >> 0xc) + iVar9;
    if (iVar14 < 0) {
      iVar14 = iVar14 + 0xff;
    }
    iVar19 = iVar2 * (iVar19 >> 0xc) + iVar18 * iVar23;
    if (iVar19 < 0) {
      iVar19 = iVar19 + 0xfff;
    }
    iVar19 = (iVar19 >> 0xc) + iVar10;
    if (iVar19 < 0) {
      iVar19 = iVar19 + 0xff;
    }
    iVar19 = SquareRoot0/*0x8004c6e4*/((iVar14 >> 8) * (iVar14 >> 8) + (iVar19 >> 8) * (iVar19 >> 8));
    local_50 = local_50 + 1;
    iVar23 = iVar23 + *(int *)(param_2 + 0x28) / iVar19;
  } while (iVar23 < 0x1000);
  iVar23 = *(int *)(param_2 + 0x14) + local_50 * 2;
  *(int *)(param_2 + 0x14) = iVar23;
  *(int *)(iVar23 + 0xc) = (*param_1 + param_1[9]) / 2;
  local_4c = 0;
  *(int *)(iVar23 + 0x14) = (param_1[2] + param_1[0xb]) / 2;
  uVar15 = Terrain_HeightAt/*0x80025400*/(*(undefined4 *)(iVar23 + 0xc));
  *(undefined4 *)(iVar23 + 0x10) = uVar15;
  *(int *)(iVar23 + 8) = param_2;
  iVar19 = 0;
  if (-1 < local_50) {
    local_44 = &local_70;
    local_40 = 0;
    local_3c = 8;
    local_38 = iVar23;
    do {
      iVar14 = iVar19 * iVar19;
      if (iVar14 < 0) {
        iVar14 = iVar14 + 0xfff;
      }
      iVar14 = iVar14 >> 0xc;
      iVar20 = iVar14 * iVar19;
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0xfff;
      }
      iVar21 = (iVar5 >> 4) * (iVar20 >> 0xc) + (iVar7 >> 4) * iVar14 + iVar9 * iVar19;
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0xff;
      }
      iVar22 = (iVar6 >> 4) * (iVar20 >> 0xc) + (iVar8 >> 4) * iVar14 + iVar10 * iVar19;
      iVar20 = (iVar21 >> 8) + iVar11;
      if (iVar22 < 0) {
        iVar22 = iVar22 + 0xff;
      }
      iVar21 = iVar1 * iVar14 + iVar13 * iVar19;
      iVar22 = (iVar22 >> 8) + iVar12;
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0xfff;
      }
      iVar21 = (iVar21 >> 0xc) + iVar9;
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0xff;
      }
      iVar14 = iVar2 * iVar14 + iVar18 * iVar19;
      iVar21 = iVar21 >> 8;
      if (iVar14 < 0) {
        iVar14 = iVar14 + 0xfff;
      }
      iVar14 = (iVar14 >> 0xc) + iVar10;
      if (iVar14 < 0) {
        iVar14 = iVar14 + 0xff;
      }
      iVar14 = iVar14 >> 8;
      iVar16 = SquareRoot0/*0x8004c6e4*/(iVar21 * iVar21 + iVar14 * iVar14);
      iVar24 = ((iVar14 * *(int *)(param_2 + 0x24)) / 2) / iVar16;
      iVar25 = ((iVar21 * *(int *)(param_2 + 0x24)) / 2) / iVar16;
      iVar14 = iVar20 - iVar24;
      iVar20 = iVar20 + iVar24;
      iVar21 = iVar22 + iVar25;
      iVar22 = iVar22 - iVar25;
      if ((param_3 & 1) != 0) {
        local_30 = (longlong)iVar19 * (longlong)param_1[10];
        lVar3 = (longlong)(0x1000 - iVar19) * (longlong)param_1[1] + local_30;
        if ((uint)local_30 <= (uint)lVar3) {
          lVar3 = lVar3 + 0xfff;
        }
        uVar17 = (uint)lVar3 >> 0xc | (int)((ulonglong)lVar3 >> 0x20) << 0x14;
        local_68 = CONCAT22((short)(uVar17 - *(int *)(iVar23 + 0x10) >> 8),
                            (short)((uint)(iVar14 - *(int *)(iVar23 + 0xc)) >> 8));
        local_64 = CONCAT22(local_64._2_2_,(short)((uint)(iVar21 - *(int *)(iVar23 + 0x14)) >> 8));
        iVar25 = func_0x80016810(local_44,&DAT_80065ab0,0,0);
        iVar24 = 0;
        if (0 < iVar25) {
          iVar24 = iVar25;
        }
        if (iVar24 < 0) {
          iVar24 = iVar24 + 0x1ffff;
        }
        iVar25 = (iVar24 >> 0x11) + 0x20;
        iVar24 = 0x80;
        if (iVar25 < 0x80) {
          iVar24 = iVar25;
        }
        local_64 = CONCAT22((short)iVar24,(undefined2)local_64);
        *(int *)(local_38 + 0x20) = local_68;
        *(int *)(local_38 + 0x24) = local_64;
        local_68 = CONCAT22((short)(uVar17 - *(int *)(iVar23 + 0x10) >> 8),
                            (short)((uint)(iVar20 - *(int *)(iVar23 + 0xc)) >> 8));
        local_64 = CONCAT22((short)iVar24,(short)((uint)(iVar22 - *(int *)(iVar23 + 0x14)) >> 8));
        iVar25 = func_0x80016810(local_44,&DAT_80065ab0);
        iVar24 = 0;
        if (0 < iVar25) {
          iVar24 = iVar25;
        }
        if (iVar24 < 0) {
          iVar24 = iVar24 + 0x1ffff;
        }
        iVar25 = (iVar24 >> 0x11) + 0x20;
        iVar24 = 0x80;
        if (iVar25 < 0x80) {
          iVar24 = iVar25;
        }
        local_64 = CONCAT22((short)iVar24,(undefined2)local_64);
        *(int *)(iVar23 + local_3c + 0x20) = local_68;
        *(int *)(iVar23 + local_3c + 0x24) = local_64;
      }
      local_68 = CONCAT22(local_68._2_2_,(short)((uint)(iVar14 - *(int *)(iVar23 + 0xc)) >> 8));
      iVar24 = Terrain_HeightAt/*0x80025400*/(iVar14,iVar21);
      uVar4 = (undefined2)((uint)(iVar24 - *(int *)(iVar23 + 0x10)) >> 8);
      local_68 = CONCAT22(uVar4,(undefined2)local_68);
      iVar24 = iVar21 - *(int *)(iVar23 + 0x14);
      if (iVar14 < 0) {
        iVar14 = iVar14 + 0xffff;
      }
      if (iVar21 < 0) {
        iVar21 = iVar21 + 0xffff;
      }
      local_64 = CONCAT22((*(ushort *)
                            (*(int *)(((uint)(iVar21 >> 0x10) >> 6) * 4 +
                                      ((uint)(iVar14 >> 0x10) >> 6) * 0x80 + -0x7ff6ee60) +
                            (iVar21 >> 0x10 & 0x3fU) * 2 + (iVar14 >> 0x10 & 0x3fU) * 0x80) >> 0xb)
                          << 2,(short)((uint)iVar24 >> 8));
      *(int *)(local_38 + 0x20) = local_68;
      *(int *)(local_38 + 0x24) = local_64;
      local_68 = CONCAT22(uVar4,(short)((uint)(iVar20 - *(int *)(iVar23 + 0xc)) >> 8));
      iVar14 = Terrain_HeightAt/*0x80025400*/(iVar20,iVar22);
      local_68 = CONCAT22((short)((uint)(iVar14 - *(int *)(iVar23 + 0x10)) >> 8),
                          (undefined2)local_68);
      iVar14 = iVar22 - *(int *)(iVar23 + 0x14);
      if (iVar20 < 0) {
        iVar20 = iVar20 + 0xffff;
      }
      if (iVar22 < 0) {
        iVar22 = iVar22 + 0xffff;
      }
      local_64 = CONCAT22((*(ushort *)
                            (*(int *)(((uint)(iVar22 >> 0x10) >> 6) * 4 +
                                      ((uint)(iVar20 >> 0x10) >> 6) * 0x80 + -0x7ff6ee60) +
                            (iVar22 >> 0x10 & 0x3fU) * 2 + (iVar20 >> 0x10 & 0x3fU) * 0x80) >> 0xb)
                          << 2,(short)((uint)iVar14 >> 8));
      *(int *)(iVar23 + local_3c + 0x20) = local_68;
      *(int *)(iVar23 + local_3c + 0x24) = local_64;
      iVar14 = func_0x80016988(iVar23 + local_40 + 0x20);
      if (iVar14 < local_48) {
        iVar14 = local_48;
      }
      local_48 = func_0x80016988(iVar23 + local_40 + 0x28);
      if (local_48 < iVar14) {
        local_48 = iVar14;
      }
      if (local_4c == local_50 + -1) {
        iVar19 = 0x1000;
      }
      iVar19 = iVar19 + *(int *)(param_2 + 0x28) / iVar16;
      local_40 = local_40 + 0x10;
      local_3c = local_3c + 0x10;
      local_38 = local_38 + 0x10;
      local_4c = local_4c + 1;
    } while (local_4c <= local_50);
  }
  iVar5 = SquareRoot0/*0x8004c6e4*/(local_48);
  *(int *)(iVar23 + 0x18) = iVar5 << 8;
  return iVar23;
}

