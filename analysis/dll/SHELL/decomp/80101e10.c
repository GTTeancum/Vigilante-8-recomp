// addr: 0x80101e10  name: FUN_80101e10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_80101e10(undefined4 param_1,int *param_2,int param_3,int param_4,uint param_5)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined *puVar5;
  int iVar6;
  short sVar7;
  short local_38;
  short local_36;
  ushort local_34;
  ushort local_32;
  undefined4 local_30;
  int *local_2c;
  
  iVar6 = 1;
  local_2c = param_2;
  if (param_2 == (int *)0x0) {
    param_2 = (int *)FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 0xc),1);
  }
  param_3 = param_3 - (((int)((uint)local_34 << 0x10) >> 0x10) -
                       ((int)((uint)local_34 << 0x10) >> 0x1f) >> 1);
  sVar7 = (short)param_3;
  iVar3 = (uint)local_32 + *(byte *)(*param_2 + 6) + 4;
  param_4 = param_4 - ((iVar3 * 0x10000 >> 0x10) - (iVar3 * 0x10000 >> 0x1f) >> 1);
  local_32 = (ushort)iVar3;
  local_36 = (short)param_4;
  local_38 = sVar7;
  if (local_2c == (int *)0x0) {
    FUN_800190a8/*0x800190a8*/(param_2,iVar3,param_2);
  }
  if ((param_5 & 8) == 0) {
    local_30 = func_0x8001a24c(&local_38);
  }
  if (local_2c == (int *)0x0) {
    param_2 = (int *)FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 0xc),1);
  }
  uVar1 = local_32;
  local_34 = (short)local_34 / 2;
  local_32 = 0;
  local_36 = local_36 + -8 + (uVar1 - *(byte *)(*param_2 + 6));
  do {
    iVar3 = 0;
    puVar5 = &DAT_8010040c;
    do {
      local_38 = sVar7 + local_34 * (short)iVar3;
      param_2[1] = param_2[1] & 0xff000000U | 0x404040;
      FUN_80019a58/*0x80019a58*/(param_2,puVar5,&local_38,2);
      iVar2 = FUN_80052544/*0x80052544*/(puVar5);
      puVar5 = puVar5 + iVar2 + 1;
      iVar3 = iVar3 + 1;
    } while (iVar3 < 2);
    do {
      Tick_PadOnly/*0x800126f0*/();
      uVar4 = _DAT_80065930 | _DAT_80065934;
    } while ((uVar4 & 0xa8500000) == 0);
    FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0);
    if ((iVar6 != 0) && ((int)uVar4 < 0)) {
      iVar6 = iVar6 + -1;
    }
    if ((iVar6 < 1) && ((uVar4 & 0x20000000) != 0)) {
      iVar6 = iVar6 + 1;
    }
  } while ((uVar4 & 0x8500000) == 0);
  _DAT_80065934 = 0;
  _DAT_80065930 = 0;
  if (local_2c == (int *)0x0) {
    FUN_800190a8/*0x800190a8*/(param_2);
  }
  if ((param_5 & 8) == 0) {
    func_0x8001a2ac(local_30,param_3,param_4);
    FUN_800183ec/*0x800183ec*/(local_30);
  }
  return iVar6;
}

