// addr: 0x80110418  name: FUN_80110418

/* WARNING: Removing unreachable block (ram,0x80110510) */
/* WARNING: Removing unreachable block (ram,0x80110550) */
/* WARNING: Removing unreachable block (ram,0x80110564) */
/* WARNING: Removing unreachable block (ram,0x80110584) */
/* WARNING: Removing unreachable block (ram,0x8011058c) */
/* WARNING: Removing unreachable block (ram,0x8011059c) */
/* WARNING: Removing unreachable block (ram,0x801105a4) */
/* WARNING: Removing unreachable block (ram,0x801105a8) */

undefined4 FUN_80110418(undefined4 param_1,undefined4 param_2)

{
  undefined1 auStack_40 [32];
  uint local_20;
  
  if (DAT_80113438 != 0) {
    printf/*0x80052604*/(s_Access_Denied____system_busy_8010124c);
  }
  strcat/*0x800523d4*/(auStack_40,param_2);
  local_20 = 1 << (DAT_80113444 & 0x1f);
  DAT_80113434 = DAT_80113434 | local_20;
  if (local_20 != 0) {
    return 0;
  }
  if (DAT_80113438 != 0) {
    DAT_8011348c = local_20;
    printf/*0x80052604*/(s_Access_Denied____event_multiple_o_801010e4);
  }
  do {
  } while( true );
}

