// addr: 0x8010f5e0  name: FUN_8010f5e0

undefined4 FUN_8010f5e0(undefined4 param_1,uint param_2,undefined4 param_3)

{
  if ((param_2 & 0x7f) == 0) {
    DAT_80113438 = 5;
    DAT_8011343c = 0;
    DAT_80113440 = 0;
    DAT_8011344c = param_2;
    DAT_80113450 = param_3;
    DAT_80113454 = param_1;
  }
  printf/*0x80052604*/(s_Access_Denied____invalid_offset_v_8010121c);
  return 0;
}

