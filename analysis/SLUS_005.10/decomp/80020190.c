// addr: 0x80020190  name: FUN_80020190

uint * FUN_80020190(int *param_1,uint param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  uint *puVar3;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (uint *)0x0;
    }
    puVar3 = (uint *)piVar1[2];
    if ((((0x1f < *(short *)((int)puVar3 + 6)) && ((*puVar3 & param_2) != 0)) &&
        ((*puVar3 & 0x8002) == 0)) && (param_3 = param_3 + -1, param_3 == -1)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return puVar3;
}

