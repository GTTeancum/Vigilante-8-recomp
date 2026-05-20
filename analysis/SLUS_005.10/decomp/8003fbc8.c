// addr: 0x8003fbc8  name: FUN_8003fbc8

uint FUN_8003fbc8(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  
  iVar3 = **(int **)(param_1 + 0x58);
  uVar1 = *(ushort *)(iVar3 + (uint)*(ushort *)(param_1 + 10) * 0x1c + 0x36);
  while( true ) {
    uVar2 = (uint)uVar1;
    if (uVar2 == 0xffff) {
      return 0;
    }
    uVar1 = *(ushort *)(iVar3 + uVar2 * 0x1c + 0x1c);
    if ((uVar1 >> 8 == 0xff) && (uVar1 != 0xffff)) break;
    uVar1 = *(ushort *)(iVar3 + uVar2 * 0x1c + 0x34);
  }
  return uVar2;
}

