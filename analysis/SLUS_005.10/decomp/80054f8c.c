// addr: 0x80054f8c  name: PADCMD_OBJ_2F8

void PADCMD_OBJ_2F8(int param_1)

{
  byte bVar1;
  
  bVar1 = *(byte *)(param_1 + 0x46);
  if (bVar1 == 3) {
    PADCMD_OBJ_900(param_1,*(undefined1 *)(param_1 + 0x47));
    PADCMD_OBJ_390();
    return;
  }
  if (bVar1 < 4) {
    if (bVar1 != 2) {
      PADCMD_OBJ_390();
      return;
    }
    PADCMD_OBJ_8E0(param_1,*(undefined1 *)(param_1 + 0x47));
    PADCMD_OBJ_390();
    return;
  }
  if (bVar1 != 4) {
    PADCMD_OBJ_390();
    return;
  }
  if (*(char *)(param_1 + 0x48) == '\0') {
    PADCMD_OBJ_920(param_1,*(undefined1 *)(param_1 + 0x47));
    PADCMD_OBJ_390();
    return;
  }
  PADCMD_OBJ_940();
  return;
}

