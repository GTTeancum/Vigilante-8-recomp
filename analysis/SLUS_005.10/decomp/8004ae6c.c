// addr: 0x8004ae6c  name: CDREAD_OBJ_158

void CDREAD_OBJ_158(void)

{
  int iVar1;
  undefined4 uVar2;
  
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
      (*DAT_80060394)(uVar2);
    }
  }
  return;
}

