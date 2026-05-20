// addr: 0x8010ad7c  name: FUN_8010ad7c

void FUN_8010ad7c(void)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  uVar4 = 0;
  bVar1 = false;
  iVar3 = 0;
  do {
    iVar2 = thunk_FUN_800523a0/*0x80052384*/(DAT_801133a0 + iVar3,s_BASLUS_00510_80100d1c,0xc);
    if ((iVar2 == 0) && (!bVar1)) {
      bVar1 = true;
    }
    uVar4 = uVar4 + 1;
    iVar3 = iVar3 + 0x28;
  } while (uVar4 < 0xf);
  return;
}

