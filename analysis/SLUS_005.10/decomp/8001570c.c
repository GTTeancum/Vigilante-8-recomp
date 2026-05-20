// addr: 0x8001570c  name: FUN_8001570c

void FUN_8001570c(int param_1)

{
  u_char *puVar1;
  uint uVar2;
  CdlLOC aCStack_18 [2];
  u_char auStack_10 [3];
  uint uStack_d;
  
  puVar1 = auStack_10 + 3;
  uVar2 = (uint)puVar1 & 3;
  *(uint *)(puVar1 + -uVar2) =
       *(uint *)(puVar1 + -uVar2) & -1 << (uVar2 + 1) * 8 | 0x80U >> (3 - uVar2) * 8;
  _auStack_10 = 0x80;
  iRam000006b0 = param_1;
  CdReadyCallback(FUN_80015644);
  uRam000006a0 = FUN_8001178c(0x800,2);
  uRam000006a4 = uRam000006a0;
  uRam000006a8 = uRam000006a0;
  CdControl('\x0e',auStack_10,(u_char *)0x0);
  CdIntToPos(iRam000006b0,aCStack_18);
  CdControl('\x06',&aCStack_18[0].minute,(u_char *)0x0);
  return;
}

