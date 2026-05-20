// addr: 0x800210a4  name: FUN_800210a4

void FUN_800210a4(uint *param_1,undefined4 param_2)

{
  int iVar1;
  
  while( true ) {
    if (*param_1 == 0) {
      FUN_8001ff0c(param_1 + 1,param_2);
      return;
    }
    if (2 < *param_1) break;
    iVar1 = FUN_800210a4(param_1[2],param_2);
    if (iVar1 != 0) {
      return;
    }
    param_1 = (uint *)param_1[3];
  }
  return;
}

