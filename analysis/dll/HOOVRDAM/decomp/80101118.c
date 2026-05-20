// addr: 0x80101118  name: FUN_80101118

undefined4 FUN_80101118(int param_1,int param_2,int *param_3)

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
  if (iVar1 != 0) {
    uVar2 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar2,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),6,param_1 + 0x24);
  }
  return 0;
}

