// addr: 0x800493cc  name: CdGetSector2

int CdGetSector2(void *madr,int size)

{
  int iVar1;
  
  iVar1 = CD_getsector2();
  return (uint)(iVar1 == 0);
}

