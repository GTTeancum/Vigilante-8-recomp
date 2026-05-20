// addr: 0x80025400  name: FUN_80025400

int FUN_80025400(uint param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  
  uVar5 = param_1 & 0xffff;
  uVar8 = param_2 & 0xffff;
  uVar7 = param_1 >> 0x10;
  uVar9 = param_2 >> 0x10;
  if (uVar5 + uVar8 < 0x10000) {
    iVar2 = (uVar9 & 0x3f) * 2;
    iVar3 = (uVar7 & 0x3f) * 0x80;
    uVar1 = *(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (param_2 >> 0x16)] + iVar2 + iVar3) & 0x7ff
    ;
    iVar2 = uVar1 * 0x10000 +
            uVar5 * ((*(ushort *)
                       ((&DAT_800911a0)[(uVar7 + 1 >> 6) * 0x20 + (param_2 >> 0x16)] +
                       iVar2 + (uVar7 + 1 & 0x3f) * 0x80) & 0x7ff) - uVar1) +
            uVar8 * ((*(ushort *)
                       ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (uVar9 + 1 >> 6)] +
                       (uVar9 + 1 & 0x3f) * 2 + iVar3) & 0x7ff) - uVar1);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0x1f;
    }
    return iVar2 >> 5;
  }
  uVar1 = uVar9 + 1 >> 6;
  uVar6 = uVar7 + 1 >> 6;
  iVar2 = (uVar9 + 1 & 0x3f) * 2;
  iVar3 = (uVar7 + 1 & 0x3f) * 0x80;
  uVar4 = *(ushort *)((&DAT_800911a0)[uVar6 * 0x20 + uVar1] + iVar2 + iVar3) & 0x7ff;
  iVar2 = uVar4 * 0x10000 +
          (0x10000 - uVar5) *
          ((*(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + uVar1] + iVar2 + (uVar7 & 0x3f) * 0x80) &
           0x7ff) - uVar4) +
          (0x10000 - uVar8) *
          ((*(ushort *)
             ((&DAT_800911a0)[uVar6 * 0x20 + (param_2 >> 0x16)] + (uVar9 & 0x3f) * 2 + iVar3) &
           0x7ff) - uVar4);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x1f;
  }
  return iVar2 >> 5;
}

