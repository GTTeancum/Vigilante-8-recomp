// addr: 0x8010fb60  name: FUN_8010fb60

/* WARNING: Removing unreachable block (ram,0x8010fbfc) */

undefined4 FUN_8010fb60(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 != 10) {
    iVar1 = 0xb;
    if ((10 < iVar2) || (iVar1 = 0, iVar2 != 0)) {
      if (iVar2 == iVar1) goto LAB_8010fc10;
      if (iVar2 == 0x14) {
        DAT_80113448 = 0xffffffff;
        return 0;
      }
    }
    DAT_80113344 = 0;
    *param_1 = 10;
  }
  if (DAT_8011343c != 0) {
    return 1;
  }
LAB_8010fc10:
  *param_1 = 0x14;
  DAT_80113448 = 0xffffffff;
  return 0;
}

