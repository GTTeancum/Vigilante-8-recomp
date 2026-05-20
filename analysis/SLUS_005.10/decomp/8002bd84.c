// addr: 0x8002bd84  name: FUN_8002bd84

void FUN_8002bd84(undefined4 param_1)

{
  undefined4 uVar1;
  char acStack_48 [64];
  
  uVar1 = FUN_8002b940();
  sprintf(acStack_48,"%s destroyed!",uVar1);
  FUN_800129e8(0,acStack_48);
  FUN_8002bc18(param_1);
  return;
}

