// addr: 0x800244c4  name: FUN_800244c4

uint FUN_800244c4(int param_1,int param_2)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  
  uVar3 = param_1 << 5;
  param_2 = param_2 << 5;
  iVar4 = iRam000006ec;
  while( true ) {
    uVar2 = uVar3 >> 0x1f;
    if (param_2 < 0) {
      uVar2 = uVar2 | 2;
    }
    uVar1 = *(ushort *)(iVar4 + uVar2 * 2 + 2);
    uVar2 = (uint)uVar1;
    if ((uVar2 == 0) || ((uVar1 & 0x8000) != 0)) break;
    iVar4 = iVar4 + uVar2 * 10;
    uVar3 = uVar3 << 1;
    param_2 = param_2 << 1;
  }
  return uVar2;
}

