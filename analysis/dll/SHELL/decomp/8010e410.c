// addr: 0x8010e410  name: FUN_8010e410

void FUN_8010e410(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  if (param_1 != 0) {
    uVar1 = 0x80000000;
    uVar2 = 1;
    if (param_1 == 1) goto LAB_8010e494;
  }
  *DAT_801124e8 = 0x80000000;
  *DAT_801124bc = 0;
  *DAT_801124c8 = 0;
  uVar1 = 0x60000000;
  *DAT_801124e8 = 0x60000000;
  uVar2 = 0x20;
LAB_8010e494:
  *DAT_801124e8 = uVar1;
  *DAT_801124bc = 0;
  *DAT_801124c8 = 0;
  *DAT_801124e8 = 0x60000000;
  printf/*0x80052604*/(s_MDEC_rest_bad_option__d__80101094,uVar2);
  return;
}

