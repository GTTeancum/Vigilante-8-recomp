// addr: 0x8010fa48  name: FUN_8010fa48

undefined4
FUN_8010fa48(undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4,
            undefined4 param_5)

{
  if ((param_4 & 0x7f) == 0) {
    strcat/*0x800523d4*/(&DAT_80113458,param_2);
    DAT_80113438 = 3;
    DAT_8011343c = 0;
    DAT_80113440 = 0;
    DAT_80113450 = param_5;
    DAT_80113444 = param_1;
    DAT_8011344c = param_4;
    DAT_80113454 = param_3;
  }
  printf/*0x80052604*/(s_Access_Denied____invalid_offset_v_8010121c);
  return 0;
}

