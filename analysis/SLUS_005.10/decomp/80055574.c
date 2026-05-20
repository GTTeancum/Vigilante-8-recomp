// addr: 0x80055574  name: PADCMD_OBJ_8E0

void PADCMD_OBJ_8E0(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x36) = 0x4c;
  *(int *)(param_1 + 0x2c) = param_1 + 0x24;
  *(undefined1 *)(param_1 + 0x24) = param_2;
  *(undefined1 *)(param_1 + 0x35) = 1;
  return;
}

