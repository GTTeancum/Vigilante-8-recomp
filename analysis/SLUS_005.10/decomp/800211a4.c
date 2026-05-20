// addr: 0x800211a4  name: FUN_800211a4

undefined4 FUN_800211a4(uint *param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*param_1 == 0) {
    uVar1 = FUN_8002002c(param_1 + 1);
  }
  else {
    uVar1 = 0;
    if (*param_1 < 3) {
      iVar2 = FUN_800211a4(param_1[2],param_2,param_3);
      if ((iVar2 == 0) && (iVar2 = FUN_800211a4(param_1[3],param_2,param_3), iVar2 == 0)) {
        return 0;
      }
      uVar1 = 1;
    }
  }
  return uVar1;
}

