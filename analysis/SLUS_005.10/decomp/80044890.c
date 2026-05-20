// addr: 0x80044890  name: FUN_80044890

int FUN_80044890(void)

{
  int iVar1;
  
  iVar1 = FUN_80016a20();
  if (0x200000 - iVar1 < 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (0x200000 - iVar1 >> 0xc) * (int)sRam000008e4;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1ff;
    }
    iVar1 = (iVar1 << 7) >> 0x10;
  }
  return iVar1;
}

