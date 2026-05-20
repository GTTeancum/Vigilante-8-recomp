// addr: 0x80015644  name: FUN_80015644

void FUN_80015644(char param_1)

{
  bool bVar1;
  CdlLOC aCStack_10 [2];
  
  if (param_1 == '\x01') {
    if (pvRam000006a8 == pvRam000006a4) {
      CdGetSector(pvRam000006a8,0x200);
      bVar1 = pvRam000006a8 == pvRam000006a0;
      pvRam000006a8 = pvRam000006a0;
      if (bVar1) {
        pvRam000006a8 = (void *)((int)pvRam000006a0 + 0x800);
      }
      iRam000006b0 = iRam000006b0 + 1;
    }
    else {
      CdIntToPos(iRam000006b0,aCStack_10);
      CdControl('\x06',&aCStack_10[0].minute,(u_char *)0x0);
    }
  }
  return;
}

