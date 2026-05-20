// addr: 0x8001d4f0  name: FUN_8001d4f0

void FUN_8001d4f0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(param_1 + 0x38);
  if (iVar1 != 0) {
    iVar2 = *(int *)(iVar1 + 0x34);
    while (iVar2 != 0) {
      iVar1 = *(int *)(iVar1 + 0x34);
      iVar2 = *(int *)(iVar1 + 0x34);
    }
    *(int *)(iVar1 + 0x34) = param_2;
    *(int *)(param_2 + 0x3c) = iVar1;
    return;
  }
  *(int *)(param_1 + 0x38) = param_2;
  *(int *)(param_2 + 0x3c) = param_1;
  return;
}

