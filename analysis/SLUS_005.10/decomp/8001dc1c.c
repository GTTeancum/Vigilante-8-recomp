// addr: 0x8001dc1c  name: FUN_8001dc1c

int FUN_8001dc1c(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar3 = *(int *)(param_1 + 0x30);
  iVar5 = 0;
  if (iVar3 != 0) {
    iVar5 = (uint)*(ushort *)(iVar3 + 0x24) << (0x10 - *(ushort *)(iVar3 + 0x26) & 0x1f);
  }
  for (iVar3 = *(int *)(param_1 + 0x38); iVar3 != 0; iVar3 = *(int *)(iVar3 + 0x34)) {
    iVar1 = FUN_8001dc1c(iVar3);
    iVar2 = FUN_80016a20(iVar3 + 0x24);
    iVar4 = iVar1 + iVar2;
    if (iVar1 + iVar2 < iVar5) {
      iVar4 = iVar5;
    }
    iVar5 = iVar4;
  }
  *(int *)(param_1 + 0x54) = iVar5;
  return iVar5;
}

