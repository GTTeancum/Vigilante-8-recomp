// addr: 0x80043d94  name: FUN_80043d94

void FUN_80043d94(char param_1)

{
  int iVar1;
  CdlLOC aCStack_10 [2];
  
  if (param_1 == '\x01') {
    CdGetSector(aCStack_10,1);
    aCStack_10[0].track = '\0';
    iVar1 = CdPosToInt(aCStack_10);
    if (iRam000008e0 < iVar1) {
      CdControl('\t',(u_char *)0x0,(u_char *)0x0);
      CdReadyCallback((CdlCB)0x0);
    }
  }
  return;
}

