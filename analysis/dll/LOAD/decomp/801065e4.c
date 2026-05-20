// addr: 0x801065e4  name: FUN_801065e4

void FUN_801065e4(undefined4 *param_1,uint param_2)

{
  *DAT_80106f44 = *DAT_80106f44 | 0x88;
  *DAT_80106f0c = (int)(param_1 + 1);
  *DAT_80106f10 = (param_2 >> 5) << 0x10 | 0x20;
  *DAT_80106f3c = *param_1;
  *DAT_80106f14 = 0x1000201;
  return;
}

