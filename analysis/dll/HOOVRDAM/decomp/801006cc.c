// addr: 0x801006cc  name: FUN_801006cc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801006cc(uint *param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  undefined2 local_20;
  int local_1c;
  int local_18;
  int local_14;
  
  if ((param_2 == 0) || (param_2 != 2)) {
    uVar3 = param_1[0x20];
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    uVar5 = param_1[0x21];
    param_1[9] = param_1[9] + ((int)uVar3 >> 7);
    if ((int)uVar5 < 0) {
      uVar5 = uVar5 + 0x7f;
    }
    uVar3 = param_1[0x22];
    param_1[10] = param_1[10] + ((int)uVar5 >> 7);
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    param_1[0xb] = param_1[0xb] + ((int)uVar3 >> 7);
    if (param_3 == 0) {
      return 0;
    }
    param_2 = FUN_800446dc/*0x800446dc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/(*(undefined1 *)((int)param_1 + 0xd3));
  }
  uVar3 = param_1[2];
  *(char *)(param_1 + 2) = (char)uVar3 + '\x01';
  iVar4 = (int)(char)uVar3;
  if (iVar4 != 1) {
    if ((1 < iVar4) || (iVar4 != 0)) {
      if (iVar4 == param_2) goto LAB_801009e0;
      if (iVar4 != 8) {
        return 0;
      }
    }
    iVar4 = 7;
    uVar1 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)param_1 + 0xd2));
    if ((char)param_1[2] == '\x01') {
      iVar4 = 8;
    }
    *(undefined1 *)(param_1 + 2) = 1;
    uVar2 = FUN_8001b038/*0x8001b038*/(uVar1,0x8000);
    FUN_8001d68c/*0x8001d68c*/(&local_30,uVar1,uVar2);
    param_1[4] = local_30;
    param_1[5] = local_2c;
    param_1[6] = local_28;
    param_1[7] = local_24;
    *(undefined2 *)(param_1 + 8) = local_20;
    uVar3 = param_1[0xe];
    *param_1 = *param_1 | 2;
    for (; uVar3 != 0; uVar3 = *(uint *)(uVar3 + 0x34)) {
      *(undefined2 *)(*(int *)(uVar3 + 0x30) + 0x28) = 0;
    }
    if (param_1[0x1a] != 0) {
      *(undefined2 *)(param_1[0x1a] + 0x28) = 0;
    }
    param_1[0x20] = (int)((local_1c - param_1[9]) * 0x80) >> iVar4;
    param_1[0x21] = (int)((local_18 - param_1[10]) * 0x80) >> iVar4;
    param_1[0x22] = (int)((local_14 - param_1[0xb]) * 0x80) >> iVar4;
    param_1[0x24] = 0;
    param_1[0x25] = 0;
    param_1[0x26] = 0;
    FUN_80020890/*0x80020890*/(param_1,1 << iVar4);
  }
  FUN_800447e8/*0x800447e8*/(*(undefined1 *)((int)param_1 + 0xd3),*(undefined4 *)(_DAT_800737e8 + 8),5,
                  param_1 + 9);
  *(undefined1 *)((int)param_1 + 0xd3) = 0;
  iVar4 = (short)param_1[5] * 0x23c3;
  *param_1 = *param_1 & 0xfefffffd;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x20] = iVar4 >> 5;
  iVar4 = *(short *)((int)param_1 + 0x1a) * 0x23c3;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x21] = iVar4 >> 5;
  iVar4 = (short)param_1[8] * 0x23c3;
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0x1f;
  }
  param_1[0x22] = iVar4 >> 5;
  FUN_80020890/*0x80020890*/(param_1,0x1e);
LAB_801009e0:
  *(undefined1 *)(param_1 + 2) = 0;
  *param_1 = *param_1 & 0xffffffdf;
  func_0x80031294();
  return 0;
}

