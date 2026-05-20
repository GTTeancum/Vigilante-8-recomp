// addr: 0x80048eec  name: CdSetDebug

int CdSetDebug(int level)

{
  int iVar1;
  
  iVar1 = DAT_80060088;
  DAT_80060088 = level;
  return iVar1;
}

