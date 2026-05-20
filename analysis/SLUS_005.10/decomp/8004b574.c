// addr: 0x8004b574  name: CdDiskReady

int CdDiskReady(int mode)

{
  int iVar1;
  int iVar2;
  byte local_18 [8];
  
  CdControlB('\x01',(u_char *)0x0,local_18);
  iVar1 = 0x10;
  if ((local_18[0] & 0x10) == 0) {
    iVar1 = CdControlB('\x13',(u_char *)0x0,local_18);
    if (mode == 1) {
      if (local_18[0] == 2) {
        if (iVar1 != 0) {
          return 2;
        }
        iVar1 = TYPE_OBJ_108();
        return iVar1;
      }
    }
    else {
      iVar2 = 0;
      do {
        if ((local_18[0] & 2) != 0) {
          if ((local_18[0] == 2) && (iVar1 != 0)) {
            return 2;
          }
          do {
            do {
              VSync(0x1e);
              iVar1 = CdControlB('\x13',(u_char *)0x0,local_18);
            } while (local_18[0] != 2);
          } while (iVar1 == 0);
          iVar1 = TYPE_OBJ_108();
          return iVar1;
        }
        VSync(0x1e);
        iVar1 = CdControlB('\x13',(u_char *)0x0,local_18);
        iVar2 = iVar2 + 1;
      } while (iVar2 < 10);
    }
    iVar1 = 5;
  }
  return iVar1;
}

