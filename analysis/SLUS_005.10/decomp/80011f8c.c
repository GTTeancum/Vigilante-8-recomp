// addr: 0x80011f8c  name: FUN_80011f8c

undefined4 FUN_80011f8c(int param_1)

{
  byte bVar1;
  
  if ((&DAT_80066458)[param_1 * 0x22] != '\0') {
    return 0;
  }
  bVar1 = (&DAT_80066459)[param_1 * 0x22];
  if (bVar1 == 0x53) {
    return 4;
  }
  if (bVar1 < 0x54) {
    if (bVar1 == 0x23) {
      return 3;
    }
    if (bVar1 == 0x41) {
      return 2;
    }
  }
  else if (bVar1 == 0x73) {
    return 5;
  }
  return 1;
}

