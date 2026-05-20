// addr: 0x800212c4  name: FUN_800212c4

void FUN_800212c4(undefined2 param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  
  piVar3 = (int *)*piRam0000077c;
  piVar2 = piRam0000077c;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    FUN_8001fcb4(piVar2[2],param_1);
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  return;
}

