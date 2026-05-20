// addr: 0x80048d24  name: CdInit

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int CdInit(void)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 4;
  do {
    iVar1 = CdReset(1);
    iVar2 = iVar2 + -1;
    if (iVar1 == 1) {
      CdSyncCallback(EVENT_OBJ_84);
      CdReadyCallback(EVENT_OBJ_AC);
      CdReadCallback(EVENT_OBJ_D4);
      CdReadMode(0);
      iVar2 = EVENT_OBJ_74();
      return iVar2;
    }
  } while (iVar2 != -1);
  printf("CdInit: Init failed\n");
  return 0;
}

