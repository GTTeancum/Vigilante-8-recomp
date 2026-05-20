// addr: 0x80015288  name: FUN_80015288

void FUN_80015288(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  
  FUN_80019960(piRam00000684,param_1,iRam00000688,iRam0000068c);
  iVar1 = iRam00000690;
  iVar2 = FUN_80019138(piRam00000684,param_1);
  if (iVar2 < iVar1) {
    iVar2 = iVar1;
  }
  iRam0000068c = iRam0000068c + (uint)*(byte *)(*piRam00000684 + 6);
  iRam00000690 = iVar2;
  if (iRam00000694 < (int)(iRam0000068c + (uint)*(byte *)(*piRam00000684 + 6))) {
    iRam0000068c = 0x20;
    iRam00000690 = 0;
    iRam00000688 = iRam00000688 + 8 + iVar2;
  }
  return;
}

