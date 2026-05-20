// addr: 0x80041fd4  name: FUN_80041fd4

void FUN_80041fd4(void)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  int local_28;
  int local_24;
  int local_20;
  
  piVar3 = (int *)*piRam000008c4;
  piVar2 = piRam000008c4;
  while (piVar1 = piVar3, piVar1 != (int *)0x0) {
    iVar4 = FUN_8001db54(piVar2 + 3,piVar2[6]);
    if ((iVar4 != 0) && (FUN_80043408(&DAT_8006f680,piVar2 + 3,&local_28), local_20 < 0x200000)) {
      SetRotMatrix((MATRIX *)&DAT_8006f680);
      gte_ldtr(local_28 >> 8,local_24 >> 8,local_20 >> 8);
      FUN_80040e38(piVar2);
    }
    piVar2 = piVar1;
    piVar3 = (int *)*piVar1;
  }
  iVar4 = 0;
  if (0 < iRam000008c0) {
    do {
      if (*(int *)(*(int *)(iVar4 * 4 + iRam000008d4) + 0x18) != 0) {
        FUN_80041c5c();
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < iRam000008c0);
  }
  return;
}

