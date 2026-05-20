// addr: 0x800157d4  name: FUN_800157d4

int FUN_800157d4(byte *param_1)

{
  int iVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  byte local_28 [16];
  
  if (*param_1 == 0x5c) {
    param_1 = param_1 + 1;
  }
  if (iRam000006b4 == 0) {
    return 0;
  }
  iVar3 = 0;
  iVar5 = iRam000006b4;
LAB_8001581c:
  do {
    bVar2 = *param_1;
    param_1 = param_1 + 1;
    if (0x60 < bVar2) {
      bVar2 = bVar2 - 0x20;
    }
    if ((bVar2 != 0) && (bVar2 != 0x5c)) {
      local_28[iVar3] = bVar2;
      iVar3 = iVar3 + 1;
      if (iVar3 < 0xc) goto LAB_8001581c;
    }
    for (; iVar3 < 0xc; iVar3 = iVar3 + 1) {
      local_28[iVar3] = 0x20;
    }
    if (bVar2 != 0x5c) {
      iVar3 = 0;
      if (*(int *)(iVar5 + 0x10) < 1) {
        return 0;
      }
      iVar4 = 0x14;
      do {
        iVar1 = thunk_FUN_800523a0(iVar5 + iVar4,local_28,0xc);
        iVar3 = iVar3 + 1;
        if (iVar1 == 0) {
          return iVar5 + iVar4;
        }
        iVar4 = iVar4 + 0x14;
      } while (iVar3 < *(int *)(iVar5 + 0x10));
      return 0;
    }
    iVar5 = *(int *)(iVar5 + 8);
    while( true ) {
      if (iVar5 == 0) {
        return 0;
      }
      iVar3 = thunk_FUN_800523a0(iVar5,local_28,8);
      if (iVar3 == 0) break;
      iVar5 = *(int *)(iVar5 + 0xc);
    }
    iVar3 = 0;
    if (iVar5 == 0) {
      return 0;
    }
  } while( true );
}

