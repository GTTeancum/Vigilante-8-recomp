// addr: 0x80101a30  name: FUN_80101a30

void FUN_80101a30(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  short sVar3;
  int iVar4;
  short local_28 [4];
  
  iVar4 = 0;
  iVar1 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801003ec,0x326478);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801003f4,0);
  *(undefined1 *)(iVar1 + 4) = 0x40;
  *(undefined1 *)(iVar1 + 5) = 0x40;
  *(undefined1 *)(iVar1 + 6) = 0x40;
  if (0 < param_1) {
    sVar3 = 1;
    do {
      local_28[0] = (short)(0x260 / (param_1 << 1)) * sVar3;
      iVar4 = iVar4 + 1;
      local_28[2] = 0;
      local_28[3] = 0;
      sVar3 = sVar3 + 2;
      local_28[1] = 0x1ba;
      FUN_80019a58/*0x80019a58*/(iVar1,param_2,local_28,10);
      iVar2 = FUN_80052544/*0x80052544*/(param_2);
      param_2 = param_2 + iVar2 + 1;
    } while (iVar4 < param_1);
  }
  FUN_800190a8/*0x800190a8*/(iVar1);
  return;
}

