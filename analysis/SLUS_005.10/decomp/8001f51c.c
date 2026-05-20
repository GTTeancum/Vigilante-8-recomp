// addr: 0x8001f51c  name: FUN_8001f51c

void FUN_8001f51c(uint *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  
  if (((*param_1 & 0x800) != 0) &&
     (iVar1 = FUN_8001f3ac(param_1,param_1 + 4,param_2,param_3,param_4), iVar1 != 0)) {
    return;
  }
  FUN_8001ef74(param_1[0x17],param_1 + 4,param_2,param_3,param_4);
  return;
}

