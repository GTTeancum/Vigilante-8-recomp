// addr: 0x80106674  name: FUN_80106674

void FUN_80106674(undefined4 param_1,uint param_2)

{
  *DAT_80106f44 = *DAT_80106f44 | 0x88;
  *DAT_80106f20 = 0;
  *DAT_80106f18 = param_1;
  *DAT_80106f1c = (param_2 >> 5) << 0x10 | 0x20;
  *DAT_80106f20 = 0x1000200;
  return;
}

