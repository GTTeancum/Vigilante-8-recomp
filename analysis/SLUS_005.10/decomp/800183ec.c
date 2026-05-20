// addr: 0x800183ec  name: FUN_800183ec

void FUN_800183ec(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0xc);
  while (iVar1 = iVar2, iVar1 != 0) {
    iVar2 = *(int *)(iVar1 + 0x10);
    if (param_1 == iVar2) {
      iVar2 = *(int *)(iVar1 + 0x14);
    }
    if (*(int *)(iVar2 + 8) != 0) break;
    FUN_80045088();
    FUN_80045088(iVar2);
    param_1 = iVar1;
    iVar2 = *(int *)(iVar1 + 0xc);
  }
  *(undefined4 *)(param_1 + 8) = 0;
  return;
}

