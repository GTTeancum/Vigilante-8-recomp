// addr: 0x800205a0  name: FUN_800205a0

void FUN_800205a0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  if (param_1 != (int *)0x0) {
    piVar3 = (int *)param_1[1];
    iVar2 = *param_1;
    iVar4 = param_1[2];
    *(int **)(iVar2 + 4) = piVar3;
    *piVar3 = iVar2;
    piVar3 = piRam00000774;
    piVar1 = param_1;
    *piRam00000774 = (int)param_1;
    piRam00000774 = piVar1;
    param_1[1] = (int)piVar3;
    *param_1 = (int)&DAT_80065a74;
    param_1[2] = 0;
    FUN_80020540(iVar4);
  }
  return;
}

