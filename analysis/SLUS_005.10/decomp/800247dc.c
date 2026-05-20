// addr: 0x800247dc  name: FUN_800247dc

int * FUN_800247dc(int param_1,int param_2)

{
  ushort uVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  piVar2 = piRam000007ec;
  if (piRam000007ec != (int *)0x0) {
    piRam000007ec = (int *)*piRam000007ec;
  }
  uVar7 = 0xb;
  if (piVar2 != (int *)0x0) {
    uVar6 = param_1 << 0x15;
    iVar5 = param_2 << 0x15;
    iVar4 = iRam000006ec;
    while( true ) {
      uVar7 = uVar7 - 1;
      uVar3 = uVar6 >> 0x1f;
      if (iVar5 < 0) {
        uVar3 = uVar3 | 2;
      }
      uVar1 = *(ushort *)(iVar4 + uVar3 * 2 + 2);
      if ((uVar1 == 0) || (uVar6 = uVar6 << 1, (uVar1 & 0x8000) != 0)) break;
      iVar5 = iVar5 << 1;
      iVar4 = iVar4 + (uint)uVar1 * 10;
    }
    uVar1 = (ushort)(-1 << (uVar7 & 0x1f));
    piVar2[2] = iVar4;
    *(char *)(piVar2 + 4) = (char)uVar3;
    *(char *)((int)piVar2 + 0x11) = (char)uVar7;
    *(ushort *)(piVar2 + 3) = (ushort)param_1 & uVar1;
    *(ushort *)((int)piVar2 + 0xe) = (ushort)param_2 & uVar1;
  }
  return piVar2;
}

