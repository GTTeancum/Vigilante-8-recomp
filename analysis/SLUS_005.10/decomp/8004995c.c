// addr: 0x8004995c  name: BIOS_OBJ_3A8

void BIOS_OBJ_3A8(void)

{
  undefined1 uVar1;
  undefined1 *puVar2;
  int iVar3;
  undefined1 *puVar4;
  int unaff_s0;
  int unaff_s1;
  
  if ((unaff_s1 != 0) && (unaff_s0 == 1)) {
    unaff_s1 = 0;
  }
  DAT_8006035d = 1;
  if (unaff_s1 != 0) {
    DAT_8006035d = 5;
  }
  puVar2 = &DAT_800a3248;
  puVar4 = &stack0x00000018;
  iVar3 = 7;
  do {
    uVar1 = *puVar4;
    puVar4 = puVar4 + 1;
    iVar3 = iVar3 + -1;
    *puVar2 = uVar1;
    puVar2 = puVar2 + 1;
  } while (iVar3 != -1);
  CDROM_REG0 = 0;
  CDROM_REG3 = 0;
  BIOS_OBJ_548();
  return;
}

