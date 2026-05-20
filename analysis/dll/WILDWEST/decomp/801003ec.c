// addr: 0x801003ec  name: FUN_801003ec

undefined4 FUN_801003ec(int param_1,uint param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  
  if (param_2 == 3) {
LAB_80100434:
    iVar1 = FUN_8002239c/*0x8002239c*/(param_1,param_3);
    iVar2 = param_1 + 0x48;
    if (iVar1 != 0) goto LAB_8010047c;
LAB_80100450:
    iVar1 = FUN_80022320/*0x80022320*/(param_1,param_3);
    iVar2 = param_1 + 0x48;
    if (iVar1 != 0) goto LAB_8010047c;
  }
  else if ((3 < param_2) || (param_2 != 1)) {
    if (param_2 != 8) goto LAB_80100434;
    goto LAB_80100450;
  }
  iVar2 = 1;
  *(undefined2 *)(*(int *)(param_1 + 0x30) + 0x28) = 0x14;
LAB_8010047c:
  iVar1 = FUN_80042cdc/*0x80042cdc*/(iVar2,0);
  *(undefined2 *)(iVar1 + 8) = 0xffff;
  return 0;
}

