// addr: 0x80015010  name: FUN_80015010

uint FUN_80015010(void)

{
  uint uVar1;
  
  uVar1 = GetRCnt(0xf2000002);
  return uVar1 | (uint)DAT_800102f2 << 0x10;
}

