// addr: 0x801005e0  name: FUN_801005e0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801005e0(int param_1,ushort param_2)

{
  int *piVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int local_48;
  int local_44;
  int local_38;
  int local_34;
  int local_28;
  int local_24;
  int local_20;
  
  *(ushort *)(param_1 + 0x46) = param_2;
  if ((param_2 & 0x7fff) < 0x7000) {
    uVar3 = (uint)(short)param_2;
    if ((int)uVar3 < 0) {
      uVar3 = 0xfffff000 - uVar3;
    }
    piVar1 = *(int **)(_DAT_800659fc + 0x80);
    for (piVar2 = (int *)**(int **)(_DAT_800659fc + 0x80);
        (piVar2 != (int *)0x0 && ((uint)piVar2[3] < uVar3)); piVar2 = (int *)*piVar2) {
      piVar1 = piVar2;
    }
    iVar7 = piVar1[2];
    iVar4 = (int)*(short *)(iVar7 + 0x42);
    iVar9 = piVar2[2];
    uVar3 = ((uVar3 - piVar1[3]) * 0x100) / (uint)(piVar2[3] - piVar1[3]);
    if (iVar4 < 0) {
      iVar4 = -iVar4;
    }
    uVar6 = 0x8000;
    if (0x400 < iVar4) {
      uVar6 = 0x8001;
    }
    iVar4 = FUN_8001b038/*0x8001b038*/(iVar7,uVar6);
    GTE_RotateLongMatTrans/*0x80043408*/(iVar7 + 0x10,iVar4 + 4,&local_48);
    iVar4 = (int)*(short *)(iVar9 + 0x42);
    if (iVar4 < 0) {
      iVar4 = -iVar4;
    }
    uVar5 = 0x8000;
    if (0x400 < iVar4) {
      uVar5 = 0x8001;
    }
    if (-1 < (int)((uint)param_2 << 0x10)) {
      uVar5 = uVar5 ^ 1;
    }
    iVar4 = FUN_8001b038/*0x8001b038*/(iVar9,uVar5);
    GTE_RotateLongMatTrans/*0x80043408*/(iVar9 + 0x10,iVar4 + 4,&local_38);
    iVar4 = (local_38 - local_48) * uVar3;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xff;
    }
    *(int *)(param_1 + 0x48) = local_48 + (iVar4 >> 8);
    iVar4 = (local_34 - local_44) * uVar3;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xff;
    }
    *(int *)(param_1 + 0x4c) = local_44 + (iVar4 >> 8);
  }
  iVar4 = -0x7ffa0000;
  if (0xefff < param_2) {
    iVar4 = *(int *)(_DAT_800659fc + 0x80);
  }
  iVar8 = *(int *)(*(int *)(*(int *)(iVar4 + 0x59fc) + 0x88) + 8);
  iVar7 = FUN_8001b038/*0x8001b038*/(iVar8,0x8000);
  iVar9 = FUN_8001b038/*0x8001b038*/(iVar8,0x8001);
  local_28 = (*(int *)(iVar7 + 4) + *(int *)(iVar9 + 4)) / 2;
  local_24 = (*(int *)(iVar7 + 8) + *(int *)(iVar9 + 8)) / 2;
  iVar4 = *(int *)(iVar9 + 4) - *(int *)(iVar7 + 4);
  local_20 = (*(int *)(iVar7 + 0xc) + *(int *)(iVar9 + 0xc)) / 2;
  GTE_RotateLongMatTrans/*0x80043408*/(iVar8 + 0x10,&local_28,&local_28);
  uVar3 = -(int)(short)param_2 / 2;
  if (param_2 < 0xf000) {
    uVar3 = uVar3 + 0x800;
  }
  *(short *)(param_1 + 0x42) = (short)uVar3;
  iVar7 = *(short *)((uVar3 & 0xfff) * 4 + -0x7ff9f84a) * iVar4;
  if (iVar7 < 0) {
    iVar7 = iVar7 + 0x1fff;
  }
  *(int *)(param_1 + 0x48) = local_28 + (iVar7 >> 0xd);
  *(int *)(param_1 + 0x4c) = local_24;
  iVar4 = *(short *)((*(ushort *)(param_1 + 0x42) & 0xfff) * 4 + -0x7ff9f84c) * iVar4;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1fff;
  }
  *(int *)(param_1 + 0x50) = local_20 - (iVar4 >> 0xd);
  FUN_8001d708/*0x8001d708*/(param_1);
  return;
}

