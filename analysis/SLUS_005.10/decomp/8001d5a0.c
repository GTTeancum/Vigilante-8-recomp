// addr: 0x8001d5a0  name: FUN_8001d5a0

int FUN_8001d5a0(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x3c);
  while ((iVar1 != 0 && (*(int *)(iVar1 + 0x38) != param_1))) {
    param_1 = *(int *)(param_1 + 0x3c);
    iVar1 = *(int *)(param_1 + 0x3c);
  }
  return iVar1;
}

