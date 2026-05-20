// addr: 0x800420f4  name: FUN_800420f4

void FUN_800420f4(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  
  iVar7 = 0;
  if (-1 < *(int *)(param_1 + 0x1c)) {
    iVar6 = 8;
    iVar5 = param_1;
    do {
      iVar2 = *(int *)(param_1 + 0xc) + *(short *)(iVar5 + 0x20) * 0x100;
      iVar3 = *(int *)(param_1 + 0x14) + *(short *)(iVar5 + 0x24) * 0x100;
      iVar1 = FUN_80025400(iVar2,iVar3);
      *(short *)(iVar5 + 0x22) = (short)((uint)(iVar1 - *(int *)(param_1 + 0x10)) >> 8);
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xffff;
      }
      if (iVar3 < 0) {
        iVar3 = iVar3 + 0xffff;
      }
      iVar4 = param_1 + iVar6;
      *(ushort *)(iVar5 + 0x26) =
           (*(ushort *)
             ((&DAT_800911a0)[((uint)(iVar2 >> 0x10) >> 6) * 0x20 + ((uint)(iVar3 >> 0x10) >> 6)] +
             (iVar3 >> 0x10 & 0x3fU) * 2 + (iVar2 >> 0x10 & 0x3fU) * 0x80) >> 0xb) << 2;
      iVar3 = *(int *)(param_1 + 0xc) + *(short *)(iVar4 + 0x20) * 0x100;
      iVar2 = *(int *)(param_1 + 0x14) + *(short *)(iVar4 + 0x24) * 0x100;
      iVar1 = FUN_80025400(iVar3,iVar2);
      *(short *)(iVar4 + 0x22) = (short)((uint)(iVar1 - *(int *)(param_1 + 0x10)) >> 8);
      if (iVar3 < 0) {
        iVar3 = iVar3 + 0xffff;
      }
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xffff;
      }
      *(ushort *)(iVar4 + 0x26) =
           (*(ushort *)
             ((&DAT_800911a0)[((uint)(iVar3 >> 0x10) >> 6) * 0x20 + ((uint)(iVar2 >> 0x10) >> 6)] +
             (iVar2 >> 0x10 & 0x3fU) * 2 + (iVar3 >> 0x10 & 0x3fU) * 0x80) >> 0xb) << 2;
      iVar6 = iVar6 + 0x10;
      iVar7 = iVar7 + 1;
      iVar5 = iVar5 + 0x10;
    } while (iVar7 <= *(int *)(param_1 + 0x1c));
  }
  return;
}

