// addr: 0x80100244  name: FUN_80100244

/* WARNING: Removing unreachable block (ram,0x80100704) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100244(uint *param_1,uint param_2,uint *param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  uint extraout_v1;
  uint uVar7;
  uint *puVar8;
  longlong lVar9;
  uint local_48;
  int local_44;
  uint local_40;
  short local_38;
  short local_36;
  short local_34;
  undefined4 local_28;
  int local_24;
  int local_20;
  
  uVar7 = 2;
  if (param_2 == 2) goto LAB_801008f0;
  uVar1 = 3;
  if (((param_2 < 3) && (uVar1 = 0, param_2 == 0)) || (param_2 != uVar1)) {
    local_48 = param_1[9];
    local_44 = param_1[10] + param_1[0x15];
    local_40 = param_1[0xb];
    iVar2 = FUN_8001d748/*0x8001d748*/(param_1,&local_48,&local_38,0);
    if (iVar2 < local_44 + 0x800) {
      uVar7 = (uint)local_38;
      uVar1 = param_1[0x20];
      iVar3 = uVar1 * uVar7 + param_1[0x21] * (int)local_36 + param_1[0x22] * (int)local_34;
      if (iVar3 < 0) {
        iVar3 = iVar3 + 0x7ff;
      }
      iVar3 = iVar3 >> 0xb;
      uVar6 = uVar7 << 1;
      if (iVar3 < 0) {
        iVar4 = iVar3 * uVar7;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x20] = uVar1 - (iVar4 >> 0xc);
        iVar4 = iVar3 * local_36;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        param_1[0x21] = param_1[0x21] - (iVar4 >> 0xc);
        iVar4 = iVar3 * local_34;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        uVar6 = (int)param_1[0x21] / 2;
        uVar7 = iVar2 - param_1[0x15];
        param_1[0x22] = param_1[0x22] - (iVar4 >> 0xc);
        param_1[0x21] = uVar6;
        param_1[10] = uVar7;
      }
      iVar2 = (uVar6 + uVar7) * 0x1e;
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      param_1[0x20] = uVar1 + (iVar2 >> 0xc);
      iVar2 = local_34 * 0x5a;
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      param_1[0x22] = param_1[0x22] + (iVar2 >> 0xc);
      uVar7 = -param_1[0x22] * (uint)(ushort)param_1[0x25];
      if ((int)uVar7 < 0) {
        uVar7 = uVar7 + 0xfff;
      }
      iVar2 = param_1[0x20] * (uint)(ushort)param_1[0x25];
      *(short *)(param_1 + 0x23) = (short)((int)uVar7 >> 0xc);
      if (iVar2 < 0) {
        iVar2 = iVar2 + 0xfff;
      }
      *(short *)(param_1 + 0x24) = (short)(iVar2 >> 0xc);
      uVar1 = (uint)(iVar3 < -0x1c9);
      if (local_36 < -0xe66) {
        uVar7 = param_1[0x21];
        uVar1 = param_1[0x20];
        if ((int)uVar7 < 0) {
          uVar7 = -uVar7;
        }
        if ((int)uVar1 < 0) {
          uVar1 = -uVar1;
        }
        if ((int)uVar7 < (int)uVar1) {
          uVar7 = uVar1;
        }
        uVar6 = param_1[0x22];
        if ((int)uVar6 < 0) {
          uVar6 = -uVar6;
        }
        if ((int)uVar6 < (int)uVar7) {
          uVar6 = uVar7;
        }
        uVar1 = (uint)(iVar3 < -0x1c9);
        if ((int)uVar6 < 0x42c) {
          uVar1 = FUN_80020778/*0x80020778*/(param_1);
          uVar7 = 1;
        }
      }
      if (uVar1 != 0) {
        uVar5 = FUN_8004410c/*0x8004410c*/(uVar7);
        FUN_8004483c/*0x8004483c*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),0,param_1 + 9);
        goto LAB_8010054c;
      }
    }
    else {
LAB_8010054c:
      param_1[0x21] = param_1[0x21] + 0x5a;
    }
    param_3 = param_1 + 4;
    func_0x800172b4(param_3,param_3,param_1 + 0x23);
    uVar7 = param_1[0x20];
    uVar1 = _DAT_80065310 - (uint)*(byte *)((int)param_1 + 9);
    param_1[9] = param_1[9] + uVar7;
    param_1[10] = param_1[10] + param_1[0x21];
    param_1[0xb] = param_1[0xb] + param_1[0x22];
    if ((uVar1 & 0xf) != 0) {
      return 0;
    }
    MatrixNormal/*0x8004c934*/(param_3,param_3);
  }
  uVar6 = *param_3;
  uVar1 = (uint)*(byte *)(uVar6 + 4);
  if (uVar1 == 7) {
    iVar2 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(uVar6 + 0xc));
    if (iVar2 == 0) {
      return 0;
    }
    uVar1 = param_1[0x15];
    if (uVar1 == 0) goto LAB_801008f0;
    uVar6 = param_1[0x1c];
    if ((int)uVar1 < 0) {
      uVar1 = uVar1 + 0xf;
    }
    *(int *)(uVar6 + 0x28) = (int)uVar1 >> 4;
    *(int *)(uVar6 + 0x24) = (int)uVar1 >> 4;
    iVar2 = param_1[0x15] * 0x93c;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0xfff;
    }
    param_1[0x15] = iVar2 >> 0xc;
    iVar2 = (iVar2 >> 0xc) * 0x3243;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0xfff;
    }
    *(short *)(param_1 + 0x25) = (short)(0x1000000 / (iVar2 >> 0xc));
    if ((*param_1 & 0x80) != 0) {
      return 0;
    }
    FUN_80020744/*0x80020744*/(param_1);
    uVar6 = 1;
    uVar1 = extraout_v1;
  }
  if ((*param_1 & 0x80) == 0) {
    return 0;
  }
  puVar8 = param_3;
  if (uVar1 == uVar7) {
    puVar8 = (uint *)(0x10000 / *(ushort *)(uVar6 + 0xa2));
    lVar9 = FUN_800171d4/*0x800171d4*/(param_1 + 0x20,uVar6 + 0x80);
    if (0 < lVar9) {
      return 0;
    }
    if (((int)(param_1[0x20] * (int)puVar8) < -0x100000) ||
       (local_28 = 0x100000, (int)(param_1[0x20] * (int)puVar8) < 0x100001)) {
      local_28 = 0xfff00000;
    }
    iVar2 = param_1[0x21] * (int)puVar8;
    local_24 = -0x100000;
    if ((-0x100001 < iVar2) && (local_24 = 0x100000, iVar2 < 0x100001)) {
      local_24 = iVar2;
    }
    iVar2 = param_1[0x22] * (int)puVar8;
    iVar3 = -0x100000;
    if ((iVar2 < -0x100000) || (iVar3 = iVar2, local_20 = 0x100000, iVar2 < 0x100001)) {
      local_20 = iVar3;
    }
    FUN_800176f8/*0x800176f8*/(uVar6,&local_28,param_1 + 9);
    FUN_8002c958/*0x8002c958*/(*param_3,0xffffff9c,&DAT_80100044,1);
  }
  FUN_8001f5a0/*0x8001f5a0*/(param_1,puVar8);
  iVar2 = param_1[0x20] * (int)(short)puVar8[8] +
          param_1[0x21] * (int)*(short *)((int)puVar8 + 0x22) +
          param_1[0x22] * (int)(short)puVar8[9];
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7ff;
  }
  iVar2 = iVar2 >> 0xb;
  if (-1 < iVar2) {
    return 0;
  }
  iVar3 = iVar2 * (short)puVar8[8];
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  param_1[0x20] = param_1[0x20] - (iVar3 >> 0xc);
  iVar3 = iVar2 * *(short *)((int)puVar8 + 0x22);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 0xfff;
  }
  param_1[0x21] = param_1[0x21] - (iVar3 >> 0xc);
  iVar2 = iVar2 * (short)puVar8[9];
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  param_1[0x22] = param_1[0x22] - (iVar2 >> 0xc);
LAB_801008f0:
  FUN_8001fe8c/*0x8001fe8c*/(&DAT_801012a0,param_1);
  FUN_800205f8/*0x800205f8*/(param_1);
  _DAT_000012ad = DAT_801012ac + -1;
  return 0;
}

