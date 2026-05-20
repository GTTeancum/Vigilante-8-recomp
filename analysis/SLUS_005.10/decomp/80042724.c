// addr: 0x80042724  name: FUN_80042724

int FUN_80042724(int *param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  int local_68 [20];
  
  local_88 = *(int *)*param_1;
  local_84 = *(int *)(*param_1 + 8);
  local_80 = *(int *)*param_1 + param_1[4];
  local_7c = *(int *)(*param_1 + 8) + param_1[5];
  local_78 = *(int *)param_1[1] + param_1[6];
  local_74 = *(int *)(param_1[1] + 8) + param_1[7];
  local_70 = *(int *)param_1[1];
  iVar12 = 0;
  local_6c = *(int *)(param_1[1] + 8);
  iVar11 = 0x1000;
  iVar13 = 0;
  do {
    local_68[0x10] = (local_80 + local_78) / 2;
    iVar3 = (local_7c + local_74) / 2;
    local_68[0] = local_88;
    local_68[1] = local_84;
    local_68[0xe] = local_70;
    local_68[0xf] = local_6c;
    local_68[2] = (local_88 + local_80) / 2;
    local_68[3] = (local_84 + local_7c) / 2;
    local_68[4] = (local_68[2] + local_68[0x10]) / 2;
    local_68[5] = (local_68[3] + iVar3) / 2;
    local_78 = (local_78 + local_70) / 2;
    local_74 = (local_74 + local_6c) / 2;
    local_68[0xc] = local_78;
    local_68[0xd] = local_74;
    local_80 = (local_78 + local_68[0x10]) / 2;
    local_7c = (local_74 + iVar3) / 2;
    local_68[10] = local_80;
    local_68[0xb] = local_7c;
    local_68[8] = (local_68[4] + local_80) / 2;
    local_68[9] = (local_68[5] + local_7c) / 2;
    local_68[6] = local_68[8];
    local_68[7] = local_68[9];
    iVar3 = 0;
    if (local_68[8] < local_88) {
      iVar3 = 3;
    }
    iVar6 = 0;
    if (local_68[9] < local_84) {
      iVar6 = 3;
    }
    iVar9 = *param_2;
    iVar1 = 0;
    if (0 < iVar9 - local_68[(3 - iVar3) * 2]) {
      iVar1 = iVar9 - local_68[(3 - iVar3) * 2];
    }
    iVar4 = 0;
    if (iVar9 - local_68[iVar3 * 2] < 0) {
      iVar4 = iVar9 - local_68[iVar3 * 2];
    }
    iVar10 = param_2[2];
    iVar3 = 0;
    if (0 < iVar10 - local_68[(3 - iVar6) * 2 + 1]) {
      iVar3 = iVar10 - local_68[(3 - iVar6) * 2 + 1];
    }
    iVar7 = 0;
    if (iVar10 - local_68[iVar6 * 2 + 1] < 0) {
      iVar7 = iVar10 - local_68[iVar6 * 2 + 1];
    }
    iVar6 = 0;
    if (local_70 < local_68[8]) {
      iVar6 = 3;
    }
    iVar8 = 0;
    if (local_6c < local_68[9]) {
      iVar8 = 3;
    }
    iVar2 = 0;
    if (0 < iVar9 - local_68[(3 - iVar6) * 2 + 8]) {
      iVar2 = iVar9 - local_68[(3 - iVar6) * 2 + 8];
    }
    iVar5 = 0;
    if (iVar9 - local_68[iVar6 * 2 + 8] < 0) {
      iVar5 = iVar9 - local_68[iVar6 * 2 + 8];
    }
    iVar6 = 0;
    if (0 < iVar10 - local_68[(3 - iVar8) * 2 + 9]) {
      iVar6 = iVar10 - local_68[(3 - iVar8) * 2 + 9];
    }
    iVar9 = 0;
    if (iVar10 - local_68[iVar8 * 2 + 9] < 0) {
      iVar9 = iVar10 - local_68[iVar8 * 2 + 9];
    }
    if ((iVar1 - iVar4) + (iVar3 - iVar7) < (iVar2 - iVar5) + (iVar6 - iVar9)) {
      iVar11 = (iVar12 + iVar11) / 2;
      local_80 = local_68[2];
      local_7c = local_68[3];
      local_78 = local_68[4];
      local_74 = local_68[5];
      local_70 = local_68[8];
      local_6c = local_68[9];
    }
    else {
      iVar12 = (iVar12 + iVar11) / 2;
      local_88 = local_68[8];
      local_84 = local_68[9];
    }
    iVar13 = iVar13 + 1;
  } while (iVar13 < 8);
  *param_3 = local_88;
  param_3[1] = local_84;
  param_3[2] = local_80;
  param_3[3] = local_7c;
  param_3[4] = local_78;
  param_3[5] = local_74;
  param_3[6] = local_70;
  param_3[7] = local_6c;
  return (iVar12 + iVar11) / 2;
}

