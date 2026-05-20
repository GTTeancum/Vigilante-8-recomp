// addr: 0x80033db4  name: FUN_80033db4

void FUN_80033db4(int param_1,int param_2,int param_3,int param_4)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  ushort *puVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  short *psVar11;
  uint uVar12;
  
  iVar2 = (param_1 - param_3) + 0xffff;
  if (iVar2 < 0) {
    iVar2 = (param_1 - param_3) + 0x1fffe;
  }
  iVar6 = param_1 + param_3;
  uVar9 = iVar2 >> 0x10;
  if (iVar6 < 0) {
    iVar6 = iVar6 + 0xffff;
  }
  iVar2 = (param_2 - param_3) + 0xffff;
  if (iVar2 < 0) {
    iVar2 = (param_2 - param_3) + 0x1fffe;
  }
  uVar8 = iVar2 >> 0x10;
  iVar2 = param_2 + param_3;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xffff;
  }
  uVar12 = iVar2 >> 0x10;
  iVar3 = FUN_8001d470((((iVar6 >> 0x10) - uVar9) + 1) * ((uVar12 - uVar8) + 1) * 2 + 0x80);
  *(int *)(iVar3 + 0x48) = param_1;
  *(int *)(iVar3 + 0x50) = param_2;
  *(undefined1 **)(iVar3 + 100) = &LAB_80033c74;
  iVar2 = (param_3 >> 8) * (param_3 >> 8);
  psVar11 = (short *)(iVar3 + 0x80);
  *(int *)(iVar3 + 0x54) = param_3;
  if (param_4 < 0) {
    param_4 = param_4 + 0x7ff;
  }
  for (; uVar9 <= (uint)(iVar6 >> 0x10); uVar9 = uVar9 + 1) {
    if (uVar8 <= uVar12) {
      iVar4 = (int)(uVar9 * 0x10000 - param_1) >> 8;
      uVar10 = uVar8;
      do {
        iVar5 = FUN_80029c64(uVar9,uVar10);
        if ((*(short *)(iVar5 + 0x16) == 0) &&
           (iVar5 = (int)(uVar10 * 0x10000 - param_2) >> 8, iVar5 = iVar4 * iVar4 + iVar5 * iVar5,
           iVar5 <= iVar2)) {
          puVar7 = (ushort *)
                   ((&DAT_800911a0)[(uVar9 >> 6) * 0x20 + (uVar10 >> 6)] + (uVar9 & 0x3f) * 0x80 +
                   (uVar10 & 0x3f) * 2);
          sVar1 = (short)(((iVar2 - iVar5) * (param_4 >> 0xb)) / iVar2);
          *puVar7 = *puVar7 + sVar1 & 0x7ff;
          *psVar11 = sVar1;
        }
        else {
          *psVar11 = 0;
        }
        uVar10 = uVar10 + 1;
        psVar11 = psVar11 + 1;
      } while (uVar10 <= uVar12);
    }
  }
  FUN_800422d8(param_1,param_2,param_3);
  FUN_80020890(iVar3,0x3c);
  return;
}

