// addr: 0x8001fd9c  name: FUN_8001fd9c

int * FUN_8001fd9c(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  
  piVar3 = (int *)*piRam0000079c;
  piVar2 = piRam0000079c;
  while( true ) {
    piVar1 = piVar3;
    if (piVar1 == (int *)0x0) {
      return (int *)0x0;
    }
    if (*(short *)((int)piVar2 + 10) == param_1) break;
    piVar3 = (int *)*piVar1;
    piVar2 = piVar1;
  }
  return piVar2;
}

