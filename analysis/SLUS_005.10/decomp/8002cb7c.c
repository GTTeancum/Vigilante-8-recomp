// addr: 0x8002cb7c  name: FUN_8002cb7c

void FUN_8002cb7c(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = FUN_8001d5a0();
  if (iVar1 != 0) {
    iVar3 = 0;
    iVar2 = 0x24;
    do {
      if (*(int *)(iVar1 + iVar2 + 0xec) == param_1) {
        FUN_8002ca94();
        return;
      }
      iVar3 = iVar3 + 1;
      iVar2 = iVar2 + 4;
    } while (iVar3 < 3);
  }
  return;
}

