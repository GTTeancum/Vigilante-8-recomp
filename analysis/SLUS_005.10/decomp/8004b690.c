// addr: 0x8004b690  name: CdGetDiskType

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int CdGetDiskType(void)

{
  int iVar1;
  int iVar2;
  CdlLOC aCStack_820 [2];
  undefined1 uStack_818;
  char acStack_817 [2047];
  byte local_18;
  byte local_17;
  
  CdControl('\x01',(u_char *)0x0,&local_18);
  iVar1 = 0x10;
  if ((local_18 & 0x10) == 0) {
    CdIntToPos(0x10,aCStack_820);
    CdControl('\x1b',&aCStack_820[0].minute,(u_char *)0x0);
    iVar1 = 0;
    while( true ) {
      iVar2 = CdReady(0,&local_18);
      iVar1 = iVar1 + 1;
      if ((iVar2 == 1) || (9 < iVar1)) break;
      CdControl('\x1b',&aCStack_820[0].minute,(u_char *)0x0);
    }
    if (iVar2 == 1) {
      CdControl('\t',(u_char *)0x0,(u_char *)0x0);
      CdGetSector(&uStack_818,0x200);
      iVar2 = strncmp(acStack_817,"CD001",5);
      iVar1 = 1;
      if (iVar2 == 0) {
        iVar1 = 2;
      }
    }
    else {
      iVar1 = 0x10;
      if ((local_18 & 0x10) == 0) {
        if (((local_18 & 1) != 0) && ((local_17 & 0x40) != 0)) {
          printf("Command Error: ");
          iVar1 = TYPE_OBJ_250();
          return iVar1;
        }
        iVar1 = TYPE_OBJ_250();
        return iVar1;
      }
    }
  }
  return iVar1;
}

