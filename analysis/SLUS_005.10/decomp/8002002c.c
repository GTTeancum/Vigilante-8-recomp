// addr: 0x8002002c  name: FUN_8002002c

void FUN_8002002c(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  code *pcVar3;
  int iVar4;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return;
    }
    pcVar3 = *(code **)(piVar1[2] + 100);
    if (pcVar3 == (code *)0x0) {
      iVar4 = 0;
    }
    else {
      iVar4 = (*pcVar3)(piVar1[2],param_2,param_3);
    }
    if (iVar4 != 0) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return;
}

