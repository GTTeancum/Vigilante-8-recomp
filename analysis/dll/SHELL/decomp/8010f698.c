// addr: 0x8010f698  name: FUN_8010f698

/* WARNING: Removing unreachable block (ram,0x8010f770) */
/* WARNING: Removing unreachable block (ram,0x8010f794) */

undefined4 FUN_8010f698(int *param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = *param_1;
  uVar2 = (uint)(iVar4 < 0xb);
  if (iVar4 != 10) {
    iVar3 = 0x1e;
    if (((uVar2 == 0) || (iVar3 = 0, iVar4 != 0)) && (uVar2 = 0, iVar4 == iVar3)) goto LAB_8010f74c;
    uVar2 = 0;
    DAT_8011333c = 0;
    *param_1 = 10;
  }
  do {
  } while (uVar2 != DAT_8011344c);
  do {
    bVar1 = uVar2 != 0;
    uVar2 = 0x1e;
  } while (bVar1);
  *param_1 = 0x1e;
LAB_8010f74c:
  if (uVar2 != 0) {
    DAT_8011333c = 0;
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

