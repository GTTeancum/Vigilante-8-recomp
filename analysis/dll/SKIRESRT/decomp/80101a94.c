// addr: 0x80101a94  name: FUN_80101a94

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101a94(uint *param_1,int param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int extraout_v1;
  uint uVar5;
  int iVar6;
  uint *puVar7;
  uint local_40;
  int local_3c;
  uint local_38;
  short local_30;
  short local_2e;
  short local_2c;
  int local_20;
  int local_1c;
  int local_18;
  
  if ((param_2 != 0) && (param_2 == 3)) goto LAB_80101dc8;
  local_40 = param_1[9];
  local_3c = param_1[10] + param_1[0x15];
  local_38 = param_1[0xb];
  iVar1 = FUN_8001d748/*0x8001d748*/(param_1,&local_40,&local_30,0);
  if (iVar1 < local_3c + 0x800) {
    iVar6 = (int)local_30;
    puVar7 = (uint *)param_1[0x20];
    iVar2 = (int)puVar7 * iVar6 + param_1[0x21] * (int)local_2e + param_1[0x22] * (int)local_2c;
    if (iVar2 < 0) {
      iVar2 = iVar2 + 0x7ff;
    }
    iVar2 = iVar2 >> 0xb;
    iVar4 = iVar6 << 1;
    if (iVar2 < 0) {
      iVar6 = iVar2 * iVar6;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x20] = (int)puVar7 - (iVar6 >> 0xc);
      iVar6 = iVar2 * local_2e;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      param_1[0x21] = param_1[0x21] - (iVar6 >> 0xc);
      iVar6 = iVar2 * local_2c;
      if (iVar6 < 0) {
        iVar6 = iVar6 + 0xfff;
      }
      uVar5 = param_1[0x21];
      param_1[0x22] = param_1[0x22] - (iVar6 >> 0xc);
      if ((int)uVar5 < 0) {
        uVar5 = uVar5 + 3;
      }
      param_1[0x21] = (int)uVar5 >> 2;
      param_1[10] = iVar1 - param_1[0x15];
      if (iVar2 < 0x1c9) {
        FUN_8003fd24/*0x8003fd24*/(&local_40,0x11);
        uVar3 = FUN_8004410c/*0x8004410c*/();
        iVar6 = *(int *)(param_1[0x16] + 8);
        puVar7 = param_1 + 9;
        FUN_8004483c/*0x8004483c*/(uVar3,iVar6,4);
        iVar4 = extraout_v1;
        goto LAB_80101c58;
      }
    }
    else {
LAB_80101c58:
      iVar1 = (iVar4 + iVar6) * 0x1e;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      param_1[0x20] = (int)puVar7 + (iVar1 >> 0xc);
      iVar1 = local_2c * 0x5a;
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0xfff;
      }
      param_1[0x22] = param_1[0x22] + (iVar1 >> 0xc);
    }
    iVar1 = -param_1[0x22] * (uint)(ushort)param_1[0x25];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0xfff;
    }
    iVar6 = param_1[0x20] * (uint)(ushort)param_1[0x25];
    *(short *)(param_1 + 0x23) = (short)(iVar1 >> 0xc);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    *(short *)(param_1 + 0x24) = (short)(iVar6 >> 0xc);
    if ((int)param_1[0xb] < 0x3b60000) {
      FUN_8003fd24/*0x8003fd24*/(param_1 + 9,0x11);
      FUN_800205f8/*0x800205f8*/(param_1);
      _DAT_00000095 = *(int *)(_DAT_800659fc + 0x94) + -1;
      goto LAB_80101d48;
    }
  }
  else {
LAB_80101d48:
    param_1[0x21] = param_1[0x21] + 0x5a;
  }
  puVar7 = param_1 + 4;
  func_0x800172b4(puVar7,puVar7,param_1 + 0x23);
  param_3 = (int *)param_1[0x20];
  uVar5 = _DAT_80065310 - (uint)*(byte *)((int)param_1 + 9);
  param_1[9] = param_1[9] + (int)param_3;
  param_1[10] = param_1[10] + param_1[0x21];
  param_1[0xb] = param_1[0xb] + param_1[0x22];
  if ((uVar5 & 0xf) != 0) {
    return 0;
  }
  MatrixNormal/*0x8004c934*/(puVar7,puVar7);
LAB_80101dc8:
  iVar1 = *param_3;
  if (*(char *)(iVar1 + 4) == '\x02') {
    uVar5 = 0x32000 / *(ushort *)(iVar1 + 0xa2);
    iVar6 = 0x100000;
    if ((int)(param_1[0x20] * uVar5) < 0x100000) {
      iVar6 = param_1[0x20] * uVar5;
    }
    local_20 = -0x100000;
    if (-0x100000 < iVar6) {
      local_20 = iVar6;
    }
    iVar6 = 0x100000;
    if ((int)(param_1[0x21] * uVar5) < 0x100000) {
      iVar6 = param_1[0x21] * uVar5;
    }
    local_1c = -0x100000;
    if (-0x100000 < iVar6) {
      local_1c = iVar6;
    }
    iVar6 = 0x100000;
    if ((int)(param_1[0x22] * uVar5) < 0x100000) {
      iVar6 = param_1[0x22] * uVar5;
    }
    local_18 = -0x100000;
    if (-0x100000 < iVar6) {
      local_18 = iVar6;
    }
    FUN_800176f8/*0x800176f8*/(iVar1,&local_20,param_1 + 9);
    if ((*param_1 & 0x10000) == 0) {
      *param_1 = *param_1 | 0x10000;
      FUN_8002c958/*0x8002c958*/(iVar1,0xffffff06,&DAT_80100148,1);
    }
  }
  return 0;
}

