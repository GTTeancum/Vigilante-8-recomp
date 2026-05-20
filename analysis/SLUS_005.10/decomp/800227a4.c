// addr: 0x800227a4  name: FUN_800227a4

void FUN_800227a4(uint param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  uint local_20;
  undefined4 local_1c;
  undefined4 local_18 [2];
  
  iVar5 = 0;
  puVar4 = &DAT_8006f7a0;
  puRam0000076c = (undefined4 *)&DAT_80065a74;
  uRam00000770 = 0;
  DAT_80065a78 = &DAT_80065a70;
  do {
    puVar2 = puRam0000076c;
    iVar5 = iVar5 + 1;
    puVar1 = puRam0000076c + 1;
    puRam0000076c = puVar4;
    *puVar1 = puVar4;
    *puVar4 = puVar2;
    puVar4[1] = &DAT_80065a70;
    puVar4 = puVar4 + 4;
  } while (iVar5 < 0x3ff);
  puVar4 = &DAT_800737a0;
  DAT_80065a04 = DAT_8006567c;
  DAT_80065a08 = 1;
  DAT_80065a09 = 1;
  puRam0000074c = &DAT_80065a54;
  uRam00000750 = 0;
  DAT_80065a58 = &DAT_80065a50;
  puRam00000714 = &DAT_80065a1c;
  uRam00000718 = 0;
  DAT_80065a20 = &DAT_80065a18;
  puRam0000077c = &DAT_80065a84;
  uRam00000780 = 0;
  DAT_80065a88 = &DAT_80065a80;
  puRam0000075c = &DAT_80065a64;
  uRam00000760 = 0;
  DAT_80065a68 = &DAT_80065a60;
  puRam000007bc = &DAT_80065ac4;
  uRam000007c0 = 0;
  DAT_80065ac8 = &DAT_80065ac0;
  puRam0000079c = &DAT_80065aa4;
  uRam000007a0 = 0;
  DAT_80065aa8 = &DAT_80065aa0;
  puRam0000078c = &DAT_80065a94;
  uRam000007d8 = 0;
  uRam00000790 = 0;
  DAT_80065a98 = &DAT_80065a90;
  uRam00000734 = 0;
  FUN_80044f64(&DAT_800737a0,0x100);
  param_1 = param_1 & 0xffff;
  if (param_1 != 0) {
    FUN_800159b4("Common.exp");
    FUN_800225d4(&local_20,local_18);
    local_18[0] = local_1c;
    do {
      iVar5 = FUN_800225d4(&local_20,local_18);
      if (iVar5 == 0) {
        if ((local_20 >> 0x18 | local_20 >> 8 & 0xff00 | (local_20 & 0xff00) << 8 | local_20 << 0x18
            ) == 0x584f4246) {
          if ((param_1 & 1) == 0) {
            FUN_80015bf0(local_1c,1);
          }
          else {
            uVar3 = FUN_8002263c(local_1c,0);
            *puVar4 = uVar3;
          }
          puVar4 = puVar4 + 1;
          param_1 = (int)param_1 >> 1;
        }
      }
      else {
        FUN_80045088(iVar5);
      }
    } while (param_1 != 0);
    FUN_80015a00();
  }
  uRam000007d4 = 0;
  uRam000007d0 = 0;
  uRam000006f8 = 0;
  uRam000006ec = 0;
  uRam000006f4 = 0;
  uRam00000708 = 0;
  uRam00000758 = 0;
  uRam000006e8 = 0;
  uRam000007a8 = 0;
  uRam0000070c = 0;
  return;
}

