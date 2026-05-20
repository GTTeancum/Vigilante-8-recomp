// addr: 0x8010266c  name: FUN_8010266c

undefined4 FUN_8010266c(uint *param_1,undefined4 param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint *puVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  uint uVar9;
  undefined8 uVar10;
  int local_38;
  int local_34;
  int local_30;
  uint local_28;
  uint local_24;
  uint local_20;
  
  switch(param_2) {
  case 0:
    puVar6 = param_1;
    if ((*param_1 & 0x10000) != 0) {
      FUN_8001787c/*0x8001787c*/(param_1,param_1[0x17] + 4);
      puVar6 = (uint *)0x1;
    }
    iVar1 = FUN_8001d748/*0x8001d748*/(puVar6,param_1 + 9,0,0);
    iVar1 = iVar1 - param_1[10];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xff;
    }
    iVar1 = iVar1 >> 8;
    uVar9 = param_1[0x24];
    if (0 < *(short *)((int)param_1 + 0x1a)) {
      uVar9 = uVar9 + 0x200;
    }
    param_1[0x24] = uVar9 - 0x200;
    uVar9 = param_1[0x26] - 0x200;
    if (*(short *)((int)param_1 + 0x16) < 0) {
      uVar9 = param_1[0x26] + 0x200;
    }
    iVar4 = iVar1;
    if (iVar1 < 0) {
      iVar4 = -iVar1;
    }
    param_1[0x26] = uVar9;
    iVar2 = 0x800;
    if (0x800 < iVar1 * iVar4) {
      iVar2 = iVar1 * iVar4;
    }
    param_1[0x21] = (param_1[0x21] + 0x1c00) - 0x11800000 / iVar2;
    FUN_80017324/*0x80017324*/(param_1);
    param_1[0x24] = (int)(param_1[0x24] * 0xf80) >> 0xc;
    param_1[0x25] = (int)(param_1[0x25] * 0xf80) >> 0xc;
    param_1[0x26] = (int)(param_1[0x26] * 0xf80) >> 0xc;
    uVar3 = param_1[0x20];
    uVar9 = uVar3;
    if ((int)uVar3 < 0) {
      uVar9 = uVar3 + 0x3f;
    }
    uVar5 = param_1[0x21];
    param_1[0x20] = uVar3 - ((int)uVar9 >> 6);
    uVar9 = uVar5;
    if ((int)uVar5 < 0) {
      uVar9 = uVar5 + 0x3f;
    }
    uVar3 = param_1[0x22];
    param_1[0x21] = uVar5 - ((int)uVar9 >> 6);
    uVar9 = uVar3;
    if ((int)uVar3 < 0) {
      uVar9 = uVar3 + 0x3f;
    }
    param_1[0x22] = uVar3 - ((int)uVar9 >> 6);
    break;
  case 1:
    goto switchD_801026b0_caseD_1;
  default:
    goto switchD_801026b0_caseD_2;
  case 3:
    break;
  case 6:
    goto switchD_801026b0_caseD_6;
  case 8:
    goto switchD_801026b0_caseD_8;
  }
  iVar1 = *param_3;
  puVar6 = param_1;
  if (*(char *)(iVar1 + 4) == '\x02') {
    uVar9 = ((uint)*(ushort *)(iVar1 + 0xa2) << 8) / (uint)*(ushort *)((int)param_1 + 0xa2);
    FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
    iVar4 = *(int *)(iVar1 + 0x80) * uVar9;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xff;
    }
    local_38 = param_1[0x20] - (iVar4 >> 8);
    iVar4 = *(int *)(iVar1 + 0x84) * uVar9;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xff;
    }
    local_34 = param_1[0x21] - (iVar4 >> 8);
    iVar1 = *(int *)(iVar1 + 0x88) * uVar9;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xff;
    }
    local_30 = param_1[0x22] - (iVar1 >> 8);
    uVar10 = FUN_80017240/*0x80017240*/(&local_38,param_3 + 8);
    uVar9 = (uint)uVar10 >> 0xd | (int)((ulonglong)uVar10 >> 0x20) << 0x13;
    if (-1 < (int)uVar9) {
      return 0;
    }
    GTE_RotatePackedXYZ/*0x800434d0*/(param_1 + 4,param_3 + 8,&local_28);
    uVar9 = -(param_3[0xc] * 2 + uVar9);
    iVar1 = (int)uVar9 >> 0x1f;
    local_28 = (uint)((ulonglong)local_28 * (ulonglong)uVar9) >> 0xc |
               ((int)((ulonglong)local_28 * (ulonglong)uVar9 >> 0x20) + local_28 * iVar1 +
               uVar9 * ((int)local_28 >> 0x1f)) * 0x100000;
    local_24 = (uint)((ulonglong)local_24 * (ulonglong)uVar9) >> 0xc |
               ((int)((ulonglong)local_24 * (ulonglong)uVar9 >> 0x20) + local_24 * iVar1 +
               uVar9 * ((int)local_24 >> 0x1f)) * 0x100000;
    local_20 = (uint)((ulonglong)local_20 * (ulonglong)uVar9) >> 0xc |
               ((int)((ulonglong)local_20 * (ulonglong)uVar9 >> 0x20) + local_20 * iVar1 +
               uVar9 * ((int)local_20 >> 0x1f)) * 0x100000;
    FUN_80017594/*0x80017594*/(param_1,&local_28,param_3 + 5);
    puVar6 = (uint *)0x1;
  }
  uVar7 = FUN_8001f5a0/*0x8001f5a0*/(puVar6,param_3);
  FUN_80017ba8/*0x80017ba8*/(param_1,uVar7);
  if (*(char *)(*param_3 + 4) == '\a') {
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
switchD_801026b0_caseD_8:
    if ((*param_1 & 0x10000) == 0) {
      iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
      uVar9 = 0;
      if (iVar1 != 0) {
        puVar8 = &DAT_80102bbc;
        *param_1 = *param_1 | 0x10000;
        do {
          uVar9 = uVar9 + 1;
          iVar1 = FUN_80017160/*0x80017160*/();
          local_28 = (iVar1 * 0xbeb >> 0xf) - 0x5f5;
          local_24 = 0xffffee1f;
          iVar1 = FUN_80017160/*0x80017160*/();
          local_20 = (iVar1 * 0xbeb >> 0xf) - 0x5f5;
          uVar7 = *puVar8;
          puVar8 = puVar8 + 1;
          FUN_8003cee0/*0x8003cee0*/(uVar7,param_1 + 0x12,&local_28);
        } while (uVar9 < 4);
switchD_801026b0_caseD_6:
switchD_801026b0_caseD_1:
        *param_1 = *param_1 | 0x88;
        FUN_8003e76c/*0x8003e76c*/(param_1);
        *(undefined2 *)(param_1 + 0x27) = 0x40;
        *(undefined2 *)((int)param_1 + 0x9e) = 0x40;
        *(undefined2 *)(param_1 + 0x28) = 0x40;
        *(undefined2 *)((int)param_1 + 0xa2) = 0x1000;
      }
switchD_801026b0_caseD_2:
    }
  }
  return 0;
}

