// addr: 0x80100540  name: FUN_80100540

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100540(int param_1,uint param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = 2;
  if (param_2 == 2) {
LAB_80100594:
    FUN_80020890/*0x80020890*/(iVar1,0x3c);
    FUN_80020890/*0x80020890*/(param_1,0xf0);
    uVar2 = FUN_8004410c/*0x8004410c*/();
    param_3 = 0x3b;
    FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x3b,iVar1 + 0x24);
LAB_801005e0:
    uVar2 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
    *(undefined4 *)(param_1 + 0x70) = uVar2;
    *(undefined4 *)(param_1 + 0x74) = uVar2;
  }
  else {
    if ((param_2 < 3) && (param_2 == 1)) goto LAB_801005e0;
    if (param_2 != 3) {
      iVar1 = 0;
      if (param_2 == 8) goto LAB_80100614;
      goto LAB_80100594;
    }
  }
  FUN_8002239c/*0x8002239c*/(param_1,param_3);
LAB_80100614:
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  if ((iVar1 != 0) && (*(int *)(param_1 + 0x74) != 0)) {
    FUN_80020890/*0x80020890*/(*(int *)(param_1 + 0x74),0);
    FUN_80020890/*0x80020890*/(param_1,0xf0);
    *(undefined4 *)(param_1 + 0x74) = 0;
  }
  return 0;
}

