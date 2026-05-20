// addr: 0x80043c34  name: FUN_80043c34

void FUN_80043c34(char param_1,int param_2)

{
  int iVar1;
  CdlLOC local_10 [2];
  
  if ((param_1 == '\x01') && ((*(byte *)(param_2 + 4) & 0x80) == 0)) {
    local_10[0].minute = *(u_char *)(param_2 + 3);
    local_10[0].second = *(u_char *)(param_2 + 4);
    local_10[0].sector = '\0';
    iVar1 = CdPosToInt(local_10);
    if (iRam000008e0 < iVar1) {
      if (cRam000008df == '\0') {
        CdControl('\x03',(u_char *)&DAT_80065be0,(u_char *)0x0);
      }
      else {
        CdControl('\t',(u_char *)0x0,(u_char *)0x0);
        CdReadyCallback((CdlCB)0x0);
      }
    }
  }
  return;
}

