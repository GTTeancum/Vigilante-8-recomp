// addr: 0x8010068c  name: FUN_8010068c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010068c(uint *param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  uint *puVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  
  if ((param_2 != 0) && (param_2 == 2)) goto LAB_80100a9c;
  if ((char)param_1[2] == '\0') {
LAB_8010094c:
    iVar11 = -(int)(short)param_1[0x11] >> 4;
    iVar10 = -0x10;
    if ((-0x11 < iVar11) && (iVar10 = 0x10, iVar11 < 0x11)) {
      iVar10 = iVar11;
    }
    iVar10 = (uint)(ushort)param_1[0x11] + iVar10;
    *(short *)(param_1 + 0x11) = (short)iVar10;
    *(short *)((int)param_1 + 0x42) =
         *(short *)((int)param_1 + 0x42) + (short)(iVar10 * 0x10000 >> 0x16);
    param_1[0x13] = param_1[0x13] - 0xbeb;
  }
  else {
    uVar9 = param_1[0x29];
    iVar10 = *(int *)(uVar9 + 0x48) - param_1[0x12];
    iVar11 = *(int *)(uVar9 + 0x50) - param_1[0x14];
    iVar3 = ratan2/*0x8004ecd4*/(iVar10,iVar11);
    iVar3 = (int)((iVar3 - (uint)*(ushort *)((int)param_1 + 0x42)) * 0x100000) >> 0x10;
    iVar4 = -0x200;
    if ((-0x201 < iVar3) && (iVar4 = 0x200, iVar3 < 0x201)) {
      iVar4 = iVar3;
    }
    iVar6 = iVar4 - (short)param_1[0x11] >> 4;
    iVar4 = -0x10;
    if ((-0x11 < iVar6) && (iVar4 = 0x10, iVar6 < 0x11)) {
      iVar4 = iVar6;
    }
    iVar4 = (uint)(ushort)param_1[0x11] + iVar4;
    *(short *)(param_1 + 0x11) = (short)iVar4;
    if (iVar10 < 0) {
      iVar10 = -iVar10;
    }
    *(short *)((int)param_1 + 0x42) =
         *(short *)((int)param_1 + 0x42) + (short)(iVar4 * 0x10000 >> 0x16);
    if (iVar10 < 0x1f4000) {
      if (iVar11 < 0) {
        iVar11 = -iVar11;
      }
      if (0x1f3fff < iVar11) goto LAB_801008e8;
      if ((_DAT_80065310 & 0x1f) == 0) {
        puVar5 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0xdb,0x98,0);
        *(undefined1 *)(puVar5 + 1) = 7;
        *puVar5 = *puVar5 | 0x80;
        uVar1 = *(undefined2 *)((int)param_1 + 6);
        puVar5[0x19] = (uint)FUN_8010059c;
        *(undefined2 *)(puVar5 + 3) = 100;
        *(undefined2 *)((int)puVar5 + 6) = uVar1;
        uVar7 = param_1[0x13];
        uVar8 = param_1[0x14];
        puVar5[0x12] = param_1[0x12];
        puVar5[0x13] = uVar7;
        puVar5[0x14] = uVar8;
        *(undefined2 *)((int)puVar5 + 0x42) = *(undefined2 *)((int)param_1 + 0x42);
        iVar10 = (short)param_1[5] * 0x1dcd;
        if (iVar10 < 0) {
          iVar10 = iVar10 + 0xfff;
        }
        puVar5[0x22] = iVar10 >> 0xc;
        iVar10 = (short)param_1[8] * 0x1dcd;
        if (iVar10 < 0) {
          iVar10 = iVar10 + 0xfff;
        }
        puVar5[0x24] = iVar10 >> 0xc;
        puVar5[0x23] = 0;
        FUN_8002036c/*0x8002036c*/();
      }
      if ((int)param_1[0x13] < *(int *)(uVar9 + 0x4c) + -0x23000) {
        param_1[0x13] = param_1[0x13] + 0x5f5;
        goto LAB_801008e8;
      }
    }
    else {
LAB_801008e8:
      if (*(int *)(uVar9 + 0x4c) + -0x64000 < (int)param_1[0x13]) {
        param_1[0x13] = param_1[0x13] - 0xbeb;
      }
    }
    if (*(short *)(uVar9 + 0xc) == 0) {
LAB_80100934:
      *(undefined1 *)(param_1 + 2) = 0;
      FUN_80020890/*0x80020890*/(param_1,600);
      goto LAB_8010094c;
    }
    if (iVar3 < 0) {
      iVar3 = -iVar3;
    }
    if (0x4000 < iVar3) goto LAB_80100934;
  }
  iVar10 = (short)param_1[5] * 0x1dcd;
  if (iVar10 < 0) {
    iVar10 = iVar10 + 0xfff;
  }
  iVar11 = (short)param_1[8] * 0x1dcd;
  param_1[0x12] = param_1[0x12] + (iVar10 >> 0xc);
  if (iVar11 < 0) {
    iVar11 = iVar11 + 0xfff;
  }
  param_1[0x14] = param_1[0x14] + (iVar11 >> 0xc);
  FUN_8001d708/*0x8001d708*/(param_1);
  if (param_3 == 0) {
    return 0;
  }
  for (uVar9 = param_1[0xe]; uVar9 != 0; uVar9 = *(uint *)(uVar9 + 0x34)) {
    if (*(short *)(uVar9 + 6) == 0) {
      *(short *)(uVar9 + 0x44) = *(short *)(uVar9 + 0x44) + (short)param_3 * 0x100;
      FUN_8001d708/*0x8001d708*/(uVar9);
    }
  }
  iVar10 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
  FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),iVar10 << 1);
LAB_80100a9c:
  if ((*param_1 & 0x80) != 0) {
    FUN_80020778/*0x80020778*/(param_1);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
    *param_1 = *param_1 | 0x22;
    *(ushort *)((int)param_1 + 0x42) = *(ushort *)((int)param_1 + 0x42) ^ 0x800;
    iVar10 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
    param_1[0x13] = iVar10 - 0x64000;
  }
  uVar9 = _DAT_80065ad4;
  iVar10 = func_0x8001fd08(*(undefined4 *)(_DAT_80065ad4 + 0x24),
                           *(undefined4 *)(_DAT_80065ad4 + 0x2c));
  uVar7 = _DAT_80065ad8;
  if ((((iVar10 != 0) && (*(short *)(iVar10 + 10) == 0)) ||
      ((_DAT_80065ad8 != 0 &&
       ((iVar10 = func_0x8001fd08(*(undefined4 *)(_DAT_80065ad8 + 0x24),
                                  *(undefined4 *)(_DAT_80065ad8 + 0x2c)), iVar10 != 0 &&
        (uVar9 = uVar7, *(short *)(iVar10 + 10) == 0)))))) &&
     (cVar2 = (char)param_1[2] + '\x01', *(char *)(param_1 + 2) = cVar2, cVar2 != '\x01')) {
    param_1[0x29] = uVar9;
    *param_1 = *param_1 & 0xffffffdd;
    cVar2 = FUN_8004410c/*0x8004410c*/();
    *(char *)((int)param_1 + 5) = cVar2;
    FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),1,0);
    FUN_80020744/*0x80020744*/(param_1);
  }
  *(undefined1 *)(param_1 + 2) = 0;
  FUN_80020890/*0x80020890*/(param_1,0x3c);
  return 0;
}

