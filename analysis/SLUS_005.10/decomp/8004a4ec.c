// addr: 0x8004a4ec  name: CD_flush

void CD_flush(void)

{
  if ((CDROM_REG3 & 7) != 0) {
    do {
    } while( true );
  }
  DAT_8006035e = 0;
  DAT_8006035d = 0;
  DAT_8006035c = 2;
  CDROM_REG0 = 0;
  CDROM_REG3 = 0;
  COMMON_DELAY = 0x1325;
  return;
}

