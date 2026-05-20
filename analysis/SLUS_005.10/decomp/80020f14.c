// addr: 0x80020f14  name: FUN_80020f14

undefined4 FUN_80020f14(int *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  undefined4 uVar3;
  int iVar4;
  
  iVar4 = *param_1;
  if (iVar4 == 1) {
    if ((*(int *)(param_2 + 0x24) - *(int *)(param_2 + 0x54) < param_1[1]) &&
       (iVar4 = FUN_80020f14(param_1[2],param_2), iVar4 == 0)) {
      return 0;
    }
    if ((param_1[1] < *(int *)(param_2 + 0x24) + *(int *)(param_2 + 0x54)) &&
       (iVar4 = FUN_80020f14(param_1[3],param_2), iVar4 == 0)) {
      return 0;
    }
    uVar3 = 1;
  }
  else {
    uVar3 = 2;
    if (iVar4 == 0) {
      piVar1 = (int *)param_1[1];
      for (piVar2 = (int *)*(int *)param_1[1]; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
        if (((*(uint *)piVar1[2] & 0x20) == 0) && (iVar4 = FUN_8001edb4(param_2), iVar4 != 0)) {
          return 0;
        }
        piVar1 = piVar2;
      }
      uVar3 = 1;
    }
    else if (iVar4 == 2) {
      if ((*(int *)(param_2 + 0x2c) - *(int *)(param_2 + 0x54) < param_1[1]) &&
         (iVar4 = FUN_80020f14(param_1[2],param_2), iVar4 == 0)) {
        return 0;
      }
      if ((param_1[1] < *(int *)(param_2 + 0x2c) + *(int *)(param_2 + 0x54)) &&
         (iVar4 = FUN_80020f14(param_1[3],param_2), iVar4 == 0)) {
        return 0;
      }
      uVar3 = 1;
    }
  }
  return uVar3;
}

