// addr: 0x8005548c  name: PADCMD_OBJ_7F8

void PADCMD_OBJ_7F8(int param_1)

{
  if (*(char *)(param_1 + 0x46) == '\x02') {
    *(undefined1 *)(param_1 + 0x36) = 0x44;
    *(int *)(param_1 + 0x2c) = param_1 + 0x51;
    *(undefined1 *)(param_1 + 0x35) = 2;
    PADCMD_OBJ_844();
    return;
  }
  if (*(char *)(param_1 + 0x46) != '\x03') {
    PADCMD_OBJ_844();
    return;
  }
  *(undefined1 *)(param_1 + 0x36) = 0x4d;
  *(int *)(param_1 + 0x2c) = param_1 + 0x5d;
  *(undefined1 *)(param_1 + 0x35) = 6;
  return;
}

