// addr: 0x80111188  name: FUN_80111188

int FUN_80111188(void)

{
  int iVar1;
  
  do {
    iVar1 = DAT_80113530 + DAT_80113534 * 2 + DAT_80113538 * 4 + DAT_8011353c * 8;
  } while (iVar1 == 0);
  TestEvent/*0x800539d4*/(DAT_80113520);
  TestEvent/*0x800539d4*/(DAT_80113524);
  TestEvent/*0x800539d4*/(DAT_80113528);
  TestEvent/*0x800539d4*/(DAT_8011352c);
  DAT_8011353c = 0;
  DAT_80113538 = 0;
  DAT_80113534 = 0;
  DAT_80113530 = 0;
  return iVar1 >> 1;
}

