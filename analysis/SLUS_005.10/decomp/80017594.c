// addr: 0x80017594  name: FUN_80017594

void FUN_80017594(int param_1,int *param_2,int *param_3)

{
  int iVar1;
  int local_20;
  int local_1c;
  int local_18;
  
  FUN_80043358(param_1 + 0x10,param_2,&local_20);
  gte_ldR11R12(*param_3 >> 4);
  gte_ldR22R23(param_3[1] >> 4);
  gte_ldR33(param_3[2] >> 4);
  gte_ldsv_(*param_2 >> 3,param_2[1] >> 3,param_2[2] >> 3);
  gte_op12();
  *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) + local_20;
  *(int *)(param_1 + 0x84) = *(int *)(param_1 + 0x84) + local_1c;
  *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + local_18;
  iVar1 = gte_stMAC1();
  iVar1 = iVar1 * *(short *)(param_1 + 0x9c);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  *(int *)(param_1 + 0x90) = *(int *)(param_1 + 0x90) + (iVar1 >> 6);
  iVar1 = gte_stMAC2();
  iVar1 = iVar1 * *(short *)(param_1 + 0x9e);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  *(int *)(param_1 + 0x94) = *(int *)(param_1 + 0x94) + (iVar1 >> 6);
  iVar1 = gte_stMAC3();
  iVar1 = iVar1 * *(short *)(param_1 + 0xa0);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0x3f;
  }
  *(int *)(param_1 + 0x98) = *(int *)(param_1 + 0x98) + (iVar1 >> 6);
  return;
}

