// addr: 0x800449bc  name: FUN_800449bc

uint FUN_800449bc(undefined4 param_1)

{
  short sVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  undefined1 auStack_28 [16];
  undefined1 auStack_18 [16];
  
  FUN_80043408(&DAT_8006f680,param_1,auStack_28);
  if (iRam00000010 == 0) {
    if (cRam000005a8 == '\0') {
      uVar4 = FUN_800448ec(auStack_28);
    }
    else {
      iVar3 = FUN_80044890(auStack_28);
      uVar4 = (iVar3 * 0x10000 >> 0x10) + iVar3 * 0x10000;
    }
  }
  else {
    FUN_80043408(&DAT_8006f6a0,param_1,auStack_18);
    if (cRam000005a8 == '\0') {
      iVar3 = FUN_80044890(auStack_28);
      sVar1 = FUN_80044890(auStack_18);
      uVar4 = iVar3 << 0x10 | (int)sVar1;
    }
    else {
      sVar1 = FUN_80044890(auStack_28);
      sVar2 = FUN_80044890(auStack_18);
      iVar3 = (int)sRam000008e4;
      if ((int)sVar1 + (int)sVar2 < (int)sRam000008e4) {
        iVar3 = (int)sVar1 + (int)sVar2;
      }
      uVar4 = iVar3 * 0x10001;
    }
  }
  return uVar4;
}

