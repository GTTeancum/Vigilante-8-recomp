// addr: 0x80024888  name: FUN_80024888

int * FUN_80024888(int param_1,uint param_2,uint param_3)

{
  ushort uVar1;
  int *piVar2;
  ushort *puVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  piVar2 = piRam000007ec;
  if (piRam000007ec != (int *)0x0) {
    piRam000007ec = (int *)*piRam000007ec;
    uVar7 = (uint)*(byte *)(param_1 + 0x11);
    puVar3 = *(ushort **)(param_1 + 8);
    iVar5 = (int)(*(ushort *)(param_1 + 0xc) ^ param_2 | *(ushort *)(param_1 + 0xe) ^ param_3) >>
            (uVar7 & 0x1f);
    while( true ) {
      iVar5 = iVar5 >> 1;
      uVar7 = uVar7 + 1;
      if (iVar5 == 0) break;
      if (*puVar3 == 0) {
        return (int *)0x0;
      }
      puVar3 = puVar3 + (uint)*puVar3 * -5;
    }
    uVar6 = param_2 << (0x20 - uVar7 & 0x1f);
    iVar5 = param_3 << (0x20 - uVar7 & 0x1f);
    while( true ) {
      uVar7 = uVar7 - 1;
      uVar4 = uVar6 >> 0x1f;
      if (iVar5 < 0) {
        uVar4 = uVar4 | 2;
      }
      uVar1 = puVar3[uVar4 + 1];
      if ((uVar1 == 0) || (uVar6 = uVar6 << 1, (uVar1 & 0x8000) != 0)) break;
      iVar5 = iVar5 << 1;
      puVar3 = puVar3 + (uint)uVar1 * 5;
    }
    uVar1 = (ushort)(-1 << (uVar7 & 0x1f));
    piVar2[2] = (int)puVar3;
    *(char *)(piVar2 + 4) = (char)uVar4;
    *(char *)((int)piVar2 + 0x11) = (char)uVar7;
    *(ushort *)(piVar2 + 3) = (ushort)param_2 & uVar1;
    *(ushort *)((int)piVar2 + 0xe) = (ushort)param_3 & uVar1;
  }
  return piVar2;
}

