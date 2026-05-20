// addr: 0x8003fc94  name: FUN_8003fc94

int FUN_8003fc94(int param_1)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar5 = **(int **)(param_1 + 0x58);
  uVar1 = *(ushort *)(iVar5 + (uint)*(ushort *)(param_1 + 10) * 0x1c + 0x36);
  iVar4 = 0;
  while (uVar2 = (uint)uVar1, uVar2 != 0xffff) {
    iVar3 = iVar5 + uVar2 * 0x1c;
    uVar1 = *(ushort *)(iVar3 + 0x1c);
    if ((uVar1 >> 8 == 0xff) && (uVar1 != 0xffff)) {
      uVar1 = *(ushort *)(iVar3 + 0x36);
      iVar4 = iVar4 + 1;
    }
    else {
      uVar1 = *(ushort *)(iVar5 + uVar2 * 0x1c + 0x34);
    }
  }
  return iVar4;
}

