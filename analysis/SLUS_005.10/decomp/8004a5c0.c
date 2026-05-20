// addr: 0x8004a5c0  name: CD_initvol

undefined4 CD_initvol(void)

{
  if ((CURR_MAIN_VOL_L == 0) && (CURR_MAIN_VOL_R == 0)) {
    SPU_MAIN_VOL_L = 0x3fff;
    SPU_MAIN_VOL_R = 0x3fff;
  }
  CD_VOL_L = 0x3fff;
  CD_VOL_R = 0x3fff;
  SPU_CTRL_REG_CPUCNT = 0xc001;
  CDROM_REG0 = 3;
  CDROM_REG1 = 0x80;
  CDROM_REG2 = 0;
  CDROM_REG3 = 0x20;
  return 0;
}

