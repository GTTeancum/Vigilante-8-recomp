// addr: 0x80048fbc  name: CdReadyCallback

CdlCB CdReadyCallback(CdlCB func)

{
  CdlCB pCVar1;
  
  pCVar1 = DAT_80060080;
  DAT_80060080 = func;
  return pCVar1;
}

