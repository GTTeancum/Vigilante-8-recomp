// addr: 0x8004a464  name: CD_vol

undefined4 CD_vol(int param_1)

{
  CDROM_REG0 = 3;
  CDROM_REG1 = *(byte *)(param_1 + 2);
  CDROM_REG2 = *(byte *)(param_1 + 3);
  CDROM_REG3 = 0x20;
  return 0;
}

