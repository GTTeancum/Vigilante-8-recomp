// addr: 0x8004ac70  name: BIOS_OBJ_16BC

void BIOS_OBJ_16BC(void)

{
  uint uVar1;
  undefined1 *unaff_s1;
  byte unaff_s2;
  undefined1 *unaff_s3;
  
  while( true ) {
    uVar1 = BIOS_OBJ_0();
    if (uVar1 == 0) break;
    if (((uVar1 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
      (*DAT_80060080)(*unaff_s1,&DAT_800a3248);
    }
    if (((uVar1 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
      (*DAT_8006007c)(*unaff_s3,&DAT_800a3240);
    }
  }
  CDROM_REG0 = unaff_s2;
  return;
}

