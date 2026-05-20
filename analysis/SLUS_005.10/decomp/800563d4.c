// addr: 0x800563d4  name: PADSEQD_OBJ_1A0

void PADSEQD_OBJ_1A0(void)

{
  byte bVar1;
  int iVar2;
  int unaff_s0;
  uint unaff_s1;
  
  if (((*(char *)(*(int *)(unaff_s0 + 0x3c) + 1) == '\0') &&
      (((*(char *)(unaff_s0 + 0x46) != '\x01' || (*(int *)(unaff_s0 + 0x14) != 0)) &&
       (*(char *)(unaff_s0 + 0x50) == '\0')))) ||
     ((((iVar2 = PADSEQD_OBJ_448(), iVar2 == 0 && (*(char *)(unaff_s0 + 0x37) == '\0')) &&
       (*(char *)(unaff_s0 + 0x4a) == '\0')) && (*(byte *)(unaff_s0 + 0xe8) != unaff_s1)))) {
    (*DAT_80065260)();
  }
  *(undefined1 *)(unaff_s0 + 0x4a) = 0;
  if (((byte)(*(char *)(unaff_s0 + 0x46) - 2U) < 0xfc) && (**(char **)(unaff_s0 + 0x3c) != -0xd)) {
    (*DAT_80065260)();
  }
  bVar1 = *(byte *)(unaff_s0 + 0x46);
  if (((bVar1 == 0) || (bVar1 == 0xff)) || (*(char *)(unaff_s0 + 0x36) != '\0')) {
    if (bVar1 == 1) {
      *(undefined1 *)(unaff_s0 + 0x47) = 0;
      *(char *)(unaff_s0 + 0x46) = *(char *)(unaff_s0 + 0x46) + '\x01';
      PADSEQD_OBJ_358();
      return;
    }
    if (bVar1 < 2) {
      if (bVar1 != 0) {
        PADSEQD_OBJ_320();
        return;
      }
      if (*(char *)(unaff_s0 + 0xe8) != '\0') {
        *(undefined1 *)(unaff_s0 + 0x49) = 1;
        PADSEQD_OBJ_30C();
        return;
      }
    }
    else {
      if (bVar1 == 0xfe) {
        *(undefined1 *)(unaff_s0 + 0x46) = 0xff;
        PADSEQD_OBJ_358();
        return;
      }
      if (bVar1 != 0xff) {
        PADSEQD_OBJ_320();
        return;
      }
    }
  }
  return;
}

