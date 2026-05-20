// addr: 0x80105060  name: FUN_80105060

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80105060(undefined4 param_1,int param_2)

{
  undefined2 uVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 local_res0 [4];
  
  local_res0[0] = param_1;
  piVar2 = (int *)Heap_AllocOrRetry/*0x800116f4*/(0x20);
  if (param_2 == 0x16) {
    uVar1 = func_0x8002249c(local_res0);
    *(undefined2 *)((int)piVar2 + 10) = uVar1;
    uVar1 = func_0x8002249c(local_res0);
    *(undefined2 *)(piVar2 + 2) = uVar1;
    *(undefined2 *)(piVar2 + 3) = 0;
  }
  uVar1 = func_0x800224b4(local_res0);
  *(undefined2 *)((int)piVar2 + 10) = uVar1;
  uVar1 = func_0x800224b4(local_res0);
  *(undefined2 *)(piVar2 + 2) = uVar1;
  uVar1 = func_0x800224b4(local_res0);
  *(undefined2 *)(piVar2 + 3) = uVar1;
  iVar3 = func_0x800224b4(local_res0);
  *piVar2 = *(int *)(iVar3 * 4 + _DAT_80065bd8);
  iVar3 = func_0x800224b4(local_res0);
  piVar2[1] = *(int *)(iVar3 * 4 + _DAT_80065bd8);
  iVar3 = func_0x800224ec(local_res0);
  piVar2[4] = iVar3;
  iVar3 = func_0x800224ec(local_res0);
  piVar2[5] = iVar3;
  iVar3 = func_0x800224ec(local_res0);
  piVar2[6] = iVar3;
  iVar3 = func_0x800224ec(local_res0);
  piVar2[7] = iVar3;
  iVar5 = 0;
  iVar4 = *(int *)(*piVar2 + 0x1c);
  iVar3 = *piVar2;
  while (iVar4 != 0) {
    iVar5 = iVar5 + 1;
    iVar4 = *(int *)(iVar3 + 0x20);
    iVar3 = iVar3 + 4;
  }
  *(int **)(*piVar2 + iVar5 * 4 + 0x1c) = piVar2;
  iVar5 = 0;
  iVar4 = *(int *)(piVar2[1] + 0x1c);
  iVar3 = piVar2[1];
  while (iVar4 != 0) {
    iVar5 = iVar5 + 1;
    iVar4 = *(int *)(iVar3 + 0x20);
    iVar3 = iVar3 + 4;
  }
  *(int **)(piVar2[1] + iVar5 * 4 + 0x1c) = piVar2;
  return;
}

