// addr: 0x8001fe50  name: FUN_8001fe50

void FUN_8001fe50(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = piRam0000076c;
  iVar3 = *piRam0000076c;
  *(undefined **)(iVar3 + 4) = &DAT_80065a70;
  piVar2 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar3;
  *piVar2 = param_2;
  piVar2 = *(int **)(param_1 + 8);
  *(int **)(param_1 + 8) = piVar1;
  *piVar2 = (int)piVar1;
  piVar1[1] = (int)piVar2;
  *piVar1 = param_1 + 4;
  return;
}

