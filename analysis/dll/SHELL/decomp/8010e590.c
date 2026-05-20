// addr: 0x8010e590  name: FUN_8010e590

void FUN_8010e590(undefined4 param_1,uint param_2)

{
  *DAT_801124ec = *DAT_801124ec | 0x88;
  *DAT_801124c8 = 0;
  *DAT_801124c0 = param_1;
  *DAT_801124c4 = (param_2 >> 5) << 0x10 | 0x20;
  *DAT_801124c8 = 0x1000200;
  return;
}

