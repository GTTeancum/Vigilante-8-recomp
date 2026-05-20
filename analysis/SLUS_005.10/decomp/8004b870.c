// addr: 0x8004b870  name: StGetBackloc

int StGetBackloc(CdlLOC *loc)

{
  int iVar1;
  
  if (DAT_800a3270 == 0) {
    iVar1 = CdPosToInt((CdlLOC *)&DAT_800a3278);
    CdIntToPos(iVar1 + 1,loc);
    iVar1 = C_004_OBJ_D4();
    return iVar1;
  }
  return -1;
}

