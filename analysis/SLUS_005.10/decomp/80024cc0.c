// addr: 0x80024cc0  name: FUN_80024cc0

int FUN_80024cc0(int param_1,short param_2,short param_3)

{
  long lVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (1 << (*(byte *)(param_1 + 0x11) & 0x1f)) / 2;
  iVar3 = ((uint)*(ushort *)(param_1 + 0xc) + iVar2) - (int)param_2;
  iVar2 = ((uint)*(ushort *)(param_1 + 0xe) + iVar2) - (int)param_3;
  lVar1 = SquareRoot0(iVar3 * iVar3 + iVar2 * iVar2);
  return lVar1 << 7;
}

