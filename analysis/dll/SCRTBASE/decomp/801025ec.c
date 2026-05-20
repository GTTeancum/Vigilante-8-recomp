// addr: 0x801025ec  name: FUN_801025ec

undefined4 FUN_801025ec(int param_1,int param_2,int *param_3)

{
  int iVar1;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    if (*(char *)(*param_3 + 4) != '\a') {
      return 0;
    }
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
  }
  iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
  if (iVar1 != 0) {
    FUN_8002185c/*0x8002185c*/(9,(int)*(short *)(param_1 + 6));
  }
  return 0;
}

