// addr: 0x8010d55c  name: FUN_8010d55c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010d55c(void)

{
  if ((DAT_8011340c != 0) && (_DAT_800a3294 != 0)) {
    StCdInterrupt/*0x8004ba54*/();
    _DAT_800a3294 = 0;
  }
  LoadImage/*0x8004f82c*/(&DAT_801133fc,(&DAT_801133dc)[DAT_801133e4]);
  DAT_801133e4 = 1 - DAT_801133e4;
  DAT_801133fc = DAT_801133fc + DAT_80113400;
  DAT_80113404 = 1;
  return;
}

