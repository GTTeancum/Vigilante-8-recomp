// addr: 0x80110c00  name: FUN_80110c00

void FUN_80110c00(undefined4 param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = DAT_80113354 + 1;
  if (3 < iVar3) {
    printf/*0x80052604*/(s_libmcrd__event_overflow_80101274);
    param_1 = 1;
  }
  iVar2 = 3;
  puVar1 = &DAT_801134cc + iVar3 * 4;
  DAT_80113354 = iVar3;
  (&DAT_80113500)[iVar3] = param_1;
  do {
    *puVar1 = 0;
    iVar2 = iVar2 + -1;
    puVar1 = puVar1 + -1;
  } while (-1 < iVar2);
  return;
}

