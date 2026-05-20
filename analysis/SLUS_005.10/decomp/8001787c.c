// addr: 0x8001787c  name: FUN_8001787c

void FUN_8001787c(int param_1,int *param_2)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int local_68;
  int local_64;
  int local_60;
  int local_58;
  int local_54;
  int local_50;
  undefined1 auStack_48 [8];
  int local_40;
  int local_3c;
  int local_38;
  int local_30;
  int local_2c;
  int local_28;
  
  uVar3 = 0;
  local_58 = 0;
  local_54 = 0x2d00;
  local_50 = 0;
  local_68 = 0;
  local_64 = 0;
  local_60 = 0;
  uVar1 = 0;
  do {
    if (uVar1 == 0) {
      local_40 = param_2[3];
    }
    else {
      local_40 = *param_2;
    }
    if ((uVar3 & 2) == 0) {
      local_3c = param_2[4];
    }
    else {
      local_3c = param_2[1];
    }
    if ((uVar3 & 4) == 0) {
      local_38 = param_2[5];
    }
    else {
      local_38 = param_2[2];
    }
    local_30 = 0;
    local_2c = 0;
    local_28 = 0;
    FUN_80043408(param_1 + 0x10,&local_40,&local_40);
    iVar2 = FUN_8001d748(param_1,&local_40,auStack_48,0);
    if (0 < local_3c - iVar2) {
      local_30 = -*(int *)(param_1 + 0x80);
      if (local_30 < 0) {
        local_30 = local_30 + 3;
      }
      local_30 = local_30 >> 2;
      if (0xb40 < local_30) {
        local_30 = 0xb40;
      }
      if (local_30 < -0xb40) {
        local_30 = -0xb40;
      }
      local_28 = -*(int *)(param_1 + 0x88);
      if (local_28 < 0) {
        local_28 = local_28 + 3;
      }
      local_28 = local_28 >> 2;
      if (0xb40 < local_28) {
        local_28 = 0xb40;
      }
      local_2c = -(local_3c - iVar2);
      if (local_28 < -0xb40) {
        local_28 = -0xb40;
      }
      if (0 < *(int *)(param_1 + 0x84)) {
        local_2c = local_2c - (*(int *)(param_1 + 0x84) >> 3);
      }
      gte_ldR11R12(local_40 - *(int *)(param_1 + 0x24) >> 4);
      gte_ldR22R23(local_3c - *(int *)(param_1 + 0x28) >> 4);
      gte_ldR33(local_38 - *(int *)(param_1 + 0x2c) >> 4);
      gte_ldsv_(local_30 >> 3,local_2c >> 3,local_28 >> 3);
      gte_op12();
      local_58 = local_58 + local_30;
      local_54 = local_54 + local_2c;
      local_50 = local_50 + local_28;
      iVar2 = gte_stMAC1();
      local_68 = local_68 + iVar2 * 2;
      iVar2 = gte_stMAC2();
      local_64 = local_64 + iVar2 * 2;
      iVar2 = gte_stMAC3();
      local_60 = local_60 + iVar2 * 2;
    }
    uVar3 = uVar3 + 1;
    uVar1 = uVar3 & 1;
  } while ((int)uVar3 < 8);
  FUN_8004352c(param_1 + 0x10,&local_68,&local_68);
  FUN_800173fc(param_1,&local_58,&local_68);
  *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) * 0xf80 >> 0xc;
  *(int *)(param_1 + 0x94) = *(int *)(param_1 + 0x94) * 0xf80 >> 0xc;
  *(int *)(param_1 + 0x98) = *(int *)(param_1 + 0x98) * 0xf80 >> 0xc;
  return;
}

