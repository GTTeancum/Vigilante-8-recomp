// addr: 0x8001d564  name: FUN_8001d564

int FUN_8001d564(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x3c);
  iVar1 = *(int *)(param_1 + 0x34);
  if (*(int *)(iVar2 + 0x38) == param_1) {
    *(int *)(iVar2 + 0x38) = iVar1;
  }
  else {
    *(int *)(iVar2 + 0x34) = iVar1;
  }
  if (iVar1 != 0) {
    *(int *)(iVar1 + 0x3c) = iVar2;
  }
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  return param_1;
}

