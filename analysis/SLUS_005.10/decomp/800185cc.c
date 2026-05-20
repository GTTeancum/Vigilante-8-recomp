// addr: 0x800185cc  name: FUN_800185cc

void FUN_800185cc(uint *param_1,int param_2)

{
  uint uVar1;
  
  uVar1 = *(uint *)(param_2 + 4);
  *param_1 = uVar1;
  if ((uVar1 & 8) == 0) {
    param_1[2] = 0;
    param_1[1] = 0;
  }
  else {
    param_1[1] = param_2 + 0xc;
    param_1[2] = param_2 + 0x14;
    param_2 = param_2 + *(int *)(param_2 + 8);
  }
  param_1[3] = param_2 + 0xc;
  param_1[4] = param_2 + 0x14;
  return;
}

