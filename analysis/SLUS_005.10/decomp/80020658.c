// addr: 0x80020658  name: FUN_80020658

void FUN_80020658(undefined4 *param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  
  puVar2 = (undefined4 *)param_1[2];
  while (puVar2 != param_1) {
    piVar5 = (int *)*param_1;
    FUN_80020540(piVar5[2]);
    piVar4 = (int *)piVar5[1];
    iVar3 = *piVar5;
    *(int **)(iVar3 + 4) = piVar4;
    *piVar4 = iVar3;
    piVar4 = piRam00000774;
    piVar1 = piVar5;
    *piRam00000774 = (int)piVar5;
    piRam00000774 = piVar1;
    piVar5[1] = (int)piVar4;
    *piVar5 = (int)&DAT_80065a74;
    piVar5[2] = 0;
    puVar2 = (undefined4 *)param_1[2];
  }
  return;
}

