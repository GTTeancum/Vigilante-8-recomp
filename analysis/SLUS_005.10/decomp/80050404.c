// addr: 0x80050404  name: SetPriority

void SetPriority(int param_1,int param_2,int param_3)

{
  uint uVar1;
  
  *(undefined1 *)(param_1 + 3) = 2;
  uVar1 = 0xe6000000;
  if (param_2 != 0) {
    uVar1 = 0xe6000002;
  }
  *(uint *)(param_1 + 4) = uVar1 | param_3 != 0;
  *(undefined4 *)(param_1 + 8) = 0;
  return;
}

