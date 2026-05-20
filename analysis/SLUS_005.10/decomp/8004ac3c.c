// addr: 0x8004ac3c  name: BIOS_OBJ_1688

void BIOS_OBJ_1688(void)

{
  uint uVar1;
  byte bVar2;
  
  bVar2 = CDROM_REG0 & 3;
  while( true ) {
    uVar1 = BIOS_OBJ_0();
    if (uVar1 == 0) break;
    if (((uVar1 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
      (*DAT_80060080)(DAT_8006035d,&DAT_800a3248);
    }
    if (((uVar1 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
      (*DAT_8006007c)(DAT_8006035c,&DAT_800a3240);
    }
  }
  CDROM_REG0 = bVar2;
  return;
}

