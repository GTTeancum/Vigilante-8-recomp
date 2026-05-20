// addr: 0x800565a0  name: _dirFailAuto

void _dirFailAuto(int param_1)

{
  byte bVar1;
  
  *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + 1;
  if (*(char *)(param_1 + 0x46) == '\0') {
PADSEQD_OBJ_404:
    if (**(char **)(param_1 + 0x3c) != -0xd) {
      **(undefined1 **)(param_1 + 0x30) = 0xff;
      *(undefined1 *)(*(int *)(param_1 + 0x30) + 1) = 0;
      *(undefined1 *)(param_1 + 0xe8) = 0;
    }
    return;
  }
  if (*(char *)(param_1 + 0x46) == '\x01') {
    bVar1 = *(byte *)(param_1 + 0x4a);
    if (1 < bVar1) {
      *(undefined1 *)(param_1 + 0x49) = 2;
      *(undefined1 *)(param_1 + 0x46) = 0xff;
      PADSEQD_OBJ_438();
      return;
    }
  }
  else {
    bVar1 = *(byte *)(param_1 + 0x4a);
    if (3 < bVar1) {
      if (*(char *)(param_1 + 0x49) != '\0') {
        (*DAT_80065260)(param_1);
      }
      goto PADSEQD_OBJ_404;
    }
  }
  *(byte *)(param_1 + 0x4a) = bVar1 + 1;
  PADSEQD_OBJ_438();
  return;
}

