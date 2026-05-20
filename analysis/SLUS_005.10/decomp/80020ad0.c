// addr: 0x80020ad0  name: FUN_80020ad0

undefined4 FUN_80020ad0(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return 0;
    }
    iVar5 = piVar1[2];
    iVar3 = *(int *)(iVar5 + 0x30);
    if (((iVar3 != 0) && (*(int *)(iVar3 + 0x1c) != 0)) && (*(int *)(iVar3 + 0x20) != 0)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  FUN_80045088(*(undefined4 *)(iVar3 + (1 - iRam00000004) * 4 + 0x1c));
  iVar4 = (1 - iRam00000004) * 4;
  *(undefined4 *)(*(int *)(iVar5 + 0x30) + iVar4 + 0x1c) = 0;
  iVar3 = *(int *)(iVar5 + 0x68);
  if (iVar3 == 0) {
    return 1;
  }
  if (*(int *)(iVar3 + 0x1c) == 0) {
    return 1;
  }
  if (*(int *)(iVar3 + 0x20) != 0) {
    FUN_80045088(*(undefined4 *)(iVar3 + iVar4 + 0x1c));
    *(undefined4 *)(*(int *)(iVar5 + 0x68) + (1 - iRam00000004) * 4 + 0x1c) = 0;
  }
  return 1;
}

