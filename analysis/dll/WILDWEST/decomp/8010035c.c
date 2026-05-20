// addr: 0x8010035c  name: FUN_8010035c

undefined4 FUN_8010035c(int param_1,int param_2,int *param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    if (*(char *)(*param_3 + 4) != '\a') {
      return 0;
    }
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  uVar2 = 0;
  if (iVar1 != 0) {
    iVar1 = FUN_8001fd9c/*0x8001fd9c*/((int)*(short *)(param_1 + 6));
    if (iVar1 != 0) {
      FUN_80024718/*0x80024718*/(iVar1 + 0xc,0x8f80);
    }
    uVar2 = 0xffffffff;
  }
  return uVar2;
}

