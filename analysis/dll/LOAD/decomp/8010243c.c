// addr: 0x8010243c  name: FUN_8010243c

void FUN_8010243c(int param_1)

{
  int iVar1;
  
  iVar1 = Heap_AllocOrRetry/*0x800116f4*/(param_1 * 0x10 + 0x30);
  *(int *)(iVar1 + 0x1c) = param_1;
  return;
}

