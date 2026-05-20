// addr: 0x8004af74  name: CDREAD_OBJ_260

void CDREAD_OBJ_260(void)

{
  DAT_800603a0 = DAT_800603a0 + DAT_800603a8 * 4;
  DAT_800603ac = DAT_800603ac + -1;
  DAT_800603b8 = DAT_800603b8 + 1;
  if (DAT_800603ac == 0) {
    CdSyncCallback(DAT_800603bc);
    CdReadyCallback(DAT_800603c0);
    if ((DAT_800603c8 & 1) != 0) {
      CdDataCallback(DAT_800603c4);
    }
    CdControlF('\t',(u_char *)0x0);
    if (DAT_80060394 != (code *)0x0) {
      (*DAT_80060394)(2,DAT_800603cc);
    }
  }
  return;
}

