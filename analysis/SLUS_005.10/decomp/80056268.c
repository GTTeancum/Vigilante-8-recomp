// addr: 0x80056268  name: PADSEQD_OBJ_34

undefined4 PADSEQD_OBJ_34(int param_1)

{
  byte bVar1;
  undefined4 uVar2;
  
  if ((**(char **)(param_1 + 0x3c) == -0xd) && (*(char *)(param_1 + 0xe8) == '\0')) {
PADSEQD_OBJ_C0:
    _padCmdParaMode(param_1,0);
    uVar2 = PADSEQD_OBJ_F8();
    return uVar2;
  }
  bVar1 = *(byte *)(param_1 + 0x46);
  if (bVar1 == 1) {
    _padCmdParaMode(param_1,1);
    uVar2 = PADSEQD_OBJ_F8();
    return uVar2;
  }
  if (bVar1 < 2) {
    if (bVar1 != 0) {
      uVar2 = PADSEQD_OBJ_D0();
      return uVar2;
    }
  }
  else {
    if (bVar1 == 0xfe) goto PADSEQD_OBJ_C0;
    if (bVar1 != 0xff) {
      uVar2 = PADSEQD_OBJ_D0();
      return uVar2;
    }
  }
  return 0;
}

