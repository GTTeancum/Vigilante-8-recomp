// addr: 0x80043aec  name: FUN_80043aec

uint FUN_80043aec(void)

{
  int iVar1;
  
  CdDiskReady(0);
  iVar1 = CdGetDiskType();
  if (iVar1 == 1) {
    uRam000005ac = 2;
  }
  else if (iVar1 < 2) {
    if (iVar1 == 0) {
      uRam000005ac = 1;
      uRam000008f8 = 0;
    }
  }
  else if (iVar1 == 2) {
    iVar1 = FUN_80015610();
    uRam000005ac = 3;
    if (iVar1 != 0) {
      uRam000005ac = 4;
    }
  }
  else if (iVar1 == 0x10) {
    uRam000005ac = 0;
    uRam000008f8 = 0;
  }
  if (1 < uRam000005ac) {
    FUN_80043a74();
  }
  return uRam000005ac;
}

