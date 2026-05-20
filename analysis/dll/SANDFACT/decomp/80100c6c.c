// addr: 0x80100c6c  name: FUN_80100c6c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c6c(uint *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  uint extraout_v1;
  uint uVar4;
  
  if ((param_2 == 0) || (param_2 != 2)) {
    uVar3 = param_1[0x20];
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    uVar4 = param_1[0x21];
    param_1[9] = param_1[9] + ((int)uVar3 >> 7);
    if ((int)uVar4 < 0) {
      uVar4 = uVar4 + 0x7f;
    }
    uVar3 = param_1[0x22];
    param_1[10] = param_1[10] + ((int)uVar4 >> 7);
    if ((int)uVar3 < 0) {
      uVar3 = uVar3 + 0x7f;
    }
    param_1[0xb] = param_1[0xb] + ((int)uVar3 >> 7);
  }
  uVar3 = *param_1;
  if ((uVar3 & 2) != 0) {
    *param_1 = uVar3 & 0xfefffffd;
    FUN_8001d708/*0x8001d708*/(param_1);
    iVar1 = (short)param_1[5] * 0x4786;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1f;
    }
    param_1[0x20] = iVar1 >> 5;
    iVar1 = *(short *)((int)param_1 + 0x1a) * 0x4786;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1f;
    }
    param_1[0x21] = iVar1 >> 5;
    iVar1 = (short)param_1[8] * 0x4786;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1f;
    }
    param_1[0x22] = iVar1 >> 5;
    if (param_1[0x38] != 0) {
      FUN_80020890/*0x80020890*/(param_1[0x38],0x3c);
    }
    FUN_80020890/*0x80020890*/(param_1,0x1e);
    uVar2 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar2,*(undefined4 *)(_DAT_800737e8 + 8),4,param_1 + 9);
    uVar3 = extraout_v1;
  }
  *param_1 = uVar3 & 0xffffffdf;
  func_0x80031294(param_1);
  return 0;
}

