// addr: 0x8001a91c  name: FUN_8001a91c

void FUN_8001a91c(int *param_1)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = *param_1;
  uVar1 = 0;
  if (0 < *(int *)(iVar2 + 0x10)) {
    do {
      FUN_8001b3d4(param_1,uVar1 & 0xffff);
      uVar1 = uVar1 + 1;
    } while ((int)uVar1 < *(int *)(iVar2 + 0x10));
  }
  FUN_80045134(iVar2,*(int *)(iVar2 + 0x14) - iVar2);
  return;
}

