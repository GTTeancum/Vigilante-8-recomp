// addr: 0x8004b230  name: CdReadBreak

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void CdReadBreak(void)

{
  if ((DAT_800603c8 & 1) != 0) {
    CdDataSync(0);
  }
  DAT_800603ac = 0;
  CdSyncCallback(DAT_800603bc);
  CdReadyCallback(DAT_800603c0);
  if ((DAT_800603c8 & 1) != 0) {
    CdDataCallback(DAT_800603c4);
  }
  CdControlF('\t',(u_char *)0x0);
  return;
}

