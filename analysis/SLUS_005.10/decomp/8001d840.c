// addr: 0x8001d840  name: FUN_8001d840

void FUN_8001d840(undefined4 param_1,undefined4 *param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined1 auStack_18 [8];
  
  uVar1 = FUN_8001d748(param_1,param_2,auStack_18,0);
  uVar2 = *param_2;
  *(undefined4 *)(param_3 + 0x18) = uVar1;
  *(undefined4 *)(param_3 + 0x14) = uVar2;
  *(undefined4 *)(param_3 + 0x1c) = param_2[2];
  FUN_80016fa8(param_3,auStack_18);
  return;
}

