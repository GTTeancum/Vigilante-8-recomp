// addr: 0x80102094  name: FUN_80102094

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_80102094(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  iVar2 = 0;
  if ((iVar1 != 0) && (iVar2 = FUN_8003fc94/*0x8003fc94*/(param_1), iVar2 == 0)) {
    DAT_80102154 = DAT_80102154 + 1;
    iVar2 = 0;
    if ((DAT_80102154 & 1) != 0) {
      uVar3 = 0x200;
      if ((uint)(_DAT_80065b34 + _DAT_80065b38) >> 1 < *(uint *)(param_1 + 0x48)) {
        uVar3 = 0x201;
      }
      iVar2 = FUN_80021924/*0x80021924*/(9,uVar3);
    }
  }
  return iVar2;
}

