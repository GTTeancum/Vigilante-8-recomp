// addr: 0x8010f414  name: FUN_8010f414

int FUN_8010f414(undefined4 param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  int local_28;
  
  iVar1 = 0;
  if (-1 < DAT_80113448) {
    printf/*0x80052604*/(s_Access_Denied____file_already_op_8010118c);
  }
  strcat/*0x800523d4*/(&DAT_80113458,param_2);
  do {
    DAT_80113444 = param_1;
    DAT_80113448 = printf/*0x80052604*/(s_into_open_801011b4);
    printf/*0x80052604*/(s_exit_open_801011c0,param_3 | 0x8000);
    if (-1 < DAT_80113448) {
      return 0;
    }
    DAT_8011348c = 0;
    if (DAT_80113438 != 0) {
      printf/*0x80052604*/(s_Access_Denied____event_multiple_o_801010e4);
    }
    DAT_80113438 = 2;
    DAT_8011343c = 0;
    DAT_80113440 = 0;
  } while ((local_28 == 3) || ((local_28 == 2 && (iVar1 = iVar1 + 1, iVar1 < 5))));
  if (local_28 == 0) {
    local_28 = 5;
  }
  DAT_80113444 = param_1;
  return local_28;
}

