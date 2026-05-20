// addr: 0x80049a5c  name: BIOS_OBJ_4A8

void BIOS_OBJ_4A8(void)

{
  undefined1 uVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  
  puVar3 = &DAT_800a3240;
  DAT_8006035d = 5;
  puVar4 = &stack0x00000018;
  DAT_8006035c = 5;
  iVar2 = 7;
  do {
    uVar1 = *puVar4;
    puVar4 = puVar4 + 1;
    iVar2 = iVar2 + -1;
    *puVar3 = uVar1;
    puVar3 = puVar3 + 1;
  } while (iVar2 != -1);
  puVar3 = &DAT_800a3248;
  puVar4 = &stack0x00000018;
  iVar2 = 7;
  do {
    uVar1 = *puVar4;
    puVar4 = puVar4 + 1;
    iVar2 = iVar2 + -1;
    *puVar3 = uVar1;
    puVar3 = puVar3 + 1;
  } while (iVar2 != -1);
  BIOS_OBJ_548();
  return;
}

