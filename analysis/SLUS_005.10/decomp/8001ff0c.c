// addr: 0x8001ff0c  name: FUN_8001ff0c

int * FUN_8001ff0c(int *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    if (piVar1[2] == param_2) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return piVar1;
}

