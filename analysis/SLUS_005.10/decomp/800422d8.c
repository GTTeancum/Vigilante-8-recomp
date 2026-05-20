// addr: 0x800422d8  name: FUN_800422d8

void FUN_800422d8(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  
  piVar3 = (int *)*piRam000008c4;
  piVar2 = piRam000008c4;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    iVar4 = piVar2[5] - param_2;
    if (iVar4 < 0) {
      iVar4 = -iVar4;
    }
    iVar5 = piVar2[3] - param_1;
    if (iVar5 < 0) {
      iVar5 = -iVar5;
    }
    if (iVar4 < iVar5) {
      iVar4 = iVar5;
    }
    if (iVar4 < piVar2[6] + param_3) {
      FUN_800420f4();
    }
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  return;
}

