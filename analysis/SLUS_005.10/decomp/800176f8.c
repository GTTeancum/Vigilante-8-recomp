// addr: 0x800176f8  name: FUN_800176f8

void FUN_800176f8(int param_1,int *param_2,int *param_3)

{
  undefined4 uVar1;
  int iVar2;
  short local_10;
  short local_e;
  short local_c;
  
  *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) + *param_2;
  *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) + param_2[1];
  *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + param_2[2];
  gte_ldR11R12(*param_3 - *(int *)(param_1 + 0x48) >> 3);
  gte_ldR22R23(param_3[1] - *(int *)(param_1 + 0x4c) >> 3);
  gte_ldR33(param_3[2] - *(int *)(param_1 + 0x50) >> 3);
  gte_ldsv_(*param_2 >> 3,param_2[1] >> 3,param_2[2] >> 3);
  gte_op12();
  uVar1 = gte_stMAC1();
  local_10 = (short)uVar1;
  uVar1 = gte_stMAC2();
  local_e = (short)uVar1;
  uVar1 = gte_stMAC3();
  local_c = (short)uVar1;
  FUN_800434f8(param_1 + 0x10,&local_10,&local_10,*(int *)(param_1 + 0x48));
  iVar2 = (int)local_10 * (int)*(short *)(param_1 + 0x9c);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) + (iVar2 >> 7);
  iVar2 = (int)local_e * (int)*(short *)(param_1 + 0x9e);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  *(int *)(param_1 + 0x94) = *(int *)(param_1 + 0x94) + (iVar2 >> 7);
  iVar2 = (int)local_c * (int)*(short *)(param_1 + 0xa0);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  *(int *)(param_1 + 0x98) = *(int *)(param_1 + 0x98) + (iVar2 >> 7);
  return;
}

