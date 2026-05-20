// addr: 0x80025648  name: FUN_80025648

short * FUN_80025648(uint param_1,uint param_2,short *param_3)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  ushort uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  
  uVar7 = param_1 >> 0x10;
  uVar8 = param_2 >> 0x10;
  if ((param_1 & 0xffff) + (param_2 & 0xffff) < 0x10000) {
    iVar3 = (uVar8 & 0x3f) * 2;
    iVar5 = (uVar7 & 0x3f) * 0x80;
    uVar4 = *(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (param_2 >> 0x16)] + iVar3 + iVar5);
    uVar1 = *(ushort *)
             ((&DAT_800911a0)[(uVar7 + 1 >> 6) * 0x20 + (param_2 >> 0x16)] +
             iVar3 + (uVar7 + 1 & 0x3f) * 0x80);
    param_3[1] = -0x20;
    uVar4 = uVar4 & 0x7ff;
    *param_3 = (uVar1 & 0x7ff) - uVar4;
    param_3[2] = (*(ushort *)
                   ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + (uVar8 + 1 >> 6)] +
                   (uVar8 + 1 & 0x3f) * 2 + iVar5) & 0x7ff) - uVar4;
  }
  else {
    uVar2 = uVar8 + 1 >> 6;
    uVar6 = uVar7 + 1 >> 6;
    iVar3 = (uVar8 + 1 & 0x3f) * 2;
    iVar5 = (uVar7 + 1 & 0x3f) * 0x80;
    uVar4 = *(ushort *)((&DAT_800911a0)[uVar6 * 0x20 + uVar2] + iVar3 + iVar5);
    uVar1 = *(ushort *)
             ((&DAT_800911a0)[(param_1 >> 0x16) * 0x20 + uVar2] + iVar3 + (uVar7 & 0x3f) * 0x80);
    param_3[1] = -0x20;
    uVar4 = uVar4 & 0x7ff;
    *param_3 = uVar4 - (uVar1 & 0x7ff);
    param_3[2] = uVar4 - (*(ushort *)
                           ((&DAT_800911a0)[uVar6 * 0x20 + (param_2 >> 0x16)] +
                           (uVar8 & 0x3f) * 2 + iVar5) & 0x7ff);
  }
  return param_3;
}

