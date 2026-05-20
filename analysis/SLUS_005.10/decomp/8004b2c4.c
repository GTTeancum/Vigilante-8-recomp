// addr: 0x8004b2c4  name: CdRead

int CdRead(int sectors,u_long *buf,int mode)

{
  int iVar1;
  
  DAT_800603a4 = mode;
  if ((mode & 0x30U) == 0) {
    DAT_800603a8 = 0x200;
    iVar1 = CDREAD_OBJ_614();
    return iVar1;
  }
  if ((mode & 0x30U) != 0x20) {
    iVar1 = CDREAD_OBJ_608();
    return iVar1;
  }
  DAT_800603a8 = 0x249;
  iVar1 = CDREAD_OBJ_614();
  return iVar1;
}

