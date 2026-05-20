// addr: 0x80030c08  name: FUN_80030c08

void FUN_80030c08(uint *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;
  
  iVar1 = FUN_8001d748(param_1,param_1 + 9,0,0);
  iVar1 = iVar1 - param_1[10];
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0xff;
  }
  iVar1 = iVar1 >> 8;
  iVar2 = FUN_80016a20(param_1 + 0x20);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  param_1[0x23] = iVar2 >> 7;
  param_1[0x25] = (int)(short)param_1[0x29] << 6;
  uVar4 = param_1[0x24] + 0x200;
  if (*(short *)((int)param_1 + 0x1a) < 1) {
    uVar4 = param_1[0x24] - 0x200;
  }
  param_1[0x24] = uVar4;
  uVar4 = param_1[0x26] - 0x200;
  if (*(short *)((int)param_1 + 0x16) < 0) {
    uVar4 = param_1[0x26] + 0x200;
  }
  param_1[0x26] = uVar4;
  iVar2 = (int)(short)param_1[5] * (int)*(short *)((int)param_1 + 0xa6);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xf;
  }
  iVar3 = iVar1;
  if (iVar1 < 0) {
    iVar3 = -iVar1;
  }
  param_1[0x20] = param_1[0x20] + (iVar2 >> 4);
  iVar2 = 0x800;
  if (0x800 < iVar1 * iVar3) {
    iVar2 = iVar1 * iVar3;
  }
  param_1[0x21] = (param_1[0x21] + 0x1c00) - (int)(param_1[0x36] * 0x1c00) / iVar2;
  iVar1 = (int)(short)param_1[8] * (int)*(short *)((int)param_1 + 0xa6);
  if (iVar1 < 0) {
    iVar1 = iVar1 + 0xf;
  }
  param_1[0x22] = param_1[0x22] + (iVar1 >> 4);
  FUN_80017324(param_1);
  param_1[0x24] = (int)(param_1[0x24] * 0xf80) >> 0xc;
  param_1[0x25] = (int)(param_1[0x25] * 0xf80) >> 0xc;
  param_1[0x26] = (int)(param_1[0x26] * 0xf80) >> 0xc;
  uVar5 = param_1[0x20];
  uVar4 = uVar5;
  if ((int)uVar5 < 0) {
    uVar4 = uVar5 + 0x3f;
  }
  uVar7 = param_1[0x21];
  param_1[0x20] = uVar5 - ((int)uVar4 >> 6);
  uVar4 = uVar7;
  if ((int)uVar7 < 0) {
    uVar4 = uVar7 + 0x3f;
  }
  uVar5 = param_1[0x22];
  param_1[0x21] = uVar7 - ((int)uVar4 >> 6);
  uVar4 = uVar5;
  if ((int)uVar5 < 0) {
    uVar4 = uVar5 + 0x3f;
  }
  param_1[0x22] = uVar5 - ((int)uVar4 >> 6);
  iVar1 = 0;
  puVar6 = param_1;
  do {
    uVar4 = puVar6[0x44];
    if ((uVar4 != 0) && (*(short *)(uVar4 + 6) != 0)) {
      *(short *)(uVar4 + 6) = *(short *)(uVar4 + 6) + -1;
    }
    iVar1 = iVar1 + 1;
    puVar6 = puVar6 + 1;
  } while (iVar1 < 3);
  iVar1 = 0;
  puVar6 = param_1;
  do {
    if ((short)puVar6[0x47] != 0) {
      *(short *)(puVar6 + 0x47) = (short)puVar6[0x47] + -1;
    }
    iVar1 = iVar1 + 1;
    puVar6 = (uint *)((int)puVar6 + 2);
  } while (iVar1 < 3);
  if ((*param_1 & 0x800000) == 0) {
    if ((short)param_1[0x48] == 0) {
      param_1[0x12] = param_1[9];
      param_1[0x13] = param_1[10];
      param_1[0x14] = param_1[0xb];
    }
    else {
      iVar1 = param_1[9] - param_1[0x12];
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x1f;
      }
      param_1[0x12] = param_1[0x12] + (iVar1 >> 5);
      iVar1 = param_1[10] - param_1[0x13];
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x1f;
      }
      param_1[0x13] = param_1[0x13] + (iVar1 >> 5);
      iVar1 = param_1[0xb] - param_1[0x14];
      if (iVar1 < 0) {
        iVar1 = iVar1 + 0x1f;
      }
      param_1[0x14] = param_1[0x14] + (iVar1 >> 5);
    }
  }
  return;
}

