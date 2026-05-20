// addr: 0x8010efa4  name: FUN_8010efa4

/* WARNING: Removing unreachable block (ram,0x8010f068) */
/* WARNING: Removing unreachable block (ram,0x8010f070) */
/* WARNING: Removing unreachable block (ram,0x8010f07c) */

undefined4 FUN_8010efa4(int *param_1)

{
  int iVar1;
  int iVar2;
  uint unaff_s1;
  
  iVar2 = *param_1;
  iVar1 = 10;
  if (iVar2 == 0) {
LAB_8010efe8:
    DAT_80113424 = 0;
    DAT_80113420 = 0;
    *param_1 = iVar1;
  }
  else {
    unaff_s1 = 10;
    if (iVar2 < 0) {
      return 0;
    }
    if (iVar2 != 10) {
      iVar1 = 0;
      if (iVar2 == 0xb) goto LAB_8010f028;
      goto LAB_8010efe8;
    }
  }
  iVar1 = *param_1 + 1;
  *param_1 = iVar1;
LAB_8010f028:
  if (iVar1 != 0) {
    DAT_80113424 = 0;
    if ((DAT_80113434 & 1 << (DAT_80113444 & 0x1f)) == 0) {
      DAT_80113424 = 4;
    }
    DAT_80113420 = DAT_80113420 + 1;
    if (DAT_80113420 < 5) {
      DAT_80113444 = unaff_s1;
    }
    DAT_8011343c = ~(1 << (DAT_80113444 & 0x1f));
    DAT_80113434 = DAT_80113434 & DAT_8011343c;
  }
  return 0;
}

