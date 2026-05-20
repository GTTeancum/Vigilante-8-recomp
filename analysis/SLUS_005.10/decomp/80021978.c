// addr: 0x80021978  name: FUN_80021978

int FUN_80021978(int *param_1,int param_2,int param_3,int *param_4)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return 0;
    }
    iVar3 = piVar1[2];
    if ((((param_2 <= *(short *)(iVar3 + 6)) && (*(short *)(iVar3 + 6) <= param_3)) &&
        (*param_4 < *(int *)(iVar3 + 0x48))) &&
       (((*(int *)(iVar3 + 0x48) < param_4[1] && (param_4[2] < *(int *)(iVar3 + 0x50))) &&
        (*(int *)(iVar3 + 0x50) < param_4[3])))) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return iVar3;
}

