// addr: 0x80017160  name: FUN_80017160

uint FUN_80017160(void)

{
  uint uVar1;
  
  uVar1 = (uint)DAT_800568d8;
  DAT_800568d8 = (char)DAT_800568d4;
  uVar1 = (DAT_800568d4 >> 1) + uVar1 * -0x80000000 ^ DAT_800568d4 << 0xc;
  DAT_800568d4 = uVar1 ^ uVar1 >> 0x14;
  return DAT_800568d4 & 0x7fff;
}

