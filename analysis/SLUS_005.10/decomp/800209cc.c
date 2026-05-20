// addr: 0x800209cc  name: FUN_800209cc

void FUN_800209cc(int *param_1,int param_2,int param_3,int param_4,int param_5)

{
  bool bVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 == 1) {
    iVar2 = param_1[1];
    if (param_2 < iVar2) {
      FUN_800209cc(param_1[2],param_2,param_3,param_4,param_5);
    }
    bVar1 = iVar2 < param_3;
  }
  else {
    if (iVar2 == 0) {
      FUN_800206f0(param_1 + 1);
      return;
    }
    if (iVar2 != 2) {
      return;
    }
    iVar2 = param_1[1];
    if (param_4 < iVar2) {
      FUN_800209cc(param_1[2],param_2,param_3,param_4,param_5);
    }
    bVar1 = iVar2 < param_5;
  }
  if (bVar1) {
    FUN_800209cc(param_1[3],param_2,param_3,param_4,param_5);
  }
  return;
}

