// addr: 0x801105d4  name: FUN_801105d4

undefined4 FUN_801105d4(undefined4 param_1,undefined1 *param_2)

{
  undefined1 auStack_48 [64];
  
  if (DAT_80113438 == 0) {
    DAT_80113434 = DAT_80113434 | 1 << (DAT_80113444 & 0x1f);
    param_2 = auStack_48;
  }
  printf/*0x80052604*/(s_Access_Denied____system_busy_8010124c,param_2);
  return 0xffffffff;
}

