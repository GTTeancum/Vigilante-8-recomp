// addr: 0x80106840  name: FUN_80106840

undefined4 FUN_80106840(undefined4 param_1)

{
  printf/*0x80052604*/(s__s_timeout__80100134,param_1);
  *DAT_80106f40 = 0x80000000;
  *DAT_80106f14 = 0;
  *DAT_80106f20 = 0;
  *DAT_80106f40 = 0x60000000;
  return 0;
}

