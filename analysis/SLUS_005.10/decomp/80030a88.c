// addr: 0x80030a88  name: FUN_80030a88

void FUN_80030a88(int param_1,short *param_2)

{
  short sVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  short *psVar5;
  uint uVar6;
  
  sVar1 = *param_2;
  uVar6 = *(uint *)(param_2 + 4);
  psVar5 = (short *)(param_1 + 0xa4);
  if (sVar1 == 3) {
    *(ushort *)(param_1 + 0xa4) = (*(byte *)(param_2 + 8) - 0x80) * 5;
    *(short *)(param_1 + 0xa6) =
         (short)((int)((uint)*(byte *)((int)param_2 + 0x11) - (uint)*(byte *)(param_2 + 9)) / 2);
    return;
  }
  if (3 < sVar1) {
    if (sVar1 < 6) {
      iVar3 = *(byte *)(param_2 + 8) - 0x80;
      iVar2 = iVar3;
      if (iVar3 < 0) {
        iVar2 = -iVar3;
      }
      *psVar5 = (short)((iVar2 * iVar3) / 0x18);
      if ((uVar6 & 0x100) == 0) {
        *(ushort *)(param_1 + 0xa6) = 0x80 - (ushort)*(byte *)((int)param_2 + 0x11);
        return;
      }
      *(undefined2 *)(param_1 + 0xa6) = 0x80;
    }
    return;
  }
  if (sVar1 != 2) {
    return;
  }
  uVar4 = uVar6 >> 8 & 1;
  if ((uVar6 & 0x200) == 0) {
    sVar1 = (short)(uVar4 << 7);
  }
  else {
    sVar1 = ((short)uVar4 + -1) * 0x80;
  }
  *(short *)(param_1 + 0xa6) = sVar1;
  if (((uVar6 & 0x800) != 0) && (-0x2aa < *psVar5)) {
    *psVar5 = *psVar5 + -0x10;
    return;
  }
  if (((uVar6 & 0x1000) != 0) && (*psVar5 < 0x2aa)) {
    *psVar5 = *psVar5 + 0x10;
    return;
  }
  iVar2 = (int)*psVar5;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xf;
  }
  *psVar5 = *psVar5 - (short)(iVar2 >> 4);
  return;
}

