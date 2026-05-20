// addr: 0x8002131c  name: FUN_8002131c

void FUN_8002131c(undefined4 param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  piVar3 = (int *)*piRam0000075c;
  piVar2 = piRam0000075c;
  while ((piVar1 = piVar3, piVar1 != (int *)0x0 && (iVar4 = piVar2[2], iVar4 != 0))) {
    if (*(code **)(iVar4 + 100) != (code *)0x0) {
      (**(code **)(iVar4 + 100))(iVar4,0,param_1);
    }
    piVar3 = (int *)*piVar1;
    piVar2 = piVar1;
  }
  return;
}

