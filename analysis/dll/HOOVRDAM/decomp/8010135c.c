// addr: 0x8010135c  name: FUN_8010135c

undefined4 FUN_8010135c(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  uVar2 = 0;
  if (iVar1 != 0) {
    iVar1 = FUN_80021888/*0x80021888*/(*(short *)(param_1 + 6) + 1000);
    uVar2 = 0;
    if (iVar1 != 0) {
      uVar2 = FUN_800205f8/*0x800205f8*/(iVar1);
    }
  }
  return uVar2;
}

