// addr: 0x8004adb0  name: CDREAD_OBJ_9C

void CDREAD_OBJ_9C(void)

{
  int iVar1;
  
  iVar1 = CdPosToInt((CdlLOC *)&stack0x00000010);
  if (iVar1 != DAT_800603b8) {
    puts("CdRead: sector error\n");
    DAT_800603ac = -1;
  }
  if ((DAT_800603c8 & 1) != 0) {
    CdGetSector2(DAT_800603a0,DAT_800603a8);
    CDREAD_OBJ_158();
    return;
  }
  CdGetSector(DAT_800603a0,DAT_800603a8);
  DAT_800603a0 = (void *)((int)DAT_800603a0 + DAT_800603a8 * 4);
  DAT_800603ac = DAT_800603ac + -1;
  DAT_800603b8 = DAT_800603b8 + 1;
  CDREAD_OBJ_158();
  return;
}

