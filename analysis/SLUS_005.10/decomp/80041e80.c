// addr: 0x80041e80  name: FUN_80041e80

void FUN_80041e80(void)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  if (puRam000008cc != &UNK_80065bc8) {
    do {
      piVar2 = (int *)piRam000008c4[1];
      iVar1 = *piRam000008c4;
      *(int **)(iVar1 + 4) = piVar2;
      *piVar2 = iVar1;
      FUN_80040e18();
    } while (puRam000008cc != &UNK_80065bc8);
  }
  iVar1 = 0;
  if (0 < iRam000008c0) {
    do {
      iVar3 = iVar1 * 4;
      iVar1 = iVar1 + 1;
      FUN_80041da0(*(undefined4 *)(iVar3 + iRam000008d4));
    } while (iVar1 < iRam000008c0);
  }
  if (iRam000008d4 != 0) {
    FUN_80045088();
  }
  iVar1 = 0;
  if (0 < iRam000008bc) {
    iVar3 = 0;
    do {
      if (*(short *)(iVar3 + iRam000008d0) != 0) {
        if (*(int *)((short *)(iVar3 + iRam000008d0) + 6) != 0) {
          FUN_80045088();
        }
        if (*(int *)(iVar3 + iRam000008d0 + 0x10) != 0) {
          FUN_80045088();
        }
      }
      iVar1 = iVar1 + 1;
      iVar3 = iVar3 + 0x34;
    } while (iVar1 < iRam000008bc);
  }
  if (iRam000008d0 != 0) {
    FUN_80045088();
  }
  return;
}

