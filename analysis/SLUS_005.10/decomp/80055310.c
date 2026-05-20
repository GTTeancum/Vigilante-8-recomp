// addr: 0x80055310  name: PADCMD_OBJ_67C

void PADCMD_OBJ_67C(int param_1)

{
  *(undefined1 *)(param_1 + 0x36) = 0x4d;
  *(undefined1 *)(param_1 + 0x35) = 6;
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_1 + 0x20);
  return;
}

