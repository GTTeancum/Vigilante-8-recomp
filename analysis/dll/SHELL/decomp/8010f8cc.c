// addr: 0x8010f8cc  name: FUN_8010f8cc

/* WARNING: Removing unreachable block (ram,0x8010f9a4) */
/* WARNING: Removing unreachable block (ram,0x8010f9c8) */

undefined4 FUN_8010f8cc(int *param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = *param_1;
  uVar1 = (uint)(iVar4 < 0xb);
  if (iVar4 != 10) {
    iVar2 = 0x1e;
    if (((uVar1 == 0) || (iVar2 = 0, iVar4 != 0)) && (iVar3 = 0, iVar4 == iVar2)) goto LAB_8010f980;
    uVar1 = 0;
    DAT_80113340 = 0;
    *param_1 = 10;
  }
  do {
  } while (uVar1 != DAT_8011344c);
  do {
    iVar4 = write/*0x80053a44*/(DAT_80113448,DAT_80113454,0x80);
    iVar3 = 0x1e;
  } while (iVar4 != 0);
  *param_1 = 0x1e;
LAB_8010f980:
  if (iVar3 != 0) {
    DAT_80113340 = 0;
    DAT_80113454 = DAT_80113454 + 0x80;
    DAT_80113450 = DAT_80113450 + -0x80;
    DAT_8011344c = DAT_8011344c + 0x80;
    if (DAT_80113450 < 1) {
      DAT_8011343c = DAT_8011344c;
    }
    *param_1 = 10;
  }
  return 0;
}

