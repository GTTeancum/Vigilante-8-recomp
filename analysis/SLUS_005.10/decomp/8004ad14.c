// addr: 0x8004ad14  name: CDREAD_OBJ_0

void CDREAD_OBJ_0(char param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  CdlLOC aCStack_20 [4];
  
  DAT_800603cc = param_2;
  if (param_1 == '\x01') {
    if (0 < DAT_800603ac) {
      if (DAT_800603a8 == 0x200) {
        if ((DAT_800603c8 & 1) != 0) {
          CdDataCallback((func *)0x0);
          CdGetSector2(aCStack_20,3);
          CdDataSync(0);
          CdDataCallback(CDREAD_OBJ_260);
          CDREAD_OBJ_9C();
          return;
        }
        CdGetSector(aCStack_20,3);
        iVar1 = CdPosToInt(aCStack_20);
        if (iVar1 != DAT_800603b8) {
          puts("CdRead: sector error\n");
          DAT_800603ac = -1;
        }
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
  }
  else {
    DAT_800603ac = -1;
  }
  DAT_800603b0 = VSync(-1);
  if (DAT_800603ac < 0) {
    CDREAD_OBJ_32C(1);
  }
  iVar1 = VSync(-1);
  if (DAT_800603b4 + 0x4b0 < iVar1) {
    DAT_800603ac = -1;
  }
  if ((DAT_800603ac == 0) || (iVar1 = VSync(-1), DAT_800603b4 + 0x4b0 < iVar1)) {
    CdSyncCallback(DAT_800603bc);
    CdReadyCallback(DAT_800603c0);
    if ((DAT_800603c8 & 1) != 0) {
      CdDataCallback(DAT_800603c4);
    }
    CdControlF('\t',(u_char *)0x0);
    if (DAT_80060394 != (code *)0x0) {
      uVar2 = 5;
      if (DAT_800603ac == 0) {
        uVar2 = 2;
      }
      (*DAT_80060394)(uVar2,param_2);
    }
  }
  return;
}

