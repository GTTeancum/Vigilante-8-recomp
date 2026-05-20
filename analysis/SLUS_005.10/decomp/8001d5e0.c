// addr: 0x8001d5e0  name: FUN_8001d5e0

int FUN_8001d5e0(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x3c);
  while (iVar1 != 0) {
    param_1 = FUN_8001d5a0();
    iVar1 = *(int *)(param_1 + 0x3c);
  }
  return param_1;
}

