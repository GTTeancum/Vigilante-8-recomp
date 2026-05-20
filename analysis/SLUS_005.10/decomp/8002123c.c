// addr: 0x8002123c  name: FUN_8002123c

int FUN_8002123c(uint *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  if (*param_1 == 0) {
    iVar1 = FUN_800200b8(param_1 + 1);
  }
  else {
    iVar1 = 0;
    if (((*param_1 < 3) && (iVar1 = FUN_8002123c(param_1[2],param_2,param_3), iVar1 == 0)) &&
       (iVar1 = FUN_8002123c(param_1[3],param_2,param_3), iVar1 == 0)) {
      iVar1 = 0;
    }
  }
  return iVar1;
}

