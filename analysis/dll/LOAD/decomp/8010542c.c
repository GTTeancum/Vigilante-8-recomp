// addr: 0x8010542c  name: FUN_8010542c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010542c(undefined4 param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 local_220;
  undefined4 local_21c;
  undefined1 auStack_218 [512];
  
  _DAT_80065afc = FUN_800187e4/*0x800187e4*/(param_1,0x80065b48);
  iVar2 = (int)*(short *)(*(int *)(_DAT_80065afc + 4) + 4);
  puVar1 = (undefined4 *)FUN_80018124/*0x80018124*/(iVar2,0x11,0x10,1,iVar2,1);
  uVar4 = *puVar1;
  local_21c = puVar1[1];
  local_220._0_2_ = (short)uVar4;
  iVar2 = (int)(short)local_220;
  local_220._2_2_ = (short)((uint)uVar4 >> 0x10);
  iVar3 = (int)local_220._2_2_;
  iVar5 = 0;
  local_220 = uVar4;
  _DAT_80065b04 = GetClut/*0x80052254*/(iVar2,iVar3);
  iVar2 = 0;
  SetFarColor/*0x8004d504*/(DAT_80065b2c,DAT_80065b2d,DAT_80065b2e);
  do {
    iVar3 = iVar2 * iVar2;
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    func_0x800189ac(*(undefined4 *)(_DAT_80065afc + 8),
                    (int)*(short *)(*(int *)(_DAT_80065afc + 4) + 4),auStack_218,iVar3 >> 0xc);
    LoadImage/*0x8004f82c*/(&local_220,auStack_218);
    iVar5 = iVar5 + 1;
    local_220 = CONCAT22(local_220._2_2_ + 1,(short)local_220);
    iVar2 = iVar2 + 0x100;
  } while (iVar5 < 0x10);
  LoadImage/*0x8004f82c*/(&local_220,auStack_218);
  return;
}

