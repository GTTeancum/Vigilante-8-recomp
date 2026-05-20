// addr: 0x8004410c  name: FUN_8004410c

int FUN_8004410c(void)

{
  int iVar1;
  uint uVar2;
  dword *pdVar3;
  
  iVar1 = 0;
  pdVar3 = &VOICE_00_LEFT_RIGHT;
  uVar2 = uRam000008fc;
  while ((((uVar2 & 1) != 0 || ((short)pdVar3[3] != 0)) ||
         ((iRamffffacb0 - (uint)(byte)(&DAT_800a2ff0)[iVar1] & 0xff) < 2))) {
    pdVar3 = pdVar3 + 4;
    iVar1 = iVar1 + 1;
    uVar2 = (int)uVar2 >> 1;
    if (0x17 < iVar1) {
      return 0;
    }
  }
  return iVar1 + 1;
}

