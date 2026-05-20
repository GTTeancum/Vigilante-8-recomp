// addr: 0x800495b4  name: BIOS_OBJ_0

undefined4 BIOS_OBJ_0(void)

{
  undefined4 uVar1;
  
  CDROM_REG0 = 1;
  if ((CDROM_REG3 & 7) != 0) {
    uVar1 = BIOS_OBJ_64();
    return uVar1;
  }
  return 0;
}

