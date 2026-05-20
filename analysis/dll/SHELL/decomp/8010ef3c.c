// addr: 0x8010ef3c  name: FUN_8010ef3c

undefined4 FUN_8010ef3c(undefined4 param_1)

{
  if (DAT_80113438 == 0) {
    DAT_80113438 = 1;
    DAT_8011343c = 0;
    DAT_80113440 = 0;
    DAT_80113444 = param_1;
  }
  printf/*0x80052604*/(s_Access_Denied____event_multiple_o_801010e4);
  return 0;
}

