// addr: 0x800566d4  name: FUN_800566d4

bool FUN_800566d4(void)

{
  uint uVar1;
  
  uVar1 = (uint)(ushort)TMR_SYSCLOCK_VAL;
  if (uVar1 < DAT_800a4f08) {
    if ((ushort)TMR_SYSCLOCK_MAX == 0) {
      uVar1 = uVar1 + 0x10000;
    }
    else {
      uVar1 = uVar1 + (ushort)TMR_SYSCLOCK_MAX;
    }
  }
  if (((ushort)TMR_SYSCLOCK_MODE & 0x200) == 0) {
    uVar1 = uVar1 - DAT_800a4f08 >> 3;
  }
  else {
    uVar1 = uVar1 - DAT_800a4f08;
  }
  return DAT_800a4f0c <= uVar1;
}

