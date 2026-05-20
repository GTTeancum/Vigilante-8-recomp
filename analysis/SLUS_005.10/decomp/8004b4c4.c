// addr: 0x8004b4c4  name: CdRead2

int CdRead2(long mode)

{
  int iVar1;
  u_char local_10 [8];
  
  local_10[0] = (u_char)mode;
  CdControl('\x0e',local_10,(u_char *)0x0);
  if ((mode & 0x100U) != 0) {
    if ((mode & 0x20U) != 0) {
      DAT_800a3270 = 0;
      iVar1 = CDREAD2_OBJ_4C();
      return iVar1;
    }
    DAT_800a3270 = 1;
    CdDataCallback(data_ready_callback);
    CdReadyCallback(CDREAD2_OBJ_84);
  }
  iVar1 = CdControl('\x1b',(u_char *)0x0,(u_char *)0x0);
  return iVar1;
}

