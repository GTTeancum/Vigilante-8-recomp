// addr: 0x80104a94  name: FUN_80104a94

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80104a94(undefined4 param_1,int param_2)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  int iVar7;
  undefined4 local_res0 [4];
  undefined4 local_220;
  short local_21c;
  undefined2 uStack_21a;
  undefined1 auStack_218 [512];
  
  iVar2 = *(int *)(_DAT_80065bd4 + 0x24);
  iVar7 = _DAT_80065bd4;
  while (iVar2 != 0) {
    iVar2 = *(int *)(iVar7 + 0x58);
    iVar7 = iVar7 + 0x34;
  }
  local_res0[0] = param_1;
  uVar3 = func_0x800224ec(local_res0);
  *(undefined4 *)(iVar7 + 0x24) = uVar3;
  uVar3 = func_0x800224ec(local_res0);
  *(undefined4 *)(iVar7 + 0x28) = uVar3;
  uVar3 = func_0x800224b4(local_res0);
  *(undefined4 *)(iVar7 + 0x1c) = uVar3;
  uVar1 = func_0x800224b4(local_res0);
  iVar2 = *(int *)(iVar7 + 0x24);
  *(undefined2 *)(iVar7 + 0x2c) = uVar1;
  *(undefined4 *)(iVar7 + 0x14) = 0;
  *(undefined4 *)(iVar7 + 0xc) = 0;
  *(undefined4 *)(iVar7 + 0x10) = 0;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xff;
  }
  iVar4 = *(int *)(iVar7 + 0x28);
  if (iVar4 < 0) {
    iVar4 = iVar4 + 0xff;
  }
  iVar2 = (iVar2 >> 8) * (iVar4 >> 8);
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0x7f;
  }
  *(short *)(iVar7 + 0x2e) = (short)(iVar2 >> 7);
  if (*(int *)(iVar7 + 0x1c) < 0x10) {
    *(undefined4 *)(iVar7 + 0x1c) = 0x20;
  }
  if (0xc < param_2) {
    iVar2 = FUN_800187e4/*0x800187e4*/(local_res0[0],iVar7);
    FUN_8001859c/*0x8001859c*/(*(undefined2 *)(iVar7 + 10));
    iVar4 = (int)*(short *)(*(int *)(iVar2 + 4) + 4);
    puVar5 = (undefined4 *)FUN_80018124/*0x80018124*/(iVar4,0x11,0x10,1,iVar4,1);
    uVar3 = *puVar5;
    local_220._0_2_ = (short)uVar3;
    iVar4 = (int)(short)local_220;
    local_220._2_2_ = (short)((uint)uVar3 >> 0x10);
    iVar6 = (int)local_220._2_2_;
    local_21c = (short)puVar5[1];
    _local_21c = CONCAT22(1,local_21c);
    local_220 = uVar3;
    uVar1 = GetClut/*0x80052254*/(iVar4,iVar6);
    *(undefined2 *)(iVar7 + 10) = uVar1;
    iVar4 = 0;
    iVar7 = 0;
    SetFarColor/*0x8004d504*/(DAT_80065b2c,DAT_80065b2d,DAT_80065b2e);
    do {
      func_0x800189ac(*(undefined4 *)(iVar2 + 8),(int)local_21c,auStack_218,iVar7);
      LoadImage/*0x8004f82c*/(&local_220,auStack_218);
      iVar4 = iVar4 + 1;
      local_220 = CONCAT22(local_220._2_2_ + 1,(short)local_220);
      iVar7 = iVar7 + 0x100;
    } while (iVar4 < 0x10);
    LoadImage/*0x8004f82c*/(&local_220,auStack_218);
  }
  return;
}

