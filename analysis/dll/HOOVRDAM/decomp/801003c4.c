// addr: 0x801003c4  name: FUN_801003c4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801003c4(uint *param_1,int param_2,int param_3)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined2 local_60;
  undefined2 local_5e;
  short local_5c;
  int local_58;
  int local_54;
  int local_50;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined2 local_38;
  
  if (param_2 != 1) {
    if ((param_2 == 0) || (param_2 != 7)) {
      if (param_3 == 0) {
        return 0;
      }
      uVar4 = _DAT_8006f6f4 + _DAT_8006f6e4 * 0x40;
      iVar7 = (param_1[9] - uVar4) + param_3 * 0x5f5;
      local_58 = iVar7 / param_3;
      uVar3 = _DAT_8006f6f8 + _DAT_8006f6ea * 0x40;
      iVar6 = (param_1[10] - uVar3) + param_3 * 0x1dcd;
      local_54 = iVar6 / param_3;
      uVar2 = _DAT_8006f6fc + _DAT_8006f6f0 * 0x40;
      iVar5 = param_1[0xb] - uVar2;
      local_50 = iVar5 / param_3;
      GTE_RotateLongMat/*0x80043358*/(0x8006f680,&local_58,&local_58);
      param_1[9] = uVar4;
      param_1[10] = uVar3;
      param_1[0xb] = uVar2;
      local_60 = ratan2/*0x8004ecd4*/(local_50,local_54);
      local_5e = 0;
      local_5c = ratan2/*0x8004ecd4*/(local_58,local_54);
      local_5c = -local_5c;
      RotMatrixYXZ_gte/*0x8004dab4*/(&local_60,&local_48);
      uVar2 = param_1[0xe];
      if (uVar2 == 0) {
        return 0;
      }
      do {
        *(undefined4 *)(uVar2 + 0x10) = local_48;
        *(undefined4 *)(uVar2 + 0x14) = local_44;
        *(undefined4 *)(uVar2 + 0x18) = local_40;
        *(undefined4 *)(uVar2 + 0x1c) = local_3c;
        *(undefined2 *)(uVar2 + 0x20) = local_38;
        *(uint *)(uVar2 + 0x24) = (*(int *)(uVar2 + 0x24) + iVar7 + 0x20000U & 0x3ffff) - 0x20000;
        *(uint *)(uVar2 + 0x28) = (*(int *)(uVar2 + 0x28) + iVar6 + 0x20000U & 0x3ffff) - 0x20000;
        *(uint *)(uVar2 + 0x2c) = (*(int *)(uVar2 + 0x2c) + iVar5 + 0x20000U & 0x3ffff) - 0x20000;
        uVar2 = *(uint *)(uVar2 + 0x34);
      } while (uVar2 != 0);
    }
    iVar5 = FUN_8001d470/*0x8001d470*/(0x80);
    *(uint **)(iVar5 + 0x58) = param_1;
    *(short *)(iVar5 + 10) = (short)param_3;
  }
  iVar5 = 0;
  if ('\x02' < DAT_80065319) {
    Heap_Free/*0x80045088*/(param_1);
  }
  *param_1 = *param_1 | 0xa0;
  do {
    puVar1 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],*(undefined2 *)((int)param_1 + 10),0x80,0);
    *puVar1 = *puVar1 | 0x410;
    iVar6 = FUN_80017160/*0x80017160*/();
    puVar1[0x12] = (iVar6 << 0x11) >> 0xd;
    iVar6 = FUN_80017160/*0x80017160*/();
    puVar1[0x13] = (iVar6 << 0x11) >> 0xd;
    iVar6 = FUN_80017160/*0x80017160*/();
    iVar5 = iVar5 + 1;
    puVar1[0x14] = (iVar6 << 0x11) >> 0xd;
    *(undefined2 *)(puVar1[0xc] + 0x28) = 0x40;
    FUN_8001d708/*0x8001d708*/(puVar1);
    FUN_8001d4f0/*0x8001d4f0*/(param_1,puVar1);
  } while (iVar5 < 0x40);
  return 0;
}

