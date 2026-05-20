// addr: 0x80111260  name: FUN_80111260

int FUN_80111260(void)

{
  int iVar1;
  
  do {
    iVar1 = DAT_80113540 + DAT_80113544 * 2 + DAT_80113548 * 4 + DAT_8011354c * 8;
  } while (iVar1 == 0);
  TestEvent/*0x800539d4*/(DAT_80113510);
  TestEvent/*0x800539d4*/(DAT_80113514);
  TestEvent/*0x800539d4*/(DAT_80113518);
  TestEvent/*0x800539d4*/(DAT_8011351c);
  DAT_8011354c = 0;
  DAT_80113548 = 0;
  DAT_80113544 = 0;
  DAT_80113540 = 0;
  return iVar1 >> 1;
}

