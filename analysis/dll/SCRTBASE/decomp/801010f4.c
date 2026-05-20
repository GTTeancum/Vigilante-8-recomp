// addr: 0x801010f4  name: FUN_801010f4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801010f4(uint *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  ushort uVar2;
  int iVar3;
  uint *puVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  uint uVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  int *piVar11;
  int iVar12;
  uint uVar13;
  int local_38;
  int local_34;
  int local_30;
  int local_28;
  int local_24;
  undefined4 local_20;
  
  puVar4 = param_1;
  switch(param_2) {
  case 0:
    break;
  case 2:
switchD_80101130_caseD_2:
    uVar5 = FUN_8004410c/*0x8004410c*/(puVar4);
    FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x39,param_1 + 9);
    FUN_800205f8/*0x800205f8*/(param_1);
    puVar4 = (uint *)0x1;
  case 1:
    cVar1 = FUN_8004410c/*0x8004410c*/(puVar4);
    *(char *)((int)param_1 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
  case 4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  default:
    return 0;
  case 3:
    goto switchD_80101130_caseD_3;
  }
  param_1[0x12] = param_1[0x12] + param_1[0x22];
  param_1[0x13] = param_1[0x13] + param_1[0x23];
  param_1[0x14] = param_1[0x14] + param_1[0x24];
  param_1[9] = param_1[0x12];
  param_1[10] = param_1[0x13];
  param_1[0xb] = param_1[0x14];
  if (param_3 != 0) {
    uVar5 = FUN_800446dc/*0x800446dc*/(param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
  }
  uVar2 = (short)param_1[0x25] + 1;
  *(ushort *)(param_1 + 0x25) = uVar2;
  if ((uVar2 & 3) == 0) {
    puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x13,0x98,8);
    *(undefined1 *)(puVar6 + 1) = 4;
    *puVar6 = 0x4b4;
    uVar7 = param_1[0x13];
    uVar13 = param_1[0x14];
    puVar6[0x12] = param_1[0x12];
    puVar6[0x13] = uVar7;
    puVar6[0x14] = uVar13;
    puVar6[0x19] = FUN_8010100c;
    *(short *)(puVar6 + 0x11) = (short)param_1[0x25] * 0x60;
    puVar6[0x22] = -(int)(short)param_1[5];
    puVar6[0x23] = -(int)*(short *)((int)param_1 + 0x1a);
    puVar6[0x24] = -(int)(short)param_1[8];
    FUN_8002036c/*0x8002036c*/();
  }
  if ((short)param_1[0x25] < 0xf0) {
    iVar8 = (short)param_1[5] * 0x1c;
    if (iVar8 < 0) {
      iVar8 = iVar8 + 0xfff;
    }
    iVar10 = *(short *)((int)param_1 + 0x1a) * 0x1c;
    param_1[0x22] = param_1[0x22] + (iVar8 >> 0xc);
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    iVar8 = (short)param_1[8] * 0x1c;
    param_1[0x23] = param_1[0x23] + (iVar10 >> 0xc);
    if (iVar8 < 0) {
      iVar8 = iVar8 + 0xfff;
    }
    param_1[0x24] = param_1[0x24] + (iVar8 >> 0xc);
  }
  uVar7 = param_1[0x17];
  piVar11 = (int *)param_1[0x21];
  if (piVar11 == (int *)0x0) {
    local_38 = 0;
    local_34 = 0x1000;
    local_30 = 0;
  }
  if ((short)piVar11[3] == 0) {
    param_1[0x21] = 0;
  }
  piVar9 = piVar11 + 0x12;
  if ((char)param_1[2] != '\0') {
    piVar11 = &local_38;
    FUN_800435c0/*0x800435c0*/(param_1 + 4,piVar9);
  }
  local_38 = piVar11[0x12] - param_1[0x12];
  local_34 = 0;
  local_30 = piVar11[0x14] - param_1[0x14];
  GTE_RotateLongMtxLow/*0x8004352c*/(param_1 + 4,&local_38,&local_38);
  iVar10 = ratan2/*0x8004ecd4*/(-local_34,local_30);
  iVar8 = -0x40;
  if ((-0x41 < iVar10) && (iVar8 = 0x40, iVar10 < 0x41)) {
    iVar8 = iVar10;
  }
  iVar3 = ratan2/*0x8004ecd4*/(local_38,local_30);
  iVar10 = -0x40;
  if ((iVar3 < -0x40) || (iVar10 = iVar3, iVar12 = 0x40, iVar3 < 0x41)) {
    iVar12 = iVar10;
  }
  FUN_800439b8/*0x800439b8*/(param_1 + 4,iVar8,iVar12,0);
  if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 0x1f) == 0) {
    MatrixNormal/*0x8004c934*/(param_1 + 4,param_1 + 4);
  }
  iVar8 = (short)param_1[5] * 0x3b9a;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x22];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  iVar8 = *(short *)((int)param_1 + 0x1a) * 0x3b9a;
  param_1[0x22] = param_1[0x22] + iVar10;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x23];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  iVar8 = (short)param_1[8] * 0x3b9a;
  param_1[0x23] = param_1[0x23] + iVar10;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x24];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  param_1[0x24] = param_1[0x24] + iVar10;
  if ((short)param_1[0x25] < 0x259) {
    if (0x3c < (short)param_1[0x25]) {
      if (local_30 < 0) {
        local_30 = -local_30;
      }
      if (local_38 < 0) {
        local_38 = -local_38;
      }
      if (local_30 < local_38) {
        local_30 = local_38;
      }
      if (local_30 < 0xfa000) goto LAB_8010160c;
    }
  }
  else {
LAB_8010160c:
    *(undefined1 *)(param_1 + 2) = 1;
  }
  local_38 = (*(int *)(uVar7 + 4) + *(int *)(uVar7 + 0x10)) / 2;
  local_34 = (*(int *)(uVar7 + 8) + *(int *)(uVar7 + 0x14)) / 2;
  local_30 = *(undefined4 *)(uVar7 + 0x18);
  GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,&local_38,&local_38);
  iVar8 = Terrain_HeightAt/*0x80025400*/(local_38,local_30);
  if (local_34 < iVar8) {
    return 0;
  }
switchD_80101130_caseD_3:
  uVar7 = param_1[0x17];
  local_28 = (*(int *)(uVar7 + 4) + *(int *)(uVar7 + 0x10)) / 2;
  local_24 = (*(int *)(uVar7 + 8) + *(int *)(uVar7 + 0x14)) / 2;
  local_20 = *(undefined4 *)(uVar7 + 0x18);
  GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,&local_28,&local_28);
  local_24 = Terrain_HeightAt/*0x80025400*/(local_28,local_20);
  puVar4 = (uint *)FUN_8003fdcc/*0x8003fdcc*/(&local_28,0x29,300);
  *puVar4 = *puVar4 | 0x10;
  FUN_8003fd24/*0x8003fd24*/(&local_28,0);
  uVar5 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x38,&local_28);
  *param_1 = *param_1 | 0x22;
  FUN_80020890/*0x80020890*/(param_1,0xf);
  FUN_80020778/*0x80020778*/(param_1);
  puVar6 = (undefined4 *)FUN_8001d470/*0x8001d470*/(0x1484);
  puVar6[0x19] = FUN_80100ca0;
  puVar6[0x12] = local_28;
  puVar6[0x13] = local_24;
  puVar6[0x14] = local_20;
  *puVar6 = 0xa0;
  FUN_8002036c/*0x8002036c*/();
  puVar4 = (uint *)0x1;
  goto switchD_80101130_caseD_2;
}

