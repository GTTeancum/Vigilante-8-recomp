// addr: 0x80048fa8  name: CdSyncCallback

CdlCB CdSyncCallback(CdlCB func)

{
  CdlCB pCVar1;
  
  pCVar1 = DAT_8006007c;
  DAT_8006007c = func;
  return pCVar1;
}

