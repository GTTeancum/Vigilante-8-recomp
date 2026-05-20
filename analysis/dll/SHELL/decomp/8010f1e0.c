// addr: 0x8010f1e0  name: FUN_8010f1e0

/* WARNING: Removing unreachable block (ram,0x8010f340) */
/* WARNING: Removing unreachable block (ram,0x8010f348) */
/* WARNING: Removing unreachable block (ram,0x8010f354) */

undefined4 FUN_8010f1e0(uint *param_1)

{
  undefined *puVar1;
  uint uVar2;
  
  uVar2 = *param_1;
  if (0x1f < uVar2) {
    return 0;
  }
  puVar1 = (&switchD_8010f218::switchdataD_8010110c)[uVar2];
  switch(uVar2) {
  case 0:
    DAT_80113430 = 0;
    DAT_8011342c = 0;
    DAT_80113428 = 0;
    *param_1 = 10;
    break;
  default:
    goto switchD_8010f218_caseD_1;
  case 10:
    break;
  case 0x15:
    goto switchD_8010f218_caseD_15;
  case 0x1e:
    goto switchD_8010f218_caseD_1e;
  case 0x1f:
    goto switchD_8010f218_caseD_1f;
  }
  if (DAT_8011343c == 0) {
LAB_8010f3ac:
    *param_1 = 0x1e;
  }
  else {
    if (DAT_8011343c != 3) {
      return 1;
    }
    DAT_80113430 = 1;
    DAT_80113434 = DAT_80113434 | 1 << (DAT_80113444 & 0x1f);
    puVar1 = (undefined *)0x15;
    *param_1 = 0x15;
switchD_8010f218_caseD_15:
    if (puVar1 == (undefined *)0x0) {
      return 0;
    }
    *param_1 = 0x1e;
switchD_8010f218_caseD_1e:
    puVar1 = (undefined *)(*param_1 + 1);
    *param_1 = (uint)puVar1;
switchD_8010f218_caseD_1f:
    if (puVar1 == (undefined *)0x0) {
      return 0;
    }
    DAT_8011342c = 0;
    DAT_80113428 = DAT_80113428 + 1;
    if (DAT_80113428 < 5) goto LAB_8010f3ac;
  }
  DAT_8011343c = 4;
  if (DAT_8011342c == 4) {
    DAT_8011343c = 1;
  }
switchD_8010f218_caseD_1:
  return 0;
}

