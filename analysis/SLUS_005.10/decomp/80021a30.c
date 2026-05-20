// addr: 0x80021a30  name: FUN_80021a30

int FUN_80021a30(int *param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 == 1) {
    iVar2 = param_1[1];
    if (((iVar2 <= *param_4) ||
        (iVar1 = FUN_80021a30(param_1[2],param_2,param_3,param_4), iVar1 == 0)) &&
       ((param_4[1] <= iVar2 ||
        (iVar1 = FUN_80021a30(param_1[3],param_2,param_3,param_4), iVar1 == 0)))) {
      iVar1 = 0;
    }
  }
  else {
    iVar1 = 2;
    if (iVar2 == 0) {
      iVar1 = FUN_80021978(param_1 + 1,param_2,param_3,param_4);
    }
    else if (((iVar2 == 2) &&
             ((iVar2 = param_1[1], iVar2 <= param_4[2] ||
              (iVar1 = FUN_80021a30(param_1[2],param_2,param_3,param_4), iVar1 == 0)))) &&
            ((param_4[3] <= iVar2 ||
             (iVar1 = FUN_80021a30(param_1[3],param_2,param_3,param_4), iVar1 == 0)))) {
      iVar1 = 0;
    }
  }
  return iVar1;
}

