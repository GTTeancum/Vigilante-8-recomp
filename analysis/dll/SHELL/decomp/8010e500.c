// addr: 0x8010e500  name: FUN_8010e500

void FUN_8010e500(undefined4 *param_1,uint param_2)

{
  *DAT_801124ec = *DAT_801124ec | 0x88;
  *DAT_801124b4 = (int)(param_1 + 1);
  *DAT_801124b8 = (param_2 >> 5) << 0x10 | 0x20;
  *DAT_801124e4 = *param_1;
  *DAT_801124bc = 0x1000201;
  return;
}

