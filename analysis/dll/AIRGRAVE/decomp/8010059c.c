// addr: 0x8010059c  name: FUN_8010059c

undefined4 FUN_8010059c(int param_1,int param_2,int *param_3)

{
  short sVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_2 != 3)) {
    *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + *(int *)(param_1 + 0x88);
    param_3 = (int *)(param_1 + 0x88);
    *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x8c);
    *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + *(int *)(param_1 + 0x90);
    sVar1 = ratan2/*0x8004ecd4*/(*(undefined4 *)(param_1 + 0x8c),0x1dcd);
    *(short *)(param_1 + 0x40) = -sVar1;
    *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) + 0x38;
    iVar2 = Terrain_HeightAt/*0x80025400*/(*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x50));
    if (iVar2 < *(int *)(param_1 + 0x4c)) goto LAB_80100668;
    FUN_8001d708/*0x8001d708*/(param_1);
  }
  if (*(char *)(*param_3 + 4) == '\a') {
    return 0;
  }
LAB_80100668:
  FUN_8003fc50/*0x8003fc50*/(param_1);
  FUN_800205f8/*0x800205f8*/(param_1);
  return 0xffffffff;
}

