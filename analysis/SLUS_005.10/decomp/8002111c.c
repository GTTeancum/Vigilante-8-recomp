// addr: 0x8002111c  name: FUN_8002111c

void FUN_8002111c(uint *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  while( true ) {
    if (*param_1 == 0) {
      FUN_80020000(param_1 + 1,param_2,param_3);
      return;
    }
    if (2 < *param_1) break;
    iVar1 = FUN_8002111c(param_1[2],param_2,param_3);
    if (iVar1 != 0) {
      return;
    }
    param_1 = (uint *)param_1[3];
  }
  return;
}

