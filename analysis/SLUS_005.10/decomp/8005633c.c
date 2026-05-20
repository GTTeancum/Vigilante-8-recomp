// addr: 0x8005633c  name: PADSEQD_OBJ_108

void PADSEQD_OBJ_108(int param_1)

{
  char cVar1;
  byte bVar2;
  int iVar3;
  
  cVar1 = *(char *)(param_1 + 0xe8);
  bVar2 = **(byte **)(param_1 + 0x3c) >> 4;
  *(byte *)(param_1 + 0xe8) = bVar2;
  if (bVar2 == 0xf) {
    *(char *)(param_1 + 0xe8) = cVar1;
    PADSEQD_OBJ_1A0();
    return;
  }
  **(undefined1 **)(param_1 + 0x30) = 0;
  *(undefined1 *)(*(int *)(param_1 + 0x30) + 1) = **(undefined1 **)(param_1 + 0x3c);
  iVar3 = 2;
  if (2 < *(byte *)(param_1 + 0x44)) {
    do {
      *(undefined1 *)(*(int *)(param_1 + 0x30) + iVar3) =
           *(undefined1 *)(*(int *)(param_1 + 0x3c) + iVar3);
      iVar3 = iVar3 + 1;
    } while (iVar3 < (int)(uint)*(byte *)(param_1 + 0x44));
  }
  if (((*(char *)(*(int *)(param_1 + 0x3c) + 1) == '\0') &&
      (((*(char *)(param_1 + 0x46) != '\x01' || (*(int *)(param_1 + 0x14) != 0)) &&
       (*(char *)(param_1 + 0x50) == '\0')))) ||
     ((((iVar3 = PADSEQD_OBJ_448(param_1), iVar3 == 0 && (*(char *)(param_1 + 0x37) == '\0')) &&
       (*(char *)(param_1 + 0x4a) == '\0')) && (*(char *)(param_1 + 0xe8) != cVar1)))) {
    (*DAT_80065260)(param_1);
  }
  *(undefined1 *)(param_1 + 0x4a) = 0;
  if (((byte)(*(char *)(param_1 + 0x46) - 2U) < 0xfc) && (**(char **)(param_1 + 0x3c) != -0xd)) {
    (*DAT_80065260)(param_1);
  }
  bVar2 = *(byte *)(param_1 + 0x46);
  if (((bVar2 == 0) || (bVar2 == 0xff)) || (*(char *)(param_1 + 0x36) != '\0')) {
    if (bVar2 == 1) {
      *(undefined1 *)(param_1 + 0x47) = 0;
      *(char *)(param_1 + 0x46) = *(char *)(param_1 + 0x46) + '\x01';
      PADSEQD_OBJ_358();
      return;
    }
    if (bVar2 < 2) {
      if (bVar2 != 0) {
        PADSEQD_OBJ_320();
        return;
      }
      if (*(char *)(param_1 + 0xe8) != '\0') {
        *(undefined1 *)(param_1 + 0x49) = 1;
        PADSEQD_OBJ_30C();
        return;
      }
    }
    else {
      if (bVar2 == 0xfe) {
        *(undefined1 *)(param_1 + 0x46) = 0xff;
        PADSEQD_OBJ_358();
        return;
      }
      if (bVar2 != 0xff) {
        PADSEQD_OBJ_320();
        return;
      }
    }
  }
  return;
}

