// addr: 0x80054bb8  name: FUN_80054bb8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80054bb8(void)

{
  int iVar1;
  
  _I_STAT = 0xffffff7f;
  do {
    if (((ushort)JOY_MCD_STAT & 0x80) == 0) {
      JOY_MCD_CTRL = JOY_MCD_CTRL | 0x10;
      return 1;
    }
    iVar1 = FUN_800566d4();
  } while (iVar1 == 0);
  return 0;
}

