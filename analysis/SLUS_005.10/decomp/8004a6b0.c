// addr: 0x8004a6b0  name: CD_initintr

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void CD_initintr(void)

{
  DAT_80060080 = 0;
  DAT_8006007c = 0;
  DAT_80060090 = 0;
  _DAT_8006008c = 0;
  ResetCallback();
  InterruptCallback(2,BIOS_OBJ_1688);
  return;
}

