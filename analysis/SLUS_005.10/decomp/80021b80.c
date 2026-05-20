// addr: 0x80021b80  name: FUN_80021b80

void FUN_80021b80(code *param_1,int param_2,undefined2 param_3,undefined4 param_4)

{
  int iVar1;
  
  if (param_1 != (code *)0x0) {
    iVar1 = (*param_1)(param_2,7,param_3);
    if (iVar1 != 0) {
      return;
    }
    iVar1 = (*param_1)(0,6,0);
    if (iVar1 != 0) goto LAB_80021bdc;
  }
  iVar1 = 0x80;
LAB_80021bdc:
  if (param_2 == 0) {
    FUN_8001d470(iVar1);
  }
  else {
    FUN_8001ac44(param_2,param_3,iVar1,param_4);
  }
  return;
}

