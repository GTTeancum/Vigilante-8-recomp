// addr: 0x80020bec  name: FUN_80020bec

undefined4 FUN_80020bec(int *param_1,int *param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 == 1) {
    if (*param_2 - param_1[1] < param_1[1] - param_2[1]) {
      iVar2 = FUN_80020bec(param_1[2],param_2);
      if (iVar2 != 0) {
        return 1;
      }
      iVar2 = param_1[3];
    }
    else {
      iVar2 = FUN_80020bec(param_1[3],param_2);
      if (iVar2 != 0) {
        return 1;
      }
      iVar2 = param_1[2];
    }
    iVar2 = FUN_80020bec(iVar2,param_2);
    if (iVar2 != 0) {
      return 1;
    }
    return 0;
  }
  if (iVar2 == 0) {
    uVar1 = FUN_80020ad0(param_1 + 1,param_2);
    return uVar1;
  }
  if (iVar2 != 2) {
    return 2;
  }
  if (param_2[2] - param_1[1] < param_1[1] - param_2[3]) {
    iVar2 = FUN_80020bec(param_1[2],param_2);
    if (iVar2 != 0) {
      return 1;
    }
    iVar2 = param_1[3];
  }
  else {
    iVar2 = FUN_80020bec(param_1[3],param_2);
    if (iVar2 != 0) {
      return 1;
    }
    iVar2 = param_1[2];
  }
  iVar2 = FUN_80020bec(iVar2,param_2);
  if (iVar2 != 0) {
    return 1;
  }
  return 0;
}

