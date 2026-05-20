// addr: 0x800289d8  name: FUN_800289d8

void FUN_800289d8(uint param_1,short *param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  byte bVar6;
  int iVar7;
  int iVar8;
  short *psVar9;
  uint uVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  int unaff_s4;
  int iVar15;
  uint uVar16;
  int unaff_s7;
  int iVar17;
  uint unaff_s8;
  undefined1 uVar18;
  uint local_54;
  uint local_50;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  
  uVar2 = 1 << (param_1 & 0x1f);
  uVar1 = -uVar2;
  local_54 = uVar1;
  if (param_1 != 2) {
    local_54 = uVar2 * -2;
  }
  iVar3 = (int)param_2[3];
  iVar8 = (int)param_2[1];
  uVar14 = 0x7fffffff;
  local_50 = 0x80000001;
  iVar7 = iVar3;
  if (iVar8 < iVar3) {
    iVar7 = iVar8;
  }
  iVar4 = iVar3;
  if (iVar3 < iVar8) {
    iVar4 = iVar8;
  }
  local_40 = 0;
  local_3c = 0;
  uVar16 = 0;
  local_38 = 0;
  local_34 = 0;
  bVar6 = (iVar3 < iVar8) << 1 | param_2[2] < *param_2;
  psVar9 = param_2;
  do {
    uVar5 = (uint)psVar9[1];
    if ((int)uVar5 < (int)uVar14) {
      uVar14 = uVar5;
      unaff_s8 = uVar16;
    }
    if ((int)local_50 < (int)uVar5) {
      local_50 = uVar5;
    }
    uVar16 = uVar16 + 1;
    psVar9 = psVar9 + 2;
  } while ((int)uVar16 < 4);
  iVar3 = (int)param_2[unaff_s8 * 2] << 0x10;
  iVar8 = iVar3;
  uVar16 = unaff_s8;
  if ((param_1 != 0) && (param_2[1] == param_2[3])) {
    local_50 = local_50 - uVar2;
  }
  do {
    if ((int)local_50 <= (int)uVar14) {
      if ((param_1 != 0) && (param_2[1] == param_2[3])) {
        FUN_800288fc(param_1,unaff_s4 >> 0x10 & uVar1,unaff_s7 >> 0x10 & uVar1,uVar14,4);
      }
      return;
    }
    if (local_3c < 1) {
      do {
        iVar15 = uVar16 * 2;
        uVar16 = uVar16 + 1 & 3;
        uVar5 = 0;
        unaff_s4 = param_2[iVar15] * 0x10000;
        if (param_1 != 2) {
          uVar5 = (uint)(uVar16 == 3);
        }
        local_3c = (int)((int)(param_2 + uVar16 * 2)[1] - uVar14) >> (param_1 + uVar5 & 0x1f);
      } while (local_3c < 1);
      local_40 = (param_2[uVar16 * 2] * 0x10000 + param_2[iVar15] * -0x10000) / local_3c;
    }
    if (local_34 < 1) {
      do {
        iVar15 = unaff_s8 * 2;
        unaff_s8 = unaff_s8 + 3 & 3;
        uVar5 = 0;
        unaff_s7 = param_2[iVar15] * 0x10000;
        if (param_1 != 2) {
          uVar5 = (uint)(unaff_s8 == 2);
        }
        local_34 = (int)((int)(param_2 + unaff_s8 * 2)[1] - uVar14) >> (param_1 + uVar5 & 0x1f);
      } while (local_34 < 1);
      local_38 = (param_2[unaff_s8 * 2] * 0x10000 + param_2[iVar15] * -0x10000) / local_34;
    }
    uVar5 = unaff_s4 >> 0x10;
    uVar11 = unaff_s7 >> 0x10;
    if (((param_1 == 0) || ((int)uVar14 < iVar7)) || (iVar4 < (int)uVar14)) {
      uVar10 = local_54;
      if (uVar16 != 3) {
        uVar10 = uVar1;
      }
      uVar10 = uVar5 & uVar10;
      uVar12 = local_54;
      if (unaff_s8 != 2) {
        uVar12 = uVar1;
      }
      uVar12 = uVar11 & uVar12;
      uVar18 = 0;
LAB_80028f9c:
      FUN_800288fc(param_1,uVar10,uVar12,uVar14,uVar18);
    }
    else {
      if (bVar6 == 1) {
        if (unaff_s7 < iVar3) {
          iVar3 = unaff_s7;
        }
        uVar13 = iVar3 >> 0x10 & uVar1;
        FUN_800288fc(param_1,uVar5 & uVar1,uVar13,uVar14,9);
        uVar10 = unaff_s4 - local_40 >> 0x10 & uVar1;
        uVar12 = local_54;
        if (unaff_s8 != 2) {
          uVar12 = uVar1;
        }
        uVar12 = uVar11 & uVar12;
        uVar18 = (int)uVar13 <= (int)(uVar5 & uVar1);
        goto LAB_80028f9c;
      }
      if (1 < bVar6) {
        if (bVar6 == 2) {
          iVar3 = unaff_s7 + local_38;
          if (unaff_s7 + local_38 < unaff_s4) {
            iVar3 = unaff_s4;
          }
          uVar13 = iVar3 >> 0x10 & uVar1;
          FUN_800288fc(param_1,uVar13,uVar11 & uVar1,uVar14,6);
          uVar10 = local_54;
          if (uVar16 != 3) {
            uVar10 = uVar1;
          }
          uVar10 = uVar5 & uVar10;
          iVar3 = unaff_s7 + local_38;
          if (unaff_s7 + local_38 < unaff_s4) {
            iVar3 = unaff_s4;
          }
          uVar12 = iVar3 >> 0x10 & uVar1;
          if ((int)uVar13 < (int)(uVar11 & uVar1)) {
            uVar18 = 0;
          }
          else {
            uVar18 = 2;
          }
        }
        else {
          if (bVar6 != 3) goto LAB_80028fb0;
          if (iVar8 < unaff_s4) {
            iVar8 = unaff_s4;
          }
          uVar13 = iVar8 >> 0x10 & uVar1;
          FUN_800288fc(param_1,uVar13,uVar11 & uVar1,uVar14,10);
          uVar10 = local_54;
          if (uVar16 != 3) {
            uVar10 = uVar1;
          }
          uVar10 = uVar5 & uVar10;
          iVar3 = unaff_s7 - local_38;
          if (unaff_s7 - local_38 < unaff_s4) {
            iVar3 = unaff_s4;
          }
          uVar12 = iVar3 >> 0x10 & uVar1;
          if ((int)uVar13 < (int)(uVar11 & uVar1)) {
            uVar18 = 0;
          }
          else {
            uVar18 = 2;
          }
        }
        goto LAB_80028f9c;
      }
      if (bVar6 == 0) {
        iVar8 = unaff_s4 + local_40;
        iVar3 = iVar8;
        if (unaff_s7 < iVar8) {
          iVar3 = unaff_s7;
        }
        uVar13 = iVar3 >> 0x10 & uVar1;
        FUN_800288fc(param_1,uVar5 & uVar1,uVar13,uVar14,5);
        uVar10 = iVar8 >> 0x10 & uVar1;
        uVar12 = local_54;
        if (unaff_s8 != 2) {
          uVar12 = uVar1;
        }
        uVar12 = uVar11 & uVar12;
        uVar18 = (int)uVar13 <= (int)(uVar5 & uVar1);
        goto LAB_80028f9c;
      }
    }
LAB_80028fb0:
    if (((param_1 == 2) || (uVar16 != 3)) || (iVar15 = unaff_s4, (uVar14 & uVar2) != 0)) {
      iVar15 = unaff_s4 + local_40;
      local_3c = local_3c + -1;
    }
    if (((param_1 == 2) || (unaff_s8 != 2)) || (iVar17 = unaff_s7, (uVar14 & uVar2) != 0)) {
      iVar17 = unaff_s7 + local_38;
      local_34 = local_34 + -1;
    }
    uVar14 = uVar14 + uVar2;
    iVar3 = unaff_s4;
    iVar8 = unaff_s7;
    unaff_s7 = iVar17;
    unaff_s4 = iVar15;
  } while( true );
}

