// addr: 0x800493ac  name: CdGetSector

int CdGetSector(void *madr,int size)

{
  int iVar1;
  
  iVar1 = CD_getsector();
  return (uint)(iVar1 == 0);
}

