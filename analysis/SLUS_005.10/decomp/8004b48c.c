// addr: 0x8004b48c  name: CdReadCallback

CdlCB CdReadCallback(CdlCB func)

{
  CdlCB pCVar1;
  
  pCVar1 = DAT_80060394;
  DAT_80060394 = func;
  return pCVar1;
}

