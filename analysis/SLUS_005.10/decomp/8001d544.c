// addr: 0x8001d544  name: FUN_8001d544

void FUN_8001d544(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x38);
  *(int *)(param_1 + 0x38) = param_2;
  *(int *)(param_2 + 0x3c) = param_1;
  *(int *)(param_2 + 0x34) = iVar1;
  if (iVar1 != 0) {
    *(int *)(iVar1 + 0x3c) = param_2;
  }
  return;
}

