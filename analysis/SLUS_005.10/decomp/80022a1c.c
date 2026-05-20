// addr: 0x80022a1c  name: FUN_80022a1c

void FUN_80022a1c(void)

{
  undefined4 uVar1;
  int *piVar2;
  int iVar3;
  
  FUN_8001af48(uRam000007d8);
  if (iRam000006f8 != 0) {
    uVar1 = FUN_800203fc();
    FUN_80045088(uVar1);
  }
  if (iRam000006ec != 0) {
    FUN_80045088();
  }
  FUN_80020658(&DAT_80065a50);
  FUN_80020658(&DAT_80065a18);
  FUN_80020968(uRam000006fc);
  uRam000006fc = 0;
  if (puRam000007c4 != &DAT_80065ac0) {
    do {
      FUN_80020540(*(undefined4 *)(iRam000007bc + 8));
    } while (puRam000007c4 != &DAT_80065ac0);
  }
  if (puRam000007a4 != &DAT_80065aa0) {
    do {
      piRam0000079c = (int *)*piRam0000079c;
      piRam0000079c[1] = (int)&DAT_80065aa0;
      FUN_80045088();
    } while (puRam000007a4 != &DAT_80065aa0);
  }
  if (iRam00000734 != 0) {
    FUN_80045088();
  }
  piVar2 = &DAT_800737a0;
  iVar3 = 0;
  if (DAT_800737e4 == DAT_800737e0) {
    DAT_800737e4 = 0;
  }
  do {
    if (*piVar2 != 0) {
      FUN_8001aa38();
    }
    iVar3 = iVar3 + 1;
    piVar2 = piVar2 + 1;
  } while (iVar3 < 0x40);
  FUN_8001884c(&DAT_80065a28);
  return;
}

