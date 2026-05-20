// addr: 0x800445b8  name: FUN_800445b8

int FUN_800445b8(void)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_80016a20();
  iVar2 = iVar1 + 0x200000;
  if (iVar2 < 0) {
    iVar2 = iVar1 + 0x200fff;
  }
  return (int)(short)(((int)sRam000008e4 << 9) / (iVar2 >> 0xc));
}

