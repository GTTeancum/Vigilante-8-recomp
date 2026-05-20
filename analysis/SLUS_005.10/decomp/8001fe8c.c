// addr: 0x8001fe8c  name: FUN_8001fe8c

undefined4 FUN_8001fe8c(undefined4 *param_1,int param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  int *piVar3;
  int iVar4;
  
  piVar1 = (int *)*param_1;
  piVar3 = *(int **)*param_1;
  while( true ) {
    if (piVar3 == (int *)0x0) {
      return 0;
    }
    if (piVar1[2] == param_2) break;
    piVar1 = piVar3;
    piVar3 = (int *)*piVar3;
  }
  piVar3 = (int *)piVar1[1];
  iVar4 = *piVar1;
  *(int **)(iVar4 + 4) = piVar3;
  *piVar3 = iVar4;
  puVar2 = piRam00000774;
  piVar3 = piVar1;
  *piRam00000774 = (int)piVar1;
  piRam00000774 = piVar3;
  piVar1[1] = (int)puVar2;
  *piVar1 = (int)&DAT_80065a74;
  piVar1[2] = 0;
  return 1;
}

