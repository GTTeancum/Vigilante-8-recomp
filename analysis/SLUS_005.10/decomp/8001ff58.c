// addr: 0x8001ff58  name: FUN_8001ff58

int * FUN_8001ff58(int *param_1,int param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    if ((piVar1[2] != param_3) && (*(short *)(piVar1[2] + 6) == param_2)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return piVar1;
}

