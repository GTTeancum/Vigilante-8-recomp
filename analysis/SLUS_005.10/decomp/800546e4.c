// addr: 0x800546e4  name: FUN_800546e4

void FUN_800546e4(void)

{
  undefined **ppuVar1;
  int iVar2;
  
  ppuVar1 = &PTR_LAB_800652e0 + DAT_800652a0;
  DAT_800652a0 = DAT_800652a0 + 1;
  iVar2 = (*(code *)*ppuVar1)();
  if (iVar2 < 0) {
    (*DAT_8006525c)();
  }
  else {
    if (DAT_800652a0 != 0) {
      FUN_800566b4(0x3c);
      iVar2 = FUN_80054bb8();
      if (iVar2 == 0) {
        (*DAT_8006525c)(0xfffffffd);
      }
    }
    if (4 < DAT_800652a0) {
      DAT_800652a0 = DAT_800652a0 + -1;
    }
  }
  return;
}

