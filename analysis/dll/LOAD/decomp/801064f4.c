// addr: 0x801064f4  name: FUN_801064f4

void FUN_801064f4(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  if (param_1 != 0) {
    uVar1 = 0x80000000;
    uVar2 = 1;
    if (param_1 == 1) goto LAB_80106578;
  }
  *DAT_80106f40 = 0x80000000;
  *DAT_80106f14 = 0;
  *DAT_80106f20 = 0;
  uVar1 = 0x60000000;
  *DAT_80106f40 = 0x60000000;
  uVar2 = 0x20;
LAB_80106578:
  *DAT_80106f40 = uVar1;
  *DAT_80106f14 = 0;
  *DAT_80106f20 = 0;
  *DAT_80106f40 = 0x60000000;
  printf/*0x80052604*/(s_MDEC_rest_bad_option__d__801000f8,uVar2);
  return;
}

