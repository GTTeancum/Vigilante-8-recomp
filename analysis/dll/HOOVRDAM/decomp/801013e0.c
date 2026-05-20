// addr: 0x801013e0  name: FUN_801013e0

undefined4 FUN_801013e0(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  uVar2 = 0;
  if (iVar1 != 0) {
    iVar1 = FUN_8001fd9c/*0x8001fd9c*/(*(short *)(param_1 + 6) + 1000);
    if (iVar1 != 0) {
      FUN_80024718/*0x80024718*/(iVar1 + 0xc,0);
    }
    uVar2 = 0xffffffff;
  }
  return uVar2;
}

