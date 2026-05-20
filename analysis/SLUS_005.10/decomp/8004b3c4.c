// addr: 0x8004b3c4  name: CdReadSync

int CdReadSync(int mode,u_char *result)

{
  int iVar1;
  int iVar2;
  
  while( true ) {
    iVar1 = VSync(-1);
    iVar2 = -1;
    if ((iVar1 <= DAT_800603b4 + 0x4b0) &&
       ((DAT_800603ac < 0 || (iVar1 = VSync(-1), iVar2 = DAT_800603ac, DAT_800603b0 + 0x3c < iVar1))
       )) break;
    if ((mode != 0) || (iVar2 < 1)) {
      CdReady(1,result);
      return iVar2;
    }
  }
  CDREAD_OBJ_32C(1);
  iVar2 = CDREAD_OBJ_740();
  return iVar2;
}

