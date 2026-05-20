// addr: 0x8010fd80  name: FUN_8010fd80

/* WARNING: Removing unreachable block (ram,0x8010fe1c) */

undefined4 FUN_8010fd80(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 != 10) {
    iVar1 = 0xb;
    if ((10 < iVar2) || (iVar1 = 0, iVar2 != 0)) {
      if (iVar2 == iVar1) goto LAB_8010fe30;
      if (iVar2 == 0x14) {
        DAT_80113448 = 0xffffffff;
        return 0;
      }
    }
    DAT_80113348 = 0;
    *param_1 = 10;
  }
  if (DAT_8011343c != 0) {
    return 1;
  }
LAB_8010fe30:
  *param_1 = 0x14;
  DAT_80113448 = 0xffffffff;
  return 0;
}

