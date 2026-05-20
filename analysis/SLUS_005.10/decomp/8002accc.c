// addr: 0x8002accc  name: FUN_8002accc

void FUN_8002accc(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint uVar3;
  
  puVar2 = &DAT_80065b70;
  uVar3 = 0;
  FUN_8001bddc(uRam000008b0);
  uRam000008b0 = 0;
  do {
    uVar1 = *puVar2;
    puVar2 = puVar2 + 1;
    uVar3 = uVar3 + 1;
    FUN_8001bddc(uVar1);
  } while (uVar3 < 4);
  FUN_800190d8(uRam0000087c);
  return;
}

